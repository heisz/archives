/**
 * JEMCC test program to test the JEMCC class packaging mechanisms.
 * Copyright (C) 1999-2004 J.M. Heisz 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU General Public License,
 * as well as further clarification on your rights to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "jeminc.h"

/* Read the JNI/JEMCC internal details */
#include "jem.h"

/* External elements from uvminit.c */
extern jint initializeTestCoreClasses(JNIEnv *env);
extern JNIEnv *createTestEnv();
extern void destroyTestEnv(JNIEnv *env);

/* Exposed test method from package.c */
extern jint JEM_LocateClassByPackage(JNIEnv *env, JEMCC_Object *loader,
                                     const char *className, 
                                     JEMCC_Class **classInst);

/* Package definition test string */
static char *libDefnData = "\n# Comment\n\n \t pkga.test \t pkgtesta  \npkga1.test \t pkgtesta\npkga2.test pkgtesta\npkgb.test\tpkgtestb\npkgb1.test\tpkgtestb\npkgb2.test\tpkgtestb\n\n pkg.badtest jembadtest \npkg.notest dummyrow\n pkg.notest jemnotest\ncore m\npkg.replace nothing\npkg.badlib dfctv\npkgc pkgtestc\npkgc.one pkgtestc\npkgc.two pkgtestc\n\n";

/* Test flags/data values for condition setups and tests */
/* Available from libpkg for linking purposes */
extern int failureTotal;
#ifdef ENABLE_ERRORSWEEP
extern int testFailureCurrentCount, testFailureCount;
#endif

/* Storage buffer for exception class and message (failure verification) */
/* Available from libpkg for linking purposes */
extern int quietMode;
extern char exClassName[], exMsg[];
extern void checkException(const char *checkClassName, const char *checkMsg,
                           const char *tstName);

/* Forward declarations */
void doValidScan(jboolean fullsweep);

/* Threaded load operations to test initialization contention */
static int activeThreadCount = 0;
void *mtLoadTestAFn(JNIEnv *env, void *userArg) {
    JEMCC_Class *tstClass;
    jint rc;

    /* Test environment has no env, but parent is passed in argument */
    env = (JNIEnv *) userArg;

    rc = JEM_LocateClassByPackage(env, NULL, "pkgc.one.TestOneA", &tstClass);
    if ((rc != JNI_OK) || (tstClass == NULL)) {
        (void) fprintf(stderr, "Unexpected return for TestOneA load\n");
        exit(1);
    }

    activeThreadCount--;
    return (void *) 0;
}
void *mtLoadTestBFn(JNIEnv *env, void *userArg) {
    JEMCC_Class *tstClass;
    jint rc;

    /* Test environment has no env, but parent is passed in argument */
    env = (JNIEnv *) userArg;

    rc = JEM_LocateClassByPackage(env, NULL, "pkgc.one.TestOneB", &tstClass);
    if ((rc != JNI_OK) || (tstClass == NULL)) {
        (void) fprintf(stderr, "Unexpected return for TestOneB load\n");
        exit(1);
    }

    activeThreadCount--;
    return (void *) 0;
}
void *mtLoadTestCFn(JNIEnv *env, void *userArg) {
    JEMCC_Class *tstClass;
    jint rc;

    /* Test environment has no env, but parent is passed in argument */
    env = (JNIEnv *) userArg;

    rc = JEM_LocateClassByPackage(env, NULL, "pkgc.one.TestOneC", &tstClass);
    if ((rc != JNI_OK) || (tstClass == NULL)) {
        (void) fprintf(stderr, "Unexpected return for TestOneC load\n");
        exit(1);
    }

    activeThreadCount--;
    return (void *) 0;
}

/* Send the JEMCC class definition methods through their paces */
int main(int argc, char *argv[]) {
    JNIEnv *env;
    JEMCC_Class *tstClass;
    JEMCC_ThreadId threadId;
    char fileData[1024];
    jint rc;

    /* No memory test just yet */
#ifdef ENABLE_ERRORSWEEP
    testFailureCount = -1;
#endif
    quietMode = JNI_TRUE;

    /* Initialize operating environment */
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, 
                       "Unexpected fatal error during test env setup\n");
        exit(1);
    }

    /* Create our initial base classes (Object, Class, Serializable) */
    initializeTestCoreClasses(env);

    /* Setup path/package information */
    if (JEM_ParsePathList(env, &((((JEM_JNIEnv *) env)->parentVM)->libPath),
                          ":.:jemcc::./.libs:./libdata:../jni//:/lib:/usr/lib:",
                          JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Unexpected failure of library path parsing\n");
        exit(1);
    }
    (void) strcpy(fileData, libDefnData);
    if (JEM_ParseLibPackageInfo(env, fileData,
                                strlen(fileData)) != JNI_OK) {
        (void) fprintf(stderr, "Unexpected failure of package defn parsing\n");
        exit(1);
    }

    /* Try some unknown/impossible packages */
    rc = JEM_LocateClassByPackage(env, NULL, "no.such.pkg.class", &tstClass);
    if ((rc != JNI_OK) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for unknown package\n");
        exit(1);
    }
    rc = JEM_LocateClassByPackage(env, NULL, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.Test", &tstClass);
    if ((rc != JNI_OK) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for ridiculous package\n");
        exit(1);
    }

    /* Try a package without a corresponding library to load */
    rc = JEM_LocateClassByPackage(env, NULL, "pkg.notest.Test", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for missing library\n");
        exit(1);
    }

    /* Try a package with a damaged library instance */
    quietMode = JNI_FALSE;
    *exClassName = *exMsg = '\0';
    rc = JEM_LocateClassByPackage(env, NULL, "pkg.badlib.Test", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for defective library\n");
        exit(1);
    }
    checkException("UnsatisfiedLinkError", NULL, "defective library");

    /* Try a package for a non-JEMCC library */
    *exClassName = *exMsg = '\0';
    rc = JEM_LocateClassByPackage(env, NULL, "core.Test", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for non-JEMCC library\n");
        exit(1);
    }
    checkException("UnsatisfiedLinkError", NULL, "non-JEMCC library");

    /* Proper library, initialization failure on JNI_OnLoad call */
    *exClassName = *exMsg = '\0';
    rc = JEM_LocateClassByPackage(env, NULL, "pkga2.test.Test", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for JNI_OnLoad failure\n");
        exit(1);
    }
    checkException("UnsatisfiedLinkError", NULL, "failed JNI_OnLoad");

    /* Proper library, initialization failure on JEMCC_Init call */
    *exClassName = *exMsg = '\0';
    rc = JEM_LocateClassByPackage(env, NULL, "pkga1.test.Test", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for JEMCC_Init failure\n");
        exit(1);
    }
    checkException("LinkageError", "forced", "failed JEMCC_Init");

    /* Third try should be successful, but defines different class */
    quietMode = JNI_TRUE;
    rc = JEM_LocateClassByPackage(env, NULL, "pkga.test.Test", &tstClass);
    if ((rc != JNI_OK) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for different pkga class\n");
        exit(1);
    }

    /* Proper library, initialization failure in JEMCC_pkg_Init call */
    quietMode = JNI_FALSE;
    *exClassName = *exMsg = '\0';
    rc = JEM_LocateClassByPackage(env, NULL, "pkgb2.test.Test", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for JEMCC_pkg_Init fail\n");
        exit(1);
    }
    checkException("LinkageError", "forced", "failed JEMCC_pkg_Init");

    /* Try again, will fail in the second leg of the initialization chain */
    *exClassName = *exMsg = '\0';
    rc = JEM_LocateClassByPackage(env, NULL, "pkgb1.test.Test", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for 2nd init failure\n");
        exit(1);
    }
    checkException("LinkageError", "forced", "failed 2nd init");

    /* Try again, will fail in the classloader leg of init chain */
    *exClassName = *exMsg = '\0';
    rc = JEM_LocateClassByPackage(env, NULL, "pkgb.test.DeadZone", &tstClass);
    if ((rc != JNI_ERR) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for classloader failure\n");
        exit(1);
    }
    checkException("LinkageError", "forced", "failed classloader init");

    /* Try again, no more failures but will not be a proper match */
    quietMode = JNI_TRUE;
    rc = JEM_LocateClassByPackage(env, NULL, "pkgb.test.NoChance", &tstClass);
    if ((rc != JNI_OK) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for unrecognized class\n");
        exit(1);
    }

    /* Check the indirect definition of the pkgbb classloader */
    rc = JEM_LocateClassByPackage(env, NULL, "pkgbb.test.RealClass", &tstClass);
    if ((rc != JNI_OK) || (tstClass == NULL)) {
        (void) fprintf(stderr, "Unexpected return for pkgbb class\n");
        exit(1);
    }

    /* Clean up from error cases */
    destroyTestEnv(env);

    /***** Multi-threaded case *****/

    /* Initialize operating environment */
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, 
                       "Unexpected fatal error during test env setup\n");
        exit(1);
    }

    /* Create our initial base classes (Object, Class, Serializable) */
    initializeTestCoreClasses(env);

    /* Setup path/package information */
    if (JEM_ParsePathList(env, &((((JEM_JNIEnv *) env)->parentVM)->libPath),
                          ":.:jemcc::./.libs:./libdata:../jni//:/lib:/usr/lib:",
                          JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Unexpected failure of library path parsing\n");
        exit(1);
    }
    (void) strcpy(fileData, libDefnData);
    if (JEM_ParseLibPackageInfo(env, fileData,
                                strlen(fileData)) != JNI_OK) {
        (void) fprintf(stderr, "Unexpected failure of package defn parsing\n");
        exit(1);
    }

    /* Launch some threads, then wait for completion */
    activeThreadCount++;
    threadId = JEMCC_CreateThread(NULL, mtLoadTestAFn, env, 1);
    if (threadId == 0) {
        (void) fprintf(stderr, "Error: thread A create failure\n");
        exit(1);
    }
    activeThreadCount++;
    threadId = JEMCC_CreateThread(NULL, mtLoadTestBFn, env, 1);
    if (threadId == 0) {
        (void) fprintf(stderr, "Error: thread B create failure\n");
        exit(1);
    }
    activeThreadCount++;
    threadId = JEMCC_CreateThread(NULL, mtLoadTestCFn, env, 1);
    if (threadId == 0) {
        (void) fprintf(stderr, "Error: thread C create failure\n");
        exit(1);
    }
    while (activeThreadCount > 0) sleep(1);

    /* See if the pkgc.two class survived the conflicts */
    rc = JEM_LocateClassByPackage(env, NULL, "pkgc.two.TestTwo", &tstClass);
    if ((rc != JNI_OK) || (tstClass == NULL)) {
        (void) fprintf(stderr, "Unexpected return for TestTwo load\n");
        exit(1);
    }

    /* All cleaned up */
    destroyTestEnv(env);

    /***** Memory failure scanning *****/
#ifdef ENABLE_ERRORSWEEP
    testFailureCurrentCount = 0;
    doValidScan(JNI_TRUE);
    failureTotal = testFailureCurrentCount;
    (void) fprintf(stderr, "Preparing to process %i memory failures\n",
                           failureTotal);
    for (testFailureCount = 1;
             testFailureCount <= failureTotal;
                 testFailureCount++) {
        testFailureCurrentCount = 0;
        doValidScan(JNI_FALSE);
    }
#else
    failureTotal = 0;
    doValidScan(JNI_TRUE);
#endif

    (void) fprintf(stderr, "All tests passed successfully (%i forced)\n",
                           failureTotal);
    exit(0);
}

static jint staticPackageInitFn(JNIEnv *env, JEMCC_Object *loader,
                                const char *pkgName,
                                JEMCC_PkgInitData *initData) {
    return JNI_ERR;
}

static jint staticPackageClassLdr(JNIEnv *env, JEMCC_Object *loader,
                                  const char *className,
                                  JEMCC_Class **classInst) {
    return JNI_ERR;
}

void doValidScan(jboolean fullsweep) {
    JNIEnv *env;
    char fileData[1024];
    jint rc;
    JEMCC_MultiPackageInitData pkgData[5];
    JEMCC_Class *tstClass;
#ifdef ENABLE_ERRORSWEEP
    int tmpFailCount = testFailureCount;
    int tmpFailCurrentCount = testFailureCurrentCount;
    testFailureCount = -1;
#endif

    /* Initialize operating environment */
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, 
                       "Unexpected fatal error during test env setup\n");
        exit(1);
    }

    /* Create our initial base classes (Object, Class, Serializable) */
    initializeTestCoreClasses(env);

    /* Add the library source path */
    if (JEM_ParsePathList(env, &((((JEM_JNIEnv *) env)->parentVM)->libPath),
                          ":.:jemcc::./.libs:./libdata:../jni//:/lib:/usr/lib:",
                          JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Unexpected failure of library path parsing\n");
        exit(1);
    }

#ifdef ENABLE_ERRORSWEEP
    testFailureCount = tmpFailCount;
    testFailureCurrentCount = tmpFailCurrentCount;
#endif

    /* Try out the static package registration methods */
    rc = JEMCC_RegisterPkgInitFn(env, NULL, "static.inittest",
                                 staticPackageInitFn);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Package init fn registration failed\n");
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        destroyTestEnv(env);
        return;
    }
    if (fullsweep != JNI_FALSE) {
        rc = JEMCC_RegisterPkgInitFn(env, NULL, "static.inittest",
                                     staticPackageInitFn);
        if (rc != JNI_ERR) {
            (void) fprintf(stderr, "Package init fn redefine successful?\n");
            exit(1);
        }
    }

    rc = JEMCC_RegisterPkgClassLdrFn(env, NULL, "static.loadertest",
                                     staticPackageClassLdr);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Package loader registration failed\n");
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        destroyTestEnv(env);
        return;
    }
    if (fullsweep != JNI_FALSE) {
        rc = JEMCC_RegisterPkgInitFn(env, NULL, "static.loadertest",
                                     staticPackageInitFn);
        if (rc != JNI_ERR) {
            (void) fprintf(stderr, "Package loader redefine successful?\n");
            exit(1);
        }
    }

    /* Initialize the package load definitions */
    (void) strcpy(fileData, libDefnData);
    if (JEM_ParseLibPackageInfo(env, fileData,
                                strlen(fileData)) != JNI_OK) {
        destroyTestEnv(env);
        return;
    }

    /* Attempt a replace operation */
    pkgData[0].pkgName = "static.inittest";
    pkgData[0].initFn = NULL;
    pkgData[0].loaderFn = staticPackageClassLdr;
    pkgData[1].pkgName = "static.inittest";
    pkgData[1].initFn = staticPackageInitFn;
    pkgData[1].loaderFn = NULL;
    pkgData[2].pkgName = "pkg.replace";
    pkgData[2].initFn = NULL;
    pkgData[2].loaderFn = NULL;
    rc = JEMCC_RegisterMultiPkgInitFn(env, NULL, pkgData, 3);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Multi-package loader registration failed\n");
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        destroyTestEnv(env);
        return;
    }

    /* Run through the JNI_OnLoad/JEMCC_Init case (no errors) */
    rc = JEM_LocateClassByPackage(env, NULL, "pkga.test.Test", &tstClass);
    if (rc == JNI_ENOMEM) {
        /* Try again to test package recovery */
        rc = JEM_LocateClassByPackage(env, NULL, "pkga.test.Test", &tstClass);
        if ((rc != JNI_OK) || (tstClass != NULL)) {
            (void) fprintf(stderr, "Unexpected return for pkga class reload\n");
            exit(1);
        }
        destroyTestEnv(env);
        return;
    }
    if ((rc != JNI_OK) || (tstClass != NULL)) {
        (void) fprintf(stderr, "Unexpected return for pkga class load\n");
        exit(1);
    }

    /* Likewise for the JEMCC_pkg_Init/Classloader case (no errors) */
    rc = JEM_LocateClassByPackage(env, NULL, "pkgb.test.TestTwo", &tstClass);
    if (rc == JNI_ENOMEM) {
        /* Try again to test package recovery */
        rc = JEM_LocateClassByPackage(env, NULL, "pkgb.test.TestTwo", 
                                      &tstClass);
        if ((rc != JNI_OK) || (tstClass == NULL)) {
            (void) fprintf(stderr, 
                           "Unexpected return for pkgb class redefine\n");
            exit(1);
        }
        destroyTestEnv(env);
        return;
    }
    if ((rc != JNI_OK) || (tstClass == NULL)) {
        (void) fprintf(stderr, "Unexpected return for pkgb defined class\n");
        exit(1);
    }

    /* Try for a real class */
    rc = JEM_LocateClassByPackage(env, NULL, "pkgb.test.RealClass", &tstClass);
    if (rc == JNI_ENOMEM) {
        /* Try again to test package recovery */
        rc = JEM_LocateClassByPackage(env, NULL, "pkgb.test.RealClass", 
                                      &tstClass);
        if ((rc != JNI_OK) || (tstClass == NULL)) {
            (void) fprintf(stderr, "Unexpected return for RealClass reload\n");
            exit(1);
        }
        destroyTestEnv(env);
        return;
    }
    if ((rc != JNI_OK) || (tstClass == NULL)) {
        (void) fprintf(stderr, "Unexpected return for RealClass load\n");
        exit(1);
    }

    /* Clean up to validate purify operation */
    destroyTestEnv(env);
}
