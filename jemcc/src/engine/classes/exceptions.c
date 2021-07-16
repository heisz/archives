/**
 * JEMCC definitions of the non-standard java.lang.*Exception-*Error classes.
 * Copyright (C) 1999-2004 J.M. Heisz
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU Lesser General Public
 * License, as well as further clarification on your rights to use this
 * software.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include "jeminc.h"

/* Read the structure/method details */
#include "jem.h"
#include "jnifunc.h"

/* Core initialization provided by throwable */
extern jint JEMCC_Throwable_init(JNIEnv *env, JEMCC_VMFrame *frame,
                                 JEMCC_ReturnValue *retVal);

static jint JEMCC_ArrayIndexOutOfBoundsException_init(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Nothing more to do, message null'ed by initialization */

    return rc;
}

static jint JEMCC_ArrayIndexOutOfBoundsException_init_I(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    jint rc, index = JEMCC_LOAD_INT(frame, 1);
    char msg[128];

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Standard message */
    if (rc == JEMCC_RET_VOID) {
        (void) sprintf(msg, "index (%i) out of bounds", index);
        data->message = JEMCC_NewStringUTF(env, msg);
        if (data->message == NULL) return JEMCC_ERR;
    }

    return rc;
}

static jint JEMCC_ArrayIndexOutOfBoundsException_init_String(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    JEMCC_Object *msgObj = JEMCC_LOAD_OBJECT(frame, 1);
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Store the message locally */
    data->message = msgObj;

    return rc;
}

JEMCC_MethodData JEMCC_ArrayIndexOutOfBoundsExceptionMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_ArrayIndexOutOfBoundsException_init },
    { ACC_PUBLIC,
         "<init>", "(I)V",
         JEMCC_ArrayIndexOutOfBoundsException_init_I },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/String;)V",
         JEMCC_ArrayIndexOutOfBoundsException_init_String }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "java.lang.ArrayIndexOutOfBoundsException",
                     NULL ** java/lang/IndexOutOfBoundsException **,
                     interfaces, 0,
                     JEMCC_ArrayIndexOutOfBoundsExceptionMethods, 3, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/

static jint JEMCC_StringIndexOutOfBoundsException_init(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Nothing more to do, message null'ed by initialization */

    return rc;
}

static jint JEMCC_StringIndexOutOfBoundsException_init_I(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    jint rc, index = JEMCC_LOAD_INT(frame, 1);
    char msg[128];

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Standard message */
    if (rc == JEMCC_RET_VOID) {
        (void) sprintf(msg, "index (%i) out of bounds", index);
        data->message = JEMCC_NewStringUTF(env, msg);
        if (data->message == NULL) return JEMCC_ERR;
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_StringIndexOutOfBoundsException_init_String(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    JEMCC_Object *msgObj = JEMCC_LOAD_OBJECT(frame, 1);
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Store the message locally */
    data->message = msgObj;

    return rc;
}

JEMCC_MethodData JEMCC_StringIndexOutOfBoundsExceptionMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_StringIndexOutOfBoundsException_init },
    { ACC_PUBLIC,
         "<init>", "(I)V",
         JEMCC_StringIndexOutOfBoundsException_init_I },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/String;)V",
         JEMCC_StringIndexOutOfBoundsException_init_String }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "java.lang.StringIndexOutOfBoundsException",
                     NULL ** java/lang/IndexOutOfBoundsException **,
                     interfaces, 0,
                     JEMCC_StringIndexOutOfBoundsExceptionMethods, 3, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/

static jint JEMCC_ExceptionInInitializerError_init(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Nothing more to do, message null'ed by initialization */

    return rc;
}

static jint JEMCC_ExceptionInInitializerError_init_String(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    JEMCC_Object *msgObj = JEMCC_LOAD_OBJECT(frame, 1);
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Store the message locally */
    data->message = msgObj;

    return rc;
}

static jint JEMCC_ExceptionInInitializerError_init_Throwable(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    JEMCC_ObjectExt *cause = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    JEMCC_ThrowableData *cData = (JEMCC_ThrowableData *) &(cause->objectData);
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Add the local details */
    data->message = cData->message;
    data->causeThrowable = (JEMCC_Object *) cause;

    return rc;
}

static jint JEMCC_ExceptionInInitializerError_getException(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);

    retVal->objVal = data->causeThrowable;
    return JEMCC_RET_OBJECT;
}

JEMCC_MethodData JEMCC_ExceptionInInitializerErrorMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_ExceptionInInitializerError_init },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/String;)V",
         JEMCC_ExceptionInInitializerError_init_String },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/Throwable;)V",
         JEMCC_ExceptionInInitializerError_init_Throwable },
    { ACC_PUBLIC,
         "getException", "()Ljava/lang/Throwable;",
         JEMCC_ExceptionInInitializerError_getException }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "java.lang.ExceptionInInitializerError",
                     NULL ** java/lang/LinkageError **,
                     interfaces, 0,
                     JEMCC_ExceptionInInitializerErrorMethods, 4, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
