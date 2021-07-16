/**
 * Initialization test cases for the package test program (multi-threaded).
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

/* Read the jni/jem internal details */
#include "jem.h"

/* Forward declarations */
jint JEMCC_One_Init(JNIEnv *env, JEMCC_Object *loader, 
                    const char *pkgName, JEMCC_PkgInitData *initData);
jint JEMCC_Two_Init(JNIEnv *env, JEMCC_Object *loader, 
                    const char *pkgName, JEMCC_PkgInitData *initData);
jint JEMCC_CClassLoader(JNIEnv *env, JEMCC_Object *loader,
                        const char *className, JEMCC_Class **classInstance);

/* Have a snooze and let the other thread come into conflict */
static void ranSleep() {
    int time = (int) (random() & 0x7FFF);
    usleep(time);
}

/**
 * Package specific JEMCC class initialization method.  This implementation
 * will insert random delays while defining 'pkgc.Test', two nested
 * initializers and itself as complete.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the package initialization is to
 *              occur, NULL if the VM bootstrap loader is to be used
 *     pkgName - the name of the package being initialized.  Allows for
 *               multi-package library initialization (ignored)
 *     initData - internal data structure through which to pass initialization
 *                state and chain initialization functions
 */
jint JEMCC_pkgtestc_Init(JNIEnv *env, JEMCC_Object *loader, 
                         const char *pkgName, JEMCC_PkgInitData *initData) {
    jint rc;
    JEMCC_MultiPackageInitData pkgData[5];

    ranSleep();
    rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                              "pkgc.Test", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0, NULL, 0, NULL, NULL);
    if (rc != JNI_OK) return rc;

    /* Initialize the subclasses, overlaying the caller */
    pkgData[0].pkgName = "pkgc.one";
    pkgData[0].initFn = JEMCC_One_Init;
    pkgData[0].loaderFn = NULL;
    pkgData[1].pkgName = "pkgc.two";
    pkgData[1].initFn = JEMCC_Two_Init;
    pkgData[1].loaderFn = NULL;
    pkgData[2].pkgName = "pkgc";
    pkgData[2].initFn = NULL;
    pkgData[2].loaderFn = NULL;

    ranSleep();
    return JEMCC_RegisterMultiPkgInitFn(env, loader, pkgData, 3);
}

/**
 * Sub-package (one) specific JEMCC class initialization method.  This 
 * implementation will insert random delays while defining 'pkgc.one.Test'
 * and then passing on to the classloader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the package initialization is to
 *              occur, NULL if the VM bootstrap loader is to be used
 *     pkgName - the name of the package being initialized.  Allows for
 *               multi-package library initialization (ignored)
 *     initData - internal data structure through which to pass initialization
 *                state and chain initialization functions
 */
jint JEMCC_One_Init(JNIEnv *env, JEMCC_Object *loader, 
                    const char *pkgName, JEMCC_PkgInitData *initData) {
    jint rc;

    ranSleep();
    rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                              "pkgc.one.Test", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0, NULL, 0, NULL, NULL);
    if (rc != JNI_OK) return rc;

    /* Switch to package classloader mode */
    ranSleep();
    initData->initState = PKGLOAD_CLASSLDR_FN;
    initData->handler.pkgClassLdrFn = JEMCC_CClassLoader;

    return JNI_OK;
}

/**
 * Sub-package (two) specific JEMCC class initialization method.  This 
 * implementation will insert random delays while defining 'pkgc.two.Test'
 * and then passing on to the classloader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the package initialization is to
 *              occur, NULL if the VM bootstrap loader is to be used
 *     pkgName - the name of the package being initialized.  Allows for
 *               multi-package library initialization (ignored)
 *     initData - internal data structure through which to pass initialization
 *                state and chain initialization functions
 */
jint JEMCC_Two_Init(JNIEnv *env, JEMCC_Object *loader, 
                    const char *pkgName, JEMCC_PkgInitData *initData) {
    jint rc;

    ranSleep();
    rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                              "pkgc.two.Test", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0, NULL, 0, NULL, NULL);
    if (rc != JNI_OK) return rc;

    /* Switch to package classloader mode */
    ranSleep();
    initData->initState = PKGLOAD_CLASSLDR_FN;
    initData->handler.pkgClassLdrFn = JEMCC_CClassLoader;

    return JNI_OK;
}

/**
 * Classloader method used to "locate" and load JEMCC class instances
 * for the child packages.  Will define 'pkgc.one.TestOne[ABC]' and 
 * 'pkgc.two.TestTwo' as requested.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the class definition is to
 *              occur, NULL if the VM bootstrap loader is to be used
 *     className - the name of the class to be loaded.  In this case,
 *                 only the class name "RealClass" is recognized
 *     classInst - a reference pointer through which a loaded class instance
 *                 is returned
 */
jint JEMCC_CClassLoader(JNIEnv *env, JEMCC_Object *loader,
                        const char *className, JEMCC_Class **classInst) {
    jint rc;

    /* Create specific classes on demand */
    ranSleep();
    if (strcmp(className, "TestOneA") == 0) {
       rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                                 "pkgc.one.TestOneA", NULL, NULL, 0,
                                 NULL, 0, NULL, NULL, 0, NULL, 0, 
                                 NULL, classInst);
       return rc;
    }
    if (strcmp(className, "TestOneB") == 0) {
       rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                                 "pkgc.one.TestOneB", NULL, NULL, 0,
                                 NULL, 0, NULL, NULL, 0, NULL, 0, 
                                 NULL, classInst);
       return rc;
    }
    if (strcmp(className, "TestOneC") == 0) {
       rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                                 "pkgc.one.TestOneC", NULL, NULL, 0,
                                 NULL, 0, NULL, NULL, 0, NULL, 0, 
                                 NULL, classInst);
       return rc;
    }
    if (strcmp(className, "TestTwo") == 0) {
       rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                                 "pkgc.two.TestTwo", NULL, NULL, 0,
                                 NULL, 0, NULL, NULL, 0, NULL, 0, 
                                 NULL, classInst);
       return rc;
    }

    ranSleep();
    return JNI_EINVAL;
}
