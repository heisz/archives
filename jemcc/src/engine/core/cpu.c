/**
 * JEMCC "processor" to manage frames and bytecode interpretation.
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
#include <math.h>

/* Read the VM structure/method definitions */
#include "jem.h"
#include "jnifunc.h"

/* Define the following to dump a LOT of internal CPU debugging details */
#undef DEBUG_CPU_INTERNALS 
/* #define DEBUG_CPU_INTERNALS 1 */

/* Methods for reading opcodes/arguments from the interpreted method block */
static jubyte read_op1(JEM_VMFrameExt *frame) {
    jubyte ret = (jubyte) 
        (frame->currentMethod->method.bcMethod->code[frame->pc++]);
    return ret;
}

static jushort read_op2(JEM_VMFrameExt *frame) {
    jushort ret = ((jushort) 
              (frame->currentMethod->method.bcMethod->code[frame->pc++])) << 8;
    ret |= (jushort) (frame->currentMethod->method.bcMethod->code[frame->pc++]);
    return ret;
}

static juint read_op4(JEM_VMFrameExt *frame) {
    juint ret = ((juint) 
              (frame->currentMethod->method.bcMethod->code[frame->pc++])) << 24;
    ret |= ((juint) 
              (frame->currentMethod->method.bcMethod->code[frame->pc++])) << 16;
    ret |= ((juint) 
              (frame->currentMethod->method.bcMethod->code[frame->pc++])) << 8;
    ret |= (juint) (frame->currentMethod->method.bcMethod->code[frame->pc++]);
    return ret;
}

/* Convenience sizes (the latter is the number of entries for frame header) */
static int frameEntrySize = sizeof(JEM_FrameEntry);
static int frameStructSize = ((int) ((sizeof(JEM_VMFrameExt) + 
                                               sizeof(JEM_FrameEntry) - 1) /
                                            sizeof(JEM_FrameEntry))) *
                              sizeof(JEM_FrameEntry);

/**
 * Debugging method to provide details on the execution state of the 
 * current execution frame.
 *
 * Parameters:
 *     env - the VM environment which is currently in context, from which the
 *           executing frame instance is identified
 */
void JEM_DumpFrame(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_VMFrameExt *wrkFrame;
    JEM_ClassMethodData *method;
    JEM_BCMethod *bcMethod;
    int i, lineNum, frameType;
    char lineBuff[128], *ptr;

    wrkFrame = jenv->topFrame;
    (void) fprintf(stderr, "Current frame (%p, depth %i):\n", wrkFrame,
                           wrkFrame->frameDepth);
    (void) fprintf(stderr, "    Op Stack %p, Local Vars %p\n",
                           wrkFrame->frameVars.operandStackTop,
                           wrkFrame->frameVars.localVars);
    (void) fprintf(stderr, "    Flags: %x Last PC: %i PC: %i\n\n",
                           wrkFrame->opFlags, wrkFrame->lastPC,
                           wrkFrame->pc);

    (void) fprintf(stderr, "Stack dump:\n");
    while (wrkFrame != NULL) {
        frameType = wrkFrame->opFlags & FRAME_TYPE_MASK;
        if (frameType == FRAME_ROOT) {
            (void) fprintf(stderr, "    [Root Frame]\n");
            break;
        }

        lineBuff[0] = '\0';
        ptr = "Unknown";
        method = wrkFrame->currentMethod;
        switch (frameType) {
            case FRAME_NATIVE:
                ptr = "Native";
                break;
            case FRAME_JEMCC:
                ptr = "JEMCC";
                break;
            case FRAME_BYTECODE:
                ptr = "Bytecode";
                bcMethod = method->method.bcMethod;
#ifndef NO_JVM_DEBUG
                lineNum = -1;
                for (i = 0; i < bcMethod->lineNumberTableLength; i++) {
                    if (bcMethod->lineNumberTable[i].startPC < wrkFrame->pc) {
                        lineNum = bcMethod->lineNumberTable[i].lineNumber;
                    }
                }
                if (lineNum < 0) {
                    (void) strcpy(lineBuff, "line unavail");
                } else {
                    (void) sprintf(lineBuff, "line %i", lineNum);
                }
#else
                (void) strcpy(lineBuff, "line unavail");
#endif
                break;
        }
        if (method != NULL) {
            (void) fprintf(stderr, "    %s::%s%s [%s Frame %p pc %i %s]\n", 
                                   method->parentClass->classData->className,
                                   method->name, method->descriptorStr, ptr, 
                                   wrkFrame, wrkFrame->pc, lineBuff);
        } else {
            (void) fprintf(stderr, "    (null) [%s Frame %p pc %i %s]\n", 
                                   ptr, wrkFrame, wrkFrame->pc, lineBuff);
        }
        wrkFrame = wrkFrame->previousFrame;
    }
}

/**
 * Generate a new frame instance on the provided environment instance
 * stack.  Externally exposed for test purposes only - the PushFrame method
 * should be used in other cases.
 *
 * Parameters:
 *     env - the VM environment into which the new frame is to be created
 *     frameType - the type of frame instance to create (bytecode, native or
 *                 jemcc)
 *     methodArgCount - the number of frame entries to consume for this
 *                      method's arguments
 *     localVarCount - the number of frame entries to allocate for local
 *                     method variables
 *     maxOpStackCount - the maximum depth of the operating stack to allocate
 *                       for method use
 *
 * Returns:
 *     The new execution frame instance or NULL if a memory allocation
 *     failed (an exception will be thrown in the current environment).
 *
 *  Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 */
JEMCC_VMFrame *JEM_CreateFrame(JNIEnv *env, int frameType, int methodArgCount,
                               int localVarCount, int maxOpStackCount) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_VMFrameExt *newFrame = NULL;
    int offset, lastFrameEndOffset = 0, newFrameBeginOffset = 0;

    /* Ensure consistency of the localVar/opStack data */
    if ((frameType == FRAME_NATIVE) || (frameType == FRAME_ROOT)) {
        localVarCount = maxOpStackCount = 0;
    } else if (localVarCount < methodArgCount) {
        localVarCount = methodArgCount;
    } 

    /* Ensure sufficient stack space for internal VM operations (classload) */
    if (maxOpStackCount < 0) maxOpStackCount = 0;
    maxOpStackCount += 16;

    /* 
     * Note that the frame support is complicated by the alignment requirements
     * of the platform.  The logic in this method guarantees that the
     * JEM_FrameEntry pointers are aligned in sequence, and that the VMFrame
     * object overlays a specific length of frame entry blocks.
     */
    switch (jenv->topFrame->opFlags & FRAME_TYPE_MASK) {
        case FRAME_ROOT:
        case FRAME_NATIVE:
            offset = (int) (((jbyte *) jenv->topFrame) - 
                                            ((jbyte *) jenv->frameStackBlock));
            lastFrameEndOffset = offset + frameStructSize;

            switch (frameType) {
                case FRAME_ROOT:
                case FRAME_NATIVE:
                    newFrameBeginOffset = lastFrameEndOffset;
                    break;
                case FRAME_JEMCC:
                case FRAME_BYTECODE:
                    newFrameBeginOffset = lastFrameEndOffset +
                                                localVarCount * frameEntrySize;
                    break;
            }
            break;
        case FRAME_JEMCC:
        case FRAME_BYTECODE:
            lastFrameEndOffset = (int) (((jbyte *) 
                                   jenv->topFrame->frameVars.operandStackTop) - 
                                            ((jbyte *) jenv->frameStackBlock));

            switch (frameType) {
                case FRAME_ROOT:
                case FRAME_NATIVE:
                    newFrameBeginOffset = lastFrameEndOffset;
                    break;
                case FRAME_JEMCC:
                case FRAME_BYTECODE:
                    newFrameBeginOffset = lastFrameEndOffset +
                                             (localVarCount - methodArgCount) *
                                                         frameEntrySize;
                    break;
            }
            break;
    }

    /* Ensure sufficient stack space for new frame */
#ifdef ENABLE_MEMSWEEP
/* TODO FIX THIS SO IT ALWAYS MALLOCS! */
    if ((newFrameBeginOffset + frameStructSize + 
               maxOpStackCount * frameEntrySize) > jenv->frameStackBlockSize) {
#else
    if ((newFrameBeginOffset + frameStructSize + 
               maxOpStackCount * frameEntrySize) > jenv->frameStackBlockSize) {
#endif
        /* TODO - reallocate the frame */
        /* TODO - capture out of stack space error */
(void) fprintf(stderr, "NEED TO RESIZE FRAME!!!!!\n");
        return NULL;
    }

    /* Build a new frame record and initialize the localVar/opStack pointers */
    newFrame = (JEM_VMFrameExt *) (((jbyte *) jenv->frameStackBlock) +
                                                           newFrameBeginOffset);
    switch (frameType) {
        case FRAME_ROOT:
        case FRAME_NATIVE:
            newFrame->frameVars.operandStackTop = NULL;
            newFrame->frameVars.localVars = NULL;
            break;
        case FRAME_JEMCC:
        case FRAME_BYTECODE:
            newFrame->frameVars.operandStackTop = 
                (JEM_FrameEntry *) (((jbyte *) newFrame) + frameStructSize);
            newFrame->frameVars.localVars = 
                (JEM_FrameEntry *) (((jbyte *) newFrame) - 
                                        localVarCount * frameEntrySize);
            break;
    }
    newFrame->opFlags = frameType;
    newFrame->previousFrame = jenv->topFrame;
    newFrame->frameDepth = jenv->topFrame->frameDepth + 1;
    newFrame->currentMethod = NULL;
    newFrame->pc = 0;

    /* Push this new frame onto the environment stack */
    jenv->topFrame = newFrame;

    return (JEMCC_VMFrame *) newFrame;
}

/**
 * Create a new frame and push it onto the given environment stack.  This
 * method uses the given method reference explicitly to construct the frame -
 * interface/static/virtual mappings must be handled by the caller (currently
 * handled by the individual opcodes or the JEMCC_Execute* wrappers).  The
 * current frame argument stack must be properly initialized for the given
 * methodID prior to calling this method.
 *
 * Parameters:
 *     env - the VM environment into which the new frame is to be created
 *     methodID - the handle to the method identifier/definition structure
 *                which specifies the method instance to push onto the
 *                environment stack
 *     frameRef - if non-NULL, the created frame instance is returned through
 *                this reference
 *
 * Returns:
 *     JNI_OK - the frame instance was created successfully
 *     JNI_ENOMEM - a memory allocation error has occurred (exception
 *                  has been thrown in the current environment)
 *     JNI_EINVAL - an invalid condtion occurred in the creation of the
 *                  frame (e.g. abstract method instance - exception has been
 *                  thrown in the current environment)
 *
 *  Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     AbstractMethodError - the indicated method is abstract (frame cannot
 *                           be created)
 */
jint JEM_PushFrame(JNIEnv *env, jmethodID methodID, JEMCC_VMFrame **frameRef) {
    JEM_ClassMethodData *method = (JEM_ClassMethodData *) methodID;
    JEM_VMFrameExt *newFrame;

#ifdef DEBUG_CPU_INTERNALS
    fprintf(stderr, ">> Pushing frame for %s%s\n", 
                    method->name, method->descriptorStr);
#endif

    /* Catch this error condition in a common place */
    if ((method->accessFlags & ACC_ABSTRACT) != 0) {
        JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_AbstractMethodError, NULL,
                                    method->parentClass->classData->className,
                                    ".", method->name, NULL);
        return JNI_EINVAL;
    }

    /* Construct the new frame instance, using method arg/var details */
    if ((method->accessFlags & ACC_NATIVE) != 0) {
        newFrame = (JEM_VMFrameExt *) JEM_CreateFrame(env, FRAME_NATIVE, 
                                                      0, 0, 0);
    } else if ((method->accessFlags & ACC_JEMCC) != 0) {
        newFrame = (JEM_VMFrameExt *) JEM_CreateFrame(env, FRAME_JEMCC, 
                                         method->stackConsumeCount, -1, 64);
    } else {
        newFrame = (JEM_VMFrameExt *) JEM_CreateFrame(env, FRAME_BYTECODE, 
                                         method->stackConsumeCount, 
                                         method->method.bcMethod->maxLocals,
                                         method->method.bcMethod->maxStack);
    }
    if (newFrame == NULL) return JNI_ENOMEM;
    newFrame->currentMethod = method;

    /* TODO - Handle the synchronize */

#ifdef DEBUG_CPU_INTERNALS
    JEM_DumpFrame(env);
#endif
    JEM_DumpFrame(env);

    if (frameRef != NULL) *frameRef = (JEMCC_VMFrame *) newFrame;
    return JNI_OK;
}

/**
 * Pop a method frame from the provided environment stack.  This will also
 * clean up any nested JNI local frames which have been allocated (if
 * within a JNI method instance).
 *
 * Parameters:
 *     env - the VM environment from which the frame is to be popped
 */
void JEM_PopFrame(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassMethodData *popMethod = jenv->topFrame->currentMethod;

    /* Avoid potential internal error where root frame ejects */
    if ((jenv->topFrame->opFlags & FRAME_TYPE_MASK) == FRAME_ROOT) {
        (void) fprintf(stderr, "Frame eject avoided!\n    [Root Frame]\n");
        return;
    }

    /* TODO - handle the synchronize cases */
    /* TODO - handle the local frame variable cleanup */
    /* TODO - handle pending exceptions stack storage */

    /* Eject the top frame record from the environment */
    jenv->topFrame = jenv->topFrame->previousFrame;

    /* Absorb stack arguments if frame called from bytecode */
    if (((jenv->topFrame->opFlags & FRAME_TYPE_MASK) == FRAME_BYTECODE) ||
        ((jenv->topFrame->opFlags & FRAME_TYPE_MASK) == FRAME_JEMCC)) {
        ((JEMCC_VMFrame *) jenv->topFrame)->operandStackTop -= 
                                             popMethod->stackConsumeCount;
    }

#ifdef DEBUG_CPU_INTERNALS
    fprintf(stderr, "<< Frame popped for %s%s\n", 
                    popMethod->name, popMethod->descriptorStr);
    JEM_DumpFrame(env);
#endif
}

/**
 * Execute the bytecode method on the top frame of the provided environment.
 *
 * Parameters:
 *     env - the VM environment containing the bytecode frame to interpret
 *
 * Exceptions:
 *     Anything is possible, it is the bytecode interpreter after all.
 */
static void JEM_RunByteCodeInterpreter(JNIEnv *env) {
    JEM_VMFrameExt *currentFrameExt;
    JEMCC_VMFrame *currentFrame;
    juint entryFrameDepth;
    jubyte opCode;

    /* Loop until frame is no longer bytecode or depth return occurs */
    currentFrameExt = ((JEM_JNIEnv *) env)->topFrame;
    entryFrameDepth = currentFrameExt->frameDepth;
    do {
        currentFrame = (JEMCC_VMFrame *) currentFrameExt;
        currentFrameExt->lastPC = currentFrameExt->pc;
        opCode = currentFrameExt->currentMethod->method.
                                        bcMethod->code[currentFrameExt->pc++];
        switch (opCode) {
            default:
                /* TODO - Ack Barf! */
#include "opcodes.c"
                break;
        }
        currentFrameExt = ((JEM_JNIEnv *) env)->topFrame;
    } while (((currentFrameExt->opFlags & FRAME_TYPE_MASK) == FRAME_BYTECODE) &&
             (currentFrameExt->frameDepth >= entryFrameDepth));
}

/**
 * Convenience method to mangle the contents of the provided source
 * string according to the JNI specification.  Returns an allocated
 * copy of the mangled name/signature or NULL if the memory allocation
 * fails.
 */
static char *JEM_MangleJNIName(JNIEnv *env, char *source) {
    char *ptr, *str, *retStr;
    jint ch, len = 0;
    jchar uch;

    /* Determine the length of the returned mangled value (with UTF8 decode) */
    ptr = source;
    while ((ch = *(ptr++)) != '\0') {
        if ((ch & 0x80) != 0) {
            if ((*ptr & 0xC0) == 0x80) ptr++;
            if (((ch & 0x20) != 0) && (*ptr & 0xC0) == 0x80) ptr++;
            len += 4;
        } else if ((ch == '_') || (ch == ';') || (ch == '[')) {
            len += 2;
        } else if (ch == ')') {
            break;
        } else {
            len++;
        }
    }

    /* Allocate the mangled return storage */
    str = retStr = (char *) JEMCC_Malloc(env, len + 1);
    if (retStr == NULL) return NULL;

    /* Mangle away, appropriately handling UTF8-ASCII collapse */
    ptr = source;
    while ((ch = *(ptr++)) != '\0') {
        uch = (jchar) ch;
        if ((ch & 0x80) != 0) {
            if ((ch & 0x20) == 0) {
                uch = (ch & 0x1f) << 6;
                if (((ch = *ptr) & 0xC0) == 0x80) {
                    uch = uch | (ch & 0x3F);
                    ptr++;
                }
            } else {
                uch = (ch & 0x0f) << 12;
                if (((ch = *ptr) & 0xC0) == 0x80) {
                    uch = uch | (ch & 0x3F) << 6;
                    ptr++;
                    if (((ch = *ptr) & 0xC0) == 0x80) {
                        uch = uch | (ch & 0x3F);
                        ptr++;
                    }
                }
            }
        }
        if ((uch & 0xFF00) != 0) {
            (void) sprintf(str, "_0%04X", uch);
            str += 4;
        } else if (uch == '_') {
            *(str++) = '_';
            *(str++) = '1';
        } else if (uch == ';') {
            *(str++) = '_';
            *(str++) = '2';
        } else if (uch == '[') {
            *(str++) = '_';
            *(str++) = '3';
        } else if ((uch == '.') || (uch == '/')) {
            *(str++) = '_';
        } else if (ch == ')') {
            break;
        } else {
            *(str++) = (char) uch;
        }
    }
    *str = '\0';

    return retStr;
}

/* ClassLoader method to track down native method instance */
extern JEM_DynaLibSymbol JEM_ClassLoader_FindNativeMethod(JNIEnv *env,
                                                          JEMCC_Object *loader,
                                                          const char *methName);

/**
 * Attempt to locate and define the JNI native method reference for the
 * current method.  Split out from the main ExecuteCurrentFrame method
 * to simplify the handling of allocation failures.  Returns JNI_OK if the
 * native method was found/defined, JNI_ERR if an exception was thrown
 * or JNI_EINVAL if the method was not found (no exception is thrown).
 */
static jint JEM_LocateNativeMethod(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassMethodData *currentMethod;
    JEM_ClassData *clData;
    char *rawName, *mangledName;

    /* Determine the load context for the method */
    currentMethod = jenv->topFrame->currentMethod;
    clData = currentMethod->parentClass->classData;
    if (clData->classLoader == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_UnsatisfiedLinkError, NULL,
                                   "No native libraries in bootstrap loader");
        return JNI_ERR;
    }

    /* Mangle the class/methodName JNI functional name */
    if (JEMCC_EnvStrBufferInit(env, 256) == NULL) return JNI_ERR;
    rawName = JEMCC_EnvStrBufferAppendSet(env, "Java/", clData->className,
                                          "/", currentMethod->name, NULL);
    if (rawName == NULL) return JNI_ERR;
    mangledName = JEM_MangleJNIName(env, rawName);
    if (mangledName == NULL) return JNI_ERR;

    /* Try to find the method instance based on the short mangle */
    currentMethod->method.ntvMethod = 
                        (void *) JEM_ClassLoader_FindNativeMethod(env,
                                         clData->classLoader, mangledName);
    JEMCC_Free(mangledName);

    if (currentMethod->method.ntvMethod != NULL) return JNI_OK;

    /* Short form didn't match, try with the combined method descriptor */
    rawName = JEMCC_EnvStrBufferAppendSet(env, "//",
                                          currentMethod->descriptorStr + 1,
                                          NULL);
    if (rawName == NULL) return JNI_ERR;
    mangledName = JEM_MangleJNIName(env, rawName);
    if (mangledName == NULL) return JNI_ERR;

    /* If at first you don't succeed, look again */
    currentMethod->method.ntvMethod = 
                        (void *) JEM_ClassLoader_FindNativeMethod(env,
                                         clData->classLoader, mangledName);
    JEMCC_Free(mangledName);

    return JNI_EINVAL;
}

/**
 * "Execute" the method associated with the current frame instance.  This
 * may launch the bytecode method interpreter, a JEMCC method function or
 * a JNI mapped method instance.
 *
 * Parameters: 
 *     env - the VM environment containing the frame to execute
 *     fromByteCode - if JNI_TRUE, this method is being called from within
 *                    a bytecode implementation and this method may return
 *                    to allow the currently operating bytecode interpreter
 *                    to "take over" a new bytecode execution frame.
 */
void JEM_ExecuteCurrentFrame(JNIEnv *env, jboolean fromByteCode) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_ReturnValue retVal;
    JEMCC_VMFrame *lastFrame;
    JEM_DescriptorData *retDesc;
    JEM_ClassMethodData *currentMethod;
    int rc;

    lastFrame = (JEMCC_VMFrame *) jenv->topFrame;
    switch (jenv->topFrame->opFlags & FRAME_TYPE_MASK) {
        case FRAME_ROOT:
            abort();
        case FRAME_BYTECODE:
            /* Just launch the interpreter */
            if (fromByteCode == 0) JEM_RunByteCodeInterpreter(env);
            break;
        case FRAME_NATIVE:
            currentMethod = jenv->topFrame->currentMethod;
            if (currentMethod->method.ntvMethod == NULL) {
                rc = JEM_LocateNativeMethod(env);
                if (rc == JNI_ERR) {
                    /* Native frame has captured - pop and rethrow */
                    JEM_PopFrame(env);
                    JEMCC_ProcessThrowable(env, NULL);
                    break;
                }
                if (rc == JNI_EINVAL) {
                    /* Not found, let someone know */
                    JEM_PopFrame(env);
                    JEMCC_ThrowStdThrowableIdxV(env, 
                              JEMCC_Class_UnsatisfiedLinkError, NULL,
                              "Native method not found for ",
                              currentMethod->parentClass->classData->className,
                              ".", currentMethod->name, NULL );
                    break;
                }
            }

            /* TODO - need to build argument list */

            /* Make the call */
            JEM_CallForeignFunction(env, NULL, 
                                    currentMethod->method.ntvMethod,
                                    currentMethod->descriptor,
                                    NULL, &retVal);

            /* Remove this frame and process any pending native exceptions */
            JEM_PopFrame(env);
            if (jenv->pendingException != NULL) {
                /* This usage rethrows the pendingException */
                JEMCC_ProcessThrowable(env, NULL);
                break;
            }

            /* Handle value based on method return and current frame type */
            retDesc = currentMethod->descriptor->method_info.returnDescriptor;
            if (retDesc != NULL) {
                if ((jenv->topFrame->opFlags & FRAME_TYPE_MASK) == 
                                                             FRAME_BYTECODE) {
                    lastFrame = (JEMCC_VMFrame *) jenv->topFrame;
                    switch (retDesc->generic.tag) {
                        case BASETYPE_Byte:
                        case BASETYPE_Char:
                        case BASETYPE_Int:
                        case BASETYPE_Short:
                        case BASETYPE_Boolean:
                            JEMCC_PUSH_STACK_INT(lastFrame, retVal.intVal);
                            break;
                        case BASETYPE_Long:
                            JEMCC_PUSH_STACK_LONG(lastFrame, retVal.longVal);
                            break;
                        case BASETYPE_Float:
                            JEMCC_PUSH_STACK_FLOAT(lastFrame, retVal.fltVal);
                            break;
                        case BASETYPE_Double:
                            JEMCC_PUSH_STACK_DOUBLE(lastFrame, retVal.dblVal);
                            break;
                        case DESCRIPTOR_ObjectType:
                        case DESCRIPTOR_ArrayType:
                            JEMCC_PUSH_STACK_OBJECT(lastFrame, retVal.objVal);
                            break;
                    }
                } else {
                /* ROOT/NATIVE/JEMCC - store in environment carrier */
                    jenv->nativeReturnValue = retVal;
                }
            }
            break;
        case FRAME_JEMCC:
            /* Call the JEMCC method implementation */
            currentMethod = jenv->topFrame->currentMethod;
            rc = (*(currentMethod->method.ccMethod))
                                       (env, (JEMCC_VMFrame *) jenv->topFrame,
                                        &retVal);

            /* Handle null pointer failures */
            if (rc == JEMCC_NULL_EXCEPTION) {
                JEMCC_ThrowStdThrowableIdx(env, 
                                           JEMCC_Class_NullPointerException, 
                                           NULL, NULL);
                break;
            }

            /* Handle exception instances */
            /* Only rethrow if currently executing frame is unchanged */
            if (rc == JEMCC_ERR) {
                if (lastFrame == (JEMCC_VMFrame *) jenv->topFrame) {
                    if (jenv->pendingException == NULL) {
                        JEMCC_ThrowStdThrowableIdxV(env, 
                                        JEMCC_Class_InternalError, NULL,
                                        "JEMCC_ERR return, no exception from ",
                                        currentMethod->name, NULL);
                    } else {
                        /* This usage rethrows the pendingException */
                        JEMCC_ProcessThrowable(env, NULL);
                    }
                }
                break;
            }

            /* Destroy the working frame (if no exception was thrown) */
            JEM_PopFrame(env);

            /* Handle value based on method return and current frame type */
            if ((jenv->topFrame->opFlags & FRAME_TYPE_MASK) == FRAME_BYTECODE) {
                lastFrame = (JEMCC_VMFrame *) jenv->topFrame;
                switch (rc) {
                    case JEMCC_RET_INT:
                        JEMCC_PUSH_STACK_INT(lastFrame, retVal.intVal);
                        break;
                    case JEMCC_RET_LONG:
                        JEMCC_PUSH_STACK_LONG(lastFrame, retVal.longVal);
                        break;
                    case JEMCC_RET_FLOAT:
                        JEMCC_PUSH_STACK_FLOAT(lastFrame, retVal.fltVal);
                        break;
                    case JEMCC_RET_DOUBLE:
                        JEMCC_PUSH_STACK_DOUBLE(lastFrame, retVal.dblVal);
                        break;
                    case JEMCC_RET_OBJECT:
                        JEMCC_PUSH_STACK_OBJECT(lastFrame, retVal.objVal);
                        break;
                    case JEMCC_RET_VOID:
                        /* Do nothing */ 
                        break;
                    default: 
                        /* Invalid return code - attempt to notify */
                        JEMCC_ThrowStdThrowableIdx(env, 
                                            JEMCC_Class_VirtualMachineError,
                                            NULL, "Invalid JEMCC return code");
                        break;
                }
            } else {
                /* ROOT/NATIVE/JEMCC - store in environment carrier */
                switch (rc) {
                    case JEMCC_RET_INT:
                    case JEMCC_RET_LONG:
                    case JEMCC_RET_FLOAT:
                    case JEMCC_RET_DOUBLE:
                    case JEMCC_RET_OBJECT:
                        jenv->nativeReturnValue = retVal;
                        break;
                    case JEMCC_RET_VOID:
                        /* Do nothing */ 
                        break;
                    default: 
                        /* Invalid return code - attempt to notify */
                        JEMCC_ThrowStdThrowableIdx(env, 
                                            JEMCC_Class_VirtualMachineError,
                                            NULL, "Invalid JEMCC return code");
                        break;
                }
            }
            break;
        default:
            abort();
    }
}

/******************************************************************/

/**
 * Seek out the "previous" object which called the currently executing
 * method (associated with the current environment frame).  This previous
 * object must be different than the object instance currently associated
 * with the executing method (i.e. will skip over any method calls made with
 * 'this').  If the currently executing method is static, then this method
 * will return the most "recent" frame which has an object instance associated
 * with it.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The previous object instance which was responsible for making the
 *     call to this object instance method, or NULL if no such object
 *     could be found.
 */
JEMCC_Object *JEMCC_GetCallingObjectInstance(JNIEnv *env) {
    /* TODO - is this needed? */
    return NULL;
}

/**
 * Seek out the previous class instance which called the currently executing
 * method (associated with the current environment frame).  This previous
 * class must be different than the class associated with the currently
 * executing method (i.e. will skip over method calls made with 'this' or
 * a static class of the current class).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The previous class instance which was responsible for making the
 *     call to the currently executing method or NULL if no such class
 *     could be found (root of the excuting frame).
 */
JEMCC_Class *JEMCC_GetCallingClassInstance(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_VMFrameExt *wrkFrame;
    JEMCC_Class *executingClass;

    wrkFrame = jenv->topFrame;
    if ((wrkFrame == NULL) ||
        ((wrkFrame->opFlags & FRAME_TYPE_MASK) == FRAME_ROOT)) {
        /* No chance of finding a lower class from the stack bottom */
        return NULL;
    }
    executingClass = wrkFrame->currentMethod->parentClass;

    while (wrkFrame != NULL) {
        if ((wrkFrame->opFlags & FRAME_TYPE_MASK) == FRAME_ROOT) {
            /* Reached stack bottom without finding another class */
            return NULL;
        }

        if (wrkFrame->currentMethod->parentClass != executingClass) {
            /* Found the first non-match, return it */
            return wrkFrame->currentMethod->parentClass;
        }

        wrkFrame = wrkFrame->previousFrame;
    }
    return NULL;
}

/**
 * Convenience method to obtain the class associated with the currently
 * executing stack frame.  Essentially allows a JEMCC method to obtain
 * a handle on the class for which it is defined from within the method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The class instance associated with the current frame (the class
 *     of the 'this' object for an instance method or the parent class
 *     of a static method).
 */
JEMCC_Class *JEMCC_GetCurrentClass(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    return jenv->topFrame->currentMethod->parentClass;
}

/**
 * Method to obtain a linked class reference, as defined through the
 * LinkClass() method.  In this case, the index refers to linked class
 * references only (the GetCurrentClass method will retrieve the local
 * class instance).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classIdx - the index of the linked class reference to retrieve
 *
 * Returns:
 *     The requested class reference associated with the provided index.
 *     Note that this will always succeed, as a JEMCC class definition should
 *     fail if any external reference links are invalid.
 */
JEMCC_Class *JEMCC_GetLinkedClassReference(JNIEnv *env, jint classIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData = 
                        jenv->topFrame->currentMethod->parentClass->classData;

    return classData->classRefs[classIdx];
}

/**
 * Method to obtain a field reference, from the local method list of the
 * indicated class (NULL indicates current class local fields).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     targClass - the target class to retrieve the field from - if NULL, use
 *                 the class associated with the currently executing method
 *     fieldIdx - the index of the local field reference to retrieve
 *
 * Returns:
 *     The requested local field reference associated with the provided
 *     class.  Note that this will always succeed, even if the index is
 *     invalid (no verification is performed by this method, use the CHECK
 *     macros to test during development).
 */
jfieldID JEMCC_GetFieldReference(JNIEnv *env, JEMCC_Class *targClass, 
                                 jint fieldIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;

    if (targClass == NULL) {
        targClass = jenv->topFrame->currentMethod->parentClass;
    }
    return (jfieldID) &(targClass->classData->localFields[fieldIdx]);
}

/**
 * Method to obtain the data pointer for a field, from the local method list
 * of the indicated class (NULL indicates current class local fields).
 *
 * NOTE: this method, while more efficient than using the fieldId and the JNI
 * methods, is inherently more dangerous due to the direct manipulation of the
 * object data area.  Pay careful attention to proper casting of the returned
 * pointer.  Also note that no type checking is performed on the validity
 * of the field reference and the class of the given target object.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     targObj - the target object to retrieve the data pointer from
 *               (only applies to non-static fields)
 *     targClass - the target class to retrieve the field offset from - if
 *                 NULL, use the class associated with the currently
 *                 executing method
 *     fieldIdx - the index of the field reference to retrieve
 *
 * Returns:
 *     The pointer to the field data slot corresponding to the given index or
 *     NULL if the field is inaccessible (null target object or static/instance
 *     conflict - an exception has been thrown in the current environment).
 *
 * Exceptions:
 *     NullPointerException - the given object instance was null (and the field
 *                            is not static)
 *     IncompatibleClassChangeError - an object instance was provided for a
 *                                    static field reference
 */
void *JEMCC_GetFieldDataPtr(JNIEnv *env, JEMCC_Object *targObj,
                            JEMCC_Class *targClass, jint fieldIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassFieldData *fieldData;
    void *retVal;

    /* Grab the field definition */
    if (targClass == NULL) {
        targClass = jenv->topFrame->currentMethod->parentClass;
    }
    fieldData = &(targClass->classData->localFields[fieldIdx]);

    /* Handle static/instance appropriately */
    if ((fieldData->accessFlags & ACC_STATIC) != 0) {
        if (targObj != NULL) {
            JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IncompatibleClassChangeError,
                                   NULL, "instance provided for static field");
            return NULL;
        }
        retVal = ((jbyte *) targClass->staticData) + fieldData->fieldOffset;
    } else {
        if (targObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                       NULL, NULL);
            return NULL;
        }
        retVal = &(((JEMCC_ObjectExt *) targObj)->objectData) + 
                                                       fieldData->fieldOffset;
    }

    return retVal;
}

/**
 * JEMCC verification method for index cross-referencing checks.
 */
void JEMCC_CheckFieldReference(JNIEnv *env, JEMCC_Class *targClass, 
                               jint fieldIdx, const char *fieldName, 
                               const char *fieldDesc) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassFieldData *fieldData;

    /* Grab the field definition */
    if (targClass == NULL) {
        targClass = jenv->topFrame->currentMethod->parentClass;
    }
    fieldData = &(targClass->classData->localFields[fieldIdx]);

    /* Check the field details */
    if ((strcmp(fieldName, fieldData->name) != 0) ||
        (strcmp(fieldDesc, fieldData->descriptorStr) != 0)) {
        (void) fprintf(stderr, 
                       "Incorrect field reference - not :%s:%s: but :%s:%s:\n",
                       fieldName, fieldDesc,
                       fieldData->name, fieldData->descriptorStr);
        JEM_DumpFrame(env);
        abort();
    }
}

/**
 * Method to obtain a linked field reference which was defined in the
 * current class through the LinkClass() method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fieldIdx - the index of the linked field reference to retrieve
 *
 * Returns:
 *     The requested external field reference associated with the provided
 *     index.  Note that this will always succeed, even if the index is
 *     invalid (no verification is performed by this method, use the CHECK
 *     macros to test during development).
 */
jfieldID JEMCC_GetLinkedFieldReference(JNIEnv *env, jint fieldIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData =
                      jenv->topFrame->currentMethod->parentClass->classData;

    /* Return the field link from the current class */
    return (jfieldID) classData->classFieldRefs[fieldIdx];
}

/**
 * Method to obtain the data pointer for a linked field which was defined in
 * the current class through the LinkClass() method.
 *
 * NOTE: this method, while more efficient than using the fieldId and the JNI
 * methods, is inherently more dangerous due to the direct manipulation of the
 * object data area.  Pay careful attention to proper casting of the returned
 * pointer.  Also note that no type checking is performed on the validity
 * of the field reference and the class of the given target object.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     targObj - the target object to retrieve the data pointer from
 *               (only applies to non-static fields)
 *     fieldIdx - the index of the linked field reference to retrieve
 *
 * Returns:
 *     The pointer to the field data slot corresponding to the given index or
 *     NULL if the field is inaccessible (null target object or static/instance
 *     conflict - an exception has been thrown in the current environment).
 *
 * Exceptions:
 *     NullPointerException - the given object instance was null (and the field
 *                            is not static)
 *     IncompatibleClassChangeError - an object instance was provided for a
 *                                    static field reference
 */
void *JEMCC_GetLinkedFieldDataPtr(JNIEnv *env, JEMCC_Object *targObj,
                                  jint fieldIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData =
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEM_ClassFieldData *fieldData;
    void *retVal;

    /* Pull the linked field reference */
    fieldData = classData->classFieldRefs[fieldIdx];

    /* Handle static/instance appropriately */
    if ((fieldData->accessFlags & ACC_STATIC) != 0) {
        if (targObj != NULL) {
            JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IncompatibleClassChangeError,
                                   NULL, "instance provided for static field");
            return NULL;
        }
        retVal = ((jbyte *) fieldData->parentClass->staticData) + 
                                                       fieldData->fieldOffset;
    } else {
        if (targObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                       NULL, NULL);
            return NULL;
        }
        retVal = &(((JEMCC_ObjectExt *) targObj)->objectData) + 
                                                       fieldData->fieldOffset;
    }

    return retVal;
}

/**
 * JEMCC verification method for index cross-referencing checks.
 */
void JEMCC_CheckLinkedFieldReference(JNIEnv *env, jint fieldIdx, 
                                     const char *className,
                                     const char *fieldName, 
                                     const char *fieldDesc) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData =
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEM_ClassFieldData *fieldData;

    /* Check the targeted class instance */
    fieldData = classData->classFieldRefs[fieldIdx];
    if (strcmp(className, fieldData->parentClass->classData->className) != 0) {
        (void) fprintf(stderr, "Incorrect field class - not :%s: but :%s:\n",
                               className, classData->className);
        JEM_DumpFrame(env);
        abort();
    }

    /* Check the field details */
    if ((strcmp(fieldName, fieldData->name) != 0) ||
        (strcmp(fieldDesc, fieldData->descriptorStr) != 0)) {
        (void) fprintf(stderr, 
                       "Incorrect field reference - not :%s:%s: but :%s:%s:\n",
                       fieldName, fieldDesc,
                       fieldData->name, fieldData->descriptorStr);
        JEM_DumpFrame(env);
        abort();
    }
}

/**
 * Obtain the active execution frame for the environment currently
 * in context.  Used for general methods (e.g. classloading) where
 * source methods do not pass execution frame information.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The VM frame currently being executed.
 */
JEMCC_VMFrame *JEMCC_GetCurrentVMFrame(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    return (JEMCC_VMFrame *) jenv->topFrame;
}

/**
 * Method used to create and execute a new frame for an interface method
 * reference call.  Automatically extracts the appropriate method reference
 * by index, from the local class method table of the indicated interface.
 * NOTE: the argument stack must  be fully "constructed" (including object
 * instance) prior to calling this method and no limit or interface type
 * checking is performed (null object will be caught).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     targIf - the target interface to obtain the method from
 *     methodIdx - the index of the local interface method to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, bad interface) (exception
 *                  has been thrown in the current environment, will not
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     IncompatibleClassChangeError - the interface was unmappable to the
 *                                    provided object instance
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteInterfaceMethod(JNIEnv *env, JEMCC_Object *obj,
                                  JEMCC_Class *targIf, jint methodIdx,
                                  JEMCC_ReturnValue *retVal) {
    JEM_ClassData *targetClassData;
    JEM_ClassMethodData *ifMethodData, *targetMethodData;
    JEMCC_Class **assignClassPtr;
    jint i, rc;

    /* Central test point simplifies external code */
    if (obj == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
        return JNI_EINVAL;
    }

    /* Obtain the appropriate method reference details */
    ifMethodData = &(targIf->classData->localMethods[methodIdx]);

    /* Interface calls need lookup of method cross reference in target object */
    targetClassData = obj->classReference->classData;
    assignClassPtr = targetClassData->assignList + 1;
    for (i = 1; i < targetClassData->assignmentCount; i++) {
        if (*assignClassPtr == ifMethodData->parentClass) break;
        assignClassPtr++;
    }
    if (i > targetClassData->assignmentCount) {
        JEMCC_ThrowStdThrowableIdx(env, 
                              JEMCC_Class_IncompatibleClassChangeError,
                              NULL, "interface method call on invalid object");
        return JNI_EINVAL;
    }
    methodIdx = targetClassData->methodLinkTables[i]
                                     [ifMethodData->methodIndex]->methodIndex;
    targetMethodData = targetClassData->methodLinkTables[0][methodIdx];
    if (targetMethodData == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_AbstractMethodError,
                                   NULL, ifMethodData->name);
        return JNI_EINVAL;
    }

    /* Construct the execution frame */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) {
        return JNI_ERR;
    } else {
        if (retVal != NULL) *retVal = ((JEM_JNIEnv *) env)->nativeReturnValue;
    }
 
    return JNI_OK;
}

/**
 * Method used to create and execute a new frame for an instance method
 * reference call.  This may be a virtual method call which handles overrides
 * of the referenced class method or a non-virtual call which exactly executes
 * the referenced method.   Automatically extracts the appropriate method
 * reference by index, from the local method table of the indicated class
 * (NULL indicates current class instance). NOTE: the argument stack must be
 * fully "constructed" (including object instance) prior to calling this method
 * and no setup checking is performed (null object will be caught).
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     targClass - the target class to obtain the method from - if NULL, use
 *                 the class associated with the currently executing method
 *     methodIdx - the index of the local class method to use
 *     isVirtual - if JEMCC_VIRTUAL_METHOD (JNI_TRUE), locate the virtual
 *                 class method (accounts for overridden methods in the class
 *                 of the object instance), otherwise, prepare to execute
 *                 the exact method referenced
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, abstract method) (exception
 *                  has been thrown in the current environment, will not
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteInstanceMethod(JNIEnv *env, JEMCC_Object *obj,
                                 JEMCC_Class *targClass, jint methodIdx,
                                 jboolean isVirtual, 
                                 JEMCC_ReturnValue *retVal) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassMethodData *targetMethodData;
    JEM_ClassData *targetClassData;
    jint rc;

    /* Central test point simplifies external code */
    if (obj == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
        return JNI_EINVAL;
    }

    /* Obtain the appropriate method reference details */
    if (targClass == NULL) {
        targClass = jenv->topFrame->currentMethod->parentClass;
    }
    targetMethodData = &(targClass->classData->localMethods[methodIdx]);

    if (isVirtual != JNI_FALSE) {
        /* Virtual calls need the method of the object class by cross index */
        methodIdx = targetMethodData->methodIndex;
        targetClassData = obj->classReference->classData;
        targetMethodData = targetClassData->methodLinkTables[0][methodIdx];
    }

    /* Construct the execution frame */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) {
        return JNI_ERR;
    } else {
        if (retVal != NULL) *retVal = ((JEM_JNIEnv *) env)->nativeReturnValue;
    }

    return JNI_OK;
}

/**
 * Method used to create and execute a new frame for a static method
 * reference call.   Automatically extracts the appropriate method reference
 * by index, from the local method table of the indicated class (NULL
 * indicates current class instance).  NOTE: the argument stack must be
 * fully "constructed" (no this!) prior to calling this method and no setup
 * checking is performed.
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     targClass - the target class to obtain the method from - if NULL, use
 *                 the class associated with the currently executing method
 *     methodIdx - the index of the local class method to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteStaticMethod(JNIEnv *env, JEMCC_Class *targClass,
                               jint methodIdx, JEMCC_ReturnValue *retVal) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassMethodData *targetMethodData;
    jint rc;

    /* Obtain the appropriate method reference details */
    if (targClass == NULL) {
        targClass = jenv->topFrame->currentMethod->parentClass;
    }
    targetMethodData = &(targClass->classData->localMethods[methodIdx]);

    /* Static methods are simple - just push the reference directly */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) {
        return JNI_ERR;
    } else {
        if (retVal != NULL) *retVal = ((JEM_JNIEnv *) env)->nativeReturnValue;
    }

    return JNI_OK;
}

/**
 * JEMCC verification method for index cross-referencing checks.
 */
void JEMCC_CheckMethodReference(JNIEnv *env, JEMCC_Class *targClass,
                                jint methodIdx, const char *methodName,
                                const char *methodDesc) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassMethodData *methodData;

    /* Obtain the appropriate method reference details */
    if (targClass == NULL) {
        targClass = jenv->topFrame->currentMethod->parentClass;
    }
    methodData = &(targClass->classData->localMethods[methodIdx]);

    /* Check the method details */
    if ((strcmp(methodName, methodData->name) != 0) ||
        (strcmp(methodDesc, methodData->descriptorStr) != 0)) {
        (void) fprintf(stderr, 
                       "Incorrect method reference - not :%s:%s: but :%s:%s:\n",
                       methodName, methodDesc,
                       methodData->name, methodData->descriptorStr);
        JEM_DumpFrame(env);
        abort();
    }
}

/**
 * Method used to create and execute a new frame for a linked interface method
 * that was defined through the LinkClass() method. NOTE: the argument stack
 * must be fully "constructed" (including object instance) prior to calling
 * this method and no limit or interface type checking is performed (null
 * object will be caught).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     methodIdx - the index of the linked interface method reference to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, bad interface) (exception
 *                  has been thrown in the current environment, will not
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     IncompatibleClassChangeError - the interface was unmappable to the
 *                                    provided object instance
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteLinkedInterfaceMethod(JNIEnv *env, JEMCC_Object *obj,
                                        jint methodIdx, 
                                        JEMCC_ReturnValue *retVal) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData =
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEM_ClassMethodData *ifMethodData, *targetMethodData;
    JEM_ClassData *targetClassData;
    JEMCC_Class **assignClassPtr;
    jint i, rc;

    /* Central test point simplifies external code */
    if (obj == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
        return JNI_EINVAL;
    }

    /* Obtain the appropriate method reference details */
    ifMethodData = classData->classMethodRefs[methodIdx];

    /* Interface calls need lookup of method cross reference in target object */
    targetClassData = obj->classReference->classData;
    assignClassPtr = targetClassData->assignList + 1;
    for (i = 1; i < targetClassData->assignmentCount; i++) {
        if (*assignClassPtr == ifMethodData->parentClass) break;
        assignClassPtr++;
    }
    if (i > targetClassData->assignmentCount) {
        JEMCC_ThrowStdThrowableIdx(env, 
                              JEMCC_Class_IncompatibleClassChangeError,
                              NULL, "interface method call on invalid object");
        return JNI_EINVAL;
    }
    targetMethodData = targetClassData->methodLinkTables[i]
                                             [ifMethodData->methodIndex];
    if (targetMethodData == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_AbstractMethodError,
                                   NULL, ifMethodData->name);
        return JNI_EINVAL;
    }
    methodIdx = targetMethodData->methodIndex;
    targetMethodData = targetClassData->methodLinkTables[0][methodIdx];

    /* Construct the execution frame */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) {
        return JNI_ERR;
    } else {
        if (retVal != NULL) *retVal = ((JEM_JNIEnv *) env)->nativeReturnValue;
    }

    return JNI_OK;
}

/**
 * Method used to create and execute a new frame for a linked method that was
 * defined through the LinkClass() method.  This may be a virtual method call
 * which handles overrides of the reference class method or a non-virtual
 * call which exactly executes the referenced method.  NOTE: the argument
 * stack must be fully "constructed" (including object instance) prior to
 * calling this method and no setup checking is  performed (null object
 * will be caught).
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     methodIdx - the index of the linked method reference to use
 *     isVirtual - if JEMCC_VIRTUAL_METHOD (JNI_TRUE), locate the virtual
 *                 class method (accounts for overridden methods in the class
 *                 of the object instance), otherwise, prepare to execute
 *                 the exact method referenced
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, abstract method) (exception
 *                  has been thrown in the current environment, will not
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteLinkedInstanceMethod(JNIEnv *env, JEMCC_Object *obj,
                                       jint methodIdx, jboolean isVirtual,
                                       JEMCC_ReturnValue *retVal) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData =
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEM_ClassMethodData *targetMethodData;
    JEM_ClassData *targetClassData;
    jint rc;

    /* Central test point simplifies external code */
    if (obj == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
        return JNI_EINVAL;
    }

    /* Obtain the appropriate method reference details */
    targetMethodData = classData->classMethodRefs[methodIdx];

    if (isVirtual != JNI_FALSE) {
        /* Virtual calls need the method of the object class by cross index */
        methodIdx = targetMethodData->methodIndex;
        targetClassData = obj->classReference->classData;
        targetMethodData = targetClassData->methodLinkTables[0][methodIdx];
    }

    /* Construct the execution frame */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) {
        return JNI_ERR;
    } else {
        if (retVal != NULL) *retVal = ((JEM_JNIEnv *) env)->nativeReturnValue;
    }

    return JNI_OK;
}

/**
 * Method used to create and execute a new frame for a linked static method
 * that was defined through the LinkClass() method.  NOTE: the argument stack
 * must be fully "constructed" (no this!) prior to calling this method and
 * no setup checking is performed.
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     methodIdx - the index of the linked method reference to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteLinkedStaticMethod(JNIEnv *env, jint methodIdx,
                                     JEMCC_ReturnValue *retVal) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData = 
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEM_ClassMethodData *targetMethodData;
    jint rc;

    /* Obtain the appropriate method reference details */
    targetMethodData = classData->classMethodRefs[methodIdx];

    /* Static methods are simple - just push the reference directly */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) {
        return JNI_ERR;
    } else {
        if (retVal != NULL) *retVal = ((JEM_JNIEnv *) env)->nativeReturnValue;
    }

    return JNI_OK;
}

/**
 * JEMCC verification method for index cross-referencing checks.
 */
void JEMCC_CheckLinkedMethodReference(JNIEnv *env, jint methodIdx,
                                      const char *className, 
                                      const char *methodName,
                                      const char *methodDesc) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData =
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEM_ClassMethodData *methodData;

    /* Remap/check the targeted class instance */
    methodData = classData->classMethodRefs[methodIdx];
    if (strcmp(className, methodData->parentClass->classData->className) != 0) {
        (void) fprintf(stderr, "Incorrect method class - not :%s: but :%s:\n",
                               className, classData->className);
        JEM_DumpFrame(env);
        abort();
    }

    /* Check the method details */
    if ((strcmp(methodName, methodData->name) != 0) ||
        (strcmp(methodDesc, methodData->descriptorStr) != 0)) {
        (void) fprintf(stderr, 
                       "Incorrect method reference - not :%s:%s: but :%s:%s:\n",
                       methodName, methodDesc,
                       methodData->name, methodData->descriptorStr);
        JEM_DumpFrame(env);
        abort();
    }
}

/**
 * Method used to create and execute a new frame for a superclass method call,
 * similar to calling the ExecuteInstance method with a local, non-virtual 
 * index corresponding to the currently executing class method.  NOTE: the 
 * argument stack must be fully "constructed" (including object instance) prior  * to calling this method and no setup or null checking is performed.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     superClass - the superclass instance to extract the method from
 *                  (may be from a lower level in the inheritance tree).  If
 *                  NULL, the immediate superclass of the current class is
 *                  used.
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (abstract method) (exception has been 
 *                  thrown in the current environment, will not have been
 *                  caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteSuperClassMethod(JNIEnv *env, JEMCC_Class *superClass,
                                   JEMCC_ReturnValue *retVal) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData, *superClassData;
    JEM_ClassMethodData *targetMethodData;
    jint rc, methodIdx;

    if (superClass == NULL) {
        classData = jenv->topFrame->currentMethod->parentClass->classData;
        superClass = *(classData->assignList);
    }
    superClassData = superClass->classData;

    /* Obtain the appropriate method reference details (from superclass) */
    methodIdx = jenv->topFrame->currentMethod->methodIndex;
    targetMethodData = superClassData->methodLinkTables[0][methodIdx];

    /* Non-virtual method, just push the reference directly */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) {
        return JNI_ERR;
    } else {
        if (retVal != NULL) *retVal = ((JEM_JNIEnv *) env)->nativeReturnValue;
    }

    return JNI_OK;
}

/**
 * Method used to create and execute a new frame for a superclass constructor
 * call, similar to calling the ExecuteInstance method with the immediate
 * superclass instance and a local, non-virtual index corresponding to the
 * desired constructor to be called.  NOTE: the argument stack must be fully
 * "constructed" (including new object instance) prior to calling this
 * method and no setup or null checking is performed.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     constIdx - the local index of the target constructor in the immediate
 *                superclass
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     Other exceptions as thrown by the method in question.
 */
jint JEMCC_ExecuteSuperClassConstructor(JNIEnv *env, jint constIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *classData =
                      jenv->topFrame->currentMethod->parentClass->classData;
    JEM_ClassData *superClassData = (*(classData->assignList))->classData;
    JEM_ClassMethodData *targetMethodData;
    jint rc;

    /* Obtain the appropriate method reference details (from superclass) */
    targetMethodData = &(superClassData->localMethods[constIdx]);

    /* Non-virtual method, just push the reference directly */
    rc = JEM_PushFrame(env, (jmethodID) targetMethodData, NULL);
    if (rc != JNI_OK) return rc;

    /* Run it (note that constructors always return void) */
    JEM_ExecuteCurrentFrame(env, JNI_FALSE);
    if (((JEM_JNIEnv *) env)->pendingException != NULL) return JNI_ERR;
    return JNI_OK;
}

/**
 * Test condition to determine if the indicated local method of the indicated
 * class (NULL indicates current class instance) has been "overridden" in the
 * target object's class.  Used in JEMCC native code to identify situations
 * where the default implementation is in effect and a frame execution is
 * not required.
 *
 * NOTE: this does not in any way validate that the target object class is
 *       assignable from the specified class
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the target object to test for default method implementation
 *     targClass - the target class to check the method from - if NULL, use
 *                 the class associated with the currently executing method
 *     methodIdx - the index of the local class method to test against
 *
 * Returns:
 *     JNI_TRUE if the target and method references are identical (default
 *     implementation applies), JNI_FALSE otherwise.
 */
jboolean JEMCC_IsDefaultMethod(JNIEnv *env, JEMCC_Object *obj,
                               JEMCC_Class *targClass, jint methodIdx) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassMethodData *methodData;
    JEM_ClassData *classData;

    /* Remap the target class/target method instance */
    if (targClass == NULL) {
        targClass = jenv->topFrame->currentMethod->parentClass;
    }
    methodData = &(targClass->classData->localMethods[methodIdx]);

    /* Compare against the associated method in the given object */
    classData = obj->classReference->classData;
    if (classData->methodLinkTables[0][methodData->methodIndex] == methodData) {
        return JNI_TRUE;
    }

    return JNI_FALSE;
}
