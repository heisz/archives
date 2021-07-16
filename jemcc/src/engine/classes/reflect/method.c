/**
 * JEMCC definitions of the java.lang.reflect.Method class.
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

static jint JEMCC_Method_equals_Object(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Method_getDeclaringClass(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassMethodData *method = (JEM_ClassMethodData *) thisObj->objectData;

    retVal->objVal = (JEMCC_Object *) method->parentClass;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Method_getExceptionTypes(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Method_getModifiers(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassMethodData *method = (JEM_ClassMethodData *) thisObj->objectData;

    retVal->intVal = method->accessFlags & 0x0FFF;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Method_getName(JNIEnv *env,
                                 JEMCC_VMFrame *frame,
                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassMethodData *method = (JEM_ClassMethodData *) thisObj->objectData;

    retVal->objVal = JEMCC_NewStringUTF(env, method->name);
    if (retVal->objVal == NULL) return JEMCC_ERR;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Method_getParameterTypes(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Method_getReturnType(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Method_hashCode(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Method_invoke_ObjectObjectArray(JNIEnv *env,
                                                  JEMCC_VMFrame *frame,
                                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Method_toString(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

JEMCC_MethodData JEMCC_MethodMethods[] = {
    { ACC_PUBLIC,
         "equals", "(Ljava/lang/Object;)Z",
         JEMCC_Method_equals_Object },
    { ACC_PUBLIC,
         "getDeclaringClass", "()Ljava/lang/Class;",
         JEMCC_Method_getDeclaringClass },
    { ACC_PUBLIC,
         "getExceptionTypes", "()[Ljava/lang/Class;",
         JEMCC_Method_getExceptionTypes },
    { ACC_PUBLIC,
         "getModifiers", "()I",
         JEMCC_Method_getModifiers },
    { ACC_PUBLIC,
         "getName", "()Ljava/lang/String;",
         JEMCC_Method_getName },
    { ACC_PUBLIC,
         "getParameterTypes", "()[Ljava/lang/Class;",
         JEMCC_Method_getParameterTypes },
    { ACC_PUBLIC,
         "getReturnType", "()Ljava/lang/Class;",
         JEMCC_Method_getReturnType },
    { ACC_PUBLIC,
         "hashCode", "()I",
         JEMCC_Method_hashCode },
    { ACC_PUBLIC,
         "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;",
         JEMCC_Method_invoke_ObjectObjectArray },
    { ACC_PUBLIC,
         "toString", "()Ljava/lang/String;",
         JEMCC_Method_toString }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC | ACC_FINAL,
                     "java.lang.reflect.Method",
                     NULL ** java/lang/Object **,
                     interfaces ** java/lang/reflect/Member,  **, 1,
                     JEMCC_MethodMethods, 10, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
