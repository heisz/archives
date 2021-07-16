/**
 * Supporting methods for JEMCC VM class definitions/operations.
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
 * Common method for locking access to the definition hashtable in either
 * the virtual machine bootstrap classloader or a provided classloader
 * instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - if non-NULL, obtain exclusive access lock to the hashing
 *              tables in this classloader, otherwise lock the bootstrap
 *              hashtable in the VM
 *
 * Returns:
 *     JNI_OK - the classloader lock was successful
 *     JNI_ENOMEM - a memory allocation of the monitor failed
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
static jint JEM_LockClassLoader(JNIEnv *env, JEMCC_Object *loader) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;
    jint rc = JNI_OK;

    if (loader == NULL) {
        JEMCC_EnterSysMonitor(jvm->monitor);
    } else {
        rc = JEMCC_EnterObjMonitor(env, (jobject) loader);
    }

    return rc;
}

#define LOADER_UNLOCK(env, jvm, loader) \
if (loader == NULL) { \
    if (JEMCC_ExitSysMonitor(jvm->monitor) != JEMCC_MONITOR_OK) abort(); \
} else { \
    if (JEMCC_ExitObjMonitor(env, (jobject) loader) != JNI_OK) abort(); \
}

/**
 * Perform an unlock of the bootstrap or provided classloader (opposite
 * of LockClassLoader method above).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - if non-NULL, release exclusive access lock to the hashing
 *              tables in this classloader, otherwise release the bootstrap
 *              hashtable in the VM
 *
 * Exceptions:
 *     This routine is only used internally and should never encounter a
 *     lock error.  It will abort if it ever does.
 */
static void JEM_UnlockClassLoader(JNIEnv *env, JEMCC_Object *loader) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;

    LOADER_UNLOCK(env, jvm, loader);
}

/**
 * Convenience method to perform a wait operation for hashtable access on
 * the VM bootstrap classloader or the provided classloader.  Must be done
 * within a classloader lock block, as the wait condition test should be
 * done during exclusive access.
 *
 * This method will time out, to capture memory failures during re-lock
 * during initialization completion.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - if non-NULL, perform a wait on the monitor for the hashing
 *              tables in this classloader, otherwise wait on the bootstrap
 *              hashtable in the VM
 *
 * Exceptions:
 *     This routine is only used internally and should never encounter a
 *     lock error.  It will abort if it ever does.
 */
static void JEM_WaitOnClassLoader(JNIEnv *env, JEMCC_Object *loader) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;

    if (loader == NULL) {
        if (JEMCC_SysMonitorMilliWait(jvm->monitor, 
                                      200) == JEMCC_MONITOR_NOT_OWNER) abort();
    } else {
        /* TODO - add 'quiet' mode when interrupts are supported */
        if (JEMCC_ObjMonitorMilliWait(env, (jobject) loader, 
                                      200) != JNI_OK) abort();
    }
}

/**
 * Convenience method to notify waiting threads of completion of an operation
 * on the classloader (i.e. a class load/initialization has completed).  Must
 * be done within a classloader lock block, as the wait condition sets should
 * be done during exclusive access.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - if non-NULL, notify all waits on the monitor for the hashing
 *              tables in this classloader, otherwise notify on the bootstrap
 *              hashtable in the VM
 *
 * Exceptions:
 *     This routine is only used internally and should never encounter a
 *     lock error.  It will abort if it ever does.
 */
static void JEM_NotifyWaitingOnClassLoader(JNIEnv *env, JEMCC_Object *loader) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;

    if (loader == NULL) {
        if (JEMCC_SysMonitorNotifyAll(jvm->monitor) ==
                                              JEMCC_MONITOR_NOT_OWNER) abort();
    } else {
        if (JEMCC_ObjMonitorNotifyAll(env, (jobject) loader) != JNI_OK) abort();
    }
}

/**
 * Convenience method to grab a class instance from a given classloader.
 * Essentially does a lock/get/unlock to retrieve from the local hashtable
 * for the loader (i.e. does NOT perform the full Java classload sequence,
 * use JEMCC_LocateClass for that).  Note that this method will also detect
 * and wait on a class instance which is defined but in the progress of
 * being resolved.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - if non-NULL, lock and examine the associated loader hashtable,
 *              otherwise utilize the bootstrap hashtable in the VM
 *     className - the name of the class to be retrieved
 *     classInst - a class reference through which the requested class is
 *                 returned (if found)
 *
 * Returns:
 *     JNI_OK - the lookup occurred successfully and the requested class was
 *              found
 *     JNI_ERR - a circular class resolution case was detected (an exception
 *               has been thrown in the current environment)
 *     JNI_ENOMEM - a memory allocation error occurred
 *     JNI_EINVAL - the lookup was successful but the requested class was
 *                  not found (no exception is thrown in this case)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassCircularityError - the requested class is being resolved by
 *                             the current thread
 */
jint JEM_RetrieveClass(JNIEnv *env, JEMCC_Object *loader,
                       const char *className, JEMCC_Class **classInst) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    JEMCC_Class *clInst = NULL;
    jint rc;

    /* Lock the classloader/VM for exclusive access */
    if (loader == NULL) {
        JEMCC_EnterSysMonitor(jvm->monitor);
    } else {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
        rc = JEMCC_EnterObjMonitor(env, (jobject) loader);
        if (rc != JNI_OK) return rc;
    }

    /* Repeat until class not found, class resolved or errors occur */
    while (clInst == NULL) {
        /* Perform the lookup */
        clInst = (JEMCC_Class *) JEMCC_HashGetEntry(env, hash,
                                                    (void *) className,
                                                    JEMCC_ClassNameHashFn,
                                                    JEMCC_ClassNameEqualsFn);

        /* Capture in-progress conditions */
        if (clInst != NULL) {
            if (clInst->classData->resolveInitState == 
                                         JEM_CLASS_RESOLVE_IN_PROGRESS) {
                if (clInst->classData->resolveInitThread ==
                                            ((JEM_JNIEnv *) env)->envThread) {
                    /* Circular resolve condition, unlock and throw */
                    LOADER_UNLOCK(env, jvm, loader);
                    JEMCC_ThrowStdThrowableIdx(env,
                                            JEMCC_Class_ClassCircularityError,
                                            NULL, className);
                    return JNI_ERR;
                }

                /* Wait on classloader for class and try again */
                /* Note that this condition cannot happen for bootstrap */
                /* TODO - add 'quiet' mode when interrupts are supported */
                if (JEMCC_ObjMonitorMilliWait(env, (jobject) loader,
                                              200) != JNI_OK) abort();
                clInst = NULL;
                continue;
            }

            /* Resolved instance discovered, stop searching */
            break;
        } else {
            /* Not available, stop searching */
            break;
        }
    }

    /* Lookup complete, unlock */
    LOADER_UNLOCK(env, jvm, loader);

    /* Return as appropriate */
    *classInst = clInst;
    return ((clInst == NULL) ? JNI_EINVAL: JNI_OK);
}

/* Read package methods, which rely on ClassLoader lock methods above */
#include "package.c"

/**
 * Locate a method instance in the given class definition, using the
 * name and descriptor of the method in question.  Only scans the primary
 * linked method table (inheritance and interfaces accounted for), not
 * the local method table (in other words, this method will not locate 
 * the <init> or <clinit> methods).
 *
 * Parameters:
 *     classData - the class definition to search for the method
 *     name - the name of the method to find
 *     descriptor - the descriptor of the method to find
 *
 * Returns:
 *     The method definition reference if the method was found, NULL otherwise.
 */
JEM_ClassMethodData *JEM_LocateClassMethod(JEM_ClassData *classData,
                                           const char *name,
                                           const char *descriptor) {
    JEM_ClassMethodData **retRef = classData->methodLinkTables[0];
    int i;

    for (i = classData->classMethodCount; i > 0; i--, retRef++) {
        if ((strcmp(name, (*retRef)->name) == 0) &&
                   (strcmp(descriptor, (*retRef)->descriptorStr) == 0)) {
            return *retRef;
        }
    }

    return NULL;
}

/**
 * Locate a field instance in the given class definition, using the
 * name and descriptor of the field in question.  Will scan local field
 * definitions, then recurse over superinterfaces and superclasses
 * according to the sequence defined in the Java VM specification.
 *
 * Parameters:
 *     classData - the class definition to search for the field
 *     name - the name of the field to find
 *     descriptor - the descriptor of the field to find
 *
 * Returns:
 *     The method definition reference if the method was found, NULL otherwise.
 */
JEM_ClassFieldData *JEM_LocateClassField(JEM_ClassData *classData,
                                         const char *name,
                                         const char *descriptor) {
    JEM_ClassFieldData *retRef = classData->localFields;
    int i;

    /* First try the local definitions */
    for (i = classData->localFieldCount; i > 0; i--, retRef++) {
        if ((strcmp(name, retRef->name) == 0) &&
                   (strcmp(descriptor, retRef->descriptorStr) == 0)) {
            return retRef;
        }
    }

    /* Recursively scan immediate interfaces and superclass */
    if (classData->assignmentCount != 0) {
        for (i = classData->interfaceCount; i >= 0; i--) {
            retRef = JEM_LocateClassField(classData->assignList[i]->classData,
                                          name, descriptor);
            if (retRef != NULL) return retRef;
        }
    }

    return NULL;
}

/**
 * Allocate the memory block for a java.lang.Class instance.  This method
 * is similar to JEMCC_AllocateObject, but does not create the Class/Object
 * instance in the standard (garbage-collectible) allocator.  Class instances
 * must be manually destroyed (through DestroyClassInstance), garbage
 * collection of classes (if enabled) only occurs on the destruction of a
 * ClassLoader instance, by definition.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     NULL if a memory allocation failed, otherwise the base class structure.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JEMCC_Object *JEM_AllocateClass(JNIEnv *env) {
    JEMCC_Object *clInst;

    /* Allocate the data structure */
    clInst = (JEMCC_Object *) JEMCC_Malloc(env, sizeof(JEMCC_Class));
    if (clInst == NULL) return NULL;
    ((JEMCC_ObjectExt *) clInst)->objectData = JEMCC_Malloc(env,
                                                        sizeof(JEM_ClassData));
    if (((JEMCC_ObjectExt *) clInst)->objectData == NULL) {
        JEMCC_Free(clInst);
        return NULL;
    }

    /* Initialize the main object structure elements */
    clInst->classReference = VM_CLASS(JEMCC_Class_Class);
    clInst->objStateSet = 0;

    return clInst;
}

/** 
 * Internal method to "mangle" the class hierarchy information (superclasses
 * and interfaces) and the class method information to build the inheritance
 * chain information and the method/interface mapping tables.
 *
 * Note: this method behaves differently based on a JEMCC or bytecode
 * class instance.  For JEMCC classes, an abstract/interface linkage error
 * is cause for immediate failure.  For a bytecode class, this method does
 * not throw an exception or return an error if an abstract method
 * implementation is missing (non-abstract class), but sets the
 * resolveInitState of the class to JEM_CLASS_ABSTRACT_ERROR to force the
 * throwing of AbstractMethodError during initialization (as per the JVM
 * specification).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class instance being constructed which will contain
 *                 the inheritance chain and method mapping tables
 *     superClass - the direct superclass of this class.  Must only be NULL
 *                  for the java.lang.Object class
 *     interfaces - an array of direct interfaces for this class
 *     interfaceCount - the number of interfaces in the interfaces array
 *     methods - an array of "local" method definitions for this class
 *     methodCount - the number of method definitions in the methods array
 *
 * Returns:
 *     JNI_OK - the package hierarchy data has been successfully created
 *     JNI_ERR - this is a JEMCC class instance and an abstract method
 *               linkage failure has occurred (an AbstractMethodError has
 *               been thrown in the current environment)
 *     JNI_ENOMEM - a memory allocation has failed during the creation of the
 *                  chain/linkage tables and an OutOfMemory error has been
 *                  thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     AbstractMethodError - a non-abstract JEMCC class is missing an "abstract"
 *                           method implementation
 */
jint JEM_BuildClassHierData(JNIEnv *env, JEMCC_Class *classInst,
                            JEMCC_Class *superClass,
                            JEMCC_Class **interfaces, jsize interfaceCount,
                            JEM_ClassMethodData *methods, jsize methodCount) {
    int i, j, tag, index, recordCount, argCount, classInitMethodIdx;
    JEM_ClassData *classData = classInst->classData;
    JEM_ClassData *superClassData, *baseClassData, *ifClassData, *tClassData;
    JEM_ClassMethodData *methodPtr, methodSwap;
    JEM_DescriptorData *argDesc;
    JEMCC_Class **tptr, **lptr;

    /* Extract the superclass/interface instance/method/field references */
    baseClassData = superClassData = NULL;
    if ((classData->accessFlags & ACC_INTERFACE) != 0) {
        /* Determine optimum superinterface copy for interface extension */
        argCount = 0;
        baseClassData = superClassData = superClass->classData;
        argCount = baseClassData->classMethodCount;
        classData->packedFieldSize = 0;
        for (i = 0; i < interfaceCount; i++) {
            if (interfaces[i]->classData->classMethodCount > argCount) {
                baseClassData = interfaces[i]->classData;
                argCount = baseClassData->classMethodCount;
            }
        }
    } else if (superClass != NULL) {
        baseClassData = superClassData = superClass->classData;
        classData->packedFieldSize = superClassData->packedFieldSize;
    } else {
        /* NB: this should only occur for java.lang.Object and primitives */
        classData->packedFieldSize = 0;
    }

    /* Make allowance for native JEMCC native instance data */
    if ((classData->accessFlags & ACC_NATIVE_DATA) != 0) {
        classData->packedFieldSize += 4;
    }

    /* Determine the maximum assignment record count */
    recordCount = interfaceCount + 1;
    if (superClass != NULL) {
        recordCount += superClassData->assignmentCount;
    }
    for (i = 0; i < interfaceCount; i++) {
        ifClassData = interfaces[i]->classData;
        /* Note: don't multiply allocate Object reference */
        recordCount += ifClassData->assignmentCount - 1;
    }
 
    /* Create assignment table and place superclass in root record */
    classData->assignList = (JEMCC_Class **)
                JEMCC_Malloc(env, (recordCount + 1) * sizeof(JEMCC_Class *));
    if (classData->assignList == NULL) return JNI_ENOMEM;
    *(classData->assignList) = superClass;

    /* Construct primary method linkage list, based on superclass list */
    classData->methodLinkTables = (JEM_ClassMethodData ***)
         JEMCC_Malloc(env, (recordCount + 1) * sizeof(JEM_ClassMethodData **));
    if (classData->methodLinkTables == NULL) return JNI_ENOMEM;
    recordCount = methodCount;
    if (superClass != NULL) recordCount += superClassData->classMethodCount;
    if ((classData->accessFlags & ACC_ABSTRACT) != 0) {
        /* Prepare for possible synthetic abstract entries */
        for (i = 0; i < interfaceCount; i++) {
            ifClassData = interfaces[i]->classData;
            /* Note: don't multiply allocate Object method set */
            if (ifClassData->classMethodCount > 10) {
                recordCount += ifClassData->classMethodCount - 10;
            }
        }
    }

    /* Allocate and clone the superclass/superinterface method list */
    if (recordCount != 0) {
        classData->methodLinkTables[0] = (JEM_ClassMethodData **)
                JEMCC_Malloc(env, recordCount * sizeof(JEM_ClassMethodData *));
        if (classData->methodLinkTables[0] == NULL) return JNI_ENOMEM;
    }
    if (baseClassData != NULL) {
        classData->classMethodCount = baseClassData->classMethodCount;
        (void) memcpy(classData->methodLinkTables[0],
                      baseClassData->methodLinkTables[0],
                      baseClassData->classMethodCount * 
                                        sizeof(JEM_ClassMethodData *));
    } else {
        classData->classMethodCount = 0;
    }
    classData->syntheticMethodCount = 0;

    /* Augment with the local method information */
    classInitMethodIdx = -1;
    for (i = 0; i < methodCount; i++) {
        /* <init>, <clinit> methods should never be mapped (not inheritable) */
        if (*(methods[i].name) == '<') {
            if (strcmp(methods[i].name, "<clinit>") == 0) {
                classInitMethodIdx = i;
            }
            methods[i].methodIndex = -1;
            continue;
        }

        /* Push the method into the appropriate slot in method table */
        methodPtr = JEM_LocateClassMethod(classData, methods[i].name,
                                          methods[i].descriptorStr);
        if (methodPtr != NULL) {
            /* Override of superclass method - capture/replace method index */
            index = methodPtr->methodIndex;
            classData->methodLinkTables[0][index] = &(methods[i]);
            methods[i].methodIndex = index;
        } else {
            /* New method, add to core method list */
            index = classData->classMethodCount++;
            classData->methodLinkTables[0][index] = &(methods[i]);
            methods[i].methodIndex = index;
        }
    }

    /* Ensure that the class initializer is the first method */
    if (classInitMethodIdx > 0) {
        methodSwap = methods[0];
        methods[0] = methods[classInitMethodIdx];
        methods[classInitMethodIdx] = methodSwap;

        /* Keep the method cross-reference table in order */
        if (methodSwap.methodIndex >= 0) {
            classData->methodLinkTables[0][methodSwap.methodIndex] = 
                                                &(methods[classInitMethodIdx]);
        }
    }

    /* Copy interface information list */
    if (interfaceCount != 0) {
        (void) memcpy(&(classData->assignList[1]), interfaces, 
                      interfaceCount * sizeof(JEMCC_Class *));
    }
    classData->interfaceCount = interfaceCount;

    /* Construct the direct interface method cross-mappings */
    for (i = 0; i < interfaceCount; i++) {
        ifClassData = interfaces[i]->classData;
        recordCount = ifClassData->classMethodCount;
        classData->methodLinkTables[i + 1] = (JEM_ClassMethodData **)
                JEMCC_Malloc(env, recordCount * sizeof(JEM_ClassMethodData *));
        if (classData->methodLinkTables[i + 1] == NULL) return JNI_ENOMEM;

        if (ifClassData == baseClassData) {
            /* Shortcut for optimum superinterface - just copy index entries */
            for (j = 0; j < recordCount; j++) {
                index = baseClassData->methodLinkTables[0][j]->methodIndex;
                methodPtr = classData->methodLinkTables[0][index];
                classData->methodLinkTables[i + 1][j] = methodPtr;
            }
        } else {
            /* Locate primary method for interface mapping instance */
            for (j = 0; j < recordCount; j++) {
                methodPtr = ifClassData->methodLinkTables[0][j];
                methodPtr = JEM_LocateClassMethod(classData, methodPtr->name,
                                                  methodPtr->descriptorStr);
                if (methodPtr == NULL) {
                    if ((classData->accessFlags & ACC_ABSTRACT) != 0) {
                        /* Create synthetic record for abstract index */
                        methodPtr = (JEM_ClassMethodData *) JEMCC_Malloc(env,
                                                  sizeof(JEM_ClassMethodData));
                        if (methodPtr == NULL) return JNI_ENOMEM;
                        *methodPtr = *(ifClassData->methodLinkTables[0][j]);
                        methodPtr->accessFlags |= ACC_SYNTHETIC;
                        methodPtr->parentClass = classInst;

                        /* Add it to the core method table */
                        index = classData->classMethodCount++;
                        classData->methodLinkTables[0][index] = methodPtr;
                        methodPtr->methodIndex = index;
                        classData->syntheticMethodCount++;
                    } else {
                        if ((classData->accessFlags & ACC_JEMCC) != 0) {
                            methodPtr = ifClassData->methodLinkTables[0][j];
                            JEMCC_ThrowStdThrowableIdxV(env, 
                                        JEMCC_Class_AbstractMethodError, NULL,
                                        "Concrete class, method undefined: ",
                                        classData->className, ".", 
                                        methodPtr->name, NULL);
                            return JNI_ERR;
                        }

                        /* Mark for subsequent initialization error */
                        classData->resolveInitState = JEM_CLASS_ABSTRACT_ERROR;
                    }
                }
                classData->methodLinkTables[i + 1][j] = methodPtr;
            }
        }
    }

    /* Build full assignment list, avoiding duplicates */
    /* Yes, this is N^2, but assignment lists shouldn't be toooo long, */
    /* and future assignment tests would be N^2 */
    recordCount = (superClass == NULL) ? 0 : 1;
    recordCount += interfaceCount;
    if (superClass != NULL) {
        tptr = superClassData->assignList;
        while (*tptr != NULL) {
            lptr = classData->assignList;
            while (*lptr != NULL) {
                if (*lptr == *tptr) break;
                lptr++;
            }
            if (*lptr == NULL) {
                *lptr = *tptr;
                tClassData = (*tptr)->classData;
                if (((tClassData->accessFlags) & ACC_INTERFACE) != 0) {
                    /* Superclass interface, grab superclass mapping table */
                    index = tptr - superClassData->assignList;
                    classData->methodLinkTables[recordCount++] = 
                                 superClassData->methodLinkTables[index];
                } else {
                    /* Superclass object, method map is identical to local */
                    classData->methodLinkTables[recordCount++] = 
                                        classData->methodLinkTables[0];
                }
            }
            tptr++;
        }
    }
    for (i = 0; i < interfaceCount; i++) {
        tptr = interfaces[i]->classData->assignList;
        while (*tptr != NULL) {
            lptr = classData->assignList;
            while (*lptr != NULL) {
                if (*lptr == *tptr) break;
                lptr++;
            }
            if (*lptr == NULL) {
                *lptr = *tptr;
                classData->methodLinkTables[recordCount++] = 
                                        classData->methodLinkTables[i + 1];
            }
            tptr++;
        }
    }

    /* Determine the total assignment count */
    recordCount = 0;
    lptr = classData->assignList;
    while (*lptr != NULL) {
        lptr++;
        recordCount++;
    }
    classData->assignmentCount = recordCount;

    /* Common method point to define the stack argument consumption length */
    for (i = 0; i < methodCount; i++) {
        argDesc = methods[i].descriptor->method_info.paramDescriptor;
        for (j = 0, argCount = 0; j < 256; j++, argDesc++) {
            tag = argDesc->generic.tag;
            if (tag == DESCRIPTOR_EndOfList) break;
            switch (tag) {
                case BASETYPE_Boolean:
                case BASETYPE_Byte:
                case BASETYPE_Char:
                case BASETYPE_Int:
                case BASETYPE_Float:
                case BASETYPE_Short:
                    argCount++;
                    break;
                case DESCRIPTOR_ObjectType:
                case DESCRIPTOR_ArrayType:
                    argCount++;
                    break;
                case BASETYPE_Long:
                case BASETYPE_Double:
                    argCount += 2;
                    break;
            }
        }
        if ((methods[i].accessFlags & ACC_STATIC) == 0) argCount++;
        methods[i].stackConsumeCount = argCount;
    }

    /* Finally, save the provided local method tables */
    classData->localMethods = methods;
    classData->localMethodCount = methodCount;
    
    return JNI_OK;
}

/* Ordering list of elements in order of size */
#define PACK_LIST_COUNT 11
static int packList[] = { 0 /* JEMCC predefined entries */,
                          BASETYPE_Byte, BASETYPE_Boolean, BASETYPE_Char,
                          BASETYPE_Short, BASETYPE_Int, BASETYPE_Float,
                          DESCRIPTOR_ObjectType, DESCRIPTOR_ArrayType,
                          BASETYPE_Long, BASETYPE_Double };

/**
 * Internal method to perform the actual packing (offset determination) of
 * the field data elements to ensure proper machine byte alignment.  Called
 * by the PackClassFieldData method for both the class and instance fields.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     className - name of the class (for error reporting)
 *     offset - the starting offset for the fields.  Zero for the static
 *              buffer or the size of the superclass instance area for
 *              instance fields.
 *     packStatic - if JNI_TRUE, pack the static class fields, otherwise
 *                  pack the instance fields
 *     fields - an array of the local field definitions for the class
 *     fieldCount - the number of field definitions in the fields array
 *
 * Returns:
 *     The offset value to the next byte following the last field.  This
 *     value indicates the total buffer size for the static fields and the
 *     next offset/object size for instance fields.  This method will
 *     also return -1 if a predefined offset value is incompatible with
 *     alignment/sizing restrictions (an exception will have been thrown).
 *
 * Exceptions:
 *     ClassFormatError - a JEMCC predefined field offset was incompatible
 *                        with alignment requirements or prior size limits
 */
static int packFields(JNIEnv *env, char *className, int offset, 
                      jboolean packStatic, JEM_ClassFieldData *fields, 
                      jsize fieldCount) {
    int i, type, toffset, typeIndex, size, alignment;
    JEMCC_ObjectExt *dummyObj = NULL;
    int objDataOffset = (int) (&(dummyObj->objectData));

    /* Account for base object structure size only for non-static */
    if (packStatic != JNI_FALSE) objDataOffset = 0;

    /* Scan packing data in order of datum sizes */
    offset += objDataOffset;
    for (typeIndex = 0; typeIndex < PACK_LIST_COUNT; typeIndex++) {
        type = packList[typeIndex];
        for (i = 0; i < fieldCount; i++) {
            /* Only do static, or not, according to request flag */
            if (packStatic != JNI_FALSE) {
                if ((fields[i].accessFlags & ACC_STATIC) == 0) continue;
            } else {
                if ((fields[i].accessFlags & ACC_STATIC) != 0) continue;
            }

            /* Pack fields according to type (JEMCC first, watch for native) */
            if (fields[i].fieldOffset < 0) {
                if (fields[i].name == NULL) {
                    JEMCC_ThrowStdThrowableIdxV(env, 
                                        JEMCC_Class_ClassFormatError, NULL,
                                        "Invalid field offset (private): ",
                                        className, ".", fields[i].name, NULL);
                    return -1;
                }
                if (typeIndex == 0) continue;
                if (fields[i].descriptor->generic.tag != type) continue;
            } else {
                if (typeIndex != 0) continue;

                if (fields[i].name == NULL) {
                    offset += fields[i].fieldOffset;
                    continue;
                }
            }

            /* Determine datum size and alignment factor */
            size = 0;
            alignment = ALIGNMENT_OF_JOBJECT;
            switch (fields[i].descriptor->generic.tag) {
                case BASETYPE_Byte:
                    size = sizeof(jbyte);
                    alignment = ALIGNMENT_OF_JBYTE;
                    break;
                case BASETYPE_Boolean:
                    size = sizeof(jboolean);
                    alignment = ALIGNMENT_OF_JBOOLEAN;
                    break;
                case BASETYPE_Char:
                    size = sizeof(jchar);
                    alignment = ALIGNMENT_OF_JCHAR;
                    break;
                case BASETYPE_Short:
                    size = sizeof(jshort);
                    alignment = ALIGNMENT_OF_JSHORT;
                    break;
                case BASETYPE_Int:
                    size = sizeof(jint);
                    alignment = ALIGNMENT_OF_JINT;
                    break;
                case BASETYPE_Float:
                    size = sizeof(jfloat);
                    alignment = ALIGNMENT_OF_JFLOAT;
                    break;
                case DESCRIPTOR_ObjectType:
                    size = sizeof(jobject);
                    alignment = ALIGNMENT_OF_JOBJECT;
                    break;
                case DESCRIPTOR_ArrayType:
                    size = sizeof(jobject);
                    alignment = ALIGNMENT_OF_JOBJECT;
                    break;
                case BASETYPE_Long:
                    size = sizeof(jlong);
                    alignment = ALIGNMENT_OF_JLONG;
                    break;
                case BASETYPE_Double:
                    size = sizeof(jdouble);
                    alignment = ALIGNMENT_OF_JDOUBLE;
                    break;
            }

            /* Handle predefined offset values for JEMCC structures */
            if (fields[i].fieldOffset < 0) {
                /* Modify offset to correct alignment factor, and store it */
                offset = ((offset + alignment - 1) / alignment) * alignment;
                fields[i].fieldOffset = offset - objDataOffset;
            } else {
                /* Validate predefined offset values */
                toffset = fields[i].fieldOffset + objDataOffset;
                if (toffset < offset) {
                    JEMCC_ThrowStdThrowableIdxV(env, 
                                        JEMCC_Class_ClassFormatError, NULL,
                                        "Invalid field offset (overlap): ",
                                        className, ".", fields[i].name, NULL);
                    return -1;
                }
                /* And the field alignment */
                if ((((int) (toffset / alignment)) * alignment) != toffset) {
                    JEMCC_ThrowStdThrowableIdxV(env, 
                                        JEMCC_Class_ClassFormatError, NULL,
                                        "Invalid field offset (align): ",
                                        className, ".", fields[i].name, NULL);
                    return -1;
                }
                offset = toffset;
            }

            /* And account for its size */
            offset += size;
        }
    }

    /* All done, send back current value */
    return offset - objDataOffset;
}

/**
 * Internal method to pack the class field information, determining the
 * appropriate offsets for each field element to ensure proper byte
 * alignment of the various field data types.  Handles both the instance
 * fields and the static fields, as well as creating the static field
 * storage area.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class for which the fields are being packed/allocated
 *     fields - an array of field definitions for this class
 *     fieldCount - the number of field definitions in the fields array
 *
 * Returns:
 *     JNI_OK - the packing of the field information completed successfully
 *     JNI_ERR - an alignment/size error occurred using predefined JEMCC
 *               class field offsets (ClassFormatError will have been thrown
 *               in the current environment)
 *     JNI_ENOMEM - a memory allocation has failed during the creation of the
 *                  field tables and an OutOfMemory error has been thrown
 *                  in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - a predefined field offset was invalid
 */
jint JEM_PackClassFieldData(JNIEnv *env, JEMCC_Class *classInst,
                            JEM_ClassFieldData *fields, jsize fieldCount) {
    int i, j, offset;
    JEM_ClassData *classData = classInst->classData;

    /* First, pack the instance fields and update class size */
    offset = packFields(env, classData->className, classData->packedFieldSize, 
                        JNI_FALSE, fields, fieldCount);
    if (offset < 0) return JNI_ERR;
    classData->packedFieldSize = offset;

    /* Then, pack the static fields and build the class static buffer */
    offset = packFields(env, classData->className, 0, JNI_TRUE, 
                        fields, fieldCount);
    if (offset < 0) return JNI_ERR;
    if (offset != 0) {
        classInst->staticData = JEMCC_Malloc(env, offset);
        if (classInst->staticData == NULL) return JNI_ENOMEM;
    }

    /* Collapse out the NULL private data placeholders */
    for (i = 0, j = 0; j < fieldCount; j++) {
        if (fields[j].name == NULL) continue;
        if (i == j) { i++; continue; }
        fields[i++] = fields[j];
    }
    fieldCount = i;

    /* Finally, save the provided local field tables */
    classData->localFields = fields;
    classData->localFieldCount = fieldCount;
    
    return JNI_OK;
}

/**
 * Store/define a class instance in the given classloader.  This method
 * will not replace an already existing class, but handles such an error
 * case differently depending on whether it is an internal (JEMCC) or
 * bytecode class definition.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to receive the class instance.  If
 *              NULL, the VM bootstrap classloader is used
 *     classInst - the class to be stored into the classloader
 *     existingClass - if non-NULL and the class being stored is already in
 *                     the classloader, this reference is used to return the
 *                     already existing class
 *     internalClass - if JNI_TRUE, this is an internal VM class (either
 *                     an array or JEMCC class) and no exception is thrown
 *                     on a duplication
 *
 * Returns:
 *     JNI_OK - the class definition in the classloader table was successful
 *     JNI_ERR - a class already exists with the given classname and no
 *               insertion occurred (the existing class instance is returned
 *               through the existingClass reference, if defined).  If the
 *               internalClass flag is JNI_FALSE, a LinkageError exception
 *               has been thrown in the current environment
 *     JNI_ENOMEM - a memory allocation has failed during the insertion of
 *                  the class and an OutOfMemory error has been thrown in the
 *                  current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     LinkageError - a class already exists in the classloader with the given
 *                    name.  This is only thrown if the internalClass flag is
 *                    JNI_FALSE
 */
jint JEM_ClassNameSpaceStore(JNIEnv *env, JEMCC_Object *loader,
                             JEMCC_Class *classInst,
                             JEMCC_Class **existingClass,
                             jboolean internalClass) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    char *className = classInst->classData->className;
    JEMCC_Class *origClass;
    jint rc;

    /* Define the origin of this class for subsequent reference handling */
    classInst->classData->classLoader = loader;

    /* Debug */
    if (VERBOSE_CLASS(jvm)) {
        if ((classInst->classData->accessFlags & ACC_JEMCC) != 0) {
            (void) fprintf(stderr, "[Defined %s from native JEMCC src]\n",
                           className);
        } else {
            (void) fprintf(stderr, "[Loaded %s]\n", className);
        }
    }

    /* If loader is specified, store in the loader namespace table */
    if (loader != NULL) {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
    }

    /* Prevent multithread conflicts */
    if (JEM_LockClassLoader(env, loader) != JNI_OK) return JNI_ENOMEM;

    /* Insert the record, avoiding an already existing instance */
    rc = JEMCC_HashInsertEntry(env, hash, (void *) className, classInst,
                               NULL, (void **) &origClass,
                               JEMCC_ClassNameHashFn, JEMCC_ClassNameEqualsFn);
    if (rc != JNI_OK) {
        JEM_UnlockClassLoader(env, loader);
        if (rc == JNI_ENOMEM) {
            if (existingClass != NULL) *existingClass = NULL;
            return JNI_ENOMEM;
        }

        if (existingClass != NULL) *existingClass = origClass;
        if (internalClass == JNI_FALSE) {
            JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_LinkageError, NULL,
                                        "Definition of existing class: ",
                                        className, NULL);
        }
        return JNI_ERR;
    }

    /* All done, release and return */
    JEM_UnlockClassLoader(env, loader);
    if (existingClass != NULL) *existingClass = NULL;

    return JNI_OK;
}

/**
 * Remove the given class instance from the classloader it belongs to.
 * There is no effect/exception if the given class instance no longer 
 * belongs to the owner classloader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class to be removed from its owner classloader
 */
void JEM_ClassNameSpaceRemove(JNIEnv *env, JEMCC_Class *classInst) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    JEMCC_Object *loader = classInst->classData->classLoader;
    char *className = classInst->classData->className;

    /* If non-NULL loader, remove from there instead of vm bootstrap */
    if (loader != NULL) {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
    }

    /* Prevent multithread conflicts */
    if (JEM_LockClassLoader(env, loader) != JNI_OK) return;

    /* Asta la vista, baby (but do so safely)! */
    (void) JEMCC_HashScanRemoveEntry(env, hash, (void *) className,
                                     JEMCC_ClassNameHashFn,
                                     JEMCC_ClassNameEqualsFn);

    /* Release the lock */
    JEM_UnlockClassLoader(env, loader);
}

/* Note: this method is defined here for access to classloader lock methods */

/**
 * Register a set of JEMCC classes into the specified classloader (or the VM
 * bootstrap loader).  This is identical to the RegisterClass method above,
 * except that it handles the definition within a single classloader lock
 * to avoid any errors due to already existing class definitions or race
 * conditions.  Due to the complexities associated with relinking partial
 * matches, this operation is completely atomic - if any class is duplicated
 * the entire set is discarded and replaced with existing instances.  If a
 * memory error occurs, all of the provided classes are destroyed and removed
 * from the classloader hashtable if partially stored.
 *
 * Note that this method should only be used in exactly reproducible
 * situations for the minimal set of cross-linked class instances.  It cannot
 * overcome instances where the class list is partially defined - in such
 * a situation, the updated array will contain NULL values.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the registered classes (or
 *              from which the replacement(s) are to come).  If NULL, the
 *              VM bootstrap classloader is used instead
 *     regClasses - a reference to an array of the classes to be registered.
 *                  If any class already exists, all are destroyed and the
 *                  references to the already existing instances are placed
 *                  in the array
 *     regClassCount - the number of classes specified in the regClasses array
 *
 * Returns:
 *     JNI_OK - the definition/replacement of the classes was successful
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEMCC_RegisterClasses(JNIEnv *env, JEMCC_Object *loader,
                           JEMCC_Class **regClasses, jsize regClassCount) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    JEMCC_Class *classInst, *origClass;
    void *className;
    jint i, j, rc;

    /* If non-NULL loader, insert there instead of vm bootstrap */
    if (loader != NULL) {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
    }

    /* Prevent multithread conflicts */
    if (JEM_LockClassLoader(env, loader) != JNI_OK) return JNI_ENOMEM;

    /* Scan for already existing instances */
    for (i = 0; i < regClassCount; i++) {
        classInst = regClasses[i];
        className = (void *) classInst->classData->className;
        if (JEMCC_HashGetEntry(env, hash, className, JEMCC_ClassNameHashFn, 
                               JEMCC_ClassNameEqualsFn) != NULL) {
            /* Duplicate found, delete and replace */
            for (j = 0; j < regClassCount; j++) {
                classInst = regClasses[j];
                className = (void *) classInst->classData->className;
                regClasses[j] = (JEMCC_Class *) 
                                       JEMCC_HashGetEntry(env, hash, className,
                                               JEMCC_ClassNameHashFn, 
                                               JEMCC_ClassNameEqualsFn);
                JEM_DestroyClassInstance(env, classInst);
            }

            /* Release lock and return */
            JEM_UnlockClassLoader(env, loader);
            return JNI_OK;
        }
    }

    /* All new and unique, store into hashtable */
    for (i = 0; i < regClassCount; i++) {
        classInst = regClasses[i];
        className = (void *) classInst->classData->className;
        rc = JEMCC_HashInsertEntry(env, hash, className,
                                   classInst, NULL, (void **) &origClass,
                                   JEMCC_ClassNameHashFn, 
                                   JEMCC_ClassNameEqualsFn);
        if (rc != JNI_OK) {
            if (rc == JNI_ENOMEM) {
                /* One fails, all fail (remember to remove already defined) */
                for (j = 0; j < regClassCount; j++) {
                    classInst = regClasses[j];
                    className = (void *) classInst->classData->className;
                    if (j < i) {
                        (void) JEMCC_HashRemoveEntry(env, hash, className,
                                                     NULL, NULL,
                                                     JEMCC_ClassNameHashFn, 
                                                     JEMCC_ClassNameEqualsFn);
                    }
                    JEM_DestroyClassInstance(env, classInst);
                }

                /* Release lock and return */
                JEM_UnlockClassLoader(env, loader);
                return JNI_ENOMEM;
            }
            /* This should not happen according to existence check */
            abort();
        }
    }

    /* All done, release and return */
    JEM_UnlockClassLoader(env, loader);
    return JNI_OK;
}

/**
 * Collection method used to destroy class instances and attachments.  Used
 * for cleanup operations following errors during class creation and for
 * final destruction of a class instance from the garbage collector.  NOTE:
 * this method does not verify if a class is in use or not - it is intended
 * for internal use by the VM management components only.
 *
 * This method will release both the main attachment data as well as the
 * JEMCC_Object instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class instance to be destroyed
 */
void JEM_DestroyClassInstance(JNIEnv *env, JEMCC_Class *classInst) {
    JEM_ClassData *classData = classInst->classData;
    JEM_ClassMethodData *methodPtr;
    JEM_ClassFieldData *fieldPtr;
    juint accessFlags = classData->accessFlags;
    int i, isRawThrowable = JNI_FALSE;

    /* Clear the primary class information */
    classData->accessFlags = 0;
    classData->classLoader = NULL;
    if (((accessFlags & ACC_JEMCC) == 0) || ((accessFlags & ACC_ARRAY) != 0)) {
        /* Note that (non-primitive) array names are dynamically constructed */
        if ((accessFlags & ACC_PRIMITIVE) == 0) {
            JEMCC_Free(classData->className);
        }
    } else if (classData->className != NULL) {
        if (JEMCC_ClassNameEqualsFn(env, classData->className,
                                    "java.lang.Throwable") == JNI_TRUE) {
            isRawThrowable = JNI_TRUE;
        }
    }
    classData->className = NULL;
    classData->resolveInitState = 0;
    classData->resolveInitThread = 0;
    if (classData->parseData != NULL) {
        JEM_DestroyParsedClassData(classData->parseData);
    }

    /* Handle arrays specially, as they are all copies of the primitive */
    if (((accessFlags & ACC_ARRAY) != 0) && 
                      ((accessFlags & ACC_PRIMITIVE) == 0)) {
        /* Note that name was released above */
        JEMCC_Free(classData);
        JEMCC_Free(classInst);
        return;
    }

    /* Same idea for standard throwable instances */
    if (((accessFlags & ACC_STD_THROW) != 0) && (isRawThrowable != JNI_TRUE)) { 
        JEMCC_Free(classData->assignList);
        JEMCC_Free(classData->methodLinkTables);
        JEMCC_Free(classData);
        JEMCC_Free(classInst);
        return;
    }

    /* Delete the reference tables - note that synthetic entries are managed */
    JEMCC_Free(classData->assignList);
    if (classData->methodLinkTables != NULL) {
        for (i = classData->classMethodCount - classData->syntheticMethodCount;
                        i < classData->classMethodCount; i++) {
            methodPtr = classData->methodLinkTables[0][i];
            if ((methodPtr != NULL) &&
                ((methodPtr->accessFlags & ACC_SYNTHETIC) != 0) &&
                (methodPtr->parentClass == classInst)) {
                JEMCC_Free(methodPtr);
            }
        }
        JEMCC_Free(classData->methodLinkTables[0]);
        for (i = 0; i < classData->interfaceCount; i++) {
            JEMCC_Free(classData->methodLinkTables[i + 1]);
        }
        JEMCC_Free(classData->methodLinkTables);
    }

    /* Release static class data if applicable */
    JEMCC_Free(classInst->staticData);

    /* Delete local method and field information */
    methodPtr = classData->localMethods;
    for (i = 0; i < classData->localMethodCount; i++) {
        if ((accessFlags & ACC_JEMCC) == 0) {
            JEMCC_Free(methodPtr->name);
            JEMCC_Free(methodPtr->descriptorStr);
        }
        JEM_DestroyDescriptor(methodPtr->descriptor, JNI_TRUE);
        if (((methodPtr->accessFlags & ACC_JEMCC) == 0) &&
            (methodPtr->method.bcMethod != NULL)) {
            JEM_DestroyMethodCode(methodPtr->method.bcMethod);
        }
        methodPtr++;
    }
    JEMCC_Free(classData->localMethods);
    fieldPtr = classData->localFields;
    for (i = 0; i < classData->localFieldCount; i++) {
        if ((accessFlags & ACC_JEMCC) == 0) {
            JEMCC_Free(fieldPtr->name);
            JEMCC_Free(fieldPtr->descriptorStr);
        }
        JEM_DestroyDescriptor(fieldPtr->descriptor, JNI_TRUE);
        fieldPtr++;
    }
    JEMCC_Free(classData->localFields);

    /* External references and constants are simple arrays */
    JEMCC_Free(classData->localConstants);
    JEMCC_Free(classData->classRefs);
    JEMCC_Free(classData->classMethodRefs);
    JEMCC_Free(classData->classFieldRefs);

    /* Miscellaneous class data */
    JEMCC_Free(classData->sourceFile);

    /* Delete the class information and the object instance */
    JEMCC_Free(classData);
    JEMCC_Free(classInst);
}

/**
 * Convenience method to obtain the "internal" name of the class.  This
 * is the raw character className, may be UTF encoded.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     class - the class instance to retrieve the name for
 *
 * Returns:
 *     The raw character representation of the fully qualified name of
 *     the specified class.  May be UTF encoded.
 */
char *JEMCC_GetClassName(JNIEnv *env, JEMCC_Class *class) {
    return class->classData->className;
}

/**
 * Convenience method for dumping internal class information (for debugging
 * purposes).  All output goes to the stderr channel.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     dbgClass - the class instance to be debugged
 */
void JEM_DumpDebugClass(JNIEnv *env, JEMCC_Class *dbgClass) {
    JEM_ClassData *classData = dbgClass->classData;
    JEMCC_Class **cptr;
    JEM_BCMethod *methodPtr;
    int i, j, k, ctr;
    char *str, *ptr;

    /* Flag details and class name */
    if ((classData->accessFlags & ACC_PUBLIC) != 0) {
        (void) fprintf(stderr, "public ");
    } else if ((classData->accessFlags & ACC_PRIVATE) != 0) {
        (void) fprintf(stderr, "private ");
    } else if ((classData->accessFlags & ACC_PROTECTED) != 0) {
        (void) fprintf(stderr, "protected ");
    }
    if ((classData->accessFlags & ACC_FINAL) != 0) {
        (void) fprintf(stderr, "final ");
    }
    if ((classData->accessFlags & ACC_ABSTRACT) != 0) {
        (void) fprintf(stderr, "abstract ");
    }
    if ((classData->accessFlags & ACC_INTERFACE) != 0) {
        (void) fprintf(stderr, "interface ");
    } else {
        (void) fprintf(stderr, "class ");
    }
    (void) fprintf(stderr, "%s (From: %s)\n", classData->className,
                   ((classData->sourceFile == NULL) ? "Unknown source" :
                                                      classData->sourceFile));

    /* Assignment details */
    (void) fprintf(stderr, "  Assignments/Methods:\n");
    cptr = classData->assignList;
    i = 0;
    while (*cptr != NULL) {
        if (*cptr != NULL) {
            str = (*cptr)->classData->className;
        } else {
            str = "NULL";
        }
        if (i == 0) {
            (void) fprintf(stderr, "    [%i] %s (superclass)\n", i, str);
        } else if (i <= classData->interfaceCount) {
            (void) fprintf(stderr, "    [%i] %s (interface)\n", i, str);
        } else {
            (void) fprintf(stderr, "    [%i] %s (superassign)\n", i, str);
        }

        if (i == 0) {
            for (j = 0; j < classData->classMethodCount; j++) {
                str = "bytecode";
                if ((classData->methodLinkTables[0][j]->accessFlags &
                                                            ACC_NATIVE) != 0) {
                    str = "native";
                }
                if ((classData->methodLinkTables[0][j]->accessFlags &
                                                            ACC_JEMCC) != 0) {
                    str = "jemcc";
                }
                ptr = "";
                for (k = classData->localMethodCount - 1; k >= 0; k--) {
                    if (classData->methodLinkTables[0][j] ==
                                      &(classData->localMethods[k])) break;
                }
                if ((classData->methodLinkTables[0][j]->accessFlags &
                                                         ACC_SYNTHETIC) != 0) {
                    if (classData->methodLinkTables[0][j]->parentClass ==
                                                                dbgClass) k = 0;
                    ptr = "synthetic ";
                } else if ((classData->methodLinkTables[0][j]->accessFlags &
                                                          ACC_ABSTRACT) != 0) {
                    ptr = "abstract ";
                }
                (void) fprintf(stderr, "      c-%i: %s%s [%s, %s%s]\n", j,
                               classData->methodLinkTables[0][j]->name,
                               classData->methodLinkTables[0][j]->descriptorStr,
                               ((k < 0) ? "inherited" : "local"), ptr, str);
                if (strcmp(str, "bytecode") == 0) {
                    methodPtr =
                        classData->methodLinkTables[0][j]->method.bcMethod;
                    if (methodPtr != NULL) {
                        (void) fprintf(stderr,
                                   "%s[Stack: %i Locals: %i Code: %i]\n",
                                   "            ", methodPtr->maxStack,
                                   methodPtr->maxLocals, methodPtr->codeLength);
                    } else {
                        (void) fprintf(stderr,
                                   "            [No code defined]\n");
                    }
                }
            }
        } else {
            ctr = (*cptr)->classData->classMethodCount;
            for (j = 0; j < ctr; j++) {
                ptr = (*cptr)->classData->methodLinkTables[0][j]->name;
                if (classData->methodLinkTables[i][j] == NULL) {
                    (void) fprintf(stderr, "      m-%i[%s]=>Missing\n", j, ptr);
                } else {
                    (void) fprintf(stderr, "      m-%i[%s]=>%i\n", j, ptr,
                               classData->methodLinkTables[i][j]->methodIndex);
                }
            }
        }

        cptr++;
        i++;
    }
    if (*(classData->assignList) == NULL) {
        for (j = 0; j < classData->classMethodCount; j++) {
            (void) fprintf(stderr, "      %c-%i: %s%s\n", 'c', j,
                           classData->methodLinkTables[0][j]->name,
                           classData->methodLinkTables[0][j]->descriptorStr);
        }
    }

    /* Field/method information */
    (void) fprintf(stderr, "    Local Methods:\n");
    for (i = 0; i < classData->localMethodCount; i++) {
        (void) fprintf(stderr, "      %i: %s%s [Index %i]\n", i,
                               classData->localMethods[i].name,
                               classData->localMethods[i].descriptorStr,
                               classData->localMethods[i].methodIndex);
    }
    (void) fprintf(stderr, "    Local Fields:\n");
    for (i = 0; i < classData->localFieldCount; i++) {
        (void) fprintf(stderr, "      %i: %s [Offset %i]\n", i,
                       classData->localFields[i].name,
                       classData->localFields[i].fieldOffset);
    }

    /* All done, just make it pretty now */
    (void) fprintf(stderr, "\n");
}
