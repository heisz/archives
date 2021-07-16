/**
 * JEMCC methods to support the JNI object interfaces.
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

jobject JEMCC_NewGlobalRef(JNIEnv *env, jobject obj) {
    return NULL; /* TODO */
}

void JEMCC_DeleteGlobalRef(JNIEnv *env, jobject globalRef) {
    /* TODO */
}

void JEMCC_DeleteLocalRef(JNIEnv *env, jobject localRef) {
    /* TODO */
}

jboolean JEMCC_IsSameObject(JNIEnv *env, jobject obja, jobject objb) {
    return JNI_FALSE; /* TODO */
}

jobject JEMCC_AllocObject(JNIEnv *env, jclass clazz) {
    return NULL; /* TODO */
}

jobject JEMCC_NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, 
                       va_list args) {
    return NULL; /* TODO */
}

jobject JEMCC_NewObjectA(JNIEnv *env, jclass clazz, jmethodID methodID, 
                       jvalue *args) {
    return NULL; /* TODO */
}

jclass JEMCC_GetObjectClass(JNIEnv *env, jobject obj) {
    JEMCC_Object *jobj = (JEMCC_Object *) obj;

    return (jclass) jobj->classReference;
}

jboolean JEMCC_IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
    jclass objclazz = ((JEMCC_Object *) obj)->classReference;

    return JEMCC_IsAssignableFrom(env, objclazz, clazz);
}
