/**
 * JEMCC test program to test the internal String features.
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

/* External elements from uvminit.c */
extern JNIEnv *createTestEnv();
extern jint initializeTestCoreClasses(JNIEnv *env);
extern void destroyTestEnv(JNIEnv *env);

/* From jnifunc.h */
extern jstring JEMCC_NewStringUTF(JNIEnv *env, const char *utf);
extern jstring JEMCC_NewString(JNIEnv *env, const jchar *unicode, jsize len);

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

/* Storage buffer for exception class and message (failure verification) */
int quietMode = JNI_TRUE;
char exClassName[256], exMsg[4096];

void checkException(const char *checkClassName, const char *checkMsg,
                    const char *tstName) {
    if ((checkClassName != NULL) &&
        (strstr(exClassName, checkClassName) == NULL)) {
        (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
        (void) fprintf(stderr, "Expected %s, got %s instead.\n",
                               checkClassName, exClassName);
        exit(1);
    }
    if ((checkMsg != NULL) && (strstr(exMsg, checkMsg) == NULL)) {
        (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
        (void) fprintf(stderr, "Expected '%s', got '%s' instead.\n",
                               checkMsg, exMsg);
        exit(1);
    }
}

/* Forward declaration */
void doValidScan();

struct tstStringData {
    int length;
    char data[1024];
};

static struct tstStringData tstStringDataA, tstStringDataB;

/* Send the JEMCC class definition methods through their paces */
int main(int argc, char *argv[]) {
    char mixedAscii[] = { 0x41, 0x73, 0x63, 0x69, 0x69, 0xBE, 0xA2, 0x0 };
    jchar uniAscii[] = { 0x41, 0x73, 0x63, 0x69, 0x69 };
    jchar unicode[] = { 0x55, 0x6E, 0x69, 0xA6F2 };
    JEMCC_StringData *tstStringA = (JEMCC_StringData *) &tstStringDataA, 
                     *tstStringB = (JEMCC_StringData *) &tstStringDataB;

    /* No memory test just yet */
#ifdef ENABLE_ERRORSWEEP
    testFailureCount = -1;
#endif
    quietMode = JNI_TRUE;

    /* Basic test cases */
    if (JEMCC_UTFStrIsAscii("This is a plain ASCII string!!") != JNI_TRUE) {
        (void) fprintf(stderr, "Invalid return for ASCII UTF string\n");
        exit(1);
    }
    if (JEMCC_UTFStrIsAscii(mixedAscii) != JNI_FALSE) {
        (void) fprintf(stderr, "Invalid return for mixed UTF string\n");
        exit(1);
    }
    if (JEMCC_UnicodeStrIsAscii(uniAscii, 5) != JNI_TRUE) {
        (void) fprintf(stderr, "Invalid return for ASCII Unicode string\n");
        exit(1);
    }
    if (JEMCC_UnicodeStrIsAscii(unicode, 4) != JNI_FALSE) {
        (void) fprintf(stderr, "Invalid return for Unicode string\n");
        exit(1);
    }

    tstStringDataA.length = -5;
    (void) strcpy(tstStringDataA.data, "abcdefg");
    tstStringDataB.length = -5;
    (void) strcpy(tstStringDataB.data, "abcdefg");
    if (JEMCC_StringHashFn(NULL, tstStringA) != 
                            JEMCC_StringHashFn(NULL, tstStringB)) {
        (void) fprintf(stderr, "Invalid return for ASCII string hashes\n");
        exit(1);
    }
    if (JEMCC_StringEqualsFn(NULL, tstStringA, tstStringB) != JNI_TRUE) {
        (void) fprintf(stderr, "Invalid return for ASCII string = compare\n");
        exit(1);
    }
    tstStringDataB.data[0] = 'x';
    if (JEMCC_StringEqualsFn(NULL, tstStringA, tstStringB) != JNI_FALSE) {
        (void) fprintf(stderr, "Invalid return for ASCII string != compare\n");
        exit(1);
    }
    tstStringDataA.length = tstStringDataB.length = 0;
    if (JEMCC_StringEqualsFn(NULL, tstStringA, tstStringB) != JNI_TRUE) {
        (void) fprintf(stderr, "Invalid return for zero string compare\n");
        exit(1);
    }

    tstStringDataA.length = 4;
    (void) memcpy(tstStringDataA.data, unicode, 8);
    tstStringDataB.length = 4;
    (void) memcpy(tstStringDataB.data, unicode, 8);
    if (JEMCC_StringHashFn(NULL, tstStringA) != 
                            JEMCC_StringHashFn(NULL, tstStringB)) {
        (void) fprintf(stderr, "Invalid return for uni string hashes\n");
        exit(1);
    }
    if (JEMCC_StringEqualsFn(NULL, tstStringA, tstStringB) != JNI_TRUE) {
        (void) fprintf(stderr, "Invalid return for uni string = compare\n");
        exit(1);
    }
    *(((jchar *) tstStringDataB.data) + 3) = 'x';
    if (JEMCC_StringEqualsFn(NULL, tstStringA, tstStringB) != JNI_FALSE) {
        (void) fprintf(stderr, "Invalid return for uni string != compare\n");
        exit(1);
    }

    /* Memory failure scanning */
#ifdef ENABLE_ERRORSWEEP
    testFailureCurrentCount = 0;
    doValidScan();
    failureTotal = testFailureCurrentCount;
    (void) fprintf(stderr, "Preparing to process %i memory failures\n",
                           failureTotal);
    for (testFailureCount = 1;
             testFailureCount <= failureTotal;
                 testFailureCount++) {
        testFailureCurrentCount = 0;
        doValidScan();
    }
#else
    failureTotal = 0;
    doValidScan();
#endif

    (void) fprintf(stderr, "All tests passed successfully (%i forced)\n",
                           failureTotal);
    exit(0);
}

/* Manual destruct operation for String objects (not for intern'd instances) */
void destroyString(JEMCC_Object *string) {
    /* JEMCC_Free(((JEMCC_ObjectExt *) string)->objectData);
    JEMCC_Free(string);  */
}

void doValidScan() {
    JNIEnv *env;
    JEMCC_Object *tstStr, *compStr, *wkgStr, *tstObj;
    JEMCC_Class *classClass;
    JEMCC_StringData *strData;
    char buff[64], buff2[64];
    jchar unibuff[64];
    int idx;
#ifdef ENABLE_ERRORSWEEP
    int tmpFailCount = testFailureCount;
    int tmpFailCurrentCount = testFailureCurrentCount;
    testFailureCount = -1;
#endif

    /* Initialize operating machines */
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, 
                       "Unexpected fatal error during test env setup\n");
        exit(1);
    }

    /* Create our initial base classes (Object, Class, Serializable) */
    if (JEM_InitializeVMClasses(env) != JNI_OK) {
        (void) fprintf(stderr, 
                       "Unexpected fatal error during class init\n");
        exit(1);
    }

    /* The following tests may need a running frame */
    if (JEM_CreateFrame(env, FRAME_JEMCC, 0, -1, 16) == NULL) {
        destroyTestEnv(env);
        return;
    }

#ifdef ENABLE_ERRORSWEEP
    testFailureCount = tmpFailCount;
    testFailureCurrentCount = tmpFailCurrentCount;
#endif

    /* Basic ASCII string retrieves/constructions */
    tstStr = JEMCC_GetInternStringUTF(env, "Ascii String");
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_GetInternStringUTF(env, "Ascii String");
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: ASCII UTF intern mismatch\n");
        exit(1);
    }

    wkgStr = (JEMCC_Object *) JEMCC_NewStringUTF(env, "Ascii String");
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: new ASCII UTF mismatch\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    wkgStr = JEMCC_StringCatUTF(env, "Ascii", " ", "String", NULL);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: ASCII UTF cat mismatch\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    /* UTF conversion test cases */
    tstStr = JEMCC_GetInternStringUTF(env, "Ascii");
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    idx = 0;
    buff[idx++] = 0x81; buff[idx++] = 0x81; /* 'A' */
    buff[idx++] = 0xA0; buff[idx++] = 0x81; buff[idx++] = 0xB3; /* 's' */
    buff[idx++] = 0x63; /* 'c' */
    buff[idx++] = 0x81; buff[idx++] = 0xA9; /* 'i' */
    buff[idx++] = 0xA0; buff[idx++] = 0x81; buff[idx++] = 0xA9;/* 'i' */
    buff[idx] = 0;
    compStr = JEMCC_GetInternStringUTF(env, buff);
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: UTF intern mismatch\n");
        exit(1);
    }

    wkgStr = (JEMCC_Object *) JEMCC_NewStringUTF(env, buff);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: new UTF mismatch\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    idx = 0;
    buff[idx++] = 0x81; buff[idx++] = 0x81; /* 'A' */
    buff[idx++] = 0xA0; buff[idx++] = 0x81; buff[idx++] = 0xB3; /* 's' */
    buff[idx++] = 0x63; /* 'c' */
    buff[idx] = 0;
    idx = 0;
    buff2[idx++] = 0x81; buff2[idx++] = 0xA9; /* 'i' */
    buff2[idx++] = 0xA0; buff2[idx++] = 0x81; buff2[idx++] = 0xA9;/* 'i' */
    buff2[idx] = 0;
    wkgStr = JEMCC_StringCatUTF(env, buff, buff2, NULL);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: UTF cat mismatch\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    /* ASCII-only Unicode test case */
    idx = 0;
    unibuff[idx++] = 0x55; unibuff[idx++] = 0x6E; unibuff[idx++] = 0x69;
    tstStr = JEMCC_GetInternString(env, unibuff, 3);
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_GetInternString(env, unibuff, 3);
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: unicode ASCII intern mismatch\n");
        exit(1);
    }

    wkgStr = (JEMCC_Object *) JEMCC_NewString(env, unibuff, 3);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: new unicode ASCII mismatch\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    /* Full Unicode test case */
    idx = 0;
    unibuff[idx++] = 0xCAFE; unibuff[idx++] = 0xBABE;
    tstStr = JEMCC_GetInternString(env, unibuff, 2);
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_GetInternString(env, unibuff, 2);
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: unicode intern mismatch\n");
        exit(1);
    }

    idx = 0;
    buff[idx++] = 0xEC; buff[idx++] = 0xAB; buff[idx++] = 0xBE; /* 0xCAFE */
    buff[idx] = 0;
    idx = 0;
    buff2[idx++] = 0xEB; buff2[idx++] = 0xAA; buff2[idx++] = 0xBE; /* 0xBABE */
    buff2[idx] = 0;
    wkgStr = JEMCC_StringCatUTF(env, buff, buff2, NULL);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: unicode UTF mismatch\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    wkgStr = (JEMCC_Object *) JEMCC_NewString(env, unibuff, 2);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != compStr) {
        (void) fprintf(stderr, "Fatal error: new unicode mismatch\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    /* Direct String instances */
    wkgStr = JEMCC_AllocateObjectIdx(env, JEMCC_Class_String,
                                     sizeof(JEMCC_StringData) + 10);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    strData = (JEMCC_StringData *) ((JEMCC_ObjectExt *) wkgStr)->objectData;
    strData->length = -5;
    (void) strcpy(&(strData->data), "AsCiI");
    tstStr = JEMCC_FindInternString(env, wkgStr);
    if (tstStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (tstStr != wkgStr) {
        (void) fprintf(stderr, "Fatal error: incorrect 1st string intern\n");
        exit(1);
    }

    wkgStr = (JEMCC_Object *) JEMCC_NewStringUTF(env, "AsCiI");
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    if (compStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (compStr != tstStr) {
        (void) fprintf(stderr, "Fatal error: incorrect 2st string intern\n");
        exit(1);
    }
    /* Clean up, since it didn't intern() */
    destroyString(wkgStr);

    /*********************************************************/

    /* A-A */
    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    wkgStr = JEMCC_NewStringUTF(env, "Ascii Ascii");
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    tstStr = JEMCC_FindInternString(env, wkgStr);
    if (tstStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }

    if (JEMCC_EnvStrBufferAppend(env, "Ascii ") == NULL) {
        destroyTestEnv(env);
        return;
    }
    wkgStr = JEMCC_NewStringUTF(env, "Ascii");
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppend(env, wkgStr) == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(wkgStr);
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    destroyString(wkgStr);
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (compStr != tstStr) {
        (void) fprintf(stderr, "Fatal error: invalid a-a string append\n");
        exit(1);
    }

    /* A-U */
    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    idx = 0;
    unibuff[idx++] = 'A';
    unibuff[idx++] = 's';
    unibuff[idx++] = 'c';
    unibuff[idx++] = 'i';
    unibuff[idx++] = 'i';
    unibuff[idx++] = ' ';
    unibuff[idx++] = 0x1111;
    unibuff[idx++] = 0x2222;
    unibuff[idx++] = 0x3333;
    wkgStr = JEMCC_NewString(env, unibuff, idx);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    tstStr = JEMCC_FindInternString(env, wkgStr);
    if (tstStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }

    wkgStr = JEMCC_NewStringUTF(env, "Ascii ");
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppend(env, wkgStr) == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(wkgStr);
    wkgStr = JEMCC_NewString(env, unibuff + 6, idx - 6);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppend(env, wkgStr) == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(wkgStr);
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    destroyString(wkgStr);
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (compStr != tstStr) {
        (void) fprintf(stderr, "Fatal error: invalid a-u string append\n");
        exit(1);
    }

    /* U-A */
    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    idx = 0;
    unibuff[idx++] = 0x1111;
    unibuff[idx++] = 0x2222;
    unibuff[idx++] = 0x1234;
    unibuff[idx++] = 0x4321;
    unibuff[idx++] = 0x3333;
    unibuff[idx++] = ' ';
    unibuff[idx++] = 'A';
    unibuff[idx++] = 's';
    unibuff[idx++] = 'c';
    unibuff[idx++] = 'i';
    unibuff[idx++] = 'i';
    wkgStr = JEMCC_NewString(env, unibuff, idx);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    tstStr = JEMCC_FindInternString(env, wkgStr);
    if (tstStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }

    wkgStr = JEMCC_NewString(env, unibuff, 5);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppend(env, wkgStr) == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(wkgStr);
    if (JEMCC_EnvStrBufferAppend(env, " Asc") == NULL) {
        destroyTestEnv(env);
        return;
    }
    wkgStr = JEMCC_NewStringUTF(env, "ii");
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppend(env, wkgStr) == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(wkgStr);
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    destroyString(wkgStr);
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (compStr != tstStr) {
        (void) fprintf(stderr, "Fatal error: invalid u-a string append\n");
        exit(1);
    }

    /* U-U */
    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    idx = 0;
    unibuff[idx++] = 0x1111;
    unibuff[idx++] = 0x2222;
    unibuff[idx++] = 0x3333;
    unibuff[idx++] = 0x4444;
    unibuff[idx++] = 0x5555;
    unibuff[idx++] = 0x1234;
    unibuff[idx++] = 0x5678;
    unibuff[idx++] = 0x4321;
    unibuff[idx++] = 0x8765;
    wkgStr = JEMCC_NewString(env, unibuff, idx);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    tstStr = JEMCC_FindInternString(env, wkgStr);
    if (tstStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }

    wkgStr = JEMCC_NewString(env, unibuff, 4);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppend(env, wkgStr) == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(wkgStr);
    wkgStr = JEMCC_NewString(env, unibuff + 4, idx - 4);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppend(env, wkgStr) == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(wkgStr);
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    compStr = JEMCC_FindInternString(env, wkgStr);
    destroyString(wkgStr);
    if (compStr == NULL) {
        destroyTestEnv(env);
        return;
    }
    if (compStr != tstStr) {
        (void) fprintf(stderr, "Fatal error: invalid u-u string append\n");
        exit(1);
    }

    /*********************************************************/

    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    tstStr = JEMCC_NewStringUTF(env, "this is a test");
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }

    if (JEMCC_EnvStringBufferAppendObj(env, tstStr) == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_StringEqualsFn(env, ((JEMCC_ObjectExt *) wkgStr)->objectData,
                       ((JEMCC_ObjectExt *) tstStr)->objectData) != JNI_TRUE) {
        (void) fprintf(stderr, "Fatal error: invalid String obj append\n");
        exit(1);
    }
    destroyString(tstStr);
    destroyString(wkgStr);

    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    tstStr = JEMCC_NewStringUTF(env, "class ");
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }

    classClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Class);
    if (JEMCC_EnvStringBufferAppendObj(env, 
                              (JEMCC_Object *) classClass) == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_StringString(env, 
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) wkgStr)->objectData,
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) tstStr)->objectData,
                           0) != 0) {
        (void) fprintf(stderr, "Fatal error: invalid Class.toString append\n");
        exit(1);
    }
    destroyString(tstStr);
    destroyString(wkgStr);

    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    tstStr = JEMCC_NewStringUTF(env, "java.lang.Object@");
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }

    tstObj = JEMCC_AllocateObject(env,
                                  JEMCC_GetCoreVMClass(env, JEMCC_Class_Object),
                                  0);
    if (tstObj == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_EnvStringBufferAppendObj(env, tstObj) == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_LastStringString(env, 
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) wkgStr)->objectData,
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) tstStr)->objectData,
                           999) != 0) {
        (void) fprintf(stderr, "Fatal error: invalid Object.toString append\n");
        exit(1);
    }
    destroyString(tstStr);
    destroyString(wkgStr);

    if (JEMCC_EnvStrBufferInit(env, 8) == NULL) {
        destroyTestEnv(env);
        return;
    }

    classClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Class);
    tstObj = JEMCC_AllocateObject(env,
                                  JEMCC_GetCoreVMClass(env, JEMCC_Class_Object),
                                  0);
    if (tstObj == NULL) {
        destroyTestEnv(env);
        return;
    }
    tstStr = JEMCC_NewStringUTF(env, "Bonjour!");
    if (tstStr == NULL) {
        destroyTestEnv(env);
        return;
    }

    if (JEMCC_EnvStringBufferAppendSet(env, "Test: ", STR_OBJECT(tstStr),
                                       STR_OBJECT(classClass), ":",
                                       STR_OBJECT(tstObj), NULL) == NULL) {
        destroyString(tstStr);
        destroyTestEnv(env);
        return;
    }
    destroyString(tstStr);
    wkgStr = JEMCC_EnvStringBufferToString(env);
    if (wkgStr == NULL) {
        destroyTestEnv(env);
        return;
    }

    tstStr = JEMCC_NewStringUTF(env, "Bon");
    if (tstStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_StringString(env, 
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) wkgStr)->objectData,
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) tstStr)->objectData,
                           0) != 6) {
        (void) fprintf(stderr, "Fatal error: invalid multi-append\n");
        exit(1);
    }
    destroyString(tstStr);

    tstStr = JEMCC_NewStringUTF(env, "java.lang.Class:java.lang.Object");
    if (tstStr == NULL) {
        destroyString(wkgStr);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_StringString(env, 
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) wkgStr)->objectData,
                (JEMCC_StringData *) ((JEMCC_ObjectExt *) tstStr)->objectData,
                           0) != 20) {
        (void) fprintf(stderr, "Fatal error: invalid multi-append\n");
        exit(1);
    }
    destroyString(tstStr);

    destroyString(wkgStr);

    /* Clean up to validate purify operation */
    destroyTestEnv(env);
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
