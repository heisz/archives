/**
 * JEMCC definitions of the java.lang.StringBuffer class.
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

/* Note: not compiled directly, read into String for direct method access */

#define STRINGBUFFER_DEFAULT_SIZE 16

#define BUFFER_LENGTH(data) \
    (((data)->capacity < 0) ? \
        ((JEMCC_StringData *) ((data)->buffer.strInst->objectData))->length : \
        (data)->buffer.strData->length)

static jint JEMCC_StringBuffer_init(JNIEnv *env,
                                    JEMCC_VMFrame *frame,
                                    JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data;

    /* Create StringBuffer region of default initial size */
    thisObj->objectData = JEMCC_Malloc(env, sizeof(StringBufferData));
    data = (StringBufferData *) thisObj->objectData;
    if (data == NULL) return JEMCC_ERR;

    data->buffer.strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                                    sizeof(JEMCC_StringData) +
                                                    STRINGBUFFER_DEFAULT_SIZE);
    if (data->buffer.strData == NULL) return JEMCC_ERR;

    data->capacity = STRINGBUFFER_DEFAULT_SIZE;
    data->buffer.strData->length = 0;

    return JEMCC_RET_VOID;
}

static jint JEMCC_StringBuffer_init_I(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint bufferSize = JEMCC_LOAD_INT(frame, 1);
    StringBufferData *data;

    if (bufferSize < 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NegativeArraySizeException,
                               NULL, "Negative initial size for StringBuffer");
        return JEMCC_ERR;
    }

    /* Create StringBuffer region of given size */
    thisObj->objectData = JEMCC_Malloc(env, sizeof(StringBufferData));
    data = (StringBufferData *) thisObj->objectData;
    if (data == NULL) return JEMCC_ERR;

    data->buffer.strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                                    sizeof(JEMCC_StringData) +
                                                    bufferSize);
    if (data->buffer.strData == NULL) return JEMCC_ERR;

    data->capacity = bufferSize;
    data->buffer.strData->length = 0;

    return JEMCC_RET_VOID;
}

static jint JEMCC_StringBuffer_init_String(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ObjectExt *strVal = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    StringBufferData *data;
    jint strLen;

    if (strVal == NULL) return JEMCC_NULL_EXCEPTION;
    strLen = ((JEMCC_StringData *) strVal->objectData)->length;

    /* Create StringBuffer based on original String value (no copy yet) */
    thisObj->objectData = JEMCC_Malloc(env, sizeof(StringBufferData));
    data = (StringBufferData *) thisObj->objectData;
    if (data == NULL) return JEMCC_ERR;

    data->buffer.strInst = strVal;
    data->capacity = ((strLen > 0) ? -strLen : strLen) -
                             STRINGBUFFER_DEFAULT_SIZE; /* Add as per JLS */

    return JEMCC_RET_VOID;
}

/**
 * Convenience method to resize the StringBuffer contents based on a
 * new capacity requirement.  Also expands/converts the buffer contents
 * from ASCII to Unicode (where Unicode is being appended).  Returns JNI_OK 
 * or JNI_ENOMEM depending on the results of the expansion.
 *
 * NOTE: This method is not synchronized - all callers must be (most are
 *       through the method definition)
 */
static jint StringBuffer_EnsureCapacity(JNIEnv *env, StringBufferData *data,
                                        jsize requestedCapacity,
                                        jboolean expandAscii) {
    jsize newCapacity, oldCapacity = data->capacity;
    JEMCC_StringData *strData, *origStrData;
    unsigned int strDataSize, charLen = (expandAscii == JNI_TRUE) ? 2 : 1;
    jint bufferLen = BUFFER_LENGTH(data);
    jchar *jptr;
    char *ptr;

    /* StringBuffer javadoc indicates this unique sizing rule */
    newCapacity = 2 * abs(oldCapacity) + 2;
    if (newCapacity < 0) {
        /* Capacity maxed out at Integer.MAX_VALUE with StringData space */
        newCapacity = 2147483637;
    } else if (requestedCapacity > newCapacity) {
        newCapacity = requestedCapacity;
    }
    
    if ((expandAscii == JNI_TRUE) || (data->capacity < 0)) {
        /* ASCII-Unicode expansion or currently sharing dataspace */
        /* Force reallocation, resizing only if required */
        if (requestedCapacity <= oldCapacity) {
            newCapacity = oldCapacity;
        }
    } else {
        /* Do nothing if sufficient room (capacity -ve for shared copy) */
        if (requestedCapacity <= oldCapacity) return JNI_OK;

        /* Or, if nothing has changed (maximum capacity reached) */
        if (newCapacity == abs(oldCapacity)) return JNI_OK;
    }

    /* Construct the new buffer storage area */
    if (bufferLen > 0) charLen = 2;
    strDataSize = sizeof(JEMCC_StringData) + 
                                   ((unsigned int) newCapacity) * charLen;
    strData = (JEMCC_StringData *) JEMCC_Malloc(env, strDataSize);
    if (strData == NULL) return JNI_ENOMEM;

    /* Copy and release the old buffer, taking into account shared data */
    if (oldCapacity < 0) {
        origStrData = (JEMCC_StringData *) data->buffer.strInst->objectData;
        if (expandAscii == JNI_FALSE) {
            (void) memcpy(strData, origStrData, 
                          sizeof(JEMCC_StringData) + 
                                    ((unsigned int) abs(bufferLen)) * charLen);
        } else {
            ptr = (char *) &(origStrData->data);
            jptr = (jchar *) &(strData->data);
            while (*ptr != '\0') *(jptr++) = (jchar) *(ptr++);
            strData->length = -bufferLen;
        }
    } else {
        if (expandAscii == JNI_FALSE) {
            (void) memcpy(strData, data->buffer.strData,
                          sizeof(JEMCC_StringData) + 
                                    ((unsigned int) abs(bufferLen)) * charLen);
        } else {
            ptr = (char *) &(data->buffer.strData->data);
            jptr = (jchar *) &(strData->data);
            while (*ptr != '\0') *(jptr++) = (jchar) *(ptr++);
            strData->length = -bufferLen;
        }
        JEMCC_Free(data->buffer.strData);
    }
    data->buffer.strData = strData;
    data->capacity = newCapacity;

    return JNI_OK;
}

/**
 * Central method to append text to the StringBuffer.  Accepts ASCII (char *)
 * values or Unicode buffers (jchar *), based on the sign of the append
 * length.  Automatically reallocates storage and internally converts
 * from ASCII to Unicode as required.  Returns JNI_OK or JNI_ENOMEM depending
 * on the results of the expansion.
 *
 * NOTE: This method is not synchronized - all callers must be (most are
 *       through the method definition)
 */
static jint StringBuffer_Append(JNIEnv *env, StringBufferData *data,
                                char *text, jint txtLen, jboolean fromJChar) {
    jchar *jptr, *jstr = (jchar *) text;
    char *ptr, *str = text;
    jint newLen, bufferLen = BUFFER_LENGTH(data);
    jboolean expandAscii = JNI_FALSE;

    /* Quick exit */
    if (txtLen == 0) return JNI_OK;

    /* Perform the expansion (as required), with sign corrections */
    if (bufferLen <= 0) {
        newLen = -bufferLen;
        if (txtLen <= 0) {
            newLen -= txtLen;
        } else {
            newLen += txtLen;
            /* Unicode added to ASCII, need to expand */
            expandAscii = JNI_TRUE;
        }
    } else {
        newLen = bufferLen + abs(txtLen);
    }
    if (StringBuffer_EnsureCapacity(env, data, newLen, expandAscii) != JNI_OK) {
        return JNI_ENOMEM;
    }

    if (bufferLen <= 0) {
        /* ASCII, check for Unicode extension */
        if (txtLen <= 0) {
            /* Simple ASCII extension */
            ptr = &(data->buffer.strData->data) - bufferLen;
            data->buffer.strData->length = -newLen;
            if (fromJChar == JNI_FALSE) {
                while (txtLen < 0) {
                    *(ptr++) = *(str++);
                    txtLen++;
                }
            } else {
                while (txtLen < 0) {
                    *(ptr++) = (char) *(jstr++);
                    txtLen++;
                }
            }
            *ptr = '\0';
        } else {
            /* Unicode addition (conversion occurred above) */
            jptr = ((jchar *) &(data->buffer.strData->data)) - bufferLen;
            data->buffer.strData->length = newLen;
            while (txtLen > 0) {
                *(jptr++) = *(jstr++);
                txtLen--;
            }
        }
    } else {
        /* Unicode base, no conversion required */
        data->buffer.strData->length = newLen;
        if (txtLen <= 0) {
            /* Append ASCII to Unicode (convert on the fly) */
            jptr = ((jchar *) &(data->buffer.strData->data)) + bufferLen;
            if (fromJChar == JNI_FALSE) {
                while (txtLen < 0) {
                    *(jptr++) = (jchar) *(str++);
                    txtLen++;
                }
            } else {
                while (txtLen < 0) {
                    *(jptr++) = *(jstr++);
                    txtLen++;
                }
            }
        } else {
            /* Simple Unicode addition */
            jptr = ((jchar *) &(data->buffer.strData->data)) + bufferLen;
            while (txtLen > 0) {
                *(jptr++) = *(jstr++);
                txtLen--;
            }
        }
    }

    return JNI_OK;
}

static jint JEMCC_StringBuffer_append_C(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jchar jch = (jchar) JEMCC_LOAD_INT(frame, 1);
    char ch;

    if ((jch > 255) || (jch == 0)) {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                (char *) &jch, 1, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        ch = (char) jch;
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                &ch, -1, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_D(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jdouble dval = JEMCC_LOAD_DOUBLE(frame, 1);
    char dbuff[64];

    /* Convert to ASCII text and append */
    JEMCC_DoubleToText(dval, dbuff);
    if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                            dbuff, -strlen(dbuff), JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_F(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jdouble fval = JEMCC_LOAD_FLOAT(frame, 1);
    char fbuff[64];

    /* Convert to ASCII text and append */
    JEMCC_FloatToText(fval, fbuff);
    if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                            fbuff, -strlen(fbuff), JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_I(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint ival = JEMCC_LOAD_INT(frame, 1);
    char ibuff[64];

    /* Convert to ASCII text and append */
    JEMCC_IntegerToText(ival, 10, ibuff);
    if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                            ibuff, -strlen(ibuff), JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_J(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jlong lval = JEMCC_LOAD_LONG(frame, 1);
    char lbuff[64];

    /* Convert to ASCII text and append */
    JEMCC_LongToText(lval, 10, lbuff);
    if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                            lbuff, -strlen(lbuff), JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_Object(JNIEnv *env,
                                             JEMCC_VMFrame *frame,
                                             JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_Object *obj = JEMCC_LOAD_OBJECT(frame, 1);
    JEMCC_Class *objClass = VM_CLASS(JEMCC_Class_Object);
    JEMCC_ReturnValue strVal;
    JEMCC_StringData *strData;

    /* Special handling for this one! */
    if (obj == NULL) {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                "null", -4, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        /* Call the toString() method */
        JEMCC_PUSH_STACK_OBJECT(frame, obj);
        JEMCC_CHECK_METHOD_REFERENCE(env, objClass, 8,
                                     "toString", "()Ljava/lang/String;");
        if (JEMCC_ExecuteInstanceMethod(env, obj, objClass, 8, 
                                        JEMCC_VIRTUAL_METHOD, 
                                        &strVal) != JNI_OK) {
            return JEMCC_ERR;
        }

        /* Append the result */
        strData = (JEMCC_StringData *) 
                        ((JEMCC_ObjectExt *) strVal.objVal)->objectData;
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                &(strData->data), strData->length, 
                                JNI_FALSE) != JNI_OK) {
            /* TODO - GC String value */
            return JEMCC_ERR;
        }
        /* TODO - GC String value */
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_String(JNIEnv *env,
                                             JEMCC_VMFrame *frame,
                                             JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ObjectExt *strObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    JEMCC_StringData *strData;

    /* Extract the string contents and use the general append method */
    if (strObj == NULL) {
        /* Special handling for this one! */
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                "null", -4, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        strData = (JEMCC_StringData *) strObj->objectData;
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                &(strData->data), strData->length,
                                JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_Z(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint bVal = JEMCC_LOAD_INT(frame, 1);

    if (bVal == JNI_FALSE) {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                "false", -5, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                "true", -4, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_CharArray(JNIEnv *env,
                                                JEMCC_VMFrame *frame,
                                                JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ArrayObject *arr = (JEMCC_ArrayObject *) JEMCC_LOAD_OBJECT(frame, 1);
    jboolean isAscii = JNI_TRUE;
    jchar jch, *jptr;
    jint i;

    /* Make a quick exit where necessary */
    if (arr->arrayLength == 0) {
        retVal->objVal = (JEMCC_Object *) thisObj;
        return JEMCC_RET_OBJECT;
    }

    /* Determine Unicode/ASCII array status */
    for (i = 0, jptr = (jchar *) arr->arrayData; i < arr->arrayLength; i++) {
        jch = *(jptr++);
        if ((jch == 0) || (jch > 255)) {
            isAscii = JNI_FALSE;
            break;
        }
    }

    /* Make the approprate append */
    if (isAscii == JNI_FALSE) {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                (char *) arr->arrayData, arr->arrayLength, 
                                JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                (char *) arr->arrayData, -arr->arrayLength, 
                                JNI_TRUE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_append_CharArrayII(JNIEnv *env,
                                                  JEMCC_VMFrame *frame,
                                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ArrayObject *arr = (JEMCC_ArrayObject *) JEMCC_LOAD_OBJECT(frame, 1);
    jint offset = JEMCC_LOAD_INT(frame, 2), len = JEMCC_LOAD_INT(frame, 3);
    jboolean isAscii = JNI_TRUE;
    jchar jch, *jptr;
    jint i;

    /* Check region and make a quick exit where necessary */
    if (JEMCC_CheckArrayRegion(env, arr, offset, len,
                       JEMCC_Class_ArrayIndexOutOfBoundsException) != JNI_OK) {
        return JEMCC_ERR;
    }
    if (len == 0) {
        retVal->objVal = (JEMCC_Object *) thisObj;
        return JEMCC_RET_OBJECT;
    }

    /* Determine Unicode/ASCII array status */
    jptr = ((jchar *) arr->arrayData) + offset;
    for (i = 0; i < len; i++) {
        jch = *(jptr++);
        if ((jch == 0) || (jch > 255)) {
            isAscii = JNI_FALSE;
            break;
        }
    }

    /* Make the approprate append */
    jptr = ((jchar *) arr->arrayData) + offset;
    if (isAscii == JNI_FALSE) {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                (char *) jptr, len, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        if (StringBuffer_Append(env, (StringBufferData *) thisObj->objectData,
                                (char *) jptr, -len, JNI_TRUE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_capacity(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint cap = ((StringBufferData *) thisObj->objectData)->capacity;

    retVal->intVal = (cap < 0) ? -cap : cap;
    return JEMCC_RET_INT;
}

static jint JEMCC_StringBuffer_charAt_I(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    jint index = JEMCC_LOAD_INT(frame, 1);
    JEMCC_StringData *strData;

    /* Make the extract from the appropriate location */
    if (data->capacity < 0) {
        strData = (JEMCC_StringData *) data->buffer.strInst->objectData;
    } else {
        strData = data->buffer.strData;
    }

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

/**
 * Convenience method to check and convert StringBuffer contents that
 * were Unicode but are now ASCII based on an in-place update or delete 
 * operation.
 */
static void StringBuffer_CollapseIfAscii(JEMCC_StringData *strData) {
    jchar jch, *jptr = (jchar *) &(strData->data);
    jint len = strData->length;
    char *ptr;

    /* Check for remaining Unicode characters */
    while (len > 0) {
        jch = *(jptr++);
        if ((jch == 0) || (jch > 255)) return;
        len--;
    }

    /* All ASCII, reduce appropriately */
    jptr = (jchar *) &(strData->data);
    len = strData->length;
    strData->length = -len;
    ptr = (char *) jptr;
    while (len > 0) {
        *(ptr++) = (char) *(jptr++);
        len--;
    }
    *ptr = '\0';
}

static jint JEMCC_StringBuffer_ensureCapacity_I(JNIEnv *env,
                                                JEMCC_VMFrame *frame,
                                                JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    jint capacity = JEMCC_LOAD_INT(frame, 1);

    /* Just do it */
    if (StringBuffer_EnsureCapacity(env, data, capacity, JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_StringBuffer_getChars_IICharArrayI(JNIEnv *env,
                                                  JEMCC_VMFrame *frame,
                                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    jint srcBegin = JEMCC_LOAD_INT(frame, 1), srcEnd = JEMCC_LOAD_INT(frame, 2);
    JEMCC_ArrayObject *arr = (JEMCC_ArrayObject *) JEMCC_LOAD_OBJECT(frame, 3);
    jint dstBegin = JEMCC_LOAD_INT(frame, 4);
    JEMCC_StringData *strData;

    /* Make the extract from the appropriate location */
    if (data->capacity < 0) {
        strData = (JEMCC_StringData *) data->buffer.strInst->objectData;
    } else {
        strData = data->buffer.strData;
    }

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

/**
 * Central method to insert text into the StringBuffer.  Accepts ASCII (char *)
 * values or Unicode buffers (jchar *), based on the sign of the insert
 * length.  Automatically reallocates storage and internally converts
 * from ASCII to Unicode as required.  Returns JNI_OK or JNI_ENOMEM depending
 * on the results of the expansion, or JNI_ERR if the insert location was
 * invalid.
 *
 * NOTE: This method is not synchronized - all callers must be (most are
 *       through the method definition)
 */
static jint StringBuffer_Insert(JNIEnv *env, StringBufferData *data, jint index,
                                char *text, jint txtLen, jboolean fromJChar) {
    jint newLen, bufferLen = BUFFER_LENGTH(data), absLen, tailLen;
    jchar *jptr, *jstr;
    char msg[128], *ptr, *str;
    jboolean expandAscii = JNI_FALSE;

    /* Capture the quick exit cases and check the insert position */
    if (bufferLen <= 0) {
        absLen = newLen = -bufferLen;
        if (txtLen <= 0) {
            newLen -= txtLen;
        } else {
            newLen += txtLen;
            /* Unicode inserted into ASCII, need to expand */
            expandAscii = JNI_TRUE;
        }
    } else {
        absLen = bufferLen;
        newLen = bufferLen + abs(txtLen);
    }
    if ((index < 0) || (index > absLen)) {
        (void) sprintf(msg, "Insert index out of range: %i", index);
        JEMCC_ThrowStdThrowableIdx(env,
                                   JEMCC_Class_StringIndexOutOfBoundsException,
                                   NULL, msg);
        return JNI_ERR;
    }
    if (index == absLen) return StringBuffer_Append(env, data, text,
                                                    txtLen, fromJChar);
    if (txtLen == 0) return JNI_OK;
    tailLen = absLen - index;

    /* Perform the expansion (as required, with signs) and shift for insert */
    if (StringBuffer_EnsureCapacity(env, data, newLen, expandAscii) != JNI_OK) {
        return JNI_ENOMEM;
    }
    if ((bufferLen <= 0) && (expandAscii == JNI_FALSE)) {
        ptr = &(data->buffer.strData->data) + newLen;
        str = ptr - abs(txtLen);
        while (tailLen >= 0) {
            *(ptr--) = *(str--);
            tailLen--;
        }
    } else {
        jptr = ((jchar *) &(data->buffer.strData->data)) + newLen - 1;
        jstr = jptr - abs(txtLen);
        while (tailLen > 0) {
            *(jptr--) = *(jstr--);
            tailLen--;
        }
    }

    str = text;
    jstr = (jchar *) text;
    if (bufferLen <= 0) {
        /* ASCII, check for Unicode insertion */
        if (txtLen <= 0) {
            /* Simple ASCII insertion */
            ptr = &(data->buffer.strData->data) + index;
            data->buffer.strData->length = -newLen;
            if (fromJChar == JNI_FALSE) {
                while (txtLen < 0) {
                    *(ptr++) = *(str++);
                    txtLen++;
                }
            } else {
                while (txtLen < 0) {
                    *(ptr++) = (char) *(jstr++);
                    txtLen++;
                }
            }
        } else {
            /* Unicode insertion (conversion occurred above) */
            jptr = ((jchar *) &(data->buffer.strData->data)) + index;
            data->buffer.strData->length = newLen;
            while (txtLen > 0) {
                *(jptr++) = *(jstr++);
                txtLen--;
            }
        }
    } else {
        /* Unicode base, no conversion required */
        data->buffer.strData->length = newLen;
        if (txtLen <= 0) {
            /* Insert ASCII into Unicode (convert on the fly) */
            jptr = ((jchar *) &(data->buffer.strData->data)) + index;
            if (fromJChar == JNI_FALSE) {
                while (txtLen < 0) {
                    *(jptr++) = (jchar) *(str++);
                    txtLen++;
                }
            } else {
                while (txtLen < 0) {
                    *(jptr++) = *(jstr++);
                    txtLen++;
                }
            }
        } else {
            /* Simple Unicode insertion */
            jptr = ((jchar *) &(data->buffer.strData->data)) + index;
            while (txtLen > 0) {
                *(jptr++) = *(jstr++);
                txtLen--;
            }
        }
    }

    return JNI_OK;
}

static jint JEMCC_StringBuffer_insert_IC(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1);
    jchar jch = (jchar) JEMCC_LOAD_INT(frame, 2);
    char ch;

    if ((jch > 255) || (jch == 0)) {
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, (char *) &jch, 1, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        ch = (char) jch;
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, &ch, -1, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_ID(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1);
    jdouble dval = JEMCC_LOAD_DOUBLE(frame, 2);
    char dbuff[64];

    /* Convert to ASCII text and insert */
    JEMCC_DoubleToText(dval, dbuff);
    if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                            index, dbuff, -strlen(dbuff), 
                            JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_IF(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1);
    jdouble fval = JEMCC_LOAD_FLOAT(frame, 2);
    char fbuff[64];

    /* Convert to ASCII text and insert */
    JEMCC_FloatToText(fval, fbuff);
    if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                            index, fbuff, -strlen(fbuff), 
                            JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_II(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1), ival = JEMCC_LOAD_INT(frame, 2);
    char ibuff[64];

    /* Convert to ASCII text and insert */
    JEMCC_IntegerToText(ival, 10, ibuff);
    if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                            index, ibuff, -strlen(ibuff), 
                            JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_IJ(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1);
    jlong lval = JEMCC_LOAD_LONG(frame, 2);
    char lbuff[64];

    /* Convert to ASCII text and insert */
    JEMCC_LongToText(lval, 10, lbuff);
    if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                            index, lbuff, -strlen(lbuff), 
                            JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_IObject(JNIEnv *env,
                                              JEMCC_VMFrame *frame,
                                              JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1);
    JEMCC_Object *obj = JEMCC_LOAD_OBJECT(frame, 2);
    JEMCC_Class *objClass = VM_CLASS(JEMCC_Class_Object);
    JEMCC_ReturnValue strVal;
    JEMCC_StringData *strData;

    /* Special handling for this one! */
    if (obj == NULL) {
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, "null", -4, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        /* Call the toString() method */
        JEMCC_PUSH_STACK_OBJECT(frame, obj);
        JEMCC_CHECK_METHOD_REFERENCE(env, objClass, 8,
                                     "toString", "()Ljava/lang/String;");
        if (JEMCC_ExecuteInstanceMethod(env, obj, objClass, 8, 
                                        JEMCC_VIRTUAL_METHOD,
                                        &strVal) != JNI_OK) {
            return JEMCC_ERR;
        }

        /* Insert the result */
        strData = (JEMCC_StringData *)
                        ((JEMCC_ObjectExt *) strVal.objVal)->objectData;
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, &(strData->data), strData->length,
                                JNI_FALSE) != JNI_OK) {
            /* TODO - GC String value */
            return JEMCC_ERR;
        }
        /* TODO - GC String value */
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_IString(JNIEnv *env,
                                              JEMCC_VMFrame *frame,
                                              JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1);
    JEMCC_ObjectExt *strObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 2);
    JEMCC_StringData *strData;

    /* Extract the string contents and use the general insert method */
    if (strObj == NULL) {
        /* Special handling for this one! */
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, "null", -4, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        strData = (JEMCC_StringData *) strObj->objectData;
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, &(strData->data), strData->length,
                                JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_IZ(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1), bVal = JEMCC_LOAD_INT(frame, 2);

    if (bVal == JNI_FALSE) {
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, "false", -5, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, "true", -4, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_insert_ICharArray(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    jint index = JEMCC_LOAD_INT(frame, 1);
    JEMCC_ArrayObject *arr = (JEMCC_ArrayObject *) JEMCC_LOAD_OBJECT(frame, 2);
    jboolean isAscii = JNI_TRUE;
    jchar jch, *jptr;
    jint i;

    /* Make a quick exit where necessary */
    if (arr->arrayLength == 0) {
        retVal->objVal = (JEMCC_Object *) thisObj;
        return JEMCC_RET_OBJECT;
    }

    /* Determine Unicode/ASCII array status */
    for (i = 0, jptr = (jchar *) arr->arrayData; i < arr->arrayLength; i++) {
        jch = *(jptr++);
        if ((jch == 0) || (jch > 255)) {
            isAscii = JNI_FALSE;
            break;
        }
    }

    /* Make the approprate insert */
    if (isAscii == JNI_FALSE) {
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, (char *) arr->arrayData, 
                                arr->arrayLength, JNI_FALSE) != JNI_OK) {
            return JEMCC_ERR;
        }
    } else {
        if (StringBuffer_Insert(env, (StringBufferData *) thisObj->objectData,
                                index, (char *) arr->arrayData, 
                                -arr->arrayLength, JNI_TRUE) != JNI_OK) {
            return JEMCC_ERR;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_length(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    jint length = BUFFER_LENGTH(data);

    retVal->intVal = (length < 0) ? -length : length;
    return JEMCC_RET_INT;
}

static jint JEMCC_StringBuffer_reverse(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    JEMCC_StringData *strData;
    jchar jch, *jptr, *jstr;
    char ch, *ptr, *str;
    jint halfLen;

    /* Ensure that there is a local working buffer */
    if (data->capacity < 0) {
        if (StringBuffer_EnsureCapacity(env, data, -data->capacity,
                                        JNI_FALSE) != JNI_OK) return JEMCC_ERR;
    }
    strData = data->buffer.strData;

    /* Perform the appropriate reverse */
    if (strData->length <= 0) {
        halfLen = -strData->length / 2;
        ptr = &(strData->data);
        str = ptr - strData->length - 1;
        while (halfLen > 0) {
            ch = *ptr;
            *(ptr++) = *str;
            *(str--) = ch;
            halfLen--;
        }
    } else {
        halfLen = strData->length / 2;
        jptr = (jchar *) &(strData->data);
        jstr = jptr + strData->length - 1;
        while (halfLen > 0) {
            jch = *jptr;
            *(jptr++) = *jstr;
            *(jstr--) = jch;
            halfLen--;
        }
    }

    /* Return myself */
    retVal->objVal = (JEMCC_Object *) thisObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_StringBuffer_setCharAt_IC(JNIEnv *env,
                                            JEMCC_VMFrame *frame,
                                            JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    jint bufferLen = BUFFER_LENGTH(data), index = JEMCC_LOAD_INT(frame, 1);
    jchar jch = (jchar) JEMCC_LOAD_INT(frame, 2);
    jboolean expAscii = JNI_FALSE;
    JEMCC_StringData *strData;

    /* May need to convert to Unicode */
    if ((bufferLen < 0) && ((jch == 0) || (jch > 255))) expAscii = JNI_TRUE;

    /* Ensure that there is a local working buffer */
    if ((data->capacity < 0) || (expAscii == JNI_TRUE)) {
        if (StringBuffer_EnsureCapacity(env, data, abs(data->capacity),
                                        expAscii) != JNI_OK) return JEMCC_ERR;
    }
    strData = data->buffer.strData;
    bufferLen = strData->length;

    /* Make sure the location is valid */
    if (JEMCC_CheckStringLimits(env, strData, index) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* Push the character as indicated */
    if (bufferLen < 0) {
        *(&(strData->data) + index) = (char) jch;
    } else {
        *(((jchar *) &(strData->data)) + index) = jch;
        if ((jch != 0) && (jch <= 255)) {
            /* Possibility that end result is pure Ascii now */
            StringBuffer_CollapseIfAscii(strData);
        }
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_StringBuffer_setLength_I(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    jint bufferLen = BUFFER_LENGTH(data), newLen = JEMCC_LOAD_INT(frame, 1);
    jboolean expAscii = JNI_FALSE;
    JEMCC_StringData *strData;
    jchar *jptr;

    /* Make sure an appropriate length was asked for */
    if (newLen < 0) {
        JEMCC_ThrowStdThrowableIdx(env,
                                   JEMCC_Class_StringIndexOutOfBoundsException,
                                   NULL, "Negative length for StringBuffer");
        return JEMCC_ERR;
    }

    /* May need to convert to Unicode (bigger length) */
    if ((bufferLen < 0) && ((-bufferLen) < newLen)) expAscii = JNI_TRUE;

    /* Ensure that there is a local working buffer (with sufficient size) */
    if (StringBuffer_EnsureCapacity(env, data, newLen,
                                    expAscii) != JNI_OK) return JEMCC_ERR;
    strData = data->buffer.strData;
    bufferLen = strData->length;

    /* Set the new length, adjusting as necessary */
    if (bufferLen < 0) {
        /* If still ASCII, no length extension (Unicode) */
        strData->length = -newLen;
        *(&(data->buffer.strData->data) + newLen) = '\0';
    } else {
        strData->length = newLen;
        if (newLen < bufferLen) {
            /* Check for Unicode truncation to Ascii */
            StringBuffer_CollapseIfAscii(strData);
        } else {
            /* Nullify extra characters */
            jptr = ((jchar *) &(strData->data)) + bufferLen;
            while (bufferLen < newLen) {
                *(jptr++) = 0;
                bufferLen++;
            }
        }
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_StringBuffer_toString(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    StringBufferData *data = (StringBufferData *) thisObj->objectData;
    JEMCC_ObjectExt *retStr;

    if (data->capacity < 0) {
        /* Already converted to a string value, return it */
        retVal->objVal = (JEMCC_Object *) data->buffer.strInst;
    } else {
        /* Create the returned string instance and assign local buffer ref */
        retStr = (JEMCC_ObjectExt *) JEMCC_AllocateObjectIdx(env,
                                                             JEMCC_Class_String,
                                                             0);
        if (retStr == NULL) return JEMCC_ERR;
        retStr->objectData = data->buffer.strData;

        /* Mark the local buffer as a shared String copy */
        data->capacity = -data->capacity;
        data->buffer.strInst = retStr;

        /* And send back the new string instance */
        retVal->objVal = (JEMCC_Object *) retStr;
    }

    return JEMCC_RET_OBJECT;
}

JEMCC_MethodData JEMCC_StringBufferMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_StringBuffer_init },
    { ACC_PUBLIC,
         "<init>", "(I)V",
         JEMCC_StringBuffer_init_I },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/String;)V",
         JEMCC_StringBuffer_init_String },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "append", "(C)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_C },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "append", "(D)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_D },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "append", "(F)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_F },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "append", "(I)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_I },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "append", "(J)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_J },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "append", "(Ljava/lang/Object;)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_Object },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_String },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "append", "(Z)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_Z },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "append", "([C)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_CharArray },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "append", "([CII)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_append_CharArrayII },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "capacity", "()I",
         JEMCC_StringBuffer_capacity },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "charAt", "(I)C",
         JEMCC_StringBuffer_charAt_I },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "ensureCapacity", "(I)V",
         JEMCC_StringBuffer_ensureCapacity_I },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "getChars", "(II[CI)V",
         JEMCC_StringBuffer_getChars_IICharArrayI },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "insert", "(IC)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_IC },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "insert", "(ID)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_ID },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "insert", "(IF)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_IF },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "insert", "(II)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_II },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "insert", "(IJ)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_IJ },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "insert", "(ILjava/lang/Object;)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_IObject },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "insert", "(ILjava/lang/String;)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_IString },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "insert", "(IZ)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_IZ },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "insert", "(I[C)Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_insert_ICharArray },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "length", "()I",
         JEMCC_StringBuffer_length },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "reverse", "()Ljava/lang/StringBuffer;",
         JEMCC_StringBuffer_reverse },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "setCharAt", "(IC)V",
         JEMCC_StringBuffer_setCharAt_IC },
    { ACC_PUBLIC | ACC_SYNCHRONIZED,
         "setLength", "(I)V",
         JEMCC_StringBuffer_setLength_I },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "toString", "()Ljava/lang/String;",
         JEMCC_StringBuffer_toString }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC | ACC_FINAL,
                     "java.lang.StringBuffer",
                     NULL ** java/lang/Object **,
                     interfaces ** java/io/Serializable,  **, 1,
                     JEMCC_StringBufferMethods, 31, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
