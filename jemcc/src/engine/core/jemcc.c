/**
 * Supporting methods for the JEMCC class definitions.
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
 * The following two methods are used to destroy the in-progress allocations
 * of the final class method and field definition tables for JEMCC constructs.
 * They are similar but not identical to those defined in classlinker.c.
 */
static void JEMCC_DestroyWorkingMethodData(JEM_ClassMethodData *data,
                                           jint methodCount) {
    JEM_ClassMethodData *methodPtr = data;
    int i;

    for (i = 0; i < methodCount; i++) {
        if (methodPtr->descriptor != NULL) {
            JEM_DestroyDescriptor(methodPtr->descriptor, JNI_TRUE);
        }
        methodPtr++;
    }
    JEMCC_Free(data);
}
static void JEMCC_DestroyWorkingFieldData(JEM_ClassFieldData *data,
                                          jint fieldCount) {
    JEM_ClassFieldData *fieldPtr = data;
    int i;

    for (i = 0; i < fieldCount; i++) {
        if (fieldPtr->descriptor != NULL) {
            JEM_DestroyDescriptor(fieldPtr->descriptor, JNI_TRUE);
        }
        fieldPtr++;
    }
    JEMCC_Free(data);
}

/**
 * Build a JEM Compiled Class instance.  The creation of a JEM compiled
 * class is done in three stages - the initial construction of the class
 * structure (this method), the linking of the class against class, field
 * and method references (the LinkClass method) and the definition of the
 * class instance in the classloader (the RegisterClass method).  Note that
 * the definition of JEMCC classes must be carefully handled to avoid reference
 * loops, as the external reference linking is performed prior to the actual
 * class definition operation (which may be altered by the definition).  In
 * cases where such loops exist, the RegisterClasses method is used to
 * "simultaneously" define a set of classes with such cross-linkages.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the constructed class
 *              instance.  Note that the class will not be inserted into
 *              the classloader tables at this time, only referenced
 *     accessFlags - the ACC_ flags which define the class permissions and
 *                   types (ACC_JEMCC is automatically set)
 *     className - the fully qualified name of the class being constructed
 *                 (period (.) package separators are used in this case)
 *     superClass - if non-NULL, the superclass of this class.  Note that
 *                  JEMCC classes should ideally have another JEMCC class as
 *                  the superclass and only core Object related classes
 *                  should have a NULL superclass
 *     interfaces - an array reference to a list of direct interfaces
 *                  implemented by this class
 *     interfaceCount - the number of interface instances in the interfaces
 *                      array
 *     methodInfo - an array of JEMCC method definitions for this class
 *     methodCount - the number of method records in the methodInfo array
 *     objMgmtFn - a reference to the object management method, NULL if
 *                 default management operations apply to this class
 *                 (no override function exists)
 *     fieldInfo - an array of JEMCC field definitions for this class
 *     fieldCount - the number of field records in the fieldInfo array
 *     classInst - a class instance reference through which the
 *                 constructed class instance is returned
 *
 * Returns:
 *     JNI_OK - the construction of the JEMCC class was successful, the 
 *              initial class instance is returned through the classInst
 *              reference
 *     JNI_ERR - a definition error occurred during the creation of the class.
 *               A ClassFormatError/AbstractMethodError will have been  
 *               thrown in the current environment and this class should 
 *               be considered permanently invalid
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory 
 *                  error has been thrown in the current environment (but the
 *                  class should be considered "reloadable")
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - provided JEMCC initialization data is invalid
 *     AbstractMethodError - this concrete class is missing a non-abstract 
 *                           method definition
 */
jint JEMCC_BuildClass(JNIEnv *env, JEMCC_Object *loader, juint accessFlags,
                      const char *className, JEMCC_Class *superClass,
                      JEMCC_Class **interfaces, jsize interfaceCount,
                      JEMCC_MethodData *methodInfo, jsize methodCount,
                      JEMCC_ObjMgmtFn *objMgmtFn,
                      JEMCC_FieldData *fieldInfo, jsize fieldCount,
                      JEMCC_Class **classInst) {
    JEMCC_Class *clInst;
    JEM_ClassData *classData;
    JEM_ClassMethodData *methodSet;
    JEM_ClassFieldData *fieldSet;
    int i, rc, memFailFlag;

    /* Pretest conditions */
    if (((accessFlags & ACC_NATIVE_DATA) != 0) && (superClass != NULL) &&
        (superClass->classData->packedFieldSize != 0)) {
        JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_ClassFormatError, NULL,
                                    className, ": JEMCC native data element ",
                                    "must be at root", NULL);
        return JNI_ERR;
    }

    /* Convenience shortcut for class definition */
    if ((accessFlags & ACC_INTERFACE) != 0) accessFlags |= ACC_ABSTRACT;

    /* Construct the Class object instance */
    clInst = (JEMCC_Class *) JEM_AllocateClass(env);
    if (clInst == NULL) return JNI_ENOMEM;
    classData = clInst->classData;
    classData->accessFlags = accessFlags | ACC_JEMCC;
    classData->parseData = NULL;

    /* Assemble the local class method definitions */
    if (methodCount != 0) {
        methodSet = (JEM_ClassMethodData *) JEMCC_Malloc(env,
                                    methodCount * sizeof(JEM_ClassMethodData));
        if (methodSet == NULL) {
            JEM_DestroyClassInstance(env, clInst);
            return JNI_ENOMEM;
        }
    } else {
        methodSet = NULL;
    }
    for (i = 0, memFailFlag = 0; i < methodCount; i++) {
        if (strcmp(methodInfo[i].name, "<clinit>") == 0) {
            methodInfo[i].accessFlags = ACC_PRIVATE | ACC_STATIC;
        }
        methodSet[i].accessFlags = methodInfo[i].accessFlags | ACC_JEMCC;
        if ((accessFlags & ACC_INTERFACE) != 0) {
            methodSet[i].accessFlags = methodSet[i].accessFlags | ACC_ABSTRACT;
        }
        if ((methodSet[i].accessFlags & ACC_ABSTRACT) != 0) {
            if (methodInfo[i].codeBody != NULL) {
                JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_ClassFormatError,
                             NULL, className,
                             ": Abstract methods cannot have code information",
                             NULL);
                break;
            }
        } else {
            if (methodInfo[i].codeBody == NULL) {
                JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_ClassFormatError,
                             NULL, className,
                             ": Non-abstract methods require code information",
                             NULL);
                break;
            }
        }
        methodSet[i].name = methodInfo[i].name;
        methodSet[i].descriptorStr = methodInfo[i].descriptor;
        methodSet[i].descriptor = JEM_ParseDescriptor(env, 
                                                      methodInfo[i].descriptor,
                                                      NULL, NULL, JNI_TRUE);
        if (methodSet[i].descriptor == NULL) {
            /* NOTE: assumption made that provided descriptor was valid */
            memFailFlag = 1;
            break;
        }
        if (methodSet[i].descriptor->generic.tag != DESCRIPTOR_MethodType) {
            JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_ClassFormatError,
                                    NULL, className,
                                    ": Invalid descriptor for method instance",
                                    NULL);
            break;
        }
        methodSet[i].parentClass = clInst;
        methodSet[i].method.ccMethod = methodInfo[i].codeBody;
    }
    if (i != methodCount) {
        JEMCC_DestroyWorkingMethodData(methodSet, methodCount);
        JEM_DestroyClassInstance(env, clInst);
        return ((memFailFlag != 0) ? JNI_ENOMEM : JNI_ERR);
    }

    /* Initialize the the superclass/interface details instance */
    classData->className = (char *) className;
    rc = JEM_BuildClassHierData(env, clInst, superClass,
                                interfaces, interfaceCount,
                                methodSet, methodCount);
    if (rc != JNI_OK) {
        JEMCC_DestroyWorkingMethodData(methodSet, methodCount);
        JEM_DestroyClassInstance(env, clInst);
        return rc;
    }

    /* Build class field record information */
    if (fieldCount != 0) {
        fieldSet = (JEM_ClassFieldData *) JEMCC_Malloc(env,
                                     fieldCount * sizeof(JEM_ClassFieldData));
        if (fieldSet == NULL) {
            JEM_DestroyClassInstance(env, clInst);
            return JNI_ENOMEM;
        }
    } else {
        fieldSet = NULL;
    }
    for (i = 0, memFailFlag = 0; i < fieldCount; i++) {
        fieldSet[i].accessFlags = fieldInfo[i].accessFlags | ACC_JEMCC;
        fieldSet[i].name = fieldInfo[i].name;
        if (fieldSet[i].name != NULL) {
            fieldSet[i].descriptorStr = fieldInfo[i].descriptor;
            fieldSet[i].descriptor = JEM_ParseDescriptor(env, 
                                                      fieldInfo[i].descriptor,
                                                      NULL, NULL, JNI_FALSE);
            if (fieldSet[i].descriptor == NULL) {
                /* NOTE: assumption made that provided descriptor was valid */
                memFailFlag = 1;
                break;
            }
            if (fieldSet[i].descriptor->generic.tag == DESCRIPTOR_MethodType) {
                JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_ClassFormatError,
                                     NULL, className,
                                     ": Invalid descriptor for field instance",
                                     NULL);
                break;
            }
        } else {
            fieldSet[i].descriptorStr = NULL;
            fieldSet[i].descriptor = NULL;
        }
        fieldSet[i].fieldOffset = fieldInfo[i].offset;
        fieldSet[i].parentClass = clInst;
    }
    if (i != fieldCount) {
        JEMCC_DestroyWorkingFieldData(fieldSet, fieldCount);
        JEM_DestroyClassInstance(env, clInst);
        return ((memFailFlag != 0) ? JNI_ENOMEM : JNI_ERR);
    }

    /* Attach fields to class and pack information */
    rc = JEM_PackClassFieldData(env, clInst, fieldSet, fieldCount);
    if (rc != JNI_OK) {
        JEMCC_DestroyWorkingFieldData(fieldSet, fieldCount);
        JEM_DestroyClassInstance(env, clInst);
        return rc;
    }

    /* Setup the class initialization state */
    if ((methodCount != 0) &&
        (strcmp(methodSet[0].name, "<clinit>") == 0)) {
        classData->resolveInitState = JEM_CLASS_RESOLVE_COMPLETE;
    } else {
        classData->resolveInitState = JEM_CLASS_INIT_COMPLETE;
    }

    if (classInst != NULL) *classInst = clInst;
    return JNI_OK;
}

/**
 * Perform the linkage of a JEMCC class instance to external class
 * elements.  This method takes a list of class, field and method
 * string references and builds the class reference tables which can then
 * be retrieved by the JEMCC method body.  As a rule, the compiled
 * class code is not expected to test for missing references - the JEMCC
 * classes are supposed to be at the lowest level where other classes that
 * are depended on are fully available (and exist as defined).  As a result,
 * the linkage of a JEMCC class throws an exception during class definition
 * if a link is missing, rather than upon first use of the reference as a
 * standard bytecode class does.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     linkClass - the class to be linked (receives the final linkage
 *                 tables).  This is also the class used if the extClassRef
 *                 data member in the linkInfo structure is NULL
 *     linkInfo - an array of external LinkData definitions
 *     linkCount - the number of link definitions in the linkInfo array
 *
 * Returns:
 *     JNI_OK - the linkage was completed successfully
 *     JNI_ERR - an error (missing field or method) occurred during the 
 *               linking of the class.  A LinkageError will have been thrown
 *               in the current environment
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory 
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     LinkageError - one of the class linkages failed (reference not found)
 */
jint JEMCC_LinkClass(JNIEnv *env, JEMCC_Class *linkClass,
                     JEMCC_LinkData *linkInfo, jsize linkCount) {
    JEM_ClassData *classData = linkClass->classData;
    JEM_ClassData *classRefData;
    int i, j, classRefCount = 0, methodRefCount = 0, fieldRefCount = 0;
    JEM_ClassMethodData *methodRef;
    JEM_ClassFieldData *fieldRef;
    char *lClassName = linkClass->classData->className;

    /* Determine/create the class definition linkage tables */
    for (i = 0; i < linkCount; i++) {
        if (linkInfo[i].extClassRef == NULL) {
            linkInfo[i].extClassRef = linkClass;
        }
        switch (linkInfo[i].linkType) {
            case JEMCC_LINK_CLASS:
                classRefCount++;
                break;
            case JEMCC_LINK_METHOD:
                methodRefCount++;
                break;
            case JEMCC_LINK_FIELD:
                fieldRefCount++;
                break;
        }
    }
    if (classRefCount != 0) {
        classData->classRefs = (JEMCC_Class **)
                      JEMCC_Malloc(env, classRefCount * sizeof(JEMCC_Class *));
        if (classData->classRefs == NULL) return JNI_ENOMEM;
    }
    if (methodRefCount != 0) {
        classData->classMethodRefs = (JEM_ClassMethodData **)
             JEMCC_Malloc(env, methodRefCount * sizeof(JEM_ClassMethodData *));
        if (classData->classMethodRefs == NULL) return JNI_ENOMEM;
    }
    if (fieldRefCount != 0) {
        classData->classFieldRefs = (JEM_ClassFieldData **)
               JEMCC_Malloc(env, fieldRefCount * sizeof(JEM_ClassFieldData *));
        if (classData->classFieldRefs == NULL) return JNI_ENOMEM;
    }

    /* Rescan the linkage data, filling in the class reference lists */
    classRefCount = methodRefCount = fieldRefCount = 0;
    for (i = 0; i < linkCount; i++) {
        switch (linkInfo[i].linkType) {
            case JEMCC_LINK_CLASS:
                classData->classRefs[classRefCount++] = 
                                              linkInfo[i].extClassRef;
                break;
            case JEMCC_LINK_METHOD:
                classRefData = linkInfo[i].extClassRef->classData;
                methodRef = NULL;
                if (strcmp(linkInfo[i].fieldMethodName, "<init>") == 0) {
                    /* <init> method must be looked up locally */
                    for (j = 0; j < classRefData->localMethodCount; j++) {
                        if ((strcmp(classRefData->localMethods[j].name,
                                    linkInfo[i].fieldMethodName) == 0) &&
                            (strcmp(classRefData->localMethods[j].descriptorStr,
                                    linkInfo[i].fieldMethodDesc) == 0)) {
                            methodRef = &(classRefData->localMethods[j]);
                            break;
                        }
                    }
                } else {
                    methodRef = JEM_LocateClassMethod(classRefData,
                                                  linkInfo[i].fieldMethodName,
                                                  linkInfo[i].fieldMethodDesc);
                }
                if (methodRef == NULL) {
                    JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_LinkageError,
                                                NULL, lClassName,
                                                ": Unable to locate method ",
                                                classRefData->className, "." ,
                                                linkInfo[i].fieldMethodName,
                                                NULL);
                    return JNI_ERR;
                }
                classData->classMethodRefs[methodRefCount++] = methodRef;
                break;
            case JEMCC_LINK_FIELD:
                classRefData = linkInfo[i].extClassRef->classData;
                fieldRef = JEM_LocateClassField(classRefData,
                                                linkInfo[i].fieldMethodName,
                                                linkInfo[i].fieldMethodDesc);
                if (fieldRef == NULL) {
                    JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_LinkageError,
                                                NULL, lClassName,
                                                ": Unable to locate field ",
                                                classRefData->className, "." ,
                                                linkInfo[i].fieldMethodName,
                                                NULL);
                    return JNI_ERR;
                }
                classData->classFieldRefs[fieldRefCount++] = fieldRef;
                break;
        }
    }

    return JNI_OK;
}

/**
 * Method by which static field information may be initialized for
 * a class definition.  The provided value array contains typed initial
 * values for the static members of the class, in the same order as
 * the field definitions in the class construction list (ignoring/skipping
 * over non-static entries).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class containing the static fields to be initialized
 *     initData - an array of initialization values for the class, in the
 *                same order/types as the field declarations.  This array
 *                must have at least as many entries as there are static
 *                field entries in the class
 */
void JEMCC_InitializeStaticClassFields(JNIEnv *env, JEMCC_Class *classInst,
                                       JEMCC_ReturnValue *initData) {
    JEM_ClassData *classData = classInst->classData;
    jubyte *dataptr;
    int i;

    for (i = 0; i < classData->localFieldCount; i++) {
        if ((classData->localFields[i].accessFlags & ACC_STATIC) == 0) continue;
        dataptr = ((jbyte *) classInst->staticData) +
                                     classData->localFields[i].fieldOffset;
        switch (classData->localFields[i].descriptor->generic.tag) {
            case BASETYPE_Boolean:
                *((jboolean *) dataptr) = 
                                (initData->intVal != 0) ? JNI_TRUE : JNI_FALSE;
                break;
            case BASETYPE_Byte:
                *((jbyte *) dataptr) = (jbyte) (initData->intVal & 0xFF);
                break;
            case BASETYPE_Short:
                *((jshort *) dataptr) = (jshort) (initData->intVal & 0xFFFF);
                break;
            case BASETYPE_Char:
                *((jchar *) dataptr) = (jchar) (initData->intVal & 0xFFFF);
                break;
            case BASETYPE_Int:
                *((jint *) dataptr) = initData->intVal;
                break;
            case BASETYPE_Float:
                *((jfloat *) dataptr) = initData->fltVal;
                break;
            case BASETYPE_Long:
                *((jlong *) dataptr) = initData->longVal;
                break;
            case BASETYPE_Double:
                *((jdouble *) dataptr) = initData->dblVal;
                break;
            case DESCRIPTOR_ObjectType:
                *((JEMCC_Object **) dataptr) = initData->objVal;
                break;
        }
        initData++;
    }
}

/**
 * Register an instance of the given JEMCC class into the specified classloader
 * (or the VM bootstrap loader).  Due to the "recoverable" capabilities of the
 * package initialization process, it is not an error to re-register an already
 * registered JEMCC class instance.  If the given class instance already appears
 * in the classloader tables, this method will discard the "new" instance and
 * return the original (already existing) instance, to maintain consistency
 * with potential class instances already created.  Note that this method
 * is only to be used for single classes with no concurrent definition
 * dependencies (as a result of this potential "redefinition").  For situations
 * where multiple classes with cross-dependencies are to be registered at the
 * same time, use the RegisterClasses method below.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the registered class (or
 *              from which the replacement class is to come).  If NULL, the
 *              VM bootstrap classloader is used instead
 *     regClass - a reference to the class instance to be registered.  If this
 *                class already exists in the classloader table, the provided
 *                instance is destroyed and the existing instance is returned
 *                via this reference (for coding simplicity)
 *
 * Returns:
 *     JNI_OK - the definition of the class was successful (or the already
 *              existing instance has been returned successfully).
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory 
 *                  error has been thrown in the current environment (the
 *                  provided class instance has been destroyed)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEMCC_RegisterClass(JNIEnv *env, JEMCC_Object *loader,
                         JEMCC_Class **regClass) {
    JEMCC_Class *existingClass;
    int flags = (*regClass)->classData->accessFlags;
    jint rc;

    /* Only non-primitive classes insert into the class registry (may be VM) */
    if ((flags & ACC_PRIMITIVE) == 0) {
        rc = JEM_ClassNameSpaceStore(env, loader, *regClass,
                                     &existingClass, JNI_TRUE);
        if (rc != JNI_OK) {
            JEM_DestroyClassInstance(env, *regClass);
            if (rc == JNI_ENOMEM) {
                *regClass = NULL;
            } else {
                /* Duplicate is ok, initializer recovery */
                rc = JNI_OK;
                *regClass = existingClass;
            }
        }
        return rc;
    }

    return JNI_OK;
}

/* Note: JEMCC_RegisterClasses moved to class.c (static method accesses) */

/**
 * Convenience method to define a single class instance which has no cross
 * dependencies on other classes being defined at the same time. Essentially a 
 * wrapper around the three steps of building/linking/registering a JEMCC 
 * class instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the constructed class
 *              instance (class will be registered)
 *     accessFlags - the ACC_ flags which define the class permissions and
 *                   types (ACC_JEMCC is automatically set)
 *     className - the fully qualified name of the class being constructed
 *                 (period (.) package separators are used in this case)
 *     superClass - if non-NULL, the superclass of this class.  Note that
 *                  JEMCC classes should ideally have another JEMCC class as
 *                  the superclass and only core Object related classes
 *                  should have a NULL superclass
 *     interfaces - an array reference to a list of direct interfaces
 *                  implemented by this class
 *     interfaceCount - the number of interface instances in the interfaces
 *                      array
 *     methodInfo - an array of JEMCC method definitions for this class
 *     methodCount - the number of method records in the methodInfo array
 *     objMgmtFn - a reference to the object management method, NULL if
 *                 default management operations apply to this class
 *                 (no override function exists)
 *     fieldInfo - an array of JEMCC field definitions for this class
 *     fieldCount - the number of field records in the fieldInfo array
 *     linkInfo - an array of external LinkData definitions
 *     linkCount - the number of link definitions in the linkInfo array
 *     initData - an array of initialization values for the class, in the
 *                same order/types as the field declarations.  This array
 *                must have at least as many entries as there are static
 *                field entries in the class
 *     classInst - a class instance reference through which the
 *                 constructed class instance is returned (or the already
 *                 existing class instance if a conflict occurs)
 *
 * Returns:
 *     JNI_OK - the building/linking/definition of the JEMCC class was
 *              successful
 *     JNI_ERR - a definition error occurred during the creation of the class.
 *               An ClassFormatError/AbstractMethodError/LinkageError will 
 *               have been thrown in the current environment
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - provided JEMCC initialization data is invalid
 *     AbstractMethodError - this concrete class is missing a non-abstract
 *                           method definition
 *     LinkageError - one of the class linkages failed (reference not found)
 */
jint JEMCC_CreateStdClass(JNIEnv *env, JEMCC_Object *loader, juint accessFlags,
                          const char *className, JEMCC_Class *superClass,
                          JEMCC_Class **interfaces, jsize interfaceCount,
                          JEMCC_MethodData *methodInfo, jsize methodCount,
                          JEMCC_ObjMgmtFn *objMgmtFn,
                          JEMCC_FieldData *fieldInfo, jsize fieldCount,
                          JEMCC_LinkData *linkInfo, jsize linkCount,
                          JEMCC_ReturnValue *initData, 
                          JEMCC_Class **classInst) {
    JEMCC_Class *clInst;
    jint rc;

    rc = JEMCC_BuildClass(env, loader, accessFlags, className, superClass,
                          interfaces, interfaceCount, methodInfo, methodCount,
                          objMgmtFn, fieldInfo, fieldCount, &clInst);
    if (rc != JNI_OK) return rc;

    if (linkCount != 0) {
        rc = JEMCC_LinkClass(env, clInst, linkInfo, linkCount);
        if (rc != JNI_OK) {
            JEM_DestroyClassInstance(env, clInst);
            return rc;
        }
    }

    if (initData != NULL) JEMCC_InitializeStaticClassFields(env, clInst,
                                                            initData);

    rc = JEMCC_RegisterClass(env, loader, &clInst);
    if (rc != JNI_OK) return rc;

    if (classInst != NULL) *classInst = clInst;
    return JNI_OK;
}

/**
 * Convenience method to define a "standard" throwable instance.  These are
 * classes which originate from another "standard" throwable class, define no
 * additional fields or methods and only have the basic constuctors.
 * Internally, the VM saves space with these classes by copying the root 
 * definition tables from the Throwable class itself.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the constructed class
 *              instance (class will be registered)
 *     className - the fully qualified name of the class being constructed
 *                 (period (.) package separators are used in this case)
 *     superClass - if superclass of this class.  Must be another "standard"
 *                  throwable class instance
 *     classInst - a class instance reference through which the
 *                 constructed class instance is returned
 *
 * Returns:
 *     JNI_OK - the building/linking/definition of the standard throwable class
 *              was successful
 *     JNI_ERR - the superclass is not a standard throwable, so the class
 *               instance could not be created.  A LinkageError has been
 *               thrown in the current environment
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     LinkageError - the superclass is not a standard throwable and linkage
 *                    could not occur
 */
jint JEMCC_CreateStdThrowableClass(JNIEnv *env, JEMCC_Object *loader,
                                   const char *className,
                                   JEMCC_Class *superClass,
                                   JEMCC_Class **classInst) {
    JEMCC_Class *clInst;
    JEM_ClassData *superClassData, *classData;
    jint rc, recordCount;

    /* Validate that the superclass is a standard throwable */
    superClassData = superClass->classData;
    if ((superClassData->accessFlags & ACC_STD_THROW) == 0) {
        JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_LinkageError,
                          NULL, className,
                          ": Std throwable must have std throwable superclass",
                          NULL);
        return JNI_ERR;
    }

    /* Construct the Class object instance */
    clInst = (JEMCC_Class *) JEM_AllocateClass(env);
    if (clInst == NULL) return JNI_ENOMEM;
    classData = clInst->classData;

    /* Methods/fields originate from the parent exception class */
    (void) memcpy(classData, superClassData, sizeof(JEM_ClassData));

    /* All except the name, assignments and method link tables */
    classData->className = (char *) className;
    classData->assignList = NULL;
    classData->methodLinkTables = NULL;

    /* And superclass/superinterface linkage information */
    recordCount = superClassData->assignmentCount + 1;
    classData->assignList = (JEMCC_Class **)
                JEMCC_Malloc(env, (recordCount + 1) * sizeof(JEMCC_Class *));
    if (classData->assignList == NULL) {
        JEM_DestroyClassInstance(env, clInst);
        return JNI_ENOMEM;
    }
    *(classData->assignList) = superClass;
    (void) memcpy(classData->assignList + 1, superClassData->assignList,
                  superClassData->assignmentCount * sizeof(JEMCC_Class *));
    classData->interfaceCount = 0;
    classData->assignmentCount = recordCount;

    /* Finally, rebuild the method linkage tables (insert newest class) */
    classData->methodLinkTables = (JEM_ClassMethodData ***)
         JEMCC_Malloc(env, (recordCount + 1) * sizeof(JEM_ClassMethodData **));
    if (classData->methodLinkTables == NULL) {
        JEM_DestroyClassInstance(env, clInst);
        return JNI_ENOMEM;
    }
    (void) memcpy(classData->methodLinkTables + 1, 
                  superClassData->methodLinkTables,
                  recordCount * sizeof(JEM_ClassMethodData **));
    classData->methodLinkTables[0] = classData->methodLinkTables[1];

    /* Store class definition in the JEMCC namespace */
    rc = JEMCC_RegisterClass(env, loader, &clInst);
    if (rc != JNI_OK) return rc;

    if (classInst != NULL) *classInst = clInst;
    return JNI_OK;
}
