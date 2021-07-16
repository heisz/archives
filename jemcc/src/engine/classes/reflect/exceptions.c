/**
 * JEMCC definitions of the java.lang.reflect.InvocationTargetException class.
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
#include "jemcc.h"

/* Core initialization provided by throwable */
extern jint JEMCC_Throwable_init(JNIEnv *env, JEMCC_VMFrame *frame,
                                 JEMCC_ReturnValue *retVal);

static jint JEMCC_InvocationTargetException_init(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Add the local details */
    data->causeThrowable = NULL;

    return rc;
}

static jint JEMCC_InvocationTargetException_init_Throwable(JNIEnv *env,
                                                  JEMCC_VMFrame *frame,
                                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    JEMCC_Object *exObj = JEMCC_LOAD_OBJECT(frame, 1);
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Add the local details */
    data->causeThrowable = exObj;

    return rc;
}

static jint JEMCC_InvocationTargetException_init_ThrowableString(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    JEMCC_Object *exObj = JEMCC_LOAD_OBJECT(frame, 1);
    JEMCC_Object *msgObj = JEMCC_LOAD_OBJECT(frame, 2);
    jint rc;

    /* Initialize the core throwable information */
    rc = JEMCC_Throwable_init(env, frame, retVal);

    /* Add the local details */
    data->message = msgObj;
    data->causeThrowable = exObj;

    return rc;
}

static jint JEMCC_InvocationTargetException_getTargetException(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);

    retVal->objVal = data->causeThrowable;
    return JEMCC_RET_OBJECT;
}

JEMCC_MethodData JEMCC_InvocationTargetExceptionMethods[] = {
    { ACC_PROTECTED,
         "<init>", "()V",
         JEMCC_InvocationTargetException_init },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/Throwable;)V",
         JEMCC_InvocationTargetException_init_Throwable },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/Throwable;Ljava/lang/String;)V",
         JEMCC_InvocationTargetException_init_ThrowableString },
    { ACC_PUBLIC,
         "getTargetException", "()Ljava/lang/Throwable;",
         JEMCC_InvocationTargetException_getTargetException }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "java.lang.reflect.InvocationTargetException",
                     NULL ** java/lang/Exception **,
                     interfaces, 0,
                     JEMCC_InvocationTargetExceptionMethods, 4, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
