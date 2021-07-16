/**
 * JEMCC methods to map variable argument list functions.
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

/*
 * In the (current) simplest form, these functions simply call the mapping
 * ---V method.  See the comments for the associated V/A function for 
 * additional details.
 */

jobject JEMCC_NewObject(JNIEnv *env, jclass clazz, 
                        jmethodID methodID, ...) {
    va_list argList;
    jobject retVal;

    va_start(argList, methodID);
    retVal = JEMCC_NewObjectV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jobject JEMCC_CallObjectMethod(JNIEnv *env, jobject obj,
                               jmethodID methodID, ...) {
    va_list argList;
    jobject retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallObjectMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jboolean JEMCC_CallBooleanMethod(JNIEnv *env, jobject obj,
                                 jmethodID methodID, ...) {
    va_list argList;
    jboolean retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallBooleanMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jbyte JEMCC_CallByteMethod(JNIEnv *env, jobject obj, 
                           jmethodID methodID, ...) {
    va_list argList;
    jbyte retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallByteMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jchar JEMCC_CallCharMethod(JNIEnv *env, jobject obj, 
                           jmethodID methodID, ...) {
    va_list argList;
    jchar retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallCharMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jshort JEMCC_CallShortMethod(JNIEnv *env, jobject obj, 
                             jmethodID methodID, ...) {
    va_list argList;
    jshort retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallShortMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jint JEMCC_CallIntMethod(JNIEnv *env, jobject obj, 
                         jmethodID methodID, ...) {
    va_list argList;
    jint retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallIntMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jlong JEMCC_CallLongMethod(JNIEnv *env, jobject obj, 
                           jmethodID methodID, ...) {
    va_list argList;
    jlong retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallLongMethod(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jfloat JEMCC_CallFloatMethod(JNIEnv *env, jobject obj, 
                             jmethodID methodID, ...) {
    va_list argList;
    jfloat retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallFloatMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

jdouble JEMCC_CallDoubleMethod(JNIEnv *env, jobject obj,
                               jmethodID methodID, ...) {
    va_list argList;
    jdouble retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallDoubleMethodV(env, obj, methodID, argList);
    va_end(argList);

    return retVal;
}

void JEMCC_CallVoidMethod(JNIEnv *env, jobject obj, 
                          jmethodID methodID, ...) {
    va_list argList;

    va_start(argList, methodID);
    JEMCC_CallVoidMethodV(env, obj, methodID, argList);
    va_end(argList);
}

jobject JEMCC_CallNonvirtualObjectMethod(JNIEnv *env, jobject obj, 
                                         jclass clazz,
                                         jmethodID methodID, ...) {
    va_list argList;
    jobject retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualObjectMethodV(env, obj, clazz,
                                               methodID, argList);
    va_end(argList);

    return retVal;
}

jboolean JEMCC_CallNonvirtualBooleanMethod(JNIEnv *env, jobject obj, 
                                           jclass clazz,
                                           jmethodID methodID, ...) {
    va_list argList;
    jboolean retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualBooleanMethodV(env, obj, clazz,
                                                methodID, argList);
    va_end(argList);

    return retVal;
}

jbyte JEMCC_CallNonvirtualByteMethod(JNIEnv *env, jobject obj, jclass clazz,
                                     jmethodID methodID, ...) {
    va_list argList;
    jbyte retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualByteMethodV(env, obj, clazz,
                                             methodID, argList);
    va_end(argList);

    return retVal;
}

jchar JEMCC_CallNonvirtualCharMethod(JNIEnv *env, jobject obj, jclass clazz,
                                     jmethodID methodID, ...) {
    va_list argList;
    jchar retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualCharMethodV(env, obj, clazz,
                                             methodID, argList);
    va_end(argList);

    return retVal;
}

jshort JEMCC_CallNonvirtualShortMethod(JNIEnv *env, jobject obj, jclass clazz,
                                       jmethodID methodID, ...) {
    va_list argList;
    jshort retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualShortMethodV(env, obj, clazz,
                                              methodID, argList);
    va_end(argList);

    return retVal;
}

jint JEMCC_CallNonvirtualIntMethod(JNIEnv *env, jobject obj, jclass clazz,
                                   jmethodID methodID, ...) {
    va_list argList;
    jint retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualIntMethodV(env, obj, clazz,
                                            methodID, argList);
    va_end(argList);

    return retVal;
}

jlong JEMCC_CallNonvirtualLongMethod(JNIEnv *env, jobject obj, jclass clazz,
                                     jmethodID methodID, ...) {
    va_list argList;
    jlong retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualLongMethodV(env, obj, clazz,
                                             methodID, argList);
    va_end(argList);

    return retVal;
}

jfloat JEMCC_CallNonvirtualFloatMethod(JNIEnv *env, jobject obj, jclass clazz,
                                       jmethodID methodID, ...) {
    va_list argList;
    jfloat retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualFloatMethodV(env, obj, clazz,
                                              methodID, argList);
    va_end(argList);

    return retVal;
}

jdouble JEMCC_CallNonvirtualDoubleMethod(JNIEnv *env, jobject obj, jclass clazz,
                                         jmethodID methodID, ...) {
    va_list argList;
    jdouble retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallNonvirtualDoubleMethodV(env, obj, clazz,
                                               methodID, argList);
    va_end(argList);

    return retVal;
}

void JEMCC_CallNonvirtualVoidMethod(JNIEnv *env, jobject obj, jclass clazz,
                                    jmethodID methodID, ...) {
    va_list argList;

    va_start(argList, methodID);
    JEMCC_CallNonvirtualVoidMethodV(env, obj, clazz, methodID, argList);
    va_end(argList);
}

jobject JEMCC_CallStaticObjectMethod(JNIEnv *env, jclass clazz,
                                     jmethodID methodID, ...) {
    va_list argList;
    jobject retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticObjectMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jboolean JEMCC_CallStaticBooleanMethod(JNIEnv *env, jclass clazz,
                                       jmethodID methodID, ...) {
    va_list argList;
    jboolean retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticBooleanMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jbyte JEMCC_CallStaticByteMethod(JNIEnv *env, jclass clazz,
                                 jmethodID methodID, ...) {
    va_list argList;
    jbyte retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticByteMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jchar JEMCC_CallStaticCharMethod(JNIEnv *env, jclass clazz,
                                 jmethodID methodID, ...) {
    va_list argList;
    jchar retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticCharMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jshort JEMCC_CallStaticShortMethod(JNIEnv *env, jclass clazz,
                                   jmethodID methodID, ...) {
    va_list argList;
    jshort retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticShortMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jint JEMCC_CallStaticIntMethod(JNIEnv *env, jclass clazz,
                               jmethodID methodID, ...) {
    va_list argList;
    jint retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticIntMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jlong JEMCC_CallStaticLongMethod(JNIEnv *env, jclass clazz,
                                 jmethodID methodID, ...) {
    va_list argList;
    jlong retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticLongMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jfloat JEMCC_CallStaticFloatMethod(JNIEnv *env, jclass clazz,
                                   jmethodID methodID, ...) {
    va_list argList;
    jfloat retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticFloatMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

jdouble JEMCC_CallStaticDoubleMethod(JNIEnv *env, jclass clazz,
                                     jmethodID methodID, ...) {
    va_list argList;
    jdouble retVal;

    va_start(argList, methodID);
    retVal = JEMCC_CallStaticDoubleMethodV(env, clazz, methodID, argList);
    va_end(argList);

    return retVal;
}

void JEMCC_CallStaticVoidMethod(JNIEnv *env, jclass clazz,
                                jmethodID methodID, ...) {
    va_list argList;

    va_start(argList, methodID);
    JEMCC_CallStaticVoidMethodV(env, clazz, methodID, argList);
    va_end(argList);
}
