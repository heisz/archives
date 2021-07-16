/**
 * JEMCC program to validate high-level class ops (arrays and complex cases)
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
extern JNIEnv *createTestEnv();
extern void destroyTestEnv(JNIEnv *env);

/* Test flags/data values for condition setups and tests */
int failureTotal;
#ifdef ENABLE_ERRORSWEEP
int testFailureCurrentCount, testFailureCount;

int JEM_CheckErrorSweep(int sweepType) {
    /* Note: don't care about sweep type */
    testFailureCurrentCount++;
    if ((testFailureCount >= 0) &&
        (testFailureCurrentCount == testFailureCount)) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}
#endif

/*
 * Different implementation from more basic tests.  This disassembles
 * the pendingException for checking and clears it as well.
 */
void destroyException(JNIEnv *env, JEMCC_Object *exception) {
    JEMCC_ThrowableData *exData = (JEMCC_ThrowableData *)
                             &(((JEMCC_ObjectExt *) exception)->objectData);

    if (exData->message != NULL) {
         JEMCC_Free(((JEMCC_ObjectExt *) (exData->message))->objectData);
         JEMCC_Free(exData->message);
    }
    JEMCC_Free(exception);
}

void checkException(JNIEnv *env, const char *checkClassName, 
                    const char *checkMsg, const char *tstName) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_ThrowableData *exData = NULL;
    JEMCC_StringData *strData = NULL;
    char *msg = NULL, *className;;

    if ((checkClassName == NULL) && (checkMsg == NULL)) {
        if (jenv->pendingException != NULL) {
            (void) fprintf(stderr, "Unexpected exception thrown for %s\n",
                                   tstName);
            exit(1);
        }
    }
    if (jenv->pendingException == NULL) {
        (void) fprintf(stderr, "Expected exception not thrown for %s\n",
                               tstName);
        exit(1);
    }

    /* Compare with the string internals */
    exData = (JEMCC_ThrowableData *) 
                   &(((JEMCC_ObjectExt *) jenv->pendingException)->objectData);
    if (exData->message == NULL) {
        msg = "(null)";
    } else {
        strData = (JEMCC_StringData *)
                    ((JEMCC_ObjectExt *) (exData->message))->objectData;
        msg = &(strData->data);
    }
    if ((checkMsg != NULL) && (strstr(msg, checkMsg) == NULL)) {
        (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
        (void) fprintf(stderr, "Expected %s, got %s instead.\n",
                               checkMsg, msg);
        exit(1);
    }
    if (checkClassName != NULL) {
        className = 
                jenv->pendingException->classReference->classData->className;
        if (strstr(className, checkClassName) == NULL) {
            (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
            (void) fprintf(stderr, "Expected '%s', got '%s' instead.\n",
                                   checkClassName, className);
            exit(1);
        }
    }

    /* Perform some cleanup */
    destroyException(env, jenv->pendingException);
    jenv->pendingException = NULL;
}

/* Thread functions to test race conditions */
static int raceCount = 0;
static jbyte raceClassB[] = { /* public class B extends A */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x00, 0x00, 0x2e,
        0x00, 0x0d, 0x0a, 0x00, 0x03, 0x00, 0x0a, 0x07,
        0x00, 0x0b, 0x07, 0x00, 0x0c, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x0a,
        0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69,
        0x6c, 0x65, 0x01, 0x00, 0x06, 0x42, 0x2e, 0x6a,
        0x61, 0x76, 0x61, 0x0c, 0x00, 0x04, 0x00, 0x05,
        0x01, 0x00, 0x0b, 0x74, 0x65, 0x73, 0x74, 0x2f,
        0x72, 0x61, 0x63, 0x65, 0x2f, 0x42, 0x01, 0x00,
        0x0b, 0x74, 0x65, 0x73, 0x74, 0x2f, 0x72, 0x61,
        0x63, 0x65, 0x2f, 0x41, 0x00, 0x21, 0x00, 0x02,
        0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x01, 0x00, 0x04, 0x00, 0x05, 0x00, 0x01,
        0x00, 0x06, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x01,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x2a, 0xb7,
        0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x08, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x09
};
static jbyte raceClassC[] = { /* public class C extends B */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x00, 0x00, 0x2e,
        0x00, 0x0d, 0x0a, 0x00, 0x03, 0x00, 0x0a, 0x07,
        0x00, 0x0b, 0x07, 0x00, 0x0c, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x0a,
        0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69,
        0x6c, 0x65, 0x01, 0x00, 0x06, 0x43, 0x2e, 0x6a,
        0x61, 0x76, 0x61, 0x0c, 0x00, 0x04, 0x00, 0x05,
        0x01, 0x00, 0x0b, 0x74, 0x65, 0x73, 0x74, 0x2f,
        0x72, 0x61, 0x63, 0x65, 0x2f, 0x43, 0x01, 0x00,
        0x0b, 0x74, 0x65, 0x73, 0x74, 0x2f, 0x72, 0x61,
        0x63, 0x65, 0x2f, 0x42, 0x00, 0x21, 0x00, 0x02,
        0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x01, 0x00, 0x04, 0x00, 0x05, 0x00, 0x01,
        0x00, 0x06, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x01,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x2a, 0xb7,
        0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x08, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x09
};

void *classLoadRaceFn(JNIEnv *env, void *userArg) {
    JEM_ParsedClassData *pData;
    JEMCC_Class *tstClass;

    /* Wait for start and check exit condition */
    while (raceCount == 0) usleep(100000);
    if (raceCount < 0) return (void *) 0;

    if (userArg == (void *) 0) {
        /* Parse B, which delays on A resolution */
        pData = JEM_ParseClassData(env, raceClassB, 181);
        if (pData != NULL) {
            tstClass = JEM_DefineAndResolveClass(env,
                         ((JEM_JNIEnv *) env)->parentVM->systemClassLoader,
                         pData);
        }
    } else {
        /* Parse C, which will wait on B resolution */
        pData = JEM_ParseClassData(env, raceClassC, 181);
        if (pData != NULL) {
            tstClass = JEM_DefineAndResolveClass(env,
                         ((JEM_JNIEnv *) env)->parentVM->systemClassLoader,
                         pData);
        }
    }

    /* Let everyone know (safely) that we are done */
    JEMCC_EnterGlobalMonitor();
    raceCount++;
    JEMCC_ExitGlobalMonitor();

    return (void *) 0;
}

/* Package definition method to support above methods */
jint racePkgClassLoader(JNIEnv *env, JEMCC_Object *loader,
                        const char *className, JEMCC_Class **classInst) {
    JEMCC_Class *objClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Object);
    jint rc;

    if (strcmp(className, "A") == 0) {
        sleep(1);
        rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC,
                                  "test.race.A", objClass,
                                  NULL, 0, NULL, 0, NULL, NULL, 0, NULL, 0,
                                  NULL, classInst);
        return rc;
    }

    return JNI_EINVAL;
}

/* Forward declarations */
void doValidScan(jboolean fullsweep);

/* Send the JEMCC class definition methods through their paces */
int main(int argc, char *argv[]) {
    JNIEnv *env;
    JEMCC_Class *tstClass;
    jint rc;

    /* No memory test just yet */
#ifdef ENABLE_ERRORSWEEP
    testFailureCount = -1;
#endif

    /* Initialize operating machines */
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, "Fatal test env initialization error\n");
        exit(1);
    }

    /* Load initial VM class instances */
    rc = JEM_InitializeVMClasses(env);
    if (rc != JNI_OK) {
        (void) fprintf(stderr, "Unexpected failure in VM class init\n");
        exit(1);
    }

    /* Try for some invalid array instances (in different modes) */
    if (JEMCC_LocateClass(env, NULL, "[[[[[[[[[", 
                          JNI_TRUE, &tstClass) != JNI_ERR) {
        (void) fprintf(stderr, "Unexpected return from no-component array\n");
        exit(1);
    }
    checkException(env, "NoClassDefFoundError", "Missing array component",
                   "no-component array");
    if (JEMCC_LocateClass(env, NULL, "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[B",
                          JNI_FALSE, &tstClass) != JNI_ERR) {
        (void) fprintf(stderr, "Unexpected return from too deep array\n");
        exit(1);
    }
    checkException(env, "ClassNotFoundException", "depth over",
                   "array too deep (>255)");
    if (JEMCC_LocateClass(env, NULL, "[[[[[[[[[XYZ", 
                          JNI_TRUE, &tstClass) != JNI_ERR) {
        (void) fprintf(stderr, "Unexpected return from invalid array comp\n");
        exit(1);
    }
    checkException(env, "NoClassDefFoundError", 
                   "Invalid array component descriptor",
                   "invalid array component descriptor");
    if (JEMCC_LocateClass(env, NULL, "[[[[[[[[[(BI)V", 
                          JNI_TRUE, &tstClass) != JNI_ERR) {
        (void) fprintf(stderr, "Unexpected return from bad array comp type\n");
        exit(1);
    }
    checkException(env, "NoClassDefFoundError", "Invalid array component type",
                   "invalid array component descriptor type");
    if (JEMCC_LocateClass(env, NULL, "[[[[[Lno.such.class;", 
                          JNI_FALSE, &tstClass) != JNI_EINVAL) {
        (void) fprintf(stderr, "Unexpected return from bad array comp class\n");
        exit(1);
    }
    checkException(env, "ClassNotFoundException", "no.such.class",
                   "invalid array component class");
    if (JEMCC_LocateClass(env, NULL, "[Lnot.this.class.either;", 
                          JNI_TRUE, &tstClass) != JNI_EINVAL) {
        (void) fprintf(stderr, "Unexpected return from bad array comp class\n");
        exit(1);
    }
    checkException(env, "NoClassDefFoundError", "not.this.class.either",
                   "invalid array component class");

    /* Test the quiet-mode local array build */
    if (JEM_BuildArrayClass(env, NULL, "[Lnot.this.class;", 
                            JNI_TRUE, JNI_TRUE, &tstClass) != JNI_EINVAL) {
        (void) fprintf(stderr, "Unexpected rtrn from local array comp class\n");
        exit(1);
    }

    /* All done, clean up the mess */
    destroyTestEnv(env);

    /* Memory failure scanning */
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

void doValidScan(jboolean fullsweep) {
    JNIEnv *env;
    JEMCC_Class *tstClass;
    jint rc;
#ifdef ENABLE_ERRORSWEEP
    int tmpFailCount = testFailureCount;
    int tmpFailCurrentCount = testFailureCurrentCount;
    testFailureCount = -1;
#endif

    /* Initialize operating machines */
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, "Fatal test env initialization error\n");
        exit(1);
    }

#ifdef ENABLE_ERRORSWEEP
    testFailureCount = tmpFailCount;
    testFailureCurrentCount = tmpFailCurrentCount;
#endif

    /* Load initial VM class instances */
    rc = JEM_InitializeVMClasses(env);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected failure in VM class init\n");
            exit(1);
        }
        destroyTestEnv(env);
        return;
    }

    /* Load some arrays */
    rc = JEMCC_LocateClass(env, NULL, "[B", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from prim array get\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[[[[B", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from byte array build\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[C", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from char array build\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[[[D", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from dbl array build\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[[[[[[[F", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from flt array build\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[I", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from int array build\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[[[[J", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from long array build\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[[[S", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from shrt array build\n");
            exit(1);
        }
    }
    rc = JEMCC_LocateClass(env, NULL, "[[[[[[[[[Z", JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from bool array build\n");
            exit(1);
        }
    }
/*
 * TODO - uncomment following once exception capture in place *
    rc = JEMCC_LocateClass(env, NULL, "[Ljava.lang.Class;", 
                           JNI_FALSE, &tstClass);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from cls array build\n");
            exit(1);
        }
    }
*/

    /* Validate the race conditions */
    rc = JEMCC_RegisterPkgClassLdrFn(env, 
                            ((JEM_JNIEnv *) env)->parentVM->systemClassLoader,
                            "test.race", racePkgClassLoader);
    if (rc != JNI_OK) {
        if ((rc != JNI_ENOMEM) || (fullsweep == JNI_TRUE)) {
            (void) fprintf(stderr, "Unexpected return from race pkg define\n");
            exit(1);
        }
    } else {
        raceCount = 0;
        if (JEMCC_CreateThread(env, classLoadRaceFn, (void *) 0, 1) == 0) {
            if (fullsweep == JNI_TRUE) {
                (void) fprintf(stderr, "Error: failed on thread create\n");
                exit(1);
            }
            raceCount = -1;
        } else {
            usleep(100000);
            if (JEMCC_CreateThread(env, classLoadRaceFn, (void *) 1, 1) == 0) {
                if (fullsweep == JNI_TRUE) {
                    (void) fprintf(stderr, "Error: failed on thread create\n");
                    exit(1);
                }
                raceCount = -1;
            } else {
                raceCount = 1;
            }
        }
        usleep(100000);
        while ((raceCount > 0) && (raceCount < 3)) usleep(100000);
    }

    /* Clean up to validate purify operation */
    destroyTestEnv(env);
}

/* Local methods to avoid full library inclusion */
void *JEMCC_Malloc(JNIEnv *env, juint size) {
#ifdef ENABLE_ERRORSWEEP
    if (JEM_CheckErrorSweep(ES_MEM) == JNI_TRUE) {
        (void) fprintf(stderr, "Error[local]: Simulated malloc failure\n");
        return NULL;
    }
#endif
    return calloc(1, size);
}

jint JEM_CallForeignFunction(JNIEnv *env, JEMCC_Object *thisObj, void *fnRef,
                             union JEM_DescriptorInfo *fnDesc,
                             JEMCC_ReturnValue *argList,
                             JEMCC_ReturnValue *retVal) {
    return JNI_OK;
}
