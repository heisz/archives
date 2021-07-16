/**
 * JEMCC definitions of the java.lang.Throwable class.
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
#define THROWABLE_GETLOCALMSG_IDX 3
#define THROWABLE_GETMESSAGE_IDX 4
#define THROWABLE_TOSTRING_IDX 8

/**
 * Local definition of the raw information for the Throwable stack trace.
 * The steInstance is not defined until such time that the stack trace
 * set is explicitly requested.
 */
typedef struct JEMCC_StackTraceEntryData {
    jmethodID invokedMethod;
    jint lineNumber;
} JEMCC_StackTraceEntryData;

/**
 * Structural definition of the contents of the StackTraceElement internals
 * (all private variables).
 */
typedef struct StackTraceElementData {
    JEMCC_Object *invokedMethodName;
    JEMCC_Object *invokedClass;
    JEMCC_Object *fileName;
    jint lineNumber;
} StackTraceElementData;

/**
 * Condensed fillInStackTrace method, suitable for use in this class as
 * well as in the ThrowStdThrowable* and ThrowSkeletonThrowable methods.
 * Collapses the currently executing frame instance into the raw data
 * elements for the Throwable stack trace.
 *
 * Note: this method is not synchronized - it should only be called from
 *       construction mechanisms (where synchronization is not an issue)
 *       or from the actual fillInStackTrace() method which is itself
 *       synchronized against the Throwable instance.
 *
 * TODO - filter out constructors of Throwable instance...
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwable - the Throwable instance into which the stack information
 *                 should be populated
 *     fromConstructor - stack initialize call is being made from throwable
 *                       constructor, skip over topFrame in stack trace
 *
 * Returns:
 *     JNI_OK if the stack trace has been successfully created, JNI_ENOMEM
 *     if a memory allocation failure occurred (an OutOfMemoryError will have
 *     been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEM_Throwable_InitStackTrace(JNIEnv *env, JEMCC_Object *throwable,
                                  jboolean fromConstructor) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_ObjectExt *throwObj = (JEMCC_ObjectExt *) throwable;
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(throwObj->objectData);
    JEMCC_StackTraceEntryData *stePtr;
    JEM_VMFrameExt *wrkFrame;
    JEM_BCMethod *bcMethod;
    int i, cnt, lineNum;

    /* First, flush the existing trace if found */
    if (data->stackTrace != NULL) {
        /* TODO - GC release any defined steInstances */
        JEMCC_Free(data->stackTrace);
        data->stackTrace = NULL;
        data->stackTraceDepth = 0;
    }

    /* Count the number of active frame instances */
    cnt = 0;
    wrkFrame = jenv->topFrame;
    if (fromConstructor == JNI_TRUE) wrkFrame = wrkFrame->previousFrame;
    while (wrkFrame != NULL) {
        if ((wrkFrame->opFlags & FRAME_TYPE_MASK) == FRAME_ROOT) break;
        cnt++;
        wrkFrame = wrkFrame->previousFrame;
    }

    /* Allocate stack storage area */
    data->stackTrace = (JEMCC_StackTraceEntryData *)
                   JEMCC_Malloc(env, cnt * sizeof(JEMCC_StackTraceEntryData));
    if (data->stackTrace == NULL) return JNI_ENOMEM;
    data->stackTraceDepth = cnt;

    /* Fill in the stack trace instance */
    stePtr = data->stackTrace;
    wrkFrame = jenv->topFrame;
    if (fromConstructor == JNI_TRUE) wrkFrame = wrkFrame->previousFrame;
    while (wrkFrame != NULL) {
        if ((wrkFrame->opFlags & FRAME_TYPE_MASK) == FRAME_ROOT) break;
        stePtr->invokedMethod = (jmethodID) wrkFrame->currentMethod;
        lineNum = -1;
#ifndef NO_JVM_DEBUG
        if ((wrkFrame->opFlags & FRAME_TYPE_MASK) == FRAME_BYTECODE) {
            bcMethod = wrkFrame->currentMethod->method.bcMethod;
            for (i = 0; i < bcMethod->lineNumberTableLength; i++) {
                if (bcMethod->lineNumberTable[i].startPC < wrkFrame->pc) {
                    lineNum = bcMethod->lineNumberTable[i].lineNumber;
                }
            }
        }
#endif
        stePtr->lineNumber = lineNum;
        wrkFrame = wrkFrame->previousFrame;
        stePtr++;
    }

    return JNI_OK;
}

/**
 * Central method for Throwable toString() action (used in main method as
 * well as in the stackTrace dump methods.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwable - the Throwable instance to obtain the toString for
 *     fromToString - if JNI_TRUE, this call is being made from the toString()
 *                    method (avoid recursive call)
 * Returns:
 *     The toString() result for the throwable or NULL if an error
 *     (most likely OutOfMemory) occurred.
 */
JEMCC_Object *JEM_Throwable_ToString(JNIEnv *env, JEMCC_Object *throwable,
                                     jboolean fromToString) {
    JEMCC_Class *throwClass = VM_CLASS(JEMCC_Class_Throwable);
    JEMCC_Object *msg, *retVal;

    /* Make the Java call, if appropriate */
    JEMCC_CHECK_METHOD_REFERENCE(env, throwClass, THROWABLE_TOSTRING_IDX,
                                 "toString", "()Ljava/lang/String;");
    if ((fromToString != JNI_TRUE) &&
        (JEMCC_IsDefaultMethod(env, throwable, throwClass, 
                               THROWABLE_TOSTRING_IDX) == JNI_FALSE)) {
        /* TODO - make the call to toString() */
        return NULL;
    }

    /* Determine the core message, using defaults as much as possible */
    msg = NULL;
    JEMCC_CHECK_METHOD_REFERENCE(env, throwClass, THROWABLE_GETLOCALMSG_IDX,
                                 "getLocalizedMessage", "()Ljava/lang/String;");
    if (JEMCC_IsDefaultMethod(env, throwable, throwClass,
                              THROWABLE_GETLOCALMSG_IDX) == JNI_TRUE) {
        JEMCC_CHECK_METHOD_REFERENCE(env, throwClass, THROWABLE_GETMESSAGE_IDX,
                                     "getMessage", "()Ljava/lang/String;");
        if (JEMCC_IsDefaultMethod(env, throwable, throwClass,
                                  THROWABLE_GETMESSAGE_IDX) == JNI_TRUE) {
            msg = ((JEMCC_ThrowableData *) 
                      &(((JEMCC_ObjectExt *) throwable)->objectData))->message;
        } else {
            /* TODO - make the call to getMessage() */
        }
    } else {
        /* TODO - make the call to getLocalizedMessage() */
    }

    /* Assemble class:message or just class */
    if (msg == NULL) {
        retVal = JEMCC_GetInternStringUTF(env,
                              throwable->classReference->classData->className);
    } else {
        /* TODO - merge! */
        retVal = msg;
    }

    return retVal;
}

/**
 * Print the stack trace associated with the provided Throwable object.  Used
 * by all of the printStackTrace() methods as well as the JNI exception print
 * related methods (through a handler callback).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwable - the Throwable instance for which the stack trace should
 *                 be printed
 *     handler - the callback handler used to output each line
 *     callData - an opaque data member used by the handler to process
 *                the output lines
 *
 * Exceptions:
 *     Any related to memory allocation and/or output handling (depends on
 *     situation).
 */
void JEM_Throwable_PrintStackTrace(JNIEnv *env, JEMCC_Object *throwable,
                                   jint handler(JNIEnv *env, JEMCC_Object *line,
                                                jboolean includeEOL,
                                                void *callData),
                                   void *callData) {
    JEMCC_ObjectExt *throwObj = (JEMCC_ObjectExt *) throwable;
    JEMCC_ThrowableData *throwData = 
                              (JEMCC_ThrowableData *) &(throwObj->objectData);
    JEM_ClassMethodData *method;
    JEMCC_Object *lineStr;
    char *ptr, buff[32];
    int i;

    /* Dump the String representation of the exception */
    lineStr = JEM_Throwable_ToString(env, throwable, JNI_FALSE);
    if (lineStr == NULL) return;
    if (handler == NULL) {
        JEMCC_DumpString(env, lineStr, JNI_TRUE);
        (void) fprintf(stderr, "\n");
    } else {
        (*handler)(env, lineStr, JNI_TRUE, callData);
    }

    /* Dump the stack contents */
    for (i = 0; i < throwData->stackTraceDepth; i++) {
        method = (JEM_ClassMethodData *) throwData->stackTrace[i].invokedMethod;
        if (method != NULL) {
            /* Standard Throwable, work with native data */
            if (JEMCC_EnvStrBufferInit(env, 256) == NULL) return;
            ptr = JEMCC_EnvStrBufferAppendSet(env, "        at ", 
                                    method->parentClass->classData->className,
                                    ".", method->name, NULL);
            if (ptr == NULL) return;
            if ((method->accessFlags & ACC_JEMCC) != 0) {
                ptr = JEMCC_EnvStrBufferAppendSet(env, "(JEMCC Method)", NULL);
            } else if ((method->accessFlags & ACC_JEMCC) != 0) {
                ptr = JEMCC_EnvStrBufferAppendSet(env, "(Native Method)", NULL);
            } else if (method->parentClass->classData->sourceFile != NULL) {
                if (throwData->stackTrace[i].lineNumber >= 0) {
                    (void) sprintf(buff, "%i", 
                                   throwData->stackTrace[i].lineNumber);
                    ptr = JEMCC_EnvStrBufferAppendSet(env, "(", 
                                    method->parentClass->classData->sourceFile,
                                    ":", buff, ")", NULL);
                } else {
                    ptr = JEMCC_EnvStrBufferAppendSet(env, "(", 
                                    method->parentClass->classData->sourceFile,
                                    ")", NULL);
                }
            } else {
                ptr = JEMCC_EnvStrBufferAppendSet(env, "(Unknown Source)", 
                                                  NULL);
            }
            if (ptr == NULL) return;

            if (handler == NULL) {
                (void) fprintf(stderr, "%s\n", ptr);
            } else {
                lineStr = JEMCC_NewStringUTF(env, ptr);
                if (lineStr == NULL) return;
                (*handler)(env, lineStr, JNI_TRUE, callData);
                /* TODO - GC the temporary object */
            }
        }
    }
}

/* This has been exposed for external exception initializers */
jint JEMCC_Throwable_init(JNIEnv *env, JEMCC_VMFrame *frame,
                          JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);

    /* Absolutely nothing defined */
    data->message = NULL;
    data->stackTrace = NULL;
    data->stackTraceDepth = -1;

    /* Stack trace defined at construction time */
    if (JEM_Throwable_InitStackTrace(env, (JEMCC_Object *) thisObj,
                                     JNI_TRUE) != JNI_OK) {
        return JEMCC_ERR;
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_Throwable_init_String(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);
    JEMCC_Object *msgObj = JEMCC_LOAD_OBJECT(frame, 1);

    /* Have a message to define */
    data->message = msgObj;
    data->stackTrace = NULL;
    data->stackTraceDepth = -1;

    /* Stack trace defined at construction time */
    if (JEM_Throwable_InitStackTrace(env, (JEMCC_Object *) thisObj,
                                     JNI_TRUE) != JNI_OK) {
        return JEMCC_ERR;
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_Throwable_fillInStackTrace(JNIEnv *env,
                                             JEMCC_VMFrame *frame,
                                             JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);

    /* Fill stack trace (will destroy original instance) */
    if (JEM_Throwable_InitStackTrace(env, (JEMCC_Object *) thisObj,
                                     JNI_FALSE) != JNI_OK) {
        return JEMCC_ERR;
    }

    return JEMCC_RET_VOID;
}

static jint JEMCC_Throwable_getLocalizedMessage(JNIEnv *env,
                                                JEMCC_VMFrame *frame,
                                                JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);

    JEMCC_CHECK_METHOD_REFERENCE(env, NULL, THROWABLE_GETMESSAGE_IDX,
                                 "getMessage", "()Ljava/lang/String;");
    if (JEMCC_IsDefaultMethod(env, (JEMCC_Object *) thisObj, NULL,
                              THROWABLE_GETMESSAGE_IDX) == JNI_TRUE) {
        /* Just grab it directly */
        retVal->objVal = data->message;
    } else {
        /* TODO - make the call to getMessage() */
    }

    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Throwable_getMessage(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_ObjectExt *thisObj = (JEMCC_ObjectExt *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_ThrowableData *data = (JEMCC_ThrowableData *) &(thisObj->objectData);

    retVal->objVal = data->message;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Throwable_printStackTrace(JNIEnv *env,
                                            JEMCC_VMFrame *frame,
                                            JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Throwable_printStackTrace_PrintStream(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Throwable_printStackTrace_PrintWriter(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Throwable_toString(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

/**
 * NB: do not mix this array as above methods use direct method indices.
 *     In particular:
 *          3 - getLocalizedMessage()
 *          4 - getMessage()
 *          8 - toString()
 */
JEMCC_MethodData JEMCC_ThrowableMethods[] = {
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_Throwable_init },
    { ACC_PUBLIC,
         "<init>", "(Ljava/lang/String;)V",
         JEMCC_Throwable_init_String },
    { ACC_PUBLIC | ACC_SYNCHRONIZED, /* NEW SYNC */
         "fillInStackTrace", "()Ljava/lang/Throwable;",
         JEMCC_Throwable_fillInStackTrace },
    { ACC_PUBLIC,
         "getLocalizedMessage", "()Ljava/lang/String;",
         JEMCC_Throwable_getLocalizedMessage },
    { ACC_PUBLIC,
         "getMessage", "()Ljava/lang/String;",
         JEMCC_Throwable_getMessage },
    { ACC_PUBLIC,
         "printStackTrace", "()V",
         JEMCC_Throwable_printStackTrace },
    { ACC_PUBLIC,
         "printStackTrace", "(Ljava/io/PrintStream;)V",
         JEMCC_Throwable_printStackTrace_PrintStream },
    { ACC_PUBLIC,
         "printStackTrace", "(Ljava/io/PrintWriter;)V",
         JEMCC_Throwable_printStackTrace_PrintWriter },
    { ACC_PUBLIC,
         "toString", "()Ljava/lang/String;",
         JEMCC_Throwable_toString }
};

JEMCC_FieldData JEMCC_ThrowableFields[] = {
    { ACC_PRIVATE, NULL, NULL, sizeof(JEMCC_ThrowableData) }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC,
                     "java.lang.Throwable",
                     NULL ** java/lang/Object **,
                     interfaces ** java/io/Serializable,  **, 1,
                     JEMCC_ThrowableMethods, 9, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
