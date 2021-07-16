/**
 * Virtual machine and environment definitions for the JEMCC implementation.
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

#ifndef JEM_JVMENV_H
#define JEM_JVMENV_H 1

/* NOTE: this is auto-read by jem.h, so it shouldn't be directly read */

/* Definition of the private virtual machine structure */
typedef struct JEM_JavaVM {
    /* Base of the structure must be the standard VM pointer */
    struct JNIInvokeInterface *coreVM;

    /*** Remainder of structure is JEM specific VM details ***/

    /* Link list pointers to maintain VM instance records */
    struct JEM_JavaVM *previousVM, *nextVM;

    /* Pointer to linked list root of VM owned environments */
    struct JEM_JNIEnv *envList;

    /* Monitor to control multi-thread VM alteration access */
    JEMCC_SysMonitor *monitor;

    /* System class and library path data (VM specific) */
    JEM_PathEntryList classPath;
    JEM_PathEntryList libPath;
    JEMCC_Object *systemClassLoader;

    /* JEMCC/Bootstrap class/package container and indexed class table */
    JEMCC_HashTable jemccClassPackageTable;
    JEMCC_Class *coreClassTbl[JEMCC_VM_CLASS_TBL_SIZE];

    /* Dynamic loader used for JEMCC and JNI dynamic loading */
    JEM_DynaLibLoader libLoader;

    /* VM-local hashtable for intern()'ed java.lang.String data */
    JEMCC_HashTable internStringTable;

    /* VM-global runtime options (as passed via invocation arguments) */
    jint verboseDebugFlags;
} JEM_JavaVM;

#define VM_CLASS(index) (((JEM_JNIEnv *) env)->parentVM->coreClassTbl[(index)])

#define VERBOSE_CLASS(vm) (((JEM_JavaVM *) vm)->verboseDebugFlags != 0) && \
                          ((((JEM_JavaVM *) vm)->verboseDebugFlags & 1) != 0)
#define VERBOSE_GC(vm) (((JEM_JavaVM *) vm)->verboseDebugFlags != 0) && \
                        ((((JEM_JavaVM *) vm)->verboseDebugFlags & 2) != 0)
#define VERBOSE_JNI(vm) (((JEM_JavaVM *) vm)->verboseDebugFlags != 0) && \
                         ((((JEM_JavaVM *) vm)->verboseDebugFlags & 4) != 0)

/* Forward declaration of the object locking queue structure */
typedef struct JEM_ObjLockQueueEntry JEM_ObjLockQueueEntry;

/* Definition of the private working environment structure */
typedef struct JEM_JNIEnv {
    /* Base of the structure must be the standard env pointer */
    struct JNINativeInterface *coreEnv;

    /*** Remainder of structure is JEM specific environment details ***/

    /* Virtual machine this environment belongs to */
    struct JEM_JavaVM *parentVM;

    /* Link list pointers to maintain environment instance records */
    struct JEM_JNIEnv *previousEnv, *nextEnv;

    /* Reference to the native thread associated with this env instance */
    JEMCC_ThreadId envThread;

    /* Queue/monitor instances used by the lightweight object monitors */
    juint stateTxfrMode, stateTxfrSet;
    JEMCC_SysMonitor *objStateTxfrMonitor, *objLockMonitor;
    JEM_ObjLockQueueEntry *freeObjLockQueue;

    /* The last exception thrown by the processing by this environment */
    JEMCC_Object *pendingException;

    /* Sizing/reference information for a working buffer (thread-specific) */
    jbyte *envBuffer, *envEndPtr;
    jint envBufferLength, envBufferStringLength;

    /* CPU based information - frame stack data block and top frame instance*/
    void *frameStackBlock;
    jsize frameStackBlockSize;
    struct JEM_VMFrameExt *topFrame;

    /* Return structure for non-bytecode operations (no return stack) */
    JEMCC_ReturnValue nativeReturnValue;

    /* Allocation tracking for first and last object records in this env */
    void *firstAllocObjectRecord, *lastAllocObjectRecord;
} JEM_JNIEnv;

/* The object locking structure (defined here to allow cleanup) */
/* Note this uses entry count (rather than reentry) to support -ve flagging */
struct JEM_ObjLockQueueEntry {
    JEM_JNIEnv *parentEnv;
    JEM_ObjLockQueueEntry *nextEntry;
    juint objStateSet;
    jint entryCount;
};

/* <jemcc_start> */

/****************** Environment Buffer Management *******************/

/*
 * All of the following methods are designed for "short-term" working
 * buffer management, without incurring additional overhead from allocating
 * and releasing working memory.  Each "environment" in the JEMCC VM has
 * a "static" buffer which can be utilized through the following methods.
 * There are no multi-threading issues, as each environment instance is tied
 * to a single thread only.  These buffers are also only able to grow in
 * size - while this may waste memory for occasional large buffers, it
 * optimizes the size for repetitive operations.
 *
 * For the more Java oriented, these methods are equivalent to similar
 * methods against a StringBuffer instance attached to the current 
 * environment.
 */

/**
 * Ensure that the buffer has at least the specified number of bytes
 * allocated for use.
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
JNIEXPORT jbyte *JNICALL JEMCC_EnvEnsureBufferCapacity(JNIEnv *env,
                                                       jsize length);

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
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferInit(JNIEnv *env, jsize length);

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
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferAppend(JNIEnv *env, char *str);

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
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferAppendSet(JNIEnv *env, ...);

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
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferDup(JNIEnv *env);

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
JNIEXPORT char *JNICALL JEMCC_EnvStringBufferAppend(JNIEnv *env,
                                                    JEMCC_Object *string);

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
 *     in the current environment) or a toString() method call threw an
 *     exception.
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 *     Other exceptions as thrown in the toString() call.
 */
JNIEXPORT char *JNICALL JEMCC_EnvStringBufferAppendObj(JNIEnv *env,
                                                       JEMCC_Object *obj);

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
#define STR_OBJECT(obj) (void *) 0xdecaf, obj
JNIEXPORT char *JNICALL JEMCC_EnvStringBufferAppendSet(JNIEnv *env, ...);

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
JNIEXPORT JEMCC_Object *JNICALL JEMCC_EnvStringBufferToString(JNIEnv *env);

/* <jemcc_end> */

#endif
