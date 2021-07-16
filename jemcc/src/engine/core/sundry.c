/**
 * Various bits and pieces which fill the gaps in the JEMCC VM.
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

/*************** Environment Buffer Management *****************/

/**
 * Ensure that the buffer has at least the specified number of bytes
 * allocated for use.
 *
 * NOTE: the error sweep implementation always requests a new buffer,
 *       regardless of new space requirements
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     length - the total number of bytes which the buffer must allocate to
 *              be available for subsequent use
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
jbyte *JEMCC_EnvEnsureBufferCapacity(JNIEnv *env, jsize length) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    jint newLength = jenv->envBufferLength;
    jbyte *newBuffer;

    /* Determine the required size (or initialize if necessary) */
#ifndef ENABLE_ERRORSWEEP
    if (newLength > length) return jenv->envBuffer;
#endif
    if (newLength == 0) return JEMCC_EnvStrBufferInit(env, length);
    while (newLength < length) newLength *= 2;

    /* Create the new buffer and clone the contents */
    newBuffer = (jbyte *) JEMCC_Malloc(env, newLength);
    if (newBuffer == NULL) return NULL;
    if (jenv->envBufferLength != 0) {
        (void) memcpy(newBuffer, jenv->envBuffer, jenv->envBufferLength);
    }
    JEMCC_Free(jenv->envBuffer);

    /* Save all the pointers and return the buffer */
    jenv->envEndPtr = newBuffer + (jenv->envEndPtr - jenv->envBuffer);
    jenv->envBuffer = newBuffer;
    jenv->envBufferLength = newLength;

    return jenv->envBuffer;
}

/**
 * Convenience method to prepare the buffer for string append operations.
 * Ensures that the buffer has preallocated at least the given number of
 * bytes and initializes the buffer to a zero-length string.  Note that
 * this will initialize the buffer for either str (char *) or full string
 * append operations (assumes ASCII initially).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     length - the total number of bytes which the buffer must allocate to
 *              be available for subsequent use
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
char *JEMCC_EnvStrBufferInit(JNIEnv *env, jsize length) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    jint trimLength = 512;

    /* Quick initialize if a buffer is already there */
    if (jenv->envBuffer != NULL) {
        if (JEMCC_EnvEnsureBufferCapacity(env, length) == NULL) return NULL;
        *jenv->envBuffer = '\0';
        jenv->envEndPtr = jenv->envBuffer;
        jenv->envBufferStringLength = 0;
        return (char *) jenv->envBuffer;
    }

    /* Try to optimize on a doubled size */
    while (trimLength < length) trimLength *= 2;

    /* Allocate and initialize the buffer pointers */
    jenv->envBuffer = (jbyte *) JEMCC_Malloc(env, trimLength);
    if (jenv->envBuffer == NULL) return NULL;
    jenv->envEndPtr = jenv->envBuffer;
    *(jenv->envEndPtr) = '\0';
    jenv->envBufferLength = trimLength;
    jenv->envBufferStringLength = 0;

    return (char *) jenv->envBuffer;
}

/**
 * Append the given character string to the end of the current buffer.
 * If required, the buffer will expand to accomodate the space requirements
 * of the resulting string.
 *
 * Parameters: 
 *     env - the VM environment which is currently in context
 *     str - the character string to append to the current buffer contents
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
char *JEMCC_EnvStrBufferAppend(JNIEnv *env, char *str) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    jint len = strlen(str);
    jchar *jptr;

    if (jenv->envBufferStringLength <= 0) {
        /* ASCII */
        /* Make sure we have the space */
        if (JEMCC_EnvEnsureBufferCapacity(env, 
                                          -jenv->envBufferStringLength +
                                                 len + 1) == NULL) return NULL;

        /* Copy the string onto the end */
        (void) memcpy(jenv->envEndPtr, str, len + 1);
        jenv->envEndPtr += len;
        jenv->envBufferStringLength -= len;
    } else {
        /* Unicode */
        /* Make sure we have the space */
        if (JEMCC_EnvEnsureBufferCapacity(env, 
                            jenv->envBufferStringLength * sizeof(jchar) +
                                                len * sizeof(jchar)) == NULL) {
            return NULL;
        }

        /* Copy the string onto the end */
        jptr = (jchar *) jenv->envEndPtr;
        while (*str != '\0') {
            *(jptr++) = (jchar) *(str++);
        }
        jenv->envEndPtr = (char *) jptr;
        jenv->envBufferStringLength += len;
    }

    return (char *) jenv->envBuffer;
}

/**
 * Append the given set of character strings to the end of the current buffer.
 * The list of append strings must be terminated by a NULL string reference.
 * If required, the buffer will expand to accomodate the space requirements
 * of the resulting string.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - the set of character strings to be appended to the buffer 
 *           (NULL terminated)
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
char *JEMCC_EnvStrBufferAppendSet(JNIEnv *env, ...) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    va_list ap;
    jchar *jptr;
    char *str;
    jint len;

    /* First, compute the total string length */
    len = 0;
    va_start(ap, env);
    str = va_arg(ap, char *);
    while (str != NULL) {
        len += strlen(str);
        str = va_arg(ap, char *);
    }
    va_end(ap);

    if (jenv->envBufferStringLength <= 0) {
        /* ASCII */
        /* Make sure we have sufficient buffer space for the contents */
        if (JEMCC_EnvEnsureBufferCapacity(env, 
                                          -jenv->envBufferStringLength +
                                                 len + 1) == NULL) return NULL;
        /* Do the appends */
        va_start(ap, env);
        str = va_arg(ap, char *);
        while (str != NULL) {
            while (*str != '\0') {
                *(jenv->envEndPtr++) = *(str++);
            }
            str = va_arg(ap, char *);
        }
        va_end(ap);
        *jenv->envEndPtr = '\0';
        jenv->envBufferStringLength -= len;
    } else {
        /* Unicode */
        /* Make sure we have sufficient buffer space for the contents */
        if (JEMCC_EnvEnsureBufferCapacity(env, 
                            jenv->envBufferStringLength * sizeof(jchar) +
                                                len * sizeof(jchar)) == NULL) {
            return NULL;
        }

        /* Do the appends */
        jptr = (jchar *) jenv->envEndPtr;
        va_start(ap, env);
        str = va_arg(ap, char *);
        while (str != NULL) {
            while (*str != '\0') {
                *(jptr++) = (jchar) *(str++);
            }
            str = va_arg(ap, char *);
        }
        va_end(ap);
        jenv->envEndPtr = (char *) jptr;
        jenv->envBufferStringLength += len;
    }

    return jenv->envBuffer;
}

/**
 * Obtain a copy of the current contents of the string buffer.  The buffer
 * must be at least initialized before calling this method or indeterminate
 * results may occur.  Note: the returned buffer may not be '\0' terminated
 * if Unicode characters were introduced through the String appends.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     Either a pointer to the character string copy of the buffer contents
 *     or NULL if the allocation of the requested memory has failed (an
 *     OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
char *JEMCC_EnvStrBufferDup(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint length;
    char *retVal;

    if (jenv->envBufferStringLength <= 0) {
        length = -jenv->envBufferStringLength + 1;
    } else {
        length = jenv->envBufferStringLength * sizeof(jchar);
    }

    retVal = (char *) JEMCC_Malloc(env, length);
    if (retVal == NULL) return retVal;

    (void) memcpy(retVal, jenv->envBuffer, length);
    return retVal;
}

/******************** Str (char *) Functions ****************************/

/**
 * Convenience method to concatenate a NULL terminated set of char*
 * text pointers into a single char* result, handling memory exceptions
 * appropriately.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the list
 *
 * Returns:
 *     The character string resulting from the concatentation of the given
 *     character sequences or NULL if a memory allocation has occurred
 *     (an OutOfMemoryError has been thrown in the current environment).
 *     The caller is responsible for free'ing this string result.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
char *JEMCC_StrCatFn(JNIEnv *env, ...) {
    va_list ap;
    char *retVal, *ptr, *str;
    int len;

    /* Determine the full length */
    len = 0;
    va_start(ap, env);
    ptr = va_arg(ap, char *);
    while (ptr != NULL) {
        len += strlen(ptr);
        ptr = va_arg(ap, char *);
    }
    va_end(ap);

    /* Allocate our string for return */
    str = retVal = (char *) JEMCC_Malloc(env, len + 1);
    if (retVal == NULL) return NULL;

    /* Build the string */
    va_start(ap, env);
    ptr = va_arg(ap, char *);
    while (ptr != NULL) {
        while (*ptr != '\0') {
            *(str++) = *(ptr++);
        }
        ptr = va_arg(ap, char *);
    }
    va_end(ap);
    *str = '\0';

    return retVal;
}
