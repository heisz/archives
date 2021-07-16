/**
 * JEMCC core methods to support java.lang.String operations.
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

/**
 * Scans the provided UTF-8 string information to determine if the contents
 * are ASCII (7-bit characters).
 *
 * Parameters:
 *     utfStrData - the UTF-8 encoded string data to scan (must be terminated
 *                  by the '\0' character)
 *
 * Returns:
 *     JNI_TRUE if the contents of the UTF-8 encoded string are ASCII
 *     characters only.
 */
jboolean JEMCC_UTFStrIsAscii(const char *utfStrData) {
    while (*utfStrData != '\0') {
        if ((*utfStrData & 0x80) != 0) return JNI_FALSE;
        utfStrData++;
    }
    return JNI_TRUE;
}

/**
 * Scans the provided Unicode string information (16-bit) to determine if the
 * contents are ASCII (7-bit characters).
 *
 * Parameters:
 *     uniStrData - the Unicode text to scan
 *     len - the number of Unicode characters in the text
 *
 * Returns:
 *     JNI_TRUE if the contents of the Unicode string are ASCII
 *     characters only.
 */
jboolean JEMCC_UnicodeStrIsAscii(const jchar *uniStrData, jsize len) {
    jchar jch;

    while (len > 0) {
        jch = *(uniStrData++);
        if ((jch == 0) || (jch > 255)) return JNI_FALSE;
        len--;
    }
    return JNI_TRUE ;
}

/**
 * Local methods to process UTF8 encoded character sequences, determining
 * length and expanding to 16-bit Unicode sequences.
 */
static jint JEM_GetUTFLength(const char *utfStrData) {
    jint len = 0;
    char ch;

    while ((ch = *utfStrData) != '\0') {
        if ((ch & 0x80) == 0) {
            utfStrData++;
        } else {
            if ((ch & 0x20) == 0) {
                if ((*(++utfStrData) & 0xC0) == 0x80) utfStrData++;
                else break;
            } else {
                if ((*(++utfStrData) & 0xC0) == 0x80) utfStrData++;
                else break;
                if ((*utfStrData & 0xC0) == 0x80) utfStrData++;
                else break;
            }
        }
        len++;
    }

    return len;
}

/* The following returns length, positive for unicode, negative for ASCII */
static jint JEM_UTFDecode(const unsigned char *utfStrData, jchar *buffer) {
    jint ch, asciiInd = -1, len = 0;
    jchar uch;

    while ((ch = *utfStrData) != '\0') {
        if ((ch & 0x80) == 0) {
            uch = (jchar) ch;
            utfStrData++;
        } else {
            if ((ch & 0x20) == 0) {
                uch = (ch & 0x1f) << 6;
                if (((ch = *(++utfStrData)) & 0xC0) == 0x80) {
                    uch = uch | (ch & 0x3F);
                } else break;
            } else {
                uch = (ch & 0x0f) << 12;
                if (((ch = *(++utfStrData)) & 0xC0) == 0x80) {
                    uch = uch | (ch & 0x3F) << 6;
                } else break;
                if (((ch = *(++utfStrData)) & 0xC0) == 0x80) {
                    uch = uch | (ch & 0x3F);
                } else break;
            }
            if ((uch == 0) || ((uch & 0xFF80) != 0)) asciiInd = 1;
            utfStrData++;
        }
        *(buffer++) = uch;
        len++;
    }

    return (asciiInd * len);
}

/**
 * Common method to retrieve/insert a String instance into the intern()'ed
 * hashtable within the VM.  Used by all three of the following methods.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the string data information used to lookup/create the
 *               intern()'ed string instance.  Automatically free'd if
 *               an intern instance is found and potential creation was
 *               indicated (string argument NULL)
 *     string - if non-NULL, and no matching string is found in the
 *              intern() table, insert this String instance into the
 *              table instead of creating a new one
 *
 * Returns:
 *     The interned String instance (either found, created or inserted) or
 *     NULL if a String creation or hashtable insertion failed (an OutOfMemory
 *     error has been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
static JEMCC_Object *JEM_InternString(JNIEnv *env, JEMCC_StringData *strData,
                                      JEMCC_Object *string) {
    JEMCC_Class *stringClass = VM_CLASS(JEMCC_Class_String);
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_JavaVM *jvm = jenv->parentVM;
    JEMCC_Object *retStr;
    jint rc;

    /* Watch for concurrent access */
    if (JEMCC_EnterObjMonitor(env, stringClass) != JNI_OK) {
        if (string == NULL) JEMCC_Free(strData);
        return NULL;
    }

    /* Easy return when already existing in table */
    retStr = (JEMCC_Object *) JEMCC_HashGetEntry(env, &(jvm->internStringTable),
                                                 strData, JEMCC_StringHashFn,
                                                 JEMCC_StringEqualsFn);
    if (retStr != NULL) {
        if (string == NULL) JEMCC_Free(strData);
        if (JEMCC_ExitObjMonitor(env, stringClass) != JNI_OK) abort();
        return retStr;
    }

    /* If indicated, just insert the string instance into the intern() table */
    if (string != NULL) {
        rc = JEMCC_HashInsertEntry(env, &(jvm->internStringTable),
                                   strData, string, NULL, NULL,
                                   JEMCC_StringHashFn, JEMCC_StringEqualsFn);
        if (JEMCC_ExitObjMonitor(env, stringClass) != JNI_OK) abort();
        if (rc == JNI_ENOMEM) return NULL;
        if (rc != JNI_OK) abort();
        return string;
    }

    /* Otherwise, create a new instance, using the provided string data */
    retStr = JEMCC_AllocateObject(env, stringClass, 0);
    if (retStr == NULL) {
        JEMCC_Free(strData);
        if (JEMCC_ExitObjMonitor(env, stringClass) != JNI_OK) abort();
        return NULL;
    }
    ((JEMCC_ObjectExt *) retStr)->objectData = strData;

    /* And insert it into the table */
    rc = JEMCC_HashInsertEntry(env, &(jvm->internStringTable),
                               strData, retStr, NULL, NULL,
                               JEMCC_StringHashFn, JEMCC_StringEqualsFn);
    if (JEMCC_ExitObjMonitor(env, stringClass) != JNI_OK) abort();
    if (rc == JNI_ENOMEM) {
        JEMCC_Free(strData);
        return NULL;
    }
    if (rc != JNI_OK) abort();

    return retStr;
}

/**
 * Generate/retrieve an "interned" string instance based on the provided
 * UTF-8 encoded string information.  If the given character sequence is found
 * in the intern()'ed String hashtable associated with the VM, that String is
 * returned.  Otherwise, a new String is created with the given characters,
 * stored in the VM hashtable and returned.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     utfStrData - the UTF-8 encoded string data to internalize (must be
 *                  terminated by the '\0' character)
 *
 * Returns:
 *     The interned String instance (either found or created) or NULL if
 *     a String creation was attempted but failed due to a memory error
 *     (an OutOfMemoryError has been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JEMCC_Object *JEMCC_GetInternStringUTF(JNIEnv *env, 
                                       const char *utfStrData) {
    JEMCC_StringData *strData;
    jchar *jptr;
    char *ptr;
    jint len;

    /* Prepare the string data lookup/create structure */
    if (JEMCC_UTFStrIsAscii((char *) utfStrData) == JNI_TRUE) {
        len = strlen(utfStrData);
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                               sizeof(JEMCC_StringData) + len);
        if (strData == NULL) return NULL;
        strData->length = -len;
        (void) strcpy(&(strData->data), utfStrData);
    } else {
        len = JEM_GetUTFLength(utfStrData);
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                               sizeof(JEMCC_StringData) + len * sizeof(jchar));
        if (strData == NULL) return NULL;

        strData->length = len;
        if (JEM_UTFDecode(utfStrData, (jchar *) &(strData->data)) < 0) {
            /* Embedded ASCII, compact and flip result */
            strData->length = -len;
            jptr = (jchar *) ptr = &(strData->data);
            while (len > 0) {
                *(ptr++) = *(jptr++) & 0xFF;
                len--;
            }
            *ptr = '\0';
        }
    }

    /* And locate/create the intern() String instance */
    return JEM_InternString(env, strData, NULL);
}

/**
 * Generate/retrieve an "interned" string instance based on the provided
 * Unicode string information.  If the given character sequence is found
 * in the intern()'ed String hashtable associated with the VM, that String is
 * returned.  Otherwise, a new String is created with the given characters,
 * stored in the VM hashtable and returned.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     uniStrData - the Unicode text to internalize
 *     len - the number of characters in the Unicode text
 *
 * Returns:
 *     The interned String instance (either found or created) or NULL if
 *     a String creation was attempted but failed due to a memory error
 *     (an OutOfMemoryError has been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JEMCC_Object *JEMCC_GetInternString(JNIEnv *env, const jchar *uniStrData, 
                                    jsize len) {
    JEMCC_StringData *strData;
    jchar *jptr;
    char *ptr;

    /* Prepare the string data lookup/create structure */
    if (JEMCC_UnicodeStrIsAscii(uniStrData, len) == JNI_TRUE) {
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                               sizeof(JEMCC_StringData) + len);
        if (strData == NULL) return NULL;

        strData->length = -len;
        ptr = &(strData->data);
        jptr = (jchar *) uniStrData;
        while (len > 0) {
            *(ptr++) = *(jptr++) & 0xFF;
            len--;
        }
        *ptr = '\0';
    } else {
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                sizeof(JEMCC_StringData) + len * sizeof(jchar));
        if (strData == NULL) return NULL;

        strData->length = len;
        (void) memcpy(&(strData->data), uniStrData, len * sizeof(jchar));
    }

    /* And locate/create the intern() String instance */
    return JEM_InternString(env, strData, NULL);
}

/**
 * Method provided for the java.lang.String class implementation to access
 * the internalized String table in the VM.  Locates the intern()'ed String
 * instance with the same character sequence as the given String, or inserts
 * the given String into the internalzed String table and returns the given
 * instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     string - the String instance containing the character sequence to locate
 *              the internalized String instance for
 *
 * Returns:
 *     The intern()'ed String instance associated with the provided String
 *     instance (will be the same instance if not found in the table) or
 *     NULL if a memory error occurred while inserting the String into the
 *     VM hashtable (an OutOfMemoryError has been thrown in the current
 *     environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JEMCC_Object *JEMCC_FindInternString(JNIEnv *env, JEMCC_Object *string) {
    JEMCC_StringData *strData =
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) string)->objectData;

    /* Simply pass on to the common method */
    return JEM_InternString(env, strData, string);
}

/**
 * Hash generation function which operates with the java.lang.String 
 * attachment data structure (the provided key is the JEMCC_StringData
 * structure, NOT the java.lang.String object reference).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the JEM_StringData reference to be hashed
 *
 * Returns:
 *     The numerical hashcode for the provided String contents.
 */
juint JEMCC_StringHashFn(JNIEnv *env, void *key) {
    JEMCC_StringData *strData = (JEMCC_StringData *) key;
    unsigned long hashCode = 0;
    char *ptr = &(strData->data);
    jint len = strData->length;

    if (len < 0) {
        /* ASCII */
        while (*ptr != '\0') {
            hashCode += *(ptr++);
        }
    } else {
        /* Unicode */
        while (len > 0) {
            hashCode += *(((jchar *) ptr)++);
            len--;
        }
    }
    return hashCode;
}

/**
 * Comparison function which operates with the java.lang.String 
 * attachment data structure (the provided keys are the JEMCC_StringData
 * structures, NOT the java.lang.String object references).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the JEM_StringData references to be compared
 *
 * Returns:
 *     JNI_TRUE if the two string instances are equal (same character
 *     sequences) otherwise JNI_FALSE.
 */
jboolean JEMCC_StringEqualsFn(JNIEnv *env, void *keya, void *keyb) {
    JEMCC_StringData *strDataA = (JEMCC_StringData *) keya;
    JEMCC_StringData *strDataB = (JEMCC_StringData *) keyb;
    char *ptrA = &(strDataA->data), *ptrB = &(strDataB->data);
    jint len = strDataA->length;

    if (len != strDataB->length) return JNI_FALSE;
    if (len < 0) {
        /* ASCII */
        return (strcmp(ptrA, ptrB) == 0) ? JNI_TRUE : JNI_FALSE;
    } else {
        /* Unicode */
        while (len > 0) {
            if (*(((jchar *) ptrA)++) != *(((jchar *) ptrB)++)) {
                return JNI_FALSE;
            }
            len--;
        }
    }

    return JNI_TRUE;
}

/**
 * Convenience method to construct a String instance from a NULL
 * terminated list of UTF-8 encoded elements.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - a series of char* UTF-8 encoded character sequences (each ending
 *           with the '\0' character), ending with a (char *) NULL pointer 
 *           marking the end of the list
 *
 * Returns:
 *     A java.lang.String instance containing the concatentation of the given
 *     character sequences or NULL if a memory allocation has occurred
 *     (an OutOfMemoryError has been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JEMCC_Object *JEMCC_StringCatUTF(JNIEnv *env, ...) {
    JEMCC_StringData *strData;
    JEMCC_Object *retStr;
    char *ptr, *str;
    jchar *jptr;
    va_list ap;
    jint isAscii = JNI_TRUE, l, len;

    /* Determine the full length, compensating for UTF/Unicode conversion */
    len = 0;
    va_start(ap, env);
    ptr = va_arg(ap, char *);
    while (ptr != NULL) {
        if (len > 0) {
            len += JEM_GetUTFLength(ptr);
        } else {
            if (JEMCC_UTFStrIsAscii(ptr) == JNI_TRUE) {
                len -= strlen(ptr);
            } else {
                len = -len + JEM_GetUTFLength(ptr);
            }
        }
        ptr = va_arg(ap, char *);
    }
    va_end(ap);

    /* Allocate the return string */
    if (len < 0) {
        retStr = JEMCC_AllocateObjectIdx(env, JEMCC_Class_String,
                                         sizeof(JEMCC_StringData) - len);
    } else {
        retStr = JEMCC_AllocateObjectIdx(env, JEMCC_Class_String,
                               sizeof(JEMCC_StringData) + len * sizeof(jchar));
    }
    if (retStr == NULL) return NULL;

    strData = (JEMCC_StringData *) ((JEMCC_ObjectExt *) retStr)->objectData;
    strData->length = len;
    str = &(strData->data);

    /* Assemble the String result */
    va_start(ap, env);
    ptr = va_arg(ap, char *);
    while (ptr != NULL) {
        if (len < 0) {
            while (*ptr != '\0') {
                *(str++) = *(ptr++);
            }
        } else {
            l = JEM_UTFDecode(ptr, (jchar *) str);
            if (l > 0) {
                isAscii = JNI_FALSE;
                str = str + l * sizeof(jchar);
            } else {
                str = str - l * sizeof(jchar);
            }
        }
        ptr = va_arg(ap, char *);
    }
    va_end(ap);

    if (isAscii == JNI_TRUE) {
        /* Check for complex ASCII UTF encoding */
        if (len > 0) {
            strData->length = -len;
            jptr = (jchar *) str = &(strData->data);
            while (len > 0) {
                *(str++) = *(jptr++) & 0xFF;
                len--;
            }
        }

        *str = '\0';
    }

    return retStr;
}

/* The following methods are JNI functions as well */

/**
 * Create a new java.lang.String instance, based on the provided
 * UTF-8 encoded text contents.  Note: this method corresponds to linkage
 * 167 of the JNI specification.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     utf - the UTF-8 encoded string data to scan (must be terminated
 *           by the '\0' character)
 *
 * Returns:
 *     The constructed String instance or NULL if a memory failure
 *     occurred (an OutOfMemoryError will have been thrown in the current
 *     environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jstring JEMCC_NewStringUTF(JNIEnv *env, const char *utf) {
    JEMCC_StringData *strData;
    JEMCC_Object *retStr;
    jchar *jptr;
    char *ptr;
    jint len;

    /* Prepare the string data lookup/create structure */
    if (JEMCC_UTFStrIsAscii(utf) == JNI_TRUE) {
        len = strlen(utf);
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                               sizeof(JEMCC_StringData) + len);
        if (strData == NULL) return NULL;
        strData->length = -len;
        (void) strcpy(&(strData->data), utf);
    } else {
        len = JEM_GetUTFLength(utf);
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                sizeof(JEMCC_StringData) + len * sizeof(jchar));
        if (strData == NULL) return NULL;

        strData->length = len;
        if (JEM_UTFDecode(utf, (jchar *) &(strData->data)) < 0) {
            /* Embedded ASCII, compact and flip result */
            strData->length = -len;
            jptr = (jchar *) ptr = &(strData->data);
            while (len > 0) {
                *(ptr++) = *(jptr++) & 0xFF;
                len--;
            }
            *ptr = '\0';
        }
    }

    /* Create and return the String instance */
    retStr = JEMCC_AllocateObjectIdx(env, JEMCC_Class_String, 0);
    if (retStr == NULL) {
        JEMCC_Free(strData);
        return NULL;
    }
    ((JEMCC_ObjectExt *) retStr)->objectData = strData;

    return (jstring) retStr;
}

/**
 * Create a new java.lang.String instance, based on the provided
 * Unicode text contents.  Note: this method corresponds to linkage
 * 163 of the JNI specification.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     unicode - the Unicode text contents of the string
 *     len - the number of characters in the Unicode text
 *
 * Returns:
 *     The constructed String instance or NULL if a memory failure
 *     occurred (an OutOfMemoryError will have been thrown in the current
 *     environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jstring JEMCC_NewString(JNIEnv *env, const jchar *unicode, jsize len) {
    JEMCC_StringData *strData;
    JEMCC_Object *retStr;
    jchar *jptr;
    char *ptr;

    /* Prepare the string data lookup/create structure */
    if (JEMCC_UnicodeStrIsAscii(unicode, len) == JNI_TRUE) {
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                               sizeof(JEMCC_StringData) + len);
        if (strData == NULL) return NULL;

        strData->length = -len;
        ptr = &(strData->data);
        jptr = (jchar *) unicode;
        while (len > 0) {
            *(ptr++) = *(jptr++) & 0xFF;
            len--;
        }
        *ptr = '\0';
    } else {
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                                sizeof(JEMCC_StringData) + len * sizeof(jchar));
        if (strData == NULL) return NULL;

        strData->length = len;
        (void) memcpy(&(strData->data), unicode, len * sizeof(jchar));
    }

    /* Create and return the String instance */
    retStr = JEMCC_AllocateObjectIdx(env, JEMCC_Class_String, 0);
    if (retStr == NULL) {
        JEMCC_Free(strData);
        return NULL;
    }
    ((JEMCC_ObjectExt *) retStr)->objectData = strData;

    return (jstring) retStr;
}

/**
 * Dump the contents of a String instance, either to the standard error
 * or output channel.  Used for debugging purposes.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     string - the String instance to dump
 *     useError - if JNI_TRUE, use the standard error channel, if JNI_FALSE,
 *                use standard output instead
 */
void JNICALL JEMCC_DumpString(JNIEnv *env, JEMCC_Object *string,
                              jboolean useError) {
    JEMCC_ObjectExt *str = (JEMCC_ObjectExt *) string;
    JEMCC_StringData *strData = (JEMCC_StringData *) str->objectData;
    FILE *fp = stderr;
    jchar *uptr;
    int i;

    if (useError == JNI_FALSE) fp = stdout;
    if (strData->length < 0) {
        /* ASCII is trivial */
        (void) fprintf(fp, &(strData->data));
    } else {
        /* Unicode takes a bit more work */
        uptr = (jchar *) &(strData->data);
        for (i = 0; i < strData->length; i++, uptr++) {
            if (*uptr < 255) {
                (void) fprintf(fp, "%c", (char) *uptr);
            } else {
                (void) fprintf(fp, "\\%o", *uptr);
            }
        }
    }
}

/**
 * Convenience method to validate the accessibility of the indexed String
 * character.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the String data structure to validate against
 *     index - the character index to validate
 *
 * Returns:
 *     JNI_OK if the String index is valid, JNI_ERR otherwise (an
 *     exception will be thrown in the current environment).
 *
 * Exceptions:
 *     StringIndexOutOfBoundsException - an invalid index was given
 */
jint  JEMCC_CheckStringLimits(JNIEnv *env, JEMCC_StringData *strData,
                              jsize index) {
    char msg[128];

    if ((index < 0) || (index >= abs(strData->length))) {
        (void) sprintf(msg, "String index out of range: %i", index);
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_StringIndexOutOfBoundsException,
                                   NULL, msg);
        return JNI_ERR;
    }
    
    return JNI_OK;
}

/**
 * Convenience method to validate the accessibility of the String characters
 * in the region offset -> offset + len - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the String data structure to validate against
 *     offset - the character index of the first member in the region
 *     len - the number of characters in the region
 *
 * Returns:
 *     JNI_OK if the array region is valid, JNI_ERR otherwise (an
 *     exception will be thrown in the current environment).
 *
 * Exceptions:
 *     StringIndexOutOfBoundsException - an invalid region was given
 */
jint JEMCC_CheckStringRegion(JNIEnv *env, JEMCC_StringData *strData,
                             jsize offset, jsize len) {
    jint length = abs(strData->length);
    char msg[128];

    if ((offset < 0) || (len < 0) || ((offset + len) > length)) {
        (void) sprintf(msg, "String region out of range: offset %i, length %i",
                            offset, len);
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_StringIndexOutOfBoundsException,
                                   NULL, msg);
        return JNI_ERR;
    }

    return JNI_OK;
}

/**
 * Convenience method to validate the accessibility of the String characters
 * in the segment from start to end - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the String data structure to validate against
 *     start - the index of the first String character
 *     end - the index of the last character plus one
 *
 * Returns:
 *     JNI_OK if the array index is valid, JNI_ERR otherwise (an
 *     exception will be thrown in the current environment).
 *
 * Exceptions:
 *     StringIndexOutOfBoundsException - an invalid segment was given
 */
jint JEMCC_CheckStringSegment(JNIEnv *env, JEMCC_StringData *strData,
                              jsize start, jsize end) {
    jint length = abs(strData->length);
    char msg[128];

    if ((start < 0) || (start >= length) || (end > length) || (start > end)) {
        (void) sprintf(msg, "String segment out of range: start %i, end %i",
                            start, end);
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_StringIndexOutOfBoundsException,
                                   NULL, msg);
        return JNI_ERR;
    }

    return JNI_OK;
}

/**
 * Common method for use with java.lang.String, java.lang.StringBuffer
 * and any other method requiring rapid access to the character array
 * contents of the StringData structure.  Does not perform any range checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the JEMCC_StringData structure containing the String data
 *     begin - the index of the first character to extract
 *     end - the index of the last character to extract plus one (e.g.
 *           extract occurs from begin to end - 1)
 *     buff - the char[] buffer to extract the String contents into
 */
void JEMCC_StringGetChars(JNIEnv *env, JEMCC_StringData *strData,
                          jint begin, jint end, jchar *buff) {
    jchar *jstr = NULL;
    char *str = NULL;
    jint len = end - begin;

    /* Setup and make the copy */
    if (strData->length < 0) {
        str = &(strData->data) + begin;
    } else {
        jstr = ((jchar *) &(strData->data)) + begin;
    }
    if (strData->length < 0) {
        while (len > 0) {
            *(buff++) = (jchar) *(str++);
            len--;
        }
    } else {
        while (len > 0) {
            *(buff++) = *(jstr++);
            len--;
        }
    }
}

/**
 * Common method for use with java.lang.String, java.lang.StringBuffer
 * and any other method requiring rapid creation of substrings based on
 * a parent String instance.  Does not perform any range checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the JEMCC_StringData structure containing the String data
 *     begin - the index of the first character to extract
 *     end - the index of the last character to extract plus one (e.g.
 *           extract occurs from begin to end - 1)
 *
 * Returns:
 *     NULL if a memory allocation failure occurred (no substring created).
 *     Otherwise, the substring as requested.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JEMCC_Object *JEMCC_StringSubstring(JNIEnv *env, JEMCC_StringData *strData,
                                    jint begin, jint end) {
    jint strlen = end - begin;
    unsigned int charLen = 1;
    JEMCC_StringData *newStrData;
    JEMCC_ObjectExt *retStr;
    jchar uch, *jptr, *jstr;
    char *ptr, *str;

    /* Determine if substring fragment is Unicode */
    if (strData->length > 0) {
        jptr = ((jchar *) &(strData->data)) + begin;
        while (strlen > 0) {
            uch = *(jptr++);
            if ((uch == 0) || (uch > 255)) {
                charLen = 2;
                break;
            }
            strlen--;
        }
        strlen = end - begin;
    }

    /* Allocate the storage first */
    retStr = (JEMCC_ObjectExt *) 
                  JEMCC_AllocateObjectIdx(env, JEMCC_Class_String,
                            sizeof(JEMCC_StringData) + charLen * (strlen + 1));
    if (retStr == NULL) return NULL;

    /* Make the copy, with all the conversion cases */
    newStrData = (JEMCC_StringData *) retStr->objectData;
    if (strData->length > 0) {
        if (charLen == 2) {
            /* Unicode to Unicode */
            newStrData->length = strlen;
            jptr = (jchar *) &(newStrData->data);
            jstr = ((jchar *) &(strData->data)) + begin;
            while (strlen > 0) {
                *(jptr++) = *(jstr++);
                strlen--;
            }
        } else {
            /* Unicode to ASCII */
            newStrData->length = -strlen;
            ptr = (char *) &(newStrData->data);
            jstr = ((jchar *) &(strData->data)) + begin;
            while (strlen > 0) {
                *(ptr++) = (char) *(jstr++);
                strlen--;
            }
            *ptr = '\0';
        }
    } else {
        /* ASCII to ASCII */
        newStrData->length = -strlen;
        ptr = (char *) &(newStrData->data);
        str = ((char *) &(strData->data)) + begin;
        while (strlen > 0) {
            *(ptr++) = *(str++);
            strlen--;
        }
        *ptr = '\0';
    }

    return (JEMCC_Object *) retStr;
}

/**
 * Common method for use with java.lang.String, java.lang.StringBuffer
 * and any other method performing subString searches within another String
 * (like strstr()).  Does not perform any range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the JEMCC_StringData structure containing the search string
 *     fromIdx - the index to begin searching at (if invalid, assumes 0)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the first 
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
jint JEMCC_StringString(JNIEnv *env, JEMCC_StringData *haystack,
                        JEMCC_StringData *needle, jint fromIdx) {
    jint scanLen, l, hLen = abs(haystack->length), nLen = abs(needle->length);
    jchar jch, *jptr, *jmptr, *jstr;
    char ch, *ptr, *mptr, *str;

    if (fromIdx < 0) fromIdx = 0;
    if ((nLen > hLen) || (fromIdx > hLen)) return -1;
    if (nLen == 0) return fromIdx;
    scanLen = hLen - nLen - fromIdx;

    if (haystack->length < 0) {
        /* ASCII haystack, only ASCII needles will match */
        if (needle->length > 0) return -1;

        /* Scan for a match on the first character, then the rest */
        ch = needle->data;
        ptr = &(haystack->data) + fromIdx;
        while (scanLen >= 0) {
            if (*(ptr++) != ch) {
                scanLen--;
                fromIdx++;
                continue;
            }
            l = nLen - 1;
            mptr = ptr;
            str = &(needle->data) + 1;
            while (l > 0) {
                if (*(mptr++) != *(str++)) break;
                l--;
            }
            if (l == 0) return fromIdx;
            scanLen--;
            fromIdx++;
        }
    } else {
        if (needle->length > 0) {
            /* Looking for Unicode in Unicode */
            jch = *((jchar *) &(needle->data));
            jptr = ((jchar *) &(haystack->data)) + fromIdx;
            while (scanLen >= 0) {
                if (*(jptr++) != jch) {
                    scanLen--;
                    fromIdx++;
                    continue;
                }
                l = nLen - 1;
                jmptr = jptr;
                jstr = ((jchar *) &(needle->data)) + 1;
                while (l > 0) {
                    if (*(jmptr++) != *(jstr++)) break;
                    l--;
                }
                if (l == 0) return fromIdx;
                scanLen--;
                fromIdx++;
            }
        } else {
            /* Looking for Ascii in Unicode */
            jch = (jchar) needle->data;
            jptr = ((jchar *) &(haystack->data)) + fromIdx;
            while (scanLen >= 0) {
                if (*(jptr++) != jch) {
                    scanLen--;
                    fromIdx++;
                    continue;
                }
                l = nLen - 1;
                jmptr = jptr;
                str = &(needle->data) + 1;
                while (l > 0) {
                    if (*(jmptr++) != (jchar) *(str++)) break;
                    l--;
                }
                if (l == 0) return fromIdx;
                scanLen--;
                fromIdx++;
            }
        }
    }

    return -1;
}

/**
 * Like the above, but performs the search in reverse order starting from
 * the given index (for lastIndexOf() calls).  Does not perform any 
 * range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the JEMCC_StringData structure containing the search string
 *     fromIdx - the index to begin searching at (if invalid, assumes end)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the last
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
jint JEMCC_LastStringString(JNIEnv *env, JEMCC_StringData *haystack,
                            JEMCC_StringData *needle, jint fromIdx) {
    jint l, hLen = abs(haystack->length), nLen = abs(needle->length);
    jchar jch, *jptr, *jmptr, *jstr;
    char ch, *ptr, *mptr, *str;

    if ((nLen > hLen) || (fromIdx < 0)) return -1;
    if (fromIdx >= (hLen - nLen)) fromIdx = hLen - nLen;
    if (nLen == 0) return fromIdx;

    if (haystack->length < 0) {
        /* ASCII haystack, only ASCII needles will match */
        if (needle->length > 0) return -1;

        /* Scan for a match on the first character, then the rest */
        ch = needle->data;
        ptr = &(haystack->data) + fromIdx;
        while (fromIdx >= 0) {
            if (*(ptr--) != ch) {
                fromIdx--;
                continue;
            }
            l = nLen - 1;
            mptr = ptr + 2;
            str = &(needle->data) + 1;
            while (l > 0) {
                if (*(mptr++) != *(str++)) break;
                l--;
            }
            if (l == 0) return fromIdx;
            fromIdx--;
        }
    } else {
        if (needle->length > 0) {
            /* Looking for Unicode in Unicode */
            jch = *((jchar *) &(needle->data));
            jptr = ((jchar *) &(haystack->data)) + fromIdx;
            while (fromIdx >= 0) {
                if (*(jptr--) != jch) {
                    fromIdx--;
                    continue;
                }
                l = nLen - 1;
                jmptr = jptr + 2;
                jstr = ((jchar *) &(needle->data)) + 1;
                while (l > 0) {
                    if (*(jmptr++) != *(jstr++)) break;
                    l--;
                }
                if (l == 0) return fromIdx;
                fromIdx--;
            }
        } else {
            /* Looking for Ascii in Unicode */
            jch = (jchar) needle->data;
            jptr = ((jchar *) &(haystack->data)) + fromIdx;
            while (fromIdx >= 0) {
                if (*(jptr--) != jch) {
                    fromIdx--;
                    continue;
                }
                l = nLen - 1;
                jmptr = jptr + 2;
                str = &(needle->data) + 1;
                while (l > 0) {
                    if (*(jmptr++) != (jchar) *(str++)) break;
                    l--;
                }
                if (l == 0) return fromIdx;
                fromIdx--;
            }
        }
    }

    return -1;
}

/**
 * Common method for use with any method performing character searches 
 * within a String (like strchr()).  Does not perform any range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the character to search for
 *     fromIdx - the index to begin searching at (if invalid, assumes 0)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the first
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
jint JEMCC_StringChar(JNIEnv *env, JEMCC_StringData *haystack,
                      jchar needle, jint fromIdx) {
    jint hLen = abs(haystack->length);
    jchar *jptr;
    char *ptr;

    if (fromIdx < 0) fromIdx = 0;
    if (fromIdx >= hLen) return -1;

    if (haystack->length < 0) {
        ptr = &(haystack->data) + fromIdx;
        while (fromIdx < hLen) {
            if (((jchar) *(ptr++)) == needle) return fromIdx;
            fromIdx++;
        }
    } else {
        jptr = ((jchar *) &(haystack->data)) + fromIdx;
        while (fromIdx < hLen) {
            if (*(jptr++) == needle) return fromIdx;
            fromIdx++;
        }
    }

    return -1;
}

/**
 * Like the above, but performs the search in reverse order starting from
 * the given index (for lastIndexOf() calls).  Does not perform any 
 * range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the character to search for
 *     fromIdx - the index to begin searching at (if invalid, assumes end)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the last
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
jint JEMCC_LastStringChar(JNIEnv *env, JEMCC_StringData *haystack,
                          jchar needle, jint fromIdx) {
    jint hLen = abs(haystack->length);
    jchar *jptr;
    char *ptr;

    if (fromIdx >= hLen) fromIdx = hLen - 1;
    if (fromIdx < 0) return -1;

    if (haystack->length < 0) {
        ptr = &(haystack->data) + fromIdx;
        while (fromIdx >= 0) {
            if (((jchar) *(ptr--)) == needle) return fromIdx;
            fromIdx--;
        }
    } else {
        jptr = ((jchar *) &(haystack->data)) + fromIdx;
        while (fromIdx >= 0) {
            if (*(jptr--) == needle) return fromIdx;
            fromIdx--;
        }
    }

    return -1;
}

/**
 * Append the given java.lang.String to the end of the current buffer.
 * If required, the buffer will expand to accomodate the space requirements
 * of the resulting string and an ASCII buffer will be converted to Unicode
 * if the String contains such characters.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     string - the String instance to append to the current buffer contents
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
char *JEMCC_EnvStringBufferAppend(JNIEnv *env, JEMCC_Object *string) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_StringData *strData;
    jchar *jptr;
    char *str;

    /* Match StringBuffer behaviour in this case */
    if (string == NULL) return JEMCC_EnvStrBufferAppend(env, "null");
    strData = (JEMCC_StringData *) ((JEMCC_ObjectExt *) string)->objectData;

    if (jenv->envBufferStringLength <= 0) {
        if (strData->length <= 0) {
            /* ASCII + ASCII */
            if (JEMCC_EnvEnsureBufferCapacity(env,
                                              -jenv->envBufferStringLength - 
                                                strData->length + 1) == NULL) {
                return NULL;
            }

            /* Copy the string onto the end */
            (void) memcpy(jenv->envEndPtr, &(strData->data), 
                          -strData->length + 1);
            jenv->envEndPtr += -strData->length;
            jenv->envBufferStringLength += strData->length;
        } else {
            /* ASCII + Unicode */
            if (JEMCC_EnvEnsureBufferCapacity(env,
                           -jenv->envBufferStringLength * sizeof(jchar) +
                                    strData->length * sizeof(jchar)) == NULL) {
                return NULL;
            }

            /* Convert existing ASCII contents to 16-bit Unicode */
            str = (char *) jenv->envEndPtr;
            jptr = ((jchar *) jenv->envBuffer) - jenv->envBufferStringLength;
            jenv->envEndPtr = (char *) jptr;
            while (str > (char *) jenv->envBuffer) {
                *(--jptr) = (jchar) *(--str);
            }
            jenv->envBufferStringLength = -jenv->envBufferStringLength;

            /* Copy the string onto the end */
            (void) memcpy(jenv->envEndPtr, &(strData->data), 
                          strData->length * sizeof(jchar));
            jenv->envEndPtr += strData->length * sizeof(jchar);
            jenv->envBufferStringLength += strData->length;
        }
    } else {
        if (strData->length <= 0) {
            /* Unicode + ASCII */
            if (JEMCC_EnvEnsureBufferCapacity(env,
                           jenv->envBufferStringLength * sizeof(jchar) -
                                    strData->length * sizeof(jchar)) == NULL) {
                return NULL;
            }

            /* Copy the string onto the end */
            jptr = (jchar *) jenv->envEndPtr;
            str = &(strData->data);
            while (*str != '\0') {
                *(jptr++) = (jchar) *(str++);
            }
            jenv->envEndPtr = (char *) jptr;
            jenv->envBufferStringLength -= strData->length;
        } else {
            /* Unicode + Unicode */
            if (JEMCC_EnvEnsureBufferCapacity(env,
                           jenv->envBufferStringLength * sizeof(jchar) +
                                    strData->length * sizeof(jchar)) == NULL) {
                return NULL;
            }

            /* Copy the string onto the end */
            (void) memcpy(jenv->envEndPtr, &(strData->data), 
                          strData->length * sizeof(jchar));
            jenv->envEndPtr += strData->length * sizeof(jchar);
            jenv->envBufferStringLength += strData->length;
        }
    }

    return (char *) jenv->envBuffer;
}

/**
 * Append the given Object to the end of the current buffer - this will make
 * a toString() call on the object.  If required, the buffer will expand to 
 * accomodate the space requirements of the resulting string and an ASCII 
 * buffer will be convereted to Unicode if the String contains such 
 * characters.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to append the string representation of
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL a memory allocation occurred (exception will be thrown
 *     in the current environment) or the toString() method call threw an
 *     exception.
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 *     Other exceptions as thrown in the toString() call.
 */
char *JEMCC_EnvStringBufferAppendObj(JNIEnv *env, JEMCC_Object *obj) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_VMFrame *frame = (JEMCC_VMFrame *) jenv->topFrame;
    JEMCC_Class *objClass = VM_CLASS(JEMCC_Class_Object);
    JEMCC_ReturnValue strVal;

    /* Match StringBuffer behaviour in this case */
    if (obj == NULL) return JEMCC_EnvStrBufferAppend(env, "null");

    /* Catch the String shortcut */
    if (obj->classReference == VM_CLASS(JEMCC_Class_String)) {
        return JEMCC_EnvStringBufferAppend(env, obj);
    }

    /* Call the toString method */
    JEMCC_PUSH_STACK_OBJECT(frame, obj);
    JEMCC_CHECK_METHOD_REFERENCE(env, objClass, 8,
                                 "toString", "()Ljava/lang/String;");
    if (JEMCC_ExecuteInstanceMethod(env, obj, objClass, 8,
                                    JEMCC_VIRTUAL_METHOD,
                                    &strVal) != JNI_OK) {
        return NULL;
    }

    /* Be consistent - null return from toString() buys you a NPE */
    if (strVal.objVal == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException,
                                   NULL, NULL);
        return NULL;
    }

    /* Append to the buffer and discard the intermediate String value */
    if (JEMCC_EnvStringBufferAppend(env, strVal.objVal) == NULL) return NULL;
    /* TODO - GC the String value */

    return (char *) jenv->envBuffer;
}

/**
 * Append the set of elements to the end of the current buffer.  This will
 * take a combination of char * strings and Strings/Objects.  The provided
 * list must be terminated by a NULL reference.  If required, the buffer 
 * will expand to accomodate the space requirements of the resulting string
 * and an ASCII  buffer will be converted to Unicode if the String contains
 * such characters.
 * 
 * NOTE: to properly mark Java Objects and Strings, they must be "wrapped"
 *       by the STR_OBJECT() macro, e.g.
 *
 *  JEMCC_EnvStringBufferAppendSet(env, "class=(", STR_OBJECT(clz), ")", NULL);
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - the set of elements to be appended to the buffer (NULL terminated)
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL a memory allocation occurred (exception will be thrown
 *     in the current environment) or a toString() method call threw an
 *     exception.
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 *     Other exceptions as thrown in the toString() call.
 */
/* define STR_OBJECT(obj) (void *) 0xdecaf, obj */
char *JEMCC_EnvStringBufferAppendSet(JNIEnv *env, ...) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    va_list ap;
    JEMCC_Object *obj;
    char *str;

    /* Note: do not perform multiple passes - lots of overhead */
    va_start(ap, env);
    str = va_arg(ap, char *);
    while (str != NULL) {
        if (str == (char *) 0xdecaf) {
            /* Object coming, handle String/Object appropriately */
            obj = va_arg(ap, JEMCC_Object *);
            if (obj->classReference == VM_CLASS(JEMCC_Class_String)) {
                if (JEMCC_EnvStringBufferAppend(env, obj) == NULL) {
                    va_end(ap);
                    return NULL;
                }
            } else {
                if (JEMCC_EnvStringBufferAppendObj(env, obj) == NULL) {
                    va_end(ap);
                    return NULL;
                }
            }
        } else {
            if (JEMCC_EnvStrBufferAppend(env, str) == NULL) {
                va_end(ap);
                return NULL;
            }
        }

        str = va_arg(ap, char *);
    }
    va_end(ap);

    return jenv->envBuffer;
}

/**
 * Convert the contents of the environment string buffer into a full
 * java.lang.String object (e.g. env.StringBuffer.toString()).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     Either a pointer to the String representation of the buffer contents
 *     or NULL if the allocation of the String object has failed (an
 *     OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the String failed
 */
JEMCC_Object *JEMCC_EnvStringBufferToString(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_StringData *strData;
    JEMCC_Object *retStr;

    /* Create the appropriate data structure */
    if (jenv->envBufferStringLength <= 0) {
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                       sizeof(JEMCC_StringData) - jenv->envBufferStringLength);
        if (strData == NULL) return NULL;
        strData->length = jenv->envBufferStringLength;
        (void) memcpy(&(strData->data), jenv->envBuffer,
                      -jenv->envBufferStringLength + 1);
    } else {
        strData = (JEMCC_StringData *) JEMCC_Malloc(env,
                          sizeof(JEMCC_StringData) + 
                                  jenv->envBufferStringLength * sizeof(jchar));
        if (strData == NULL) return NULL;

        strData->length = jenv->envBufferStringLength;
        (void) memcpy(&(strData->data), jenv->envBuffer,
                      jenv->envBufferStringLength * sizeof(jchar));
    }

    /* Create and return the String instance */
    retStr = JEMCC_AllocateObjectIdx(env, JEMCC_Class_String, 0);
    if (retStr == NULL) {
        JEMCC_Free(strData);
        return NULL;
    }
    ((JEMCC_ObjectExt *) retStr)->objectData = strData;

    return retStr;
}
