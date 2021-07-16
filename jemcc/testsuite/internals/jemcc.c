/**
 * JEM test program to test the JEMCC functional interfaces.
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
extern JEMCC_Class *JEMCCTestClass_Object;
extern JEMCC_Class *JEMCCTestClass_Serializable;
extern JEMCC_Class *JEMCCTestClass_Class;
extern JEMCC_Class *JEMCCTestClass_Runnable;
extern JEMCC_Class *JEMCCTestClass_Throwable;
extern jint initializeTestCoreClasses(JNIEnv *env);
extern JNIEnv *createTestEnv();
extern void destroyTestEnv(JNIEnv *env);

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
int quietMode = JNI_FALSE;
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

/* Test data for the various success and failure cases */
static jint JEMCC_TestMethod(JNIEnv *env, JEMCC_VMFrame *frame,
                             JEMCC_ReturnValue *retVal) {
    return JEMCC_RET_VOID;
}

static struct jemcc_test_data {
    int flags;
    JEMCC_MethodData methods[10];
    int methodCount;
    JEMCC_FieldData fields[25];
    int fieldCount;
    char *msgFragment;
} errJemccTests[] = {
    { ACC_PUBLIC, 
      { { ACC_PUBLIC | ACC_ABSTRACT,
          "run", "()V",
          JEMCC_TestMethod } }, 1,
      { }, 0,
      "cannot have code"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "()V",
          NULL } }, 1,
      { }, 0,
      "require code"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "(XYZ)I",
          JEMCC_TestMethod } }, 1,
      { }, 0,
      "character in descriptor"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "Ljava/lang/Object;",
          JEMCC_TestMethod } }, 1,
      { }, 0,
      "descriptor for method"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "notTheIFMethod", "()V",
          JEMCC_TestMethod } }, 1,
      { }, 0,
      "method undefined"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "()V",
          JEMCC_TestMethod } }, 1,
      { { ACC_PUBLIC, "testField", "Y", -1 } }, 1,
      "character in descriptor"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "()V",
          JEMCC_TestMethod } }, 1,
      { { ACC_PUBLIC, "testField", "(Ljava/lang/Object;)V", -1 } }, 1,
      "descriptor for field"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "()V",
          JEMCC_TestMethod } }, 1,
      { { ACC_PUBLIC, "testField", "Ljava/lang/Object;", 4 },
        { ACC_PUBLIC, "testFieldB", "Ljava/lang/Object;", 0 } }, 2,
      "field offset (overlap)"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "()V",
          JEMCC_TestMethod } }, 1,
      { { ACC_PUBLIC, "testField", "Ljava/lang/Object;", 0 },
        { ACC_PUBLIC, "testFieldB", "Ljava/lang/Object;", 13 } }, 2,
      "field offset (align)"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "()V",
          JEMCC_TestMethod } }, 1,
      { { ACC_PUBLIC, "testField", "Ljava/lang/Object;", 0 },
        { ACC_PUBLIC, NULL, NULL, -1 } }, 2,
      "field offset (private)"},
    { ACC_PUBLIC, 
      { { ACC_PUBLIC,
          "run", "()V",
          JEMCC_TestMethod } }, 1,
      { { ACC_PUBLIC, "testByte", "B", -1 },
        { ACC_PUBLIC, "testChar", "C", -1 },
        { ACC_PUBLIC, "testDouble", "D", -1 },
        { ACC_PUBLIC, "testFloat", "F", -1 },
        { ACC_PUBLIC, NULL, NULL, 256 },
        { ACC_PUBLIC, "testInt", "I", -1 },
        { ACC_PUBLIC, "testLong", "J", -1 },
        { ACC_PUBLIC, "testShort", "S", -1 },
        { ACC_PUBLIC, "testBool", "Z", -1 },
        { ACC_PUBLIC, "testArr", "[[B", -1 },
        { ACC_PUBLIC, "testObj", "Ljava/lang/Object;", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statByte", "B", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statChar", "C", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statDouble", "D", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statFloat", "F", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statInt", "I", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statLong", "J", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statShort", "S", -1 },
        { ACC_PUBLIC | ACC_STATIC, NULL, NULL, 1024 },
        { ACC_PUBLIC | ACC_STATIC , "statBool", "Z", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statArr", "[[B", -1 },
        { ACC_PUBLIC | ACC_STATIC , "statObj", "Ljava/lang/Object;", -1 } }, 22,
      NULL},
};

/* Forward declarations */
void checkFullJEMCCHierarchies();
void doJEMCCOkScan();

/* Send the JEMCC class definition methods through their paces */
int main(int argc, char *argv[]) {
    JNIEnv *env;
    int i, nErrTests = sizeof(errJemccTests)/sizeof(struct jemcc_test_data);
    int rc, targTest = -1, doDebug = 0, okIdx = 0;
    JEMCC_Class *interfaces[5], *okClasses[5], *retClass;
    JEMCC_LinkData linkInfo[5];
    char *exceptionMsg;

    /* Check for a specific test to be run */
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-d") == 0) {
                doDebug = 1;
            } else {
                targTest = atoi(argv[1]);
                (void) fprintf(stderr, "Running target test %i\n", targTest);
            }
        }
    }

    fprintf(stderr, "Beginning JEMCC operation tests\n");

    /* No memory test just yet */
#ifdef ENABLE_ERRORSWEEP
    testFailureCount = -1;
#endif
    quietMode = JNI_TRUE;

    /* Initialize operating machines */
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, "Fatal test initialization error\n");
        exit(1);
    }

    /* Create our initial base classes (Object, Class, Serializable) */
    if (initializeTestCoreClasses(env) != JNI_OK) {
        (void) fprintf(stderr, "Fatal core initialization error\n");
        exit(1);
    }
    quietMode = JNI_FALSE;

    /* Test the various JEMCC definition failure cases */
    interfaces[0] = JEMCCTestClass_Runnable;
    for (i = 0; i < nErrTests; i++) {
        if ((targTest > 0) && (i != (targTest - 1))) continue;
        *exClassName = *exMsg = '\0';
        rc = JEMCC_CreateStdClass(env, NULL,
                                  errJemccTests[i].flags,
                                  "jemcc.Test",
                                  JEMCCTestClass_Object,
                                  interfaces, 1,
                                  errJemccTests[i].methods,
                                  errJemccTests[i].methodCount,
                                  NULL,
                                  errJemccTests[i].fields,
                                  errJemccTests[i].fieldCount,
                                  NULL, 0, NULL, &retClass);
        exceptionMsg = exMsg;
        if (strlen(exceptionMsg) == 0) exceptionMsg = NULL;
        if (errJemccTests[i].msgFragment != NULL) {
            if (exceptionMsg == NULL) {
                (void) fprintf(stderr, 
                               "Test %i: didn't get error '%s'\n",
                               i + 1, errJemccTests[i].msgFragment);
                exit(1);
            }
            if (strstr(exceptionMsg, 
                       errJemccTests[i].msgFragment) == NULL) {
                (void) fprintf(stderr, 
                               "Test %i: didn't get fragment '%s'\n",
                               i + 1, errJemccTests[i].msgFragment);
                (void) fprintf(stderr, "    Got '%s' instead\n",
                                       exceptionMsg);
                exit(1);
            }
        } else {
            if (exceptionMsg != NULL) {
                (void) fprintf(stderr, 
                               "Test %i: got unexpected error '%s'\n",
                               i + 1, exceptionMsg);
                exit(1);
            }
            okClasses[okIdx++] = retClass;
            if (doDebug != 0) {
                JEM_DumpDebugClass(env, retClass);
            }
        }
    }

    /* Quick tests with the final class - native data alignment */
    *exClassName = *exMsg = '\0';
    if (JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC | ACC_NATIVE_DATA,
                             "jemcc.TestX", okClasses[0],
                             NULL, 0, NULL, 0, NULL,
                             NULL, 0, NULL, 0, NULL, &retClass) == JNI_OK) {
        (void) fprintf(stderr, "Unexpected success of native subclass\n");
        exit(1);
    }
    checkException("ClassFormatError", "native data element", 
                   "non-root ACC_NATIVE_DATA entry");

    /* Method linkage failure */
    *exClassName = *exMsg = '\0';
    linkInfo[0].linkType = JEMCC_LINK_METHOD;
    linkInfo[0].extClassRef = okClasses[0];
    linkInfo[0].fieldMethodName = "compareTo";
    linkInfo[0].fieldMethodDesc = "(Lno/such/class;)V";
    *exClassName = *exMsg = '\0';
    if (JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC | ACC_NATIVE_DATA,
                             "jemcc.TestX", JEMCCTestClass_Object,
                             NULL, 0, NULL, 0, NULL,
                             NULL, 0, linkInfo, 1, NULL, &retClass) == JNI_OK) {
        (void) fprintf(stderr, "Unexpected success of bad method link\n");
        exit(1);
    }
    checkException("LinkageError", "Unable to locate method", 
                   "invalid method linkage");

    /* Field linkage failure */
    *exClassName = *exMsg = '\0';
    linkInfo[0].linkType = JEMCC_LINK_FIELD;
    linkInfo[0].extClassRef = okClasses[0];
    linkInfo[0].fieldMethodName = "testLong";
    linkInfo[0].fieldMethodDesc = "S";
    *exClassName = *exMsg = '\0';
    if (JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC | ACC_NATIVE_DATA,
                             "jemcc.TestX", JEMCCTestClass_Object,
                             NULL, 0, NULL, 0, NULL,
                             NULL, 0, linkInfo, 1, NULL, &retClass) == JNI_OK) {
        (void) fprintf(stderr, "Unexpected success of bad field link\n");
        exit(1);
    }
    checkException("LinkageError", "Unable to locate field", 
                   "invalid field linkage");

    /* Bad basic throwable instance (improper superclass) */
    if (JEMCC_CreateStdThrowableClass(env, NULL, "jemcc.TestThrow",
                                      okClasses[0], &retClass) == JNI_OK) {
        (void) fprintf(stderr, "Unexpected success of invalid throw class\n");
        exit(1);
    }
    checkException("LinkageError", "Std throwable",
                   "invalid throwable superclass");

    destroyTestEnv(env);

    /* Run through all of the linking hierarchy cases */
    checkFullJEMCCHierarchies();

    /* Memory failure scanning */
#ifdef ENABLE_ERRORSWEEP
    testFailureCurrentCount = 0;
    doJEMCCOkScan();
    failureTotal = testFailureCurrentCount;
    (void) fprintf(stderr, "Preparing to process %i memory failures\n",
                           failureTotal);
    for (testFailureCount = 1;
             testFailureCount <= failureTotal;
                 testFailureCount++) {
        testFailureCurrentCount = 0;
        doJEMCCOkScan();
    }
#else
    failureTotal = 0;
    doJEMCCOkScan();
#endif

    (void) fprintf(stderr, "All tests passed successfully (%i forced)\n",
                           failureTotal);
    exit(0);
}

/* Linkage hierarchy test cases, exactly match Indigo Hierarchies classes */
static struct jemcc_hier_data {
    int flags;
    char *className;
    int superClassIndex;
    int interfaceIndices[10];
    int interfaceCount;
    JEMCC_MethodData methods[10];
    int methodCount;
} hierJemccTests[] = {
    { ACC_PUBLIC | ACC_INTERFACE, 
     "ZIF", -1, { /* No IFs */ }, 0,
     { /* No methods */ }, 0 },

    { ACC_PUBLIC | ACC_INTERFACE, 
     "NIF", -1, { /* No IFs */ }, 0,
     { { ACC_PUBLIC | ACC_ABSTRACT,
         "methodOne", "()V",
         NULL },
       { ACC_PUBLIC | ACC_ABSTRACT,
         "methodTwo", "()V",
         NULL } }, 2 },

    { ACC_PUBLIC | ACC_INTERFACE, 
     "LIF", -1, { /* No IFs */ }, 0,
     { { ACC_PUBLIC | ACC_ABSTRACT,
         "methodThree", "()V",
         NULL } }, 1 },

    { ACC_PUBLIC | ACC_INTERFACE, 
     "NZIF", -1, { 0 /* ZIF */ }, 1,
     { { ACC_PUBLIC | ACC_ABSTRACT,
         "methodOne", "()V",
         NULL },
       { ACC_PUBLIC | ACC_ABSTRACT,
         "methodTwo", "()V",
         NULL },
       { ACC_PUBLIC | ACC_ABSTRACT,
         "methodThree", "()V",
         NULL } }, 3 },

    { ACC_PUBLIC | ACC_INTERFACE, 
     "ZNIF", -1, { 1, /* NIF */ }, 1,
     { /* No methods */ }, 0 },

    { ACC_PUBLIC | ACC_INTERFACE, 
     "MZNIF", -1, { 0 /* ZIF */, 1 /* NIF */}, 2,
     { { ACC_PUBLIC | ACC_ABSTRACT,
         "methodFour", "()V",
         NULL } }, 1 },

    { ACC_PUBLIC | ACC_INTERFACE, 
     "MNZIF", -1, { 1 /* NIF */, 0 /* ZIF */}, 2,
     { { ACC_PUBLIC | ACC_ABSTRACT,
         "methodFour", "()V",
         NULL } }, 1 },

    { ACC_PUBLIC | ACC_INTERFACE, 
     "MZNLIF", -1, { 4 /* ZNIF */, 2 /* LIF */}, 2,
     { { ACC_PUBLIC | ACC_ABSTRACT,
         "methodFour", "()V",
         NULL } }, 1 },

    { ACC_PUBLIC | ACC_ABSTRACT, 
     "ZA", -1, { /* No IFs */ }, 0,
     { /* No methods */ }, 0 },

    { ACC_PUBLIC | ACC_ABSTRACT, 
     "NA", -1, { /* No IFs */ }, 0,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod },
       { ACC_PUBLIC,
         "methodOne", "()V",
         JEMCC_TestMethod },
       { ACC_PUBLIC | ACC_ABSTRACT,
         "methodTwo", "()V",
         NULL } }, 2 },

    { ACC_PUBLIC | ACC_ABSTRACT, /* -- 10 -- */
     "ZNAMNZIF", 9 /* NA */ , { 6 /* MNZIF */ }, 1,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod },
       { 0,
         "<clinit>", "()V",
         JEMCC_TestMethod } }, 2 },

    { ACC_PUBLIC, 
     "MZNAMNZIF", 10 /* ZNAMNZIF */ , { /* No IFs*/ }, 0,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod },
       { ACC_PUBLIC,
         "methodTwo", "()V",
         JEMCC_TestMethod },
       { ACC_PUBLIC,
         "methodThree", "()V",
         JEMCC_TestMethod },
       { ACC_PUBLIC,
         "methodFour", "()V",
         JEMCC_TestMethod } }, 4 },

    { ACC_PUBLIC,
     "ZC", -1, { /* No IFs */ }, 0,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod} }, 1 },

    { ACC_PUBLIC,
     "NC", -1, { /* No IFs */ }, 0,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodTwo", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodOne", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "<clinit>", "()V",
         JEMCC_TestMethod} }, 4 },

    { ACC_PUBLIC,
     "NZC", 12 /* ZC */, { /* No IFs */ }, 0,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodOne", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodTwo", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodThree", "()V",
         JEMCC_TestMethod} }, 4 },

    { ACC_PUBLIC,
     "ZNC", 13 /* NC */, { /* No IFs */ }, 0,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod} }, 1 },

    { ACC_PUBLIC,
     "MZCNZIF", 12 /* ZC */, { 3 /* NZIF */ }, 1,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodOne", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodFour", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodThree", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodTwo", "()V",
         JEMCC_TestMethod} }, 5 },

    { ACC_PUBLIC,
     "ZNCNIF", 15 /* ZNC */, { 1 /* NIF */ }, 1,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod} }, 1 },

    { ACC_PUBLIC,
     "MNZCMZNIF", 14 /* NZC */, { 5 /* MZNIF */ }, 1,
     { { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod},
       { ACC_PUBLIC,
         "methodFour", "()V",
         JEMCC_TestMethod} }, 2 }
};

void checkFullJEMCCHierarchies() {
    JNIEnv *env;
    JEMCC_Class *superClass, *tstClasses[64], *interfaces[10];
    int nHierTests = sizeof(hierJemccTests)/sizeof(struct jemcc_hier_data);
    jint i, j, rc;

    /* Initialize operating machines */
    quietMode = JNI_TRUE;
    if ((env = createTestEnv()) == NULL) {
        (void) fprintf(stderr, 
                       "Unexpected fatal error during test env setup\n");
        exit(1);
    }

    /* Create our initial base classes (Object, Class, Serializable) */
    initializeTestCoreClasses(env);

    /* Build the hierarchy combinations */
    for (i = 0; i < nHierTests; i++) {
        superClass = JEMCCTestClass_Object;
        if (hierJemccTests[i].superClassIndex >= 0) {
            superClass = tstClasses[hierJemccTests[i].superClassIndex];
        }
        for (j = 0; j < hierJemccTests[i].interfaceCount; j++) {
            interfaces[j] = tstClasses[hierJemccTests[i].interfaceIndices[j]];
        }
        rc = JEMCC_CreateStdClass(env, NULL,
                                  hierJemccTests[i].flags,
                                  hierJemccTests[i].className,
                                  superClass, interfaces, 
                                  hierJemccTests[i].interfaceCount,
                                  hierJemccTests[i].methods,
                                  hierJemccTests[i].methodCount,
                                  NULL, NULL, 0, NULL, 0, NULL, 
                                  &(tstClasses[i]));
        if (rc != JNI_OK) {
            (void) fprintf(stderr, "Unexpected error in hierarchy test %s\n",
                                   hierJemccTests[i].className);
            exit(1);
        }
        /* JEM_DumpDebugClass(env, tstClasses[i]); */
    }

    /* Clean up to validate purify operation */
    destroyTestEnv(env);
    quietMode = JNI_FALSE;
}

static JEMCC_MethodData okObjMethodsA[] = {
    { ACC_PUBLIC,
         "testMethod", "(J)I",
         JEMCC_TestMethod },
    { ACC_PUBLIC,
         "<init>", "()V",
         JEMCC_TestMethod },
    { ACC_PUBLIC,
         "<clinit>", "()V",
         JEMCC_TestMethod }
};
static JEMCC_FieldData okObjFieldsA[] = {
    { ACC_PUBLIC, "testField", "Ljava/lang/Object;", -1 }
};

static JEMCC_MethodData okObjMethodsB[] = {
    { ACC_PUBLIC,
         "testMethodB", "()V",
         JEMCC_TestMethod }
};
static JEMCC_FieldData okObjFieldsB[] = {
    { ACC_PUBLIC, "testByte", "B", -1 },
    { ACC_PUBLIC, "testChar", "C", -1 },
    { ACC_PUBLIC, "testDouble", "D", -1 },
    { ACC_PUBLIC, "testFloat", "F", -1 },
    { ACC_PUBLIC, "testInt", "I", -1 },
    { ACC_PUBLIC, "testLong", "J", -1 },
    { ACC_PUBLIC, "testShort", "S", -1 },
    { ACC_PUBLIC, "testBool", "Z", -1 },
    { ACC_PUBLIC, "testArr", "[B", -1 },
    { ACC_PUBLIC, "testObj", "Ljava/lang/Object;", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statByte", "B", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statChar", "C", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statDouble", "D", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statFloat", "F", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statInt", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statLong", "J", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statShort", "S", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statBool", "Z", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statArr", "[B", -1 },
    { ACC_PUBLIC | ACC_STATIC , "statObj", "Ljava/lang/Object;", -1 } 
};


void doJEMCCOkScan() {
    JNIEnv *env;
    JEMCC_Class *testClass, *cmpClass, *byteArrClass;
    JEMCC_Class *testClasses[5], *cmpClasses[5];
    JEMCC_LinkData linkInfo[10];
    JEMCC_ReturnValue statInit[20];
    jint rc;
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
    initializeTestCoreClasses(env);

#ifdef ENABLE_ERRORSWEEP
    testFailureCount = tmpFailCount;
    testFailureCurrentCount = tmpFailCurrentCount;
#endif

    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC,
                              "jemcc.TestA",
                              JEMCCTestClass_Object,
                              NULL, 0,
                              okObjMethodsA, 3, NULL,
                              okObjFieldsA, 1,
                              NULL, 0, NULL, &testClass);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Unexpected class A error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        destroyTestEnv(env);
        return;
    }

    /* Redefine with slightly different name */
    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC,
                              "jemcc/TestA",
                              JEMCCTestClass_Object,
                              NULL, 0,
                              okObjMethodsA, 3, NULL,
                              okObjFieldsA, 1,
                              NULL, 0, NULL, &cmpClass);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Unexpected class A' error %s\n", exMsg);
        exit(1);
    }
    if ((rc == JNI_OK) && (cmpClass != testClass)) {
        (void) fprintf(stderr, "Class overlay didn't return original\n");
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        destroyTestEnv(env);
        return;
    }

    /* This is really not an array, in any sense of the word! */
    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_FINAL | 
                              ACC_ARRAY | ACC_PRIMITIVE,
                              "[B", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &byteArrClass);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Byte array class create error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        destroyTestEnv(env);
        return;
    }

    /* A really big class, with cross references and static initializers */
    linkInfo[0].linkType = JEMCC_LINK_CLASS;
    linkInfo[0].extClassRef = byteArrClass;
    linkInfo[1].linkType = JEMCC_LINK_CLASS;
    linkInfo[1].extClassRef = testClass;
    linkInfo[2].linkType = JEMCC_LINK_METHOD;
    linkInfo[2].extClassRef = NULL;
    linkInfo[2].fieldMethodName = "testMethod";
    linkInfo[2].fieldMethodDesc = "(J)I";
    linkInfo[3].linkType = JEMCC_LINK_METHOD;
    linkInfo[3].extClassRef = testClass;
    linkInfo[3].fieldMethodName = "<init>";
    linkInfo[3].fieldMethodDesc = "()V";
    linkInfo[4].linkType = JEMCC_LINK_FIELD;
    linkInfo[4].extClassRef = NULL;
    linkInfo[4].fieldMethodName = "statDouble";
    linkInfo[4].fieldMethodDesc = "D";
    linkInfo[5].linkType = JEMCC_LINK_FIELD;
    linkInfo[5].extClassRef = NULL;
    linkInfo[5].fieldMethodName = "testField";
    linkInfo[5].fieldMethodDesc = "Ljava/lang/Object;";
    statInit[0].intVal = 12;
    statInit[1].intVal = 24;
    statInit[2].dblVal = 24.2;
    statInit[3].fltVal = (float) 12.4;
    statInit[4].intVal = 1212;
    statInit[5].longVal = 12122424L;
    statInit[6].intVal = 6;
    statInit[7].intVal = JNI_TRUE;
    statInit[8].objVal = NULL;
    statInit[9].objVal = (JEMCC_Object *) cmpClass;
    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC,
                              "jemcc.TestB", testClass,
                              NULL, 0,
                              okObjMethodsB, 1, NULL,
                              okObjFieldsB, 20,
                              linkInfo, 6, statInit, &testClass);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Test class B create error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }
    /* TODO - validate static initialization values */

    /* Try a throwable instance */
    rc = JEMCC_CreateStdThrowableClass(env, NULL, "jemcc.TestThrow",
                                       JEMCCTestClass_Throwable, &testClass);
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Throwable class create error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }

    /* Run through concurrent definition tests */
    rc = JEMCC_BuildClass(env, NULL, ACC_PUBLIC,
                          "jemcc.XRefA", JEMCCTestClass_Object,
                          NULL, 0, okObjMethodsA, 3, NULL,
                          okObjFieldsA, 1, &(testClasses[0]));
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Unexpected XRefA error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }
    rc = JEMCC_BuildClass(env, NULL, ACC_PUBLIC,
                          "jemcc.XRefB", JEMCCTestClass_Object,
                          NULL, 0, okObjMethodsA, 3, NULL,
                          okObjFieldsA, 1, &(testClasses[1]));
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Unexpected XRefB error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, testClasses[0]);
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_RegisterClasses(env, NULL, testClasses, 2) != JNI_OK) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }

    /* Run through concurrent definition tests */
    rc = JEMCC_BuildClass(env, NULL, ACC_PUBLIC,
                          "jemcc.XRefA", JEMCCTestClass_Object,
                          NULL, 0, okObjMethodsA, 3, NULL,
                          okObjFieldsA, 1, &(cmpClasses[1]));
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Unexpected XRefA' error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }
    rc = JEMCC_BuildClass(env, NULL, ACC_PUBLIC,
                          "jemcc.XRefB", JEMCCTestClass_Object,
                          NULL, 0, okObjMethodsA, 3, NULL,
                          okObjFieldsA, 1, &(cmpClasses[0]));
    if (rc == JNI_ERR) {
        (void) fprintf(stderr, "Unexpected XRefB' error %s\n", exMsg);
        exit(1);
    }
    if (rc == JNI_ENOMEM) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, cmpClasses[1]);
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }
    if (JEMCC_RegisterClasses(env, NULL, cmpClasses, 2) != JNI_OK) {
        /* Clean up to validate purify operation */
        JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
        destroyTestEnv(env);
        return;
    }
    /* Should be completely replaced (note reversal) */
    if ((testClasses[0] != cmpClasses[1]) ||
        (testClasses[1] != cmpClasses[0])) {
        (void) fprintf(stderr, "Overlaid class lists don't match\n");
    }

    /* Clean up to validate purify operation */
    JEM_DestroyClassInstance(env, (JEMCC_Class *) byteArrClass);
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

void JEMCC_Free(void *block) {
    free(block);
}

void JEMCC_ThrowStdThrowableIdx(JNIEnv *env, JEMCC_VMClassIndex idx,
                                JEMCC_Object *causeThrowable, const char *msg) {
    char *className = "unknown";
    if (msg == NULL) msg = "(null)";

    if (idx == JEMCC_Class_ClassFormatError) {
        className = "java.lang.ClassFormatError";
    } else if (idx == JEMCC_Class_AbstractMethodError) {
        className = "java.lang.AbstractMethodError";
    } else if (idx == JEMCC_Class_LinkageError) {
        className = "java.lang.LinkageError";
    } else {
        (void) fprintf(stderr, "Fatal error: unexpected exception index %i.\n",
                               idx);
        exit(1);
    }

    if (quietMode != JNI_FALSE) {
        (void) fprintf(stderr, "Fatal error: exception in quiet mode [%s] %s\n",
                               className, msg);
        exit(1);
    }
    (void) fprintf(stderr, "Error[%s]: %s.\n", className, msg);

    (void) strcpy(exClassName, className);
    (void) strcpy(exMsg, msg);
}

void JEMCC_ThrowStdThrowableByName(JNIEnv *env, JEMCC_Object *src,
                                   const char *className, 
                                   JEMCC_Object *causeThrowable,
                                   const char *msg) {
    if (msg == NULL) msg = "(null)";

    if (quietMode != JNI_FALSE) {
        (void) fprintf(stderr, "Fatal error: exception in quiet mode [%s] %s\n",
                               className, msg);
        exit(1);
    }
    (void) fprintf(stderr, "Error[%s]: %s.\n", className, msg);

    (void) strcpy(exClassName, className);
    (void) strcpy(exMsg, msg);
}

static char msgBuffer[1024];

void JEMCC_ThrowStdThrowableIdxV(JNIEnv *env, JEMCC_VMClassIndex idx,
                                 JEMCC_Object *causeThrowable, ...) {
    va_list ap;
    char *str;

    *msgBuffer = '\0';
    va_start(ap, causeThrowable);
    str = va_arg(ap, char *);
    while (str != NULL) {
        (void) strcat(msgBuffer, str);
        str = va_arg(ap, char *);
    }
    va_end(ap);

    JEMCC_ThrowStdThrowableIdx(env, idx, causeThrowable, msgBuffer);
}

void JEMCC_ThrowStdThrowableByNameV(JNIEnv *env, JEMCC_Object *src,
                                    const char *className,  
                                    JEMCC_Object *causeThrowable, ...) {
    va_list ap;
    char *str;

    *msgBuffer = '\0';
    va_start(ap, causeThrowable);
    str = va_arg(ap, char *);
    while (str != NULL) {
        (void) strcat(msgBuffer, str);
        str = va_arg(ap, char *);
    }
    va_end(ap);

    JEMCC_ThrowStdThrowableByName(env, src, className, causeThrowable,
                                  msgBuffer);
}

JEMCC_Object *JEMCC_AllocateObject(JNIEnv *env, JEMCC_Class *classInst,
                                   juint objDataSize) {
    JEMCC_Object *retVal = NULL;
    unsigned int totalSize = sizeof(JEMCC_Object);

    /* Add packing space for object field information */
    if (classInst != NULL) {
        totalSize += classInst->classData->packedFieldSize;
    } else {
        /* NULL case is the core class initializers - size appropriately */
        totalSize = sizeof(JEMCC_Class);
    }

    /* Create the memory space */
    if ((retVal = (JEMCC_Object *) JEMCC_Malloc(env, totalSize)) == NULL) {
        return NULL;
    }
    if (objDataSize > 0) {
        ((JEMCC_ObjectExt *) retVal)->objectData = JEMCC_Malloc(env,
                                                                objDataSize);
        if (((JEMCC_ObjectExt *) retVal)->objectData == NULL) {
            JEMCC_Free(retVal);
            return NULL;
        }
    }

    /* Initialize the main object structure elements */
    retVal->classReference = classInst;
    retVal->objStateSet = 0;

    return retVal;
}
