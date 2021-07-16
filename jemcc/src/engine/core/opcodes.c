/**
 * JEMCC opcode definitions for use in the runtime interpreter.
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

/* This file is not compiled separately, read into cpu.c as switch contents */

/* See the cpu.c file for the location of this definition */
#ifdef DEBUG_CPU_INTERNALS

/* Convenience macros for debugging stack data */
#define JEMCC_DEBUG_STACK_INT(frame) \
    fprintf(stderr, "Stack int val %i %x\n", \
                    (frame->operandStackTop - 1)->i, \
                    (frame->operandStackTop - 1)->i);

#define JEMCC_DEBUG_STACK_FLOAT(frame) \
    fprintf(stderr, "Stack float val %f\n", \
                    (frame->operandStackTop - 1)->f);

#define JEMCC_DEBUG_STACK_OBJECT(frame) \
    fprintf(stderr, "Stack object val %p\n", \
                    (frame->operandStackTop - 1)->obj);

#define JEMCC_DEBUG_STACK_LONG(frame) \
    fprintf(stderr, "Stack long val %lli %llx\n", \
                    (((JEM_DblFrameEntry *) (frame->operandStackTop)) - 1)->l,\
                    (((JEM_DblFrameEntry *) (frame->operandStackTop)) - 1)->l);

#define JEMCC_DEBUG_STACK_DOUBLE(frame) \
    fprintf(stderr, "Stack long val %f\n", \
                    (((JEM_DblFrameEntry *) (frame->operandStackTop)) - 1)->d);

/* Wrapper macro to make contents more readable */
#define OPCODE(opname, opcode) \
       (void) fprintf(stderr, "Completing opcode\n"); \
    break; \
    case opcode: \
       (void) fprintf(stderr, "[%i] Starting opcode %s\n", \
                              currentFrameExt->pc - 1, opname);

#else

#define JEMCC_DEBUG_STACK_INT(frame)
#define JEMCC_DEBUG_STACK_FLOAT(frame)
#define JEMCC_DEBUG_STACK_OBJECT(frame)
#define JEMCC_DEBUG_STACK_LONG(frame)
#define JEMCC_DEBUG_STACK_DOUBLE(frame)
#define OPCODE(opname, opcode) \
    break; \
    case opcode:

#endif

OPCODE("aaload", 0x32)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_OBJECT(currentFrame, 
                          *(((JEMCC_Object **) array->arrayData) + index));
        }
    }
}

OPCODE("aastore", 0x53)
{
    JEMCC_Object *val = JEMCC_POP_STACK_OBJECT(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
fprintf(stderr, "Storing %p in %p at %i\n", val, array, index);
        /* TODO - check component type information (assignments) */
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((JEMCC_Object **) array->arrayData) + index) = val;
        }
    }
}

OPCODE("aconst_null", 0x01)
{
    JEMCC_PUSH_STACK_OBJECT(currentFrame, NULL);
}

OPCODE("aload", 0x19)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_PUSH_STACK_OBJECT(currentFrame, 
                            JEMCC_LOAD_OBJECT(currentFrame, index));
}

OPCODE("aload_0", 0x2a)
{
    JEMCC_PUSH_STACK_OBJECT(currentFrame, JEMCC_LOAD_OBJECT(currentFrame, 0));
}

OPCODE("aload_1", 0x2b)
{
    JEMCC_PUSH_STACK_OBJECT(currentFrame, JEMCC_LOAD_OBJECT(currentFrame, 1));
}

OPCODE("aload_2", 0x2c)
{
    JEMCC_PUSH_STACK_OBJECT(currentFrame, JEMCC_LOAD_OBJECT(currentFrame, 2));
}

OPCODE("aload_3", 0x2d)
{
    JEMCC_PUSH_STACK_OBJECT(currentFrame, JEMCC_LOAD_OBJECT(currentFrame, 3));
}

OPCODE("anewarray", 0xbd)
{
    juint index = (juint) read_op2(currentFrameExt);
    jint length = JEMCC_POP_STACK_INT(currentFrame);
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    JEMCC_Class *clRef = classData->classRefs[index];
    JEMCC_Class *classClass = VM_CLASS(JEMCC_Class_Class);
    JEMCC_Object *objArray;

    if (clRef->classReference != classClass) {
        /* TODO - class resolution error - throw it !*/
(void) fprintf(stderr, "RESOLUTION ERROR %p %s\n", clRef, clRef->classData->className);
(void) fprintf(stderr, "SUPERCLASS %p\n", clRef->classReference);
abort();
    } else if (length < 0) {
(void) fprintf(stderr, "NEGATIVE SIZE\n");
abort();
        /* TODO Throw NegativeArraySizeException */
    } else {
        objArray = JEMCC_NewObjectArray(env, length, (jclass) clRef, NULL);
        if (objArray != NULL) JEMCC_PUSH_STACK_OBJECT(currentFrame, objArray);
    }
}

OPCODE("areturn", 0xb0)
{
    /* Grab the value from the working stack */
    JEMCC_Object *retVal = JEMCC_POP_STACK_OBJECT(currentFrame);

    /* Pop the frame instance, absorbing method arguments */
    JEM_PopFrame(env);

    /* Handle the return value according to the current frame type */
    currentFrameExt = ((JEM_JNIEnv *) env)->topFrame;
    switch (currentFrameExt->opFlags & FRAME_TYPE_MASK) {
        case FRAME_ROOT:
        case FRAME_NATIVE:
        case FRAME_JEMCC:
            ((JEM_JNIEnv *) env)->nativeReturnValue.objVal = retVal;
            break;
        case FRAME_BYTECODE:
            currentFrame = (JEMCC_VMFrame *) currentFrameExt;
            JEMCC_PUSH_STACK_OBJECT(currentFrame, retVal);
            break;
    }
}

OPCODE("arraylength", 0xbe)
{
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, array->arrayLength);
    }
}

OPCODE("astore", 0x3a)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_STORE_OBJECT(currentFrame, index, 
                       JEMCC_POP_STACK_OBJECT(currentFrame));
}

OPCODE("astore_0", 0x4b)
{
    JEMCC_STORE_OBJECT(currentFrame, 0, JEMCC_POP_STACK_OBJECT(currentFrame));
}

OPCODE("astore_1", 0x4c)
{
    JEMCC_STORE_OBJECT(currentFrame, 1, JEMCC_POP_STACK_OBJECT(currentFrame));
}

OPCODE("astore_2", 0x4d)
{
    JEMCC_STORE_OBJECT(currentFrame, 2, JEMCC_POP_STACK_OBJECT(currentFrame));
}

OPCODE("astore_3", 0x4e)
{
    JEMCC_STORE_OBJECT(currentFrame, 3, JEMCC_POP_STACK_OBJECT(currentFrame));
}

OPCODE("athrow", 0xbf)
{
    jobject ex = JEMCC_POP_STACK_OBJECT(currentFrame);
    if (ex == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        JEMCC_ProcessThrowable(env, (JEMCC_Object *) ex);
    }
}

OPCODE("baload", 0x33)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_INT(currentFrame, 
                          (int) *(((jbyte *) array->arrayData) + index));
        }
    }
}

OPCODE("bastore", 0x54)
{
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((jbyte *) array->arrayData) + index) = (jbyte) (val & 0xFF);
        }
    }
}

OPCODE("bipush", 0x10)
{
    jbyte res = (jbyte) read_op1(currentFrameExt);
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) res);
}

OPCODE("breakpoint", 0xca) {
    /* JVMDI - someday handle the debugging interface */
}

OPCODE("caload", 0x34)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_INT(currentFrame, 
                          (int) *(((jchar *) array->arrayData) + index));
        }
    }
}

OPCODE("castore", 0x55)
{
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((jchar *) array->arrayData) + index) = 
                                                    (jchar) (val & 0xFFFF);
        }
    }
}

OPCODE("checkcast", 0xc0)
{
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    JEMCC_Class *checkClass = classData->classRefs[index];
    JEMCC_Class *objClass, **compClassPtr;
    JEMCC_Object *obj;

    /* Handle unknown class cast */
    if (checkClass == NULL) {
        /* TODO - really do missing class check */
    } else {     
        obj = (currentFrame->operandStackTop - 1)->obj;
        /* Note that 'null' object can always be cast */
        if (obj != NULL) {
            objClass = (JEMCC_Class *) obj->classReference;
            if (objClass != checkClass) {
                compClassPtr = objClass->classData->assignList;
                while (*compClassPtr != NULL) {
                    if (*compClassPtr == checkClass) break;
                    compClassPtr++;
                }

                /* No match was found in assignment list - cast is invalid */
                if (*compClassPtr == NULL) {
                    JEMCC_ThrowStdThrowableIdxV(env, 
                                          JEMCC_Class_ClassCastException, NULL,
                                          "Class is not instance of ",
                                          checkClass->classData->className,
                                          NULL);
                }
            }
        }
    }
}

OPCODE("d2f", 0x90)
{
    jdouble dbl = JEMCC_POP_STACK_DOUBLE(currentFrame);
    /* TODO NaN -> NaN, too big/Inf -> Inf, too small -> 0.0 */
    JEMCC_PUSH_STACK_FLOAT(currentFrame, (jfloat) dbl);
}

OPCODE("d2i", 0x8e)
{
    jdouble dbl = JEMCC_POP_STACK_DOUBLE(currentFrame);
    /* TODO NaN -> 0, too big/Inf -> +- biggest int, too small -> 0 */
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) dbl);
}

OPCODE("d2l", 0x8f)
{
    jdouble dbl = JEMCC_POP_STACK_DOUBLE(currentFrame);
    /* TODO NaN -> 0, too big/Inf -> +- biggest long, too small -> 0 */
    JEMCC_PUSH_STACK_LONG(currentFrame, (jlong) dbl);
}

OPCODE("dadd", 0x63)
{
    jdouble res = JEMCC_POP_STACK_DOUBLE(currentFrame);
    res = res + JEMCC_POP_STACK_DOUBLE(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, res);
}

OPCODE("daload", 0x31)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_DOUBLE(currentFrame, 
                          *(((jdouble *) array->arrayData) + index));
        }
    }
}

OPCODE("dastore", 0x52)
{
    jdouble val = JEMCC_POP_STACK_DOUBLE(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((jdouble *) array->arrayData) + index) = val;
        }
    }
}

OPCODE("dcmpg", 0x98)
{
    jdouble bval = JEMCC_POP_STACK_DOUBLE(currentFrame);
    jdouble aval = JEMCC_POP_STACK_DOUBLE(currentFrame);
    if (isnan(aval) || isnan(bval)) {
        JEMCC_PUSH_STACK_INT(currentFrame, 1);
    } else if (aval > bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 1);
    } else if (aval == bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 0);
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, -1);
    }
}

OPCODE("dcmpl", 0x97)
{
    jdouble bval = JEMCC_POP_STACK_DOUBLE(currentFrame);
    jdouble aval = JEMCC_POP_STACK_DOUBLE(currentFrame);
    if (isnan(aval) || isnan(bval)) {
        JEMCC_PUSH_STACK_INT(currentFrame, -1);
    } else if (aval > bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 1);
    } else if (aval == bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 0);
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, -1);
    }
}

OPCODE("dconst_0", 0x0e)
{
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, 0.0);
}

OPCODE("dconst_1", 0x0f)
{
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, 1.0);
}

OPCODE("ddiv", 0x6f)
{
    jdouble divisor = JEMCC_POP_STACK_DOUBLE(currentFrame);
    jdouble numerand = JEMCC_POP_STACK_DOUBLE(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, numerand / divisor);
}

OPCODE("dload", 0x18)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, 
                            JEMCC_LOAD_DOUBLE(currentFrame, index));
}

OPCODE("dload_0", 0x26)
{
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, JEMCC_LOAD_DOUBLE(currentFrame, 0));
}

OPCODE("dload_1", 0x27)
{
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, JEMCC_LOAD_DOUBLE(currentFrame, 1));
}

OPCODE("dload_2", 0x28)
{
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, JEMCC_LOAD_DOUBLE(currentFrame, 2));
}

OPCODE("dload_3", 0x29)
{
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, JEMCC_LOAD_DOUBLE(currentFrame, 3));
}

OPCODE("dmul", 0x6b)
{
    jdouble res = JEMCC_POP_STACK_DOUBLE(currentFrame);
    res = res * JEMCC_POP_STACK_DOUBLE(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, res);
}

OPCODE("dneg", 0x77)
{
    jdouble val = JEMCC_POP_STACK_DOUBLE(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, -val);
}

OPCODE("drem", 0x73)
{
    jdouble divisor = JEMCC_POP_STACK_DOUBLE(currentFrame);
    jdouble numerand = JEMCC_POP_STACK_DOUBLE(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, fmod(numerand, divisor));
}

OPCODE("dreturn", 0xaf)
{
    /* Grab the value from the working stack */
    jdouble retVal = JEMCC_POP_STACK_DOUBLE(currentFrame);

    /* Pop the frame instance, absorbing method arguments */
    JEM_PopFrame(env);

    /* Handle the return value according to the current frame type */
    currentFrameExt = ((JEM_JNIEnv *) env)->topFrame;
    switch (currentFrameExt->opFlags & FRAME_TYPE_MASK) {
        case FRAME_ROOT:
        case FRAME_NATIVE:
        case FRAME_JEMCC:
            ((JEM_JNIEnv *) env)->nativeReturnValue.dblVal = retVal;
            break;
        case FRAME_BYTECODE:
            currentFrame = (JEMCC_VMFrame *) currentFrameExt;
            JEMCC_PUSH_STACK_DOUBLE(currentFrame, retVal);
            break;
    }
}

OPCODE("dstore", 0x39)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_STORE_DOUBLE(currentFrame, index, 
                       JEMCC_POP_STACK_DOUBLE(currentFrame));
}

OPCODE("dstore_0", 0x47)
{
    JEMCC_STORE_DOUBLE(currentFrame, 0, JEMCC_POP_STACK_DOUBLE(currentFrame));
}

OPCODE("dstore_1", 0x48)
{
    JEMCC_STORE_DOUBLE(currentFrame, 1, JEMCC_POP_STACK_DOUBLE(currentFrame));
}

OPCODE("dstore_2", 0x49)
{
    JEMCC_STORE_DOUBLE(currentFrame, 2, JEMCC_POP_STACK_DOUBLE(currentFrame));
}

OPCODE("dstore_3", 0x4a)
{
    JEMCC_STORE_DOUBLE(currentFrame, 3, JEMCC_POP_STACK_DOUBLE(currentFrame));
}

OPCODE("dsub", 0x67)
{
    jdouble sval = JEMCC_POP_STACK_DOUBLE(currentFrame);
    jdouble val = JEMCC_POP_STACK_DOUBLE(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, val - sval);
}

OPCODE("dup", 0x59)
{
    JEMCC_DEBUG_STACK_OBJECT(currentFrame);
    *(currentFrame->operandStackTop) = *(currentFrame->operandStackTop - 1);
    currentFrame->operandStackTop++;
    JEMCC_DEBUG_STACK_OBJECT(currentFrame);
}

OPCODE("dup_x1", 0x5a)
{
    *(currentFrame->operandStackTop) = *(currentFrame->operandStackTop - 1);
    *(currentFrame->operandStackTop - 1) = *(currentFrame->operandStackTop - 2);
    *(currentFrame->operandStackTop - 2) = *(currentFrame->operandStackTop);
    currentFrame->operandStackTop++;
}

OPCODE("dup_x2", 0x5b)
{
    *(currentFrame->operandStackTop) = *(currentFrame->operandStackTop - 1);
    *(currentFrame->operandStackTop - 1) = *(currentFrame->operandStackTop - 2);
    *(currentFrame->operandStackTop - 2) = *(currentFrame->operandStackTop - 3);
    *(currentFrame->operandStackTop - 3) = *(currentFrame->operandStackTop);
    currentFrame->operandStackTop++;
}

OPCODE("dup2", 0x5c)
{
    *((JEM_DblFrameEntry *) (currentFrame->operandStackTop)) = 
            *(((JEM_DblFrameEntry *) (currentFrame->operandStackTop)) - 1);
    ((JEM_DblFrameEntry *) currentFrame->operandStackTop)++;
}

OPCODE("dup2_x1", 0x5d)
{
    (void) memmove(currentFrame->operandStackTop - 3,
                   currentFrame->operandStackTop - 1,
                   3 * sizeof(JEM_FrameEntry));
    *((JEM_DblFrameEntry *) (currentFrame->operandStackTop - 3)) =
                    *((JEM_DblFrameEntry *) (currentFrame->operandStackTop));
    ((JEM_DblFrameEntry *) currentFrame->operandStackTop)++;
}

OPCODE("dup2_x2", 0x5e)
{
    (void) memmove(currentFrame->operandStackTop - 4,
                   currentFrame->operandStackTop - 2,
                   4 * sizeof(JEM_FrameEntry));
    *((JEM_DblFrameEntry *) (currentFrame->operandStackTop - 4)) =
                    *((JEM_DblFrameEntry *) (currentFrame->operandStackTop));
    ((JEM_DblFrameEntry *) currentFrame->operandStackTop)++;
}

OPCODE("f2d", 0x8d)
{
    jfloat f = JEMCC_POP_STACK_FLOAT(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, (jdouble) f);
}

OPCODE("f2i", 0x8b)
{
    jfloat f = JEMCC_POP_STACK_FLOAT(currentFrame);
    /* TODO NaN -> 0, too big/Inf -> +- biggest int, too small -> 0 */
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) f);
}

OPCODE("f2l", 0x8c)
{
    jfloat f = JEMCC_POP_STACK_FLOAT(currentFrame);
    /* TODO NaN -> 0, too big/Inf -> +- biggest int, too small -> 0 */
    JEMCC_PUSH_STACK_LONG(currentFrame, (jlong) f);
}

OPCODE("fadd", 0x62)
{
    jfloat res = JEMCC_POP_STACK_FLOAT(currentFrame);
    res = res + JEMCC_POP_STACK_FLOAT(currentFrame);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, res);
}

OPCODE("faload", 0x30)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_FLOAT(currentFrame, 
                              *(((jfloat *) array->arrayData) + index));
        }
    }
}

OPCODE("fastore", 0x51)
{
    jfloat val = JEMCC_POP_STACK_FLOAT(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((jfloat *) array->arrayData) + index) = val;
        }
    }
}

OPCODE("fcmpg", 0x96)
{
    jfloat bval = JEMCC_POP_STACK_FLOAT(currentFrame);
    jfloat aval = JEMCC_POP_STACK_FLOAT(currentFrame);
    if (isnan(aval) || isnan(bval)) {
        JEMCC_PUSH_STACK_INT(currentFrame, 1);
    } else if (aval > bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 1);
    } else if (aval == bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 0);
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, -1);
    }
}

OPCODE("fcmpl", 0x95)
{
    jfloat bval = JEMCC_POP_STACK_FLOAT(currentFrame);
    jfloat aval = JEMCC_POP_STACK_FLOAT(currentFrame);
    if (isnan(aval) || isnan(bval)) {
        JEMCC_PUSH_STACK_INT(currentFrame, -1);
    } else if (aval > bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 1);
    } else if (aval == bval) {
        JEMCC_PUSH_STACK_INT(currentFrame, 0);
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, -1);
    }
}

OPCODE("fconst_0", 0x0b)
{
    JEMCC_PUSH_STACK_FLOAT(currentFrame, 0.0);
}

OPCODE("fconst_1", 0x0c)
{
    JEMCC_PUSH_STACK_FLOAT(currentFrame, 1.0);
}

OPCODE("fconst_2", 0x0d)
{
    JEMCC_PUSH_STACK_FLOAT(currentFrame, 2.0);
}

OPCODE("fdiv", 0x6e)
{
    jfloat divisor = JEMCC_POP_STACK_FLOAT(currentFrame);
    jfloat numerand = JEMCC_POP_STACK_FLOAT(currentFrame);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, numerand / divisor);
}

OPCODE("fload", 0x17)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, 
                           JEMCC_LOAD_FLOAT(currentFrame, index));
}

OPCODE("fload_0", 0x22)
{
    JEMCC_PUSH_STACK_FLOAT(currentFrame, JEMCC_LOAD_FLOAT(currentFrame, 0));
}

OPCODE("fload_1", 0x23)
{
    JEMCC_PUSH_STACK_FLOAT(currentFrame, JEMCC_LOAD_FLOAT(currentFrame, 1));
}

OPCODE("fload_2", 0x24)
{
    JEMCC_PUSH_STACK_FLOAT(currentFrame, JEMCC_LOAD_FLOAT(currentFrame, 2));
}

OPCODE("fload_3", 0x25)
{
    JEMCC_PUSH_STACK_FLOAT(currentFrame, JEMCC_LOAD_FLOAT(currentFrame, 3));
}

OPCODE("fmul", 0x6a)
{
    jfloat res = JEMCC_POP_STACK_FLOAT(currentFrame);
    res = res * JEMCC_POP_STACK_FLOAT(currentFrame);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, res);
}

OPCODE("fneg", 0x76)
{
    jfloat res = JEMCC_POP_STACK_FLOAT(currentFrame);
    /* NAN TESTS? TODO */
    JEMCC_PUSH_STACK_FLOAT(currentFrame, -res);
}

OPCODE("frem", 0x72)
{
    jfloat divisor = JEMCC_POP_STACK_FLOAT(currentFrame);
    jfloat numerand = JEMCC_POP_STACK_FLOAT(currentFrame);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, 
                           (jfloat) fmod((double) numerand, 
                                         (double) divisor));
}

OPCODE("freturn", 0xae)
{
    /* Grab the value from the working stack */
    jfloat retVal = JEMCC_POP_STACK_FLOAT(currentFrame);

    /* Pop the frame instance, absorbing method arguments */
    JEM_PopFrame(env);

    /* Handle the return value according to the current frame type */
    currentFrameExt = ((JEM_JNIEnv *) env)->topFrame;
    switch (currentFrameExt->opFlags & FRAME_TYPE_MASK) {
        case FRAME_ROOT:
        case FRAME_NATIVE:
        case FRAME_JEMCC:
            ((JEM_JNIEnv *) env)->nativeReturnValue.fltVal = retVal;
            break;
        case FRAME_BYTECODE:
            currentFrame = (JEMCC_VMFrame *) currentFrameExt;
            JEMCC_PUSH_STACK_FLOAT(currentFrame, retVal);
            break;
    }
}

OPCODE("fstore", 0x38)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_STORE_FLOAT(currentFrame, index, 
                      JEMCC_POP_STACK_FLOAT(currentFrame));
}

OPCODE("fstore_0", 0x43)
{
    JEMCC_STORE_FLOAT(currentFrame, 0, JEMCC_POP_STACK_FLOAT(currentFrame));
}

OPCODE("fstore_1", 0x44)
{
    JEMCC_STORE_FLOAT(currentFrame, 1, JEMCC_POP_STACK_FLOAT(currentFrame));
}

OPCODE("fstore_2", 0x45)
{
    JEMCC_STORE_FLOAT(currentFrame, 2, JEMCC_POP_STACK_FLOAT(currentFrame));
}

OPCODE("fstore_3", 0x46)
{
    JEMCC_STORE_FLOAT(currentFrame, 3, JEMCC_POP_STACK_FLOAT(currentFrame));
}

OPCODE("fsub", 0x66)
{
    jfloat sval = JEMCC_POP_STACK_FLOAT(currentFrame);
    jfloat val = JEMCC_POP_STACK_FLOAT(currentFrame);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, val - sval);
}

OPCODE("getfield", 0xb4)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassFieldData *fieldRef = classData->classFieldRefs[index];
    JEMCC_Object *targObj;
    jbyte *basePtr;

    /* NOTE: static test is already performed during verification */
    if ((fieldRef->accessFlags & ACC_RESOLVE_ERROR) != 0) {
        /* TODO - throw the resolution error */
    } else {
        /* Obtain the source object for the field */
        targObj = JEMCC_POP_STACK_OBJECT(currentFrame);
        if (targObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                       NULL, NULL);
        } else {
#ifdef DEBUG_CPU_INTERNALS
(void) fprintf(stderr, "Retrieving field from object %s\n",
                       targObj->classReference->classData->className);
(void) fprintf(stderr, "Field definition %s::%s[%i]\n",
                       fieldRef->name,
                       fieldRef->parentClass->classData->className,
                       fieldRef->fieldOffset);
#endif
            /* Grab and push the correct field type onto the operating stack */
            basePtr = ((jubyte *) &(((JEMCC_ObjectExt *) 
                                               targObj)->objectData)) + 
                                                         fieldRef->fieldOffset;
            switch (fieldRef->descriptor->generic.tag) {
                case BASETYPE_Boolean:
                case BASETYPE_Byte:
                    JEMCC_PUSH_STACK_INT(currentFrame,
                                         (jint) *((jbyte *) basePtr));
                    JEMCC_DEBUG_STACK_INT(currentFrame);
                    break;
                case BASETYPE_Char:
                    JEMCC_PUSH_STACK_INT(currentFrame,
                                         (jint) *((jchar *) basePtr));
                    JEMCC_DEBUG_STACK_INT(currentFrame);
                    break;
                case BASETYPE_Double:
                    JEMCC_PUSH_STACK_DOUBLE(currentFrame,
                                            *((jdouble *) basePtr));
                    JEMCC_DEBUG_STACK_DOUBLE(currentFrame);
                    break;
                case BASETYPE_Float:
                    JEMCC_PUSH_STACK_FLOAT(currentFrame,
                                           *((jfloat *) basePtr));
                    JEMCC_DEBUG_STACK_FLOAT(currentFrame);
                    break;
                case BASETYPE_Int:
                    JEMCC_PUSH_STACK_INT(currentFrame,
                                         *((jint *) basePtr));
                    JEMCC_DEBUG_STACK_INT(currentFrame);
                    break;
                case BASETYPE_Long:
                    JEMCC_PUSH_STACK_LONG(currentFrame,
                                          *((jlong *) basePtr));
                    JEMCC_DEBUG_STACK_LONG(currentFrame);
                    break;
                case BASETYPE_Short:
                    JEMCC_PUSH_STACK_INT(currentFrame,
                                         (jint) *((jshort *) basePtr));
                    JEMCC_DEBUG_STACK_INT(currentFrame);
                    break;
                case DESCRIPTOR_ObjectType:
                case DESCRIPTOR_ArrayType:
                    JEMCC_PUSH_STACK_OBJECT(currentFrame,
                                            *((JEMCC_Object **) basePtr));
                    JEMCC_DEBUG_STACK_OBJECT(currentFrame);
                    break;
                default:
                    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError, 
                                           NULL, "Unknown getfield data type");
                    break;
            }
        }
    }
}

OPCODE("getstatic", 0xb2)
{
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    JEM_ClassFieldData *fieldRef = classData->classFieldRefs[index];
    jbyte *staticData;

    /* NOTE: static test is already performed during verification */
    if ((fieldRef->accessFlags & ACC_RESOLVE_ERROR) != 0) {
        /* TODO - throw the resolution error */
    } else if (JEMCC_InitializeClass(env, fieldRef->parentClass) != JNI_OK) {
        /* Static initialization failed - exception is already thrown */
    } else {
        /* Yes, this could call JEM_GetStatic, but speed is better */
        staticData = ((jbyte *) fieldRef->parentClass->staticData) +
                                                       fieldRef->fieldOffset;
        switch (fieldRef->descriptor->generic.tag) {
            case BASETYPE_Boolean:
            case BASETYPE_Byte:
                JEMCC_PUSH_STACK_INT(currentFrame,
                                     (jint) *((jbyte *) staticData));
                JEMCC_DEBUG_STACK_INT(currentFrame);
                break;
            case BASETYPE_Char:
                JEMCC_PUSH_STACK_INT(currentFrame,
                                     (jint) *((jchar *) staticData));
                JEMCC_DEBUG_STACK_INT(currentFrame);
                break;
            case BASETYPE_Double:
                JEMCC_PUSH_STACK_DOUBLE(currentFrame,
                                        *((jdouble *) staticData));
                JEMCC_DEBUG_STACK_DOUBLE(currentFrame);
                break;
            case BASETYPE_Float:
                JEMCC_PUSH_STACK_FLOAT(currentFrame,
                                       *((jfloat *) staticData));
                JEMCC_DEBUG_STACK_FLOAT(currentFrame);
                break;
            case BASETYPE_Int:
                JEMCC_PUSH_STACK_INT(currentFrame,
                                     *((jint *) staticData));
                JEMCC_DEBUG_STACK_INT(currentFrame);
                break;
            case BASETYPE_Long:
                JEMCC_PUSH_STACK_LONG(currentFrame,
                                      *((jlong *) staticData));
                JEMCC_DEBUG_STACK_LONG(currentFrame);
                break;
            case BASETYPE_Short:
                JEMCC_PUSH_STACK_INT(currentFrame,
                                     (jint) *((jshort *) staticData));
                JEMCC_DEBUG_STACK_INT(currentFrame);
                break;
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                JEMCC_PUSH_STACK_OBJECT(currentFrame,
                                        *((JEMCC_Object **) staticData));
                JEMCC_DEBUG_STACK_OBJECT(currentFrame);
                break;
            default:
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError, 
                                           NULL, "Unknown getstatic data type");
                break;
        }
    }
}

OPCODE("goto", 0xa7)
{
    jshort offset = (jshort) read_op2(currentFrameExt);
    currentFrameExt->pc += offset - 3;
}

OPCODE("goto_w", 0xc8)
{
    jint offset = (jint) read_op4(currentFrameExt);
    currentFrameExt->pc += offset - 3;
}

OPCODE("i2b", 0x91)
{
    jint i = JEMCC_POP_STACK_INT(currentFrame);
    jbyte b = (jbyte) (i & 0xFF);
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) b);
}

OPCODE("i2c", 0x92)
{
    /* NOTE: unlike others, this one is not sign extending! */
    jint i = JEMCC_POP_STACK_INT(currentFrame);
    jchar ch = (jchar) (i & 0xFFFF);
    JEMCC_PUSH_STACK_INT(currentFrame, ch);
}

OPCODE("i2d", 0x87)
{
    jint i = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, (jdouble) i);
}

OPCODE("i2f", 0x86)
{
    jint i = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, (jfloat) i);
}

OPCODE("i2l", 0x85)
{
    jint i = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, (jlong) i);
}

OPCODE("i2s", 0x93)
{
    jint i = JEMCC_POP_STACK_INT(currentFrame);
    jshort s = (jshort) (i & 0xFFFF);
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) s);
}

OPCODE("iadd", 0x60)
{
    jint res = JEMCC_POP_STACK_INT(currentFrame);
    res = res + JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, res);
}

OPCODE("iaload", 0x2e)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_INT(currentFrame, 
                          *(((jint *) array->arrayData) + index));
        }
    }
}

OPCODE("iand", 0x7e)
{
    jint aval = JEMCC_POP_STACK_INT(currentFrame);
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, val & aval);
}

OPCODE("iastore", 0x4f)
{
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((jint *) array->arrayData) + index) = val;
        }
    }
}

OPCODE("iconst_0", 0x03)
{
    JEMCC_PUSH_STACK_INT(currentFrame, 0);
}

OPCODE("iconst_1", 0x04)
{
    JEMCC_PUSH_STACK_INT(currentFrame, 1);
}

OPCODE("iconst_2", 0x05)
{
    JEMCC_PUSH_STACK_INT(currentFrame, 2);
}

OPCODE("iconst_3", 0x06)
{
    JEMCC_PUSH_STACK_INT(currentFrame, 3);
}

OPCODE("iconst_4", 0x07)
{
    JEMCC_PUSH_STACK_INT(currentFrame, 4);
}

OPCODE("iconst_5", 0x08)
{
    JEMCC_PUSH_STACK_INT(currentFrame, 5);
}

OPCODE("iconst_m1", 0x02)
{
    JEMCC_PUSH_STACK_INT(currentFrame, -1);
}

OPCODE("idiv", 0x6c)
{
    jint divisor = JEMCC_POP_STACK_INT(currentFrame);
    jint numerand = JEMCC_POP_STACK_INT(currentFrame);

    if (divisor == 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ArithmeticException, 
                                   NULL, "Integer divide by zero");
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, numerand / divisor);
    }
}

OPCODE("ifeq", 0x99)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_INT(currentFrame) == 0) currentFrameExt->pc += offset;
}

OPCODE("ifge", 0x9c)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_INT(currentFrame) >= 0) currentFrameExt->pc += offset;
}

OPCODE("ifgt", 0x9d)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_INT(currentFrame) > 0) currentFrameExt->pc += offset;
}

OPCODE("ifle", 0x9e)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_INT(currentFrame) <= 0) currentFrameExt->pc += offset;
}

OPCODE("iflt", 0x9b)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_INT(currentFrame) < 0) currentFrameExt->pc += offset;
}

OPCODE("ifne", 0x9a)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_INT(currentFrame) != 0) currentFrameExt->pc += offset;
}

OPCODE("ifnonnull", 0xc7)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_OBJECT(currentFrame) != NULL) {
        currentFrameExt->pc += offset;
    }
}

OPCODE("ifnull", 0xc6)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    if (JEMCC_POP_STACK_OBJECT(currentFrame) == NULL) {
        currentFrameExt->pc += offset;
    }
}

OPCODE("if_acmpeq", 0xa5)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    JEMCC_Object *objb = JEMCC_POP_STACK_OBJECT(currentFrame);
    JEMCC_Object *obja = JEMCC_POP_STACK_OBJECT(currentFrame);
    if (obja == objb) currentFrameExt->pc += offset;
}

OPCODE("if_acmpne", 0xa6)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    JEMCC_Object *objb = JEMCC_POP_STACK_OBJECT(currentFrame);
    JEMCC_Object *obja = JEMCC_POP_STACK_OBJECT(currentFrame);
    if (obja != objb) currentFrameExt->pc += offset;
}

OPCODE("if_icmpeq", 0x9f)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    jint ib = JEMCC_POP_STACK_INT(currentFrame);
    jint ia = JEMCC_POP_STACK_INT(currentFrame);
    if (ia == ib) currentFrameExt->pc += offset;
}

OPCODE("if_icmpge", 0xa2)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    jint ib = JEMCC_POP_STACK_INT(currentFrame);
    jint ia = JEMCC_POP_STACK_INT(currentFrame);
    if (ia >= ib) currentFrameExt->pc += offset;
}

OPCODE("if_icmpgt", 0xa3)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    jint ib = JEMCC_POP_STACK_INT(currentFrame);
    jint ia = JEMCC_POP_STACK_INT(currentFrame);
    if (ia > ib) currentFrameExt->pc += offset;
}

OPCODE("if_icmple", 0xa4)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    jint ib = JEMCC_POP_STACK_INT(currentFrame);
    jint ia = JEMCC_POP_STACK_INT(currentFrame);
    if (ia <= ib) currentFrameExt->pc += offset;
}

OPCODE("if_icmplt", 0xa1)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    jint ib = JEMCC_POP_STACK_INT(currentFrame);
    jint ia = JEMCC_POP_STACK_INT(currentFrame);
    if (ia < ib) currentFrameExt->pc += offset;
}

OPCODE("if_icmpne", 0xa0)
{
    jshort offset = (jshort) read_op2(currentFrameExt) - 3;
    jint ib = JEMCC_POP_STACK_INT(currentFrame);
    jint ia = JEMCC_POP_STACK_INT(currentFrame);
    if (ia != ib) currentFrameExt->pc += offset;
}

OPCODE("iinc", 0x84)
{
    juint index = (juint) read_op1(currentFrameExt);
    jbyte adj = read_op1(currentFrameExt);
    currentFrame->localVars[index].i += adj;
}

OPCODE("iload", 0x15)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_PUSH_STACK_INT(currentFrame, 
                         JEMCC_LOAD_INT(currentFrame, index));
}

OPCODE("iload_0", 0x1a)
{
    JEMCC_PUSH_STACK_INT(currentFrame, JEMCC_LOAD_INT(currentFrame, 0));
}

OPCODE("iload_1", 0x1b)
{
    JEMCC_PUSH_STACK_INT(currentFrame, JEMCC_LOAD_INT(currentFrame, 1));
}

OPCODE("iload_2", 0x1c)
{
    JEMCC_PUSH_STACK_INT(currentFrame, JEMCC_LOAD_INT(currentFrame, 2));
}

OPCODE("iload_3", 0x1d)
{
    JEMCC_PUSH_STACK_INT(currentFrame, JEMCC_LOAD_INT(currentFrame, 3));
}

OPCODE("imul", 0x68)
{
    jint res = JEMCC_POP_STACK_INT(currentFrame);
    res = res * JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, res);
}

OPCODE("ineg", 0x74)
{
    jint res = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, -res);
}

OPCODE("instanceof", 0xc1)
{
    juint index = (juint) read_op2(currentFrameExt);
    JEMCC_Object *obj = JEMCC_POP_STACK_OBJECT(currentFrame);
    /* TODO - lots of complex rules appear here */
    (void) fprintf(stderr, "instanceof %p %i\n", obj, index);
}

OPCODE("invokeinterface", 0xb9)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassMethodData *ifMethodData, *targetMethodData;
    JEMCC_Object *targetObj;
    JEMCC_Class **assignClassPtr;
    JEM_ClassData *targetClassData;
    int i, methodIdx;
fprintf(stderr, "INVOKE INTERFACE INDEX %i\n", index);

    /* Discard the proprietary Java encoding areas */
    (void) read_op1(currentFrameExt); /* numArgs */
    (void) read_op1(currentFrameExt); /* reserved */

    ifMethodData = classData->classMethodRefs[index];
    if (ifMethodData == NULL) {
        /* TODO - VM ERROR */
    } else if ((ifMethodData->accessFlags & ACC_RESOLVE_ERROR) != 0) {
        /* TODO - compose and throw reference exception */
    } else {
        /* Need to resolve specific interface method by reference/index */
        targetObj = (currentFrame->operandStackTop - 
                                        ifMethodData->stackConsumeCount)->obj;
        if (targetObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                       NULL, NULL);
        } else {
JEM_DumpDebugClass(env, targetObj->classReference);
            targetClassData = targetObj->classReference->classData;
            /* Locate the interface assignment index */
            assignClassPtr = targetClassData->assignList + 1;
            for (i = 1; i <= targetClassData->assignmentCount; i++) {
                if (*assignClassPtr == ifMethodData->parentClass) break;
                assignClassPtr++;
            }
            if (i > targetClassData->assignmentCount) {
                JEMCC_ThrowStdThrowableIdx(env,
                                     JEMCC_Class_IncompatibleClassChangeError,
                                     NULL, "interface call on invalid object");
            } else {
                /* Locate the target method based on interface index mapping */
                targetMethodData = targetClassData->methodLinkTables[i]
                                                   [ifMethodData->methodIndex];
                if (targetMethodData == NULL) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                               JEMCC_Class_AbstractMethodError,
                                               NULL, ifMethodData->name);
                } else {
                    methodIdx = targetMethodData->methodIndex;
                    targetMethodData = 
                              targetClassData->methodLinkTables[0][methodIdx];

                    /* Push and execute the target method instance */
                    if (JEM_PushFrame(env, (jmethodID) targetMethodData,
                                                             NULL) == JNI_OK) {
                        JEM_ExecuteCurrentFrame(env, JNI_TRUE);
                    }
                }
            }
        }
    }
}

OPCODE("invokespecial", 0xb7)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassMethodData *methodData;
    JEMCC_Object *targetObj;

    methodData = classData->classMethodRefs[index];
    if (methodData == NULL) {
fprintf(stderr, "NULL SPECIAL METHOD DATA for %s(%i)\n", classData->className, index);
        /* TODO - VM ERROR */
    } else if ((methodData->accessFlags & ACC_RESOLVE_ERROR) != 0) {
fprintf(stderr, "ERROR SPECIAL METHOD DATA\n");
        /* TODO - compose and throw reference exception */
    } else {
        /* Not virtual, method is called directly.  But still do null-check */
        targetObj = (currentFrame->operandStackTop - 
                                       methodData->stackConsumeCount)->obj;
        if (targetObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                       NULL, NULL);
        } else {
            /* Make the method call */
            if (JEM_PushFrame(env, (jmethodID) methodData, NULL) == JNI_OK) {
                JEM_ExecuteCurrentFrame(env, JNI_TRUE);
            }
/* TODOTODOX - think about ACC_SUPER: method is not private, not <init>, target class is superclass and ACC_SUPER is set */
        }
    }
}

OPCODE("invokestatic", 0xb8)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassMethodData *methodData;

    (void) fprintf(stderr, "STATIC CALL %i\n", index);
    methodData = classData->classMethodRefs[index];
    if (methodData == NULL) {
fprintf(stderr, "NULL STATIC METHOD DATA\n");
        /* TODO - VM ERROR */
    } else if ((methodData->accessFlags & ACC_RESOLVE_ERROR) != 0) {
fprintf(stderr, "ERROR STATIC METHOD DATA\n");
        /* TODO - compose and throw reference exception */
    } else if (JEMCC_InitializeClass(env, methodData->parentClass) != JNI_OK) {
        /* Static initialization failed - exception is already thrown */
    } else {
        /* Static call is easy, just push and execute the method */
        if (JEM_PushFrame(env, (jmethodID) methodData, NULL) == JNI_OK) {
            JEM_ExecuteCurrentFrame(env, JNI_TRUE);
        }
    }
}

OPCODE("invokevirtual", 0xb6)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassMethodData *methodData, *targetMethodData;
    JEM_ClassData *targetClassData;
    JEMCC_Object *targetObj;
    int methodIdx;
fprintf(stderr, "INVOKE VIRTUAL INDEX %i\n", index);

    methodData = classData->classMethodRefs[index];
    if (methodData == NULL) {
        /* TODO - VM ERROR */
    } else if ((methodData->accessFlags & ACC_RESOLVE_ERROR) != 0) {
        /* TODO - compose and throw reference exception */
    } else {
        /* Virtual call, need to get the method by index from target object */
        methodIdx = methodData->methodIndex;
fprintf(stderr, "VIRTUAL METHOD INDEX(%s) %i\n", methodData->name, methodIdx);
        targetObj = (currentFrame->operandStackTop - 
                                       methodData->stackConsumeCount)->obj;
        if (targetObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                       NULL, NULL);
        } else {
/* TODO - handle constructor context here! */
fprintf(stderr, "Target Obj %p, Class %p\n", targetObj, targetObj->classReference);
            targetClassData = targetObj->classReference->classData;
            targetMethodData = targetClassData->methodLinkTables[0][methodIdx];
fprintf(stderr, "TARGET METHOD %s\n", targetMethodData->name);
            /* Push and execute the target method instance */
            if (JEM_PushFrame(env, (jmethodID) targetMethodData, 
                                                        NULL) == JNI_OK) {
                JEM_ExecuteCurrentFrame(env, JNI_TRUE);
            }
        }
    }
}

OPCODE("ior", 0x80)
{
    jint oval = JEMCC_POP_STACK_INT(currentFrame);
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, val | oval);
}

OPCODE("irem", 0x70)
{
    jint divisor = JEMCC_POP_STACK_INT(currentFrame);
    jint numerand = JEMCC_POP_STACK_INT(currentFrame);

    if (divisor == 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ArithmeticException, 
                                   NULL, NULL);
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, numerand % divisor);
    }
}

OPCODE("ireturn", 0xac)
{
    /* Grab the value from the working stack */
    jint retVal = JEMCC_POP_STACK_INT(currentFrame);

    /* Pop the frame instance, absorbing method arguments */
    JEM_PopFrame(env);

    /* Handle the return value according to the current frame type */
    currentFrameExt = ((JEM_JNIEnv *) env)->topFrame;
    switch (currentFrameExt->opFlags & FRAME_TYPE_MASK) {
        case FRAME_ROOT:
        case FRAME_NATIVE:
        case FRAME_JEMCC:
            ((JEM_JNIEnv *) env)->nativeReturnValue.intVal = retVal;
            break;
        case FRAME_BYTECODE:
            currentFrame = (JEMCC_VMFrame *) currentFrameExt;
            JEMCC_PUSH_STACK_INT(currentFrame, retVal);
            break;
    }
}

OPCODE("ishl", 0x78)
{
    jint shft = JEMCC_POP_STACK_INT(currentFrame);
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) (val << (shft & 0x1F)));
}

OPCODE("ishr", 0x7a)
{
    jint shft = JEMCC_POP_STACK_INT(currentFrame);
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    /* TODO - sign extend */
    JEMCC_PUSH_STACK_INT(currentFrame, val >> (shft & 0x1F));
}

OPCODE("istore", 0x36)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_STORE_INT(currentFrame, index, 
                    JEMCC_POP_STACK_INT(currentFrame));
}

OPCODE("istore_0", 0x3b)
{
    JEMCC_STORE_INT(currentFrame, 0, JEMCC_POP_STACK_INT(currentFrame));
}

OPCODE("istore_1", 0x3c)
{
    JEMCC_STORE_INT(currentFrame, 1, JEMCC_POP_STACK_INT(currentFrame));
}

OPCODE("istore_2", 0x3d)
{
    JEMCC_STORE_INT(currentFrame, 2, JEMCC_POP_STACK_INT(currentFrame));
}

OPCODE("istore_3", 0x3e)
{
    JEMCC_STORE_INT(currentFrame, 3, JEMCC_POP_STACK_INT(currentFrame));
}

OPCODE("isub", 0x64)
{
    jint sval = JEMCC_POP_STACK_INT(currentFrame);
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, val - sval);
}

OPCODE("iushr", 0x7c)
{
    jint shft = JEMCC_POP_STACK_INT(currentFrame);
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    /* TODO - don't sign extend */
    JEMCC_PUSH_STACK_INT(currentFrame, val >> (shft & 0x1F));
}

OPCODE("ixor", 0x82)
{
    jint xval = JEMCC_POP_STACK_INT(currentFrame);
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_PUSH_STACK_INT(currentFrame, val ^ xval);
}

OPCODE("jsr", 0xa8)
{
    jshort offset = (jshort) read_op2(currentFrameExt);
    JEMCC_PUSH_STACK_INT(currentFrame, currentFrameExt->pc);
    currentFrameExt->pc += offset - 1;
}

OPCODE("jsr_w", 0xc9)
{
    jint offset = (jint) read_op4(currentFrameExt);
    JEMCC_PUSH_STACK_INT(currentFrame, currentFrameExt->pc);
    currentFrameExt->pc += offset - 1;
}

OPCODE("l2d", 0x8a)
{
    jlong l = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, (jdouble) l);
}

OPCODE("l2f", 0x89)
{
    jlong l = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, (jfloat) l);
}

OPCODE("l2i", 0x88)
{
    jlong l = JEMCC_POP_STACK_LONG(currentFrame);
    jint i = (jint) (l & 0xFFFFFFFF);
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) i);
}

OPCODE("ladd", 0x61)
{
    jlong res = JEMCC_POP_STACK_LONG(currentFrame);
    res = res + JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, res);
}

OPCODE("land", 0x7f)
{
    jlong aval = JEMCC_POP_STACK_LONG(currentFrame);
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, val & aval);
}

OPCODE("laload", 0x2f)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_LONG(currentFrame, 
                          *(((jlong *) array->arrayData) + index));
        }
    }
}

OPCODE("lastore", 0x50)
{
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((jlong *) array->arrayData) + index) = val;
        }
    }
}

OPCODE("lcmp", 0x94)
{
    jlong lb = JEMCC_POP_STACK_LONG(currentFrame);
    jlong la = JEMCC_POP_STACK_LONG(currentFrame);
    if (la > lb) {
        JEMCC_PUSH_STACK_INT(currentFrame, 1);
    } else if (la == lb) {
        JEMCC_PUSH_STACK_INT(currentFrame, 0);
    } else {
        JEMCC_PUSH_STACK_INT(currentFrame, -1);
    }
}

OPCODE("lconst_0", 0x09)
{
    JEMCC_PUSH_STACK_LONG(currentFrame, 0);
}

OPCODE("lconst_1", 0x0a)
{
    JEMCC_PUSH_STACK_LONG(currentFrame, 1);
}

OPCODE("ldc", 0x12)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op1(currentFrameExt);

    switch (classData->localConstants[index].generic.tag) {
        case CONSTANT_Integer:
            JEMCC_PUSH_STACK_INT(currentFrame, 
                      classData->localConstants[index].integer_const.value);
            break;
        case CONSTANT_Float:
            JEMCC_PUSH_STACK_FLOAT(currentFrame, 
                      classData->localConstants[index].float_const.value);
            break;
        case CONSTANT_String:
            JEMCC_PUSH_STACK_OBJECT(currentFrame, 
                      classData->localConstants[index].string_const.stringRef);
            break;
        default:
            /* TODO - throw VM internal error */
            break;
    }
}

OPCODE("ldc_w", 0x13)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);

    switch (classData->localConstants[index].generic.tag) {
        case CONSTANT_Integer:
            JEMCC_PUSH_STACK_INT(currentFrame, 
                      classData->localConstants[index].integer_const.value);
            break;
        case CONSTANT_Float:
            JEMCC_PUSH_STACK_FLOAT(currentFrame, 
                      classData->localConstants[index].float_const.value);
            break;
        case CONSTANT_String:
            JEMCC_PUSH_STACK_OBJECT(currentFrame, 
                      classData->localConstants[index].string_const.stringRef);
            break;
        default:
            /* TODO - throw VM internal error */
            break;
    }
}

OPCODE("ldc2_w", 0x14)
{
    JEM_ClassData *classData = 
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);

    switch (classData->localConstants[index].generic.tag) {
        case CONSTANT_Long:
fprintf(stderr, "LONG CONSTANT %lli\n",  classData->localConstants[index].long_const.value);
            JEMCC_PUSH_STACK_LONG(currentFrame, 
                          classData->localConstants[index].long_const.value);
            break;
        case CONSTANT_Double:
            JEMCC_PUSH_STACK_DOUBLE(currentFrame, 
                          classData->localConstants[index].double_const.value);
            break;
        default:
            /* TODO - throw VM internal error */
            break;
    }
}

OPCODE("ldiv", 0x6d)
{
    jlong divisor = JEMCC_POP_STACK_LONG(currentFrame);
    jlong numerand = JEMCC_POP_STACK_LONG(currentFrame);

    if (divisor == 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ArithmeticException, 
                                   NULL, NULL);
    } else {
        JEMCC_PUSH_STACK_LONG(currentFrame, numerand / divisor);
    }
}

OPCODE("lload", 0x16)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_PUSH_STACK_LONG(currentFrame, 
                          JEMCC_LOAD_LONG(currentFrame, index));
}

OPCODE("lload_0", 0x1e)
{
    JEMCC_PUSH_STACK_LONG(currentFrame, JEMCC_LOAD_LONG(currentFrame, 0));
}

OPCODE("lload_1", 0x1f)
{
    JEMCC_PUSH_STACK_LONG(currentFrame, JEMCC_LOAD_LONG(currentFrame, 1));
}

OPCODE("lload_2", 0x20)
{
    JEMCC_PUSH_STACK_LONG(currentFrame, JEMCC_LOAD_LONG(currentFrame, 2));
}

OPCODE("lload_3", 0x21)
{
    JEMCC_PUSH_STACK_LONG(currentFrame, JEMCC_LOAD_LONG(currentFrame, 3));
}

OPCODE("lmul", 0x69)
{
    jlong res = JEMCC_POP_STACK_LONG(currentFrame);
    res = res * JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, res);
}

OPCODE("lneg", 0x75)
{
    jlong res = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, -res);
}

OPCODE("lookupswitch", 0xab)
{
    jint switchValue = JEMCC_POP_STACK_INT(currentFrame);
    jint basePC = currentFrameExt->pc - 1;
    jint i, switchOffset, caseCount, caseValue, caseOffset;

    /* Adjust pad to arrive at four byte offset */
    currentFrameExt->pc = ((currentFrameExt->pc + 3) >> 2) << 2;

    /* Read the default lookup offset */
    switchOffset = (jint) read_op4(currentFrameExt);

    /* Scan through the match table looking for our case value */
    caseCount = (jint) read_op4(currentFrameExt);
    for (i = 0; i < caseCount; i++) {
        caseValue = (jint) read_op4(currentFrameExt);
        caseOffset = (jint) read_op4(currentFrameExt);
        if (switchValue == caseValue) {
            switchOffset = caseOffset;
            break;
        }
    }

    /* Jump to the final target address */
    currentFrameExt->pc = basePC + switchOffset;
}

OPCODE("lor", 0x81)
{
    jlong oval = JEMCC_POP_STACK_LONG(currentFrame);
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, val | oval);
}

OPCODE("lrem", 0x71)
{
    jlong divisor = JEMCC_POP_STACK_LONG(currentFrame);
    jlong numerand = JEMCC_POP_STACK_LONG(currentFrame);

    if (divisor == 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ArithmeticException, 
                                   NULL, NULL);
    } else {
        JEMCC_PUSH_STACK_LONG(currentFrame, numerand % divisor);
    }
}

OPCODE("lreturn", 0xad)
{
    /* Grab the value from the working stack */
    jlong retVal = JEMCC_POP_STACK_LONG(currentFrame);

    /* Pop the frame instance, absorbing method arguments */
    JEM_PopFrame(env);

    /* Handle the return value according to the current frame type */
    currentFrameExt = ((JEM_JNIEnv *) env)->topFrame;
    switch (currentFrameExt->opFlags & FRAME_TYPE_MASK) {
        case FRAME_ROOT:
        case FRAME_NATIVE:
        case FRAME_JEMCC:
            ((JEM_JNIEnv *) env)->nativeReturnValue.longVal = retVal;
            break;
        case FRAME_BYTECODE:
            currentFrame = (JEMCC_VMFrame *) currentFrameExt;
            JEMCC_PUSH_STACK_LONG(currentFrame, retVal);
            break;
    }
}

OPCODE("lshl", 0x79)
{
    jint shft = JEMCC_POP_STACK_INT(currentFrame);
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, (jlong) (val << (shft & 0x3F)));
}

OPCODE("lshr", 0x7b)
{
    jint shft = JEMCC_POP_STACK_INT(currentFrame);
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    /* TODO - sign extend */
    JEMCC_PUSH_STACK_LONG(currentFrame, val >> (shft & 0x3F));
}

OPCODE("lstore", 0x37)
{
    juint index = (juint) read_op1(currentFrameExt);
    JEMCC_STORE_LONG(currentFrame, index, 
                     JEMCC_POP_STACK_LONG(currentFrame));
}

OPCODE("lstore_0", 0x3f)
{
    JEMCC_STORE_LONG(currentFrame, 0, JEMCC_POP_STACK_LONG(currentFrame));
}

OPCODE("lstore_1", 0x40)
{
    JEMCC_STORE_LONG(currentFrame, 1, JEMCC_POP_STACK_LONG(currentFrame));
}

OPCODE("lstore_2", 0x41)
{
    JEMCC_STORE_LONG(currentFrame, 2, JEMCC_POP_STACK_LONG(currentFrame));
}

OPCODE("lstore_3", 0x42)
{
    JEMCC_STORE_LONG(currentFrame, 3, JEMCC_POP_STACK_LONG(currentFrame));
}

OPCODE("lsub", 0x65)
{
    jlong sval = JEMCC_POP_STACK_LONG(currentFrame);
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, val - sval);
}

OPCODE("lushr", 0x7d)
{
    jint shft = JEMCC_POP_STACK_INT(currentFrame);
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    /* TODO - no sign extend */
    JEMCC_PUSH_STACK_LONG(currentFrame, val >> (shft & 0x3F));
}

OPCODE("lxor", 0x83)
{
    jlong xval = JEMCC_POP_STACK_LONG(currentFrame);
    jlong val = JEMCC_POP_STACK_LONG(currentFrame);
    JEMCC_PUSH_STACK_LONG(currentFrame, val ^ xval);
}

OPCODE("monitorenter", 0xc2)
{
    JEMCC_Object *obj = JEMCC_POP_STACK_OBJECT(currentFrame);
    if (obj == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        JEMCC_EnterObjMonitor(env, (jobject) obj);
    }
}

OPCODE("monitorexit", 0xc3)
{
    JEMCC_Object *obj = JEMCC_POP_STACK_OBJECT(currentFrame);
    if (obj == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        JEMCC_ExitObjMonitor(env, (jobject) obj);
    }
}

OPCODE("multinewarray", 0xc5)
{
    juint index = (juint) read_op2(currentFrameExt);
    juint dimensionCount = (juint) read_op1(currentFrameExt);
    jint i, dimension;

    /* Get and resolve class definition of array to create */
    (void) fprintf(stderr, "Get class for index %i\n", index);

    /* Create the multiple array levels according to stack sizes */
    for (i = 0; i < dimensionCount; i++) {
        dimension = JEMCC_POP_STACK_INT(currentFrame);
        /* TODO - do it! */
    }
}

OPCODE("new", 0xbb)
{
    JEM_ClassData *classData =
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEMCC_Object *newObject;
    JEMCC_Class *newClass;

    newClass = classData->classRefs[index];
    if (newClass == NULL) {
        /* TODO - need to handle failed class references */
    } else {
        newObject = JEMCC_AllocateObject(env, newClass, 0);
        if (newObject != NULL) {
fprintf(stderr, "OBJECT CREATED %p\n", newObject);
            JEMCC_PUSH_STACK_OBJECT(currentFrame, newObject);
        }
    }
}

OPCODE("newarray", 0xbc)
{
    jint type = read_op1(currentFrameExt);
    jsize size = (jsize) JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_Object *array = NULL;

    /* Determine the primitive type from yet another index map */
    switch (type) {
        case 4:
            array = (JEMCC_Object *) JEMCC_NewBooleanArray(env, size);
            break;
        case 5:
            array = (JEMCC_Object *) JEMCC_NewCharArray(env, size);
            break;
        case 6:
            array = (JEMCC_Object *) JEMCC_NewFloatArray(env, size);
            break;
        case 7:
            array = (JEMCC_Object *) JEMCC_NewDoubleArray(env, size);
            break;
        case 8:
            array = (JEMCC_Object *) JEMCC_NewByteArray(env, size);
            break;
        case 9:
            array = (JEMCC_Object *) JEMCC_NewShortArray(env, size);
            break;
        case 10:
            array = (JEMCC_Object *) JEMCC_NewIntArray(env, size);
            break;
        case 11:
            array = (JEMCC_Object *) JEMCC_NewLongArray(env, size);
            break;
    }
    if (array != NULL) JEMCC_PUSH_STACK_OBJECT(currentFrame, array);
}

OPCODE("nop", 0x0)
{
    /* Well, this is really difficult to implement :) */
}

OPCODE("pop", 0x57)
{
    (currentFrame->operandStackTop)--;
}

OPCODE("pop2", 0x58)
{
    ((JEM_DblFrameEntry *) currentFrame->operandStackTop)--;
}

OPCODE("putfield", 0xb5)
{
    JEM_ClassData *classData =
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassFieldData *fieldRef = classData->classFieldRefs[index];
    JEMCC_Object *targObj;
    jbyte *basePtr;

    /* NOTE: static test is already performed during verification */
    if ((fieldRef->accessFlags & ACC_RESOLVE_ERROR) != 0) {
        /* TODO - throw the resolution error */
    } else {
        /* First, extract the value based on the reference type */
        JEMCC_ReturnValue val;
        switch (fieldRef->descriptor->generic.tag) {
            case BASETYPE_Boolean:
            case BASETYPE_Byte:
            case BASETYPE_Char:
            case BASETYPE_Int:
            case BASETYPE_Short:
                JEMCC_DEBUG_STACK_INT(currentFrame);
                val.intVal = JEMCC_POP_STACK_INT(currentFrame);
                break;
            case BASETYPE_Double:
                JEMCC_DEBUG_STACK_DOUBLE(currentFrame);
                val.dblVal = JEMCC_POP_STACK_DOUBLE(currentFrame);
                break;
            case BASETYPE_Float:
                JEMCC_DEBUG_STACK_FLOAT(currentFrame);
                val.fltVal = JEMCC_POP_STACK_FLOAT(currentFrame);
                break;
            case BASETYPE_Long:
                JEMCC_DEBUG_STACK_LONG(currentFrame);
                val.longVal = JEMCC_POP_STACK_LONG(currentFrame);
                break;
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                JEMCC_DEBUG_STACK_OBJECT(currentFrame);
                val.objVal = JEMCC_POP_STACK_OBJECT(currentFrame);
                break;
            default:
                /* Do nothing, exception will be thrown shortly */
                break;
        }

        /* Snare the object into which the value is stored */
        targObj = JEMCC_POP_STACK_OBJECT(currentFrame);
        if (targObj == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                       NULL, NULL);
        } else {
#ifdef DEBUG_CPU_INTERNALS
(void) fprintf(stderr, "Storing field to object %s\n",
                       targObj->classReference->classData->className);
(void) fprintf(stderr, "Field definition %s::%s [%i]\n",
                       fieldRef->name,
                       fieldRef->parentClass->classData->className,
                       fieldRef->fieldOffset);
#endif
            /* And store the instance value */
            basePtr = ((jubyte *) &(((JEMCC_ObjectExt *) 
                                               targObj)->objectData)) + 
                                                         fieldRef->fieldOffset;
            switch (fieldRef->descriptor->generic.tag) {
                case BASETYPE_Boolean:
                case BASETYPE_Byte:
fprintf(stderr, "TARGET %p offset %i\n", targObj, fieldRef->fieldOffset);
                    *((jbyte *) basePtr) = (jbyte) (val.intVal & 0xFF);
                    break;
                case BASETYPE_Char:
                    *((jchar *) basePtr) = (jchar) (val.intVal & 0xFFFF);
                    break;
                case BASETYPE_Double:
                    *((jdouble *) basePtr) = val.dblVal;
                    break;
                case BASETYPE_Float:
                    *((jfloat *) basePtr) = val.fltVal;
                    break;
                case BASETYPE_Int:
                    *((jint *) basePtr) = val.intVal;
                    break;
                case BASETYPE_Long:
                    *((jlong *) basePtr) = val.longVal;
                    break;
                case BASETYPE_Short:
                    *((jshort *) basePtr) = (jshort) (val.intVal & 0xFFFF);
                    break;
                case DESCRIPTOR_ObjectType:
                case DESCRIPTOR_ArrayType:
                    *((JEMCC_Object **) basePtr) = val.objVal;
                    break;
                default:
                    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError, 
                                           NULL, "Unknown putfield data type");
                    break;
            }
        }
    }
}

OPCODE("putstatic", 0xb3)
{
    JEM_ClassData *classData =
                      currentFrameExt->currentMethod->parentClass->classData;
    juint index = (juint) read_op2(currentFrameExt);
    JEM_ClassFieldData *fieldRef = classData->classFieldRefs[index];
    jbyte *staticData;

    /* NOTE: static test is already performed during verification */
    if ((fieldRef->accessFlags & ACC_RESOLVE_ERROR) != 0) {
        /* TODO - throw the resolution error */
    } else if (JEMCC_InitializeClass(env, fieldRef->parentClass) != JNI_OK) {
        /* Static initialization failed - exception is already thrown */
    } else {
        /* Yes, this could call JEM_SetStatic, but speed is better */
        staticData = ((jbyte *) fieldRef->parentClass->staticData) +
                                                       fieldRef->fieldOffset;
        switch (fieldRef->descriptor->generic.tag) {
            case BASETYPE_Boolean:
            case BASETYPE_Byte:
                JEMCC_DEBUG_STACK_INT(currentFrame);
                *((jbyte *) staticData) = 
                        (jbyte) (JEMCC_POP_STACK_INT(currentFrame) & 0xFF);
                break;
            case BASETYPE_Char:
                JEMCC_DEBUG_STACK_INT(currentFrame);
                *((jchar *) staticData) = 
                        (jchar) (JEMCC_POP_STACK_INT(currentFrame) & 0xFFFF);
                break;
            case BASETYPE_Double:
                JEMCC_DEBUG_STACK_DOUBLE(currentFrame);
                *((jdouble *) staticData) = 
                        JEMCC_POP_STACK_DOUBLE(currentFrame);
                break;
            case BASETYPE_Float:
                JEMCC_DEBUG_STACK_FLOAT(currentFrame);
                *((jfloat *) staticData) =
                        JEMCC_POP_STACK_FLOAT(currentFrame);
                break;
            case BASETYPE_Int:
                JEMCC_DEBUG_STACK_INT(currentFrame);
                *((jint *) staticData) =
                        JEMCC_POP_STACK_INT(currentFrame);
                break;
            case BASETYPE_Long:
                JEMCC_DEBUG_STACK_LONG(currentFrame);
                *((jlong *) staticData) =
                        JEMCC_POP_STACK_LONG(currentFrame);
                break;
            case BASETYPE_Short:
                JEMCC_DEBUG_STACK_INT(currentFrame);
                *((jshort *) staticData) = 
                        (jshort) (JEMCC_POP_STACK_INT(currentFrame) & 0xFFFF);
                break;
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                JEMCC_DEBUG_STACK_OBJECT(currentFrame);
                *((JEMCC_Object **) staticData) =
                        JEMCC_POP_STACK_OBJECT(currentFrame);
                break;
            default:
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError, 
                                           NULL, "Unknown putstatic data type");
                break;
        }
    }
}

OPCODE("ret", 0xa9)
{
    juint index = (juint) read_op1(currentFrameExt);
    currentFrameExt->pc = JEMCC_LOAD_INT(currentFrame, index);
}

OPCODE("return", 0xb1)
{
    /* delete the current frame (and contained words) */
    /* if synchronized, release monitor */
    /* make invoker frame current and continue execution */
    JEM_PopFrame(env);
}

OPCODE("saload", 0x35)
{
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            JEMCC_PUSH_STACK_INT(currentFrame, 
                          (int) *(((jshort *) array->arrayData) + index));
        }
    }
}

OPCODE("sastore", 0x56)
{
    jint val = JEMCC_POP_STACK_INT(currentFrame);
    jint index = JEMCC_POP_STACK_INT(currentFrame);
    JEMCC_ArrayObject *array = 
               (JEMCC_ArrayObject *) JEMCC_POP_STACK_OBJECT(currentFrame);
    if (array == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_NullPointerException, 
                                   NULL, NULL);
    } else {
        if (JEMCC_CheckArrayLimits(env, array, index, -1) == JNI_OK) {
            *(((jshort *) array->arrayData) + index) = 
                                                     (jshort) (val & 0xFFFF);
        }
    }
}

OPCODE("sipush", 0x11)
{
    jshort res = (jshort) read_op2(currentFrameExt);
    JEMCC_PUSH_STACK_INT(currentFrame, (jint) res);
}

OPCODE("swap", 0x5f)
{
    JEM_FrameEntry swapVal;

    swapVal = *(currentFrame->operandStackTop - 1);
    *(currentFrame->operandStackTop - 1) = *(currentFrame->operandStackTop - 2);
    *(currentFrame->operandStackTop - 1) = swapVal;
}

OPCODE("tableswitch", 0xaa)
{
    jint switchIndex = JEMCC_POP_STACK_INT(currentFrame);
    jint basePC = currentFrameExt->pc - 1;
    jint offset, lowIndex, highIndex;

    /* Adjust pad to arrive at four byte offset */
    currentFrameExt->pc = ((currentFrameExt->pc + 3) >> 2) << 2;

    /* Read the default offset and low/high index marks */
    offset = (jint) read_op4(currentFrameExt);
    lowIndex = (jint) read_op4(currentFrameExt);
    highIndex = (jint) read_op4(currentFrameExt);

    /* Go to default offset if index out of range, read table otherwise */
    if ((switchIndex < lowIndex) || (switchIndex > highIndex)) {
        currentFrameExt->pc = basePC + offset;
    } else {
        currentFrameExt->pc = currentFrameExt->pc + 
                                          4 * (switchIndex - lowIndex);
        offset = (jint) read_op4(currentFrameExt);
        currentFrameExt->pc = basePC + offset;
    }
}

OPCODE("wide", 0xc4)
{
    /* Grab the contained opcode and the expanded local variable index */
    jubyte wideOpCode = read_op1(currentFrameExt);
    juint index = (juint) read_op2(currentFrameExt);

    /* Perform the opcode based handling of the wide index */
    switch (wideOpCode) {
        default:
            /* TODO - Ack Barf! */
        OPCODE("aload", 0x19)
            {
                JEMCC_PUSH_STACK_OBJECT(currentFrame, 
                                        JEMCC_LOAD_OBJECT(currentFrame, index));
            }
        OPCODE("astore", 0x3a)
            {
                JEMCC_STORE_OBJECT(currentFrame, index, 
                                   JEMCC_POP_STACK_OBJECT(currentFrame));
            }
        OPCODE("dload", 0x18)
            {
                JEMCC_PUSH_STACK_DOUBLE(currentFrame, 
                                        JEMCC_LOAD_DOUBLE(currentFrame, index));
            }
        OPCODE("dstore", 0x39)
            {
                JEMCC_STORE_DOUBLE(currentFrame, index, 
                                   JEMCC_POP_STACK_DOUBLE(currentFrame));
            }
        OPCODE("fload", 0x17)
            {
                JEMCC_PUSH_STACK_FLOAT(currentFrame, 
                                       JEMCC_LOAD_FLOAT(currentFrame, index));
            }
        OPCODE("fstore", 0x38)
            {
                JEMCC_STORE_FLOAT(currentFrame, index, 
                                  JEMCC_POP_STACK_FLOAT(currentFrame));
            }
        OPCODE("iinc", 0x84)
            {
                jshort adj = (jshort) read_op2(currentFrameExt);
                currentFrame->localVars[index].i += adj;
            }
        OPCODE("iload", 0x15)
            {
                JEMCC_PUSH_STACK_INT(currentFrame, 
                                     JEMCC_LOAD_INT(currentFrame, index));
            }
        OPCODE("istore", 0x36)
            {
                JEMCC_STORE_INT(currentFrame, index, 
                                JEMCC_POP_STACK_INT(currentFrame));
            }
        OPCODE("lload", 0x16)
            {
                JEMCC_PUSH_STACK_LONG(currentFrame, 
                                      JEMCC_LOAD_LONG(currentFrame, index));
            }
        OPCODE("lstore", 0x37)
            {
                JEMCC_STORE_LONG(currentFrame, index, 
                                 JEMCC_POP_STACK_LONG(currentFrame));
            }
        OPCODE("ret", 0xa9)
            {
                currentFrameExt->pc = JEMCC_LOAD_INT(currentFrame, index);
            }
       OPCODE("dummy", 0xff) 
            {
               /* This will never be called, just here for debugging */
            }
            break;
    }
}

OPCODE("dummy", 0xff) 
{
    /* This will never be called, just here for debugging */
}
