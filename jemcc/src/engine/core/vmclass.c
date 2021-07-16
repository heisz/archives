/**
 * Full JEMCC class support, with VM support for lookups and initialization.
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
#include "jnifunc.h"

/* Read the separated class definition and packaging methods */
#include "class.c"

/**
 * Internal method to construct array class instances on demand.  This method
 * manually constructs the JEMCC_ArrayClass instance (which overlays the
 * standard JEMCC_Class structure), utilizing the base class method/field
 * structures from the primitive array instance.  Note that the generated
 * array class instance for object-based arrays are stored in the
 * component classloader (while primitive-based arrays are stored in the
 * VM loader), to reduce the number of definitions of the array class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader from which the array class is to be defined.  
 *              NULL if the VM bootstrap loader is to be used
 *     arrayName - the name of the array instances (Java descriptor).  Parsed
 *                 for depth as well as primitive/component class information
 *     isLink - if JNI_TRUE, this array is being defined as part of the 
 *              construction of another class (linking) and should throw
 *              NoClassDefFoundError on build failure.  If JNI_FALSE, should
 *              throw ClassNotFoundException instead (from a forName() call,
 *              for instance)
 *     localOnly - determines how the component class is "located" for 
 *                 object-based arrays.  If JNI_TRUE, only look in the
 *                 hashtable of the provided classloader (used for manual
 *                 classloading).  If JNI_FALSE, use the LocateClass method
 *                 to search for the class instance (used for linking).  Only
 *                 the latter case will throw an exception if the class is
 *                 not found.
 *     classInst - a class instance reference through which the constructed
 *                 array class instance is returned (if successful)
 *
 * Returns:
 *     JNI_OK - the array class was successfully constructed and the class
 *              instance has been returned through the classInst reference
 *     JNI_ERR - an error was found in the specification of the array
 *               name (invalid depth or bad component descriptor - an
 *               exception has been thrown in the current environment, based
 *               on the isLink value) or a component class load error occurred
 *               (with exception)
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *     JNI_EINVAL - the requested array was object based and the associated
 *                  component class was not found.  An exception has been
 *                  thrown in the current environment, based on the isLink
 *                  value, if the localOnly value was false (linking requires
 *                  class to be found)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     NoClassDefFoundError - the array specification was invalid or the
 *                            associated component class was not found
 *     ClassNotFoundException - the array specification was invalid or the
 *                              associated component class was not found
 */
jint JEM_BuildArrayClass(JNIEnv *env, JEMCC_Object *loader, 
                         const char *arrayName, jboolean isLink,
                         jboolean localOnly, JEMCC_Class **classInst) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_Class *primArrayClass = VM_CLASS(JEMCC_Primitive_Array);
    JEMCC_Class *componentClass, *existingClass;
    JEMCC_ArrayClass *arrayClass;
    JEM_DescriptorData componentDesc;
    JEMCC_Object *componentLoader, *loadExc;
    jint rc, descTag, depth = 0;
    const char *compName;
    juint origFrameOpFlags;
    jint errIdx = (isLink == JNI_FALSE) ? JEMCC_Class_ClassNotFoundException :
                                          JEMCC_Class_NoClassDefFoundError;

    /* Remove and count the array depth indicators */
    compName = arrayName;
    while (*compName == '[') {
        compName++;
        depth++;
    }
    if (*compName == '\0') {
        JEMCC_ThrowStdThrowableIdxV(env, errIdx, NULL,
                                    "Missing array component type: ",
                                    arrayName, NULL);
        return JNI_ERR;
    } else if (depth > 255) {
        JEMCC_ThrowStdThrowableIdxV(env, errIdx, NULL,
                                    "Array depth over 255: ", 
                                    arrayName, NULL);
        return JNI_ERR;
    }

    /* Determine and check the component type of the array (with capture) */
    origFrameOpFlags = jenv->topFrame->opFlags;
    jenv->topFrame->opFlags |= FRAME_THROWABLE_CAPTURE;
    if (JEM_ParseDescriptor(env, compName, &componentDesc, NULL,
                            JNI_FALSE) == NULL) {
        loadExc = jenv->pendingException;
        jenv->topFrame->opFlags = origFrameOpFlags;
        jenv->pendingException = NULL;

        /* Rethrow out of memory, recast everything else */
        if (loadExc != NULL) {
            if (loadExc->classReference ==
                                  VM_CLASS(JEMCC_Class_OutOfMemoryError)) {
                JEMCC_ProcessThrowable(env, loadExc);
            } else {
                JEMCC_ThrowStdThrowableIdxV(env, errIdx, NULL,
                                        "Invalid array component descriptor: ",
                                        arrayName, NULL);
            }
        }
        return JNI_ERR;
    }
    jenv->topFrame->opFlags = origFrameOpFlags;
    if ((componentDesc.generic.tag & 
                  (DESCRIPTOR_BaseType | DESCRIPTOR_ObjectType)) == 0) {
        JEMCC_ThrowStdThrowableIdxV(env, errIdx, NULL,
                                    "Invalid array component type: ",
                                    arrayName, NULL);
        JEM_DestroyDescriptor(&componentDesc, JNI_FALSE);
        return JNI_ERR;
    }

    /* If componentType is not primitive, locate the reference class */
    if ((componentDesc.generic.tag & DESCRIPTOR_ObjectType) != 0) {
        if (localOnly == JNI_FALSE) {
            /* Non-local, invoke full lookup for component class */
            rc = JEMCC_LocateClass(env, loader, 
                                   componentDesc.object_info.className,
                                   isLink, &componentClass);
            if (rc != JNI_OK) {
                /* Exception has been thrown by class load operations */
                JEM_DestroyDescriptor(&componentDesc, JNI_FALSE);
                return rc;
            }

            /* See if array instance already defined (different loader) */
            componentLoader = componentClass->classData->classLoader;
            if (componentLoader != loader) {
                rc = JEM_RetrieveClass(env, componentLoader, arrayName,
                                       classInst);
                if (rc != JNI_EINVAL) return rc;
            }
        } else {
            /* Only check loader (assumes local array lookup already done) */
            rc = JEM_RetrieveClass(env, loader,
                                   componentDesc.object_info.className,
                                   &componentClass);
            if (rc != JNI_OK) {
                /* Quietly make our exit, not available from local hash */
                JEM_DestroyDescriptor(&componentDesc, JNI_FALSE);
                return rc;
            }
        }
    } else {
        /* No component, but perhaps primitive array already defined in VM */
        componentClass = NULL;
        rc = JEM_RetrieveClass(env, NULL, arrayName, classInst);
        if (rc != JNI_EINVAL) return rc;
    }
    descTag = componentDesc.generic.tag;
    JEM_DestroyDescriptor(&componentDesc, JNI_FALSE);

    /* Create the class instance.  Not standard as typing refs are required */
    arrayClass = (JEMCC_ArrayClass *) JEMCC_Malloc(env, 
                                                   sizeof(JEMCC_ArrayClass));
    if (arrayClass == NULL) return JNI_ENOMEM;
    arrayClass->classData = JEMCC_Malloc(env, sizeof(JEM_ClassData));
    if (arrayClass->classData == NULL) {
        JEMCC_Free(arrayClass);
        return JNI_ENOMEM;
    }
    arrayClass->classReference = VM_CLASS(JEMCC_Class_Class);
    arrayClass->objStateSet = 0;

    /* Clone the primitive array class definitions, except for className */
    (void) memcpy(arrayClass->classData, primArrayClass->classData,
                  sizeof(JEM_ClassData));
    arrayClass->classData->accessFlags = ACC_PUBLIC | ACC_FINAL | ACC_ARRAY |
                                         ACC_JEMCC | ACC_NATIVE_DATA;
    arrayClass->classData->className = JEMCC_StrDupFn(env, (void *) arrayName);
    if (arrayClass->classData->className == NULL) {
        JEMCC_Free(arrayClass->classData);
        JEMCC_Free(arrayClass);
        return JNI_ENOMEM;
    }

    /* Based on the descriptor type, finalize the class definition */
    switch (descTag) {
        case BASETYPE_Boolean:
            arrayClass->typeDepthInfo = PRIMITIVE_BOOLEAN | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Boolean);
            break;
        case BASETYPE_Byte:
            arrayClass->typeDepthInfo = PRIMITIVE_BYTE | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Byte);
            break;
        case BASETYPE_Char:
            arrayClass->typeDepthInfo = PRIMITIVE_CHAR | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Char);
            break;
        case BASETYPE_Short:
            arrayClass->typeDepthInfo = PRIMITIVE_SHORT | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Short);
            break;
        case BASETYPE_Int:
            arrayClass->typeDepthInfo = PRIMITIVE_INT | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Int);
            break;
        case BASETYPE_Float:
            arrayClass->typeDepthInfo = PRIMITIVE_FLOAT | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Float);
            break;
        case BASETYPE_Long:
            arrayClass->typeDepthInfo = PRIMITIVE_LONG | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Long);
            break;
        case BASETYPE_Double:
            arrayClass->typeDepthInfo = PRIMITIVE_DOUBLE | depth;
            arrayClass->referenceClass = VM_CLASS(JEMCC_Primitive_Double);
            break;
        case DESCRIPTOR_ObjectType:
            arrayClass->typeDepthInfo = depth;
            arrayClass->referenceClass = componentClass;
            break;
        default:
            abort();
    }

    /* Store the array definition for future generations */
    if ((descTag & DESCRIPTOR_BaseType) != 0) {
        /* Primitive array classes are stored in JEMCC VM table */
        rc = JEM_ClassNameSpaceStore(env, NULL, (JEMCC_Class *) arrayClass, 
                                     &existingClass, JNI_TRUE);
    } else {
        /* Object array classes are stored in the object's classloader */
        componentLoader = componentClass->classData->classLoader;
        rc = JEM_ClassNameSpaceStore(env, componentLoader, 
                                     (JEMCC_Class *) arrayClass, 
                                     &existingClass, JNI_TRUE);
    }

    /* Handle the error/existing cases appropriately */
    if (rc != JNI_OK) {
        /* Clean up the local instance */
        JEMCC_Free(arrayClass->classData->className);
        JEMCC_Free(arrayClass->classData);
        JEMCC_Free(arrayClass);

        /* If it already exists in the classloader, return that instance */
        if (rc == JNI_ENOMEM) {
            return JNI_ENOMEM;
        } else {
            arrayClass = (JEMCC_ArrayClass *) existingClass;
        }
    }

    if (classInst != NULL) *classInst = (JEMCC_Class *) arrayClass;
    return JNI_OK;
}

/**
 * Load a "system" class (available via the CLASSPATH definitions in the
 * VM properties).  This method is not intended for general consumption -
 * it is equivalent to calling ClassLoader.findSystemClass() (e.g. load through
 * the system classloader) but does NOT perform the associated checks for
 * already loaded classes or classes available through the bootstrap loader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     className - the fully qualified name of the class to load
 *     classInst - a class instance reference through which the desired class
 *                 is returned (if found)
 *
 * Returns:
 *     JNI_OK - the class was successfully located/constructed and has been
 *              returned in the classInst reference
 *     JNI_ERR - an error has occurred in the loading/parsing/linking of
 *               the requested class (an exception has been thrown in the
 *               current environment)
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *     JNI_EINVAL - the requested class was not found or recognized - no
 *                  exception is thrown in this instance
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     <other class errors> - a failure occurred in the parsing, linking
 *                            or verification of the class (if found)
 */
jint JEM_GetSystemClass(JNIEnv *env, const char *className,
                        JEMCC_Class **classInst) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;
    JEM_ParsedClassData *pData;
    jbyte *rawClassData;
    int rc, rawClassLen;
    char *ptr, *fileName;

    /* Build the 'directory' classname for loading */
    if (JEMCC_EnvStrBufferInit(env, 100) == NULL) return JNI_ENOMEM;
    if (JEMCC_EnvStrBufferAppendSet(env, (char *) className, ".class", 
                                    (char *) NULL) == NULL) return JNI_ENOMEM;
    if ((fileName = JEMCC_EnvStrBufferDup(env)) == NULL) return JNI_ENOMEM;
    ptr = fileName + strlen(fileName) - 7;
    while (ptr >= fileName) {
        if (*ptr == '.') *ptr = JEMCC_FileSeparator;
        ptr--;
    }

    /* Locate the class instance using the VM classpath information */
    rc = JEM_ReadPathFileContents(env, &(jvm->classPath), fileName,
                                  &rawClassData, &rawClassLen);
    JEMCC_Free(fileName);
    if ((rc == JNI_OK) && (rawClassLen >= 0)) {
        /* Parse and link it */
        pData = JEM_ParseClassData(env, rawClassData, rawClassLen);
        JEMCC_Free(rawClassData);
        if (pData == NULL) return JNI_ENOMEM;
        *classInst = JEM_DefineAndResolveClass(env, jvm->systemClassLoader, 
                                               pData);
        return ((*classInst == NULL) ? JNI_ERR : JNI_OK);
    }

    // No matching class information found, return in silence */
    return JNI_EINVAL;
}

/* External method from classloader.c (loadClass implementation) */
extern jint JEM_ClassLoader_LoadClass(JNIEnv *env, JEMCC_Object *loader,
                                      const char *className,
                                      JEMCC_Object *classNameString,
                                      JEMCC_Class **classInst);

/**
 * External method for locating class instances by name.  Will operate with
 * a specific classloader instance or the VM bootstrap classloader.  This
 * method is similar to the ClassLoader.loadClass() method (and does in fact
 * call it where appropriate), but there are a number of subtle differences:
 *
 *   - it always reads the JVM bootstrap classtable first, to allow rapid
 *     retrieval of bootstrap JEMCC classes and to prevent any possible
 *     overriding of the bootstrap specific classes
 *   - array classes are constructed within this method, but directly
 *     manipulate the classloader from which the component class is obtained
 *     (similar to utilizing the classloading chain without the calling 
 *     sequences)
 *   - the classloader's internal classtable is consulted for the class
 *     instance prior to any attempt to load/define the class (rather than
 *     relying on any loadClass() hashing instances)
 *   - the JEMCC package initializers are called before the loadClass()
 *     method on the classloader to permit native integration (and to
 *     avoid potential conflicts with concurrent bytecode class
 *     implementations)
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to supply the requested class.  NULL
 *              if the class is to be loaded from the VM bootstrap loader
 *     className - the fully qualified name of the class to be loaded (both
 *                 '/' and '.' separators are recognized, which are accepted
 *                 by the ClassLoader.loadClass() method)
 *     isLink - if JNI_TRUE, this class is being located as part of the
 *              construction of another class (linking) and should throw
 *              NoClassDefFoundError if the requested instance is not found.
 *              If JNI_FALSE, throw ClassNotFoundException instead (from a
 *              forName() call, for instance)
 *     classInst - a class instance reference through which the desired class
 *                 is returned (if found)
 *
 * Returns:
 *     JNI_OK - the class was successfully located/constructed and has been
 *              returned in the classInst reference
 *     JNI_ERR - an error has occurred in the loading/parsing/linking of
 *               the requested class (an exception has been thrown in the
 *               current environment, based on the isLink value)
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *     JNI_EINVAL - the requested class was not found or recognized and a
 *                  ClassNotFoundException or NoClassDefFoundError has been 
 *                  thrown in the current environment, depending on the isLink
 *                  value)
 *
 * Exceptions:
 *     Many exceptions are possibly thrown, all corresponding to some failure
 *     to load the class (format, data, memory, etc.).  Note that an exception
 *     is guaranteed to be thrown (often ClassNotFoundException or 
 *     NoClassDefFoundError) if no class instance is returned from this 
 *     method (and vice versa, no exception is thrown if a class is returned).
 */
jint JEMCC_LocateClass(JNIEnv *env, JEMCC_Object *loader, const char *className,
                       jboolean isLink, JEMCC_Class **classInst) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    jint errIdx = (isLink == JNI_FALSE) ? JEMCC_Class_ClassNotFoundException :
                                          JEMCC_Class_NoClassDefFoundError;
    int rc;

    /* Search the JEMCC registration hash first.  Two reasons: */
    /*    - it makes it much faster to locate native classes    */
    /*    - it prevents definition of JEMCC overrides in Java  */
    *classInst = NULL;
    rc = JEM_RetrieveClass(env, NULL, className, classInst);
    if (rc != JNI_EINVAL) return rc;

    /* Is it part of a yet to be loaded bootstrap JEMCC package? */
    if (*className != '[') {
        rc = JEM_LocateClassByPackage(env, NULL, className, classInst);
        if ((rc != JNI_OK) || (*classInst != NULL)) return rc;
    }

    /* Not 'native' JEMCC.  Look in the classloader definition table */
    if (loader != NULL) {
        rc = JEM_RetrieveClass(env, loader, className, classInst);
        if (rc != JNI_EINVAL) return rc;

        /* Maybe a localized JEMCC package definition */
        if (*className != '[') {
            rc = JEM_LocateClassByPackage(env, loader, className, classInst);
            if ((rc != JNI_OK) || (*classInst != NULL)) return rc;
        }
    }

    /* Handle array classes "specially" */
    if (*className == '[') {
        return JEM_BuildArrayClass(env, loader, className, isLink, 
                                   JNI_FALSE, classInst);
    }

    /* Bootstrap classloader can do no more */
    if (loader == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, errIdx, NULL, className);
        return JNI_EINVAL;
    }

    /* If this is the system classloader, take a shortcut */
    if (loader == jvm->systemClassLoader) {
        rc = JEM_GetSystemClass(env, className, classInst);
        if (rc == JNI_EINVAL) {
            JEMCC_ThrowStdThrowableIdx(env, errIdx, NULL, className);
        }
        return rc;
    }

    /* Not available by direct means, use the classloader sequences */
    rc = JEM_ClassLoader_LoadClass(env, loader, className, NULL, classInst);
    if (rc != JNI_EINVAL) return rc;

    /* All out of options, guess the class is unavailable */
    JEMCC_ThrowStdThrowableIdx(env, errIdx, NULL, className);
    return JNI_EINVAL;
}

/**
 * Method which launches class initialization, if required.  This method
 * will block if another thread is initializing the class, until the
 * class initialization is complete.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class to be initialized
 *
 * Returns:
 *     JNI_OK - the class initialization was successful
 *     JNI_ERR - an error occurred while initializing the class.  An exception
 *               has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory error occurred while handling an init
 *                        exception
 *     ExceptionInInitializerError - an exception occurred during the
 *                                   operation of the initialization method
 *     NoClassDefFoundError - the class was previously initialized and
 *                            an error had occurred at that time
 */
jint JEMCC_InitializeClass(JNIEnv *env, JEMCC_Class *classInst) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_Class *superClass;
    JEM_ClassData *classData;
    JEMCC_Object *initExc;
    juint origFrameOpFlags;
    jint initComplete = JNI_FALSE;

    /* Handle base/primitive class conditions */
    if (classInst == NULL) return JNI_OK;
    classData = classInst->classData;

    /* Quick check to save a lock overhead (dominant case) */
    if (classData->resolveInitState > JEM_CLASS_INIT_IN_PROGRESS) {
        if (classData->resolveInitState == JEM_CLASS_INIT_COMPLETE) {
            return JNI_OK;
        }
        /* Java VM spec states that NoClassDef is thrown in error case */
        JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_NoClassDefFoundError,
                                    NULL, classData->className,
                                    ": Prior error in class initializer",
                                    NULL);
        return JNI_ERR;
    }

    /* Also quick return where self-initialization in-progress */
    if ((classData->resolveInitState == JEM_CLASS_INIT_IN_PROGRESS) &&
           (classData->resolveInitThread == ((JEM_JNIEnv *) env)->envThread)) {
        return JNI_OK;
    }

    /* From this point on, work in full control of object */
    if (JEMCC_EnterObjMonitor(env, (jobject) classInst) != JNI_OK) {
        return JNI_ERR;
    }

    /* This may repeat depending on wait conditions */
    while (initComplete == JNI_FALSE) {
        switch (classData->resolveInitState) {
            case JEM_CLASS_INIT_COMPLETE:
                /* Mark as complete and exit normally */
                if (JEMCC_ExitObjMonitor(env, 
                                    (jobject) classInst) != JNI_OK) abort();
                initComplete = JNI_TRUE;
                break;
            case JEM_CLASS_INIT_FAILED:
                /* Throw NoClassDefError (JVM spec) and return abnormally */
                if (JEMCC_ExitObjMonitor(env, 
                                    (jobject) classInst) != JNI_OK) abort();
                JEMCC_ThrowStdThrowableIdxV(env, 
                                        JEMCC_Class_NoClassDefFoundError,
                                        NULL, classData->className,
                                        ": Prior error in class initializer",
                                        NULL);
                initComplete = JNI_TRUE;
                break;
            case JEM_CLASS_INIT_IN_PROGRESS:
                /* Just wait for the other thread to complete and loop again */
                /* TODO - add 'quiet' mode when interrupts are supported */
                if (JEMCC_ObjMonitorMilliWait(env, (jobject) classInst,
                                              200) != JNI_OK) abort();
                break;
            default:
                /* All other states allow this thread to initialize */
                classData->resolveInitState = JEM_CLASS_INIT_IN_PROGRESS;
                classData->resolveInitThread = ((JEM_JNIEnv *) env)->envThread;
                if (JEMCC_ExitObjMonitor(env, 
                                    (jobject) classInst) != JNI_OK) abort();

                /* Initialize superclass as required */
                superClass = *(classData->assignList);
                if (superClass != NULL) {
                    if (JEMCC_InitializeClass(env, superClass) != JNI_OK) {
                        /* Higher level exception, this class is also toast */
                        classData->resolveInitState = JEM_CLASS_INIT_FAILED;
                        initComplete = JNI_TRUE;
                        break;
                    } 
                }

                /* Perform the delayed linkage actions, only for bytecode */
                if ((classData->accessFlags & ACC_JEMCC) == 0) {
                    /* Construct the external field/method references */
                    if (JEM_LinkClassReferences(env, classData) != JNI_OK) {
                        /* TODO - mem fail only? */
                        classData->resolveInitState = JEM_CLASS_INIT_FAILED;
                        initComplete = JNI_TRUE;
                        break;
                    }

                    /* Perform class method verification */
                    if (JEM_VerifyClassByteCode(env, classData) != JNI_OK) {
                        /* TODO - mem fail only? */
                        classData->resolveInitState = JEM_CLASS_INIT_FAILED;
                        initComplete = JNI_TRUE;
                        break;
                    }

                    /* All done linkages, set the parse data free */
                    JEM_DestroyParsedClassData(classData->parseData);
                    classData->parseData = NULL;
                }

                /* Perhaps that is all that is required */
                if ((classData->localMethodCount == 0) ||
                    (strcmp(classData->localMethods[0].name, 
                                                      "<clinit>") != 0)) {
                    classData->resolveInitState = JEM_CLASS_INIT_COMPLETE;
                    initComplete = JNI_TRUE;
                    break;
                }

                /* Call the initialization method, with exception capture */
                origFrameOpFlags = jenv->topFrame->opFlags;
                jenv->topFrame->opFlags |= FRAME_THROWABLE_CAPTURE;
                if (JEM_PushFrame(env,
                                  (jmethodID) &(classData->localMethods[0]),
                                  NULL) == JNI_OK) {
                    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
                }
                jenv->topFrame->opFlags = origFrameOpFlags;

                if (jenv->pendingException != NULL) {
                    initExc = jenv->pendingException;
                    jenv->pendingException = NULL;

                    /* Throw Errors, recast all others */
                    if (initExc != NULL) {
                        if (JEMCC_IsAssignableFrom(env, initExc->classReference,
                                     VM_CLASS(JEMCC_Class_Error)) == JNI_TRUE) {
                            JEMCC_ProcessThrowable(env, initExc);
                        } else {
                            JEMCC_ThrowStdThrowableIdxV(env,
                                       JEMCC_Class_ExceptionInInitializerError,
                                       NULL, classData->className,
                                       ": Failed to initialize class",
                                       NULL);
                        }
                    }

                    classData->resolveInitState = JEM_CLASS_INIT_FAILED;
                } else {
                    classData->resolveInitState = JEM_CLASS_INIT_COMPLETE;
                }
                if (JEMCC_EnterObjMonitor(env, (jobject) classInst) == JNI_OK) {
                    /* Do the notify thing */
                    /* Note that if lock fails, timeout will allow recovery */
                    if (JEMCC_ObjMonitorNotifyAll(env,
                                       (jobject) classInst) != JNI_OK) abort();
                    if (JEMCC_ExitObjMonitor(env, 
                                       (jobject) classInst) != JNI_OK) abort();
                }
                initComplete = JNI_TRUE;
                break;
        }
    }

    /* All done, pass along the final state */
    return (classData->resolveInitState == JEM_CLASS_INIT_FAILED) ? JNI_ERR :
                                                                    JNI_OK;
}
