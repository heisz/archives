/**
 * JEMCC definitions of the java.lang.String class.
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
#include "numerics.h"

/**
 * Structure used to contain StringBuffer working data (in the object
 * attachment record).  The capacity reflects the number of ASCII or
 * Unicode characters available in the buffer.  If the capacity value
 * is negative, the buffer is currently being shared with a String.
 * The length indicates the number of characters currently in the buffer,
 * according to the StringData convention (negative indicates pure ASCII
 * data in the buffer).
 */
typedef struct StringBufferData {
    jsize capacity;

    union {
        JEMCC_ObjectExt *strInst;
        JEMCC_StringData *strData;
    } buffer;
} StringBufferData;

static jint JEMCC_String_init(JNIEnv *env,
                              JEMCC_VMFrame *frame,
                              JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);

    /* Allocate a zero length string */
    thisObj->objectData = JEMCC_Malloc(env, sizeof(JEMCC_StringData));
    if (thisObj->objectData == NULL) return JEMCC_ERR;
    ((JEMCC_StringData *) thisObj->objectData)->length = 0;

    return JEMCC_RET_VOID;
}

static jint JEMCC_String_init_String(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ObjectExt *strObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    JEMCC_StringData *strData;
    juint strDataLen;

    /* Duplicate the string internals */
    if (strObj == NULL) return JEMCC_NULL_EXCEPTION;
    strData = (JEMCC_StringData *) strObj->objectData;
    if (strData->length < 0) {
        strDataLen = sizeof(JEMCC_StringData) - strData->length;
    } else {
        strDataLen = sizeof(JEMCC_StringData) + 2 * strData->length - 1;
    }
    thisObj->objectData = JEMCC_Malloc(env, strDataLen);
    if (thisObj->objectData == NULL) return JEMCC_ERR;
    (void) memcpy(thisObj->objectData, strObj->objectData, strDataLen);

    return JEMCC_RET_VOID;
}

static jint JEMCC_String_init_StringBuffer(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_ByteArray(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_ByteArrayI(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_ByteArrayII(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_ByteArrayIII(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_ByteArrayIIString(JNIEnv *env,
                                                JEMCC_VMFrame *frame,
                                                JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_ByteArrayString(JNIEnv *env,
                                              JEMCC_VMFrame *frame,
                                              JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_CharArray(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_init_CharArrayII(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_charAt_I(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jint index = JEMCC_LOAD_INT(frame, 1);

    /* Make sure the character is valid */
    if (JEMCC_CheckStringLimits(env, strData, index) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Pull the character as requested */
    if (strData->length < 0) {
        retVal->intVal = (int) *(&(strData->data) + index);
    } else {
        retVal->intVal = (int) *(((jchar *) &(strData->data)) + index);
    }

    return JEMCC_RET_INT;
}

static jint JEMCC_String_compareTo_String(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_concat_String(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_copyValueOf_CharArray(JNIEnv *env,
                                               JEMCC_VMFrame *frame,
                                               JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_copyValueOf_CharArrayII(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_endsWith_String(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_equals_Object(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ObjectExt *compObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    JEMCC_StringData *thisStrData, *compStrData;

    /* Quick tests - null check, same object, same class, same length */
    if (compObj == NULL) {
        retVal->intVal = JNI_FALSE;
        return JEMCC_RET_INT;
    }
    if (thisObj == compObj) {
        retVal->intVal = JNI_TRUE;
        return JEMCC_RET_INT;
    }
    if (thisObj->classReference != compObj->classReference) {
        retVal->intVal = JNI_FALSE;
        return JEMCC_RET_INT;
    }
    thisStrData = (JEMCC_StringData *) thisObj->objectData;
    compStrData = (JEMCC_StringData *) compObj->objectData;

#ifdef ENABLE_ERRORSWEEP
    /* Special test case, ensure that ASCII string data ends with '\0' */
    if (thisStrData->length < 0) {
        if (*(&(thisStrData->data) - thisStrData->length) != '\0') abort();
    }
    if (compStrData->length < 0) {
        if (*(&(compStrData->data) - compStrData->length) != '\0') abort();
    }
#endif
    retVal->intVal = JEMCC_StringEqualsFn(env, thisStrData, compStrData);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_equalsIgnoreCase_String(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_getBytes(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_getBytes_IIByteArrayI(JNIEnv *env,
                                               JEMCC_VMFrame *frame,
                                               JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_getBytes_String(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_getChars_IICharArrayI(JNIEnv *env,
                                               JEMCC_VMFrame *frame,
                                               JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jint srcBegin = JEMCC_LOAD_INT(frame, 1), srcEnd = JEMCC_LOAD_INT(frame, 2);
    JEMCC_ArrayObject *arr = (JEMCC_ArrayObject *) JEMCC_LOAD_OBJECT(frame, 3);
    jint dstBegin = JEMCC_LOAD_INT(frame, 4);

    /* Check the string and array limits */
    if (JEMCC_CheckStringSegment(env, strData, srcBegin, srcEnd) != JNI_OK) {
        return JEMCC_ERR;
    }
    if (arr == NULL) return JEMCC_NULL_EXCEPTION;
    if (JEMCC_CheckArrayRegion(env, arr, dstBegin, srcEnd - srcBegin,
                       JEMCC_Class_ArrayIndexOutOfBoundsException) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Finally, do the extract */
    JEMCC_StringGetChars(env, strData, srcBegin, srcEnd,
                         ((jchar *) (arr->arrayData)) + dstBegin);

    return JEMCC_RET_VOID;
}

static jint JEMCC_String_hashCode(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_indexOf_I(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jchar jch = (jchar) JEMCC_LOAD_INT(frame, 1);

    /* Find it and return the result */
    retVal->intVal = JEMCC_StringChar(env, strData, jch, 0);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_indexOf_II(JNIEnv *env,
                                    JEMCC_VMFrame *frame,
                                    JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jchar jch = (jchar) JEMCC_LOAD_INT(frame, 1);
    jint fromIdx = JEMCC_LOAD_INT(frame, 2);

    /* Find it and return the result */
    retVal->intVal = JEMCC_StringChar(env, strData, jch, fromIdx);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_indexOf_String(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    JEMCC_ObjectExt *strObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);

    /* Find it and return the result */
    if (strObj == NULL) return JEMCC_NULL_EXCEPTION;
    retVal->intVal = JEMCC_StringString(env, strData,
                                        (JEMCC_StringData *) strObj->objectData,
                                        0);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_indexOf_StringI(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    JEMCC_ObjectExt *strObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    jint fromIdx = JEMCC_LOAD_INT(frame, 2);

    /* Find it and return the result */
    if (strObj == NULL) return JEMCC_NULL_EXCEPTION;
    retVal->intVal = JEMCC_StringString(env, strData,
                                        (JEMCC_StringData *) strObj->objectData,
                                        fromIdx);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_intern(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_lastIndexOf_I(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jchar jch = (jchar) JEMCC_LOAD_INT(frame, 1);

    /* Find it and return the result */
    retVal->intVal = JEMCC_LastStringChar(env, strData, jch, 2147483647);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_lastIndexOf_II(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jchar jch = (jchar) JEMCC_LOAD_INT(frame, 1);
    jint fromIdx = JEMCC_LOAD_INT(frame, 2);

    /* Find it and return the result */
    retVal->intVal = JEMCC_LastStringChar(env, strData, jch, fromIdx);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_lastIndexOf_String(JNIEnv *env,
                                            JEMCC_VMFrame *frame,
                                            JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    JEMCC_ObjectExt *strObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);

    /* Find it and return the result */
    if (strObj == NULL) return JEMCC_NULL_EXCEPTION;
    retVal->intVal = JEMCC_LastStringString(env, strData,
                                        (JEMCC_StringData *) strObj->objectData,
                                        2147483647);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_lastIndexOf_StringI(JNIEnv *env,
                                             JEMCC_VMFrame *frame,
                                             JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    JEMCC_ObjectExt *strObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    jint fromIdx = JEMCC_LOAD_INT(frame, 2);

    /* Find it and return the result */
    if (strObj == NULL) return JEMCC_NULL_EXCEPTION;
    retVal->intVal = JEMCC_LastStringString(env, strData,
                                        (JEMCC_StringData *) strObj->objectData,
                                        fromIdx);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_length(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);

    /* Pretty simple, this one is */
    retVal->intVal = abs(((JEMCC_StringData *) thisObj->objectData)->length);
    return JEMCC_RET_INT;
}

static jint JEMCC_String_regionMatches_IStringII(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_regionMatches_ZIStringII(JNIEnv *env,
                                                  JEMCC_VMFrame *frame,
                                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_replace_CC(JNIEnv *env,
                                    JEMCC_VMFrame *frame,
                                    JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_startsWith_String(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_startsWith_StringI(JNIEnv *env,
                                            JEMCC_VMFrame *frame,
                                            JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_substring_I(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jint start = JEMCC_LOAD_INT(frame, 1);

    /* Make sure the start point is valid */
    if (JEMCC_CheckStringLimits(env, strData, start) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Extract the substring to the end of the string */
    retVal->objVal = JEMCC_StringSubstring(env, strData, start,
                                           abs(strData->length));

    return ((retVal->objVal == NULL) ? JEMCC_ERR : JEMCC_RET_OBJECT);
}

static jint JEMCC_String_substring_II(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jint start = JEMCC_LOAD_INT(frame, 1), end = JEMCC_LOAD_INT(frame, 2);

    /* Make sure the subregion is valid */
    if (JEMCC_CheckStringSegment(env, strData, start, end) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Extract the substring as requested */
    retVal->objVal = JEMCC_StringSubstring(env, strData, start, end);

    return ((retVal->objVal == NULL) ? JEMCC_ERR : JEMCC_RET_OBJECT);
}

static jint JEMCC_String_toCharArray(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_StringData *strData = (JEMCC_StringData *) thisObj->objectData;
    jint len = abs(strData->length);
    JEMCC_ArrayObject *array;

    /* Allocate and fill an appropriate cast of characters */
    array = (JEMCC_ArrayObject *) JEMCC_NewCharArray(env, len);
    if (array == NULL) return JEMCC_ERR;
    JEMCC_StringGetChars(env, strData, 0, len, (jchar *) array->arrayData);

    retVal->objVal = (JEMCC_Object *) array;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_String_toLowerCase(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_toLowerCase_Locale(JNIEnv *env,
                                            JEMCC_VMFrame *frame,
                                            JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_toString(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    retVal->objVal = JEMCC_LOAD_OBJECT(frame, 0);
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_String_toUpperCase(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_toUpperCase_Locale(JNIEnv *env,
                                            JEMCC_VMFrame *frame,
                                            JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_trim(JNIEnv *env,
                              JEMCC_VMFrame *frame,
                              JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_C(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_D(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_F(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_I(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_J(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_Object(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_Z(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_CharArray(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_String_valueOf_CharArrayII(JNIEnv *env,
                                             JEMCC_VMFrame *frame,
                                             JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

JEMCC_MethodData JEMCC_StringMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_String_init },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/String;)V",
         JEMCC_String_init_String },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/StringBuffer;)V",
         JEMCC_String_init_StringBuffer },
    { ACC_PUBLIC,
         "<init>", "([B)V",
         JEMCC_String_init_ByteArray },
    { ACC_PUBLIC,
         "<init>", "([BI)V",
         JEMCC_String_init_ByteArrayI },
    { ACC_PUBLIC,
         "<init>", "([BII)V",
         JEMCC_String_init_ByteArrayII },
    { ACC_PUBLIC,
         "<init>", "([BIII)V",
         JEMCC_String_init_ByteArrayIII },
    { ACC_PUBLIC,
         "<init>", "([BIILjava/lang/String;)V",
         JEMCC_String_init_ByteArrayIIString },
    { ACC_PUBLIC,
         "<init>", "([BLjava/lang/String;)V",
         JEMCC_String_init_ByteArrayString },
    { ACC_PUBLIC,
         "<init>", "([C)V",
         JEMCC_String_init_CharArray },
    { ACC_PUBLIC,
         "<init>", "([CII)V",
         JEMCC_String_init_CharArrayII },
    { ACC_PUBLIC,
         "charAt", "(I)C",
         JEMCC_String_charAt_I },
    { ACC_PUBLIC,
         "compareTo", "(Ljava/lang/String;)I",
         JEMCC_String_compareTo_String },
    { ACC_PUBLIC,
         "concat", "(Ljava/lang/String;)Ljava/lang/String;",
         JEMCC_String_concat_String },
    { ACC_PUBLIC | ACC_STATIC,
         "copyValueOf", "([C)Ljava/lang/String;",
         JEMCC_String_copyValueOf_CharArray },
    { ACC_PUBLIC | ACC_STATIC,
         "copyValueOf", "([CII)Ljava/lang/String;",
         JEMCC_String_copyValueOf_CharArrayII },
    { ACC_PUBLIC,
         "endsWith", "(Ljava/lang/String;)Z",
         JEMCC_String_endsWith_String },
    { ACC_PUBLIC,
         "equals", "(Ljava/lang/Object;)Z",
         JEMCC_String_equals_Object },
    { ACC_PUBLIC,
         "equalsIgnoreCase", "(Ljava/lang/String;)Z",
         JEMCC_String_equalsIgnoreCase_String },
    { ACC_PUBLIC,
         "getBytes", "()[B",
         JEMCC_String_getBytes },
    { ACC_PUBLIC,
         "getBytes", "(II[BI)V",
         JEMCC_String_getBytes_IIByteArrayI },
    { ACC_PUBLIC,
         "getBytes", "(Ljava/lang/String;)[B",
         JEMCC_String_getBytes_String },
    { ACC_PUBLIC,
         "getChars", "(II[CI)V",
         JEMCC_String_getChars_IICharArrayI },
    { ACC_PUBLIC,
         "hashCode", "()I",
         JEMCC_String_hashCode },
    { ACC_PUBLIC,
         "indexOf", "(I)I",
         JEMCC_String_indexOf_I },
    { ACC_PUBLIC,
         "indexOf", "(II)I",
         JEMCC_String_indexOf_II },
    { ACC_PUBLIC,
         "indexOf", "(Ljava/lang/String;)I",
         JEMCC_String_indexOf_String },
    { ACC_PUBLIC,
         "indexOf", "(Ljava/lang/String;I)I",
         JEMCC_String_indexOf_StringI },
    { ACC_PUBLIC,
         "intern", "()Ljava/lang/String;",
         JEMCC_String_intern },
    { ACC_PUBLIC,
         "lastIndexOf", "(I)I",
         JEMCC_String_lastIndexOf_I },
    { ACC_PUBLIC,
         "lastIndexOf", "(II)I",
         JEMCC_String_lastIndexOf_II },
    { ACC_PUBLIC,
         "lastIndexOf", "(Ljava/lang/String;)I",
         JEMCC_String_lastIndexOf_String },
    { ACC_PUBLIC,
         "lastIndexOf", "(Ljava/lang/String;I)I",
         JEMCC_String_lastIndexOf_StringI },
    { ACC_PUBLIC,
         "length", "()I",
         JEMCC_String_length },
    { ACC_PUBLIC,
         "regionMatches", "(ILjava/lang/String;II)Z",
         JEMCC_String_regionMatches_IStringII },
    { ACC_PUBLIC,
         "regionMatches", "(ZILjava/lang/String;II)Z",
         JEMCC_String_regionMatches_ZIStringII },
    { ACC_PUBLIC,
         "replace", "(CC)Ljava/lang/String;",
         JEMCC_String_replace_CC },
    { ACC_PUBLIC,
         "startsWith", "(Ljava/lang/String;)Z",
         JEMCC_String_startsWith_String },
    { ACC_PUBLIC,
         "startsWith", "(Ljava/lang/String;I)Z",
         JEMCC_String_startsWith_StringI },
    { ACC_PUBLIC,
         "substring", "(I)Ljava/lang/String;",
         JEMCC_String_substring_I },
    { ACC_PUBLIC,
         "substring", "(II)Ljava/lang/String;",
         JEMCC_String_substring_II },
    { ACC_PUBLIC,
         "toCharArray", "()[C",
         JEMCC_String_toCharArray },
    { ACC_PUBLIC,
         "toLowerCase", "()Ljava/lang/String;",
         JEMCC_String_toLowerCase },
    { ACC_PUBLIC,
         "toLowerCase", "(Ljava/util/Locale;)Ljava/lang/String;",
         JEMCC_String_toLowerCase_Locale },
    { ACC_PUBLIC,
         "toString", "()Ljava/lang/String;",
         JEMCC_String_toString },
    { ACC_PUBLIC,
         "toUpperCase", "()Ljava/lang/String;",
         JEMCC_String_toUpperCase },
    { ACC_PUBLIC,
         "toUpperCase", "(Ljava/util/Locale;)Ljava/lang/String;",
         JEMCC_String_toUpperCase_Locale },
    { ACC_PUBLIC,
         "trim", "()Ljava/lang/String;",
         JEMCC_String_trim },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "(C)Ljava/lang/String;",
         JEMCC_String_valueOf_C },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "(D)Ljava/lang/String;",
         JEMCC_String_valueOf_D },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "(F)Ljava/lang/String;",
         JEMCC_String_valueOf_F },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "(I)Ljava/lang/String;",
         JEMCC_String_valueOf_I },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "(J)Ljava/lang/String;",
         JEMCC_String_valueOf_J },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "(Ljava/lang/Object;)Ljava/lang/String;",
         JEMCC_String_valueOf_Object },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "(Z)Ljava/lang/String;",
         JEMCC_String_valueOf_Z },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "([C)Ljava/lang/String;",
         JEMCC_String_valueOf_CharArray },
    { ACC_PUBLIC | ACC_STATIC,
         "valueOf", "([CII)Ljava/lang/String;",
         JEMCC_String_valueOf_CharArrayII }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC | ACC_FINAL,
                     "java.lang.String",
                     NULL ** java/lang/Object **,
                     interfaces ** java/io/Serializable,  **, 1,
                     JEMCC_StringMethods, 57, NULL,
                     JEMCC_StringFields, 1,
                     NULL, 0, NULL, classInstance);
*/

/* Include the stringbuffer class definitions directly to share code */
#include "stringbuffer.c"
