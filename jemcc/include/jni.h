/**
 * Java Native Interface definitions for the JEMCC implementation.
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

/* Avoid JNI_H (Sun's tag) - big compile error if both included */
#ifndef WRDG_JNI_H
#define WRDG_JNI_H 1

/* Prepare for the variable argument methods */
#include <stdarg.h>

/* Read the platform specific type information */
#if (defined(_WIN32) || defined(__WIN32__))
#include "jnitypes.w32"
#else
#include "jnitypes.h"
#endif

/* Versioning information */
#define JNI_VERSION_1_1 0x00010001

/* Status return codes from JNI functions */
#define JNI_OK          0   /* function was successful */
#define JNI_ERR       (-1)  /* general or unknown function error */
#define JNI_EDETACHED (-2)  /* thread detached from virtual machine */
#define JNI_EVERSION  (-3)  /* invalid or mismatched version error */
#define JNI_ENOMEM    (-4)  /* virtual machine has insufficient memory */
#define JNI_EEXIST    (-5)  /* virtual machine already exists */
#define JNI_EINVAL    (-6)  /* invalid arguments for request */

/* Identifiers used in array element release methods */
#define JNI_COMMIT 1
#define JNI_ABORT 2

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declare the primary types of the native machine structures */
#ifdef __cplusplus
typedef struct JavaVM JavaVM;
typedef struct JNIEnv JNIEnv;
#else
typedef const struct JNIInvokeInterface *JavaVM;
typedef const struct JNINativeInterface *JNIEnv;
#endif

/* Revert to Java 1.1 specifications (book) */
typedef struct JDK1_1InitArgs {
    jint version; 

    char **properties;
    jint checkSource;
    jint nativeStackSize;
    jint javaStackSize;
    jint minHeapSize;
    jint maxHeapSize;
    jint verifyMode;
    char *classpath;

    jint (*vfprintf)(FILE *fp, const char *format, va_list args);
    void (*exit)(jint code);
    void (*abort)();

    jint enableClassGC;
    jint enableVerboseGC;
    jint disableAsyncGC;

    /* Note: these are reserved by Sun, used by JEMCC */
    void *libpath;
    jint reservedb;
    jint reservedc;
} JDK1_1InitArgs;

/* Invocation interface for defining JavaVM structure/class */
struct JNIInvokeInterface {
    void *reserveda;  /* These are always NULL */
    void *reservedb;
    void *reservedc;

    jint (JNICALL *DestroyJavaVM)(JavaVM *vm);
    jint (JNICALL *AttachCurrentThread)(JavaVM *vm, JNIEnv **pEnv,
                                        void *args);
    jint (JNICALL *DetachCurrentThread)(JavaVM *vm);
};

JNIEXPORT jint JNICALL JNI_GetDefaultJavaVMInitArgs(void *args);
JNIEXPORT jint JNICALL JNI_GetCreatedJavaVMs(JavaVM **vmBuff, jsize buffLen,
                                             jsize *nVMs);
JNIEXPORT jint JNICALL JNI_CreateJavaVM(JavaVM **pVm, JNIEnv **pEnv,
                                        void *args);

#ifdef __cplusplus
struct JavaVM {
    JNIInvokeInterface *fnTbl;

    jint DestroyJavaVM() {
        return fnTbl->DestroyJavaVM(this);
    }

    jint AttachCurrentThread(JNIEnv **pEnv, void *args) {
        return fnTbl->AttachCurrentThread(this, pEnv, args);
    }

    jint DetachCurrentThread() {
        return fnTbl->DetachCurrentThread(this);
    }
};
#endif

/* Native interface structures, as defined in Chapter 4 */

/* Structure used from native method registration */
typedef struct {
    char *name;
    char *signature;
    void *fnPtr;
} JNINativeMethod;

/* Really big table follows! */
struct JNINativeInterface {
    void *reserveda;  /* These are always NULL */
    void *reservedb;
    void *reservedc;
    void *reservedd;

    jint (JNICALL *GetVersion)(JNIEnv *env);
    jclass (JNICALL *DefineClass)(JNIEnv *env, jobject loader,
                                  const jbyte *buff, jsize buffLen);
    jclass (JNICALL *FindClass)(JNIEnv *env, const char *name);

    void *reservede;
    void *reservedf;
    void *reservedg;

    jclass (JNICALL *GetSuperclass)(JNIEnv *env, jclass clazz);
    jboolean (JNICALL *IsAssignableFrom)(JNIEnv *env,
                                         jclass clazza, jclass clazzb);

    void *reservedh;

    jint (JNICALL *Throw)(JNIEnv *env, jthrowable obj);
    jint (JNICALL *ThrowNew)(JNIEnv *env, jclass clazz, const char *msg);
    jthrowable (JNICALL *ExceptionOccurred)(JNIEnv *env);
    void (JNICALL *ExceptionDescribe)(JNIEnv *env);
    void (JNICALL *ExceptionClear)(JNIEnv *env);
    void (JNICALL *FatalError)(JNIEnv *env, const char *msg);

    void *reservedi;
    void *reservedj;

    jobject (JNICALL *NewGlobalRef)(JNIEnv *env, jobject obj);
    void (JNICALL *DeleteGlobalRef)(JNIEnv *env, jobject globalRef);
    void (JNICALL *DeleteLocalRef)(JNIEnv *env, jobject localRef);
    jboolean (JNICALL *IsSameObject)(JNIEnv *env, jobject obja, jobject objb);

    void *reservedk;
    void *reservedl;

    jobject (JNICALL *AllocObject)(JNIEnv *env, jclass clazz);
    jobject (JNICALL *NewObject)(JNIEnv *env, jclass clazz,
                                 jmethodID methodID, ...);
    jobject (JNICALL *NewObjectV)(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, va_list args);
    jobject (JNICALL *NewObjectA)(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, jvalue *args);

    jclass (JNICALL *GetObjectClass)(JNIEnv *env, jobject obj);
    jboolean (JNICALL *IsInstanceOf)(JNIEnv *env, jobject obj, jclass clazz);


    jmethodID (JNICALL *GetMethodID)(JNIEnv *env, jclass clazz,
                                     const char *name, const char *sig);

    jobject (JNICALL *CallObjectMethod)(JNIEnv *env, jobject obj,
                                        jmethodID methodID, ...);
    jobject (JNICALL *CallObjectMethodV)(JNIEnv *env, jobject obj,
                                         jmethodID methodID, va_list args);
    jobject (JNICALL *CallObjectMethodA)(JNIEnv *env, jobject obj,
                                         jmethodID methodID, jvalue *args);

    jboolean (JNICALL *CallBooleanMethod)(JNIEnv *env, jobject obj,
                                          jmethodID methodID, ...);
    jboolean (JNICALL *CallBooleanMethodV)(JNIEnv *env, jobject obj,
                                           jmethodID methodID, va_list args);
    jboolean (JNICALL *CallBooleanMethodA)(JNIEnv *env, jobject obj,
                                           jmethodID methodID, jvalue *args);

    jbyte (JNICALL *CallByteMethod)(JNIEnv *env, jobject obj,
                                    jmethodID methodID, ...);
    jbyte (JNICALL *CallByteMethodV)(JNIEnv *env, jobject obj,
                                     jmethodID methodID, va_list args);
    jbyte (JNICALL *CallByteMethodA)(JNIEnv *env, jobject obj,
                                     jmethodID methodID, jvalue *args);

    jchar (JNICALL *CallCharMethod)(JNIEnv *env, jobject obj,
                                    jmethodID methodID, ...);
    jchar (JNICALL *CallCharMethodV)(JNIEnv *env, jobject obj,
                                     jmethodID methodID, va_list args);
    jchar (JNICALL *CallCharMethodA)(JNIEnv *env, jobject obj,
                                     jmethodID methodID, jvalue *args);

    jshort (JNICALL *CallShortMethod)(JNIEnv *env, jobject obj,
                                      jmethodID methodID, ...);
    jshort (JNICALL *CallShortMethodV)(JNIEnv *env, jobject obj,
                                       jmethodID methodID, va_list args);
    jshort (JNICALL *CallShortMethodA)(JNIEnv *env, jobject obj,
                                       jmethodID methodID, jvalue *args);

    jint (JNICALL *CallIntMethod)(JNIEnv *env, jobject obj,
                                  jmethodID methodID, ...);
    jint (JNICALL *CallIntMethodV)(JNIEnv *env, jobject obj,
                                   jmethodID methodID, va_list args);
    jint (JNICALL *CallIntMethodA)(JNIEnv *env, jobject obj,
                                   jmethodID methodID, jvalue *args);

    jlong (JNICALL *CallLongMethod)(JNIEnv *env, jobject obj,
                                    jmethodID methodID, ...);
    jlong (JNICALL *CallLongMethodV)(JNIEnv *env, jobject obj,
                                     jmethodID methodID, va_list args);
    jlong (JNICALL *CallLongMethodA)(JNIEnv *env, jobject obj,
                                     jmethodID methodID, jvalue *args);

    jfloat (JNICALL *CallFloatMethod)(JNIEnv *env, jobject obj,
                                      jmethodID methodID, ...);
    jfloat (JNICALL *CallFloatMethodV)(JNIEnv *env, jobject obj,
                                       jmethodID methodID, va_list args);
    jfloat (JNICALL *CallFloatMethodA)(JNIEnv *env, jobject obj,
                                       jmethodID methodID, jvalue *args);

    jdouble (JNICALL *CallDoubleMethod)(JNIEnv *env, jobject obj,
                                        jmethodID methodID, ...);
    jdouble (JNICALL *CallDoubleMethodV)(JNIEnv *env, jobject obj,
                                         jmethodID methodID, va_list args);
    jdouble (JNICALL *CallDoubleMethodA)(JNIEnv *env, jobject obj,
                                         jmethodID methodID, jvalue *args);

    void (JNICALL *CallVoidMethod)(JNIEnv *env, jobject obj,
                                   jmethodID methodID, ...);
    void (JNICALL *CallVoidMethodV)(JNIEnv *env, jobject obj,
                                    jmethodID methodID, va_list args);
    void (JNICALL *CallVoidMethodA)(JNIEnv *env, jobject obj,
                                    jmethodID methodID, jvalue *args);


    jobject (JNICALL *CallNonvirtualObjectMethod)(JNIEnv *env, jobject obj,
                                                  jclass clazz,
                                                  jmethodID methodID, ...);
    jobject (JNICALL *CallNonvirtualObjectMethodV)(JNIEnv *env, jobject obj,
                                                   jclass clazz,
                                                   jmethodID methodID,
                                                   va_list args);
    jobject (JNICALL *CallNonvirtualObjectMethodA)(JNIEnv *env, jobject obj,
                                                   jclass clazz,
                                                   jmethodID methodID,
                                                   jvalue *args);

    jboolean (JNICALL *CallNonvirtualBooleanMethod)(JNIEnv *env, jobject obj,
                                                    jclass clazz,
                                                    jmethodID methodID, ...);
    jboolean (JNICALL *CallNonvirtualBooleanMethodV)(JNIEnv *env, jobject obj,
                                                     jclass clazz,
                                                     jmethodID methodID,
                                                     va_list args);
    jboolean (JNICALL *CallNonvirtualBooleanMethodA)(JNIEnv *env, jobject obj,
                                                     jclass clazz,
                                                     jmethodID methodID,
                                                     jvalue *args);

    jbyte (JNICALL *CallNonvirtualByteMethod)(JNIEnv *env, jobject obj,
                                              jclass clazz,
                                              jmethodID methodID, ...);
    jbyte (JNICALL *CallNonvirtualByteMethodV)(JNIEnv *env, jobject obj,
                                               jclass clazz,
                                               jmethodID methodID,
                                               va_list args);
    jbyte (JNICALL *CallNonvirtualByteMethodA)(JNIEnv *env, jobject obj,
                                               jclass clazz,
                                               jmethodID methodID,
                                               jvalue *args);

    jchar (JNICALL *CallNonvirtualCharMethod)(JNIEnv *env, jobject obj,
                                              jclass clazz,
                                              jmethodID methodID, ...);
    jchar (JNICALL *CallNonvirtualCharMethodV)(JNIEnv *env, jobject obj,
                                               jclass clazz,
                                               jmethodID methodID,
                                               va_list args);
    jchar (JNICALL *CallNonvirtualCharMethodA)(JNIEnv *env, jobject obj,
                                               jclass clazz,
                                               jmethodID methodID,
                                               jvalue *args);

    jshort (JNICALL *CallNonvirtualShortMethod)(JNIEnv *env, jobject obj,
                                                jclass clazz,
                                                jmethodID methodID, ...);
    jshort (JNICALL *CallNonvirtualShortMethodV)(JNIEnv *env, jobject obj,
                                                jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
    jshort (JNICALL *CallNonvirtualShortMethodA)(JNIEnv *env, jobject obj,
                                                jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

    jint (JNICALL *CallNonvirtualIntMethod)(JNIEnv *env, jobject obj,
                                            jclass clazz,
                                            jmethodID methodID, ...);
    jint (JNICALL *CallNonvirtualIntMethodV)(JNIEnv *env, jobject obj,
                                             jclass clazz,
                                             jmethodID methodID,
                                             va_list args);
    jint (JNICALL *CallNonvirtualIntMethodA)(JNIEnv *env, jobject obj,
                                             jclass clazz,
                                             jmethodID methodID,
                                             jvalue *args);

    jlong (JNICALL *CallNonvirtualLongMethod)(JNIEnv *env, jobject obj,
                                              jclass clazz,
                                              jmethodID methodID, ...);
    jlong (JNICALL *CallNonvirtualLongMethodV)(JNIEnv *env, jobject obj,
                                               jclass clazz,
                                               jmethodID methodID,
                                               va_list args);
    jlong (JNICALL *CallNonvirtualLongMethodA)(JNIEnv *env, jobject obj,
                                               jclass clazz,
                                               jmethodID methodID,
                                               jvalue *args);

    jfloat (JNICALL *CallNonvirtualFloatMethod)(JNIEnv *env, jobject obj,
                                                jclass clazz,
                                                jmethodID methodID, ...);
    jfloat (JNICALL *CallNonvirtualFloatMethodV)(JNIEnv *env, jobject obj,
                                                 jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
    jfloat (JNICALL *CallNonvirtualFloatMethodA)(JNIEnv *env, jobject obj,
                                                 jclass clazz,
                                                 jmethodID methodID,
                                                 jvalue *args);

    jdouble (JNICALL *CallNonvirtualDoubleMethod)(JNIEnv *env, jobject obj,
                                                  jclass clazz,
                                                  jmethodID methodID, ...);
    jdouble (JNICALL *CallNonvirtualDoubleMethodV)(JNIEnv *env, jobject obj,
                                                   jclass clazz,
                                                   jmethodID methodID,
                                                   va_list args);
    jdouble (JNICALL *CallNonvirtualDoubleMethodA)(JNIEnv *env, jobject obj,
                                                   jclass clazz,
                                                   jmethodID methodID,
                                                   jvalue *args);

    void (JNICALL *CallNonvirtualVoidMethod)(JNIEnv *env, jobject obj,
                                             jclass clazz,
                                             jmethodID methodID, ...);
    void (JNICALL *CallNonvirtualVoidMethodV)(JNIEnv *env, jobject obj,
                                              jclass clazz,
                                              jmethodID methodID,
                                              va_list args);
    void (JNICALL *CallNonvirtualVoidMethodA)(JNIEnv *env, jobject obj,
                                              jclass clazz,
                                              jmethodID methodID,
                                              jvalue *args);


    jfieldID (JNICALL *GetFieldID)(JNIEnv *env, jclass clazz,
                                   const char *name, const char *sig);

    jobject (JNICALL *GetObjectField)(JNIEnv *env, jobject obj,
                                      jfieldID fieldID);
    jboolean (JNICALL *GetBooleanField)(JNIEnv *env, jobject obj,
                                        jfieldID fieldID);
    jbyte (JNICALL *GetByteField)(JNIEnv *env, jobject obj,
                                  jfieldID fieldID);
    jchar (JNICALL *GetCharField)(JNIEnv *env, jobject obj,
                                  jfieldID fieldID);
    jshort (JNICALL *GetShortField)(JNIEnv *env, jobject obj,
                                    jfieldID fieldID);
    jint (JNICALL *GetIntField)(JNIEnv *env, jobject obj,
                                jfieldID fieldID);
    jlong (JNICALL *GetLongField)(JNIEnv *env, jobject obj,
                                  jfieldID fieldID);
    jfloat (JNICALL *GetFloatField)(JNIEnv *env, jobject obj,
                                    jfieldID fieldID);
    jdouble (JNICALL *GetDoubleField)(JNIEnv *env, jobject obj,
                                      jfieldID fieldID);

    void (JNICALL *SetObjectField)(JNIEnv *env, jobject obj,
                                   jfieldID fieldID, jobject val);
    void (JNICALL *SetBooleanField)(JNIEnv *env, jobject obj,
                                    jfieldID fieldID, jboolean val);
    void (JNICALL *SetByteField)(JNIEnv *env, jobject obj,
                                 jfieldID fieldID, jbyte val);
    void (JNICALL *SetCharField)(JNIEnv *env, jobject obj,
                                 jfieldID fieldID, jchar val);
    void (JNICALL *SetShortField)(JNIEnv *env, jobject obj,
                                  jfieldID fieldID, jshort val);
    void (JNICALL *SetIntField)(JNIEnv *env, jobject obj,
                                jfieldID fieldID, jint val);
    void (JNICALL *SetLongField)(JNIEnv *env, jobject obj,
                                 jfieldID fieldID, jlong val);
    void (JNICALL *SetFloatField)(JNIEnv *env, jobject obj,
                                  jfieldID fieldID, jfloat val);
    void (JNICALL *SetDoubleField)(JNIEnv *env, jobject obj,
                                   jfieldID fieldID, jdouble val);


    jmethodID (JNICALL *GetStaticMethodID)(JNIEnv *env, jclass clazz,
                                           const char *name, const char *sig);

    jobject (JNICALL *CallStaticObjectMethod)(JNIEnv *env, jclass clazz,
                                              jmethodID methodID, ...);
    jobject (JNICALL *CallStaticObjectMethodV)(JNIEnv *env, jclass clazz,
                                               jmethodID methodID,
                                               va_list args);
    jobject (JNICALL *CallStaticObjectMethodA)(JNIEnv *env, jclass clazz,
                                               jmethodID methodID,
                                               jvalue *args);

    jboolean (JNICALL *CallStaticBooleanMethod)(JNIEnv *env, jclass clazz,
                                                jmethodID methodID, ...);
    jboolean (JNICALL *CallStaticBooleanMethodV)(JNIEnv *env, jclass clazz,
                                                 jmethodID methodID,
                                                 va_list args);
    jboolean (JNICALL *CallStaticBooleanMethodA)(JNIEnv *env, jclass clazz,
                                                 jmethodID methodID,
                                                 jvalue *args);

    jbyte (JNICALL *CallStaticByteMethod)(JNIEnv *env, jclass clazz,
                                          jmethodID methodID, ...);
    jbyte (JNICALL *CallStaticByteMethodV)(JNIEnv *env, jclass clazz,
                                           jmethodID methodID, va_list args);
    jbyte (JNICALL *CallStaticByteMethodA)(JNIEnv *env, jclass clazz,
                                           jmethodID methodID, jvalue *args);

    jchar (JNICALL *CallStaticCharMethod)(JNIEnv *env, jclass clazz,
                                          jmethodID methodID, ...);
    jchar (JNICALL *CallStaticCharMethodV)(JNIEnv *env, jclass clazz,
                                           jmethodID methodID, va_list args);
    jchar (JNICALL *CallStaticCharMethodA)(JNIEnv *env, jclass clazz,
                                           jmethodID methodID, jvalue *args);

    jshort (JNICALL *CallStaticShortMethod)(JNIEnv *env, jclass clazz,
                                            jmethodID methodID, ...);
    jshort (JNICALL *CallStaticShortMethodV)(JNIEnv *env, jclass clazz,
                                             jmethodID methodID, va_list args);
    jshort (JNICALL *CallStaticShortMethodA)(JNIEnv *env, jclass clazz,
                                             jmethodID methodID, jvalue *args);

    jint (JNICALL *CallStaticIntMethod)(JNIEnv *env, jclass clazz,
                                        jmethodID methodID, ...);
    jint (JNICALL *CallStaticIntMethodV)(JNIEnv *env, jclass clazz,
                                         jmethodID methodID, va_list args);
    jint (JNICALL *CallStaticIntMethodA)(JNIEnv *env, jclass clazz,
                                         jmethodID methodID, jvalue *args);

    jlong (JNICALL *CallStaticLongMethod)(JNIEnv *env, jclass clazz,
                                          jmethodID methodID, ...);
    jlong (JNICALL *CallStaticLongMethodV)(JNIEnv *env, jclass clazz,
                                           jmethodID methodID, va_list args);
    jlong (JNICALL *CallStaticLongMethodA)(JNIEnv *env, jclass clazz,
                                           jmethodID methodID, jvalue *args);

    jfloat (JNICALL *CallStaticFloatMethod)(JNIEnv *env, jclass clazz,
                                            jmethodID methodID, ...);
    jfloat (JNICALL *CallStaticFloatMethodV)(JNIEnv *env, jclass clazz,
                                             jmethodID methodID, va_list args);
    jfloat (JNICALL *CallStaticFloatMethodA)(JNIEnv *env, jclass clazz,
                                             jmethodID methodID, jvalue *args);

    jdouble (JNICALL *CallStaticDoubleMethod)(JNIEnv *env, jclass clazz,
                                              jmethodID methodID, ...);
    jdouble (JNICALL *CallStaticDoubleMethodV)(JNIEnv *env, jclass clazz,
                                               jmethodID methodID,
                                               va_list args);
    jdouble (JNICALL *CallStaticDoubleMethodA)(JNIEnv *env, jclass clazz,
                                               jmethodID methodID,
                                               jvalue *args);

    void (JNICALL *CallStaticVoidMethod)(JNIEnv *env, jclass clazz,
                                         jmethodID methodID, ...);
    void (JNICALL *CallStaticVoidMethodV)(JNIEnv *env, jclass clazz,
                                          jmethodID methodID, va_list args);
    void (JNICALL *CallStaticVoidMethodA)(JNIEnv *env, jclass clazz,
                                          jmethodID methodID, jvalue *args);

    jfieldID (JNICALL *GetStaticFieldID)(JNIEnv *env, jclass clazz,
                                         const char *name, const char *sig);

    jobject (JNICALL *GetStaticObjectField)(JNIEnv *env, jclass clazz,
                                            jfieldID fieldID);
    jboolean (JNICALL *GetStaticBooleanField)(JNIEnv *env, jclass clazz,
                                              jfieldID fieldID);
    jbyte (JNICALL *GetStaticByteField)(JNIEnv *env, jclass clazz,
                                        jfieldID fieldID);
    jchar (JNICALL *GetStaticCharField)(JNIEnv *env, jclass clazz,
                                        jfieldID fieldID);
    jshort (JNICALL *GetStaticShortField)(JNIEnv *env, jclass clazz,
                                          jfieldID fieldID);
    jint (JNICALL *GetStaticIntField)(JNIEnv *env, jclass clazz,
                                      jfieldID fieldID);
    jlong (JNICALL *GetStaticLongField)(JNIEnv *env, jclass clazz,
                                        jfieldID fieldID);
    jfloat (JNICALL *GetStaticFloatField)(JNIEnv *env, jclass clazz,
                                          jfieldID fieldID);
    jdouble (JNICALL *GetStaticDoubleField)(JNIEnv *env, jclass clazz,
                                            jfieldID fieldID);

    void (JNICALL *SetStaticObjectField)(JNIEnv *env, jclass clazz,
                                         jfieldID fieldID, jobject val);
    void (JNICALL *SetStaticBooleanField)(JNIEnv *env, jclass clazz,
                                          jfieldID fieldID, jboolean val);
    void (JNICALL *SetStaticByteField)(JNIEnv *env, jclass clazz,
                                       jfieldID fieldID, jbyte val);
    void (JNICALL *SetStaticCharField)(JNIEnv *env, jclass clazz,
                                       jfieldID fieldID, jchar val);
    void (JNICALL *SetStaticShortField)(JNIEnv *env, jclass clazz,
                                        jfieldID fieldID, jshort val);
    void (JNICALL *SetStaticIntField)(JNIEnv *env, jclass clazz,
                                      jfieldID fieldID, jint val);
    void (JNICALL *SetStaticLongField)(JNIEnv *env, jclass clazz,
                                       jfieldID fieldID, jlong val);
    void (JNICALL *SetStaticFloatField)(JNIEnv *env, jclass clazz,
                                        jfieldID fieldID, jfloat val);
    void (JNICALL *SetStaticDoubleField)(JNIEnv *env, jclass clazz,
                                         jfieldID fieldID, jdouble val);

    jstring (JNICALL *NewString)(JNIEnv *env, const jchar *unicode, jsize len);
    jsize (JNICALL *GetStringLength)(JNIEnv *env, jstring str);
    const jchar *(JNICALL *GetStringChars)(JNIEnv *env, jstring str,
                                           jboolean *isCopy);
    void (JNICALL *ReleaseStringChars)(JNIEnv *env, jstring str,
                                       const jchar *chars);

    jstring (JNICALL *NewStringUTF)(JNIEnv *env, const char *utf);
    jsize (JNICALL *GetStringUTFLength)(JNIEnv *env, jstring str);
    const char *(JNICALL *GetStringUTFChars)(JNIEnv *env, jstring str,
                                             jboolean *isCopy);
    void (JNICALL *ReleaseStringUTFChars)(JNIEnv *env, jstring str,
                                          const char *chars);

    jsize (JNICALL *GetArrayLength)(JNIEnv *env, jarray array);

    jobjectArray (JNICALL *NewObjectArray)(JNIEnv *env, jsize len,
                                           jclass clazz, jobject init);
    jobject (JNICALL *GetObjectArrayElement)(JNIEnv *env, jobjectArray array,
                                             jsize index);
    void (JNICALL *SetObjectArrayElement)(JNIEnv *env, jobjectArray array,
                                          jsize index, jobject val);

    jbooleanArray (JNICALL *NewBooleanArray)(JNIEnv *env, jsize len);
    jbyteArray (JNICALL *NewByteArray)(JNIEnv *env, jsize len);
    jcharArray (JNICALL *NewCharArray)(JNIEnv *env, jsize len);
    jshortArray (JNICALL *NewShortArray)(JNIEnv *env, jsize len);
    jintArray (JNICALL *NewIntArray)(JNIEnv *env, jsize len);
    jlongArray (JNICALL *NewLongArray)(JNIEnv *env, jsize len);
    jfloatArray (JNICALL *NewFloatArray)(JNIEnv *env, jsize len);
    jdoubleArray (JNICALL *NewDoubleArray)(JNIEnv *env, jsize len);

    jboolean *(JNICALL *GetBooleanArrayElements)(JNIEnv *env,
                                                 jbooleanArray array,
                                                 jboolean *isCopy);
    jbyte *(JNICALL *GetByteArrayElements)(JNIEnv *env,
                                           jbyteArray array,
                                           jboolean *isCopy);
    jchar *(JNICALL *GetCharArrayElements)(JNIEnv *env,
                                           jcharArray array,
                                           jboolean *isCopy);
    jshort *(JNICALL *GetShortArrayElements)(JNIEnv *env,
                                             jshortArray array,
                                             jboolean *isCopy);
    jint *(JNICALL *GetIntArrayElements)(JNIEnv *env,
                                         jintArray array,
                                         jboolean *isCopy);
    jlong *(JNICALL *GetLongArrayElements)(JNIEnv *env,
                                           jlongArray array,
                                           jboolean *isCopy);
    jfloat *(JNICALL *GetFloatArrayElements)(JNIEnv *env,
                                             jfloatArray array,
                                             jboolean *isCopy);
    jdouble *(JNICALL *GetDoubleArrayElements)(JNIEnv *env,
                                               jdoubleArray array,
                                               jboolean *isCopy);

    void (JNICALL *ReleaseBooleanArrayElements)(JNIEnv *env,
                                                jbooleanArray array,
                                                jboolean *elems, jint mode);
    void (JNICALL *ReleaseByteArrayElements)(JNIEnv *env,
                                             jbyteArray array,
                                             jbyte *elems, jint mode);
    void (JNICALL *ReleaseCharArrayElements)(JNIEnv *env,
                                             jcharArray array,
                                             jchar *elems, jint mode);
    void (JNICALL *ReleaseShortArrayElements)(JNIEnv *env,
                                              jshortArray array,
                                              jshort *elems, jint mode);
    void (JNICALL *ReleaseIntArrayElements)(JNIEnv *env,
                                            jintArray array,
                                            jint *elems, jint mode);
    void (JNICALL *ReleaseLongArrayElements)(JNIEnv *env,
                                             jlongArray array,
                                             jlong *elems, jint mode);
    void (JNICALL *ReleaseFloatArrayElements)(JNIEnv *env,
                                              jfloatArray array,
                                              jfloat *elems, jint mode);
    void (JNICALL *ReleaseDoubleArrayElements)(JNIEnv *env,
                                               jdoubleArray array,
                                               jdouble *elems, jint mode);

    void (JNICALL *GetBooleanArrayRegion)(JNIEnv *env,
                                          jbooleanArray array,
                                          jsize start, jsize len,
                                          jboolean *buff);
    void (JNICALL *GetByteArrayRegion)(JNIEnv *env,
                                       jbyteArray array,
                                       jsize start, jsize len,
                                       jbyte *buff);
    void (JNICALL *GetCharArrayRegion)(JNIEnv *env,
                                       jcharArray array,
                                       jsize start, jsize len,
                                       jchar *buff);
    void (JNICALL *GetShortArrayRegion)(JNIEnv *env,
                                        jshortArray array,
                                        jsize start, jsize len,
                                        jshort *buff);
    void (JNICALL *GetIntArrayRegion)(JNIEnv *env,
                                      jintArray array,
                                      jsize start, jsize len,
                                      jint *buff);
    void (JNICALL *GetLongArrayRegion)(JNIEnv *env,
                                       jlongArray array,
                                       jsize start, jsize len,
                                       jlong *buff);
    void (JNICALL *GetFloatArrayRegion)(JNIEnv *env,
                                        jfloatArray array,
                                        jsize start, jsize len,
                                        jfloat *buff);
    void (JNICALL *GetDoubleArrayRegion)(JNIEnv *env,
                                         jdoubleArray array,
                                         jsize start, jsize len,
                                         jdouble *buff);

    void (JNICALL *SetBooleanArrayRegion)(JNIEnv *env,
                                          jbooleanArray array,
                                          jsize start, jsize len,
                                          jboolean *buff);
    void (JNICALL *SetByteArrayRegion)(JNIEnv *env,
                                       jbyteArray array,
                                       jsize start, jsize len,
                                       jbyte *buff);
    void (JNICALL *SetCharArrayRegion)(JNIEnv *env,
                                       jcharArray array,
                                       jsize start, jsize len,
                                       jchar *buff);
    void (JNICALL *SetShortArrayRegion)(JNIEnv *env,
                                        jshortArray array,
                                        jsize start, jsize len,
                                        jshort *buff);
    void (JNICALL *SetIntArrayRegion)(JNIEnv *env,
                                      jintArray array,
                                      jsize start, jsize len,
                                      jint *buff);
    void (JNICALL *SetLongArrayRegion)(JNIEnv *env,
                                       jlongArray array,
                                       jsize start, jsize len,
                                       jlong *buff);
    void (JNICALL *SetFloatArrayRegion)(JNIEnv *env,
                                        jfloatArray array,
                                        jsize start, jsize len,
                                        jfloat *buff);
    void (JNICALL *SetDoubleArrayRegion)(JNIEnv *env,
                                         jdoubleArray array,
                                         jsize start, jsize len,
                                         jdouble *buff);

    jint (JNICALL *RegisterNatives)(JNIEnv *env, jclass clazz,
                                    const JNINativeMethod *methods,
                                    jint nMethods);
    jint (JNICALL *UnregisterNatives)(JNIEnv *env, jclass clazz);

    jint (JNICALL *MonitorEnter)(JNIEnv *env, jobject obj);
    jint (JNICALL *MonitorExit)(JNIEnv *env, jobject obj);

    jint (JNICALL *GetJavaVM)(JNIEnv *env, JavaVM **vm);
};

/* And another really big table follows! */
#ifdef __cplusplus
struct JNIEnv {
    JNINativeInterface *fnTbl;

    jint GetVersion() {
        return fnTbl->GetVersion(this);
    }

    jclass DefineClass(jobject loader, const jbyte *buff, jsize buffLen) {
        return fnTbl->DefineClass(this, loader, buff, buffLen);
    }
    jclass FindClass(const char *name) {
        return fnTbl->FindClass(this, name);
    }

    jclass GetSuperclass(jclass clazz) {
        return fnTbl->GetSuperclass(this, clazz);
    }
    jboolean IsAssignableFrom(jclass clazza, jclass clazzb) {
        return fnTbl->IsAssignableFrom(this, clazza, clazzb);
    }

    jint Throw(jthrowable obj) {
        return fnTbl->Throw(this, obj);
    }
    jint ThrowNew(jclass clazz, const char *msg) {
        return fnTbl->ThrowNew(this, clazz, msg);
    }
    jthrowable ExceptionOccurred() {
        return fnTbl->ExceptionOccurred(this);
    }
    void ExceptionDescribe() {
        return fnTbl->ExceptionDescribe(this);
    }
    void ExceptionClear() {
        return fnTbl->ExceptionClear(this);
    }
    void FatalError(const char *msg) {
        return fnTbl->FatalError(this, msg);
    }

    jobject NewGlobalRef(jobject obj) {
        return fnTbl->NewGlobalRef(this, obj);
    }
    void DeleteGlobalRef(jobject globalRef) {
        return fnTbl->DeleteGlobalRef(this, globalRef);
    }
    void DeleteLocalRef(jobject localRef) {
        return fnTbl->DeleteLocalRef(this, localRef);
    }
    jboolean IsSameObject(jobject obja, jobject objb) {
        return fnTbl->IsSameObject(this, obja, objb);
    }

    jobject AllocObject(jclass clazz) {
        return fnTbl->AllocObject(this, clazz);
    }
    jobject NewObject(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jobject retVal;

        va_start(argList, methodID);
        retVal = fnTbl->NewObjectV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jobject NewObjectV(jclass clazz, jmethodID methodID, va_list args) {
        return fnTbl->NewObjectV(this, clazz, methodID, args);
    }
    jobject NewObjectA(jclass clazz, jmethodID methodID, jvalue *args) {
        return fnTbl->NewObjectA(this, clazz, methodID, args);
    }

    jclass GetObjectClass(jobject obj) {
        return fnTbl->GetObjectClass(this, obj);
    }
    jboolean IsInstanceOf(jobject obj, jclass clazz) {
        return fnTbl->IsInstanceOf(this, obj, clazz);
    }

    jmethodID GetMethodID(jclass clazz, const char *name, const char *sig) {
        return fnTbl->GetMethodID(this, clazz, name, sig);
    }

    jobject CallObjectMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jobject retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallObjectMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jobject CallObjectMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallObjectMethodV(this, obj, methodID, args);
    }
    jobject CallObjectMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallObjectMethodA(this, obj, methodID, args);
    }

    jboolean CallBooleanMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jboolean retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallBooleanMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jboolean CallBooleanMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallBooleanMethodV(this, obj, methodID, args);
    }
    jboolean CallBooleanMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallBooleanMethodA(this, obj, methodID, args);
    }

    jbyte CallByteMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jbyte retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallByteMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jbyte CallByteMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallByteMethodV(this, obj, methodID, args);
    }
    jbyte CallByteMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallByteMethodA(this, obj, methodID, args);
    }

    jchar CallCharMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jchar retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallCharMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jchar CallCharMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallCharMethodV(this, obj, methodID, args);
    }
    jchar CallCharMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallCharMethodA(this, obj, methodID, args);
    }

    jshort CallShortMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jshort retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallShortMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jshort CallShortMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallShortMethodV(this, obj, methodID, args);
    }
    jshort CallShortMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallShortMethodA(this, obj, methodID, args);
    }

    jint CallIntMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jint retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallIntMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jint CallIntMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallIntMethodV(this, obj, methodID, args);
    }
    jint CallIntMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallIntMethodA(this, obj, methodID, args);
    }

    jlong CallLongMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jlong retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallLongMethod(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jlong CallLongMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallLongMethodV(this, obj, methodID, args);
    }
    jlong CallLongMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallLongMethodA(this, obj, methodID, args);
    }

    jfloat CallFloatMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jfloat retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallFloatMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jfloat CallFloatMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallFloatMethodV(this, obj, methodID, args);
    }
    jfloat CallFloatMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallFloatMethodA(this, obj, methodID, args);
    }

    jdouble CallDoubleMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;
        jdouble retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallDoubleMethodV(this, obj, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jdouble CallDoubleMethodV(jobject obj, jmethodID methodID, va_list args) {
        return fnTbl->CallDoubleMethodV(this, obj, methodID, args);
    }
    jdouble CallDoubleMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        return fnTbl->CallDoubleMethodA(this, obj, methodID, args);
    }

    void CallVoidMethod(jobject obj, jmethodID methodID, ...) {
        va_list argList;

        va_start(argList, methodID);
        fnTbl->CallVoidMethodV(this, obj, methodID, argList);
        va_end(argList);
    }
    void CallVoidMethodV(jobject obj, jmethodID methodID, va_list args) {
        fnTbl->CallVoidMethodV(this, obj, methodID, args);
    }
    void CallVoidMethodA(jobject obj, jmethodID methodID, jvalue *args) {
        fnTbl->CallVoidMethodA(this, obj, methodID, args);
    }

    jobject CallNonvirtualObjectMethod(jobject obj, jclass clazz,
                                       jmethodID methodID, ...) {
        va_list argList;
        jobject retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualObjectMethodV(this, obj, clazz,
                                                    methodID, argList);
        va_end(argList);

        return retVal;
    }
    jobject CallNonvirtualObjectMethodV(jobject obj, jclass clazz,
                                        jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualObjectMethodV(this, obj, clazz,
                                                  methodID, args);
    }
    jobject CallNonvirtualObjectMethodA(jobject obj, jclass clazz,
                                        jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualObjectMethodA(this, obj, clazz,
                                                  methodID, args);
    }

    jboolean CallNonvirtualBooleanMethod(jobject obj, jclass clazz,
                                         jmethodID methodID, ...) {
        va_list argList;
        jboolean retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualBooleanMethodV(this, obj, clazz,
                                                     methodID, argList);
        va_end(argList);

        return retVal;
    }
    jboolean CallNonvirtualBooleanMethodV(jobject obj, jclass clazz,
                                          jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualBooleanMethodV(this, obj, clazz,
                                                   methodID, args);
    }
    jboolean CallNonvirtualBooleanMethodA(jobject obj, jclass clazz,
                                          jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualBooleanMethodA(this, obj, clazz,
                                                   methodID, args);
    }

    jbyte CallNonvirtualByteMethod(jobject obj, jclass clazz,
                                   jmethodID methodID, ...) {
        va_list argList;
        jbyte retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualByteMethodV(this, obj, clazz,
                                                  methodID, argList);
        va_end(argList);

        return retVal;
    }
    jbyte CallNonvirtualByteMethodV(jobject obj, jclass clazz,
                                    jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualByteMethodV(this, obj, clazz,
                                                methodID, args);
    }
    jbyte CallNonvirtualByteMethodA(jobject obj, jclass clazz,
                                    jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualByteMethodA(this, obj, clazz,
                                                methodID, args);
    }

    jchar CallNonvirtualCharMethod(jobject obj, jclass clazz,
                                   jmethodID methodID, ...) {
        va_list argList;
        jchar retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualCharMethodV(this, obj, clazz,
                                                  methodID, argList);
        va_end(argList);

        return retVal;
    }
    jchar CallNonvirtualCharMethodV(jobject obj, jclass clazz,
                                    jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualCharMethodV(this, obj, clazz,
                                                methodID, args);
    }
    jchar CallNonvirtualCharMethodA(jobject obj, jclass clazz,
                                    jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualCharMethodA(this, obj, clazz,
                                                methodID, args);
    }

    jshort CallNonvirtualShortMethod(jobject obj, jclass clazz,
                                     jmethodID methodID, ...) {
        va_list argList;
        jshort retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualShortMethodV(this, obj, clazz,
                                                   methodID, argList);
        va_end(argList);

        return retVal;
    }
    jshort CallNonvirtualShortMethodV(jobject obj, jclass clazz,
                                      jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualShortMethodV(this, obj, clazz,
                                                 methodID, args);
    }
    jshort CallNonvirtualShortMethodA(jobject obj, jclass clazz,
                                      jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualShortMethodA(this, obj, clazz,
                                                 methodID, args);
    }

    jint CallNonvirtualIntMethod(jobject obj, jclass clazz,
                                 jmethodID methodID, ...) {
        va_list argList;
        jint retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualIntMethodV(this, obj, clazz,
                                                 methodID, argList);
        va_end(argList);

        return retVal;
    }
    jint CallNonvirtualIntMethodV(jobject obj, jclass clazz,
                                  jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualIntMethodV(this, obj, clazz,
                                               methodID, args);
    }
    jint CallNonvirtualIntMethodA(jobject obj, jclass clazz,
                                  jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualIntMethodA(this, obj, clazz,
                                               methodID, args);
    }

    jlong CallNonvirtualLongMethod(jobject obj, jclass clazz,
                                   jmethodID methodID, ...) {
        va_list argList;
        jlong retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualLongMethodV(this, obj, clazz,
                                                  methodID, argList);
        va_end(argList);

        return retVal;
    }
    jlong CallNonvirtualLongMethodV(jobject obj, jclass clazz,
                                    jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualLongMethodV(this, obj, clazz,
                                                methodID, args);
    }
    jlong CallNonvirtualLongMethodA(jobject obj, jclass clazz,
                                    jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualLongMethodA(this, obj, clazz,
                                                methodID, args);
    }

    jfloat CallNonvirtualFloatMethod(jobject obj, jclass clazz,
                                     jmethodID methodID, ...) {
        va_list argList;
        jfloat retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualFloatMethodV(this, obj, clazz,
                                                   methodID, argList);
        va_end(argList);

        return retVal;
    }
    jfloat CallNonvirtualFloatMethodV(jobject obj, jclass clazz,
                                      jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualFloatMethodV(this, obj, clazz,
                                                 methodID, args);
    }
    jfloat CallNonvirtualFloatMethodA(jobject obj, jclass clazz,
                                      jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualFloatMethodA(this, obj, clazz,
                                                 methodID, args);
    }

    jdouble CallNonvirtualDoubleMethod(jobject obj, jclass clazz,
                                       jmethodID methodID, ...) {
        va_list argList;
        jdouble retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallNonvirtualDoubleMethodV(this, obj, clazz,
                                                    methodID, argList);
        va_end(argList);

        return retVal;
    }
    jdouble CallNonvirtualDoubleMethodV(jobject obj, jclass clazz,
                                        jmethodID methodID, va_list args) {
        return fnTbl->CallNonvirtualDoubleMethodV(this, obj, clazz,
                                                  methodID, args);
    }
    jdouble CallNonvirtualDoubleMethodA(jobject obj, jclass clazz,
                                        jmethodID methodID, jvalue *args) {
        return fnTbl->CallNonvirtualDoubleMethodA(this, obj, clazz,
                                                  methodID, args);
    }

    void CallNonvirtualVoidMethod(jobject obj, jclass clazz,
                                  jmethodID methodID, ...) {
        va_list argList;

        va_start(argList, methodID);
        fnTbl->CallNonvirtualVoidMethodV(this, obj, clazz, methodID, argList);
        va_end(argList);
    }
    void CallNonvirtualVoidMethodV(jobject obj, jclass clazz,
                                   jmethodID methodID, va_list args) {
        fnTbl->CallNonvirtualVoidMethodV(this, obj, clazz, methodID, args);
    }
    void CallNonvirtualVoidMethodA(jobject obj, jclass clazz,
                                   jmethodID methodID, jvalue *args) {
        fnTbl->CallNonvirtualVoidMethodA(this, obj, clazz, methodID, args);
    }

    jfieldID GetFieldID(jclass clazz, const char *name, const char *sig) {
        return fnTbl->GetFieldID(this, clazz, name, sig);
    }

    jobject GetObjectField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetObjectField(this, obj, fieldID);
    }
    jboolean GetBooleanField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetBooleanField(this, obj, fieldID);
    }
    jbyte GetByteField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetByteField(this, obj, fieldID);
    }
    jchar GetCharField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetCharField(this, obj, fieldID);
    }
    jshort GetShortField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetShortField(this, obj, fieldID);
    }
    jint GetIntField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetIntField(this, obj, fieldID);
    }
    jlong GetLongField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetLongField(this, obj, fieldID);
    }
    jfloat GetFloatField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetFloatField(this, obj, fieldID);
    }
    jdouble GetDoubleField(jobject obj, jfieldID fieldID) {
        return fnTbl->GetDoubleField(this, obj, fieldID);
    }

    void SetObjectField(jobject obj, jfieldID fieldID, jobject val) {
        return fnTbl->SetObjectField(this, obj, fieldID, val);
    }
    void SetBooleanField(jobject obj, jfieldID fieldID, jboolean val) {
        return fnTbl->SetBooleanField(this, obj, fieldID, val);
    }
    void SetByteField(jobject obj, jfieldID fieldID, jbyte val) {
        return fnTbl->SetByteField(this, obj, fieldID, val);
    }
    void SetCharField(jobject obj, jfieldID fieldID, jchar val) {
        return fnTbl->SetCharField(this, obj, fieldID, val);
    }
    void SetShortField(jobject obj, jfieldID fieldID, jshort val) {
        return fnTbl->SetShortField(this, obj, fieldID, val);
    }
    void SetIntField(jobject obj, jfieldID fieldID, jint val) {
        return fnTbl->SetIntField(this, obj, fieldID, val);
    }
    void SetLongField(jobject obj, jfieldID fieldID, jlong val) {
        return fnTbl->SetLongField(this, obj, fieldID, val);
    }
    void SetFloatField(jobject obj, jfieldID fieldID, jfloat val) {
        return fnTbl->SetFloatField(this, obj, fieldID, val);
    }
    void SetDoubleField(jobject obj, jfieldID fieldID, jdouble val) {
        return fnTbl->SetDoubleField(this, obj, fieldID, val);
    }

    jmethodID GetStaticMethodID(jclass clazz, const char *name,
                                const char *sig) {
        return fnTbl->GetStaticMethodID(this, clazz, name, sig);
    }

    jobject CallStaticObjectMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jobject retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticObjectMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jobject CallStaticObjectMethodV(jclass clazz, jmethodID methodID,
                                    va_list args) {
        return fnTbl->CallStaticObjectMethodV(this, clazz, methodID, args);
    }
    jobject CallStaticObjectMethodA(jclass clazz, jmethodID methodID,
                                    jvalue *args) {
        return fnTbl->CallStaticObjectMethodA(this, clazz, methodID, args);
    }

    jboolean CallStaticBooleanMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jboolean retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticBooleanMethodV(this, clazz, methodID, 
                                                 argList);
        va_end(argList);

        return retVal;
    }
    jboolean CallStaticBooleanMethodV(jclass clazz, jmethodID methodID,
                                      va_list args) {
        return fnTbl->CallStaticBooleanMethodV(this, clazz, methodID, args);
    }
    jboolean CallStaticBooleanMethodA(jclass clazz, jmethodID methodID,
                                      jvalue *args) {
        return fnTbl->CallStaticBooleanMethodA(this, clazz, methodID, args);
    }

    jbyte CallStaticByteMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jbyte retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticByteMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jbyte CallStaticByteMethodV(jclass clazz, jmethodID methodID,
                                va_list args) {
        return fnTbl->CallStaticByteMethodV(this, clazz, methodID, args);
    }
    jbyte CallStaticByteMethodA(jclass clazz, jmethodID methodID,
                                jvalue *args) {
        return fnTbl->CallStaticByteMethodA(this, clazz, methodID, args);
    }

    jchar CallStaticCharMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jchar retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticCharMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jchar CallStaticCharMethodV(jclass clazz, jmethodID methodID,
                                va_list args) {
        return fnTbl->CallStaticCharMethodV(this, clazz, methodID, args);
    }
    jchar CallStaticCharMethodA(jclass clazz, jmethodID methodID,
                                jvalue *args) {
        return fnTbl->CallStaticCharMethodA(this, clazz, methodID, args);
    }

    jshort CallStaticShortMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jshort retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticShortMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jshort CallStaticShortMethodV(jclass clazz, jmethodID methodID,
                                  va_list args) {
        return fnTbl->CallStaticShortMethodV(this, clazz, methodID, args);
    }
    jshort CallStaticShortMethodA(jclass clazz, jmethodID methodID,
                                  jvalue *args) {
        return fnTbl->CallStaticShortMethodA(this, clazz, methodID, args);
    }

    jint CallStaticIntMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jint retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticIntMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jint CallStaticIntMethodV(jclass clazz, jmethodID methodID,
                              va_list args) {
        return fnTbl->CallStaticIntMethodV(this, clazz, methodID, args);
    }
    jint CallStaticIntMethodA(jclass clazz, jmethodID methodID,
                              jvalue *args) {
        return fnTbl->CallStaticIntMethodA(this, clazz, methodID, args);
    }

    jlong CallStaticLongMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jlong retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticLongMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jlong CallStaticLongMethodV(jclass clazz, jmethodID methodID,
                                va_list args) {
        return fnTbl->CallStaticLongMethodV(this, clazz, methodID, args);
    }
    jlong CallStaticLongMethodA(jclass clazz, jmethodID methodID,
                                jvalue *args) {
        return fnTbl->CallStaticLongMethodA(this, clazz, methodID, args);
    }

    jfloat CallStaticFloatMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jfloat retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticFloatMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jfloat CallStaticFloatMethodV(jclass clazz, jmethodID methodID,
                                  va_list args) {
        return fnTbl->CallStaticFloatMethodV(this, clazz, methodID, args);
    }
    jfloat CallStaticFloatMethodA(jclass clazz, jmethodID methodID,
                                  jvalue *args) {
        return fnTbl->CallStaticFloatMethodA(this, clazz, methodID, args);
    }

    jdouble CallStaticDoubleMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;
        jdouble retVal;

        va_start(argList, methodID);
        retVal = fnTbl->CallStaticDoubleMethodV(this, clazz, methodID, argList);
        va_end(argList);

        return retVal;
    }
    jdouble CallStaticDoubleMethodV(jclass clazz, jmethodID methodID,
                                    va_list args) {
        return fnTbl->CallStaticDoubleMethodV(this, clazz, methodID, args);
    }
    jdouble CallStaticDoubleMethodA(jclass clazz, jmethodID methodID,
                                    jvalue *args) {
        return fnTbl->CallStaticDoubleMethodA(this, clazz, methodID, args);
    }

    void CallStaticVoidMethod(jclass clazz, jmethodID methodID, ...) {
        va_list argList;

        va_start(argList, methodID);
        fnTbl->CallStaticVoidMethodV(this, clazz, methodID, argList);
        va_end(argList);
    }
    void CallStaticVoidMethodV(jclass clazz, jmethodID methodID,
                               va_list args) {
        return fnTbl->CallStaticVoidMethodV(this, clazz, methodID, args);
    }
    void CallStaticVoidMethodA(jclass clazz, jmethodID methodID,
                               jvalue *args) {
        return fnTbl->CallStaticVoidMethodA(this, clazz, methodID, args);
    }

    jfieldID GetStaticFieldID(jclass clazz, const char *name,
                              const char *sig) {
        return fnTbl->GetStaticFieldID(this, clazz, name, sig);
    }

    jobject GetStaticObjectField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticObjectField(this, clazz, fieldID);
    }
    jboolean GetStaticBooleanField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticBooleanField(this, clazz, fieldID);
    }
    jbyte GetStaticByteField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticByteField(this, clazz, fieldID);
    }
    jchar GetStaticCharField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticCharField(this, clazz, fieldID);
    }
    jshort GetStaticShortField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticShortField(this, clazz, fieldID);
    }
    jint GetStaticIntField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticIntField(this, clazz, fieldID);
    }
    jlong GetStaticLongField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticLongField(this, clazz, fieldID);
    }
    jfloat GetStaticFloatField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticFloatField(this, clazz, fieldID);
    }
    jdouble GetStaticDoubleField(jclass clazz, jfieldID fieldID) {
        return fnTbl->GetStaticDoubleField(this, clazz, fieldID);
    }

    void SetStaticObjectField(jclass clazz, jfieldID fieldID, jobject val) {
        return fnTbl->SetStaticObjectField(this, clazz, fieldID, val);
    }
    void SetStaticBooleanField(jclass clazz, jfieldID fieldID, jboolean val) {
        return fnTbl->SetStaticBooleanField(this, clazz, fieldID, val);
    }
    void SetStaticByteField(jclass clazz, jfieldID fieldID, jbyte val) {
        return fnTbl->SetStaticByteField(this, clazz, fieldID, val);
    }
    void SetStaticCharField(jclass clazz, jfieldID fieldID, jchar val) {
        return fnTbl->SetStaticCharField(this, clazz, fieldID, val);
    }
    void SetStaticShortField(jclass clazz, jfieldID fieldID, jshort val) {
        return fnTbl->SetStaticShortField(this, clazz, fieldID, val);
    }
    void SetStaticIntField(jclass clazz, jfieldID fieldID, jint val) {
        return fnTbl->SetStaticIntField(this, clazz, fieldID, val);
    }
    void SetStaticLongField(jclass clazz, jfieldID fieldID, jlong val) {
        return fnTbl->SetStaticLongField(this, clazz, fieldID, val);
    }
    void SetStaticFloatField(jclass clazz, jfieldID fieldID, jfloat val) {
        return fnTbl->SetStaticFloatField(this, clazz, fieldID, val);
    }
    void SetStaticDoubleField(jclass clazz, jfieldID fieldID, jdouble val) {
        return fnTbl->SetStaticDoubleField(this, clazz, fieldID, val);
    }

    jstring NewString(const jchar *unicode, jsize len) {
        return fnTbl->NewString(this, unicode, len);
    }
    jsize GetStringLength(jstring str) {
        return fnTbl->GetStringLength(this, str);
    }
    const jchar *GetStringChars(jstring str, jboolean *isCopy) {
        return fnTbl->GetStringChars(this, str, isCopy);
    }
    void ReleaseStringChars(jstring str, const jchar *chars) {
        return fnTbl->ReleaseStringChars(this, str, chars);
    }

    jstring NewStringUTF(const char *utf) {
        return fnTbl->NewStringUTF(this, utf);
    }
    jsize GetStringUTFLength(jstring str) {
        return fnTbl->GetStringUTFLength(this, str);
    }
    const char *GetStringUTFChars(jstring str, jboolean *isCopy) {
        return fnTbl->GetStringUTFChars(this, str, isCopy);
    }
    void ReleaseStringUTFChars(jstring str, const char *chars) {
        return fnTbl->ReleaseStringUTFChars(this, str, chars);
    }

    jsize GetArrayLength(jarray array) {
        return fnTbl->GetArrayLength(this, array);
    }

    jobjectArray NewObjectArray(jsize len, jclass clazz, jobject init) {
        return fnTbl->NewObjectArray(this, len, clazz, init);
    }
    jobject GetObjectArrayElement(jobjectArray array, jsize index) {
        return fnTbl->GetObjectArrayElement(this, array, index);
    }
    void SetObjectArrayElement(jobjectArray array, jsize index, jobject val) {
        return fnTbl->SetObjectArrayElement(this, array, index, val);
    }

    jbooleanArray NewBooleanArray(jsize len) {
        return fnTbl->NewBooleanArray(this, len);
    }
    jbyteArray NewByteArray(jsize len) {
        return fnTbl->NewByteArray(this, len);
    }
    jcharArray NewCharArray(jsize len) {
        return fnTbl->NewCharArray(this, len);
    }
    jshortArray NewShortArray(jsize len) {
        return fnTbl->NewShortArray(this, len);
    }
    jintArray NewIntArray(jsize len) {
        return fnTbl->NewIntArray(this, len);
    }
    jlongArray NewLongArray(jsize len) {
        return fnTbl->NewLongArray(this, len);
    }
    jfloatArray NewFloatArray(jsize len) {
        return fnTbl->NewFloatArray(this, len);
    }
    jdoubleArray NewDoubleArray(jsize len) {
        return fnTbl->NewDoubleArray(this, len);
    }

    jboolean *GetBooleanArrayElements(jbooleanArray array, jboolean *isCopy) {
        return fnTbl->GetBooleanArrayElements(this, array, isCopy);
    }
    jbyte *GetByteArrayElements(jbyteArray array, jboolean *isCopy) {
        return fnTbl->GetByteArrayElements(this, array, isCopy);
    }
    jchar *GetCharArrayElements(jcharArray array, jboolean *isCopy) {
        return fnTbl->GetCharArrayElements(this, array, isCopy);
    }
    jshort *GetShortArrayElements(jshortArray array, jboolean *isCopy) {
        return fnTbl->GetShortArrayElements(this, array, isCopy);
    }
    jint *GetIntArrayElements(jintArray array, jboolean *isCopy) {
        return fnTbl->GetIntArrayElements(this, array, isCopy);
    }
    jlong *GetLongArrayElements(jlongArray array, jboolean *isCopy) {
        return fnTbl->GetLongArrayElements(this, array, isCopy);
    }
    jfloat *GetFloatArrayElements(jfloatArray array, jboolean *isCopy) {
        return fnTbl->GetFloatArrayElements(this, array, isCopy);
    }
    jdouble *GetDoubleArrayElements(jdoubleArray array, jboolean *isCopy) {
        return fnTbl->GetDoubleArrayElements(this, array, isCopy);
    }

    void ReleaseBooleanArrayElements(jbooleanArray array, jboolean *elems,
                                     jint mode) {
        return fnTbl->ReleaseBooleanArrayElements(this, array, elems, mode);
    }
    void ReleaseByteArrayElements(jbyteArray array, jbyte *elems,
                                  jint mode) {
        return fnTbl->ReleaseByteArrayElements(this, array, elems, mode);
    }
    void ReleaseCharArrayElements(jcharArray array, jchar *elems,
                                  jint mode) {
        return fnTbl->ReleaseCharArrayElements(this, array, elems, mode);
    }
    void ReleaseShortArrayElements(jshortArray array, jshort *elems,
                                   jint mode) {
        return fnTbl->ReleaseShortArrayElements(this, array, elems, mode);
    }
    void ReleaseIntArrayElements(jintArray array, jint *elems,
                                 jint mode) {
        return fnTbl->ReleaseIntArrayElements(this, array, elems, mode);
    }
    void ReleaseLongArrayElements(jlongArray array, jlong *elems,
                                  jint mode) {
        return fnTbl->ReleaseLongArrayElements(this, array, elems, mode);
    }
    void ReleaseFloatArrayElements(jfloatArray array, jfloat *elems,
                                   jint mode) {
        return fnTbl->ReleaseFloatArrayElements(this, array, elems, mode);
    }
    void ReleaseDoubleArrayElements(jdoubleArray array, jdouble *elems,
                                    jint mode) {
        return fnTbl->ReleaseDoubleArrayElements(this, array, elems, mode);
    }

    void GetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len,
                               jboolean *buff) {
        return fnTbl->GetBooleanArrayRegion(this, array, start, len, buff);
    }
    void GetByteArrayRegion(jbyteArray array, jsize start, jsize len,
                            jbyte *buff) {
        return fnTbl->GetByteArrayRegion(this, array, start, len, buff);
    }
    void GetCharArrayRegion(jcharArray array, jsize start, jsize len,
                            jchar *buff) {
        return fnTbl->GetCharArrayRegion(this, array, start, len, buff);
    }
    void GetShortArrayRegion(jshortArray array, jsize start, jsize len,
                             jshort *buff) {
        return fnTbl->GetShortArrayRegion(this, array, start, len, buff);
    }
    void GetIntArrayRegion(jintArray array, jsize start, jsize len,
                           jint *buff) {
        return fnTbl->GetIntArrayRegion(this, array, start, len, buff);
    }
    void GetLongArrayRegion(jlongArray array, jsize start, jsize len,
                            jlong *buff) {
        return fnTbl->GetLongArrayRegion(this, array, start, len, buff);
    }
    void GetFloatArrayRegion(jfloatArray array, jsize start, jsize len,
                             jfloat *buff) {
        return fnTbl->GetFloatArrayRegion(this, array, start, len, buff);
    }
    void GetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len,
                              jdouble *buff) {
        return fnTbl->GetDoubleArrayRegion(this, array, start, len, buff);
    }

    void SetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len,
                               jboolean *buff) {
        return fnTbl->SetBooleanArrayRegion(this, array, start, len, buff);
    }
    void SetByteArrayRegion(jbyteArray array, jsize start, jsize len,
                            jbyte *buff) {
        return fnTbl->SetByteArrayRegion(this, array, start, len, buff);
    }
    void SetCharArrayRegion(jcharArray array, jsize start, jsize len,
                            jchar *buff) {
        return fnTbl->SetCharArrayRegion(this, array, start, len, buff);
    }
    void SetShortArrayRegion(jshortArray array, jsize start, jsize len,
                             jshort *buff) {
        return fnTbl->SetShortArrayRegion(this, array, start, len, buff);
    }
    void SetIntArrayRegion(jintArray array, jsize start, jsize len,
                           jint *buff) {
        return fnTbl->SetIntArrayRegion(this, array, start, len, buff);
    }
    void SetLongArrayRegion(jlongArray array, jsize start, jsize len,
                            jlong *buff) {
        return fnTbl->SetLongArrayRegion(this, array, start, len, buff);
    }
    void SetFloatArrayRegion(jfloatArray array, jsize start, jsize len,
                             jfloat *buff) {
        return fnTbl->SetFloatArrayRegion(this, array, start, len, buff);
    }
    void SetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len,
                              jdouble *buff) {
        return fnTbl->SetDoubleArrayRegion(this, array, start, len, buff);
    }

    jint RegisterNatives(jclass clazz, const JNINativeMethod *methods,
                         jint nMethods) {
        return fnTbl->RegisterNatives(this, clazz, methods, nMethods);
    }
    jint UnregisterNatives(jclass clazz) {
        return fnTbl->UnregisterNatives(this, clazz);
    }

    jint MonitorEnter(jobject obj) {
        return fnTbl->MonitorEnter(this, obj);
    }
    jint MonitorExit(jobject obj) {
        return fnTbl->MonitorExit(this, obj);
    }

    jint GetJavaVM(JavaVM **vm) {
        return fnTbl->GetJavaVM(this, vm);
    }
};
#endif

#ifdef __cplusplus
}
#endif

#endif
