/**
 * JEMCC definitions of the wrdg.mauve.VMTestInterface class.
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

/* Read the structure/method details */
#include "jemcc.h"

static jint JEMCC_VMTestInterface_init(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "Class is not instantiable");
    return JEMCC_ERR;
}

static jint JEMCC_VMTestInterface_die_String(JNIEnv *env,
                                             JEMCC_VMFrame *frame,
                                             JEMCC_ReturnValue *retVal) {
    JEMCC_Object *string = JEMCC_LOAD_OBJECT(frame, 0);

    (void) fprintf(stderr, ">>>> FATAL ERROR: ");
    JEMCC_DumpString(env, string, JNI_TRUE);
    (void) fprintf(stderr, "\n");

    exit(1);
    return JEMCC_ERR;
}

static jint JEMCC_VMTestInterface_print_StringZZ(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_Object *string = JEMCC_LOAD_OBJECT(frame, 0);
    jboolean isError = JEMCC_LOAD_INT(frame, 1);
    jboolean newline = JEMCC_LOAD_INT(frame, 2);

    (void) fprintf(stderr, ">>>> ");
    JEMCC_DumpString(env, string, isError);
    if (newline == JNI_TRUE) {
        if (isError == JNI_TRUE) (void) fprintf(stderr, "\n");
        else (void) fprintf(stdout, "\n");
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_VMTestInterface_scanTests_String(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *str = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) str->objectData;
    JEMCC_Object *retArray;
    char *testArray[4096];
    int testCount = 0;

fprintf(stderr, "TARGET IS %s\n", (char *) &(strData->data));
    if (JEMCC_FileIsDirectory((char *) &(strData->data)) <= 0) {
        retArray =
             (JEMCC_Object *) JEMCC_NewObjectArray(env, 1,
                                 JEMCC_GetCoreVMClass(env, JEMCC_Class_String),
                                 NULL);
        if (retArray == NULL) return JEMCC_ERR;
        JEMCC_SetObjectArrayElement(env, retArray, 0, str);
    } else {
        retArray = NULL;
    }

    retVal->objVal = retArray;
    return JEMCC_RET_ARRAY;
}

JEMCC_MethodData JEMCC_VMTestInterfaceMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_VMTestInterface_init },
    { ACC_PUBLIC | ACC_STATIC,
         "die", "(Ljava/lang/String;)V",
         JEMCC_VMTestInterface_die_String },
    { ACC_PUBLIC | ACC_STATIC,
         "print", "(Ljava/lang/String;ZZ)V",
         JEMCC_VMTestInterface_print_StringZZ },
    { ACC_PUBLIC | ACC_STATIC,
         "scanTests", "(Ljava/lang/String;)[Ljava/lang/String;",
         JEMCC_VMTestInterface_scanTests_String }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "wrdg.mauve.VMTestInterface",
                     NULL ** java/lang/Object **,
                     interfaces, 0,
                     JEMCC_VMTestInterfaceMethods, 4, NULL
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
