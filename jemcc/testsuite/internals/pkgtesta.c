/**
 * Initialization test cases for the package test program (generic).
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

/**
 * To handle all of the possible test cases without creating a large number
 * of library instances, this flag is used to manage different "phases" of the
 * package initialization.  With each increment of the flag, the failure mode
 * moves to a different location:
 *     0 - the JNI_OnLoad method will indicate an incompatible JNI_VERSION
 *     1 - the JEMCC_Init method will simulate a failure of the class creation
 *     2 - the init function will define "TestOne" and mark the package
 *         complete
 */
static int passFlag = 0;

/**
 * Standard method defined by the JNI specification to be called upon
 * the loading of a Java library extension.  May do some JEMCC initialization
 * but the conventional model is to use the JEMCC_***_Init methods.
 *
 * Parameters:
 *     jvm - the Java VM which loaded this library
 *     reserved - a Java specification reserved value
 */
jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    /* Fail on the first pass through */
    if (passFlag == 0) {
        passFlag++;
        return 0;
    }

    return JNI_VERSION_1_1;
}

/**
 * General (not package specific) JEMCC class initialization method.  Note
 * that this particular implementation does not set up the 'next' state flags
 * correctly, to test the fallback mode of the package initialization
 * handlers.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the package initialization is to
 *              occur, NULL if the VM bootstrap loader is to be used
 *     pkgName - the name of the package being initialized.  Allows for
 *               multi-package library initialization (ignored)
 *     initData - ignored data structure normally used to indicate subsequent
 *                initialization state
 */
jint JEMCC_Init(JNIEnv *env, JEMCC_Object *loader, 
                const char *pkgName, JEMCC_PkgInitData *initData) {
    /* Fail on the second pass too */
    if (passFlag == 1) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_LinkageError,
                                   NULL, "JEMCC_Init: forced JEMCC failure");
        passFlag++;
        return JNI_ERR;
    }

    /* Build a class, but not the one that is being looked for */
    return JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_ABSTRACT,
                                "pkg.test.TestOne", NULL, NULL, 0, 
                                NULL, 0, NULL, NULL, 0, NULL, 0, NULL, NULL);

    /* No state update, JEMCC handler should mark package complete */
}
