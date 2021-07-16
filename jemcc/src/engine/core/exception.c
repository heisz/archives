/**
 * JEMCC methods for exception handling within the VM.
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

/* From cpu.c (exceptions split out) */
extern void JEM_DumpFrame(JNIEnv *env);
extern void JEM_PopFrame(JNIEnv *env);

/* From throwable.c (exception stack definition) */
extern jint JEM_Throwable_InitStackTrace(JNIEnv *env, JEMCC_Object *throwable,
                                         jboolean fromConstructor);

/**
 * Process the specified throwable in the context of the current environment.
 * This will search for an appropriate "catcher" of the exception, be it
 * a bytecode defined exception handler or a calling native/JEMCC method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context and in which
 *           the exception is to be thrown
 *     throwable - the Throwable instance which is to be thrown.  If NULL,
 *                 the "native" exception which is contained in the given
 *                 environment will be rethrown (pass on mechanism).
 *
 * Exceptions:
 *     It is possible that another more critical exception (such as
 *     OutOfMemoryError) may be thrown in place of the specified throwable,
 *     if an error occurs in processing the exception.
 */
void JEMCC_ProcessThrowable(JNIEnv *env, JEMCC_Object *throwable) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_VMFrameExt *currentFrameExt;
    JEMCC_VMFrame *currentFrame;
    JEM_BCMethod *bcMethodPtr;
    JEM_MethodExceptionBlock *exBlockPtr;
    JEMCC_Class **exClassPtr, *compClass, *exClass;
    int i, pc, mode, throwableCaught = JNI_FALSE, firstFrame = JNI_TRUE;

    if (throwable == NULL) {
        throwable = jenv->pendingException;
        jenv->pendingException = NULL;
    }

    while (throwableCaught == JNI_FALSE) {
        currentFrameExt = jenv->topFrame;
        currentFrame = (JEMCC_VMFrame *) currentFrameExt;
        mode = currentFrameExt->opFlags;

        /* When capture frame encountered, store throwable and halt collapse */
        if ((mode & FRAME_THROWABLE_CAPTURE) != 0) {
            jenv->pendingException = throwable;
            throwableCaught = JNI_TRUE;
            break;
        }

        switch (mode & FRAME_TYPE_MASK) {
            case FRAME_ROOT:
            case FRAME_NATIVE:
                jenv->pendingException = throwable;
                throwableCaught = JNI_TRUE;
                break;
            case FRAME_JEMCC:
                if (firstFrame == JNI_TRUE) {
                    /* Initial throw from JEMCC method - pop and continue */
                    /* NOTE: assign exception to this frame so trace is ok */
                    JEM_PopFrame(env);
                } else {
                    /* TODO - store in env */
                    jenv->pendingException = throwable;
                    throwableCaught = JNI_TRUE;
                }
                break;
            case FRAME_BYTECODE:
                /* Scan the method exception table and jump if match found */
                pc = currentFrameExt->lastPC;
                bcMethodPtr = currentFrameExt->currentMethod->method.bcMethod;
                exBlockPtr = bcMethodPtr->exceptionTable;
                for (i = 0; i < bcMethodPtr->exceptionTableLength; 
                                                           i++, exBlockPtr++) {
                    /* Validate that this exception is in the correct block */
                    if ((exBlockPtr->startPC > pc) ||
                                   (exBlockPtr->endPC < pc)) continue;

                    /* Verify the class cast directly (don't use JNI method) */
                    compClass = (JEMCC_Class *) 
                                     exBlockPtr->exceptionClass.instance;
                    exClass = throwable->classReference;

                    /* Same class is easy */
                    if (exClass == compClass) {
                        currentFrameExt->pc = exBlockPtr->handlerPC;
                        JEMCC_PUSH_STACK_OBJECT(currentFrame, throwable);
                        throwableCaught = JNI_TRUE;
                        break;
                    }

                    /* Perhaps it is an assignable parent */
                    exClassPtr = exClass->classData->assignList;
                    while (*exClassPtr != NULL) {
                        if (*exClassPtr == compClass) {
                            currentFrameExt->pc = exBlockPtr->handlerPC;
                            JEMCC_PUSH_STACK_OBJECT(currentFrame, throwable);
                            throwableCaught = JNI_TRUE;
                            break;
                        }
                        exClassPtr++;
                    }
                    if (throwableCaught != JNI_FALSE) break;
                }

                if (throwableCaught == JNI_FALSE) {
                    /* No match - pop this frame and try the parent */
                    /* NOTE: assign exception to this frame so trace is ok */
                    JEM_PopFrame(env);
                }
                break;
            default:
                /* TODO - what the heck? */
                break;
        }

        firstFrame = JNI_FALSE;
    }

    JEM_DumpFrame(env);
}

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * index.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to be created and thrown
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
void JEMCC_ThrowStdThrowableIdx(JNIEnv *env, JEMCC_VMClassIndex idx, 
                                JEMCC_Object *causeThrowable, const char *msg) {
    /* Simply convert the index to the class, if we have the env to do it */
    /* TODO Are the null cases necessary with new split */
    if (env == NULL) {
        JEMCC_ThrowStdThrowable(env, NULL, causeThrowable, msg);
    } else if (((JEM_JNIEnv *) env)->parentVM == NULL) {
        JEMCC_ThrowStdThrowable(env, NULL, causeThrowable, msg);
    } else {
        JEMCC_ThrowStdThrowable(env, VM_CLASS(idx), causeThrowable, msg);
    }
}

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * name.  Performs a simple class lookup for the provided name against the
 * classloader associated with the given object or the class to which the
 * currently executing method belongs.  See the 'jemcc.h' file for more
 * details on what constitutes a "standard" throwable instance in JEMCC
 * (essentially, a Throwable subclass which does nothing more than provide
 * a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     srcObj - if non-NULL, load the throwable class from the classloader
 *              associated with the given object.  If NULL, use the classloader
 *              which is associated with the currently executing method
 *     className - the classname for the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError or ClassNotFoundException) due to difficulties
 *     in loading or creating the throwable instance.
 */
void JEMCC_ThrowStdThrowableByName(JNIEnv *env, JEMCC_Object *srcObj,
                                   const char *className, 
                                   JEMCC_Object *causeThrowable, 
                                   const char *msg) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_Object *loader = NULL;
    JEMCC_Class *throwClass = NULL;

    /* Locate the throw class from the source object classloader */
    if (srcObj != NULL) {
        loader = srcObj->classReference->classData->classLoader;
    } else {
        loader = 
            jenv->topFrame->currentMethod->parentClass->classData->classLoader;
    }
    if (JEMCC_LocateClass(env, loader, className, 
                          JNI_FALSE, &throwClass) != JNI_OK) {
        /* A different exception has already been thrown */
        return;
    }

    /* Throw the requested instance */
    JEMCC_ThrowStdThrowable(env, throwClass, causeThrowable, msg);
}

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * instance.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
void JEMCC_ThrowStdThrowable(JNIEnv *env, JEMCC_Class *throwClass, 
                             JEMCC_Object *causeThrowable, const char *msg) {
    JEM_JavaVM *jvm;
    JEMCC_Object *throwable, *msgString = NULL;
    JEMCC_ThrowableData *exData;

    fprintf(stderr, "[Exception :%s: (%s)]\n", ((msg != NULL) ? msg : "(null)"),
            ((throwClass == NULL) ? "null" : throwClass->classData->className));

    /* Emergency (testcase) checks (note that exceptions are costly anyways) */
    if ((env == NULL) || 
             ((jvm = ((JEM_JNIEnv *) env)->parentVM) == NULL)) return;
    if (throwClass == NULL) throwClass = VM_CLASS(JEMCC_Class_Throwable);
    if (throwClass == NULL) return;

    /* Validate the throwing class */
    if ((throwClass->classData->accessFlags & ACC_THROWABLE) == 0) {
        throwClass = VM_CLASS(JEMCC_Class_InternalError);
    }

    /* Allocate and initialize the throwable instance */
    if (msg != NULL) {
        msgString = (JEMCC_Object *) JEMCC_NewStringUTF(env, msg);
        /* If that failed, exception has already been thrown */
        if (msgString == NULL) return;
    }
    throwable = JEMCC_AllocateObject(env, throwClass, 0);
    if (throwable == NULL) return;
    exData = (JEMCC_ThrowableData *) 
                         &((JEMCC_ObjectExt *) throwable)->objectData;
    exData->causeThrowable = causeThrowable;
    exData->message = msgString;
    if (JEM_Throwable_InitStackTrace(env, throwable, 
                                     JNI_FALSE) != JNI_OK) return;

    /* Process the throwable, collapsing frames as required */
    JEMCC_ProcessThrowable(env, throwable);
}

/**
 * Supermethod for all of the following variable argument message methods.
 */
static void JEMCC_ThrowStdThrowableVA(JNIEnv *env, JEMCC_Class *throwClass,
                                      JEMCC_Object *causeThrowable,
                                      va_list msgSegments) {
    JEM_JavaVM *jvm;
    JEMCC_Object *throwable, *msgString = NULL;
    JEMCC_ThrowableData *exData;
    char *ptr;

    fprintf(stderr, "[Exception :%s: (multi-part msg)]\n",
            ((throwClass == NULL) ? "null" : throwClass->classData->className));

    /* Emergency (testcase) checks (note that exceptions are costly anyways) */
    if ((env == NULL) || 
             ((jvm = ((JEM_JNIEnv *) env)->parentVM) == NULL)) return;
    if (throwClass == NULL) throwClass = VM_CLASS(JEMCC_Class_Throwable);
    if (throwClass == NULL) return;

    /* Validate the throwing class */
    if ((throwClass->classData->accessFlags & ACC_THROWABLE) == 0) {
        throwClass = VM_CLASS(JEMCC_Class_InternalError);
    }

    /* Build the exception message in the environment buffer */
    if (JEMCC_EnvStrBufferInit(env, 16) == NULL) return;
    ptr = va_arg(msgSegments, char *);
    while (ptr != NULL) {
        if (JEMCC_EnvStrBufferAppend(env, ptr) == NULL) return;
        ptr = va_arg(msgSegments, char *);
    }
    msgString = JEMCC_EnvStringBufferToString(env);
    if (msgString == NULL) return;

    /* Allocate and initialize the throwable instance */
    throwable = JEMCC_AllocateObject(env, throwClass, 0);
    if (throwable == NULL) return;
    exData = (JEMCC_ThrowableData *) 
                         &((JEMCC_ObjectExt *) throwable)->objectData;
    exData->causeThrowable = causeThrowable;
    exData->message = msgString;
    if (JEM_Throwable_InitStackTrace(env, throwable, 
                                     JNI_FALSE) != JNI_OK) return;

    /* Process the throwable, collapsing frames as required */
    JEMCC_ProcessThrowable(env, throwable);
}

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * index.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * This method is identical to the above method except that it allows for
 * vararg message construction.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to be created and thrown
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
void JEMCC_ThrowStdThrowableIdxV(JNIEnv *env, JEMCC_VMClassIndex idx, 
                                 JEMCC_Object *causeThrowable, ...) {
    va_list ap;

    /* Initialize the message list */
    va_start(ap, causeThrowable);

    /* Simply convert the index to the class, if we have the env to do it */
    /* TODO Are the null cases necessary with new split */
    if (env == NULL) {
        JEMCC_ThrowStdThrowableVA(env, NULL, causeThrowable, ap);
    } else if (((JEM_JNIEnv *) env)->parentVM == NULL) {
        JEMCC_ThrowStdThrowableVA(env, NULL, causeThrowable, ap);
    } else {
        JEMCC_ThrowStdThrowableVA(env, VM_CLASS(idx), causeThrowable, ap);
    }
    va_end(ap);
}

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * name.  Performs a simple class lookup for the provided name against the
 * classloader associated with the given object or the class to which the
 * currently executing method belongs.  See the 'jemcc.h' file for more
 * details on what constitutes a "standard" throwable instance in JEMCC
 * (essentially, a Throwable subclass which does nothing more than provide
 * a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     srcObj - if non-NULL, load the throwable class from the classloader
 *              associated with the given object.  If NULL, use the classloader
 *              which is associated with the currently executing method
 *     className - the classname for the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError or ClassNotFoundException) due to difficulties
 *     in loading or creating the throwable instance.
 */
void JEMCC_ThrowStdThrowableByNameV(JNIEnv *env, JEMCC_Object *srcObj,
                                    const char *className, 
                                    JEMCC_Object *causeThrowable, ...) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_Object *loader = NULL;
    JEMCC_Class *throwClass = NULL;
    va_list ap;

    /* Initialize the message list */
    va_start(ap, causeThrowable);

    /* Locate the throw class from the source object classloader */
    if (srcObj != NULL) {
        loader = srcObj->classReference->classData->classLoader;
    } else {
        loader = 
            jenv->topFrame->currentMethod->parentClass->classData->classLoader;
    }
    if (JEMCC_LocateClass(env, loader, className, 
                          JNI_FALSE, &throwClass) != JNI_OK) {
        /* A different exception has already been thrown */
        va_end(ap);
        return;
    }

    /* Throw the requested instance */
    JEMCC_ThrowStdThrowableVA(env, throwClass, causeThrowable, ap);
    va_end(ap);
}

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * instance.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * This method is identical to the above method except that it allows for
 * vararg message construction.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
void JEMCC_ThrowStdThrowableV(JNIEnv *env, JEMCC_Class *throwClass, 
                              JEMCC_Object *causeThrowable, ...) {
    va_list ap;

    /* Initialize the message list */
    va_start(ap, causeThrowable);

    /* Just do it */
    JEMCC_ThrowStdThrowableVA(env, throwClass, causeThrowable, ap);
    va_end(ap);
}

/**
 * "Catch" an instance of a core VM throwable class (based on an outstanding
 * exception in the current environment).  Will return and clear the pending
 * exception if an exception exists which is assignable to the indicated core 
 * class instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to be caught
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JEMCC_Object *JEMCC_CatchThrowableIdx(JNIEnv *env, JEMCC_VMClassIndex idx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_Object *caughtException;

    if (jenv->pendingException == NULL) return NULL;
    if (JEMCC_IsInstanceOf(env, jenv->pendingException, 
                           VM_CLASS(idx)) != JNI_FALSE) {
        caughtException = jenv->pendingException;
        jenv->pendingException = NULL;
        return caughtException;
    }
    return NULL;
}

/**
 * "Catch" an instance of a throwable class (based on an outstanding exception
 * in the current environment) for the external class reference established 
 * during linking.  Will return and clear the pending exception if an exception  * exists which is assignable to the indicated reference class instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classIdx - the index of the external throwable class reference to
 *                catch
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JEMCC_Object *JEMCC_CatchThrowableRef(JNIEnv *env, jint classIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData = 
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEMCC_Object *caughtException;

    if (jenv->pendingException == NULL) return NULL;
    if (JEMCC_IsInstanceOf(env, jenv->pendingException, 
                            classData->classRefs[classIdx]) != JNI_FALSE) {
        caughtException = jenv->pendingException;
        jenv->pendingException = NULL;
        return caughtException;
    }
    return NULL;
}

/**
 * "Catch" an instance of a throwable class (based on an outstanding exception
 * in the current environment).  Will return and clear the pending exception if
 * an exception exists which is assignable to the provided class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the throwable to be caught
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JEMCC_Object *JEMCC_CatchThrowable(JNIEnv *env, JEMCC_Class *throwClass) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_Object *caughtException;

    if (jenv->pendingException == NULL) return NULL;
    if (JEMCC_IsInstanceOf(env, jenv->pendingException, 
                                throwClass) != JNI_FALSE) {
        caughtException = jenv->pendingException;
        jenv->pendingException = NULL;
        return caughtException;
    }
    return NULL;
}

/*
 * Manage the capture and rethrow of skeleton throwables.  TBD.
 */
JEMCC_Object *JEM_ExtractSkeletonThrowable(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_Object *throwable;

    /* Pull the throwable instance from the environment pending exception */
    if (jenv->pendingException == NULL) return NULL;
    throwable = (JEMCC_Object *) jenv->pendingException;
    jenv->pendingException = NULL;

    /* Remove current stack trace information (skeletonize) */

    return throwable;
}

void JEM_ThrowSkeletonThrowable(JEMCC_Object *skeleton) {
}
