/**
 * Initialization test cases for the package test program (package specific).
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
jint JEMCC_Second_Init(JNIEnv *env, JEMCC_Object *loader, 
                       const char *pkgName, JEMCC_PkgInitData *initData);
jint JEMCC_Third_Init(JNIEnv *env, JEMCC_Object *loader, 
                      const char *pkgName, JEMCC_PkgInitData *initData);
jint JEMCC_ClassLoader(JNIEnv *env, JEMCC_Object *loader,
                       const char *className, JEMCC_Class **classInstance);

/**
 * This "library" is almost identical to the pkgtesta.c file, except that
 * it would not be possible to "hide" the JEMCC_Init method.  Hence this
 * version, which tests the library specific initializer.  The passFlag is
 * very basic, as only one failure type needs to be tested.
 *
 *     0 - the JEMCC_***_Init method will simulate a class creation failure
 *     1 - the initialization function will define "TestTwo" and pass on to
 *         the second bulk initializater.  Next request will fail in the
 *         second initializer
 *     2 - the second initializer will define "TestThree" and install
 *         a set of initializers (including itself).  That initializer will
 *         install a direct classloader.  Next request will fail from the 
 *         classloader.
 *     3 - direct classloader will work properly, creating a "RealClass"
 *         if requested and returning invalid otherwise
 */
static int passFlag = 0;

/**
 * Package specific JEMCC class initialization method.  This implementation
 * will either fail according to the passFlag setting or perform the basic 
 * definition of "TestTwo" and then insert the second bulk initializer method.
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
jint JEMCC_pkgtestb_Init(JNIEnv *env, JEMCC_Object *loader, 
                         const char *pkgName, JEMCC_PkgInitData *initData) {
    jint rc;

    /* Fail on the first pass */
    if (passFlag == 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_LinkageError, NULL,
                                   "JEMCC_pkgtestb_Init: forced JEMCC failure");
        passFlag++;
        return JNI_ERR;
    }

    /* Build a class, but not the one that is being looked for */
    rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                              "pkgb.test.TestTwo", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0, NULL, 0, NULL, NULL);
    if (rc != JNI_OK) return rc;

    /* Next initializer is responsible for more havoc */
    initData->handler.pkgInitFn = JEMCC_Second_Init;

    return rc;
}

/**
 * Second package specific JEMCC class initialization method.  This 
 * implementation will either fail according the the passFlag setting or
 * perform the basic definition of "TestThree" and then insert the third
 * initializer function.
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
jint JEMCC_Second_Init(JNIEnv *env, JEMCC_Object *loader, 
                       const char *pkgName, JEMCC_PkgInitData *initData) {
    jint rc;
    JEMCC_MultiPackageInitData pkgData[5];

    /* Fail on the first pass */
    if (passFlag == 1) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_LinkageError, NULL,
                                   "JEMCC_Second_Init: forced JEMCC failure");
        passFlag++;
        return JNI_ERR;
    }

    /* Build a class, but not the one that is being looked for */
    rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                              "pkgb.test.TestThree", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0, NULL, 0, NULL, NULL);
    if (rc != JNI_OK) return rc;

    /* Now insert the third class initializer (and a companion) */
    pkgData[0].pkgName = "pkgb.test";
    pkgData[0].initFn = JEMCC_Third_Init;
    pkgData[0].loaderFn = NULL;
    pkgData[1].pkgName = "pkgbb.test";
    pkgData[1].initFn = NULL;
    pkgData[1].loaderFn = JEMCC_ClassLoader;

    return JEMCC_RegisterMultiPkgInitFn(env, loader, pkgData, 2);
}

/**
 * Third package specific JEMCC class initialization method.  This 
 * implementation will simply pass on to the classloader instance.
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
jint JEMCC_Third_Init(JNIEnv *env, JEMCC_Object *loader, 
                       const char *pkgName, JEMCC_PkgInitData *initData) {
    /* Switch to package classloader mode */
    initData->initState = PKGLOAD_CLASSLDR_FN;
    initData->handler.pkgClassLdrFn = JEMCC_ClassLoader;

    return JNI_OK;
}

/**
 * Classloader method used to "locate" and load JEMCC class instances
 * for this package.  This classloader with either fail according to the
 * passFlag setting or will load a "RealClass" if requested or return
 * invalid if not.
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
jint JEMCC_ClassLoader(JNIEnv *env, JEMCC_Object *loader,
                       const char *className, JEMCC_Class **classInst) {
    jint rc;

    /* Fail on the first pass */
    if (passFlag == 2) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_LinkageError, NULL,
                                   "JEMCC_ClassLoader: forced JEMCC failure");
        passFlag++;
        return JNI_ERR;
    }

    /* Produce the class if requested */
    if (strcmp(className, "RealClass") == 0) {
       rc = JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                                 "pkgb.test.RealClass", NULL, NULL, 0,
                                 NULL, 0, NULL, NULL, 0, NULL, 0, 
                                 NULL, classInst);
       return rc;
    }

    return JNI_EINVAL;
}
