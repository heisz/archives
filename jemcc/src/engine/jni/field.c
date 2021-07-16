/**
 * JEMCC methods to support the JNI field interfaces.
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

/* Read the VM structure/method definitions */
#include "jem.h"
#include "jnifunc.h"

jfieldID JEMCC_GetFieldID(JNIEnv *env, jclass clazz, const char *name, 
                          const char *sig) {
    return NULL; /* TODO */
}

jobject JEMCC_GetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return (jobject) *((JEMCC_Object **) basePtr);
}

jboolean JEMCC_GetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return (jboolean) *((jbyte *) basePtr);
}

jbyte JEMCC_GetByteField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return *((jbyte *) basePtr);
}

jchar JEMCC_GetCharField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return *((jchar *) basePtr);
}

jshort JEMCC_GetShortField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return *((jshort *) basePtr);
}

jint JEMCC_GetIntField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return *((jint *) basePtr);
}

jlong JEMCC_GetLongField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return *((jlong *) basePtr);
}

jfloat JEMCC_GetFloatField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return *((jfloat *) basePtr);
}

jdouble JEMCC_GetDoubleField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    return *((jdouble *) basePtr);
}

void JEMCC_SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                          jobject val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    JEMCC_MarkNonLocalObject(env, (JEMCC_Object *) val);
    *((JEMCC_Object **) basePtr) = (JEMCC_Object *) val;
}

void JEMCC_SetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                           jboolean val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jbyte *) basePtr) = (jbyte) val;
}

void JEMCC_SetByteField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                        jbyte val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jbyte *) basePtr) = val;
}

void JEMCC_SetCharField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                        jchar val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jchar *) basePtr) = val;
}

void JEMCC_SetShortField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                         jshort val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jshort *) basePtr) = val;
}

void JEMCC_SetIntField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                       jint val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jint *) basePtr) = val;
}

void JEMCC_SetLongField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                        jlong val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jlong *) basePtr) = val;
}

void JEMCC_SetFloatField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                         jfloat val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jfloat *) basePtr) = val;
}

void JEMCC_SetDoubleField(JNIEnv *env, jobject obj, jfieldID fieldID, 
                          jdouble val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *basePtr = ((jbyte *) &(((JEMCC_ObjectExt *) obj)->objectData)) +
                                                         fieldRef->fieldOffset;

    *((jdouble *) basePtr) = val;
}

jfieldID JEMCC_GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, 
                                const char *sig) {
    return NULL; /* TODO */
}

jobject JEMCC_GetStaticObjectField(JNIEnv *env, jclass clazz, 
                                   jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return (jobject) *((JEMCC_Object **) (staticData + fieldRef->fieldOffset));
}

jboolean JEMCC_GetStaticBooleanField(JNIEnv *env, jclass clazz, 
                                     jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return (jboolean) *((jbyte *) (staticData + fieldRef->fieldOffset));
}

jbyte JEMCC_GetStaticByteField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return *((jbyte *) (staticData + fieldRef->fieldOffset));
}

jchar JEMCC_GetStaticCharField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return *((jchar *) (staticData + fieldRef->fieldOffset));
}

jshort JEMCC_GetStaticShortField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return *((jshort *) (staticData + fieldRef->fieldOffset));
}

jint JEMCC_GetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return *((jint *) (staticData + fieldRef->fieldOffset));
}

jlong JEMCC_GetStaticLongField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return *((jlong *) (staticData + fieldRef->fieldOffset));
}

jfloat JEMCC_GetStaticFloatField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return *((jfloat *) (staticData + fieldRef->fieldOffset));
}

jdouble JEMCC_GetStaticDoubleField(JNIEnv *env, jclass clazz, 
                                   jfieldID fieldID) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    return *((jdouble *) (staticData + fieldRef->fieldOffset));
}

void JEMCC_SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                                jobject val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((JEMCC_Object **) (staticData + fieldRef->fieldOffset)) =
                                                     (JEMCC_Object *) val;
}

void JEMCC_SetStaticBooleanField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                                 jboolean val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jbyte *) (staticData + fieldRef->fieldOffset)) = (jbyte) val;

}

void JEMCC_SetStaticByteField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                              jbyte val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jbyte *) (staticData + fieldRef->fieldOffset)) = val;
}

void JEMCC_SetStaticCharField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                              jchar val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jchar *) (staticData + fieldRef->fieldOffset)) = val;
}

void JEMCC_SetStaticShortField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                               jshort val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jshort *) (staticData + fieldRef->fieldOffset)) = val;
}

void JEMCC_SetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                             jint val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jint *) (staticData + fieldRef->fieldOffset)) = val;
}

void JEMCC_SetStaticLongField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                              jlong val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jlong *) (staticData + fieldRef->fieldOffset)) = val;
}

void JEMCC_SetStaticFloatField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                               jfloat val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jfloat *) (staticData + fieldRef->fieldOffset)) = val;
}

void JEMCC_SetStaticDoubleField(JNIEnv *env, jclass clazz, jfieldID fieldID, 
                                jdouble val) {
    JEM_ClassFieldData *fieldRef = (JEM_ClassFieldData *) fieldID;
    jbyte *staticData = (jbyte *) ((JEMCC_Class *) clazz)->staticData;

    (void) JEMCC_InitializeClass(env, (JEMCC_Class *) clazz);
    *((jdouble *) (staticData + fieldRef->fieldOffset)) = val;
}
