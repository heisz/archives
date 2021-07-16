/**
 * JEMCC definitions of the java.lang.reflect.Modifier class.
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

static jint JEMCC_Modifier_init(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    /* Instantiate if you want, it's all empty */
    return JEMCC_RET_VOID;
}

static jint JEMCC_Modifier_isAbstract_I(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_ABSTRACT) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isFinal_I(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_FINAL) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isInterface_I(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_INTERFACE) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isNative_I(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_NATIVE) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isPrivate_I(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_PRIVATE) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isProtected_I(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_PROTECTED) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isPublic_I(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_PUBLIC) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isStatic_I(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_STATIC) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isSynchronized_I(JNIEnv *env,
                                            JEMCC_VMFrame *frame,
                                            JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_SYNCHRONIZED) != 0) ? JNI_TRUE : 
                                                            JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isTransient_I(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_TRANSIENT) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_isVolatile_I(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);

    retVal->intVal = ((modifier & ACC_VOLATILE) != 0) ? JNI_TRUE : JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Modifier_toString_I(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    jint modifier = JEMCC_LOAD_INT(frame, 0);
    char *ptr;

    ptr = JEMCC_EnvStrBufferInit(env, -1);
    if (ptr == NULL) return JEMCC_ERR;

    /* Keep the following ordering correct! */
    /* Hmmm, Sun allows invalid combinations, sigh... */
    if ((modifier & ACC_PUBLIC) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " public");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_PRIVATE) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " private");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_PROTECTED) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " protected");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_ABSTRACT) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " abstract");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_STATIC) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " static");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_FINAL) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " final");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_TRANSIENT) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " transient");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_VOLATILE) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " volatile");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_NATIVE) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " native");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_SYNCHRONIZED) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " synchronized");
        if (ptr == NULL) return JEMCC_ERR;
    }
    if ((modifier & ACC_INTERFACE) != 0) {
        ptr = JEMCC_EnvStrBufferAppend(env, " interface");
        if (ptr == NULL) return JEMCC_ERR;
    }

    if (*ptr == ' ') ptr++;
    retVal->objVal = (JEMCC_Object *) JEMCC_NewStringUTF(env, ptr);
    if (retVal->objVal == NULL) return JEMCC_ERR;

    return JEMCC_RET_OBJECT;
}

JEMCC_MethodData JEMCC_ModifierMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_Modifier_init },
    { ACC_PUBLIC | ACC_STATIC,
         "isAbstract", "(I)Z",
         JEMCC_Modifier_isAbstract_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isFinal", "(I)Z",
         JEMCC_Modifier_isFinal_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isInterface", "(I)Z",
         JEMCC_Modifier_isInterface_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isNative", "(I)Z",
         JEMCC_Modifier_isNative_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isPrivate", "(I)Z",
         JEMCC_Modifier_isPrivate_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isProtected", "(I)Z",
         JEMCC_Modifier_isProtected_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isPublic", "(I)Z",
         JEMCC_Modifier_isPublic_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isStatic", "(I)Z",
         JEMCC_Modifier_isStatic_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isSynchronized", "(I)Z",
         JEMCC_Modifier_isSynchronized_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isTransient", "(I)Z",
         JEMCC_Modifier_isTransient_I },
    { ACC_PUBLIC | ACC_STATIC,
         "isVolatile", "(I)Z",
         JEMCC_Modifier_isVolatile_I },
    { ACC_PUBLIC | ACC_STATIC,
         "toString", "(I)Ljava/lang/String;",
         JEMCC_Modifier_toString_I }
};

JEMCC_FieldData JEMCC_ModifierFields[] = {
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "PUBLIC", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "PRIVATE", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "PROTECTED", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "STATIC", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "FINAL", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "SYNCHRONIZED", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "VOLATILE", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "TRANSIENT", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "NATIVE", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "INTERFACE", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "ABSTRACT", "I", -1 }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "java.lang.reflect.Modifier",
                     NULL ** java/lang/Object **,
                     interfaces, 0,
                     JEMCC_ModifierMethods, 13, NULL,
                     JEMCC_ModifierFields, 11,
                     NULL, 0, NULL, classInstance);
*/
