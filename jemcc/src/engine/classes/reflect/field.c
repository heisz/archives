/**
 * JEMCC definitions of the java.lang.reflect.Field class.
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

static jint Field_checkReadAccess(JNIEnv *env, JEM_ClassFieldData *data,
                                  JEMCC_Object *targObj) {
    if ((data->accessFlags & ACC_STATIC) == 0) {
        if (targObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException,
                                       NULL, NULL);
            return JNI_ERR;
        }
    }

    if (JEMCC_IsAssignableFrom(env, targObj->classReference,
                               data->parentClass) == JNI_FALSE) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IllegalArgumentException,
                                   NULL, "Object not instance of field class");
        return JNI_ERR;
    }

    /* Need access control check */

    return JNI_OK;
}

static jint Field_checkWriteAccess(JNIEnv *env, JEM_ClassFieldData *data,
                                   JEMCC_Object *targObj) {
    if ((data->accessFlags & ACC_STATIC) == 0) {
        if (targObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException,
                                       NULL, NULL);
            return JNI_ERR;
        }
    }

    if (JEMCC_IsAssignableFrom(env, targObj->classReference,
                               data->parentClass) == JNI_FALSE) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IllegalArgumentException,
                                   NULL, "Object not instance of field class");
        return JNI_ERR;
    }

    /* Need access control check */

    if ((data->accessFlags & ACC_FINAL) != 0) {
        if (targObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IllegalAccessException,
                                    NULL, "Cannot set a final field instance");
            return JNI_ERR;
        }
    }

    return JNI_OK;
}

static jint JEMCC_Field_equals_Object(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_get_Object(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getBoolean_Object(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getByte_Object(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getChar_Object(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getDeclaringClass(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;

    retVal->objVal = (JEMCC_Object *) field->parentClass;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Field_getDouble_Object(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getFloat_Object(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getInt_Object(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getLong_Object(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getModifiers(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;

    retVal->intVal = field->accessFlags & 0x0FFF;
    return JEMCC_RET_INT;
}

static jint JEMCC_Field_getName(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;

    retVal->objVal = JEMCC_NewStringUTF(env, field->name);
    if (retVal->objVal == NULL) return JEMCC_ERR;
    return JEMCC_RET_INT;
}

static jint JEMCC_Field_getShort_Object(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkReadAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_getType(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_hashCode(JNIEnv *env,
                                 JEMCC_VMFrame *frame,
                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_set_ObjectObject(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setBoolean_ObjectZ(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setByte_ObjectB(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setChar_ObjectC(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setDouble_ObjectD(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setFloat_ObjectF(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setInt_ObjectI(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setLong_ObjectJ(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_setShort_ObjectS(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassFieldData *field = (JEM_ClassFieldData *) thisObj->objectData;
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    if (Field_checkWriteAccess(env, field, targObj) != JNI_OK) return JEMCC_ERR;

    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Field_toString(JNIEnv *env,
                                 JEMCC_VMFrame *frame,
                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

JEMCC_MethodData JEMCC_FieldMethods[] = {
    { ACC_PUBLIC,
         "equals", "(Ljava/lang/Object;)Z",
         JEMCC_Field_equals_Object },
    { ACC_PUBLIC,
         "get", "(Ljava/lang/Object;)Ljava/lang/Object;",
         JEMCC_Field_get_Object },
    { ACC_PUBLIC,
         "getBoolean", "(Ljava/lang/Object;)Z",
         JEMCC_Field_getBoolean_Object },
    { ACC_PUBLIC,
         "getByte", "(Ljava/lang/Object;)B",
         JEMCC_Field_getByte_Object },
    { ACC_PUBLIC,
         "getChar", "(Ljava/lang/Object;)C",
         JEMCC_Field_getChar_Object },
    { ACC_PUBLIC,
         "getDeclaringClass", "()Ljava/lang/Class;",
         JEMCC_Field_getDeclaringClass },
    { ACC_PUBLIC,
         "getDouble", "(Ljava/lang/Object;)D",
         JEMCC_Field_getDouble_Object },
    { ACC_PUBLIC,
         "getFloat", "(Ljava/lang/Object;)F",
         JEMCC_Field_getFloat_Object },
    { ACC_PUBLIC,
         "getInt", "(Ljava/lang/Object;)I",
         JEMCC_Field_getInt_Object },
    { ACC_PUBLIC,
         "getLong", "(Ljava/lang/Object;)J",
         JEMCC_Field_getLong_Object },
    { ACC_PUBLIC,
         "getModifiers", "()I",
         JEMCC_Field_getModifiers },
    { ACC_PUBLIC,
         "getName", "()Ljava/lang/String;",
         JEMCC_Field_getName },
    { ACC_PUBLIC,
         "getShort", "(Ljava/lang/Object;)S",
         JEMCC_Field_getShort_Object },
    { ACC_PUBLIC,
         "getType", "()Ljava/lang/Class;",
         JEMCC_Field_getType },
    { ACC_PUBLIC,
         "hashCode", "()I",
         JEMCC_Field_hashCode },
    { ACC_PUBLIC,
         "set", "(Ljava/lang/Object;Ljava/lang/Object;)V",
         JEMCC_Field_set_ObjectObject },
    { ACC_PUBLIC,
         "setBoolean", "(Ljava/lang/Object;Z)V",
         JEMCC_Field_setBoolean_ObjectZ },
    { ACC_PUBLIC,
         "setByte", "(Ljava/lang/Object;B)V",
         JEMCC_Field_setByte_ObjectB },
    { ACC_PUBLIC,
         "setChar", "(Ljava/lang/Object;C)V",
         JEMCC_Field_setChar_ObjectC },
    { ACC_PUBLIC,
         "setDouble", "(Ljava/lang/Object;D)V",
         JEMCC_Field_setDouble_ObjectD },
    { ACC_PUBLIC,
         "setFloat", "(Ljava/lang/Object;F)V",
         JEMCC_Field_setFloat_ObjectF },
    { ACC_PUBLIC,
         "setInt", "(Ljava/lang/Object;I)V",
         JEMCC_Field_setInt_ObjectI },
    { ACC_PUBLIC,
         "setLong", "(Ljava/lang/Object;J)V",
         JEMCC_Field_setLong_ObjectJ },
    { ACC_PUBLIC,
         "setShort", "(Ljava/lang/Object;S)V",
         JEMCC_Field_setShort_ObjectS },
    { ACC_PUBLIC,
         "toString", "()Ljava/lang/String;",
         JEMCC_Field_toString }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC | ACC_FINAL,
                     "java.lang.reflect.Field",
                     NULL ** java/lang/Object **,
                     interfaces ** java/lang/reflect/Member,  **, 1,
                     JEMCC_FieldMethods, 25, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
