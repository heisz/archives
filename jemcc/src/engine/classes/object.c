/**
 * JEMCC definitions of the java.lang.Object class.
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

/* Internal class, needs to access JEMCC internal details */
#include "jem.h"
#include "jnifunc.h"

static jint JEMCC_Object_init(JNIEnv *env,
                              JEMCC_VMFrame *frame,
                              JEMCC_ReturnValue *retVal) {
    /* Nothing to do here, hash is driven by address */
    return JEMCC_RET_VOID;
}

static jint JEMCC_Object_clone(JNIEnv *env,
                               JEMCC_VMFrame *frame,
                               JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_Class *cloneable = JEMCC_GetCoreVMClass(env, JEMCC_Class_Cloneable);

    /* Only if someone has indicated it is permitted */
    if (JEMCC_IsAssignableFrom(env, thisObj->classReference, 
                                              cloneable) == JNI_FALSE) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_CloneNotSupportedException,
                                   NULL, "Object is not cloneable");
        return JEMCC_ERR;
    }

    /* Perform a shallow clone */
    retVal->objVal = JEMCC_CloneObject(env, thisObj);
    if (retVal->objVal == NULL) return JEMCC_ERR;

    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Object_equals_Object(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_Object *compObj = JEMCC_LOAD_OBJECT(frame, 1);

    retVal->intVal = (thisObj == compObj) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Object_finalize(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    /* Object only defines contract - this method does nothing */
    return JEMCC_RET_VOID;
}

static jint JEMCC_Object_getClass(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);

    retVal->objVal = (JEMCC_Object *) thisObj->classReference;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Object_hashCode(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);

    retVal->intVal = (jint) thisObj;
    return JEMCC_RET_INT;
}

static jint JEMCC_Object_notify(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);

    if (JEMCC_ObjMonitorNotify(env, thisObj) != JNI_OK) return JEMCC_ERR;
    return JEMCC_RET_VOID;
}

static jint JEMCC_Object_notifyAll(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);

    if (JEMCC_ObjMonitorNotifyAll(env, thisObj) != JNI_OK) return JEMCC_ERR;
    return JEMCC_RET_VOID;
}

static char hexCodes[16] = { '0', '1', '2', '3', '4', '5', '6', '7', 
                             '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

static jint JEMCC_Object_toString(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);
    void *classData = thisObj->classReference->classData;
    char *className = ((JEM_ClassData *) classData)->className;
    JEMCC_ReturnValue hashVal;
    char *ptr, hexCode[16];

    /* Call the hashCode method */
    JEMCC_PUSH_STACK_OBJECT(frame, thisObj);
    JEMCC_CHECK_METHOD_REFERENCE(env, NULL, 5, "hashCode", "()I");
    if (JEMCC_ExecuteInstanceMethod(env, thisObj, NULL, 5,
                                    JEMCC_VIRTUAL_METHOD, &hashVal) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Convert it to hex (basic copy of the Integer.toHexString method) */
    ptr = hexCode + 16;
    *(--ptr) = '\0';
    if (hashVal.intVal == 0) {
        *(--ptr) = '0';
    } else {
        while (hashVal.intVal != 0) {
            *(--ptr) = hexCodes[hashVal.intVal & 0x0F];
            hashVal.intVal >>= 4;
        }
    }

    /* Glue it together and send it on home */
    retVal->objVal = JEMCC_StringCatUTF(env, className, "@", 
                                        ptr, (char *) NULL);
    return (retVal->objVal == NULL) ? JEMCC_ERR : JEMCC_RET_OBJECT;
}

static jint JEMCC_Object_wait(JNIEnv *env,
                              JEMCC_VMFrame *frame,
                              JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);

    if (JEMCC_ObjMonitorWait(env, thisObj) != JNI_OK) return JEMCC_ERR;
    return JEMCC_RET_VOID;
}

static jint JEMCC_Object_wait_J(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);
    jlong timeout = JEMCC_LOAD_LONG(frame, 1);

    if (JEMCC_ObjMonitorMilliWait(env, thisObj, 
                                  timeout) != JNI_OK) return JEMCC_ERR;
    return JEMCC_RET_VOID;
}

static jint JEMCC_Object_wait_JI(JNIEnv *env,
                                 JEMCC_VMFrame *frame,
                                 JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);
    jlong timeout = JEMCC_LOAD_LONG(frame, 1);
    jint nanos = JEMCC_LOAD_LONG(frame, 3);

    if (JEMCC_ObjMonitorNanoWait(env, thisObj, 
                                 timeout, nanos) != JNI_OK) return JEMCC_ERR;
    return JEMCC_RET_VOID;
}

JEMCC_MethodData JEMCC_ObjectMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_Object_init },
    { ACC_PROTECTED,
         "clone", "()Ljava/lang/Object;",
         JEMCC_Object_clone },
    { ACC_PUBLIC,
         "equals", "(Ljava/lang/Object;)Z",
         JEMCC_Object_equals_Object },
    { ACC_PROTECTED,
         "finalize", "()V",
         JEMCC_Object_finalize },
    { ACC_PUBLIC | ACC_FINAL,
         "getClass", "()Ljava/lang/Class;",
         JEMCC_Object_getClass },
    { ACC_PUBLIC,
         "hashCode", "()I",
         JEMCC_Object_hashCode },
    { ACC_PUBLIC | ACC_FINAL,
         "notify", "()V",
         JEMCC_Object_notify },
    { ACC_PUBLIC | ACC_FINAL,
         "notifyAll", "()V",
         JEMCC_Object_notifyAll },
    { ACC_PUBLIC,
         "toString", "()Ljava/lang/String;",
         JEMCC_Object_toString },
    { ACC_PUBLIC | ACC_FINAL,
         "wait", "()V",
         JEMCC_Object_wait },
    { ACC_PUBLIC | ACC_FINAL,
         "wait", "(J)V",
         JEMCC_Object_wait_J },
    { ACC_PUBLIC | ACC_FINAL,
         "wait", "(JI)V",
         JEMCC_Object_wait_JI }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "java.lang.Object",
                     NULL ** NULL **,
                     interfaces, 0,
                     JEMCC_ObjectMethods, 12, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
