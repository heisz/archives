/**
 * JEMCC methods to support the JNI array interface methods.
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

/**
 * Convenience method to validate the accessibility of the array member
 * defined by the given index.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     array - the array instance to validate against
 *     index - the array index to validate
 *     excIdx - the VM exception index to throw (sometimes IndexOutOfBounds,
 *              sometimes ArrayIndexOutOfBounds).  If negative, the
 *              ArrayIndexOutOfBounds exception will be used by default.
 *
 * Returns:
 *     JNI_OK if the array index is valid, JNI_ERR otherwise (the
 *     indicated exception will be thrown in the current environment).
 *
 * Exceptions:
 *     As determined by the excIdx value.
 */
jint JEMCC_CheckArrayLimits(JNIEnv *env, JEMCC_ArrayObject *array, 
                            jsize index, jint excIdx) {
    char msg[128];

    if ((index < 0) || (index >= array->arrayLength)) {
        (void) sprintf(msg, "Array index out of range: %i", index);
        if (excIdx < 0) excIdx = JEMCC_Class_ArrayIndexOutOfBoundsException;
        JEMCC_ThrowStdThrowableIdx(env, excIdx, NULL, msg);
        return JNI_ERR;
    }

    return JNI_OK;
}

/**
 * Convenience method to validate the accessibility of the array members
 * in the region offset -> offset + len - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     array - the array instance to validate against
 *     offset - the array index of the first member in the region
 *     len - the number of array members in the region
 *     excIdx - the VM exception index to throw (sometimes IndexOutOfBounds,
 *              sometimes ArrayIndexOutOfBounds).  If negative, the
 *              ArrayIndexOutOfBounds exception will be used by default.
 *
 * Returns:
 *     JNI_OK if the array region is valid, JNI_ERR otherwise (the
 *     indicated exception will be thrown in the current environment).
 *
 * Exceptions:
 *     As determined by the excIdx value.
 */
jint JEMCC_CheckArrayRegion(JNIEnv *env, JEMCC_ArrayObject *array,
                            jsize offset, jsize len, jint excIdx) {
    char msg[128];

    if ((offset < 0) || (len < 0) || ((offset + len) > array->arrayLength)) {
        (void) sprintf(msg, "Array region out of range: offset %i, length %i", 
                            offset, len);
        if (excIdx < 0) excIdx = JEMCC_Class_ArrayIndexOutOfBoundsException;
        JEMCC_ThrowStdThrowableIdx(env, excIdx, NULL, msg);
        return JNI_ERR;
    }

    return JNI_OK;
}

/**
 * Convenience method to validate the accessibility of the array members
 * in the segment from start to end - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     array - the array instance to validate against
 *     start - the index of the first array member
 *     end - the index of the last array member plus one
 *     excIdx - the VM exception index to throw (sometimes IndexOutOfBounds,
 *              sometimes ArrayIndexOutOfBounds).  If negative, the
 *              ArrayIndexOutOfBounds exception will be used by default.
 *
 * Returns:
 *     JNI_OK if the array index is valid, JNI_ERR otherwise (the
 *     indicated exception will be thrown in the current environment).
 *
 * Exceptions:
 *     As determined by the excIdx value.
 */
jint JEMCC_CheckArraySegment(JNIEnv *env, JEMCC_ArrayObject *array, 
                             jsize start, jsize end, jint excIdx) {
    char msg[128];

    if ((start < 0) || (start >= array->arrayLength) ||
          (end < 0) || (end > array->arrayLength) || (start > end)) {
        (void) sprintf(msg, "Array segment out of range: start %i, end %i", 
                            start, end);
        if (excIdx < 0) excIdx = JEMCC_Class_ArrayIndexOutOfBoundsException;
        JEMCC_ThrowStdThrowableIdx(env, excIdx, NULL, msg);
        return JNI_ERR;
    }

    return JNI_OK;
}

/* TODO - comments need updates from here onward */

/**
 * Obtain the length of the provided array instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     array - the array instance to obtain the length of
 *
 * Returns:
 *     The length (total number of elements) of the provided array.
 *
 * Exceptions:
 *     NullPointerException - the provided array instance was NULL
 */
jsize JEMCC_GetArrayLength(JNIEnv *env, jarray array) {
    /* TODO - capture NULL reference and return value is? */
    return (jsize) ((JEMCC_ArrayObject *) array)->arrayLength;
}

jobjectArray JEMCC_NewObjectArray(JNIEnv *env, jsize len, jclass clazz, 
                                  jobject init) {
    JEMCC_ArrayObject *retArray;
    JEMCC_Class *arrayClass;
    JEM_ClassData *compClData;
    char *arrayClName;
    jint i, rc;

    /* Construct the target array classname */
    if (JEMCC_EnvStrBufferInit(env, 100) == NULL) return NULL;
    compClData = ((JEMCC_Class *) clazz)->classData;
    if ((compClData->accessFlags & ACC_ARRAY) != 0) {
        arrayClName = JEMCC_EnvStrBufferAppendSet(env, "[", 
                                                  compClData->className,
                                                  (char *) NULL);
    } else {
        arrayClName = JEMCC_EnvStrBufferAppendSet(env, "[L", 
                                                  compClData->className,
                                                  ";", (char *) NULL);
    }
    if (arrayClName == NULL) return NULL;

    /* Get the array class instance */
    rc = JEMCC_LocateClass(env, compClData->classLoader, arrayClName,
                           JNI_FALSE, &arrayClass);
    if (rc != JNI_OK) return NULL;

    /* Build the array instance */
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObject(env, arrayClass,
                                                 len * sizeof(JEMCC_Object *));

    /* Store the length and initialize the array */
    retArray->arrayLength = len;
    for (i = 0; i < len; i++) {
        *(((JEMCC_Object **) retArray->arrayData) + i) = 
                                               (JEMCC_Object *) init;
    }

    return (jobjectArray) retArray;
}

jobject JEMCC_GetObjectArrayElement(JNIEnv *env, jobjectArray array, 
                                    jsize index) {
    JEMCC_ArrayObject *arrayObj = (JEMCC_ArrayObject *) array;

    if (JEMCC_CheckArrayLimits(env, arrayObj, index, -1) != JNI_OK) {
        return NULL;
    }
    return (jobject) *(((JEMCC_Object **) arrayObj->arrayData) + index);
}

void JEMCC_SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, 
                                 jobject val) {
    JEMCC_ArrayObject *arrayObj = (JEMCC_ArrayObject *) array;

    if (JEMCC_CheckArrayLimits(env, arrayObj, index, -1) != JNI_OK) return;
    /* TODO - validate proper assignment details */
    *(((JEMCC_Object **) arrayObj->arrayData) + index) = (JEMCC_Object *) val;
}

jbooleanArray JEMCC_NewBooleanArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                       JEMCC_Array_Boolean,
                                                       len * sizeof(jboolean));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jbooleanArray) retArray;
}

jbyteArray JEMCC_NewByteArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                        JEMCC_Array_Byte,
                                                        len * sizeof(jbyte));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jbyteArray) retArray;
}

jcharArray JEMCC_NewCharArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                         JEMCC_Array_Char,
                                                         len * sizeof(jchar));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jcharArray) retArray;
}

jshortArray JEMCC_NewShortArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                        JEMCC_Array_Short,
                                                        len * sizeof(jshort));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jshortArray) retArray;
}

jintArray JEMCC_NewIntArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                         JEMCC_Array_Int,
                                                         len * sizeof(jint));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jintArray) retArray;
}

jlongArray JEMCC_NewLongArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                          JEMCC_Array_Long,
                                                          len * sizeof(jlong));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jlongArray) retArray;
}

jfloatArray JEMCC_NewFloatArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                         JEMCC_Array_Float,
                                                         len * sizeof(jfloat));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jfloatArray) retArray;
}

jdoubleArray JEMCC_NewDoubleArray(JNIEnv *env, jsize len) {
    JEMCC_ArrayObject *retArray;

    if (len < 0) len = 0;
    retArray = (JEMCC_ArrayObject *) JEMCC_AllocateObjectIdx(env, 
                                                       JEMCC_Array_Double,
                                                       len * sizeof(jdouble));
    if (retArray != NULL) retArray->arrayLength = len;

    return (jdoubleArray) retArray;
}

jboolean *JEMCC_GetBooleanArrayElements(JNIEnv *env, jbooleanArray array,
                                        jboolean *isCopy) {
    return NULL; /* TODO */
}

jbyte *JEMCC_GetByteArrayElements(JNIEnv *env, jbyteArray array, 
                                  jboolean *isCopy) {
    return NULL; /* TODO */
}

jchar *JEMCC_GetCharArrayElements(JNIEnv *env, jcharArray array, 
                                  jboolean *isCopy) {
    return NULL; /* TODO */
}

jshort *JEMCC_GetShortArrayElements(JNIEnv *env, jshortArray array, 
                                    jboolean *isCopy) {
    return NULL; /* TODO */
}

jint *JEMCC_GetIntArrayElements(JNIEnv *env, jintArray array, 
                                jboolean *isCopy) {
    return NULL; /* TODO */
}

jlong *JEMCC_GetLongArrayElements(JNIEnv *env, jlongArray array, 
                                  jboolean *isCopy) {
    return NULL; /* TODO */
}

jfloat *JEMCC_GetFloatArrayElements(JNIEnv *env, jfloatArray array,
                                    jboolean *isCopy) {
    return NULL; /* TODO */
}

jdouble *JEMCC_GetDoubleArrayElements(JNIEnv *env, jdoubleArray array,
                                      jboolean *isCopy) {
    return NULL; /* TODO */
}

void JEMCC_ReleaseBooleanArrayElements(JNIEnv *env, jbooleanArray array,
                                       jboolean *elems, jint mode) {
    /* TODO */
}

void JEMCC_ReleaseByteArrayElements(JNIEnv *env, jbyteArray array,
                                    jbyte *elems, jint mode) {
    /* TODO */
}

void JEMCC_ReleaseCharArrayElements(JNIEnv *env, jcharArray array,
                                    jchar *elems, jint mode) {
    /* TODO */
}

void JEMCC_ReleaseShortArrayElements(JNIEnv *env, jshortArray array,
                                     jshort *elems, jint mode) {
    /* TODO */
}

void JEMCC_ReleaseIntArrayElements(JNIEnv *env, jintArray array,
                                   jint *elems, jint mode) {
    /* TODO */
}

void JEMCC_ReleaseLongArrayElements(JNIEnv *env, jlongArray array,
                                    jlong *elems, jint mode) {
    /* TODO */
}

void JEMCC_ReleaseFloatArrayElements(JNIEnv *env, jfloatArray array,
                                     jfloat *elems, jint mode) {
    /* TODO */
}

void JEMCC_ReleaseDoubleArrayElements(JNIEnv *env, jdoubleArray array,
                                      jdouble *elems, jint mode) {
    /* TODO */
}

void JEMCC_GetBooleanArrayRegion(JNIEnv *env, jbooleanArray array,
                                 jsize start, jsize len, jboolean *buff) {
    /* TODO */
}

void JEMCC_GetByteArrayRegion(JNIEnv *env, jbyteArray array,
                              jsize start, jsize len, jbyte *buff) {
    /* TODO */
}

void JEMCC_GetCharArrayRegion(JNIEnv *env, jcharArray array,
                              jsize start, jsize len, jchar *buff) {
    /* TODO */
}

void JEMCC_GetShortArrayRegion(JNIEnv *env, jshortArray array,
                               jsize start, jsize len, jshort *buff) {
    /* TODO */
}

void JEMCC_GetIntArrayRegion(JNIEnv *env, jintArray array,
                             jsize start, jsize len, jint *buff) {
    /* TODO */
}

void JEMCC_GetLongArrayRegion(JNIEnv *env, jlongArray array,
                              jsize start, jsize len, jlong *buff) {
    /* TODO */
}

void JEMCC_GetFloatArrayRegion(JNIEnv *env, jfloatArray array,
                               jsize start, jsize len, jfloat *buff) {
    /* TODO */
}

void JEMCC_GetDoubleArrayRegion(JNIEnv *env, jdoubleArray array,
                                jsize start, jsize len, jdouble *buff) {
    /* TODO */
}

void JEMCC_SetBooleanArrayRegion(JNIEnv *env, jbooleanArray array,
                                 jsize start, jsize len, jboolean *buff) {
    /* TODO */
}

void JEMCC_SetByteArrayRegion(JNIEnv *env, jbyteArray array,
                              jsize start, jsize len, jbyte *buff) {
    /* TODO */
}

void JEMCC_SetCharArrayRegion(JNIEnv *env, jcharArray array,
                              jsize start, jsize len, jchar *buff) {
    /* TODO */
}

void JEMCC_SetShortArrayRegion(JNIEnv *env, jshortArray array,
                               jsize start, jsize len, jshort *buff) {
    /* TODO */
}

void JEMCC_SetIntArrayRegion(JNIEnv *env, jintArray array,
                             jsize start, jsize len, jint *buff) {
    /* TODO */
}

void JEMCC_SetLongArrayRegion(JNIEnv *env, jlongArray array,
                              jsize start, jsize len, jlong *buff) {
    /* TODO */
}

void JEMCC_SetFloatArrayRegion(JNIEnv *env, jfloatArray array,
                               jsize start, jsize len, jfloat *buff) {
    /* TODO */
}

void JEMCC_SetDoubleArrayRegion(JNIEnv *env, jdoubleArray array,
                                jsize start, jsize len, jdouble *buff) {
    /* TODO */
}
