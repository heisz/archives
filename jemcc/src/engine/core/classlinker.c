/**
 * JEMCC methods to support Java bytecode class linkage and definition.
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

/* Quick and dirty macro to get at the class constant information */
#define CL_CONSTANT(x) pData->constantPool[x - 1]

/**
 * Convenience method to lookup a class instance based on a reference
 * in the constant pool information.  Immediately retrieves the associated
 * class if the lookup has already been performed.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the loader from which the referenced class is to be
 *              retrieved (NULL indicates the VM bootstrap loader)
 *     pData - the parsed bytecode class information
 *     classIndex - the constant pool index for the reference class (already
 *                  realigned to zero-based)
 *     classInst - a class instance reference through which the class is
 *                 returned (if located/referenced successfully).  NULL if
 *                 the reference is invalid
 *     captureError - if JNI_TRUE, any exceptions are "captured" by this
 *                    method and retained for future use on reference.
 *                    Otherwise, the exception will be properly thrown and
 *                    the class linkage should immediately fail
 *
 * Returns:
 *     JNI_OK - the reference was loaded successfully or the root exception
 *              was captured for later use
 *     JNI_ERR - the captureError value was false and an exception occurred
 *               (which has been thrown in the current environment)
 *     JNI_ENOMEM - a memory allocation failure occurred
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *
 *     All other exceptions are captured within this method and retained
 *     until the class reference is required (late binding).
 */
static jint JEM_ResolveClassInstance(JNIEnv *env, JEMCC_Object *loader,
                                     JEM_ParsedClassData *pData,
                                     int classIndex, JEMCC_Class **classInst,
                                     jboolean captureError) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint origFrameOpFlags = jenv->topFrame->opFlags;
    JEM_ConstantPoolData *cpe = &(pData->constantPool[classIndex]);
    jint rc, nameIndex;

    if (cpe->generic.tag == CONSTANT_ResolvedClass) {
        *classInst = (JEMCC_Class *) cpe->res_class_info.extClassRef;
        return JNI_OK;
    } else if (cpe->generic.tag == CONSTANT_ResolvedClassErr) {
        /* Note that this should not happen for required class */
        if (captureError == JNI_FALSE) abort();
        *classInst = NULL;
        return JNI_OK;
    } else {
        /* Capture class resolution errors in current frame */
        if (captureError == JNI_TRUE) {
            jenv->topFrame->opFlags |= FRAME_THROWABLE_CAPTURE;
        }

        /* Attempt to load/resolve the associated class instance */
        nameIndex = cpe->class_info.nameIndex;
        rc = JEMCC_LocateClass(env, loader,
                               (char *) CL_CONSTANT(nameIndex).utf8_info.bytes,
                               JNI_TRUE, classInst);
        if (rc == JNI_ENOMEM) {
            /* Rethrow the caught exception (if required) */
            if (captureError == JNI_TRUE) {
                jenv->topFrame->opFlags = origFrameOpFlags;
                JEMCC_ProcessThrowable(env, NULL);
            }
            return rc;
        }
        if (rc == JNI_OK) {
            cpe->res_class_info.tag = CONSTANT_ResolvedClass;
            cpe->res_class_info.refTblIndex = -1;
            cpe->res_class_info.extClassRef = (JEMCC_Object *) *classInst;
        } else {
            if (captureError == JNI_FALSE) return JNI_ERR;
            cpe->res_class_info.tag = CONSTANT_ResolvedClassErr;
            cpe->res_class_info.refTblIndex = -1;
            cpe->res_class_info.extClassRef = 
                                  JEM_ExtractSkeletonThrowable(env);
        }

        /* Release the exception capture mode on the current frame */
        if (captureError == JNI_TRUE) {
            jenv->topFrame->opFlags = origFrameOpFlags;
        }
    }

    return JNI_OK;
}

/**
 * Parse and validate the field attribute information for the bytecode class.
 * Initializes static field information as well are marking synthetic field
 * instances.  Moved into a separate method to simplify multiple exit (error)
 * points and associated class cleanup.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     pData - the parsed bytecode class information
 *     classInst - the class instance for which static field information is
 *                 obtained/initialized
 *
 * Returns:
 *     JNI_OK - the static bytecode fields were successfully initialized
 *     JNI_ERR - an initialization error occurred (an exception has been
 *               thrown in the current environment)
 *
 * Exceptions:
 *     ClassFormatException - an invalid initialization specification was
 *                            encountered
 */
static jint JEM_ProcessFieldAttributes(JNIEnv *env, JEM_ParsedClassData *pData, 
                                       JEMCC_Class *classInst) {
    JEM_DescriptorData *pDesc;
    JEM_ConstantPoolData *cpe;
    int i, j, foundConstant;
    const jbyte *bptr;
    jubyte *dataptr;
    jint index;
    char *ptr;

    /* Walk the class fields, handling ConstantValue and Synthetic attrs */
    for (i = 0; i < classInst->classData->localFieldCount; i++) {
        foundConstant = 0;
        for (j = 0; j < pData->fields[i].attributesCount; j++) {
            index = pData->fields[i].attributes[j].attributeNameIndex;
            ptr = CL_CONSTANT(index).utf8_info.bytes;
            if (strcmp(ptr, "ConstantValue") == 0) {
                if (foundConstant != 0) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                    JEMCC_Class_ClassFormatError, NULL,
                                    "Multiple constant values for same field");
                    return JNI_ERR;
                }
                if (pData->fields[i].attributes[j].attributeLength != 2) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                      JEMCC_Class_ClassFormatError, NULL,
                                      "Invalid constant value attribute data");
                    return JNI_ERR;
                }
                if ((pData->fields[i].accessFlags & ACC_STATIC) == 0) {
                    /* Constant value is only for static fields, ignore */
                    continue;
                }
                bptr = (jubyte *) pData->fields[i].attributes[j].info;
                /* Note: no type check here, done in following switch */
                index = JEM_ReadConstantPoolIndex(env, &bptr, pData, 0, NULL);
                if (index < 0) return JNI_ERR;

                pDesc = classInst->classData->localFields[i].descriptor;
                dataptr = ((jbyte *) classInst->staticData) +
                              classInst->classData->localFields[i].fieldOffset;
                cpe = &(pData->constantPool[index - 1]);
                switch (cpe->generic.tag) {
                    case CONSTANT_Integer:
                        switch (pDesc->generic.tag) {
                            case BASETYPE_Boolean:
                                *((jboolean *) dataptr) = 
                                    (cpe->integer_info.value != 0) ? 
                                                          JNI_TRUE : JNI_FALSE;
                                break;
                            case BASETYPE_Byte:
                                *((jbyte *) dataptr) = (jbyte)
                                            (cpe->integer_info.value & 0xFF);
                                break;
                            case BASETYPE_Short:
                                *((jshort *) dataptr) = (jshort)
                                            (cpe->integer_info.value & 0xFFFF);
                                break;
                            case BASETYPE_Char:
                                *((jchar *) dataptr) = (jchar)
                                            (cpe->integer_info.value & 0xFFFF);
                                break;
                            case BASETYPE_Int:
                                *((jint *) dataptr) = cpe->integer_info.value;
                                break;
                            default:
                                JEMCC_ThrowStdThrowableIdx(env, 
                                        JEMCC_Class_ClassFormatError, NULL,
                                        "Invalid field type for int constant");
                                return JNI_ERR;
                        }
                        break;
                    case CONSTANT_Float:
                        if (pDesc->generic.tag != BASETYPE_Float) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                      JEMCC_Class_ClassFormatError, NULL,
                                      "Invalid field type for float constant");
                            return JNI_ERR;
                        }
                        *((jfloat *) dataptr) = cpe->float_info.value;
                        break;
                    case CONSTANT_Long:
                        if (pDesc->generic.tag != BASETYPE_Long) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                       JEMCC_Class_ClassFormatError, NULL,
                                       "Invalid field type for long constant");
                            return JNI_ERR;
                        }
                        *((jlong *) dataptr) = cpe->long_info.value;
                        break;
                    case CONSTANT_Double:
                        if (pDesc->generic.tag != BASETYPE_Double) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                     JEMCC_Class_ClassFormatError, NULL,
                                     "Invalid field type for double constant");
                            return JNI_ERR;
                        }
                        *((jdouble *) dataptr) = cpe->double_info.value;
                        break;
                    case CONSTANT_ResolvedString:
                        if ((pDesc->generic.tag != DESCRIPTOR_ObjectType) ||
                            (JEMCC_ClassNameEqualsFn(env,
                                               pDesc->object_info.className,
                                               "java.lang.String") == 0)) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                     JEMCC_Class_ClassFormatError, NULL,
                                     "Invalid field type for string constant");
                            return JNI_ERR;
                        }
                        *((JEMCC_Object **) dataptr) = 
                               (JEMCC_Object *) cpe->res_string_info.stringRef;
                        break;
                    default:
                        JEMCC_ThrowStdThrowableIdx(env, 
                                            JEMCC_Class_ClassFormatError, NULL,
                                            "Invalid constant value type");
                        return JNI_ERR;
                }
                foundConstant = 1;
            } else if (strcmp(ptr, "Synthetic") == 0) {
                classInst->classData->localFields[i].accessFlags |= 
                                                             ACC_SYNTHETIC;
            }
            /* Ignore all else - so far, only 'Deprecated' is possible */
        }
    }

    return JNI_OK;
}

/**
 * Construct the external class, method and field linkages as required by
 * the bytecode class.  Moved into a separate method to simplify multiple 
 * exit (error) points and associated class cleanup.
 *
 * Note: some error conditions result in immediate failure of this method
 *       (and hence the class instance).  Other missing class/method/field
 *       exceptions are held until the reference is used (late binding).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the compiled bytecode class information, for which the
 *                 linkage information is obtained
 *
 * Returns:
 *     JNI_OK - the linkage information was successfully constructed
 *     JNI_ENOMEM - a memory allocation error occurred
 *     JNI_ERR - a memory or immediately fatal error occurred (an exception 
 *               has been thrown in the current environment)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatException - an invalid reference descriptor was found
 *     <OtherClassErrors> - an immediately fatal condition occurred in the
 *                          loading of an exception class
 */
jint JEM_LinkClassReferences(JNIEnv *env, JEM_ClassData *classData) {
    int i, j, tag, count, classIdx, nameIdx, fieldIdx, methodIdx;
    JEM_ParsedClassData *pData = classData->parseData;
    JEMCC_Object *loader = classData->classLoader;
    JEM_ClassMethodData *methodPtr;
    JEM_ClassData *classRefData;
    JEM_ClassFieldData *fieldPtr;
    JEM_BCMethod *bcMethodPtr;
    JEM_DescriptorData *errDescriptor;
    JEMCC_Class *classRef;
    char *namePtr, *descPtr;
    jint rc;

    /* Convert the class references in the exception handling tables */
    /* Note that the class is invalid if these are not found */
    for (methodIdx = 0; methodIdx < classData->localMethodCount; methodIdx++) {
        bcMethodPtr = classData->localMethods[methodIdx].method.bcMethod;
        if (bcMethodPtr != NULL) {
            for (i = 0; i < bcMethodPtr->exceptionTableLength; i++) {
                classIdx = bcMethodPtr->exceptionTable[i].exceptionClass.index;
                if (classIdx != 0) {
                    rc = JEM_ResolveClassInstance(env, loader, pData,
                                                  classIdx - 1, &classRef,
                                                  JNI_FALSE);
                    if (rc != JNI_OK) return rc;
                    bcMethodPtr->exceptionTable[i].exceptionClass.instance =
                                                                      classRef;
                } else {
                    /* Finally clause has no exception */
                    bcMethodPtr->exceptionTable[i].exceptionClass.instance =
                                                                          NULL;
                }
            }
        }
    }

    /* Construct the external field references */
    count = 0;
    for (i = 0; i < pData->constantPoolCount - 1; i++) {
        if (pData->constantPool[i].generic.tag == CONSTANT_Fieldref) count++;
    }
    if (count != 0) {
        classData->classFieldRefs = (JEM_ClassFieldData **) JEMCC_Malloc(env,
                                         count * sizeof(JEM_ClassFieldData *));
        if (classData->classFieldRefs == NULL) return JNI_ENOMEM;
    } else {
        classData->classFieldRefs = NULL;
    }
    fieldIdx = 0;
    for (i = 0; i < pData->constantPoolCount - 1; i++) {
        if (pData->constantPool[i].generic.tag != CONSTANT_Fieldref) continue;
        rc = JEM_ResolveClassInstance(env, loader, pData,
                                pData->constantPool[i].ref_info.classIndex - 1,
                                &classRef, JNI_TRUE);
        if (rc == JNI_ENOMEM) return rc;
        if (classRef == NULL) {
            /* TODO - build the exception record */
fprintf(stderr, "****** FIELD REFERENCE CLASS NOT FOUND *******\n");
            fieldPtr = NULL;
        } else {
            classRefData = classRef->classData;
            nameIdx = pData->constantPool[i].ref_info.nameAndTypeIndex - 1;
            namePtr = CL_CONSTANT(pData->constantPool[nameIdx].
                                  nameandtype_info.nameIndex).utf8_info.bytes;
            descPtr = CL_CONSTANT(pData->constantPool[nameIdx].
                                  nameandtype_info.descIndex).utf8_info.bytes;
            fieldPtr = JEM_LocateClassField(classRefData, namePtr, descPtr);
            if (fieldPtr == NULL) {
                errDescriptor = JEM_ParseDescriptor(env, descPtr, NULL, NULL,
                                                    JNI_FALSE);
                if (errDescriptor == NULL) return JNI_ERR;
                if (errDescriptor->generic.tag == DESCRIPTOR_MethodType) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                     JEMCC_Class_ClassFormatError, NULL,
                                     "Invalid descriptor for field reference");
                    JEM_DestroyDescriptor(errDescriptor, JNI_TRUE);
                    return JNI_ERR;
                }

                JEM_DestroyDescriptor(errDescriptor, JNI_TRUE); /* TODO - TMP */
fprintf(stderr, "****** FIELD REFERENCE NOT FOUND *******\n");
                /* TODO - build the exception record */
                fieldPtr = NULL;
            } else {
                /* TODO - check reference permission information! */
            }
        }
        classData->classFieldRefs[fieldIdx] = fieldPtr;
        pData->constantPool[i].res_ref_info.tag = CONSTANT_ResolvedFieldref;
        /* TODO - handle the field ref error */
        pData->constantPool[i].res_ref_info.tableIndex = fieldIdx;
        fieldIdx++;
    }

    /* Construct the method references */
    count = 0;
    for (i = 0; i < pData->constantPoolCount - 1; i++) {
        if ((pData->constantPool[i].generic.tag == CONSTANT_Methodref) ||
            (pData->constantPool[i].generic.tag == CONSTANT_InterfaceMethodref))
                count++;
    }
    if (count != 0) {
        classData->classMethodRefs = (JEM_ClassMethodData **) JEMCC_Malloc(env,
                                         count * sizeof(JEM_ClassMethodData *));
        if (classData->classMethodRefs == NULL) return JNI_ENOMEM;
    } else {
        classData->classMethodRefs = NULL;
    }
    methodIdx = 0;
    for (i = 0; i < pData->constantPoolCount - 1; i++) {
        tag = pData->constantPool[i].generic.tag;
        if ((tag != CONSTANT_Methodref) && 
            (tag != CONSTANT_InterfaceMethodref)) {
                continue;
        }
        methodPtr = NULL;
        classRefData = NULL;
        rc = JEM_ResolveClassInstance(env, loader, pData,
                                pData->constantPool[i].ref_info.classIndex - 1,
                                &classRef, JNI_TRUE);
        if (rc == JNI_ENOMEM) return rc;
        if (classRef != NULL) classRefData = classRef->classData;

        if (classRef == NULL) {
            /* TODO - build the exception record */
fprintf(stderr, "****** METHOD REFERENCE CLASS NOT FOUND *******\n");
        } else if ((tag == CONSTANT_Methodref) && 
                   ((classRefData->accessFlags & ACC_INTERFACE) != 0)) {
            /* TODO - build the exception record */
fprintf(stderr, "****** METHOD REFERENCE TO AN INTERFACE? *******\n");
        } else if ((tag == CONSTANT_InterfaceMethodref) && 
                   ((classRefData->accessFlags & ACC_INTERFACE) == 0)) {
            /* TODO - build the exception record */
fprintf(stderr, "****** INTERFACEMETHOD REFERENCE TO AN METHOD? *******\n");
        } else {
            nameIdx = pData->constantPool[i].ref_info.nameAndTypeIndex - 1;
            namePtr = CL_CONSTANT(pData->constantPool[nameIdx].
                                  nameandtype_info.nameIndex).utf8_info.bytes;
            descPtr = CL_CONSTANT(pData->constantPool[nameIdx].
                                  nameandtype_info.descIndex).utf8_info.bytes;
            /* <init> method must be manual found in local table */
            if (strcmp(namePtr, "<init>") == 0) {
                for (j = 0; j < classRefData->localMethodCount; j++) {
                    if ((strcmp(classRefData->localMethods[j].name, 
                                namePtr) == 0) &&
                        (strcmp(classRefData->localMethods[j].descriptorStr, 
                                descPtr) == 0)) {
                        methodPtr = &(classRefData->localMethods[j]);
                        break;
                    }
                }
            } else {
                methodPtr = JEM_LocateClassMethod(classRefData, 
                                                  namePtr, descPtr);
            }
            if (methodPtr == NULL) {
                errDescriptor = JEM_ParseDescriptor(env, descPtr, NULL, NULL,
                                                    JNI_TRUE);
                if (errDescriptor == NULL) return JNI_ERR;
                if (errDescriptor->generic.tag != DESCRIPTOR_MethodType) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                    JEMCC_Class_ClassFormatError, NULL,
                                    "Invalid descriptor for method reference");
                    JEM_DestroyDescriptor(errDescriptor, JNI_TRUE);
                    return JNI_ERR;
                }

                JEM_DestroyDescriptor(errDescriptor, JNI_TRUE); /* TODO - TMP */
                /* TODO - build the exception record */
fprintf(stderr, "****** METHOD DESCRIPTOR [%s %s] NOT FOUND *******\n",
                namePtr, descPtr);
                methodPtr = NULL;
            } else {
                /* TODO - check reference permission information! */
            }
        }
        classData->classMethodRefs[methodIdx] = methodPtr;
        if (tag == CONSTANT_Methodref) {
            pData->constantPool[i].res_ref_info.tag = 
                                    CONSTANT_ResolvedMethodref;
        } else {
            pData->constantPool[i].res_ref_info.tag = 
                                    CONSTANT_ResolvedInterfaceMethodref;
        }
        /* TODO - handle the method ref error */
        pData->constantPool[i].res_ref_info.tableIndex = methodIdx;
        methodIdx++;
    }

    /* NOTE: class reference linking occurs in the validation phase */
    /* But complete all of the resolution actions now under throw protection */
    for (i = 0; i < pData->constantPoolCount - 1; i++) {
        tag = pData->constantPool[i].generic.tag;
        if (tag != CONSTANT_Class) continue;
        (void) JEM_ResolveClassInstance(env, loader, pData, i, &classRef, 
                                        JNI_TRUE);
    }

    return JNI_OK;
}

/* Wrapper methods to simplify cleanup of a class linkage error */
static void JEM_DestroyInProgressClass(JNIEnv *env, 
                                       JEMCC_Class *classInstance) {
    JEMCC_Object *loader = classInstance->classData->classLoader;

    /* First, remove it from the namespace to prevent collisions */
    JEM_ClassNameSpaceRemove(env, classInstance);

    /* Notify anyone who is waiting that the class is in transition */
    /* Note that waits always include timeout for memfail safety */
    if (loader != NULL) {
        if (JEMCC_EnterObjMonitor(env, (jobject) loader) == JNI_OK) {
            (void) JEMCC_ObjMonitorNotifyAll(env, (jobject) loader);
            (void) JEMCC_ExitObjMonitor(env, (jobject) loader);
        }
    }

    /* Now it is safe to destroy all of the intermediate data */
    JEM_DestroyClassInstance(env, classInstance);
}

/**
 * The following two methods are used to destroy the in-progress allocations
 * of the class method and field definition tables for bytecode classes.
    classData->parseData = pData;
 * They are similar but not identical to those defined in jemcc.c
 */
static void JEM_DestroyWorkingMethodData(JEM_ClassMethodData *data,
                                         jint methodCount) {
    JEM_ClassMethodData *methodPtr = data;
    int i;

    for (i = 0; i < methodCount; i++) {
        JEMCC_Free(methodPtr->name);
        if (methodPtr->descriptor != NULL) {
            JEM_DestroyDescriptor(methodPtr->descriptor, JNI_TRUE);
        }
        JEMCC_Free(methodPtr->descriptorStr);
        if (methodPtr->method.bcMethod != NULL) {
            JEM_DestroyMethodCode(methodPtr->method.bcMethod);
        }
        methodPtr++;
    }
    JEMCC_Free(data);
}

static void JEM_DestroyWorkingFieldData(JEM_ClassFieldData *data,
                                        jint fieldCount) {
    JEM_ClassFieldData *fieldPtr = data;
    int i;

    for (i = 0; i < fieldCount; i++) {
        JEMCC_Free(fieldPtr->name);
        if (fieldPtr->descriptor != NULL) {
            JEM_DestroyDescriptor(fieldPtr->descriptor, JNI_TRUE);
        }
        JEMCC_Free(fieldPtr->descriptorStr);
        fieldPtr++;
    }
    JEMCC_Free(data);
}

/**
 * Define and resolve a bytecode class instance based on the provided
 * parse information.  This will create and store the class instance in the
 * provided classloader, resolving all classes required to construct the
 * hierarchy tables.   While this method does validate the linkage information
 * (for format, etc.), it does not perform the actual external linking and
 * bytecode verification - this occurs during initialization.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to receive the class instance.  If
 *              NULL, the VM bootstrap classloader is used
 *     pData - the parsed bytecode class information.  Once passed to this
 *             method, this parse information is "owned" by the class (caller
 *             must not free/destroy it)
 *
 * Returns:
 *     The fully defined and resolved bytecode class or NULL if a fatal
 *     processing error occurred.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - a data error occurred while parsing the method
 *                        code information
 *     NoClassDefFoundError - a class required for resolving was not found or
 *                            was not loadable by the VM
 */
JEMCC_Class *JEM_DefineAndResolveClass(JNIEnv *env, JEMCC_Object *loader,
                                       JEM_ParsedClassData *pData) {
    JEMCC_Class *classInstance, *superClass, **interfaces;
    JEM_ClassData *classData;
    JEM_ClassMethodData *classMethods = NULL;
    JEM_ClassFieldData *classFields = NULL;
    int i, j, count, foundCode;
    char *className, *ptr;
    const jbyte *bptr;
    jint rc, index, strIndex;

    /* Make the primary class record */
    classInstance = (JEMCC_Class *) JEM_AllocateClass(env);
    if (classInstance == NULL) {
        JEM_DestroyParsedClassData(pData);
        return NULL;
    }
    classData = classInstance->classData;
    classData->accessFlags = pData->classAccessFlags;
    classData->parseData = pData;

    /* Determine and standardize the class name */
    index = CL_CONSTANT(pData->classIndex).class_info.nameIndex;
    ptr = CL_CONSTANT(index).utf8_info.bytes;
    if ((className = (char *) JEMCC_StrDupFn(env, ptr)) == NULL) {
        JEM_DestroyParsedClassData(pData);
        JEMCC_Free(classInstance->classData);
        JEMCC_Free(classInstance);
        return NULL;
    }
    JEM_SlashToDot(className);
    classData->className = className;

    /* Obviously, self-reference classes are already resolved */
    CL_CONSTANT(pData->classIndex).res_class_info.tag = CONSTANT_ResolvedClass;
    CL_CONSTANT(pData->classIndex).res_class_info.refTblIndex = -1;
    CL_CONSTANT(pData->classIndex).res_class_info.extClassRef = 
                                                (JEMCC_Object *) classInstance;

    /* Initialize class state to "in-progress" */
    classData->resolveInitState = JEM_CLASS_RESOLVE_IN_PROGRESS;
    classData->resolveInitThread = ((JEM_JNIEnv *) env)->envThread;

    /* Insert working record into classloader namespace table */
    /* Once this is complete, always destroy in-progress (external interact) */
    if (JEM_ClassNameSpaceStore(env, loader,
                                classInstance, NULL, JNI_FALSE) != JNI_OK) {
        JEM_DestroyClassInstance(env, classInstance);
        return NULL;
    }

    /* Grab the superclass to initialize class data */
    index = CL_CONSTANT(pData->superClassIndex).class_info.nameIndex;
    className = CL_CONSTANT(index).utf8_info.bytes;
    if (*className == '[') {
        /* Capture special format case */
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                   "Invalid superclass name (array)");
        JEM_DestroyInProgressClass(env, classInstance);
        return NULL;
    }
    rc = JEMCC_LocateClass(env, loader, className, JNI_TRUE, &superClass);
    if (rc != JNI_OK) {
        JEM_DestroyInProgressClass(env, classInstance);
        return NULL;
    }

    /* Capture the extend failure cases */
    if ((superClass->classData->accessFlags & ACC_INTERFACE) != 0) {
        JEMCC_ThrowStdThrowableIdxV(env, 
                                    JEMCC_Class_IncompatibleClassChangeError,
                                    NULL, classData->className, 
                                    ": Class cannot extend interface", NULL);
        JEM_DestroyInProgressClass(env, classInstance);
        return NULL;
    }
    if ((superClass->classData->accessFlags & ACC_FINAL) != 0) {
        JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_VerifyError, NULL,
                                    classData->className,
                                    ": Class cannot extend a final class", 
                                    NULL);
        JEM_DestroyInProgressClass(env, classInstance);
        return NULL;
    }

    /* Grab all of the direct interface information as well */
    if (pData->interfacesCount > 0) {
        interfaces = (JEMCC_Class **) JEMCC_Malloc(env,
                               pData->interfacesCount * sizeof(JEMCC_Class *));
        if (interfaces == NULL) {
            JEM_DestroyInProgressClass(env, classInstance);
            return NULL;
        }
    } else {
        interfaces = NULL;
    }
    for (i = 0; i < pData->interfacesCount; i++) {
        index = CL_CONSTANT(pData->interfaces[i]).class_info.nameIndex;
        className = CL_CONSTANT(index).utf8_info.bytes;
        if (*className == '[') {
            /* Capture special format case */
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                       "Invalid superinterface name (array)");
            JEM_DestroyInProgressClass(env, classInstance);
            return NULL;
        }
        rc = JEMCC_LocateClass(env, loader, className, 
                               JNI_TRUE, &interfaces[i]);
        if (rc != JNI_OK) {
            JEM_DestroyInProgressClass(env, classInstance);
            JEMCC_Free(interfaces);
            return NULL;
        }

        /* Ensure it is an interface */
        if ((interfaces[i]->classData->accessFlags & ACC_INTERFACE) == 0) {
            JEMCC_ThrowStdThrowableIdxV(env, 
                                       JEMCC_Class_IncompatibleClassChangeError,
                                       NULL, classData->className,
                                       ": Class cannot implement a class",
                                       NULL);
            JEM_DestroyInProgressClass(env, classInstance);
            return NULL;
        }
    }

    /* Build class method record information */
    if (pData->methodsCount != 0) {
        classMethods = (JEM_ClassMethodData *) JEMCC_Malloc(env,
                           pData->methodsCount * sizeof(JEM_ClassMethodData));
        if (classMethods == NULL) {
            JEM_DestroyInProgressClass(env, classInstance);
            JEMCC_Free(interfaces);
            return NULL;
        }
    }
    for (i = 0; i < pData->methodsCount; i++) {
        /* Clone/reformat method record information */
        classMethods[i].accessFlags = pData->methods[i].accessFlags;
        ptr = CL_CONSTANT(pData->methods[i].nameIndex).utf8_info.bytes;
        classMethods[i].name = JEMCC_StrDupFn(env, ptr);
        if (classMethods[i].name == NULL) break;
        ptr = CL_CONSTANT(pData->methods[i].descIndex).utf8_info.bytes;
        classMethods[i].descriptorStr = JEMCC_StrDupFn(env, ptr);
        if (classMethods[i].descriptorStr == NULL) break;
        classMethods[i].descriptor = JEM_ParseDescriptor(env, ptr, NULL, NULL,
                 ((classMethods[i].accessFlags & ACC_STATIC) != 0) ? JNI_TRUE : 
                                                                     JNI_FALSE);
        if (classMethods[i].descriptor == NULL) break;
        if (classMethods[i].descriptor->generic.tag != DESCRIPTOR_MethodType) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                    "Invalid descriptor for method instance");
            break;
        }
        classMethods[i].parentClass = classInstance;

        /* Parse method attribute information */
        foundCode = 0;
        for (j = pData->methods[i].attributesCount - 1; j >= 0; j--) {
            index = pData->methods[i].attributes[j].attributeNameIndex;
            ptr = CL_CONSTANT(index).utf8_info.bytes;
            if (strcmp(ptr, "Code") == 0) {
                if (foundCode != 0) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                       JEMCC_Class_ClassFormatError, NULL,
                                       "Multiple code definitions for method");
                    break;
                }
                classMethods[i].method.bcMethod = 
                    JEM_ParseMethodCode(env,
                               pData->methods[i].attributes[j].info,
                               pData->methods[i].attributes[j].attributeLength,
                               pData);
                if (classMethods[i].method.bcMethod == NULL) break;
                foundCode = 1;
            } else if (strcmp(ptr, "Synthetic") == 0) {
                classMethods[i].accessFlags |= ACC_SYNTHETIC;
            }
            /* Ignore all else - note that 'Deprecated' is irrelevant and */
            /* 'Exceptions' is not validated by the VM. */
        }
        if (j >= 0) break;

        /* Validate the abstract cases */
        if ((classMethods[i].accessFlags & (ACC_ABSTRACT | ACC_NATIVE)) != 0) {
            if (foundCode) {
                JEMCC_ThrowStdThrowableIdx(env, 
                       JEMCC_Class_ClassFormatError, NULL,
                       "Abstract/native methods cannot have code information");
                break;
            }
        } else {
            if (!foundCode) {
                JEMCC_ThrowStdThrowableIdx(env,
                       JEMCC_Class_ClassFormatError, NULL,
                       "Non-abstract/native method missing code information");
                break;
            }
        }
    }
    if (i != pData->methodsCount) {
        JEM_DestroyWorkingMethodData(classMethods, pData->methodsCount);
        JEM_DestroyInProgressClass(env, classInstance);
        JEMCC_Free(interfaces);
        return NULL;
    }

    /* Initialize the hierarchical class information */
    if (JEM_BuildClassHierData(env, classInstance, superClass,
                               interfaces, pData->interfacesCount,
                               classMethods, pData->methodsCount) != JNI_OK) {
        /* Note: if hierarchy creation failed, method data is not yet stored */
        JEM_DestroyWorkingMethodData(classMethods, pData->methodsCount);
        JEM_DestroyInProgressClass(env, classInstance);
        JEMCC_Free(interfaces);
        return NULL;
    }
    JEMCC_Free(interfaces);

    /* Build class field record information */
    if (pData->fieldsCount != 0) {
        classFields = (JEM_ClassFieldData *) JEMCC_Malloc(env,
                           pData->fieldsCount * sizeof(JEM_ClassFieldData));
        if (classFields == NULL) {
            JEM_DestroyInProgressClass(env, classInstance);
            return NULL;
        }
    }
    for (i = 0; i < pData->fieldsCount; i++) {
        classFields[i].accessFlags = pData->fields[i].accessFlags;
        ptr = CL_CONSTANT(pData->fields[i].nameIndex).utf8_info.bytes;
        classFields[i].name = (char *) JEMCC_StrDupFn(env, ptr);
        if (classFields[i].name == NULL) break;
        ptr = CL_CONSTANT(pData->fields[i].descIndex).utf8_info.bytes;
        classFields[i].descriptorStr = JEMCC_StrDupFn(env, ptr);
        if (classFields[i].descriptorStr == NULL) break;
        classFields[i].descriptor = JEM_ParseDescriptor(env, ptr, NULL, NULL,
                                                        JNI_FALSE);
        if (classFields[i].descriptor == NULL) break;
        if (classFields[i].descriptor->generic.tag == DESCRIPTOR_MethodType) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                     "Invalid descriptor for field instance");
            break;
        }
        classFields[i].fieldOffset = -1;
        classFields[i].parentClass = classInstance;
    }
    if (i != pData->fieldsCount) {
        JEM_DestroyWorkingFieldData(classFields, pData->fieldsCount);
        JEM_DestroyInProgressClass(env, classInstance);
        return NULL;
    }

    /* Attach fields to class and pack information */
    if (JEM_PackClassFieldData(env, classInstance,
                               classFields, pData->fieldsCount) != JNI_OK) {
        /* Note: if packing has failed, field data is not yet stored */
        JEM_DestroyWorkingFieldData(classFields, pData->fieldsCount);
        JEM_DestroyInProgressClass(env, classInstance);
        return NULL;
    }

    /* Resolve all of the intern()'d String constants */
    for (i = 0; i < pData->constantPoolCount - 1; i++) {
        if (pData->constantPool[i].generic.tag == CONSTANT_String) {
            strIndex = pData->constantPool[i].string_info.stringIndex;
            ptr = (char *) pData->constantPool[strIndex - 1].utf8_info.bytes;
            pData->constantPool[i].res_string_info.tag = 
                                            CONSTANT_ResolvedString;
            pData->constantPool[i].res_string_info.constTblIndex = -1;
            pData->constantPool[i].res_string_info.stringRef = 
                                            JEMCC_GetInternStringUTF(env, ptr);
            if (pData->constantPool[i].res_string_info.stringRef == NULL) {
                JEM_DestroyInProgressClass(env, classInstance);
                return NULL;
            }
        }
    }

    /* Handle attribute information for field information */
    if (JEM_ProcessFieldAttributes(env, pData, classInstance) != JNI_OK) {
        JEM_DestroyInProgressClass(env, classInstance);
        return NULL;
    }

    /* Miscellaneous attributes and other bits of class information */
    for (i = 0; i < pData->attributesCount; i++) {
        index = pData->attributes[i].attributeNameIndex;
        ptr = CL_CONSTANT(index).utf8_info.bytes;
        if (strcmp(ptr, "SourceFile") == 0) {
            if (pData->attributes[i].attributeLength != 2) {
                JEMCC_ThrowStdThrowableIdx(env, 
                                  JEMCC_Class_ClassFormatError, NULL,
                                  "Incorrect length for SourceFile attribute");
                JEM_DestroyInProgressClass(env, classInstance);
                return NULL;
            }
            if (classData->sourceFile != NULL) {
                JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_ClassFormatError, NULL,
                                   "Multiple SourceFile attribute statements");
                JEM_DestroyInProgressClass(env, classInstance);
                return NULL;
            }
            bptr = pData->attributes[i].info;
            index = JEM_ReadConstantPoolIndex(env, &bptr,
                                              pData, CONSTANT_Utf8,
                                              "Invalid SourceFile data index");
            if (index < 0) {
                JEM_DestroyInProgressClass(env, classInstance);
                return NULL;
            }
            ptr = CL_CONSTANT(index).utf8_info.bytes;
            classData->sourceFile = (char *) JEMCC_StrDupFn(env, ptr);
            if (classData->sourceFile == NULL) {
                JEM_DestroyInProgressClass(env, classInstance);
                return NULL;
            }
        } else if (strcmp(ptr, "InnerClasses") == 0) {
            if (pData->attributes[i].attributeLength < 2) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                               NULL, "Insufficient InnerClass attribute data");
                JEM_DestroyInProgressClass(env, classInstance);
                return NULL;
            }
            bptr = pData->attributes[i].info;
            count = JEM_ReadConstantPoolIndex(env, &bptr, pData, 0, NULL);
            if (pData->attributes[i].attributeLength != 2 + count * 8) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                NULL, "Incorrect InnerClass attribute length");
                JEM_DestroyInProgressClass(env, classInstance);
                return NULL;
            }
            for (j = 0; j < count; j++) {
                index = JEM_ReadConstantPoolIndex(env, &bptr,
                                            pData, -CONSTANT_Class,
                                            "Invalid inner class info index");
                if (index < 0) {
                    JEM_DestroyInProgressClass(env, classInstance);
                    return NULL;
                }
                index = JEM_ReadConstantPoolIndex(env, &bptr,
                                            pData, -CONSTANT_Class,
                                            "Invalid outer class info index");
                if (index < 0) {
                    JEM_DestroyInProgressClass(env, classInstance);
                    return NULL;
                }
                index = JEM_ReadConstantPoolIndex(env, &bptr,
                                            pData, -CONSTANT_Utf8,
                                            "Invalid inner class name index");
                if (index < 0) {
                    JEM_DestroyInProgressClass(env, classInstance);
                    return NULL;
                }
                bptr += 2;
            }
        }
        /* Ignore all else - so far, only 'Deprecated' is possible */
    }

    /* Resolution is now complete */
    classData->resolveInitState = JEM_CLASS_RESOLVE_COMPLETE;

    /* Notify anyone waiting on class completion */
    /* Note that waits always include timeout for memfail safety */
    if (loader != NULL) {
        if (JEMCC_EnterObjMonitor(env, (jobject) loader) == JNI_OK) {
            (void) JEMCC_ObjMonitorNotifyAll(env, (jobject) loader);
            (void) JEMCC_ExitObjMonitor(env, (jobject) loader);
        }
    }

    return classInstance;
}
