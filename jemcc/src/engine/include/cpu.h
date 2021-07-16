/**
 * Internal function/structure definitions for JEMCC bytecode cpu components.
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

#ifndef JEM_CPU_H
#define JEM_CPU_H 1

/* NOTE: this is auto-read by jem.h, so it shouldn't be directly read */

/* <jemcc_start> */

/********************** CPU/Engine Operations **************************/

/**
 * This defines the type union for both local variables and operand stacks.
 * While it is exposed, it should only be used indirectly through the
 * JEMCC_* macros defined below.
 */
typedef union JEM_FrameEntry {
    jint i;
    jfloat f;
#ifdef DBL_SNGL_FRAME_MISALIGNED
    jlong l;
    jdouble d;
#endif
    JEMCC_Object *obj;
    /* Note that retAddr is equivalent to object, as VM doesn't differentiate */
    void *retAddr;
} JEM_FrameEntry;

/**
 * The structure definition of the base references of a method frame
 * instance (the operand stack and the local variable set).  This structure
 * should not be used directly but rather the JEMCC_* macros defined
 * below should be used to manipulate the frame information.
 */
struct JEMCC_VMFrame {
    JEM_FrameEntry *operandStackTop;
    JEM_FrameEntry *localVars;
};

/**
 * Structure definition to overlay double/long dual stack record entries.
 * Note - this needs to be exactly twice the size of JEM_FrameEntry, to
 * properly handle the entry indexing as defined in the Java virtual machine
 * specification.  Also note that this should not be used directly by
 * JEMCC method instances but the JEMCC_* macros defined below should be
 * used instead.
 */
typedef union JEM_DblFrameEntry {
    /* Dummy union entry to ensure sizeof two frame units */
    struct dblrecord {
        JEM_FrameEntry one;
        JEM_FrameEntry two;
    } dblrec;

    /* Actual frame data elements which require two stack index units */
    jlong l;
    jdouble d;
} JEM_DblFrameEntry;

/**
 * The following macros define the "methods" by which a JEMCC method
 * can manipulate the operand stack (for method calls) or the local
 * variable store (for calling arguments).
 */
#define JEMCC_STORE_INT(frame, index, val) frame->localVars[index].i = val;
#define JEMCC_LOAD_INT(frame, index) frame->localVars[index].i

#define JEMCC_STORE_FLOAT(frame, index, val) frame->localVars[index].f = val;
#define JEMCC_LOAD_FLOAT(frame, index) frame->localVars[index].f

#define JEMCC_STORE_OBJECT(frame, index, val) frame->localVars[index].obj = val;
#define JEMCC_LOAD_OBJECT(frame, index) frame->localVars[index].obj

#define JEMCC_STORE_LONG(frame, index, val) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->l = val;
#define JEMCC_LOAD_LONG(frame, index) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->l

#define JEMCC_STORE_DOUBLE(frame, index, val) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->d = val;
#define JEMCC_LOAD_DOUBLE(frame, index) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->d

/* Macros for manipulating operand stack frame entries */
#define JEMCC_PUSH_STACK_INT(frame, val) \
               ((frame->operandStackTop)++)->i = val;
#define JEMCC_POP_STACK_INT(frame) \
               (--(frame->operandStackTop))->i

#define JEMCC_PUSH_STACK_FLOAT(frame, val) \
               ((frame->operandStackTop)++)->f = val;
#define JEMCC_POP_STACK_FLOAT(frame) \
               (--(frame->operandStackTop))->f

#define JEMCC_PUSH_STACK_OBJECT(frame, val) \
               ((frame->operandStackTop)++)->obj = val;
#define JEMCC_POP_STACK_OBJECT(frame) \
               (--(frame->operandStackTop))->obj

#define JEMCC_PUSH_STACK_LONG(frame, val) \
               (((JEM_DblFrameEntry *) (frame->operandStackTop))++)->l = val;
#define JEMCC_POP_STACK_LONG(frame) \
               (--((JEM_DblFrameEntry *) (frame->operandStackTop)))->l

#define JEMCC_PUSH_STACK_DOUBLE(frame, val) \
               (((JEM_DblFrameEntry *) (frame->operandStackTop))++)->d = val;
#define JEMCC_POP_STACK_DOUBLE(frame) \
               (--((JEM_DblFrameEntry *) (frame->operandStackTop)))->d

/* <jemcc_end> */

/* Definitions for the VM internal frame management operations */
#define FRAME_ROOT 0
#define FRAME_BYTECODE 1
#define FRAME_NATIVE 2
#define FRAME_JEMCC 3
#define FRAME_TYPE_MASK 0x03

#define FRAME_THROWABLE_CAPTURE 0x08

/* This is the actual frame structure used within the VM */
typedef struct JEM_VMFrameExt {
    /* This structure must be first, for mapping purposes! */
    JEMCC_VMFrame frameVars;

    /* Flags describing the operational mode of this frame */
    juint opFlags;

    /* Pointer to the previous frame in this stack block */
    struct JEM_VMFrameExt *previousFrame;

    /* Frame depth number for this stack, starting from one */
    juint frameDepth;

    /* Currently executing class method and corresponding program counters */
    struct JEM_ClassMethodData *currentMethod;
    jint lastPC, pc;

    /* Pointer to the first object allocated in this frame */
    void *firstAllocObjectRecord;
} JEM_VMFrameExt;

/* <jemcc_start> */

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
 *                 environment will be rethrown (pass-on mechanism).
 *
 * Exceptions:
 *     It is possible that another more critical exception (such as
 *     OutOfMemoryError) may be thrown in place of the specified throwable,
 *     if an error occurs in processing the exception.
 */
JNIEXPORT void JNICALL JEMCC_ProcessThrowable(JNIEnv *env,
                                              JEMCC_Object *throwable);

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
JNIEXPORT JEMCC_Object *JNICALL JEMCC_GetCallingObjectInstance(JNIEnv *env);

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
JNIEXPORT JEMCC_Class *JNICALL JEMCC_GetCallingClassInstance(JNIEnv *env);

/* <jemcc_end> */

/**
 * Generate a new frame instance on the provided environment stack.
 * Externally exposed for test purposes only - the PushFrame method
 * should be used in other cases.
 *
 * Parameters:
 *     env - the VM environment into which the new frame is to be created
 *     frameType - the type of frame instance to create (bytecode, native or
 *                 JEMCC)
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
 *     StackOverflowError - the new frame has exceeded the recursion limit
 *                          for the application
 */
JNIEXPORT JEMCC_VMFrame *JNICALL JEM_CreateFrame(JNIEnv *env, 
                                       int frameType, int methodArgCount,
                                       int localVarCount, int maxOpStackCount);

/**
 * Create a new frame and push it onto the given environment stack.  This
 * method uses the given method reference explicitly to construct the frame -
 * interface/static/virtual mappings must be handled by the caller (currently
 * handled by the individual opcodes or the JEMCC_Push* wrappers).  The
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
 *     StackOverflowError - the new frame has exceeded the recursion limit
 *                          for the application
 */
JNIEXPORT jint JNICALL JEM_PushFrame(JNIEnv *env, jmethodID methodID,
                                     JEMCC_VMFrame **frameRef);

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
JNIEXPORT void JNICALL JEM_ExecuteCurrentFrame(JNIEnv *env,
                                               jboolean fromByteCode);

#endif
