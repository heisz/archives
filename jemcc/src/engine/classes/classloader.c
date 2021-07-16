/**
 * JEMCC definitions of the java.lang.ClassLoader class.
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
#include "jnifunc.h"

/* Local class method table indices */
#define CLASSLDR_LOADCLASS_IDX 9
#define CLASSLDR_LOADCLASSZ_IDX 10

/* Native data structures for the classloader instance */
struct ClassLoaderNativeLib;

typedef struct ClassLoaderNativeLib {
    JEM_DynaLib library;
    struct ClassLoaderNativeLib *nextNativeLib;
} ClassLoaderNativeLib;

typedef struct {
    JEMCC_HashTable classTable;
    ClassLoaderNativeLib *nativeLibs;
} ClassLoaderData;

/**
 * Initialize the ClassLoader data structures (similar to calling the
 * constructor) - exposed for use in initializing the system classloader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader instance to initialize
 *     parentLoader - the parent loader for this classloader instance
 *
 * Returns:
 *     JNI_OK if initialization was successful, JNI_ENOMEM otherwise (an 
 *     exception will have been thrown in the current environment)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEM_ClassLoader_Init(JNIEnv *env, JEMCC_Object *loader) {
    JEMCC_ObjectExt *clObj = (JEMCC_ObjectExt *) loader;
    ClassLoaderData *clData = (ClassLoaderData *) &(clObj->objectData);

    /* Initialize the hashtable */
    if (JEMCC_HashInitTable(env, &(clData->classTable), -1) != JNI_OK) {
        return JNI_ENOMEM;
    }

    return JNI_OK;
}

/**
 * Perform the ClassLoader.loadClass() operation.  This method is highly
 * optimized to avoid making bytecode method calls where the default ClassLoader
 * implementation is in effect.  This method is also exposed to allow direct 
 * access from the LocateClass() (linking) method.
 *
 * NOTE: this method assumes that the classloader has already been checked
 *       for an existing instance of the class (e.g. findLoadedClass() has
 *       already occurred and that the NULL (bootstrap) load sequence has
 *       been attempted.  The "real" loadClass() method(s) below will have
 *       already done so.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader on which this operation is taking place
 *     className - the UTF8 encoded class name characters
 *     classNameString - the String instance of the class name (may be NULL)
 *     classInst - a class instance reference through which the requested
 *                 class is returned
 *
 * Returns:
 *     JNI_OK - the class was successfully located/constructed and has been
 *              returned in the classInst reference
 *     JNI_ERR - an error has occurred in the loading/parsing/linking of
 *               the requested class (an exception has been thrown in the
 *               current environment)
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *     JNI_EINVAL - the requested class was not found (no exception will be
 *                  thrown, some may be absorbed)
 *
 * Exceptions:
 *     Any of the possible errors which could arise from the reading, parsing
 *     linking or verifying of a bytecode class or the compilation of JEMCC
 *     classes or packages.
 */
jint JEM_ClassLoader_LoadClass(JNIEnv *env, JEMCC_Object *loader,
                               const char *className, 
                               JEMCC_Object *classNameString,
                               JEMCC_Class **classInst) {
    JEMCC_Class *loaderClass = VM_CLASS(JEMCC_Class_ClassLoader);
    JEMCC_VMFrame *frame = JEMCC_GetCurrentVMFrame(env);
    JEMCC_ReturnValue retVal;
    JEMCC_Object *nameStr;
    jint rc;

fprintf(stderr, "Invoking ClassLoader_loadClass for %p -> %s\n", 
                loader, className);
    JEMCC_CHECK_METHOD_REFERENCE(env, loaderClass, CLASSLDR_LOADCLASS_IDX,
                                 "loadClass",  
                                 "(Ljava/lang/String;)Ljava/lang/Class;");
    JEMCC_CHECK_METHOD_REFERENCE(env, loaderClass, CLASSLDR_LOADCLASSZ_IDX,
                                 "loadClass",  
                                 "(Ljava/lang/String;Z)Ljava/lang/Class;");
    if (JEMCC_IsDefaultMethod(env, loader, loaderClass,
                              CLASSLDR_LOADCLASS_IDX) == JNI_FALSE) {
        /* loadClass(S) overridden, make the call and return */
fprintf(stderr, "***** loadClass(S) overridden *****\n");
    }
    if (JEMCC_IsDefaultMethod(env, loader, loaderClass,
                              CLASSLDR_LOADCLASSZ_IDX) == JNI_FALSE) {
        /* loadClass(SZ) overridden, make the call and return */
fprintf(stderr, "***** loadClass(SZ) overridden *****\n");
        nameStr = JEMCC_NewStringUTF(env, className);
        if (nameStr == NULL) return JNI_ENOMEM;
        JEMCC_PUSH_STACK_OBJECT(frame, loader);
        JEMCC_PUSH_STACK_OBJECT(frame, nameStr);
        JEMCC_PUSH_STACK_INT(frame, (jint) JNI_FALSE);

        rc = JEMCC_ExecuteInstanceMethod(env, loader, loaderClass,
                                         CLASSLDR_LOADCLASSZ_IDX,
                                         JEMCC_VIRTUAL_METHOD, &retVal);
        if (rc != JNI_OK) {
/* NEED TO CAPTURE! */
            return rc;
        }
        *classInst = (JEMCC_Class *) retVal.objVal;
        return JNI_OK;
    }

    /* TODO - if not defined, abstract method error? */

    /* TODO - if loadClass(S) or loadClass(S, Z) is overridden, call it */
    /* TODO - unless, of course, that's where this is coming from */
    /* TODO - remember to wrap/capture exceptions */

    /* Note: findLoadedClass() has already been performed by the caller */
    /* Bootstrap (NULL) loader has already been checked as well */

    /* No sign of the requested class */
    return JNI_EINVAL;
}

/**
 * Perform a native library load against this classloader.  Optimized to
 * utilize the findLibrary() call only if the specified load implementation
 * defines that method.  Also encapsulates the other standard library
 * load operations and the linkages between the ClassLoader and the library
 * instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader against which the native load is to occur
 *     libName - the root library name to be loaded.  Will convert to the
 *               system specific library using the MapLibraryName method
 *
 * Returns:
 *     JNI_OK - the library was successfully loaded and linked into the
 *              virtual machine
 *     JNI_ERR - an error has occurred in the loading/linking of the
 *               library instance (an exception has been thrown in the 
 *               current environment)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failure has occurred in the
 *                        loading/tracking of the library
 *     UnsatisfiedLinkError - the library load operation failed
 *
 *     Other exceptions may arise due to IOErrors, etc. during the load
 *     operations. TODO - is this the case?
 */
jint JEM_ClassLoader_LoadLibrary(JNIEnv *env, JEMCC_Object *loader,
                                 const char *libName) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_JavaVM *jvm = (JEM_JavaVM *) jenv->parentVM;
    JEMCC_ObjectExt *clObj = (JEMCC_ObjectExt *) loader;
    ClassLoaderData *clData = (ClassLoaderData *) &(clObj->objectData);
    ClassLoaderNativeLib *nativeLib;
    char *libFileName;

    /* TODO - handle findLibrary() call here, only if defined! */

    /* Determine the system specific library name */
    libFileName = JEM_MapLibraryName(env, libName);
    if (libFileName == NULL) return JNI_ERR;

    /* Pre-allocate the native library record */
    nativeLib = (ClassLoaderNativeLib *) JEMCC_Malloc(env, 
                                                 sizeof(ClassLoaderNativeLib));
    if (nativeLib == NULL) {
        JEMCC_Free(libFileName);
        return JNI_ERR;
    }

    /* Load the library instance (if possible) */
    if (JEM_LoadPathLibrary(env, jvm->libLoader, &(jvm->libPath), libFileName,
                            NULL, &(nativeLib->library)) != JNI_OK) {
        JEMCC_Free(libFileName);
        JEMCC_Free(nativeLib);
        return JNI_ERR;
    }

    /* TODO - add this classloader as conflict point! */

    /* Insert into the ClassLoader native library list */
    /* TODO - synchronize this! */
    nativeLib->nextNativeLib = clData->nativeLibs;
    clData->nativeLibs = nativeLib;

    return JNI_OK;
}

/**
 * Lookup a native method instance against all native libraries loaded and 
 * linked against this classloader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader with the native library instances to search
 *     methName - the native method instance to retrieve from the libraries
 *
 * Returns:
 *     The native method symbol if found or NULL if the requested method
 *     name does not appear in the native libraries defined in this
 *     classloader (no exception will have been thrown).
 *
 *     TODO - walk through parents as well?
 */
JEM_DynaLibSymbol JEM_ClassLoader_FindNativeMethod(JNIEnv *env, 
                                                   JEMCC_Object *loader,
                                                   const char *methName) {
    JEMCC_ObjectExt *clObj = (JEMCC_ObjectExt *) loader;
    ClassLoaderData *clData = (ClassLoaderData *) &(clObj->objectData);
    ClassLoaderNativeLib *nativeLib = clData->nativeLibs;
    JEM_DynaLibSymbol retSym;

    /* Walk the loaders library list, looking for the named symbol */
    while (nativeLib != NULL) {
        retSym = JEM_DynaLibGetSymbol(nativeLib->library, methName);
        if (retSym != NULL) return retSym;
        nativeLib = nativeLib->nextNativeLib;
    }

    return NULL;
}

static jint JEMCC_ClassLoader_init(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisObj = JEMCC_LOAD_OBJECT(frame, 0);

    /* TODO - check SecurityManager.checkCreateClassLoader() */

    if (JEM_ClassLoader_Init(env, thisObj) != JNI_OK) {
        return JEMCC_ERR;
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_ClassLoader_defineClass_StringByteArrayII(JNIEnv *env,
                                                            JEMCC_VMFrame *frame,
                                                            JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ObjectExt *clName = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 1);
    JEMCC_ArrayObject *byteArray = 
                             (JEMCC_ArrayObject *) JEMCC_LOAD_OBJECT(frame, 2);
    jint offset = JEMCC_LOAD_INT(frame, 3), length = JEMCC_LOAD_INT(frame, 4);
    JEM_ParsedClassData *pData;
    JEMCC_Class *retClass;

fprintf(stderr, "DEFINING CLASS offset %i length %i\n", offset, length);
/* TODO CHECK ARRAY REGION */
    /* Pretty straightforward: parse it, check the name and link it! */
    pData = JEM_ParseClassData(env, ((jbyte *) byteArray->arrayData) + offset,
                               length);
    if (pData == NULL) return JEMCC_ERR;
    retClass = JEM_DefineAndResolveClass(env, (JEMCC_Object *) thisObj, pData);
    if (retClass == NULL) return JEMCC_ERR;

/* XXX CHECK THE NAME */

    retVal->objVal = (JEMCC_Object *) retClass;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_ClassLoader_defineClass_ByteArrayII(JNIEnv *env,
                                                      JEMCC_VMFrame *frame,
                                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_findLoadedClass_String(JNIEnv *env,
                                                     JEMCC_VMFrame *frame,
                                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_findSystemClass_String(JNIEnv *env,
                                                     JEMCC_VMFrame *frame,
                                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_getResource_String(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_getResourceAsStream_String(JNIEnv *env,
                                                         JEMCC_VMFrame *frame,
                                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_getSystemResource_String(JNIEnv *env,
                                                       JEMCC_VMFrame *frame,
                                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_getSystemResourceAsStream_String(JNIEnv *env,
                                                               JEMCC_VMFrame *frame,
                                                               JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_loadClass_String(JNIEnv *env,
                                               JEMCC_VMFrame *frame,
                                               JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_resolveClass_Class(JNIEnv *env,
                                                 JEMCC_VMFrame *frame,
                                                 JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_ClassLoader_setSigners_ClassObjectArray(JNIEnv *env,
                                                          JEMCC_VMFrame *frame,
                                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

/**
 * NB: do not mix this array as the above methods use direct method indices.
 *     In particular:
 *         10 - loadClass(S)
 *         11 - loadClass(SZ)
 */
JEMCC_MethodData JEMCC_ClassLoaderMethods[] = {
    { ACC_PROTECTED,
         "<init>", "()V",
         JEMCC_ClassLoader_init },
    { ACC_PROTECTED | ACC_FINAL,
         "defineClass", "(Ljava/lang/String;[BII)Ljava/lang/Class;",
         JEMCC_ClassLoader_defineClass_StringByteArrayII },
    { ACC_PROTECTED | ACC_FINAL,
         "defineClass", "([BII)Ljava/lang/Class;",
         JEMCC_ClassLoader_defineClass_ByteArrayII },
    { ACC_PROTECTED | ACC_FINAL,
         "findLoadedClass", "(Ljava/lang/String;)Ljava/lang/Class;",
         JEMCC_ClassLoader_findLoadedClass_String },
    { ACC_PROTECTED | ACC_FINAL,
         "findSystemClass", "(Ljava/lang/String;)Ljava/lang/Class;",
         JEMCC_ClassLoader_findSystemClass_String },
    { ACC_PUBLIC,
         "getResource", "(Ljava/lang/String;)Ljava/net/URL;",
         JEMCC_ClassLoader_getResource_String },
    { ACC_PUBLIC,
         "getResourceAsStream", "(Ljava/lang/String;)Ljava/io/InputStream;",
         JEMCC_ClassLoader_getResourceAsStream_String },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL,
         "getSystemResource", "(Ljava/lang/String;)Ljava/net/URL;",
         JEMCC_ClassLoader_getSystemResource_String },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL,
         "getSystemResourceAsStream", 
         "(Ljava/lang/String;)Ljava/io/InputStream;",
         JEMCC_ClassLoader_getSystemResourceAsStream_String },
    { ACC_PUBLIC,
         "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;",
         JEMCC_ClassLoader_loadClass_String },
    { ACC_PROTECTED | ACC_ABSTRACT,
         "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;",
         NULL },
    { ACC_PROTECTED | ACC_FINAL,
         "resolveClass", "(Ljava/lang/Class;)V",
         JEMCC_ClassLoader_resolveClass_Class },
    { ACC_PROTECTED | ACC_FINAL,
         "setSigners", "(Ljava/lang/Class;[Ljava/lang/Object;)V",
         JEMCC_ClassLoader_setSigners_ClassObjectArray }
};

JEMCC_FieldData JEMCC_ClassLoaderFields[] = {
    { ACC_PRIVATE, NULL, NULL, sizeof(ClassLoaderData) }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC | ACC_ABSTRACT,
                     "java.lang.ClassLoader",
                     NULL ** java/lang/Object **,
                     interfaces, 0,
                     JEMCC_ClassLoaderMethods, 13, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
