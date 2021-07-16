/**
 * JEMCC test program to test the field/method descriptor parsing.
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

/* Read the jni/jem internal details */
#include "jem.h"

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

/* Test cases, some valid, some not */
static struct desc_test_data {
    char *textData;
    char *msgFragment;
} descTests[] = {
    {  
       "B", NULL
    },
    {  
       "C", NULL
    },
    {  
       "D", NULL
    },
    {  
       "F", NULL
    },
    {  
       "I", NULL
    },
    {  
       "J", NULL
    },
    {  
       "S", NULL
    },
    {  
       "Z", NULL
    },
    {  
       "Ljava/lang/Object;", NULL
    },
    {  
       "[[B", NULL  
    },
    {  
       "(JSZLjava/lang/Object;)V", NULL   /* Test 10 */
    },
    {  
       "(BCDF[Ljava/lang/Object;[[Z)S", NULL
    },
    {  
       "Ljava/lang/Object", "Missing semi-colon"
    },
    {  
       "(Ljava/lang/ObjectS)Ljava/lang/Object;", "Missing semi-colon"
    },
    {  
       "X", "Invalid character"
    },
    {  
       "Ljava/lang/Object;X", "Extra text"
    },
    {  
       "IX", "Extra text"
    },
    {  
       "[[[[[IX", "Extra text"
    },
    {  
       "(()V", "embedded"
    },
    {  
       "(IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII)V", "Too many parameters"
    },
    {  
       "(I)X", "Invalid character"   /* Test 20 */
    },
    {  
       "(IX)V", "Invalid character"
    },
    {  
       "(I)VX", "Extra text"
    },
    {  
       "(I)Ljava/lang/Object;X", "Extra text"
    },
    {  
       "S(Ljava/lang/Object;)V", "Extra text"
    },
    {  
       "(SILjava/lang/Object;", "Unterminated method"
    },
    {  
       "[[[", "Unterminated array"
    },
    {  
       "(SI)[[[V", "Invalid character"
    },
    {  
       "(Ljava/lang/Object;[[[)V", "Invalid character"
    },
    {  
       "", "empty"
    },
};

/* Forward declaration, see below */
void doValidScan(jboolean print);

/* Comparison function which handles mixed class package separators */
jboolean strslcmp(const char *ptr, const char *str) {
    while (*ptr != '\0') {
        if (*ptr != *str) {
            if (((*ptr != '.') && (*ptr != '/')) ||
                ((*str != '.') && (*str != '/'))) {
                return JNI_TRUE;
            }
        }
        ptr++;
        str++;
    }
    return JNI_FALSE;
}

/* Main program will send the descriptor parser through its paces */
int main(int argc, char *argv[]) {
    int i, rc, nTests = sizeof(descTests)/sizeof(struct desc_test_data);
    JEM_DescriptorData *data, dummyData[2], dummyMethData;
    JEM_JNIEnv envData;
    char *ptr, *exceptionMessage;

    /* Spoof an environment to support buffer usage, but with NO VM */
    envData.envBuffer = envData.envEndPtr = NULL;
    envData.envBufferLength = 0;
    envData.parentVM = NULL;

    /* Data failure tests first, do not force internal errors */
#ifdef ENABLE_ERRORSWEEP
    testFailureCount = -1;
#endif

    /* First, just try to parse the descTests data array */
    for (i = 0; i < nTests; i++) {
        *exClassName = *exMsg = '\0';
        data = JEM_ParseDescriptor((JNIEnv *) &envData, 
                                   descTests[i].textData, NULL, NULL, 
                                   JNI_FALSE);
        exceptionMessage = exMsg;
        if (strlen(exceptionMessage) == 0) exceptionMessage = NULL;
        if (descTests[i].msgFragment != NULL) {
            if (exceptionMessage == NULL) {
                (void) fprintf(stderr, 
                               "Test %i: didn't get parse error '%s'\n",
                               i, descTests[i].msgFragment);
                exit(1);
            }
            if (strstr(exceptionMessage, 
                       descTests[i].msgFragment) == NULL) {
                (void) fprintf(stderr, 
                               "Test %i: didn't get parse fragment '%s'\n",
                               i, descTests[i].msgFragment);
                (void) fprintf(stderr, "    Got '%s' instead\n",
                                       exceptionMessage);
                exit(1);
            }
        } else {
            if (exceptionMessage != NULL) {
                (void) fprintf(stderr, 
                               "Test %i: got unexpected parse error '%s'\n",
                               i, exceptionMessage);
                exit(1);
            } else if (data != NULL) {
                /* Check that we can reconstruct it */
                ptr = JEM_ConvertDescriptor((JNIEnv *) &envData, data, 0);
                if (strslcmp(descTests[i].textData, ptr) != JNI_FALSE) {
                    (void) fprintf(stderr, 
                                   "Test %i: couldn't reconstruct %s->%s\n",
                                   i, descTests[i].textData, ptr);
                    exit(1);
                }
                JEMCC_Free(ptr);
            }
        }
        if (data != NULL) JEM_DestroyDescriptor(data, 1);

        *exClassName = *exMsg = '\0';
        rc = JEM_ValidateDescriptor((JNIEnv *) &envData, descTests[i].textData,
                      (*(descTests[i].textData) == '(') ? JNI_TRUE : JNI_FALSE);
        exceptionMessage = exMsg;
        if (strlen(exceptionMessage) == 0) exceptionMessage = NULL;
        if (descTests[i].msgFragment != NULL) {
            if ((rc == JNI_OK) || (exceptionMessage == NULL)) {
                (void) fprintf(stderr, 
                               "Test %i: didn't get validate error '%s'\n",
                               i, descTests[i].msgFragment);
                exit(1);
            }
            if (strstr(exceptionMessage, 
                       descTests[i].msgFragment) == NULL) {
                (void) fprintf(stderr, 
                               "Test %i: didn't get validate fragment '%s'\n",
                               i, descTests[i].msgFragment);
                (void) fprintf(stderr, "    Got '%s' instead\n",
                                       exceptionMessage);
                exit(1);
            }
        } else {
            if ((rc != JNI_OK) || (exceptionMessage != NULL)) {
                (void) fprintf(stderr, 
                               "Test %i: got unexpected validate error '%s'\n",
                               i, exceptionMessage);
                exit(1);
            } 
        }
    }

    /* Special failure cases */
    data = JEM_ParseDescriptor((JNIEnv *) &envData, "Ljava/lang/Object;",
                               NULL, NULL, JNI_FALSE);
    ptr = JEM_ConvertMethodDescriptor((JNIEnv *) &envData, "notAMethod", data);
    if (strlen(ptr) != 0) {
        (void) fprintf(stderr, "Non-method conversion failed\n");
        exit(1);
    }
    JEM_DestroyDescriptor(data, 1);
    JEMCC_Free(ptr);

    /* Make sure the abstract data failure cases don't choke */
    dummyData[0].generic.tag = 1000;
    dummyData[0].generic.tag = DESCRIPTOR_EndOfList;
    ptr = JEM_ConvertDescriptor((JNIEnv *) &envData, dummyData, 0);
    JEMCC_Free(ptr);
    dummyMethData.generic.tag = DESCRIPTOR_MethodType;
    dummyMethData.method_info.paramDescriptor = dummyData;
    dummyMethData.method_info.returnDescriptor = dummyData;
    ptr = JEM_ConvertMethodDescriptor((JNIEnv *) &envData, "garbage",
                                      &dummyMethData);
    JEMCC_Free(ptr);

    /* Memory failure scanning */
#ifdef ENABLE_ERRORSWEEP
    testFailureCurrentCount = 0;
    doValidScan(JNI_TRUE);
    failureTotal = testFailureCurrentCount;
    (void) fprintf(stderr, "Preparing to process %i memory failures\n",
                           failureTotal);
    for (testFailureCount = 1; 
             testFailureCount <= failureTotal;
                 testFailureCount++) {
        testFailureCurrentCount = 0;
        doValidScan(JNI_FALSE);
    }
#else
    failureTotal = 0;
    doValidScan(JNI_TRUE);
#endif

    /* Clean up our working buffer for purify */
    JEMCC_Free(envData.envBuffer);

    (void) fprintf(stderr, "All tests passed successfully (%i forced)\n",
                           failureTotal);
    exit(0);
}

void doValidScan(jboolean print) {
    char *ptr, *str;
    JEM_DescriptorData *data;
    JEM_JNIEnv envData;

    /* Spoof an environment to support buffer usage, but with NO VM */
    envData.envBuffer = envData.envEndPtr = NULL;
    envData.envBufferLength = 0;
    envData.parentVM = NULL;

    /* Look and see cases */
    str = "(BCDFIJSZ)V";
    data = JEM_ParseDescriptor((JNIEnv *) &envData, str, NULL, NULL, JNI_FALSE);
    if (data != NULL) {
        ptr = JEM_ConvertMethodDescriptor((JNIEnv *) &envData, "method", data);
        if (ptr != NULL) {
            if (print) fprintf(stderr, "Check descriptor: %s\n%s\n", str, ptr);
            JEMCC_Free(ptr);
        }
        ptr = JEM_ConvertDescriptor((JNIEnv *) &envData, data, 0);
        if (ptr != NULL) JEMCC_Free(ptr);
        JEM_DestroyDescriptor(data, 1);
    }

    envData.envBufferLength = 1;
    str = "(B[[CI[[[[J[S)[B";
    data = JEM_ParseDescriptor((JNIEnv *) &envData, str, NULL, NULL, JNI_FALSE);
    if (data != NULL) {
        ptr = JEM_ConvertMethodDescriptor((JNIEnv *) &envData, "method", data);
        if (ptr != NULL) {
            if (print) fprintf(stderr, "Check descriptor: %s\n%s\n", str, ptr);
            JEMCC_Free(ptr);
        }
        ptr = JEM_ConvertDescriptor((JNIEnv *) &envData, data, 0);
        if (ptr != NULL) JEMCC_Free(ptr);
        JEM_DestroyDescriptor(data, 1);
    }

    envData.envBufferLength = 1;
    str = "(ILjava/lang/Object;[[B[FD)[Ljava/lang/Object;";
    data = JEM_ParseDescriptor((JNIEnv *) &envData, str, NULL, NULL, JNI_FALSE);
    if (data != NULL) {
        ptr = JEM_ConvertMethodDescriptor((JNIEnv *) &envData, "method", data);
        if (ptr != NULL) {
            if (print) fprintf(stderr, "Check descriptor: %s\n%s\n", str, ptr);
            JEMCC_Free(ptr);
        }
        ptr = JEM_ConvertDescriptor((JNIEnv *) &envData, data, 0);
        if (ptr != NULL) JEMCC_Free(ptr);
        JEM_DestroyDescriptor(data, 1);
    }

    /* When done, clear out the environment buffer */
    JEMCC_Free(envData.envBuffer);
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

void JEMCC_Free(void *block) {
    free(block);
}

void JEMCC_ThrowStdThrowableIdx(JNIEnv *env, JEMCC_VMClassIndex idx,
                                JEMCC_Object *causeThrowable, const char *msg) {
    char *className = "unknown";
    if (msg == NULL) msg = "(null)";

    if (idx == JEMCC_Class_ClassFormatError) {
        className = "java.lang.ClassFormatError";
    } else {
        (void) fprintf(stderr, "Fatal error: unexpected exception index %i.\n",
                               idx);
        exit(1);
    }
    (void) fprintf(stderr, "Error[%s]: %s.\n", className, msg);

    (void) strcpy(exClassName, className);
    (void) strcpy(exMsg, msg);
}

void *JEMCC_StrDupFn(JNIEnv *env, void *key) {
    char *newKey = JEMCC_Malloc(env, strlen(key) + 1);

    if (newKey == NULL) return NULL;
    (void) strcpy(newKey, (char *) key);
    return (void *) newKey;
}
