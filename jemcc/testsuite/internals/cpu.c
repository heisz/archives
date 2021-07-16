/**
 * JEMCC test program to test the frame/bytecode processor.
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
#include "jnifunc.h"

/* External elements from uvminit.c */
extern JNIEnv *createTestEnv();
extern jint initializeTestCoreClasses(JNIEnv *env);
extern void destroyTestEnv(JNIEnv *env);
extern JEMCC_Class *JEMCCTestClass_String;

/* Test flags/data values for condition setups and tests */
int failureTotal;
#ifdef ENABLE_ERRORSWEEP
int testFailureCurrentCount, testFailureCount;

int JEM_CheckErrorSweep(int sweepType) {
    /* Note: don't care about sweep type */
    testFailureCurrentCount++;
    if ((testFailureCount >= 0) &&
        (testFailureCurrentCount == testFailureCount)) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}
#endif

/*
 * Different implementation from more basic tests.  This disassembles
 * the pendingException for checking and clears it as well.
 */
void destroyException(JNIEnv *env, JEMCC_Object *exception) {
    JEMCC_ThrowableData *exData = (JEMCC_ThrowableData *)
                             &(((JEMCC_ObjectExt *) exception)->objectData);

    if (exData->message != NULL) {
         JEMCC_Free(((JEMCC_ObjectExt *) (exData->message))->objectData);
         JEMCC_Free(exData->message);
    }
    JEMCC_Free(exception);
}

void checkException(JNIEnv *env, const char *checkClassName, 
                    const char *checkMsg, const char *tstName) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEMCC_ThrowableData *exData = NULL;
    JEMCC_StringData *strData = NULL;
    char *msg = NULL, *className;;

    if ((checkClassName == NULL) && (checkMsg == NULL)) {
        if (jenv->pendingException != NULL) {
            (void) fprintf(stderr, "Unexpected exception thrown for %s\n",
                                   tstName);
            exit(1);
        }
    }
    if (jenv->pendingException == NULL) {
        (void) fprintf(stderr, "Expected exception not thrown for %s\n",
                               tstName);
        exit(1);
    }

    /* Compare with the string internals */
    exData = (JEMCC_ThrowableData *) 
                   &(((JEMCC_ObjectExt *) jenv->pendingException)->objectData);
    if (exData->message == NULL) {
        msg = "(null)";
    } else {
        strData = (JEMCC_StringData *)
                    ((JEMCC_ObjectExt *) (exData->message))->objectData;
        msg = &(strData->data);
    }
    if ((checkMsg != NULL) && (strstr(msg, checkMsg) == NULL)) {
        (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
        (void) fprintf(stderr, "Expected %s, got %s instead.\n",
                               checkMsg, msg);
        exit(1);
    }
    if (checkClassName != NULL) {
        className = 
                jenv->pendingException->classReference->classData->className;
        if (strstr(className, checkClassName) == NULL) {
            (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
            (void) fprintf(stderr, "Expected '%s', got '%s' instead.\n",
                                   checkClassName, className);
            exit(1);
        }
    }

    /* Perform some cleanup */
    destroyException(env, jenv->pendingException);
    jenv->pendingException = NULL;
}

#define VOID_RETURN 0
#define OBJ_RETURN 1
#define INT_RETURN 2
#define FLOAT_RETURN 3
#define LONG_RETURN 4
#define DOUBLE_RETURN 5
#define EXCEPTION_RETURN 6

#define OBJ_INPUT 1
#define OBJ_ARRAY_INPUT 2
#define INT_INPUT 10
#define BYTE_ARRAY_INPUT 11
#define CHAR_ARRAY_INPUT 12
#define SHORT_ARRAY_INPUT 13
#define FLOAT_INPUT 20
#define LONG_INPUT 30
#define DOUBLE_INPUT 40

static struct code_test_data {
    jbyte codeData[1000];
    int inputType;
    int returnType;
    int returnVal;
    char *exceptionStr;
} codeSlices[] = {
   { {0x01 /* aconst_null */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0, NULL },
   { {0x2a /* aload_0 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0xdead, NULL },
   { {0x2b /* aload_1 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0xbeef, NULL },
   { {0x2c /* aload_2 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0xcafe, NULL },
   { {0x2d /* aload_3 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0xbabe, NULL },
   { {0x19, 0x03 /* aload 3 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0xbabe, NULL },
   { {0x01 /* aconst_null */, 
      0x4b /* astore_0 */,
      0x2a /* aload_0 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0, NULL },
   { {0x2a /* aload_0 */, 
      0x4c /* astore_1 */,
      0x2b /* aload_1 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0xdead, NULL },
   { {0x01 /* aconst_null */, 
      0x4d /* astore_2 */,
      0x2c /* aload_2 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0, NULL },
   { {0x01 /* aconst_null */, 
      0x4e /* astore_3 */,
      0x2d /* aload_3 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0, NULL }, /**** 10 ****/
   { {0x01 /* aconst_null */, 
      0x3a, 0x02 /* astore 2 */,
      0x2c /* aload 2 */, 
      0xb0 /* areturn */}, 
     OBJ_INPUT, OBJ_RETURN, 0, NULL },
   { {0x01 /* aconst_null */, 
      0x1d /* iload_0 */, 
      0x32 /* aaload */, 
      0xb0 /* areturn */}, 
     OBJ_ARRAY_INPUT, EXCEPTION_RETURN, 0, "(null)" }, 
   { {0x2c /* aload_2 */, 
      0x06 /* iconst_3 */,
      0x74 /* ineg */, 
      0x32 /* aaload */, 
      0xb0 /* areturn */}, 
     OBJ_ARRAY_INPUT, EXCEPTION_RETURN, 0, "index out of range" }, 
   { {0x2c /* aload_2 */, 
      0x08 /* iconst_5 */,
      0x32 /* aaload */, 
      0xb0 /* areturn */}, 
     OBJ_ARRAY_INPUT, EXCEPTION_RETURN, 0, "index out of range" }, 
   { {0x2c /* aload_2 */, 
      0x04 /* iconst_1 */,
      0x32 /* aaload */, 
      0xb0 /* areturn */}, 
     OBJ_ARRAY_INPUT, OBJ_RETURN, (int) NULL, NULL },
   { {0x01 /* aconst_null */, 
      0x1d /* iload_0 */, 
      0x01 /* aconst_null */,
      0x53 /* aastore */, 
      0xb1 /* return */}, 
     OBJ_ARRAY_INPUT, EXCEPTION_RETURN, 0, "(null)" }, 
   { {0x2c /* aload_2 */, 
      0x06 /* iconst_3 */,
      0x74 /* ineg */, 
      0x01 /* aconst_null */,
      0x53 /* aastore */, 
      0xb1 /* return */}, 
     OBJ_ARRAY_INPUT, EXCEPTION_RETURN, 0, "index out of range" }, 
   { {0x2c /* aload_2 */, 
      0x08 /* iconst_5 */,
      0x01 /* aconst_null */,
      0x53 /* aastore */, 
      0xb1 /* return */}, 
     OBJ_ARRAY_INPUT, EXCEPTION_RETURN, 0, "index out of range" }, 
   { {0x2c /* aload_2 */, 
      0x04 /* iconst_1 */,
      0x01 /* aconst_null */,
      0x53 /* aastore */, 
      0x2c /* aload_2 */, 
      0x04 /* iconst_1 */,
      0x32 /* aaload */, 
      0xb0 /* areturn */}, 
     OBJ_ARRAY_INPUT, OBJ_RETURN, (int) NULL, NULL },
   { {0x01 /* aconst_null */, 
      0xbe /* arraylength */, 
      0xb1 /* return */}, 
     OBJ_ARRAY_INPUT, EXCEPTION_RETURN, 0, "(null)" }, /**** 20 ****/
   { {0x2c /* aload_2 */, 
      0xbe /* arraylength */, 
      0xac /* ireturn */}, 
     OBJ_ARRAY_INPUT, INT_RETURN, 2, NULL },
   { {0x01 /* aconst_null */, 
      0x04 /* iconst_1 */, 
      0x33 /* baload */, 
      0xb1 /* return */}, 
     BYTE_ARRAY_INPUT, EXCEPTION_RETURN, 0, "(null)" }, 
   { {0x26 /* dload_0 */, 0x28 /* dload_2 */,
      0x63 /* dadd */, 0xaf /* dreturn */}, 
     DOUBLE_INPUT, DOUBLE_RETURN, 2, NULL },
   { {0x26 /* dload_0 */, 0x28 /* dload_2 */,
      0x98 /* dcmpg */, 0xac /* ireturn */}, 
     DOUBLE_INPUT, INT_RETURN, -1, NULL },
   { {0x28 /* dload_2 */, 0x26 /* dload_0 */,
      0x98 /* dcmpg */, 0xac /* ireturn */}, 
     DOUBLE_INPUT, INT_RETURN, 1, NULL },
   { {0x28 /* dload_2 */, 0x28 /* dload_2 */,
      0x98 /* dcmpg */, 0xac /* ireturn */}, 
     DOUBLE_INPUT, INT_RETURN, 0, NULL },
   { {0x26 /* dload_0 */, 0x28 /* dload_2 */,
      0x97 /* dcmpl */, 0xac /* ireturn */}, 
     DOUBLE_INPUT, INT_RETURN, -1, NULL },
   { {0x28 /* dload_2 */, 0x26 /* dload_0 */,
      0x97 /* dcmpl */, 0xac /* ireturn */}, 
     DOUBLE_INPUT, INT_RETURN, 1, NULL },
   { {0x28 /* dload_2 */, 0x28 /* dload_2 */,
      0x97 /* dcmpl */, 0xac /* ireturn */}, 
     DOUBLE_INPUT, INT_RETURN, 0, NULL },
   { {0x0e /* dconst_0 */, 0x0f /* dconst_1 */,
      0x6f /* ddiv */, 0xaf /* dreturn */}, 
     DOUBLE_INPUT, DOUBLE_RETURN, 0, NULL }, /**** 30 ****/
   { {0x18, 0x02 /* dload 2 */, 0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, 2, NULL },
   { {0x0f /* dconst_1 */, 0x48 /* dstore_1 */,
      0x27 /* dload_1 */, 0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, 1, NULL },
   { {0x26 /* dload_0 */, 0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, 0, NULL },
   { {0x0f /* dconst_1 */, 0x77 /* dneg */,
      0x39, 0x03 /* dstore 3 */, 0x29 /* dload_3 */,
      0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, -1, NULL },
   { {0x0f /* dconst_1 */, 0x77 /* dneg */,
      0x28 /* dload_2 */, 0x6b /* dmul */,
      0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, -2, NULL },
   { {0x0f /* dconst_1 */, 0x47 /* dstore_0 */,
      0x0f /* dconst_1 */, 0x49 /* dstore_2 */,
      0x28 /* dload_2 */, 0x26 /* dload_0 */,
      0x6b /* dmul */, 0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, 1, NULL },
   { {0x0f /* dconst_1 */, 0x4a /* dstore_3 */,
      0x29 /* dload_3 */, 0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, 1, NULL },
   { {0x0f /* dconst_1 */, 0x0f /* dconst_1 */,
      0x0f /* dconst_1 */, 0x63 /* dadd */,
      0x63 /* dadd */, 0x28 /* dload_2 */,
      0x73 /* drem */, 0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, 1, NULL },
   { {0x28 /* dload_2 */, 0x0f /* dconst_1 */,
      0x67 /* dsub */, 0xaf /* dreturn */},
     DOUBLE_INPUT, DOUBLE_RETURN, 1, NULL },

   { {0x10, 0x20 /* bipush 32 */, 0xac /* ireturn */}, 
     INT_INPUT, INT_RETURN, 32, NULL }, /**** 40 ****/
   { {0x04 /* iconst_1 */, 0xac /* ireturn */}, 
     INT_INPUT, INT_RETURN, 1, NULL },
   { {0x06 /* iconst_3 */, 0x04 /* iconst_1 */, 
      0x64 /* isub */, 0xac /* ireturn */}, 
     INT_INPUT, INT_RETURN, 2, NULL },

   /* XXX - need complex int/long->float/double conversions for
            special numerical representations */
};

/* Main program will send the CPU opcodes through their paces */
int main(int argc, char *argv[]) {
    JEMCC_VMFrame *currentFrame;
    int i, nTestBlocks = sizeof(codeSlices) / sizeof(struct code_test_data);
    JEM_ClassMethodData method;
    JEM_BCMethod bcMethod;
    JEMCC_Object *tstArray;
    JEMCC_Class *tstClass;
    JEM_JNIEnv *env;

    /* Initialize operating machines */
    if ((env = (JEM_JNIEnv *) createTestEnv()) == NULL) {
        (void) fprintf(stderr, "Fatal test initialization error\n");
        exit(1);
    }

    /* Create our initial base classes (Object, Class, Serializable) */
    if (JEM_InitializeVMClasses((JNIEnv *) env) != JNI_OK) {
        (void) fprintf(stderr, "Fatal core initialization error\n");
        exit(1);
    }

    /* Create a basic working frame for the test cases */
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_BYTECODE, 3, 4, 5);

    /* Build and test the stack macros */
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_FLOAT(currentFrame, 12.0);
    JEMCC_PUSH_STACK_DOUBLE(currentFrame, 24.0);
    JEMCC_PUSH_STACK_OBJECT(currentFrame, (JEMCC_Object *) 0xdead);
    if (JEMCC_POP_STACK_OBJECT(currentFrame) != (JEMCC_Object *) 0xdead) {
        (void) fprintf(stderr, "Error: stack frame object pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_DOUBLE(currentFrame) != 24.0) {
        (void) fprintf(stderr, "Error: stack frame double pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_FLOAT(currentFrame) != 12.0) {
        (void) fprintf(stderr, "Error: stack frame float pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }

    /* Test the local variable storage/retrieval */
    JEMCC_STORE_INT(currentFrame, 0, 6);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 1, 12);
    if (JEMCC_LOAD_INT(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_FLOAT(currentFrame, 0, 6.0);
    if (JEMCC_LOAD_FLOAT(currentFrame, 0) != 6.0) {
        (void) fprintf(stderr, "Error: stack frame even float var failed\n");
        exit(1);
    }
    JEMCC_STORE_FLOAT(currentFrame, 1, 12.0);
    if (JEMCC_LOAD_FLOAT(currentFrame, 1) != 12.0) {
        (void) fprintf(stderr, "Error: stack frame odd float var failed\n");
        exit(1);
    }
    JEMCC_STORE_OBJECT(currentFrame, 0, (JEMCC_Object *) 0xdead);
    if (JEMCC_LOAD_OBJECT(currentFrame, 0) != (JEMCC_Object *) 0xdead) {
        (void) fprintf(stderr, "Error: stack frame even object var failed\n");
        exit(1);
    }
    JEMCC_STORE_OBJECT(currentFrame, 1, (JEMCC_Object *) 0xdead);
    if (JEMCC_LOAD_OBJECT(currentFrame, 1) != (JEMCC_Object *) 0xdead) {
        (void) fprintf(stderr, "Error: stack frame odd object var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 0, 6);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 1, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_DOUBLE(currentFrame, 0, 6.0);
    if (JEMCC_LOAD_DOUBLE(currentFrame, 0) != 6.0) {
        (void) fprintf(stderr, "Error: stack frame even double var failed\n");
        exit(1);
    }
    JEMCC_STORE_DOUBLE(currentFrame, 1, 12.0);
    if (JEMCC_LOAD_DOUBLE(currentFrame, 1) != 12.0) {
        (void) fprintf(stderr, "Error: stack frame odd double var failed\n");
        exit(1);
    }
    (void) fprintf(stderr, "General stack test complete\n");

    /* Test lots of frame combinations, to verify alignments */
    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    /*** ROOT -> NATIVE ***/
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_NATIVE, 3, 4, 5);

    (void) fprintf(stderr, "ROOT->NATIVE test complete\n");

    /*** NATIVE->JEMCC ***/
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_JEMCC, 3, -1, 3);

    JEMCC_STORE_INT(currentFrame, 0, 6);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 1, 12);
    if (JEMCC_LOAD_INT(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 1, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 0, 6);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }

    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    (void) fprintf(stderr, "NATIVE->JEMCC test complete\n");

    /*** NATIVE->BYTECODE ***/
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_BYTECODE, 3, 4, 3);

    JEMCC_STORE_INT(currentFrame, 0, 6);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 1, 12);
    if (JEMCC_LOAD_INT(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 2, 6);
    if (JEMCC_LOAD_LONG(currentFrame, 2) != 6) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 1, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 0, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 12) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }

    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    (void) fprintf(stderr, "NATIVE->BYTECODE test complete\n");

    /*** ROOT->JEMCC ***/
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_JEMCC, 3, -1, 3);

    JEMCC_STORE_INT(currentFrame, 0, 6);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 1, 12);
    if (JEMCC_LOAD_INT(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 2, 12);
    if (JEMCC_LOAD_INT(currentFrame, 2) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 1, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 0, 6);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }

    (void) fprintf(stderr, "ROOT->JEMCC test complete\n");

    /*** JEMCC->BYTECODE ***/
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_BYTECODE, 3, 4, 3);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode iarg transfer failed\n");
        exit(1);
    }
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 24) {
        (void) fprintf(stderr, "Error: jemcc->bytecode larg transfer failed\n");
        exit(1);
    }

    JEMCC_STORE_INT(currentFrame, 0, 6);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 1, 12);
    if (JEMCC_LOAD_INT(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 2, 6);
    if (JEMCC_LOAD_LONG(currentFrame, 2) != 6) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 2, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 2) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 1, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 0, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 12) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }

    JEMCC_STORE_LONG(currentFrame, 0, 6);
    JEMCC_STORE_INT(currentFrame, 2, 12);
    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 6) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_BYTECODE, 3, 4, 3);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 24) {
        (void) fprintf(stderr, "Error: jemcc->bytecode larg transfer failed\n");
        exit(1);
    }
    if (JEMCC_LOAD_INT(currentFrame, 2) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode iarg transfer failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 0, 12);
    JEMCC_STORE_LONG(currentFrame, 1, 6);
    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    if (JEMCC_POP_STACK_LONG(currentFrame) != 6) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);

    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    (void) fprintf(stderr, "JEMCC->BYTECODE test complete\n");

    /*** ROOT->BYTECODE ***/
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_BYTECODE, 3, 4, 3);

    JEMCC_STORE_INT(currentFrame, 0, 6);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 1, 12);
    if (JEMCC_LOAD_INT(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 2, 12);
    if (JEMCC_LOAD_INT(currentFrame, 2) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 2, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 2) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 1, 6);
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 6) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }

    (void) fprintf(stderr, "ROOT->BYTECODE test complete\n");

    /*** BYTECODE->JEMCC ***/
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_JEMCC, 3, -1, 3);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode iarg transfer failed\n");
        exit(1);
    }
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 24) {
        (void) fprintf(stderr, "Error: jemcc->bytecode larg transfer failed\n");
        exit(1);
    }

    JEMCC_STORE_INT(currentFrame, 0, 6);
    if (JEMCC_LOAD_INT(currentFrame, 0) != 6) {
        (void) fprintf(stderr, "Error: stack frame even integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 1, 12);
    if (JEMCC_LOAD_INT(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd integer var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 1, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 1) != 12) {
        (void) fprintf(stderr, "Error: stack frame odd long var failed\n");
        exit(1);
    }
    JEMCC_STORE_LONG(currentFrame, 0, 12);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 12) {
        (void) fprintf(stderr, "Error: stack frame even long var failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: stack frame integer pop failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 24) {
        (void) fprintf(stderr, "Error: stack frame long pop failed\n");
        exit(1);
    }

    JEMCC_STORE_LONG(currentFrame, 0, 6);
    JEMCC_STORE_INT(currentFrame, 2, 12);
    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_LONG(currentFrame) != 6) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_LONG(currentFrame, 24);
    JEMCC_PUSH_STACK_INT(currentFrame, 12);
    currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_BYTECODE, 3, 4, 3);
    if (JEMCC_LOAD_LONG(currentFrame, 0) != 24) {
        (void) fprintf(stderr, "Error: jemcc->bytecode larg transfer failed\n");
        exit(1);
    }
    if (JEMCC_LOAD_INT(currentFrame, 2) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode iarg transfer failed\n");
        exit(1);
    }
    JEMCC_STORE_INT(currentFrame, 0, 12);
    JEMCC_STORE_LONG(currentFrame, 1, 6);
    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    if (JEMCC_POP_STACK_LONG(currentFrame) != 6) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }
    if (JEMCC_POP_STACK_INT(currentFrame) != 12) {
        (void) fprintf(stderr, "Error: jemcc->bytecode arg return failed\n");
        exit(1);
    }

    JEMCC_PUSH_STACK_INT(currentFrame, 12);

    currentFrame = (JEMCC_VMFrame *) env->topFrame = 
                   ((JEM_VMFrameExt *) currentFrame)->previousFrame;
    (void) fprintf(stderr, "BYTECODE->JEMCC test complete\n");

    /* Build the bytecode method and frame instance */
    bcMethod.maxStack = 10;
    bcMethod.maxLocals = 10;
    bcMethod.codeLength = 1000;
    bcMethod.exceptionTable = NULL;
    bcMethod.exceptionTableLength = 0;
    method.method.bcMethod = &bcMethod;
    method.name = "testMethod";
    method.descriptorStr = "()V";
    /* Scan through the code slices */
    for (i = 0; i < nTestBlocks; i++) {
        (void) fprintf(stderr, "** Running test %i **\n", i + 1);
        /* Initialize the method and the frame */
        bcMethod.code = codeSlices[i].codeData;
        currentFrame = JEM_CreateFrame((JNIEnv *) env, FRAME_BYTECODE, 
                                       bcMethod.maxStack, bcMethod.maxStack, 
                                       bcMethod.maxLocals);
        ((JEM_VMFrameExt *) currentFrame)->currentMethod = &method;
        ((JEM_VMFrameExt *) currentFrame)->lastPC = 0;

        /* Initialize the arguments for the processing code */
        tstArray = NULL;
        switch (codeSlices[i].inputType) {
            case OBJ_INPUT:
                JEMCC_STORE_OBJECT(currentFrame, 0, (JEMCC_Object *) 0xdead);
                JEMCC_STORE_OBJECT(currentFrame, 1, (JEMCC_Object *) 0xbeef);
                JEMCC_STORE_OBJECT(currentFrame, 2, (JEMCC_Object *) 0xcafe);
                JEMCC_STORE_OBJECT(currentFrame, 3, (JEMCC_Object *) 0xbabe);
                break;
            case OBJ_ARRAY_INPUT:
                if (JEMCC_LocateClass((JNIEnv *) env, NULL, "java.lang.Object",
                                      JNI_FALSE, &tstClass) != JNI_OK) {
                    (void) fprintf(stderr, "Could not find Object class\n");
                    exit(1);
                }
                tstArray = JEMCC_NewObjectArray((JNIEnv *) env, 2,
                                                (jclass) tstClass, NULL);
                if (tstArray == NULL) {
                    (void) fprintf(stderr, "Could not create tst array\n");
                    exit(1);
                }
                JEMCC_SetObjectArrayElement((JNIEnv *) env,
                                            (jobject) tstArray,
                                            0, (jobject) tstClass);
                JEMCC_SetObjectArrayElement((JNIEnv *) env,
                                            (jobject) tstArray,
                                            1, (jobject) NULL);
                JEMCC_STORE_OBJECT(currentFrame, 0, (JEMCC_Object *) 0xdead);
                JEMCC_STORE_OBJECT(currentFrame, 1, (JEMCC_Object *) 0xbeef);
                JEMCC_STORE_OBJECT(currentFrame, 2, tstArray);
                break;
            case INT_INPUT:
                JEMCC_STORE_INT(currentFrame, 0, 0);
                JEMCC_STORE_INT(currentFrame, 1, 1);
                JEMCC_STORE_INT(currentFrame, 2, 2);
                JEMCC_STORE_INT(currentFrame, 3, 3);
                break;
            case BYTE_ARRAY_INPUT:
                tstArray = JEMCC_NewBooleanArray((JNIEnv *) env, 8);
                if (tstArray == NULL) {
                    (void) fprintf(stderr, "Could not create tst array\n");
                    exit(1);
                }
                JEMCC_STORE_INT(currentFrame, 0, 2);
                JEMCC_STORE_INT(currentFrame, 1, 4);
                JEMCC_STORE_OBJECT(currentFrame, 2, tstArray);
                break;
            case LONG_INPUT:
                JEMCC_STORE_LONG(currentFrame, 0, 0);
                JEMCC_STORE_LONG(currentFrame, 2, 2);
                break;
            case FLOAT_INPUT:
                JEMCC_STORE_FLOAT(currentFrame, 0, 0.0);
                JEMCC_STORE_FLOAT(currentFrame, 1, 1.0);
                JEMCC_STORE_FLOAT(currentFrame, 2, 2.0);
                JEMCC_STORE_FLOAT(currentFrame, 3, 3.0);
                break;
            case DOUBLE_INPUT:
                JEMCC_STORE_DOUBLE(currentFrame, 0, 0.0);
                JEMCC_STORE_DOUBLE(currentFrame, 2, 2.0);
                break;
        }

        /* Run the bytecode */
        JEM_ExecuteCurrentFrame((JNIEnv *) env, 0);

        /* Verify the processing return code */
        switch (codeSlices[i].returnType) {
            case OBJ_RETURN:
                if (env->nativeReturnValue.objVal != 
                                (JEMCC_Object *) codeSlices[i].returnVal) {
                    (void) fprintf(stderr, 
                                   "Error: code test %i did not return %p\n",
                                   i + 1, 
                                   (JEMCC_Object *) codeSlices[i].returnVal);
                    (void) fprintf(stderr, "       Return value %p\n",
                                           env->nativeReturnValue.objVal);
                    exit(1);
                }
                break;
            case INT_RETURN:
                if (env->nativeReturnValue.intVal != codeSlices[i].returnVal) {
                    (void) fprintf(stderr, 
                                   "Error: code test %i did not return %i\n", 
                                   i + 1, codeSlices[i].returnVal);
                    (void) fprintf(stderr, "       Return value %i\n",
                                           env->nativeReturnValue.intVal);
                    exit(1);
                }
                break;
            case LONG_RETURN:
                if (env->nativeReturnValue.longVal != 
                                  (long) codeSlices[i].returnVal) {
                    (void) fprintf(stderr, 
                                   "Error: code test %i did not return %i\n", 
                                   i + 1, codeSlices[i].returnVal);
                    (void) fprintf(stderr, "       Return value %lli\n",
                                           env->nativeReturnValue.longVal);
                    exit(1);
                }
                break;
            case FLOAT_RETURN:
                if (env->nativeReturnValue.fltVal != 
                                  1.0 * codeSlices[i].returnVal) {
                    (void) fprintf(stderr, 
                                   "Error: code test %i did not return %f\n", 
                                   i + 1, (float) codeSlices[i].returnVal);
                    (void) fprintf(stderr, "       Return value %f\n",
                                           env->nativeReturnValue.fltVal);
                    exit(1);
                }
                break;
            case DOUBLE_RETURN:
                if (env->nativeReturnValue.dblVal != 
                                  1.0 * codeSlices[i].returnVal) {
                    (void) fprintf(stderr, 
                                   "Error: code test %i did not return %f\n", 
                                   i + 1, (float) codeSlices[i].returnVal);
                    (void) fprintf(stderr, "       Return value %f\n",
                                           env->nativeReturnValue.dblVal);
                    exit(1);
                }
                break;
            case EXCEPTION_RETURN:
                if (env->pendingException == NULL) {
                    (void) fprintf(stderr, 
                           "Error: code test %i did not throw exception\n", i);
                    exit(1);
                }
                if (codeSlices[i].exceptionStr != NULL) {
                    checkException((JNIEnv *) env, NULL, 
                                   codeSlices[i].exceptionStr, "");
                }
                break;
        }

        /* Clean up locally allocated objects */
        if (tstArray != NULL) {
            JEMCC_Free(((JEMCC_ObjectExt *) tstArray)->objectData);
            JEMCC_Free(tstArray);
        }
    }

    /* Clean up the test environment */
    destroyTestEnv((JNIEnv *) env);

    exit(0);
}

/* Local methods to avoid full library inclusion */
void *JEMCC_Malloc(JNIEnv *env, juint size) {
#ifdef ENABLE_ERRORSWEEP
    if (JEM_CheckErrorSweep(ES_MEM) == JNI_TRUE) {
        (void) fprintf(stderr, "Error[local]: Simulated malloc failure\n");
        return NULL;
    }
#endif
    return calloc(1, size);
}

jint JEM_CallForeignFunction(JNIEnv *env, JEMCC_Object *thisObj, void *fnRef,
                             union JEM_DescriptorInfo *fnDesc,
                             JEMCC_ReturnValue *argList,
                             JEMCC_ReturnValue *retVal) {
    return JNI_OK;
}
