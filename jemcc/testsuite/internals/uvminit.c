/**
 * Setup methods to create a basic "micro"-VM for advanced test cases.
 * Copyright (C) 1999-2004 J.M. Heisz 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU General Public License,
 * as well as further clarification on your rights to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "jeminc.h"

/* Read the JNI/JEMCC internal details */
#include "jem.h"
#include "jni.h"

/* Exposed references to the base test class instances */
JEMCC_Class *JEMCCTestClass_Object = NULL;
JEMCC_Class *JEMCCTestClass_Serializable = NULL;
JEMCC_Class *JEMCCTestClass_Class = NULL;
JEMCC_Class *JEMCCTestClass_Throwable = NULL;
JEMCC_Class *JEMCCTestClass_Exception = NULL;
JEMCC_Class *JEMCCTestClass_Runnable = NULL;
JEMCC_Class *JEMCCTestClass_String = NULL;
JEMCC_Class *JEMCCTestClass_bytePrimitive = NULL;
JEMCC_Class *JEMCCTestClass_arrayPrimitive = NULL;

/* Stolen access macro from init.c */
#define JVM_Class(idx) (jvm->coreClassTbl[(idx)])

/* NOTE: no methods have bodies - they are for linkage purposes only */

static jint JEMCCTest_Object_init(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    return JEMCC_RET_VOID;
}

static jint JEMCCTest_Object_clone(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    return JEMCC_NULL_EXCEPTION;
}

static jint JEMCCTest_Object_equals_Object(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    retVal->intVal = JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCCTest_Object_finalize(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    return JEMCC_RET_VOID;
}

static jint JEMCCTest_Object_getClass(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    retVal->objVal = NULL;
    return JEMCC_RET_OBJECT;
}

static jint JEMCCTest_Object_hashCode(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    retVal->intVal = 12;
    return JEMCC_RET_INT;
}

static jint JEMCCTest_Object_notify(JNIEnv *env,
                                    JEMCC_VMFrame *frame,
                                    JEMCC_ReturnValue *retVal) {
    return JEMCC_NULL_EXCEPTION;
}

static jint JEMCCTest_Object_notifyAll(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    return JEMCC_NULL_EXCEPTION;
}

static jint JEMCCTest_Object_toString(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    retVal->objVal = (JEMCC_Object *) NULL;
    return JEMCC_RET_OBJECT;
}

static jint JEMCCTest_Object_wait(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    return JEMCC_NULL_EXCEPTION;
}

static jint JEMCCTest_Object_wait_long(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    return JEMCC_NULL_EXCEPTION;
}

static jint JEMCCTest_Object_wait_longInt(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    return JEMCC_NULL_EXCEPTION;
}

static JEMCC_MethodData JEMCCTest_ObjectMethods[] = {
    { ACC_PUBLIC,
          "<init>", "()V", 
          JEMCCTest_Object_init },
    { ACC_PROTECTED,
          "clone", "()Ljava/lang/Object;", 
          JEMCCTest_Object_clone },
    { ACC_PUBLIC,
         "equals", "(Ljava/lang/Object;)Z",
         JEMCCTest_Object_equals_Object },
    { ACC_PROTECTED,
         "finalize", "()V",
         JEMCCTest_Object_finalize },
    { ACC_PUBLIC | ACC_FINAL,
         "getClass", "()Ljava/lang/Class;",
         JEMCCTest_Object_getClass },
    { ACC_PUBLIC,
         "hashCode", "()I",
         JEMCCTest_Object_hashCode },
    { ACC_PUBLIC | ACC_FINAL,
         "notify", "()V",
         JEMCCTest_Object_notify },
    { ACC_PUBLIC | ACC_FINAL,
         "notifyAll", "()V",
         JEMCCTest_Object_notifyAll },
    { ACC_PUBLIC,
         "toString", "()Ljava/lang/String;",
         JEMCCTest_Object_toString },
    { ACC_PUBLIC | ACC_FINAL,
         "wait", "()V",
         JEMCCTest_Object_wait },
    { ACC_PUBLIC | ACC_FINAL,
         "wait", "(J)V",
         JEMCCTest_Object_wait_long },
    { ACC_PUBLIC | ACC_FINAL,
         "wait", "(JI)V",
         JEMCCTest_Object_wait_longInt }
};

static jint JEMCCTest_Class_forName_String(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    retVal->objVal = (JEMCC_Object *) NULL;
    return JEMCC_RET_OBJECT;
}

static jint JEMCCTest_Class_getClassLoader(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    return JEMCC_NULL_EXCEPTION;
}   

static JEMCC_MethodData JEMCCTest_ClassMethods[] = {
    { ACC_PUBLIC,
          "toString", "()Ljava.lang.String;", 
           JEMCCTest_Object_toString },
    { ACC_PUBLIC | ACC_STATIC,
          "forName", "(Ljava.lang.String;)Ljava.lang.Class;",
          JEMCCTest_Class_forName_String },
    { ACC_PUBLIC,
          "getClassLoader", "()Ljava.lang.ClassLoader;",
          JEMCCTest_Class_getClassLoader }
};

static JEMCC_MethodData JEMCCTest_RunnableMethods[] = {
    { ACC_PUBLIC | ACC_ABSTRACT, "run", "()V",
         NULL }
};

static jint JEMCCTest_ArrayPrim_clone(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    retVal->objVal = (JEMCC_Object *) NULL;
    return JEMCC_RET_OBJECT;
}

static JEMCC_MethodData JEMCCTest_ArrayPrimMethods[] = {
    { ACC_PROTECTED,
         "clone", "()Ljava/lang/Object;",
         JEMCCTest_ArrayPrim_clone }
};

static JEMCC_FieldData JEMCCTest_ArrayPrimFields[] = {
    { ACC_PUBLIC | ACC_FINAL, "length", "I", sizeof(void *) }
};

/**
 * Exposed method to create initial instances of base classes.
 *
 * Parameters:
 *     env - the test environment to generate the classes in
 *
 * Returns:
 *     JNI_OK - all base classes were created and initialized successfully
 *     JNI_ERR - a definition error occurred in the base class definitions
 *     JNI_ENOMEM - a memory allocation error occurred
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - the JEMCC information for a base class was invalid
 *     LinkageError - one of the class linkages failed (reference not found)
 */
jint initializeTestCoreClasses(JNIEnv *env) {
    JEMCC_Class *interfaces[5];
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    jint rc;

    /* First, construct the "defective" Object/Class instances */
    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC, 
                              "java.lang.Object",
                              NULL, interfaces, 0,
                              JEMCCTest_ObjectMethods, 12, NULL,
                              NULL, 0, NULL, 0, NULL, &JEMCCTestClass_Object);
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdClass(env, NULL, 
                              ACC_PUBLIC | ACC_INTERFACE,
                              "java.io.Serializable",
                              JEMCCTestClass_Object, interfaces, 0, 
                              NULL, 0, NULL, NULL, 0, 
                              NULL, 0, NULL, &JEMCCTestClass_Serializable);
    if (rc != JNI_OK) return rc;

    interfaces[0] = JEMCCTestClass_Serializable;
    rc = JEMCC_CreateStdClass(env, NULL, 
                              ACC_PUBLIC | ACC_FINAL | ACC_NATIVE_DATA,
                              "java.lang.Class",
                              JEMCCTestClass_Object, interfaces, 1,
                              JEMCCTest_ClassMethods, 3, NULL,
                              NULL, 0, NULL, 0, NULL, &JEMCCTestClass_Class);
    if (rc != JNI_OK) return rc;

    /* Now, fix them */
    JEMCCTestClass_Object->classReference = JEMCCTestClass_Class;
    JVM_Class(JEMCC_Class_Object) = JEMCCTestClass_Object;
    JEMCCTestClass_Serializable->classReference = JEMCCTestClass_Class;
    JVM_Class(JEMCC_Class_Serializable) = JEMCCTestClass_Serializable;
    JEMCCTestClass_Class->classReference = JEMCCTestClass_Class;
    JVM_Class(JEMCC_Class_Class) = JEMCCTestClass_Class;

    /* Special adjustment for additional elements of Class instances */
    JEMCCTestClass_Class->classData->packedFieldSize =
                                 sizeof(JEMCC_Class) - sizeof(JEMCC_Object);

    /* Primitives and arrays */
    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_FINAL | ACC_PRIMITIVE,
                              "byte", NULL, interfaces, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &JEMCCTestClass_bytePrimitive);
    if (rc != JNI_OK) return rc;
    JVM_Class(JEMCC_Primitive_Byte) = JEMCCTestClass_bytePrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, 
                              ACC_PUBLIC | ACC_FINAL | ACC_ARRAY |
                              ACC_PRIMITIVE | ACC_NATIVE_DATA,
                              "array", JEMCCTestClass_Object, interfaces, 0,
                              JEMCCTest_ArrayPrimMethods, 1, NULL,
                              JEMCCTest_ArrayPrimFields, 1,
                              NULL, 0, NULL, &JEMCCTestClass_arrayPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_Class(JEMCC_Primitive_Array) = JEMCCTestClass_arrayPrimitive;

    /* Just a few more classes needed for the test cases */
    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_THROWABLE | ACC_STD_THROW,
                              "java.lang.Throwable",
                              JEMCCTestClass_Object, interfaces, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &JEMCCTestClass_Throwable);
    if (rc != JNI_OK) return rc;
    JVM_Class(JEMCC_Class_Throwable) = JEMCCTestClass_Throwable;

    rc = JEMCC_CreateStdClass(env, NULL, 
                              ACC_PUBLIC,
                              "java.lang.Exception",
                              JEMCCTestClass_Throwable, interfaces, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &JEMCCTestClass_Exception);
    if (rc != JNI_OK) return rc;
    JVM_Class(JEMCC_Class_Exception) = JEMCCTestClass_Exception;

    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_INTERFACE,
                              "java.lang.Runnable",
                              JEMCCTestClass_Object, interfaces, 0,
                              JEMCCTest_RunnableMethods, 1, NULL,
                              NULL, 0, NULL, 0, NULL, 
                              &JEMCCTestClass_Runnable);
    if (rc != JNI_OK) return rc;
    JVM_Class(JEMCC_Class_Runnable) = JEMCCTestClass_Runnable;

    /* Note: this is obviously not a real String class instance */
    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_FINAL | ACC_NATIVE_DATA,
                              "java.lang.String",
                              JEMCCTestClass_Object, interfaces, 0,
                              NULL, 0, NULL,
                              NULL, 0, NULL, 0, NULL, 
                              &JEMCCTestClass_String);
    if (rc != JNI_OK) return rc;
    JVM_Class(JEMCC_Class_String) = JEMCCTestClass_String;

    return JNI_OK;
}

/**
 * Dummy method to support "proper" multi-threading.
 */
static jint attachCurrentThread(JavaVM *vm, JNIEnv **pEnv, void *args) {
    JEM_JNIEnv *envData = (JEM_JNIEnv *) calloc(sizeof(JEM_JNIEnv), 1);

    if (envData == NULL) return JNI_ERR;
    envData->envBuffer = envData->envEndPtr = NULL;
    envData->envBufferLength = 0;
    envData->parentVM = (JEM_JavaVM *) vm;
    envData->firstAllocObjectRecord = envData->lastAllocObjectRecord = NULL;

    envData->frameStackBlockSize = 1024;
    envData->frameStackBlock =
                    calloc((unsigned int) envData->frameStackBlockSize, 1);
    if (envData->frameStackBlock == NULL) return JNI_ERR;
    envData->topFrame = (JEM_VMFrameExt *) envData->frameStackBlock;
    ((JEMCC_VMFrame *) envData->topFrame)->operandStackTop = NULL;
    ((JEMCC_VMFrame *) envData->topFrame)->localVars = NULL;
    envData->topFrame = (JEM_VMFrameExt *) envData->frameStackBlock;
    envData->topFrame->opFlags = FRAME_ROOT;
    envData->topFrame->previousFrame = NULL;
    envData->topFrame->currentMethod = NULL;
    envData->topFrame->firstAllocObjectRecord = NULL;

    envData->freeObjLockQueue = NULL;
    envData->objStateTxfrMonitor = JEMCC_CreateSysMonitor(NULL);
    if (envData->objStateTxfrMonitor == NULL) {
        /* Ensure this passes */
        envData->objStateTxfrMonitor = JEMCC_CreateSysMonitor(NULL);
    }
    envData->objLockMonitor = JEMCC_CreateSysMonitor(NULL);
    if (envData->objLockMonitor == NULL) {
        /* Ensure this passes */
        envData->objLockMonitor = JEMCC_CreateSysMonitor(NULL);
    }

    envData->pendingException = NULL;

    envData->envThread = JEMCC_GetCurrentThread();

    *pEnv = (JNIEnv *) envData;
    return JNI_OK;
}

static struct JNIInvokeInterface invokeIF = {
    NULL, NULL, NULL,
    NULL, attachCurrentThread, NULL
};

/**
 * Construct a basic threading environment for testing.
 *
 * Returns:
 *     NULL if the environment initialization failed, otherwise a sufficiently
 *     initialized JNIEnv instance for testing.
 */
JNIEnv *createTestEnv() {
    JEM_JavaVM *jvm = (JEM_JavaVM *) calloc(sizeof(JEM_JavaVM), 1);
    JEM_JNIEnv *envData = (JEM_JNIEnv *) calloc(sizeof(JEM_JNIEnv), 1);

    if ((jvm == NULL) || (envData == NULL)) return NULL;

    envData->envBuffer = envData->envEndPtr = NULL;
    envData->envBufferLength = 0;
    envData->parentVM = jvm;

    if (JEMCC_HashInitTable((JNIEnv *) envData,
                            &(jvm->jemccClassPackageTable), 32) != JNI_OK) {
        return NULL;
    }
    if (JEMCC_HashInitTable((JNIEnv *) envData,
                            &(jvm->internStringTable), 32) != JNI_OK) {
        return NULL;
    }

    jvm->monitor = JEMCC_CreateSysMonitor(NULL);
    if (jvm->monitor == NULL) return NULL;

    (void) memset(&(jvm->coreClassTbl), 0,
                  JEMCC_VM_CLASS_TBL_SIZE * sizeof(JEMCC_Class *));

    envData->frameStackBlockSize = 1024;
    envData->frameStackBlock = 
                    calloc((unsigned int) envData->frameStackBlockSize, 1);
    if (envData->frameStackBlock == NULL) return NULL;
    envData->topFrame = (JEM_VMFrameExt *) envData->frameStackBlock;
    ((JEMCC_VMFrame *) envData->topFrame)->operandStackTop = NULL;
    ((JEMCC_VMFrame *) envData->topFrame)->localVars = NULL;
    envData->topFrame = (JEM_VMFrameExt *) envData->frameStackBlock;
    envData->topFrame->opFlags = FRAME_ROOT;
    envData->topFrame->previousFrame = NULL;
    envData->topFrame->currentMethod = NULL;

    envData->freeObjLockQueue = NULL;
    envData->objStateTxfrMonitor = JEMCC_CreateSysMonitor(NULL);
    if (envData->objStateTxfrMonitor == NULL) return NULL;
    envData->objLockMonitor = JEMCC_CreateSysMonitor(NULL);
    if (envData->objLockMonitor == NULL) return NULL;

    envData->pendingException = NULL;

    /* WARNING: don't define this in the uvm! It will confuse the thread
     * race detectors for NULL environment instances.
    envData->envThread = JEMCC_GetCurrentThread();
     */

    jvm->classPath.entries = NULL;
    jvm->classPath.entryCount = 0;
    jvm->libPath.entries = NULL;
    jvm->libPath.entryCount = 0;

    jvm->libLoader = JEM_DynaLibLoaderInit();
    jvm->coreVM = &invokeIF;

    return (JNIEnv *) envData;
}

/**
 * Cleanup method to destroy the contents of the class table hash.  Called
 * as a scan callback from the primary virtual machine hash table.  Note that 
 * this involves both destroying class instances as well as JEMCC package
 * load records.
 *
 * Parameters:
 *     env - the test environment being cleaned up
 *     table - the virtual machine classtable being scanned/cleaned up
 *     key - in this instance, the class/package name
 *     obj - the class instance/package record for this entry
 *     userData - ignored
 *
 * Returns:
 *     JNI_OK always (hash scan to continue until completion).
 */
static jint classRemovalScanner(JNIEnv *env, JEMCC_HashTable *table, 
                                void *key, void *obj, void *userData) {
    char *classEntry = (char *) key;
    JEMCC_PkgInitData *pkgData = (JEMCC_PkgInitData *) obj;

    if (*classEntry == '+') {
        /* Initially parsed package-library name */
        JEMCC_Free(key);
        JEMCC_Free(pkgData->handler.pkgLibName);
        JEMCC_Free(pkgData);
    } else if ((*classEntry == '!') || (*classEntry == '?')) {
        (void) fprintf(stderr, "Unexpected class race condition found %s\n",
                               classEntry);
        /* Shouldn't end up here! */
        JEMCC_Free(key);
        JEMCC_Free(pkgData);
    } else if (*classEntry == '-') {
        /* Unfinished initializer, free the key */
        JEMCC_Free(key);
        JEMCC_Free(pkgData);
    } else if (*classEntry == '@') {
        /* Do nothing but free the key */
        JEMCC_Free(key);
        JEMCC_Free(pkgData);
    } else {
        JEM_ClassNameSpaceRemove(env, (JEMCC_Class *) obj);
        JEM_DestroyClassInstance(env, (JEMCC_Class *) obj);
    }

    return JNI_OK;
}

/**
 * Cleanup method to destroy the contents of the intern()'ed String table.
 *
 * Parameters:
 *     env - the test environment being cleaned up
 *     table - the virtual machine String table being scanned/cleaned up
 *     key - ignored (internal String data)
 *     obj - the String instance to be destroyed
 *     userData - ignored
 *
 * Returns:
 *     JNI_OK always (hash scan to continue until completion).
 */
static jint stringRemovalScanner(JNIEnv *env, JEMCC_HashTable *table, 
                                 void *key, void *obj, void *userData) {
    JEMCC_Free(((JEMCC_ObjectExt *) obj)->objectData);
    JEMCC_Free(obj);

    return JNI_OK;
}

/**
 * Perform a full destruction of the test environment and virtual machine
 * instance for purify validation.
 *
 * Parameters:
 *     env - the test environment instance to be destroyed
 */
void destroyTestEnv(JNIEnv *env) {
    JEM_JNIEnv *envData = (JEM_JNIEnv *) env;
    JEM_JavaVM *jvm = envData->parentVM;
    JEM_ObjLockQueueEntry *lockEntry, *nextEntry;
    JEMCC_HashTable *hash;

    /* Destroy the system classloader and associated classes */
    if (jvm->systemClassLoader != NULL) {
        hash = (JEMCC_HashTable *) 
                  &(((JEMCC_ObjectExt *) jvm->systemClassLoader)->objectData);
        JEMCC_HashScan(env, hash, classRemovalScanner, NULL);
        JEMCC_HashDestroyTable(hash);
        /* JEMCC_Free(jvm->systemClassLoader); */
        jvm->systemClassLoader = NULL;
    }

    /* Destroy the intern'd String instances */
    JEMCC_HashScan(env, &(jvm->internStringTable),
                   stringRemovalScanner, NULL);
    JEMCC_HashDestroyTable(&(jvm->internStringTable));

    /* Primitives must be handled specially */
    if (JVM_Class(JEMCC_Primitive_Boolean) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Boolean));
    if (JVM_Class(JEMCC_Primitive_Byte) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Byte));
    if (JVM_Class(JEMCC_Primitive_Char) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Char));
    if (JVM_Class(JEMCC_Primitive_Short) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Short));
    if (JVM_Class(JEMCC_Primitive_Int) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Int));
    if (JVM_Class(JEMCC_Primitive_Float) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Float));
    if (JVM_Class(JEMCC_Primitive_Long) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Long));
    if (JVM_Class(JEMCC_Primitive_Double) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Double));
    if (JVM_Class(JEMCC_Primitive_Void) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Void));
    if (JVM_Class(JEMCC_Primitive_Array) != NULL) 
              JEM_DestroyClassInstance(env, JVM_Class(JEMCC_Primitive_Array));

    /* Destroy the class instances from the hash table */
    JEMCC_HashScan(env, &(jvm->jemccClassPackageTable),
                   classRemovalScanner, NULL);

    /* Destroy the path tables, if they have been created */
    JEM_DestroyPathList(env, &(jvm->classPath));
    JEM_DestroyPathList(env, &(jvm->libPath));

    /* Destroy the vm hash/monitor */
    JEMCC_HashDestroyTable(&(jvm->jemccClassPackageTable));
    JEMCC_DestroySysMonitor(jvm->monitor);

    /* Finally, nuke the env/vm structures */
    JEMCC_DestroySysMonitor(envData->objStateTxfrMonitor);
    JEMCC_DestroySysMonitor(envData->objLockMonitor);
    lockEntry = envData->freeObjLockQueue;
    while (lockEntry != NULL) {
        nextEntry = lockEntry->nextEntry;
        JEMCC_Free(lockEntry);
        lockEntry = nextEntry;
    }

    JEMCC_Free(envData->envBuffer);
    JEMCC_Free(envData->frameStackBlock);
    JEMCC_Free(envData);
    JEMCC_Free(jvm);

    /* Nullify pointers to capture purify tests immediately */
    JEMCCTestClass_Object = NULL;
    JEMCCTestClass_Serializable = NULL;
    JEMCCTestClass_Class = NULL;
    JEMCCTestClass_Throwable = NULL;
    JEMCCTestClass_Exception = NULL;
    JEMCCTestClass_Runnable = NULL;
    JEMCCTestClass_String = NULL;
    JEMCCTestClass_bytePrimitive = NULL;
    JEMCCTestClass_arrayPrimitive = NULL;
}
