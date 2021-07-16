/**
 * Methods for memory allocation, release and garbage collection.
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

/* Bits and bytes */
#define NONLOCAL_BIT 1

/* Forward declarations to actual garbage allocators/collectors */
static JEMCC_Object *JEM_AllocateObjectRecord(JNIEnv *env, juint totalSize);

#ifndef TEST_INTERNAL
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
#ifdef ENABLE_ERRORSWEEP
void *JEMCC_Malloc(JNIEnv *env, juint size) {
    void *retVal;

    if (JEM_CheckErrorSweep(ES_MEM) == JNI_TRUE) {
        retVal = NULL;
    } else {
        retVal = calloc(1, size);
    }

    if (retVal == NULL) {
        /* TODO throw the Memory exception! */
    }
    return retVal;
}
#else
void *JEMCC_Malloc(JNIEnv *env, juint size) {
    void *retVal = calloc(1, size);

    if (retVal == NULL) {
        /* TODO try to recover */
        /* TODO throw the Memory exception! */
    }
    return retVal;
}
#endif
#endif

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
void JEMCC_Free(void *block) {
    /* Right now, do the basic thing */
    free(block);
}

/**
 * Allocate the memory block for a java.lang.Object based instance, including
 * the internal attachment data, based on the core index value.  This does
 * NOT call any <init> constructors of the given class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to create an instance of
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
 */
JEMCC_Object *JEMCC_AllocateObjectIdx(JNIEnv *env, JEMCC_VMClassIndex idx,
                                      juint objDataSize) {
    return JEMCC_AllocateObject(env, VM_CLASS(idx), objDataSize);
}

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
JEMCC_Object *JEMCC_AllocateObject(JNIEnv *env, JEMCC_Class *classInst, 
                                   juint objDataSize) {
    JEMCC_Object *retObj = NULL;
    unsigned int totalSize = sizeof(JEMCC_Object);

    /* Ensure the class instance is initialized */
    if (JEMCC_InitializeClass(env, classInst) != JNI_OK) return NULL;

    /* Add packing space for object field information */
    if (classInst != NULL) {
        totalSize += classInst->classData->packedFieldSize;
    }

    /* Create the memory space */
    if ((retObj = JEM_AllocateObjectRecord(env, totalSize)) == NULL) {
        return NULL;
    }
    if (objDataSize > 0) {
        ((JEMCC_ObjectExt *) retObj)->objectData = JEMCC_Malloc(env, 
                                                                objDataSize);
        if (((JEMCC_ObjectExt *) retObj)->objectData == NULL) {
            return NULL;
        }
    }

    /* Initialize the main object structure elements */
    retObj->classReference = classInst;
    retObj->objStateSet = 0;

    return retObj;
}

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
 * cloneable (implements the Cloneable interface).  Nor does it perform
 * a NULL check on the provided object.
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
JEMCC_Object *JEMCC_CloneObject(JNIEnv *env, JEMCC_Object *obj) {
    JEMCC_Object *retObj = NULL;
    unsigned int totalSize = sizeof(JEMCC_Object);
    JEMCC_Class *classInst = obj->classReference;

    /* Determine object size and allocate appropriately */
    totalSize += classInst->classData->packedFieldSize;
    if ((retObj = JEM_AllocateObjectRecord(env, totalSize)) == NULL) {
        return NULL;
    }

    /* Perform the shallow clone */
    (void) memcpy(retObj, obj, totalSize);

    /* Handle array clone operation */
    if ((classInst->classData->accessFlags & ACC_ARRAY) != 0) {
        switch (((JEMCC_ArrayClass *) classInst)->typeDepthInfo) {
            case PRIMITIVE_BOOLEAN | 1:
            case PRIMITIVE_BYTE | 1:
                totalSize = ((JEMCC_ArrayObject *) obj)->arrayLength;
                break;
            case PRIMITIVE_CHAR | 1:
            case PRIMITIVE_SHORT | 1:
                totalSize = 2 * ((JEMCC_ArrayObject *) obj)->arrayLength;
                break;
            case PRIMITIVE_INT | 1:
                totalSize = 4 * ((JEMCC_ArrayObject *) obj)->arrayLength;
                break;
            case PRIMITIVE_LONG | 1:
            case PRIMITIVE_DOUBLE | 1:
                totalSize = 8 * ((JEMCC_ArrayObject *) obj)->arrayLength;
                break;
            default:
                /* Everything else is objects or nested arrays */
                totalSize = 4 * ((JEMCC_ArrayObject *) obj)->arrayLength;
                break;
        }
        ((JEMCC_ArrayObject *) retObj)->arrayData =
                                          JEMCC_Malloc(env, totalSize);
        if (((JEMCC_ArrayObject *) retObj)->arrayData == NULL) {
            return NULL;
        }

        /* Again, shallow copy of array contents */
        (void) memcpy(((JEMCC_ArrayObject *) retObj)->arrayData,
                      ((JEMCC_ArrayObject *) obj)->arrayData, totalSize);
    }

    return retObj;
}

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
void JEMCC_MarkNonLocalObject(JNIEnv *env, JEMCC_Object *object) {
    juint linkPtr;

    if (object != NULL) return;
    linkPtr = *((juint *) ((void *) object - sizeof(void *)));

    /**
     * Only set this bit when it is still local.  In that case, the local
     * thread still has control of the object and no thread concurrency
     * issues arise (GC will not see it yet).  Once it is non-local, no
     * such guarantees exist (garbage collection thread may be modifying
     * at same time).
     *
     * May be more expensive to do extra compare, but may also be cheaper
     * than universal compare_and_swap operation.
     */
    if ((linkPtr & NONLOCAL_BIT) != 0) return;
    *((juint *) ((void *) object - sizeof(void *))) = linkPtr | NONLOCAL_BIT;
}

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
JEMCC_Object *JEMCC_NewObjectInstance(JNIEnv *env, JEMCC_VMFrame *frame,
                                      JEMCC_Class *classInst, 
                                      const char *desc) {
    JEM_ClassData *classData = classInst->classData;
    JEM_ClassMethodData *conMethod = NULL;
    JEMCC_Object *retObj;
    int i;
    
    /* Ensure the class is initialized */
    if (JEMCC_InitializeClass(env, classInst) != JNI_OK) return NULL;

    /* Locate the constructor method instance */
    for (i = 0; i < classData->localMethodCount; i++) {
        if ((strcmp(classData->localMethods[i].name, "<init>") == 0) &&
            (strcmp(classData->localMethods[i].descriptorStr, desc) == 0)) {
            conMethod = &(classData->localMethods[i]);
            break;
        }
    }
    if (conMethod == NULL) {
        JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_NoSuchMethodException,
                                    NULL, classInst->classData->className, ".",
                                    "<init>", desc, NULL);
        return NULL;
    }

    /* Allocate the object */
    retObj = JEMCC_AllocateObject(env, classInst, 0);
    if (retObj == NULL) return NULL;

    /* Prepare for constructor call */
    if (conMethod->stackConsumeCount <= 1) {
       /* JEMCC_PUSH_STACK_OBJECT(frame, retObj); */
       ((frame->operandStackTop)++)->obj = retObj;
    } else {
       /* Shift arguments and push "this" object into first position */
       for (i = 0; i < conMethod->stackConsumeCount - 1; i++) {
           *(frame->operandStackTop - i) = *(frame->operandStackTop - i - 1);
       }
       (frame->operandStackTop)++;
       (frame->operandStackTop - conMethod->stackConsumeCount)->obj = retObj;
    }

    /* Make the call, returning the newly constructed object when complete */
    if (JEM_PushFrame(env, (jmethodID) conMethod, NULL) == JNI_OK) {
        JEM_ExecuteCurrentFrame(env, JNI_FALSE);
        if (((JEM_JNIEnv *) env)->pendingException != NULL) {
            return NULL;
        }
    }
    return retObj;
}

/**
 * Perform the allocation of a base object instance, adding it to the 
 * frame-specific garbage collection table.
 *
 * Note that, because this is environment specific, there are no
 * threading/concurrency issues in the record management.
 */
static JEMCC_Object *JEM_AllocateObjectRecord(JNIEnv *env, juint totalSize) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint linkPtr;
    void *retVal;

    /* Allocate the memory block (for now, standard malloc) */
    retVal = JEMCC_Malloc(env, totalSize + sizeof(void *));
    if (retVal == NULL) return NULL;

    /* Update the linked lists for allocation tracking (null flags) */
    if (jenv->firstAllocObjectRecord == NULL) {
        jenv->firstAllocObjectRecord = jenv->lastAllocObjectRecord = retVal;
        jenv->topFrame->firstAllocObjectRecord = retVal;
    } else {
        linkPtr = *((juint *) jenv->lastAllocObjectRecord);
        *((juint *) jenv->lastAllocObjectRecord) = 
                                     ((juint) retVal) | (linkPtr & 0x03);
        jenv->lastAllocObjectRecord = retVal;
        if (jenv->topFrame->firstAllocObjectRecord == NULL) 
                             jenv->topFrame->firstAllocObjectRecord = retVal;
    }

    /* Actual object is offset by tracking link */
    return (JEMCC_Object *) (retVal + sizeof(void *));
}

/**
 * Perform a flush of all local object instances for the current frame
 * (which is exiting/popping from the stack).  If specified, do *not*
 * flush the given return value or exception being processed.
 */
void JEM_FlushLocalFrameAllocations(JNIEnv *env, JEMCC_Object *retVal,
                                    JEMCC_Object *pendingException) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_ThrowableData *throwData;
    JEMCC_Object *currentObject;
    void *currentRecord, *lastRecord, *nextRecord;
    juint linkPtr;

    /* Start from the current frame's initial allocation */
    currentRecord = jenv->topFrame->firstAllocObjectRecord;
    if (currentRecord == NULL) return;

    /* Walk the linked list, moving into appropriate buckets */
    if (pendingException != NULL) throwData = (JEMCC_ThrowableData *)
                            ((JEMCC_ObjectExt *) pendingException)->objectData;
    lastRecord = NULL;
    while (currentRecord != NULL) {
        linkPtr = *((juint *) currentRecord);
        currentObject = (JEMCC_Object *) (currentRecord + sizeof(void *));
        if ((retVal != NULL) && (currentObject == retVal)) {
fprintf(stderr, "Returning object\n", currentObject);
            /* Move on to the next record */
            lastRecord = currentRecord;
            currentRecord = (void *) (linkPtr & 0xFFFFFFFC);
            continue;
        }
        if (pendingException != NULL) {
           if ((currentObject == pendingException) ||
               (currentObject == throwData->message) ||
               (currentObject == throwData->causeThrowable)) {
fprintf(stderr, "Pending exception (contents) %p\n", currentObject);
                /* Move on to the next record */
                lastRecord = currentRecord;
                currentRecord = (void *) (linkPtr & 0xFFFFFFFC);
                continue;
           }
        }

        /* Destroy local or move non-local records to GC list */
        nextRecord = (void *) (linkPtr & 0xFFFFFFFC);
        if ((linkPtr & NONLOCAL_BIT) != 0) {
fprintf(stderr, "Non-local object %p\n", currentObject);
        } else {
fprintf(stderr, "Local object %p\n", currentObject);
            /* Perform destruct sequence, including known objects */
        }

        /* Unlink from current chain, move to next record */
        if (lastRecord == NULL) {
        } else {
        *((juint *) jenv->lastAllocObjectRecord) = 
                                     ((juint) retVal) | (linkPtr & 0x03);
        }
        currentRecord = nextRecord;
    }
}
