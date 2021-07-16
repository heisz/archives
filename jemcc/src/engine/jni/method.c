/**
 * JEMCC methods to support the JNI method interfaces.
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

/* Push and initialize a new method frame using the variable argument list */
static JEMCC_VMFrame *pushRunMethodFrameV(JNIEnv *env, jclass clazz, 
                                          jobject obj, jmethodID methodID, 
                                          va_list args) {
    JEM_ClassMethodData *methodData = (JEM_ClassMethodData *) methodID;
    JEM_DescriptorData *argDesc;
    JEMCC_VMFrame *frame;
    int index;

    /* Create our operating frame */
    if (JEM_PushFrame(env, methodID, &frame) != JNI_OK) return NULL;

    /* Push the variable argument list onto the stack */
    index = 0;
    if (obj != NULL) JEMCC_STORE_OBJECT(frame, index++, obj);
    argDesc = methodData->descriptor->method_info.paramDescriptor;
    while (argDesc->generic.tag != DESCRIPTOR_EndOfList) {
        switch (argDesc->generic.tag) {
            case BASETYPE_Boolean:
                JEMCC_STORE_INT(frame, index++, 
                                (jint) va_arg(args, jint /* jboolean */));
                break;
            case BASETYPE_Byte:
                JEMCC_STORE_INT(frame, index++, 
                                (jint) va_arg(args, jint /* jbyte */));
                break;
            case BASETYPE_Char:
                JEMCC_STORE_INT(frame, index++, 
                                (jint) va_arg(args, jint /* jchar */));
                break;
            case BASETYPE_Short:
                JEMCC_STORE_INT(frame, index++, 
                                (jint) va_arg(args, jint /* jshort */));
                break;
            case BASETYPE_Int:
                JEMCC_STORE_INT(frame, index++, 
                                (jint) va_arg(args, jint));
                break;
            case BASETYPE_Float:
                JEMCC_STORE_FLOAT(frame, index++, 
                                  (jfloat) va_arg(args, jdouble /* jfloat */));
                break;
            case BASETYPE_Long:
                JEMCC_STORE_LONG(frame, index, 
                                 (jlong) va_arg(args, jlong));
                index += 2;
                break;
            case BASETYPE_Double:
                JEMCC_STORE_DOUBLE(frame, index, 
                                   (jdouble) va_arg(args, jdouble));
                index += 2;
                break;
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                JEMCC_STORE_OBJECT(frame, index++, 
                                   (jobject) va_arg(args, jobject));
                break;
        }
        argDesc++;
    }

    /* Execute the method for this frame */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);

    return frame;
}

/* Push and initialize a new method frame using the argument array ptr */
static JEMCC_VMFrame *pushRunMethodFrameA(JNIEnv *env, jclass clazz, 
                                          jobject obj, jmethodID methodID, 
                                          jvalue *args) {
    JEM_ClassMethodData *methodData = (JEM_ClassMethodData *) methodID;
    JEM_DescriptorData *argDesc;
    JEMCC_VMFrame *frame;
    int index;

    /* Create our operating frame */
    if (JEM_PushFrame(env, methodID, &frame) != JNI_OK) return NULL;

    /* Push the variable argument list onto the stack */
    index = 0;
    if (obj != NULL) JEMCC_STORE_OBJECT(frame, index++, obj);
    argDesc = methodData->descriptor->method_info.paramDescriptor;
    while (argDesc->generic.tag != DESCRIPTOR_EndOfList) {
        switch (argDesc->generic.tag) {
            case BASETYPE_Boolean:
                JEMCC_STORE_INT(frame, index++, (jint) args->z);
                break;
            case BASETYPE_Byte:
                JEMCC_STORE_INT(frame, index++, (jint) args->b);
                break;
            case BASETYPE_Char:
                JEMCC_STORE_INT(frame, index++, (jint) args->c);
                break;
            case BASETYPE_Short:
                JEMCC_STORE_INT(frame, index++, (jint) args->s);
                break;
            case BASETYPE_Int:
                JEMCC_STORE_INT(frame, index++, args->i);
                break;
            case BASETYPE_Float:
                JEMCC_STORE_FLOAT(frame, index++, args->f);
                break;
            case BASETYPE_Long:
                JEMCC_STORE_LONG(frame, index, args->j);
                index += 2;
                break;
            case BASETYPE_Double:
                JEMCC_STORE_DOUBLE(frame, index, args->d);
                index += 2;
                break;
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                JEMCC_STORE_OBJECT(frame, index++, args->l);
                break;
        }
        argDesc++;
        args++;
    }

    /* Execute the method for this frame */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);

    return frame;
}

jmethodID JEMCC_GetMethodID(JNIEnv *env, jclass clazz, const char *name, 
                            const char *sig) {
    return NULL; /* TODO */
}

jobject JEMCC_CallObjectMethodV(JNIEnv *env, jobject obj,
                                jmethodID methodID, va_list args) {
    return NULL; /* TODO */
}

jobject JEMCC_CallObjectMethodA(JNIEnv *env, jobject obj,
                                jmethodID methodID, jvalue *args) {
    return NULL; /* TODO */
}

jboolean JEMCC_CallBooleanMethodV(JNIEnv *env, jobject obj,
                                  jmethodID methodID, va_list args) {
    return JNI_FALSE; /* TODO */
}

jboolean JEMCC_CallBooleanMethodA(JNIEnv *env, jobject obj,
                                  jmethodID methodID, jvalue *args) {
    return JNI_FALSE; /* TODO */
}

jbyte JEMCC_CallByteMethodV(JNIEnv *env, jobject obj,
                            jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jbyte JEMCC_CallByteMethodA(JNIEnv *env, jobject obj,
                            jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jchar JEMCC_CallCharMethodV(JNIEnv *env, jobject obj,
                            jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jchar JEMCC_CallCharMethodA(JNIEnv *env, jobject obj,
                            jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jshort JEMCC_CallShortMethodV(JNIEnv *env, jobject obj,
                              jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jshort JEMCC_CallShortMethodA(JNIEnv *env, jobject obj,
                              jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jint JEMCC_CallIntMethodV(JNIEnv *env, jobject obj,
                          jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jint JEMCC_CallIntMethodA(JNIEnv *env, jobject obj,
                          jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jlong JEMCC_CallLongMethodV(JNIEnv *env, jobject obj,
                            jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jlong JEMCC_CallLongMethodA(JNIEnv *env, jobject obj,
                            jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jfloat JEMCC_CallFloatMethodV(JNIEnv *env, jobject obj,
                              jmethodID methodID, va_list args) {
    return 0.0; /* TODO */
}

jfloat JEMCC_CallFloatMethodA(JNIEnv *env, jobject obj,
                              jmethodID methodID, jvalue *args) {
    return 0.0; /* TODO */
}

jdouble JEMCC_CallDoubleMethodV(JNIEnv *env, jobject obj,
                                jmethodID methodID, va_list args) {
    return 0.0; /* TODO */
}

jdouble JEMCC_CallDoubleMethodA(JNIEnv *env, jobject obj,
                                jmethodID methodID, jvalue *args) {
    return 0.0; /* TODO */
}

void JEMCC_CallVoidMethodV(JNIEnv *env, jobject obj,
                           jmethodID methodID, va_list args) {
    /* TODO */
}

void JEMCC_CallVoidMethodA(JNIEnv *env, jobject obj,
                           jmethodID methodID, jvalue *args) {
    /* TODO */
}

jobject JEMCC_CallNonvirtualObjectMethodV(JNIEnv *env, jobject obj,
                                          jclass clazz, jmethodID methodID,
                                          va_list args) {
    return NULL; /* TODO */
}

jobject JEMCC_CallNonvirtualObjectMethodA(JNIEnv *env, jobject obj,
                                          jclass clazz, jmethodID methodID,
                                          jvalue *args) {
    return NULL; /* TODO */
}

jboolean JEMCC_CallNonvirtualBooleanMethodV(JNIEnv *env, jobject obj,
                                            jclass clazz, jmethodID methodID,
                                            va_list args) {
    return JNI_FALSE; /* TODO */
}

jboolean JEMCC_CallNonvirtualBooleanMethodA(JNIEnv *env, jobject obj,
                                            jclass clazz, jmethodID methodID,
                                            jvalue *args) {
    return JNI_FALSE; /* TODO */
}

jbyte JEMCC_CallNonvirtualByteMethodV(JNIEnv *env, jobject obj,
                                      jclass clazz, jmethodID methodID,
                                      va_list args) {
    return 0; /* TODO */
}

jbyte JEMCC_CallNonvirtualByteMethodA(JNIEnv *env, jobject obj,
                                      jclass clazz, jmethodID methodID,
                                      jvalue *args) {
    return 0; /* TODO */
}

jchar JEMCC_CallNonvirtualCharMethodV(JNIEnv *env, jobject obj,
                                      jclass clazz, jmethodID methodID,
                                      va_list args) {
    return 0; /* TODO */
}

jchar JEMCC_CallNonvirtualCharMethodA(JNIEnv *env, jobject obj,
                                      jclass clazz, jmethodID methodID,
                                      jvalue *args) {
    return 0; /* TODO */
}

jshort JEMCC_CallNonvirtualShortMethodV(JNIEnv *env, jobject obj,
                                       jclass clazz, jmethodID methodID,
                                       va_list args) {
    return 0; /* TODO */
}

jshort JEMCC_CallNonvirtualShortMethodA(JNIEnv *env, jobject obj,
                                       jclass clazz, jmethodID methodID,
                                       jvalue *args) {
    return 0; /* TODO */
}

jint JEMCC_CallNonvirtualIntMethodV(JNIEnv *env, jobject obj,
                                    jclass clazz, jmethodID methodID,
                                    va_list args) {
    return 0; /* TODO */
}

jint JEMCC_CallNonvirtualIntMethodA(JNIEnv *env, jobject obj,
                                    jclass clazz, jmethodID methodID,
                                    jvalue *args) {
    return 0; /* TODO */
}

jlong JEMCC_CallNonvirtualLongMethodV(JNIEnv *env, jobject obj,
                                      jclass clazz, jmethodID methodID,
                                      va_list args) {
    return 0; /* TODO */
}

jlong JEMCC_CallNonvirtualLongMethodA(JNIEnv *env, jobject obj,
                                      jclass clazz, jmethodID methodID,
                                      jvalue *args) {
    return 0; /* TODO */
}

jfloat JEMCC_CallNonvirtualFloatMethodV(JNIEnv *env, jobject obj,
                                        jclass clazz, jmethodID methodID,
                                        va_list args) {
    return 0.0; /* TODO */
}

jfloat JEMCC_CallNonvirtualFloatMethodA(JNIEnv *env, jobject obj,
                                        jclass clazz, jmethodID methodID,
                                        jvalue *args) {
    return 0.0; /* TODO */
}

jdouble JEMCC_CallNonvirtualDoubleMethodV(JNIEnv *env, jobject obj,
                                          jclass clazz, jmethodID methodID,
                                          va_list args) {
    return 0.0; /* TODO */
}

jdouble JEMCC_CallNonvirtualDoubleMethodA(JNIEnv *env, jobject obj,
                                          jclass clazz, jmethodID methodID,
                                          jvalue *args) {
    return 0.0; /* TODO */
}

void JEMCC_CallNonvirtualVoidMethodV(JNIEnv *env, jobject obj,
                                     jclass clazz, jmethodID methodID,
                                     va_list args) {
    /* TODO */
}

void JEMCC_CallNonvirtualVoidMethodA(JNIEnv *env, jobject obj,
                                     jclass clazz, jmethodID methodID,
                                     jvalue *args) {
    /* TODO */
}

jmethodID JEMCC_GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, 
                                  const char *sig) {
    JEM_ClassData *classData = ((JEMCC_Class *) clazz)->classData;
    JEM_ClassMethodData *methodData;

    /* Initialize the class first */
    /* TODO DO IT - WATCH FOR INIT ERROR */

    /* Retrieve the class method by hash key */
    methodData = JEM_LocateClassMethod(classData, name, sig);
    if (methodData == NULL) {
        /* TODO - check the descriptor and give a nice method description */
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NoSuchMethodError, 
                                   NULL, name);
        return NULL;
    }

    /* Confirm returned method is in fact static */
    if ((methodData->accessFlags & ACC_STATIC) == 0) {
        JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_NoSuchMethodError, NULL, 
                                    "JNI: Requested method is not static",
                                    name, NULL);
        return NULL;
    } 

    return (jmethodID) methodData;
}

jobject JEMCC_CallStaticObjectMethodV(JNIEnv *env, jclass clazz,
                                      jmethodID methodID, va_list args) {
    return NULL; /* TODO */
}

jobject JEMCC_CallStaticObjectMethodA(JNIEnv *env, jclass clazz,
                                      jmethodID methodID, jvalue *args) {
    return NULL; /* TODO */
}

jboolean JEMCC_CallStaticBooleanMethodV(JNIEnv *env, jclass clazz,
                                        jmethodID methodID, va_list args) {
    return JNI_FALSE; /* TODO */
}

jboolean JEMCC_CallStaticBooleanMethodA(JNIEnv *env, jclass clazz,
                                        jmethodID methodID, jvalue *args) {
    return JNI_FALSE; /* TODO */
}

jbyte JEMCC_CallStaticByteMethodV(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jbyte JEMCC_CallStaticByteMethodA(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jchar JEMCC_CallStaticCharMethodV(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jchar JEMCC_CallStaticCharMethodA(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jshort JEMCC_CallStaticShortMethodV(JNIEnv *env, jclass clazz,
                                    jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jshort JEMCC_CallStaticShortMethodA(JNIEnv *env, jclass clazz,
                                    jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jint JEMCC_CallStaticIntMethodV(JNIEnv *env, jclass clazz,
                                jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jint JEMCC_CallStaticIntMethodA(JNIEnv *env, jclass clazz,
                                jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jlong JEMCC_CallStaticLongMethodV(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, va_list args) {
    return 0; /* TODO */
}

jlong JEMCC_CallStaticLongMethodA(JNIEnv *env, jclass clazz,
                                  jmethodID methodID, jvalue *args) {
    return 0; /* TODO */
}

jfloat JEMCC_CallStaticFloatMethodV(JNIEnv *env, jclass clazz,
                                    jmethodID methodID, va_list args) {
    return 0.0; /* TODO */
}

jfloat JEMCC_CallStaticFloatMethodA(JNIEnv *env, jclass clazz,
                                    jmethodID methodID, jvalue *args) {
    return 0.0; /* TODO */
}

jdouble JEMCC_CallStaticDoubleMethodV(JNIEnv *env, jclass clazz,
                                      jmethodID methodID, va_list args) {
    return 0.0; /* TODO */
}

jdouble JEMCC_CallStaticDoubleMethodA(JNIEnv *env, jclass clazz,
                                      jmethodID methodID, jvalue *args) {
    return 0.0; /* TODO */
}

void JEMCC_CallStaticVoidMethodV(JNIEnv *env, jclass clazz,
                                 jmethodID methodID, va_list args) {
    /* Just run the method has we have no return value */
    if (JEMCC_InitializeClass(env, (JEMCC_Class *) clazz) != JNI_OK) return;
    (void) pushRunMethodFrameV(env, clazz, NULL, methodID, args);
}

void JEMCC_CallStaticVoidMethodA(JNIEnv *env, jclass clazz,
                               jmethodID methodID, jvalue *args) {
    /* TODO */
}
