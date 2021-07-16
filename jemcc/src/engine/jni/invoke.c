/**
 * JEMCC core library functions to support the JNI invocation interfaces.
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

/* Linked list table to track created VM instances */
static JEM_JavaVM *vmList = NULL;

/* Forward declarations */
static JEM_JNIEnv *JEM_CreateJNIEnv(JEM_JavaVM *vm);
static jint JEM_InitializeJNIEnv(JEM_JNIEnv *env);
static void JEM_DestroyJNIEnv(JEM_JNIEnv *env);

/* NOTE: methods are defined first in order to create the interface table */

/* Destroy the specified Java virtual machine instance */
static jint JEM_DestroyJavaVM(JavaVM *vm) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) vm;

    /* Control multi-thread access to the VM link table */
    JEMCC_EnterGlobalMonitor();

    /* Remove the VM instance from the tracking list */
    if (vmList == (JEM_JavaVM *) vm) {
        vmList = jvm->nextVM;
        if (vmList != NULL) vmList->previousVM = NULL;
    } else {
        (jvm->previousVM)->nextVM = jvm->nextVM;
        if (jvm->nextVM != NULL) {
            (jvm->nextVM)->previousVM = jvm->previousVM;
        }
    }

    /* All finished the linkage destruction, release the monitor */
    JEMCC_ExitGlobalMonitor();

    /* Destroy any environments assigned to the VM */
    while (jvm->envList != NULL) {
        JEM_DestroyJNIEnv(jvm->envList);
    }

    /* No longer need the virtual machine environment monitor */
    JEMCC_DestroySysMonitor(jvm->monitor);

    /* Free the associated memory */
    /* TODO - lots of other stuff too! */

    /* Clean up/destroy the class/library path information */
    JEM_DestroyPathList(NULL, &(jvm->classPath));
    JEM_DestroyPathList(NULL, &(jvm->libPath));

    return JNI_OK;
}

/* Attach the current native thread to the VM - new env created */
static jint JEM_AttachCurrentThread(JavaVM *vm, JNIEnv **pEnv,
                                    void *args) {
    JEM_JNIEnv *jenv;
    int rc;

    /* Control multi-thread access to the VM link table */
    /* TODO JEM_EnterMonitor(vmMonitor); */

    /* Verify that this thread doesn't already have an attached environment */

    /* Create the new environment for this thread */
    jenv = JEM_CreateJNIEnv((JEM_JavaVM *) vm);
    if (jenv == NULL) return JNI_ENOMEM;
    if ((rc = JEM_InitializeJNIEnv(jenv)) != JNI_OK) {
        /* TODO - clean it up! */
        return rc;
    }

    if (pEnv != NULL) *pEnv = (JNIEnv *) jenv;
    return JNI_OK;
}

/* Detach the current native thread from the given virtual machine */
static jint JEM_DetachCurrentThread(JavaVM *vm) {
    /* TODO - do it! */

    return JNI_OK;
}

/* The static invocation interface which defines the external VM */
static struct JNIInvokeInterface localInvokeInterface = {
    NULL, NULL, NULL,
    JEM_DestroyJavaVM, JEM_AttachCurrentThread,
    JEM_DetachCurrentThread
};

/* The static invocation interface which defines the native environment */
static struct JNINativeInterface localNativeInterface = {
    NULL, NULL, NULL, NULL,
    JEMCC_GetVersion, JEMCC_DefineClass, JEMCC_FindClass,
    NULL, NULL, NULL,
    JEMCC_GetSuperclass, JEMCC_IsAssignableFrom,
    NULL,
    JEMCC_Throw, JEMCC_ThrowNew,
    JEMCC_ExceptionOccurred, JEMCC_ExceptionDescribe, JEMCC_ExceptionClear,
    JEMCC_FatalError,
    NULL, NULL,
    JEMCC_NewGlobalRef, JEMCC_DeleteGlobalRef, JEMCC_DeleteLocalRef,
    JEMCC_IsSameObject,
    NULL, NULL,
    JEMCC_AllocObject, JEMCC_NewObject, JEMCC_NewObjectV, JEMCC_NewObjectA,
    JEMCC_GetObjectClass, JEMCC_IsInstanceOf, 
    JEMCC_GetMethodID,
    JEMCC_CallObjectMethod, JEMCC_CallObjectMethodV, JEMCC_CallObjectMethodA,
    JEMCC_CallBooleanMethod, JEMCC_CallBooleanMethodV, JEMCC_CallBooleanMethodA,
    JEMCC_CallByteMethod, JEMCC_CallByteMethodV, JEMCC_CallByteMethodA,
    JEMCC_CallCharMethod, JEMCC_CallCharMethodV, JEMCC_CallCharMethodA,
    JEMCC_CallShortMethod, JEMCC_CallShortMethodV, JEMCC_CallShortMethodA,
    JEMCC_CallIntMethod, JEMCC_CallIntMethodV, JEMCC_CallIntMethodA,
    JEMCC_CallLongMethod, JEMCC_CallLongMethodV, JEMCC_CallLongMethodA,
    JEMCC_CallFloatMethod, JEMCC_CallFloatMethodV, JEMCC_CallFloatMethodA,
    JEMCC_CallDoubleMethod, JEMCC_CallDoubleMethodV, JEMCC_CallDoubleMethodA,
    JEMCC_CallVoidMethod, JEMCC_CallVoidMethodV, JEMCC_CallVoidMethodA,
    JEMCC_CallNonvirtualObjectMethod, JEMCC_CallNonvirtualObjectMethodV, 
    JEMCC_CallNonvirtualObjectMethodA,
    JEMCC_CallNonvirtualBooleanMethod, JEMCC_CallNonvirtualBooleanMethodV, 
    JEMCC_CallNonvirtualBooleanMethodA,
    JEMCC_CallNonvirtualByteMethod, JEMCC_CallNonvirtualByteMethodV, 
    JEMCC_CallNonvirtualByteMethodA,
    JEMCC_CallNonvirtualCharMethod, JEMCC_CallNonvirtualCharMethodV, 
    JEMCC_CallNonvirtualCharMethodA,
    JEMCC_CallNonvirtualShortMethod, JEMCC_CallNonvirtualShortMethodV, 
    JEMCC_CallNonvirtualShortMethodA,
    JEMCC_CallNonvirtualIntMethod, JEMCC_CallNonvirtualIntMethodV, 
    JEMCC_CallNonvirtualIntMethodA,
    JEMCC_CallNonvirtualLongMethod, JEMCC_CallNonvirtualLongMethodV, 
    JEMCC_CallNonvirtualLongMethodA,
    JEMCC_CallNonvirtualFloatMethod, JEMCC_CallNonvirtualFloatMethodV, 
    JEMCC_CallNonvirtualFloatMethodA,
    JEMCC_CallNonvirtualDoubleMethod, JEMCC_CallNonvirtualDoubleMethodV, 
    JEMCC_CallNonvirtualDoubleMethodA,
    JEMCC_CallNonvirtualVoidMethod, JEMCC_CallNonvirtualVoidMethodV, 
    JEMCC_CallNonvirtualVoidMethodA,
    JEMCC_GetFieldID, 
    JEMCC_GetObjectField, JEMCC_GetBooleanField, JEMCC_GetByteField,
    JEMCC_GetCharField, JEMCC_GetShortField, JEMCC_GetIntField,
    JEMCC_GetLongField, JEMCC_GetFloatField, JEMCC_GetDoubleField,
    JEMCC_SetObjectField, JEMCC_SetBooleanField, JEMCC_SetByteField,
    JEMCC_SetCharField, JEMCC_SetShortField, JEMCC_SetIntField,
    JEMCC_SetLongField, JEMCC_SetFloatField, JEMCC_SetDoubleField,
    JEMCC_GetStaticMethodID,
    JEMCC_CallStaticObjectMethod, JEMCC_CallStaticObjectMethodV, 
    JEMCC_CallStaticObjectMethodA,
    JEMCC_CallStaticBooleanMethod, JEMCC_CallStaticBooleanMethodV, 
    JEMCC_CallStaticBooleanMethodA,
    JEMCC_CallStaticByteMethod, JEMCC_CallStaticByteMethodV, 
    JEMCC_CallStaticByteMethodA,
    JEMCC_CallStaticCharMethod, JEMCC_CallStaticCharMethodV, 
    JEMCC_CallStaticCharMethodA,
    JEMCC_CallStaticShortMethod, JEMCC_CallStaticShortMethodV, 
    JEMCC_CallStaticShortMethodA,
    JEMCC_CallStaticIntMethod, JEMCC_CallStaticIntMethodV, 
    JEMCC_CallStaticIntMethodA,
    JEMCC_CallStaticLongMethod, JEMCC_CallStaticLongMethodV, 
    JEMCC_CallStaticLongMethodA,
    JEMCC_CallStaticFloatMethod, JEMCC_CallStaticFloatMethodV, 
    JEMCC_CallStaticFloatMethodA,
    JEMCC_CallStaticDoubleMethod, JEMCC_CallStaticDoubleMethodV, 
    JEMCC_CallStaticDoubleMethodA,
    JEMCC_CallStaticVoidMethod, JEMCC_CallStaticVoidMethodV, 
    JEMCC_CallStaticVoidMethodA,
    JEMCC_GetStaticFieldID, 
    JEMCC_GetStaticObjectField, JEMCC_GetStaticBooleanField, 
    JEMCC_GetStaticByteField, JEMCC_GetStaticCharField, 
    JEMCC_GetStaticShortField, JEMCC_GetStaticIntField,
    JEMCC_GetStaticLongField, JEMCC_GetStaticFloatField, 
    JEMCC_GetStaticDoubleField, JEMCC_SetStaticObjectField, 
    JEMCC_SetStaticBooleanField, JEMCC_SetStaticByteField,
    JEMCC_SetStaticCharField, JEMCC_SetStaticShortField, 
    JEMCC_SetStaticIntField, JEMCC_SetStaticLongField, 
    JEMCC_SetStaticFloatField, JEMCC_SetStaticDoubleField,
    JEMCC_NewString, JEMCC_GetStringLength, 
    JEMCC_GetStringChars, JEMCC_ReleaseStringChars,
    JEMCC_NewStringUTF, JEMCC_GetStringUTFLength,
    JEMCC_GetStringUTFChars, JEMCC_ReleaseStringUTFChars,
    JEMCC_GetArrayLength, JEMCC_NewObjectArray,
    JEMCC_GetObjectArrayElement, JEMCC_SetObjectArrayElement,
    JEMCC_NewBooleanArray, JEMCC_NewByteArray, JEMCC_NewCharArray,
    JEMCC_NewShortArray, JEMCC_NewIntArray, JEMCC_NewLongArray,
    JEMCC_NewFloatArray, JEMCC_NewDoubleArray,
    JEMCC_GetBooleanArrayElements, JEMCC_GetByteArrayElements,
    JEMCC_GetCharArrayElements, JEMCC_GetShortArrayElements,
    JEMCC_GetIntArrayElements, JEMCC_GetLongArrayElements,
    JEMCC_GetFloatArrayElements, JEMCC_GetDoubleArrayElements,
    JEMCC_ReleaseBooleanArrayElements, JEMCC_ReleaseByteArrayElements,
    JEMCC_ReleaseCharArrayElements, JEMCC_ReleaseShortArrayElements,
    JEMCC_ReleaseIntArrayElements, JEMCC_ReleaseLongArrayElements,
    JEMCC_ReleaseFloatArrayElements, JEMCC_ReleaseDoubleArrayElements,
    JEMCC_GetBooleanArrayRegion, JEMCC_GetByteArrayRegion,
    JEMCC_GetCharArrayRegion, JEMCC_GetShortArrayRegion,
    JEMCC_GetIntArrayRegion, JEMCC_GetLongArrayRegion,
    JEMCC_GetFloatArrayRegion, JEMCC_GetDoubleArrayRegion,
    JEMCC_SetBooleanArrayRegion, JEMCC_SetByteArrayRegion,
    JEMCC_SetCharArrayRegion, JEMCC_SetShortArrayRegion,
    JEMCC_SetIntArrayRegion, JEMCC_SetLongArrayRegion,
    JEMCC_SetFloatArrayRegion, JEMCC_SetDoubleArrayRegion,
    JEMCC_RegisterNatives, JEMCC_UnregisterNatives,
    JEMCC_EnterObjMonitor, JEMCC_ExitObjMonitor,
    JEMCC_GetJavaVM,
};

/*
 * Initialize the VM argument array.  Currently handle 1.1 only (1.2 is
 * for future work, once Sun gets its plans straight).
 */
jint JNI_GetDefaultJavaVMInitArgs(void *args) {
    JDK1_1InitArgs *args11 = (JDK1_1InitArgs *) args;

    if (args != NULL) {
        if (args11->version == JNI_VERSION_1_1) {
            args11->properties = NULL;
            args11->checkSource = 0; /* Ignored */
            args11->nativeStackSize = 0; /* Ignored */
            args11->javaStackSize = 0; /* Ignored */
            args11->minHeapSize = 0; /* Ignored */
            args11->maxHeapSize = 0; /* Ignored */
            args11->verifyMode = 0; /* Ignored */
            args11->classpath = "."; 
            args11->vfprintf = &vfprintf;
            args11->exit = &exit;
            args11->abort = &abort;
            args11->enableClassGC = 0; /* Ignored */
            args11->enableVerboseGC = 0;
            args11->disableAsyncGC = 0; /* Ignored */
            args11->libpath = NULL;
        } else {
            return JNI_EVERSION;
        }
    }

    return JNI_OK;
}

/* Create a new instance of a Java virtual machine */
jint JNI_CreateJavaVM(JavaVM **pVm, JNIEnv **pEnv, void *args) {
    JEM_JavaVM *jvm;
    JEM_JNIEnv *jenv;
    JDK1_1InitArgs *jvmArgs11 = (JDK1_1InitArgs *) args;
    jbyte *pkgFileData;
    jsize pkgFileLen;
    jint rc;

    /* Quick argument verification */
    if (args != NULL) {
        if (jvmArgs11->version != JNI_VERSION_1_1) {
            return JNI_EVERSION;
        }
    }

    /* Create the virtual machine record, with the JNI function table ptr */
    jvm = (JEM_JavaVM *) calloc((unsigned int) sizeof(struct JEM_JavaVM), 1);
    if (jvm == NULL) return JNI_ENOMEM;
    jvm->coreVM = &localInvokeInterface;

    /* Create the VM local monitor (for class/link MT management) */
    jvm->monitor = JEMCC_CreateSysMonitor(NULL);
    if (jvm->monitor == NULL) {
        JEMCC_Free(jvm);
        return JNI_ENOMEM;
    }

    /* Create the basic VM environment record */
    jenv = JEM_CreateJNIEnv(jvm);
    if (jenv == NULL) {
        /* TODO - destroy monitor */
        JEMCC_Free(jvm);
        return JNI_ENOMEM;
    }

    /* Initialize the bootstrap class/package hash table */
    if (JEMCC_HashInitTable((JNIEnv *) jenv, 
                            &(jvm->jemccClassPackageTable), 32) != JNI_OK) {
        /* TODO - destroy monitor, environment */
        JEMCC_Free(jvm);
        return JNI_ENOMEM;
    }

    /* Initialize the internal string table */
    if (JEMCC_HashInitTable((JNIEnv *) jenv, 
                            &(jvm->internStringTable), 32) != JNI_OK) {
        /* TODO - destroy monitor, environment */
        JEMCC_Free(jvm);
        return JNI_ENOMEM;
    }

    /* Initialize the VM specific dynamic library loader */
    jvm->libLoader = JEM_DynaLibLoaderInit();
    if (jvm->libLoader == NULL) {
        /* TODO - destroy monitor, environment */
        JEMCC_Free(jvm);
        return JNI_ENOMEM;
    }

    /* Parse the VM initialization arguments */
    /* Split and parse the class/library path information */
    /* TODO - clean up in here! */
    rc = JEM_ParsePathList((JNIEnv *) jenv, &(jvm->classPath), 
                           jvmArgs11->classpath, JNI_TRUE);
    if (rc != JNI_OK) return rc;
    rc = JEM_ParsePathList((JNIEnv *) jenv, &(jvm->libPath),
                           jvmArgs11->libpath, JNI_FALSE);
    if (rc != JNI_OK) return rc;

    /* Initialize the package definition information as well */
    rc = JEM_ReadPathFileContents((JNIEnv *) jenv, &(jvm->libPath),
                                  "jemcclib.txt", &pkgFileData, &pkgFileLen);
    if (rc == JNI_ENOMEM) return rc;
    if (rc == JNI_OK) {
        rc = JEM_ParseLibPackageInfo((JNIEnv *) jenv, (char *) pkgFileData, 
                                     pkgFileLen);
        JEMCC_Free(pkgFileData);
        if (rc != JNI_OK) return rc;
    }

    /* Initialize the core class set for the VM instance */
    if ((rc = JEM_InitializeVMClasses((JNIEnv *) jenv)) != JNI_OK) {
        /* Dump the last exception message */
        /* TODO CLEAN UP */
        return rc;
    }

    /* Do this after the above (lots of unnecessary noise) */
    jvm->verboseDebugFlags = 1 | 2; /* TODO */
    if (jvmArgs11->enableVerboseGC != 0) jvm->verboseDebugFlags |= 2;

    /* Complete the initialization of the environment (needed Thread) */
    if ((rc = JEM_InitializeJNIEnv(jenv)) != JNI_OK) {
        /* TODO CLEAN UP */
        return rc;
    }

    /* Control multi-thread access to the VM link table */
    JEMCC_EnterGlobalMonitor();

    /* Record the VM instance in the tracking list */
    jvm->previousVM = NULL;
    if (vmList == NULL) {
        vmList = jvm;
        jvm->nextVM = NULL;
    } else {
        vmList->previousVM = jvm;
        jvm->nextVM = vmList;
        vmList = jvm;
    }

    /* All finished the linkage, release the monitor */
    JEMCC_ExitGlobalMonitor();

    if (pVm != NULL) *pVm = (JavaVM *) jvm;
    if (pEnv != NULL) *pEnv = (JNIEnv *) jenv;
    return JNI_OK;
}

/*
 * Determine the number of VM's, as well as returning an array of them.
 */
jint JNI_GetCreatedJavaVMs(JavaVM **vmBuff, jsize buffLen, jsize *nVMs) {
    JEM_JavaVM *jvmptr;
    int count = 0;

    /* Control multi-thread access to the VM link table */
    JEMCC_EnterGlobalMonitor();

    /* Walk the linked list, filling up the buffer and updating the count */
    jvmptr = vmList;
    while (jvmptr != NULL) {
        /* Do this instead of breaking so count is correct regardless */
        if ((vmBuff != NULL) && (count < buffLen)) {
            vmBuff[count] = (JavaVM *) jvmptr;
        }
        jvmptr = jvmptr->nextVM;
        count++;
    }
    if (nVMs != NULL) *nVMs = count;

    /* All finished reading, release the monitor */
    JEMCC_ExitGlobalMonitor();

    return JNI_OK;
}

/*
 * Ok, these are not technically invocation methods, but it might as well be.
 */
jint JEMCC_GetJavaVM(JNIEnv *env, JavaVM **vm) {
    *vm = (JavaVM *) ((JEM_JNIEnv *) env)->parentVM;

    return JNI_OK;
}

jint JEMCC_GetVersion(JNIEnv *env) {
    return JNI_VERSION_1_1;
}

/* Simplify the management of the env -> VM -> thread linkages */

/* Creation of env only creates basic env references */
static JEM_JNIEnv *JEM_CreateJNIEnv(JEM_JavaVM *vm) {
    JEM_JNIEnv *jenv;

    /* Create the native interface record */
    jenv = (JEM_JNIEnv *) calloc((unsigned int) sizeof(struct JEM_JNIEnv), 1);
    if (jenv == NULL) return NULL;
    jenv->coreEnv = &localNativeInterface;
    jenv->pendingException = NULL;
    jenv->parentVM = vm;

    /* Buffer is always NULL'd at first */
    jenv->envBuffer = jenv->envEndPtr = NULL;
    jenv->envBufferLength = 0;

    /* Initialize the memory allocation components */
    jenv->firstAllocObjectRecord = jenv->lastAllocObjectRecord = NULL;

    /* Construct the frame buffer and push the root native frame */
    jenv->frameStackBlockSize = 1024;
    jenv->frameStackBlock = calloc((unsigned int) jenv->frameStackBlockSize, 1);
    if (jenv->frameStackBlock == NULL) {
        /* TODO - handle calloc failure */
        return NULL;
    }
    jenv->topFrame = (JEM_VMFrameExt *) jenv->frameStackBlock;
    ((JEMCC_VMFrame *) jenv->topFrame)->operandStackTop = NULL;
    ((JEMCC_VMFrame *) jenv->topFrame)->localVars = NULL;
    jenv->topFrame->opFlags = FRAME_ROOT;
    jenv->topFrame->previousFrame = NULL;
    jenv->topFrame->frameDepth = 1;
    jenv->topFrame->currentMethod = NULL;
    jenv->topFrame->firstAllocObjectRecord = NULL;

    return jenv;
}

/* Initialization fills out the operating essentials of the environment */
static jint JEM_InitializeJNIEnv(JEM_JNIEnv *env) {
    JEM_JavaVM *jvm = env->parentVM;

    /* Attach the current thread to the environment */
    /* TODO - do it!!!! */

    /* Construct the linkages in the parent virtual machine */
    JEMCC_EnterSysMonitor(jvm->monitor);
    env->previousEnv = NULL;
    if (jvm->envList == NULL) {
        jvm->envList = env;
        env->nextEnv = NULL;
    } else {
        (jvm->envList)->previousEnv = env;
        env->nextEnv = jvm->envList;
        jvm->envList = env;
    }
    JEMCC_ExitSysMonitor(jvm->monitor);

    return JNI_OK;
}

/* Centralized environment destruction.  Not as easy as it looks... */
static void JEM_DestroyJNIEnv(JEM_JNIEnv *env) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) env->parentVM;

    /* Remove the env instance from the VM linkage list */
    JEMCC_EnterSysMonitor(jvm->monitor);
    if (jvm->envList == env) {
        jvm->envList = env->nextEnv;
        if (jvm->envList != NULL) {
           (jvm->envList)->previousEnv = NULL;
        }
    } else {
        (env->previousEnv)->nextEnv = env->nextEnv;
        if (env->nextEnv != NULL) {
            (env->nextEnv)->previousEnv = env->previousEnv;
        }
    }
    env->parentVM = NULL;
    JEMCC_ExitSysMonitor(jvm->monitor);

    /* Destroy the buffer if present */
    if (env->envBuffer != NULL) JEMCC_Free(env->envBuffer);

    /* TODO - Nuke the threading info */

    /* TODO - free the pendingException instance? */

    /* TODO - destroy the memory */
}
