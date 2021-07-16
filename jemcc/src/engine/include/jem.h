/**
 * Definitions of basic structures and utilities for the JEMCC libraries.
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

#ifndef JEM_H
#define JEM_H 1

/* Ensure inclusion of JNI and our local system definitions */
#include "jni.h"
#include "jemcc.h"
#include "sysenv.h"

/* <jemcc_start> */

/*********************** Coverage Test Methods *************************/

/**
 * Macros and counter/test methods which provide for code coverage testing
 * by "simulating" extraneous error conditions through a sequence counter.
 */
#ifdef ENABLE_ERRORSWEEP

#define ES_MEM 1
#define ES_DATA 2
#define ES_EXC 4
JNIEXPORT int JNICALL JEM_CheckErrorSweep(int sweepType);
#define ERROR_SWEEP(t, x) ((JEM_CheckErrorSweep(t) == JNI_TRUE) || (x))

#else

#define ERROR_SWEEP(t, x) (x)

#endif

/********************* Memory/Object Management ************************/

/**
 * Allocate a block of memory for storage purposes.  This method will
 * automatically trigger GC object cleanups as required, as well as
 * providing a central point for the memory failure test conditions.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     size - the number of bytes to allocate
 *
 * Returns:
 *     NULL if memory allocation failed, otherwise the allocated storage block.
 *     The allocated block of memory will be initialized to zero (calloc).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT void *JNICALL JEMCC_Malloc(JNIEnv *env, juint size);

/**
 * Free a block of memory that was allocated from JEMCC_Malloc.  Required
 * method as certain JEMCC implementations may use thread-local allocation
 * mechanisms outside of the standard malloc/free.
 *
 * Note: Object instances should never be passed through this method, let
 *       the garbage collector do its job!
 *
 * Parameters:
 *     block - the block of allocated memory to be freed
 */
JNIEXPORT void JNICALL JEMCC_Free(void *block);

/*
 * The main object (java.lang.Object) structure.  Every JEMCC object consists
 * of the common fixed structural data (class reference and state bitsets)
 * followed by the dynamic elements (class field data).  Note that the latter
 * structure definition is more commonly used by JEMCC instances which
 * directly access attached data elements and the final structure definition
 * provides convenient access to class definition data.
 */
struct JEMCC_Object {
    JEMCC_Class *classReference;
    juint objStateSet;
};

typedef struct JEMCC_ObjectExt {
    JEMCC_Class *classReference;
    juint objStateSet;

    /*
     * This element doesn't necessarily exist.  It just simplifies the
     * referencing of the dynamic object elements.
     */
    void *objectData;
} JEMCC_ObjectExt;

/* Convienence structural definitions for "core" typed VM objects */

/**
 * A standard class instance is just a base Object with native reference to
 * the class definition information (opaque) and the static data area
 * reference.
 */
struct JEMCC_Class {
    JEMCC_Class *classReference;
    juint objStateSet;

    struct JEM_ClassData *classData;
    void *staticData;
};

/**
 * An array contains an opaque reference to the native contents and the
 * defined length of the native array.
 */
typedef struct JEMCC_ArrayObject {
    JEMCC_Class *classReference;
    juint objStateSet;

    void *arrayData;
    jint arrayLength;
} JEMCC_ArrayObject;

/**
 * An array class maps to the base class definition with additional
 * members for depth and reference information.
 */
typedef struct JEMCC_ArrayClass {
    JEMCC_Class *classReference;
    juint objStateSet;

    /* In this case, the native data reference is the class information */
    /* and the array details are appended manually */
    struct JEM_ClassData *classData;
    juint typeDepthInfo;
    JEMCC_Class *referenceClass;
} JEMCC_ArrayClass;

/**
 * Allocate the memory block for a java.lang.Object based instance.  Also
 * provides the auto-allocation of internal attachment data which can be
 * used by the VM and JEMCC classes.  This does NOT call any <init>
 * constructors of the given class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class of the Object instance to be allocated (determines
 *                 the trailing dynamic element size and permits basic
 *                 initialization of the object)
 *     objDataSize - if greater than zero, allocate a native object data
 *                   block of this size in bytes.  The object class must
 *                   be a JEMCC class instance which has defined a native
 *                   reference pointer
 *
 * Returns:
 *     NULL if memory allocation failed, otherwise the object instance.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ExceptionInInitializerError - the initialization of the given class
 *                                   failed in some respect
 *     NoClassDefFoundError - the specified class has had a previous
 *                            initialization error and is now unusable
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_AllocateObject(JNIEnv *env,
                                                     JEMCC_Class *classInst,
                                                     juint objDataSize);

/**
 * Perform a clone operation on the indicated object - essentially a 
 * direct call of the Object.clone() method.  As described in that method,
 * this creates a new object of the same class and makes an exact copy
 * of the field contents of the object - it does not perform a "deep"
 * clone of the object.  JEMCC implementations which utilize native
 * object data will need to implement a clone() method to ensure that
 * a duplicate is made of native data area.
 *
 * The only exception to the above is arrays - if an array object is
 * provided to this method, a copy will also be made of the native
 * storage area automatically.
 *
 * Note: this method does not validate that the given object is, in fact,
 * cloneable (implements the Cloneable interface).  Nor does it perform a
 * NULL check on the provided object.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to be cloned
 *
 * Returns:
 *     NULL if memory allocation failed, otherwise the cloned object instance.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_CloneObject(JNIEnv *env,
                                                  JEMCC_Object *obj);

/**
 * Method used to "mark" an object when it is promoted to a non-local
 * context (stored in a field of another object).  Once this mark has
 * been made, full referential GC must be performed on the object to
 * ensure that it is not in use in other environments.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     object - the object instance which is to be marked non-local
 */
JNIEXPORT void JNICALL JEMCC_MarkNonLocalObject(JNIEnv *env,
                                                JEMCC_Object *object);

/**
 * Create a new instance of an object, calling the specified constructor
 * instance (essentially the Java "new" operator).  Note that the provided
 * frame instance must have the constructor arguments properly initialized
 * for the constructor call, not including the "this" object which will be
 * pushed onto the stack by this method.
 *
 * Note: this will create the object as a local frame object, which will
 * be immediately GC'd upon the exit of the current frame, unless the 
 * MarkNonLocalObject call is made (or the JNI field storage methods are
 * called).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     frame - the executing frame which contains the constructor arguments
 *             (if required)
 *     classInst - the class of the object to be instantiated
 *     desc - the descriptor of the constructor <init> method to be called
 *
 * Returns:
 *     The newly instantiated object of the requested class, or NULL if
 *     an error occurred in creating the object (an exception will be thrown
 *     in the current environment). 
 *
 * Exceptions:
 *     Any which may arise from the comparable Java "new" operator (class
 *     initialization errors, resolution errors, constructor opcode errors,
 *     memory errors and others).
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_NewObjectInstance(JNIEnv *env,
                                                        JEMCC_VMFrame *frame, 
                                                        JEMCC_Class *classInst,
                                                        const char *desc);

/********************* Array Utilities *********************************/

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
JNIEXPORT jint JNICALL JEMCC_CheckArrayLimits(JNIEnv *env,
                                              JEMCC_ArrayObject *array,
                                              jsize index, jint excIdx);

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
JNIEXPORT jint JNICALL JEMCC_CheckArrayRegion(JNIEnv *env,
                                              JEMCC_ArrayObject *array,
                                              jsize offset, jsize len,
                                              jint excIdx);

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
JNIEXPORT jint JNICALL JEMCC_CheckArraySegment(JNIEnv *env,
                                               JEMCC_ArrayObject *array,
                                               jsize start, jsize end,
                                               jint excIdx);

/********************* String Functions *********************************/

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
JNIEXPORT char *JNICALL JEMCC_StrCatFn(JNIEnv *env, ...);

/**
 * Attachment data structure associated with java.lang.String instances.
 * In all cases, the allocation of memory associated with this structure
 * is variable, as the byte information for the string is allocated as
 * part of the structure and begins with the "data" character.
 *
 * Note that the JEMCC implementation stores strings in ASCII or Unicode,
 * to save memory for pure ASCII situations.  If the length is less than
 * zero, the byte information beginning at the data marker is 7-bit ASCII
 * text (of the given negative length) which is always NULL terminated.
 * If the length is greater than zero, the byte information beginning at
 * the data marker is 16-bit (jchar) Unicode information of the given
 * length.
 */
typedef struct JEMCC_StringData {
    jsize length;

    char data;
} JEMCC_StringData;

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
JNIEXPORT jboolean JNICALL JEMCC_UTFStrIsAscii(const char *utfStrData);

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
JNIEXPORT jboolean JNICALL JEMCC_UnicodeStrIsAscii(const jchar *uniStrData,
                                                   jsize len);

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
JNIEXPORT JEMCC_Object *JNICALL JEMCC_GetInternStringUTF(JNIEnv *env,
                                                        const char *utfStrData);

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
JNIEXPORT JEMCC_Object *JNICALL JEMCC_GetInternString(JNIEnv *env,
                                                      const jchar *uniStrData,
                                                      jsize len);

/**
 * Method provided for the java.lang.String class implementation to access
 * the internalized String table in the VM.  Locates the intern()'ed String
 * instance with the same character sequence as the given String, or inserts
 * the given String into the internalized String table and returns the given
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
JNIEXPORT JEMCC_Object *JNICALL JEMCC_FindInternString(JNIEnv *env,
                                                       JEMCC_Object *string);

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
JNIEXPORT JEMCC_Object *JNICALL JEMCC_StringCatUTF(JNIEnv *env, ...);

/**
 * Hash generation function which operates with the java.lang.String
 * attachment data structure (the provided key is the JEMCC_StringData
 * structure, NOT the java.lang.String object reference).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the JEMCC_StringData reference to be hashed
 *
 * Returns:
 *     The numerical hashcode for the provided String contents.
 */
JNIEXPORT juint JNICALL JEMCC_StringHashFn(JNIEnv *env, void *key);

/**
 * Comparison function which operates with the java.lang.String
 * attachment data structure (the provided keys are the JEMCC_StringData
 * structures, NOT the java.lang.String object references).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the JEMCC_StringData references to be compared
 *
 * Returns:
 *     JNI_TRUE if the two string instances are equal (same character
 *     sequences) otherwise JNI_FALSE.
 */
JNIEXPORT jboolean JNICALL JEMCC_StringEqualsFn(JNIEnv *env, void *keya,
                                                void *keyb);

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
JNIEXPORT void JNICALL JEMCC_DumpString(JNIEnv *env, JEMCC_Object *string,
                                        jboolean useError);

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
JNIEXPORT jint JNICALL JEMCC_CheckStringLimits(JNIEnv *env,
                                               JEMCC_StringData *strData,
                                               jsize index);

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
JNIEXPORT jint JNICALL JEMCC_CheckStringRegion(JNIEnv *env,
                                               JEMCC_StringData *strData,
                                               jsize offset, jsize len);
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
JNIEXPORT jint JNICALL JEMCC_CheckStringSegment(JNIEnv *env,
                                                JEMCC_StringData *strData,
                                                jsize start, jsize end);

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
JNIEXPORT void JNICALL JEMCC_StringGetChars(JNIEnv *env, 
                                            JEMCC_StringData *strData,
                                            jint begin, jint end,
                                            jchar *buff);

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
JNIEXPORT JEMCC_Object *JNICALL JEMCC_StringSubstring(JNIEnv *env, 
                                                      JEMCC_StringData *strData,
                                                      jint begin, jint end);

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
JNIEXPORT jint JNICALL JEMCC_StringString(JNIEnv *env, 
                                          JEMCC_StringData *haystack,
                                          JEMCC_StringData *needle,
                                          jint fromIdx);

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
JNIEXPORT jint JNICALL JEMCC_LastStringString(JNIEnv *env, 
                                              JEMCC_StringData *haystack,
                                              JEMCC_StringData *needle,
                                              jint fromIdx);

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
JNIEXPORT jint JNICALL JEMCC_StringChar(JNIEnv *env, 
                                        JEMCC_StringData *haystack,
                                        jchar needle, jint fromIdx);

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
JNIEXPORT jint JNICALL JEMCC_LastStringChar(JNIEnv *env, 
                                            JEMCC_StringData *haystack,
                                            jchar needle, jint fromIdx);

/********************* Hashtable Management ************************/

/*
 * Hashtable data management.  Generic hashing structure and methods for
 * maintaining keyed data records.  Note that this is used internally by
 * the engine routines and is also used by the Hashtable implementation.
 */
typedef struct JEMCC_HashTable {
    juint entryCount, occupied;
    juint tableMask;
    struct JEM_HashEntry *entries;
} JEMCC_HashTable;

/**
 * Hash generation function prototype for use with the hashtable methods
 * below.  This function must always return the same hashcode for keys
 * which are "equal", but unequal keys are allowed to generate the same
 * hashcode (although it is potentially faster if they don't).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the key associated with the object to be hashed
 *
 * Returns:
 *     The numeric hashcode for the given key instance.
 */
typedef juint (*JEMCC_KeyHashFn)(JNIEnv *env, void *key);

/**
 * Comparison function for locating key matches in hashtables for use
 * with the hashtable methods below.  This function may do a simple
 * pointer comparison or a more complex equals comparison based on data
 * contained within the key structures.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the two keys to be compared
 *
 * Returns:
 *     JNI_TRUE if the values of the two keys are "equal", JNI_FALSE otherwise.
 */
typedef jboolean (*JEMCC_KeyEqualsFn)(JNIEnv *env, void *keya, void *keyb);

/**
 * Duplication function to create a new key based on the given key.  This
 * may be a simple pointer copy (easiest case) or a full duplication of the
 * information represented by the key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the key instance to be copied
 *
 * Returns:
 *     The "copy" of the provided key, or NULL if a memory allocation
 *     failed in the key duplication (and exception must be thrown in this
 *     case).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
typedef void *(*JEMCC_KeyDupFn)(JNIEnv *env, void *key);

/**
 * Callback method to enumerate entries within a hashtable.  Allows for
 * termination of the scanning through the return code.  NOTE: the hashtable
 * should only be modified during scanning using the "safe" methods such
 * as HashScanRemoveEntry below.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the table which is currently being scanned
 *     key - the key associated with the currently scanned entry
 *     object - the object associated with the currently scanned entry
 *     userData - the caller provided information attached to the scan
 *                request
 *
 * Returns:
 *     JNI_OK - continue with scanning of hashtable
 *     JNI_ERR - terminate scanning of hashtable
 */
typedef jint (*JEMCC_EntryScanCB)(JNIEnv *env, JEMCC_HashTable *table,
                                  void *key, void *obj, void *userData);

/**
 * Initialize a hash table instance to the given number of base hash
 * points.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - pointer to an existing instance of the hashtable to
 *             be initialized.  Already existing entries in the table
 *             will not be cleaned up
 *     startSize - the number of hash blocks to initially allocate in the
 *                 table.  If <= 0, an appropriate start size will be selected.
 *
 * Returns:
 *     JNI_OK - hashtable was initialized
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashInitTable(JNIEnv *env, JEMCC_HashTable *table,
                                           jsize startSize);
/**
 * Store an object into a hashtable.  Hashtable will expand as necessary,
 * and object will replace an already existing object with an equal key.
 * If an existing object is replaced, it is not destroyed but the key/object
 * pair is returned to allow the caller to clean up.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to put the key->value pair into
 *     key - the key associated with the entry
 *     object - the object to store in the hashtable according to the given key
 *     lastKey - if this pointer is non-NULL, the previous key is returned if
 *               the put entry replaces one in the hashtable or NULL is
 *               returned if the entry is new
 *     lastObject - if this pointer is non-NULL, the previous object is
 *                  returned if the put entry replaces one in the hashtable or
 *                  NULL is returned if the entry is new
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - hashtable insertion was successfuly
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashPutEntry(JNIEnv *env, JEMCC_HashTable *table,
                                          void *key, void *object,
                                          void **lastKey, void **lastObject,
                                          JEMCC_KeyHashFn keyHashFn,
                                          JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Almost identical to the HashPutEntry method, this method stores an
 * key->object entry into a hashtable unless there already exists an entry
 * in the hashtable with an "equal" key (i.e. this method will not replace
 * already existing hashtable entries where HashPutEntry does).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to put the key->value pair into
 *     key - the key associated with the entry
 *     object - the object to store in the hashtable according to the given key
 *     lastKey - if this pointer is non-NULL, the existing key is returned if
 *               the insert did not happen (no replace) or NULL if the entry
 *               is new and was inserted
 *     lastObject - if this pointer is non-NULL, the existing key is returned
 *                  if the insert did not happen (no replace) or NULL if the
 *                  entry is new and was inserted
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - hashtable insertion was successful
 *     JNI_ERR - an entry already exists in the hashtable for the given key
 *               and no action was taken (no exception has been thrown either)
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashInsertEntry(JNIEnv *env,
                                             JEMCC_HashTable *table,
                                             void *key, void *object,
                                             void **lastKey, void **lastObject,
                                             JEMCC_KeyHashFn keyHashFn,
                                             JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Remove an entry from the hashtable.  This does not destroy the removed
 * object/key, only the reference to them.  The original key/object pair
 * can be returned to the caller for cleanup purposes.  NOTE: this method
 * is not "safe" for use during hashtable scanning - use HashScanRemoveEntry
 * instead.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to remove the entry from
 *     key - the key of the pair entry to be removed
 *     origKey - if this pointer is non-NULL and an entry is removed, the
 *               original key of the entry is returned here
 *     origObject - if this pointer is non-NULL and an entry is removed, the
 *                  object associated with the entry is returned here
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - a matching entry was found and has been removed
 *     JNI_ERR - no entry matching the specified key was found (no exception
 *               is thrown as it may not really be an error)
 */
JNIEXPORT jint JNICALL JEMCC_HashRemoveEntry(JNIEnv *env,
                                             JEMCC_HashTable *table,
                                             void *key,
                                             void **origKey, void **origObject,
                                             JEMCC_KeyHashFn keyHashFn,
                                             JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Retrieve an object from the hashtable according to the specified key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to retrieve the entry from
 *     key - the key of the object to be obtained
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     NULL if no object entry has a matching key, otherwise the matching
 *     object reference.
 */
JNIEXPORT void *JNICALL JEMCC_HashGetEntry(JNIEnv *env, JEMCC_HashTable *table,
                                           void *key,
                                           JEMCC_KeyHashFn keyHashFn,
                                           JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Similar to the HashGetEntry method, this retrieves entry information
 * for the provided key, but obtains both the object and the key associated
 * with the hashtable entry.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to retrieve the entry from
 *     key - the key of the object to be obtained
 *     retKey - if non-NULL, the entry key is returned if a matching entry
 *              was found, otherwise NULL is returned
 *     retObject - if non-NULL, the entry object is returned if a matching
 *                 entry was found, otherwise NULL is returned
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - an entry matching the key was found and the data was returned
 *     JNI_ERR - no entry matching the key was found
 */
JNIEXPORT jint JNICALL JEMCC_HashGetFullEntry(JNIEnv *env,
                                              JEMCC_HashTable *table,
                                              void *key,
                                              void **retKey, void **retObject,
                                              JEMCC_KeyHashFn keyHashFn,
                                              JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Duplicate the given hashtable.  This will create copies of the internal
 * management structures of the hashtable and may possibly create duplicates
 * of the entry keys, if a duplication function is provided.  It does not
 * duplicate the object instances, only the references to the objects.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     dest - the hashtable to copy information into.  Any entries in this
 *            table will be lost without any sort of cleanup
 *     source - the hashtable containing the information to be copied
 *     keyDupFn - if non-NULL, this function will be called to duplicate
 *                the key instances between the tables
 *
 * Returns:
 *     JNI_OK - hashtable was duplicated
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment.  The duplicate hashtable may be partially
 *                  filled, if the memory failure occurred during key
 *                  duplication.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashDuplicate(JNIEnv *env, JEMCC_HashTable *dest,
                                           JEMCC_HashTable *source,
                                           JEMCC_KeyDupFn keyDupFn);

/**
 * Scan through all entries in a hashtable, calling the specified
 * callback function for each valid hashtable entry.  NOTE: only the
 * "safe" methods (such as HashScanRemoveEntry below) should be used while
 * a hashtable scan is in progress.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable containing the entries to be scanned
 *     entryCB - a function reference which is called for each valid
 *               entry in the hashtable
 *     userData - a caller provided data set which is included in the
 *                scan arguments
 */
JNIEXPORT void JNICALL JEMCC_HashScan(JNIEnv *env, JEMCC_HashTable *table,
                                      JEMCC_EntryScanCB entryCB,
                                      void *userData);

/**
 * Identical to the HashRemoveEntry method, this removes an entry from
 * the hashtable but is "safe" to use while a scan of the hashtable is
 * in progress.  This does not destroy the removed object/key - only the
 * reference to them.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to remove the entry from
 *     key - the key of the pair entry to be removed
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - a matching entry was found and has been removed
 *     JNI_ERR - no entry matching the specified key was found (no exception
 *               is thrown as it may not really be an error)
 */
JNIEXPORT void JNICALL JEMCC_HashScanRemoveEntry(JNIEnv *env,
                                                 JEMCC_HashTable *table,
                                                 void *key,
                                                 JEMCC_KeyHashFn keyHashFn,
                                                 JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Destroy the internals of a hashtable instance.  Does NOT destroy the
 * objects contained in the hashtable or the hashtable structure itself.
 *
 * Parameters:
 *     table - the hashtable instance to be internally destroyed
 */
JNIEXPORT void JNICALL JEMCC_HashDestroyTable(JEMCC_HashTable *table);

/* * * * * * * * * * * Convenience Hash Methods * * * * * * * * * * */

/**
 * Generate the hashcode value for a char sequence of characters (may
 * be ASCII or UTF-8 encoded Unicode) given in the key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the character sequence to be hashed (must be \0 terminated)
 *
 * Returns:
 *     The numerical hashcode associated with the given characters.
 */
JNIEXPORT juint JNICALL JEMCC_StrHashFn(JNIEnv *env, void *key);

/**
 * Perform a comparison of the character information contained in the
 * two given keys.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the two character sequences to be compared
 *                  (must be \0 terminated)
 *
 * Returns:
 *     JNI_TRUE if the two character sequences are identical (including
 *     null character), JNI_FALSE otherwise.
 */
JNIEXPORT jboolean JNICALL JEMCC_StrEqualsFn(JNIEnv *env, void *keya,
                                             void *keyb);

/**
 * Generate a duplicate of the character information provided in the given
 * key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the character sequence to be duplicated (must be \0 terminated)
 *
 * Returns:
 *     An allocated copy of the provided character sequence or NULL if
 *     the memory allocation failed (an OutOfMemoryError will have been
 *     thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT void *JNICALL JEMCC_StrDupFn(JNIEnv *env, void *key);

/**
 * Generate the hashcode value for a fully qualified Java class name.  In
 * this case, the hash result is the same whether the '.' or '/' package
 * separator is used (i.e. 'java/lang/Object' and 'java.lang.Object' will
 * generate the same hashcode).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the fully qualified classname to be hashed (must be \0 terminated)
 *
 * Returns:
 *     The numerical hashcode for the classname, "ignoring" package
 *     separators.
 */
JNIEXPORT juint JNICALL JEMCC_ClassNameHashFn(JNIEnv *env, void *key);

/**
 * Perform a string comparison of two fully qualified Java class names,
 * ignoring any differences in the '.' or '/' package separators
 * (i.e. 'java/lang/Object' and 'java.lang.Object' are identical according
 * to this method).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya - the fully qualified classname to be compared
 *     keyb - the fully qualified classname to compare against
 *            (both classnames must be \0 terminated)
 *
 * Returns:
 *     JNI_TRUE if the two classnames are equal ignoring separator differences,
 *     JNI_FALSE if they are different.
 */
JNIEXPORT jboolean JNICALL JEMCC_ClassNameEqualsFn(JNIEnv *env,
                                                   void *keya, void *keyb);

/* <jemcc_end> */

/******************* Path Management Functions *************************/

#define JEM_PATH_DIR 1
#define JEM_PATH_JARZIP 2
#define JEM_PATH_INV 3

struct JEMCC_ZipFile;
typedef struct JEM_PathEntryList {
    struct JEM_PathEntry {
        char *path;
        struct JEMCC_ZipFile *zipFile;
        jint type;
    } *entries;

    int entryCount;
} JEM_PathEntryList;

/**
 * Parse a path list string according to the Java CLASSPATH model.  Splits
 * entries based on a system dependent character (: or ;) and constructs a
 * path list of valid entries (either directories or zip files).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     list - the path entry set to be populated with the parse results
 *     path - the path list string to be parsed
 *     allowZip - if JNI_TRUE, path entries which are zip or jar files
 *                are included in the final list.  If JNI_FALSE, such
 *                files are skipped (list contains directories only).
 *
 * Returns:
 *     JNI_OK - path list has been successfully parsed
 *     JNI_ENOMEM - a memory error has occurred (an OutOfMemory exception
 *                  will have been thrown in the current environment)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEM_ParsePathList(JNIEnv *env, JEM_PathEntryList *list,
                                         const char *path, jboolean allowZip);

/**
 * Destroy and clean up the path list allocated by the ParsePathList method
 * above.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     list - the parsed path list instance to be destroyed
 */
JNIEXPORT void JNICALL JEM_DestroyPathList(JNIEnv *env,
                                           JEM_PathEntryList *list);

/**
 * Read the contents of a file located in the specified path list.
 * Scans each path location in order until it finds the requested file,
 * whereupon it reads it (expanding if necessary from a Zip file).  Note
 * that only the first instance of the file is tried - if an error occurs
 * during reading of the file, the search does not resume.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     pathList - the path entry set on which the file is located
 *     fileName - the name of the file to be read
 *     rawFileData - an allocated buffer containing the contents of the
 *                   file (the caller must free the buffer)
 *     rawFileSize - the size of the file (and hence the buffer contents)
 *
 * Returns:
 *     JNI_OK - the file was found and the read completed successfully
 *     JNI_EINVAL - the specified file was not found on the path
 *     JNI_ERR - the file was found and a read error occurred or a memory
 *               failure occurred while *reading* the file (an exception
 *               will be thrown in the current environment)
 *     JNI_ENOMEM - a memory failure occurred while *searching* for the
 *                  file (an exception will be thrown in the current
 *                  environment)
 *
 * Exceptions:
 *     ZipException - a Zipfile specific error occurred (invalid signature,
 *                    record information, etc.)
 *     IOException - a general file access exception occurred
 *     InternalError - an unexpected zlib library error occurred
 *     OutOfMemoryError - a memory allocation failed (search or read)
 */
JNIEXPORT jint JNICALL JEM_ReadPathFileContents(JNIEnv *env,
                                                JEM_PathEntryList *pathList,
                                                const char *fileName,
                                                jbyte **rawFileData,
                                                jsize *rawFileSize);

/**
 * Load a dynamic library through a specified source path (similar to
 * LD_LIBRARY_PATH).  Searches each directory entry in the path for the
 * indicated filename and attempts to load/link the library if found.
 * Note that only the first instance of the library is tried - if an error
 * occurs during linking, the search does not resume.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the dynamic loader instance through which the library is
 *              loaded.  If NULL, the main virtual machine loader is used.
 *     pathList - the path entry set on which the library is located.  If NULL,
 *            the virtual machine libPath is used.
 *     libFileName - the name of the library to load/link.  This must be
 *                   the system qualified library name, as returned from
 *                   the MapLibraryName method.  It can also be an absolute
 *                   filename, in which case the path entries are ignored.
 *     attachment - an opaque data object to associate with the dynamically
 *                  loaded library (for classloader cross-reference)
 *     libHandle - a reference pointer into which the linked library instance
 *                 will be returned, if the search/link action was successful
 *
 * Returns:
 *     JNI_OK - the library was found and linked successfully
 *     JNI_EINVAL - the specified library was not found on the vm path
 *     JNI_ERR - the library was found and a memory or data error occurred
 *               while reading/linking the library (an exception will be thrown
 *               in the current environment)
 *     JNI_ENOMEM - a memory failure occurred while searching for the
 *                  library (an exception will be thrown in the current
 *                  environment)
 *
 * Exceptions:
 *     IOException - a general file access exception occurred
 *     OutOfMemoryError - a memory allocation failed (search or read)
 *     UnsatisfiedLinkError - the library was found but the load/link failed
 */
JNIEXPORT jint JNICALL JEM_LoadPathLibrary(JNIEnv *env,
                                           JEM_DynaLibLoader loader,
                                           JEM_PathEntryList *pathList,
                                           const char *libFileName,
                                           void *attachment,
                                           JEM_DynaLib *libHandle);

/* <jemcc_start> */

/************************ Throwable Management ****************************/

/**
 * Structure which defines the private throwable details in a more
 * "program-friendly" fashion.  Methods within exception handlers can
 * access this structure using the following cast:
 *
 *   (JEMCC_ThrowableData *) &(object->objectData)
 */
typedef struct JEMCC_ThrowableData {
    JEMCC_Object *message;
    JEMCC_Object *causeThrowable;

    /* The following are intended to be "opaque" to non-VM entities */
    struct JEMCC_StackTraceEntryData *stackTrace;
    jint stackTraceDepth;
} JEMCC_ThrowableData;

/*
 * Standard throwable handling.  Most exceptions in Java are a basic class
 * which extends Throwable and simply carry a message and (as of 1.4)
 * possibly a cause exception.  These methods simply the throwing of
 * exceptions by instantiating an exception instance and defining this
 * basic information without the overhead of constructor invocations.
 * Of course, these methods should NOT be used in cases where more exact
 * constructors are to be used to initialize other information.
 */

/* See vmclasses.h for similar methods using VM core indices */

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * name.  Performs a simple class lookup for the provided name against the
 * classloader associated with the given object or the class to which the
 * currently executing method belongs.  See the 'jemcc.h' file for more 
 * details on what constitutes a "standard" throwable instance in JEMCC 
 * (essentially, a Throwable subclass which does nothing more than provide 
 * a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     srcObj - if non-NULL, load the throwable class from the classloader
 *              associated with the given object.  If NULL, use the classloader
 *              which is associated with the currently executing method
 *     className - the classname for the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError or ClassNotFoundException) due to difficulties
 *     in loading or creating the throwable instance.
 *     
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableByName(JNIEnv *env,
                                                  JEMCC_Object *srcObj,
                                                  const char *className,
                                                  JEMCC_Object *causeThrowable,
                                                  const char *msg);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * name.  Performs a simple class lookup for the provided name against the
 * classloader associated with the given object or the class to which the
 * currently executing method belongs.  See the 'jemcc.h' file for more 
 * details on what constitutes a "standard" throwable instance in JEMCC 
 * (essentially, a Throwable subclass which does nothing more than provide 
 * a new classname).
 *
 * This method is identical to the above method except that it allows for
 * vararg message construction.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     srcObj - if non-NULL, load the throwable class from the classloader
 *              associated with the given object.  If NULL, use the classloader
 *              which is associated with the currently executing method
 *     className - the classname for the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError or ClassNotFoundException) due to difficulties
 *     in loading or creating the throwable instance.
 *     
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableByNameV(JNIEnv *env,
                                                  JEMCC_Object *srcObj,
                                                  const char *className,
                                                  JEMCC_Object *causeThrowable,
                                                  ...);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * instance.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowable(JNIEnv *env,
                                               JEMCC_Class *throwclass,
                                               JEMCC_Object *causeThrowable,
                                               const char *msg);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * instance.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * This method is identical to the above method except that it allows for
 * vararg message construction.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableV(JNIEnv *env,
                                                JEMCC_Class *throwclass,
                                                JEMCC_Object *causeThrowable,
                                                ...);

/**
 * "Catch" an instance of a throwable class (based on an outstanding exception 
 * in the current environment) for the external class reference established 
 * during linking.  Will return and clear the pending exception if an exception
 * exists which is assignable to the indicated reference class instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classIdx - the index of the external throwable class reference to
 *                catch
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_CatchThrowableRef(JNIEnv *env,
                                                        jint classIdx);

/**
 * "Catch" an instance of a throwable class (based on an outstanding exception 
 * in the current environment).  Will return and clear the pending exception if
 * an exception exists which is assignable to the provided class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the throwable to be caught
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_CatchThrowable(JNIEnv *env,
                                                     JEMCC_Class *throwClass);

/* <jemcc_end> */

/**
 * Methods to manage the extraction and rethrow of throwables. TBD.
 */
JNIEXPORT JEMCC_Object *JNICALL JEM_ExtractSkeletonThrowable(JNIEnv *env);
JNIEXPORT void JNICALL JEM_ThrowSkeletonThrowable(JEMCC_Object *skeleton);

/*
 * Make this file the "one-stop" source for all of the VM definitions.
 */
#include "cpu.h"
#include "class.h"
#include "vmclasses.h"
#include "jvmenv.h"

#endif
