/**
 * Functional prototypes for the JEMCC Java Native Interface functions.
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

/*
 * NOTE: this file serves two purposes.  First it allows the implementations
 * of the native interface functions in the engine source code to be split
 * apart.  Second, it allows the JEMCC native library implementations to
 * access these methods without the extra redirection of the JNIEnv structure.
 */

#ifndef JEM_JNIFUNC_H
#define JEM_JNIFUNC_H 1

/* Ensure inclusion of JNI definitions */
#include "jni.h"

/* <jemcc_start> */

/************************ Direct JNI Functions ***********************/
/* TODO - Move comment blocks from code implementations to this file */

JNIEXPORT jint JNICALL JEMCC_GetVersion(JNIEnv *env);
JNIEXPORT jclass JNICALL JEMCC_DefineClass(JNIEnv *env, jobject loader,
                                           const jbyte *buff, jsize buffLen);
JNIEXPORT jclass JNICALL JEMCC_FindClass(JNIEnv *env, const char *name);

JNIEXPORT jclass JNICALL JEMCC_GetSuperclass(JNIEnv *env, jclass clazz);
JNIEXPORT jboolean JNICALL JEMCC_IsAssignableFrom(JNIEnv *env,
                                                  jclass clazza, jclass clazzb);

JNIEXPORT jint JNICALL JEMCC_Throw(JNIEnv *env, jthrowable obj);
JNIEXPORT jint JNICALL JEMCC_ThrowNew(JNIEnv *env, jclass clazz,
                                      const char *msg);
JNIEXPORT jthrowable JNICALL JEMCC_ExceptionOccurred(JNIEnv *env);
JNIEXPORT void JNICALL JEMCC_ExceptionDescribe(JNIEnv *env);
JNIEXPORT void JNICALL JEMCC_ExceptionClear(JNIEnv *env);
JNIEXPORT void JNICALL JEMCC_FatalError(JNIEnv *env, const char *msg);

JNIEXPORT jobject JNICALL JEMCC_NewGlobalRef(JNIEnv *env, jobject obj);
JNIEXPORT void JNICALL JEMCC_DeleteGlobalRef(JNIEnv *env, jobject globalRef);
JNIEXPORT void JNICALL JEMCC_DeleteLocalRef(JNIEnv *env, jobject localRef);
JNIEXPORT jboolean JNICALL JEMCC_IsSameObject(JNIEnv *env,
                                              jobject obja, jobject objb);

JNIEXPORT jobject JNICALL JEMCC_AllocObject(JNIEnv *env, jclass clazz);
JNIEXPORT jobject JNICALL JEMCC_NewObject(JNIEnv *env, jclass class,
                                          jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_NewObjectV(JNIEnv *env, jclass class,
                                           jmethodID methodID, va_list args);
JNIEXPORT jobject JNICALL JEMCC_NewObjectA(JNIEnv *env, jclass class,
                                           jmethodID methodID, jvalue *args);

JNIEXPORT jclass JNICALL JEMCC_GetObjectClass(JNIEnv *env, jobject obj);
JNIEXPORT jboolean JNICALL JEMCC_IsInstanceOf(JNIEnv *env, jobject obj,
                                              jclass clazz);

JNIEXPORT jmethodID JNICALL JEMCC_GetMethodID(JNIEnv *env, jclass clazz,
                                              const char *name,
                                              const char *sig);

JNIEXPORT jobject JNICALL JEMCC_CallObjectMethod(JNIEnv *env, jobject obj,
                                                 jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_CallObjectMethodV(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  va_list args);
JNIEXPORT jobject JNICALL JEMCC_CallObjectMethodA(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  jvalue *args);

JNIEXPORT jboolean JNICALL JEMCC_CallBooleanMethod(JNIEnv *env, jobject obj,
                                                   jmethodID methodID, ...);
JNIEXPORT jboolean JNICALL JEMCC_CallBooleanMethodV(JNIEnv *env, jobject obj,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jboolean JNICALL JEMCC_CallBooleanMethodA(JNIEnv *env, jobject obj,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jbyte JNICALL JEMCC_CallByteMethod(JNIEnv *env, jobject obj,
                                             jmethodID methodID, ...);
JNIEXPORT jbyte JNICALL JEMCC_CallByteMethodV(JNIEnv *env, jobject obj,
                                              jmethodID methodID, va_list args);
JNIEXPORT jbyte JNICALL JEMCC_CallByteMethodA(JNIEnv *env, jobject obj,
                                              jmethodID methodID, jvalue *args);

JNIEXPORT jchar JNICALL JEMCC_CallCharMethod(JNIEnv *env, jobject obj,
                                             jmethodID methodID, ...);
JNIEXPORT jchar JNICALL JEMCC_CallCharMethodV(JNIEnv *env, jobject obj,
                                              jmethodID methodID, va_list args);
JNIEXPORT jchar JNICALL JEMCC_CallCharMethodA(JNIEnv *env, jobject obj,
                                              jmethodID methodID, jvalue *args);

JNIEXPORT jshort JNICALL JEMCC_CallShortMethod(JNIEnv *env, jobject obj,
                                               jmethodID methodID, ...);
JNIEXPORT jshort JNICALL JEMCC_CallShortMethodV(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jshort JNICALL JEMCC_CallShortMethodA(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jint JNICALL JEMCC_CallIntMethod(JNIEnv *env, jobject obj,
                                           jmethodID methodID, ...);
JNIEXPORT jint JNICALL JEMCC_CallIntMethodV(JNIEnv *env, jobject obj,
                                            jmethodID methodID, va_list args);
JNIEXPORT jint JNICALL JEMCC_CallIntMethodA(JNIEnv *env, jobject obj,
                                            jmethodID methodID, jvalue *args);

JNIEXPORT jlong JNICALL JEMCC_CallLongMethod(JNIEnv *env, jobject obj,
                                             jmethodID methodID, ...);
JNIEXPORT jlong JNICALL JEMCC_CallLongMethodV(JNIEnv *env, jobject obj,
                                              jmethodID methodID, va_list args);
JNIEXPORT jlong JNICALL JEMCC_CallLongMethodA(JNIEnv *env, jobject obj,
                                              jmethodID methodID, jvalue *args);

JNIEXPORT jfloat JNICALL JEMCC_CallFloatMethod(JNIEnv *env, jobject obj,
                                               jmethodID methodID, ...);
JNIEXPORT jfloat JNICALL JEMCC_CallFloatMethodV(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jfloat JNICALL JEMCC_CallFloatMethodA(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jdouble JNICALL JEMCC_CallDoubleMethod(JNIEnv *env, jobject obj,
                                                 jmethodID methodID, ...);
JNIEXPORT jdouble JNICALL JEMCC_CallDoubleMethodV(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  va_list args);
JNIEXPORT jdouble JNICALL JEMCC_CallDoubleMethodA(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  jvalue *args);

JNIEXPORT void JNICALL JEMCC_CallVoidMethod(JNIEnv *env, jobject obj,
                                            jmethodID methodID, ...);
JNIEXPORT void JNICALL JEMCC_CallVoidMethodV(JNIEnv *env, jobject obj,
                                             jmethodID methodID, va_list args);
JNIEXPORT void JNICALL JEMCC_CallVoidMethodA(JNIEnv *env, jobject obj,
                                             jmethodID methodID, jvalue *args);


JNIEXPORT jobject JNICALL JEMCC_CallNonvirtualObjectMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_CallNonvirtualObjectMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jobject JNICALL JEMCC_CallNonvirtualObjectMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jboolean JNICALL JEMCC_CallNonvirtualBooleanMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jboolean JNICALL JEMCC_CallNonvirtualBooleanMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jboolean JNICALL JEMCC_CallNonvirtualBooleanMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jbyte JNICALL JEMCC_CallNonvirtualByteMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jbyte JNICALL JEMCC_CallNonvirtualByteMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jbyte JNICALL JEMCC_CallNonvirtualByteMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jchar JNICALL JEMCC_CallNonvirtualCharMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jchar JNICALL JEMCC_CallNonvirtualCharMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jchar JNICALL JEMCC_CallNonvirtualCharMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jshort JNICALL JEMCC_CallNonvirtualShortMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jshort JNICALL JEMCC_CallNonvirtualShortMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jshort JNICALL JEMCC_CallNonvirtualShortMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jint JNICALL JEMCC_CallNonvirtualIntMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jint JNICALL JEMCC_CallNonvirtualIntMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jint JNICALL JEMCC_CallNonvirtualIntMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jlong JNICALL JEMCC_CallNonvirtualLongMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jlong JNICALL JEMCC_CallNonvirtualLongMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jlong JNICALL JEMCC_CallNonvirtualLongMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jfloat JNICALL JEMCC_CallNonvirtualFloatMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jfloat JNICALL JEMCC_CallNonvirtualFloatMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jfloat JNICALL JEMCC_CallNonvirtualFloatMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jdouble JNICALL JEMCC_CallNonvirtualDoubleMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jdouble JNICALL JEMCC_CallNonvirtualDoubleMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jdouble JNICALL JEMCC_CallNonvirtualDoubleMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT void JNICALL JEMCC_CallNonvirtualVoidMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT void JNICALL JEMCC_CallNonvirtualVoidMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT void JNICALL JEMCC_CallNonvirtualVoidMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);


JNIEXPORT jfieldID JNICALL JEMCC_GetFieldID(JNIEnv *env, jclass clazz,
                                            const char *name, const char *sig);

JNIEXPORT jobject JNICALL JEMCC_GetObjectField(JNIEnv *env, jobject obj,
                                               jfieldID fieldID);
JNIEXPORT jboolean JNICALL JEMCC_GetBooleanField(JNIEnv *env, jobject obj,
                                                 jfieldID fieldID);
JNIEXPORT jbyte JNICALL JEMCC_GetByteField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID);
JNIEXPORT jchar JNICALL JEMCC_GetCharField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID);
JNIEXPORT jshort JNICALL JEMCC_GetShortField(JNIEnv *env, jobject obj,
                                             jfieldID fieldID);
JNIEXPORT jint JNICALL JEMCC_GetIntField(JNIEnv *env, jobject obj,
                                         jfieldID fieldID);
JNIEXPORT jlong JNICALL JEMCC_GetLongField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID);
JNIEXPORT jfloat JNICALL JEMCC_GetFloatField(JNIEnv *env, jobject obj,
                                             jfieldID fieldID);
JNIEXPORT jdouble JNICALL JEMCC_GetDoubleField(JNIEnv *env, jobject obj,
                                               jfieldID fieldID);

JNIEXPORT void JNICALL JEMCC_SetObjectField(JNIEnv *env, jobject obj,
                                            jfieldID fieldID, jobject val);
JNIEXPORT void JNICALL JEMCC_SetBooleanField(JNIEnv *env, jobject obj,
                                             jfieldID fieldID, jboolean val);
JNIEXPORT void JNICALL JEMCC_SetByteField(JNIEnv *env, jobject obj,
                                          jfieldID fieldID, jbyte val);
JNIEXPORT void JNICALL JEMCC_SetCharField(JNIEnv *env, jobject obj,
                                          jfieldID fieldID, jchar val);
JNIEXPORT void JNICALL JEMCC_SetShortField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID, jshort val);
JNIEXPORT void JNICALL JEMCC_SetIntField(JNIEnv *env, jobject obj,
                                         jfieldID fieldID, jint val);
JNIEXPORT void JNICALL JEMCC_SetLongField(JNIEnv *env, jobject obj,
                                          jfieldID fieldID, jlong val);
JNIEXPORT void JNICALL JEMCC_SetFloatField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID, jfloat val);
JNIEXPORT void JNICALL JEMCC_SetDoubleField(JNIEnv *env, jobject obj,
                                            jfieldID fieldID, jdouble val);


JNIEXPORT jmethodID JNICALL JEMCC_GetStaticMethodID(JNIEnv *env, jclass clazz,
                                                    const char *name,
                                                    const char *sig);

JNIEXPORT jobject JNICALL JEMCC_CallStaticObjectMethod(JNIEnv *env,
                                                       jclass clazz,
                                                       jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_CallStaticObjectMethodV(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        va_list args);
JNIEXPORT jobject JNICALL JEMCC_CallStaticObjectMethodA(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        jvalue *args);

JNIEXPORT jboolean JNICALL JEMCC_CallStaticBooleanMethod(JNIEnv *env,
                                                         jclass clazz,
                                                         jmethodID methodID,
                                                         ...);
JNIEXPORT jboolean JNICALL JEMCC_CallStaticBooleanMethodV(JNIEnv *env,
                                                          jclass clazz,
                                                          jmethodID methodID,
                                                          va_list args);
JNIEXPORT jboolean JNICALL JEMCC_CallStaticBooleanMethodA(JNIEnv *env,
                                                          jclass clazz,
                                                          jmethodID methodID,
                                                          jvalue *args);

JNIEXPORT jbyte JNICALL JEMCC_CallStaticByteMethod(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID, ...);
JNIEXPORT jbyte JNICALL JEMCC_CallStaticByteMethodV(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jbyte JNICALL JEMCC_CallStaticByteMethodA(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jchar JNICALL JEMCC_CallStaticCharMethod(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID, ...);
JNIEXPORT jchar JNICALL JEMCC_CallStaticCharMethodV(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jchar JNICALL JEMCC_CallStaticCharMethodA(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jshort JNICALL JEMCC_CallStaticShortMethod(JNIEnv *env, jclass clazz,
                                                     jmethodID methodID, ...);
JNIEXPORT jshort JNICALL JEMCC_CallStaticShortMethodV(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      va_list args);
JNIEXPORT jshort JNICALL JEMCC_CallStaticShortMethodA(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      jvalue *args);

JNIEXPORT jint JNICALL JEMCC_CallStaticIntMethod(JNIEnv *env, jclass clazz,
                                                 jmethodID methodID, ...);
JNIEXPORT jint JNICALL JEMCC_CallStaticIntMethodV(JNIEnv *env, jclass clazz,
                                                  jmethodID methodID,
                                                  va_list args);
JNIEXPORT jint JNICALL JEMCC_CallStaticIntMethodA(JNIEnv *env, jclass clazz,
                                                  jmethodID methodID,
                                                  jvalue *args);

JNIEXPORT jlong JNICALL JEMCC_CallStaticLongMethod(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID, ...);
JNIEXPORT jlong JNICALL JEMCC_CallStaticLongMethodV(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jlong JNICALL JEMCC_CallStaticLongMethodA(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jfloat JNICALL JEMCC_CallStaticFloatMethod(JNIEnv *env, jclass clazz,
                                                     jmethodID methodID, ...);
JNIEXPORT jfloat JNICALL JEMCC_CallStaticFloatMethodV(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      va_list args);
JNIEXPORT jfloat JNICALL JEMCC_CallStaticFloatMethodA(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      jvalue *args);

JNIEXPORT jdouble JNICALL JEMCC_CallStaticDoubleMethod(JNIEnv *env,
                                                       jclass clazz,
                                                       jmethodID methodID, ...);
JNIEXPORT jdouble JNICALL JEMCC_CallStaticDoubleMethodV(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        va_list args);
JNIEXPORT jdouble JNICALL JEMCC_CallStaticDoubleMethodA(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        jvalue *args);

JNIEXPORT void JNICALL JEMCC_CallStaticVoidMethod(JNIEnv *env, jclass clazz,
                                                  jmethodID methodID, ...);
JNIEXPORT void JNICALL JEMCC_CallStaticVoidMethodV(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID,
                                                   va_list args);
JNIEXPORT void JNICALL JEMCC_CallStaticVoidMethodA(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID,
                                                   jvalue *args);

JNIEXPORT jfieldID JNICALL JEMCC_GetStaticFieldID(JNIEnv *env, jclass clazz,
                                                  const char *name,
                                                  const char *sig);

JNIEXPORT jobject JNICALL JEMCC_GetStaticObjectField(JNIEnv *env, jclass clazz,
                                                     jfieldID fieldID);
JNIEXPORT jboolean JNICALL JEMCC_GetStaticBooleanField(JNIEnv *env,
                                                       jclass clazz,
                                                       jfieldID fieldID);
JNIEXPORT jbyte JNICALL JEMCC_GetStaticByteField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID);
JNIEXPORT jchar JNICALL JEMCC_GetStaticCharField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID);
JNIEXPORT jshort JNICALL JEMCC_GetStaticShortField(JNIEnv *env, jclass clazz,
                                                   jfieldID fieldID);
JNIEXPORT jint JNICALL JEMCC_GetStaticIntField(JNIEnv *env, jclass clazz,
                                               jfieldID fieldID);
JNIEXPORT jlong JNICALL JEMCC_GetStaticLongField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID);
JNIEXPORT jfloat JNICALL JEMCC_GetStaticFloatField(JNIEnv *env, jclass clazz,
                                                   jfieldID fieldID);
JNIEXPORT jdouble JNICALL JEMCC_GetStaticDoubleField(JNIEnv *env, jclass clazz,
                                                     jfieldID fieldID);

JNIEXPORT void JNICALL JEMCC_SetStaticObjectField(JNIEnv *env, jclass clazz,
                                                  jfieldID fieldID,
                                                  jobject val);
JNIEXPORT void JNICALL JEMCC_SetStaticBooleanField(JNIEnv *env, jclass clazz,
                                                   jfieldID fieldID,
                                                   jboolean val);
JNIEXPORT void JNICALL JEMCC_SetStaticByteField(JNIEnv *env, jclass clazz,
                                                jfieldID fieldID, jbyte val);
JNIEXPORT void JNICALL JEMCC_SetStaticCharField(JNIEnv *env, jclass clazz,
                                                jfieldID fieldID, jchar val);
JNIEXPORT void JNICALL JEMCC_SetStaticShortField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID, jshort val);
JNIEXPORT void JNICALL JEMCC_SetStaticIntField(JNIEnv *env, jclass clazz,
                                               jfieldID fieldID, jint val);
JNIEXPORT void JNICALL JEMCC_SetStaticLongField(JNIEnv *env, jclass clazz,
                                                jfieldID fieldID, jlong val);
JNIEXPORT void JNICALL JEMCC_SetStaticFloatField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID, jfloat val);
JNIEXPORT void JNICALL JEMCC_SetStaticDoubleField(JNIEnv *env, jclass clazz,
                                                  jfieldID fieldID,
                                                  jdouble val);

JNIEXPORT jstring JNICALL JEMCC_NewString(JNIEnv *env, const jchar *unicode,
                                          jsize len);
JNIEXPORT jsize JNICALL JEMCC_GetStringLength(JNIEnv *env, jstring str);
JNIEXPORT const jchar *JNICALL JEMCC_GetStringChars(JNIEnv *env, jstring str,
                                                    jboolean *isCopy);
JNIEXPORT void JNICALL JEMCC_ReleaseStringChars(JNIEnv *env, jstring str,
                                                const jchar *chars);

JNIEXPORT jstring JNICALL JEMCC_NewStringUTF(JNIEnv *env, const char *utf);
JNIEXPORT jsize JNICALL JEMCC_GetStringUTFLength(JNIEnv *env, jstring str);
JNIEXPORT const char *JNICALL JEMCC_GetStringUTFChars(JNIEnv *env, jstring str,
                                                      jboolean *isCopy);
JNIEXPORT void JNICALL JEMCC_ReleaseStringUTFChars(JNIEnv *env, jstring str,
                                                   const char *chars);

JNIEXPORT jsize JNICALL JEMCC_GetArrayLength(JNIEnv *env, jarray array);
JNIEXPORT jobjectArray JNICALL JEMCC_NewObjectArray(JNIEnv *env, jsize len,
                                                    jclass clazz, jobject init);
JNIEXPORT jobject JNICALL JEMCC_GetObjectArrayElement(JNIEnv *env,
                                                      jobjectArray array,
                                                      jsize index);
JNIEXPORT void JNICALL JEMCC_SetObjectArrayElement(JNIEnv *env,
                                                   jobjectArray array,
                                                   jsize index, jobject val);

JNIEXPORT jbooleanArray JNICALL JEMCC_NewBooleanArray(JNIEnv *env, jsize len);
JNIEXPORT jbyteArray JNICALL JEMCC_NewByteArray(JNIEnv *env, jsize len);
JNIEXPORT jcharArray JNICALL JEMCC_NewCharArray(JNIEnv *env, jsize len);
JNIEXPORT jshortArray JNICALL JEMCC_NewShortArray(JNIEnv *env, jsize len);
JNIEXPORT jintArray JNICALL JEMCC_NewIntArray(JNIEnv *env, jsize len);
JNIEXPORT jlongArray JNICALL JEMCC_NewLongArray(JNIEnv *env, jsize len);
JNIEXPORT jfloatArray JNICALL JEMCC_NewFloatArray(JNIEnv *env, jsize len);
JNIEXPORT jdoubleArray JNICALL JEMCC_NewDoubleArray(JNIEnv *env, jsize len);

JNIEXPORT jboolean *JNICALL JEMCC_GetBooleanArrayElements(JNIEnv *env,
                                                          jbooleanArray array,
                                                          jboolean *isCopy);
JNIEXPORT jbyte *JNICALL JEMCC_GetByteArrayElements(JNIEnv *env,
                                                    jbyteArray array,
                                                    jboolean *isCopy);
JNIEXPORT jchar *JNICALL JEMCC_GetCharArrayElements(JNIEnv *env,
                                                    jcharArray array,
                                                    jboolean *isCopy);
JNIEXPORT jshort *JNICALL JEMCC_GetShortArrayElements(JNIEnv *env,
                                                      jshortArray array,
                                                      jboolean *isCopy);
JNIEXPORT jint *JNICALL JEMCC_GetIntArrayElements(JNIEnv *env,
                                                  jintArray array,
                                                  jboolean *isCopy);
JNIEXPORT jlong *JNICALL JEMCC_GetLongArrayElements(JNIEnv *env,
                                                    jlongArray array,
                                                    jboolean *isCopy);
JNIEXPORT jfloat *JNICALL JEMCC_GetFloatArrayElements(JNIEnv *env,
                                                      jfloatArray array,
                                                      jboolean *isCopy);
JNIEXPORT jdouble *JNICALL JEMCC_GetDoubleArrayElements(JNIEnv *env,
                                                        jdoubleArray array,
                                                        jboolean *isCopy);

JNIEXPORT void JNICALL JEMCC_ReleaseBooleanArrayElements(JNIEnv *env,
                                                         jbooleanArray array,
                                                         jboolean *elems,
                                                         jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseByteArrayElements(JNIEnv *env,
                                                      jbyteArray array,
                                                      jbyte *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseCharArrayElements(JNIEnv *env,
                                                      jcharArray array,
                                                      jchar *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseShortArrayElements(JNIEnv *env,
                                                       jshortArray array,
                                                       jshort *elems,
                                                       jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseIntArrayElements(JNIEnv *env,
                                                     jintArray array,
                                                     jint *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseLongArrayElements(JNIEnv *env,
                                                      jlongArray array,
                                                      jlong *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseFloatArrayElements(JNIEnv *env,
                                                       jfloatArray array,
                                                       jfloat *elems,
                                                       jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseDoubleArrayElements(JNIEnv *env,
                                                        jdoubleArray array,
                                                        jdouble *elems,
                                                        jint mode);

JNIEXPORT void JNICALL JEMCC_GetBooleanArrayRegion(JNIEnv *env,
                                                   jbooleanArray array,
                                                   jsize start, jsize len,
                                                   jboolean *buff);
JNIEXPORT void JNICALL JEMCC_GetByteArrayRegion(JNIEnv *env,
                                                jbyteArray array,
                                                jsize start, jsize len,
                                                jbyte *buff);
JNIEXPORT void JNICALL JEMCC_GetCharArrayRegion(JNIEnv *env,
                                                jcharArray array,
                                                jsize start, jsize len,
                                                jchar *buff);
JNIEXPORT void JNICALL JEMCC_GetShortArrayRegion(JNIEnv *env,
                                                 jshortArray array,
                                                 jsize start, jsize len,
                                                 jshort *buff);
JNIEXPORT void JNICALL JEMCC_GetIntArrayRegion(JNIEnv *env,
                                               jintArray array,
                                               jsize start, jsize len,
                                               jint *buff);
JNIEXPORT void JNICALL JEMCC_GetLongArrayRegion(JNIEnv *env,
                                                jlongArray array,
                                                jsize start, jsize len,
                                                jlong *buff);
JNIEXPORT void JNICALL JEMCC_GetFloatArrayRegion(JNIEnv *env,
                                                 jfloatArray array,
                                                 jsize start, jsize len,
                                                 jfloat *buff);
JNIEXPORT void JNICALL JEMCC_GetDoubleArrayRegion(JNIEnv *env,
                                                  jdoubleArray array,
                                                  jsize start, jsize len,
                                                  jdouble *buff);

JNIEXPORT void JNICALL JEMCC_SetBooleanArrayRegion(JNIEnv *env,
                                                   jbooleanArray array,
                                                   jsize start, jsize len,
                                                   jboolean *buff);
JNIEXPORT void JNICALL JEMCC_SetByteArrayRegion(JNIEnv *env,
                                                jbyteArray array,
                                                jsize start, jsize len,
                                                jbyte *buff);
JNIEXPORT void JNICALL JEMCC_SetCharArrayRegion(JNIEnv *env,
                                                jcharArray array,
                                                jsize start, jsize len,
                                                jchar *buff);
JNIEXPORT void JNICALL JEMCC_SetShortArrayRegion(JNIEnv *env,
                                                 jshortArray array,
                                                 jsize start, jsize len,
                                                 jshort *buff);
JNIEXPORT void JNICALL JEMCC_SetIntArrayRegion(JNIEnv *env,
                                               jintArray array,
                                               jsize start, jsize len,
                                               jint *buff);
JNIEXPORT void JNICALL JEMCC_SetLongArrayRegion(JNIEnv *env,
                                                jlongArray array,
                                                jsize start, jsize len,
                                                jlong *buff);
JNIEXPORT void JNICALL JEMCC_SetFloatArrayRegion(JNIEnv *env,
                                                 jfloatArray array,
                                                 jsize start, jsize len,
                                                 jfloat *buff);
JNIEXPORT void JNICALL JEMCC_SetDoubleArrayRegion(JNIEnv *env,
                                                  jdoubleArray array,
                                                  jsize start, jsize len,
                                                  jdouble *buff);

JNIEXPORT jint JNICALL JEMCC_RegisterNatives(JNIEnv *env, jclass clazz,
                                             const JNINativeMethod *methods,
                                             jint nMethods);
JNIEXPORT jint JNICALL JEMCC_UnregisterNatives(JNIEnv *env, jclass clazz);

/* NOTE: MonitorEnter/MonitorExit are directly defined by sysenv.h */

JNIEXPORT jint JNICALL JEMCC_GetJavaVM(JNIEnv *env, JavaVM **vm);

/* <jemcc_end> */

#endif
