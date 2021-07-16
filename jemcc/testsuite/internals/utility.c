/**
 * JEMCC test program to test the general utility functions of the VM.
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

/* Forward declarations, see below */
void doValidEnvStrScan(jboolean fullsweep);
void doValidHashScan(jboolean fullsweep);

/* Main program will send the class linker through its paces */
int main(int argc, char *argv[]) {
    char bigTable[1024][64], *origKey, *origObj;
    JEMCC_HashTable hashTable, dupHashTable;
    int i, j;

    /* 'Conventional' tests first, do not force internal errors */
#ifdef ENABLE_ERRORSWEEP
    testFailureCount = -1;
#endif

    fprintf(stderr, "Beginning hashtable tests\n");

    /* Really fill a hashtable, without memory interference */
    if (JEMCC_HashInitTable(NULL, &hashTable, -1) != JNI_OK) {
        (void) fprintf(stderr, "Error: unexpected memory failure\n");
        exit(1);
    }
    for (i = 0; i < 1024; i++) {
        (void) sprintf(bigTable[i], "%i", i);
        if (JEMCC_HashInsertEntry(NULL, &hashTable, bigTable[i], bigTable[i],
                                  NULL, NULL, JEMCC_StrHashFn, 
                                  JEMCC_StrEqualsFn) != JNI_OK) {
            (void) fprintf(stderr, "Big table insert failure\n");
            exit(1);
        }
    }
    for (i = 0; i < 512; i++) {
        j = rand() & 1023;
        if ((j & 1) != 0) {
            JEMCC_HashRemoveEntry(NULL, &hashTable, bigTable[j], NULL, NULL,
                                  JEMCC_StrHashFn, JEMCC_StrEqualsFn);
        } else {
            JEMCC_HashScanRemoveEntry(NULL, &hashTable, bigTable[j],
                                      JEMCC_StrHashFn, JEMCC_StrEqualsFn);
        }
    }
    for (i = 0; i < 1024; i++) {
        if (JEMCC_HashPutEntry(NULL, &hashTable, bigTable[i], bigTable[i],
                               NULL, NULL,
                               JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_OK) {
            (void) fprintf(stderr, "Big, empty table insert failed\n");
            exit(1);
        }
    }
    for (i = 0; i < 512; i++) {
        j = rand() & 1023;
        if ((j & 1) != 0) {
            JEMCC_HashRemoveEntry(NULL, &hashTable, bigTable[j], NULL, NULL,
                                  JEMCC_StrHashFn, JEMCC_StrEqualsFn);
        } else {
            JEMCC_HashScanRemoveEntry(NULL, &hashTable, bigTable[j],
                                      JEMCC_StrHashFn, JEMCC_StrEqualsFn);
        }
    }
    if (JEMCC_HashDuplicate(NULL, &dupHashTable, 
                            &hashTable, NULL) != JNI_OK) {
        (void) fprintf(stderr, "Unexpected memory failure on duplicate\n");
        exit(1);
    }
    for (i = 0; i < 1024; i++) {
        if (JEMCC_HashGetFullEntry(NULL, &hashTable, bigTable[i],
                                   (void *) &origKey, (void *) &origObj,
                                   JEMCC_StrHashFn, 
                                   JEMCC_StrEqualsFn) != JNI_ERR) {
            if ((origKey != bigTable[i]) || (origObj != bigTable[i])) {
                (void) fprintf(stderr, "Big table entry missing?\n");
                exit(1);
            }
            if (JEMCC_HashGetEntry(NULL, &hashTable, bigTable[i],
                                   JEMCC_StrHashFn, 
                                   JEMCC_StrEqualsFn) != origObj) {
                (void) fprintf(stderr, "Big table get mismatch?\n");
                exit(1);
            }
        } else {
            if ((origKey != NULL) || (origObj != NULL)) {
                (void) fprintf(stderr, "Big table data return for non-entry\n");
                exit(1);
            }
            if (JEMCC_HashGetEntry(NULL, &hashTable, bigTable[i],
                                   JEMCC_StrHashFn, 
                                   JEMCC_StrEqualsFn) != NULL) {
                (void) fprintf(stderr, "Big table get data for non-entry\n");
                exit(1);
            }
        }
    }
    JEMCC_HashDestroyTable(&hashTable);
    JEMCC_HashDestroyTable(&dupHashTable);

    /* Memory failure scanning */
#ifdef ENABLE_ERRORSWEEP
    testFailureCurrentCount = 0;
    doValidEnvStrScan(JNI_TRUE);
    doValidHashScan(JNI_TRUE);
    failureTotal = testFailureCurrentCount;
    (void) fprintf(stderr, "Preparing to process %i forced failures\n",
                           failureTotal);
    for (testFailureCount = 1; 
             testFailureCount <= failureTotal;
                 testFailureCount++) {
        testFailureCurrentCount = 0;
        doValidEnvStrScan(JNI_FALSE);
        doValidHashScan(JNI_FALSE);
    }
#else
    failureTotal = 0;
    doValidHashScan(JNI_TRUE);
#endif

    (void) fprintf(stderr, "All tests passed successfully (%i forced)\n",
                           failureTotal);
    exit(0);
}

void doValidEnvStrScan(jboolean fullsweep) {
    JEM_JNIEnv envData;
    char *ptr;

    /* Spoof an environment to support buffer usage, but with no VM */
    envData.envBuffer = envData.envEndPtr = NULL;
    envData.envBufferLength = 0;
    envData.parentVM = NULL;

    /* Standard initialize */
    ptr = JEMCC_EnvStrBufferInit((JNIEnv *) &envData, 16);
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on envstr init\n");
            exit(1);
        }
        return;
    }
    if (strlen(ptr) != 0) {
        (void) fprintf(stderr, "Invalid result on envstr init\n");
        exit(1);
    }

    /* Append, to empty */
    ptr = JEMCC_EnvStrBufferAppend((JNIEnv *) &envData, "Testing 123...");
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on empty append\n");
            exit(1);
        }
        JEMCC_Free(envData.envBuffer);
        return;
    }
    if (strcmp(ptr, "Testing 123...") != 0) {
        (void) fprintf(stderr, "Invalid empty append result\n");
        exit(1);
    }

    /* Append, to current */
    ptr = JEMCC_EnvStrBufferAppend((JNIEnv *) &envData, "456...");
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on append\n");
            exit(1);
        }
        JEMCC_Free(envData.envBuffer);
        return;
    }
    if (strcmp(ptr, "Testing 123...456...") != 0) {
        (void) fprintf(stderr, "Invalid append result\n");
        exit(1);
    }

    /* Append list */
    ptr = JEMCC_EnvStrBufferAppendSet((JNIEnv *) &envData, "789" , "..." ,
                                      "10!", (char *) NULL);
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on list append\n");
            exit(1);
        }
        JEMCC_Free(envData.envBuffer);
        return;
    }
    if (strcmp(ptr, "Testing 123...456...789...10!") != 0) {
        (void) fprintf(stderr, "Invalid list append result\n");
        exit(1);
    }

    /* Re-initialize */
    ptr = JEMCC_EnvStrBufferInit((JNIEnv *) &envData, 1025);
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on envstr re-init\n");
            exit(1);
        }
        JEMCC_Free(envData.envBuffer);
        return;
    }
    if (strlen(ptr) != 0) {
        (void) fprintf(stderr, "Invalid result on envstr re-init\n");
        exit(1);
    }

    /* Append empty list */
    ptr = JEMCC_EnvStrBufferAppendSet((JNIEnv *) &envData, (char *) NULL);
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on null list append\n");
            exit(1);
        }
        JEMCC_Free(envData.envBuffer);
        return;
    }
    if (strlen(ptr) != 0) {
        (void) fprintf(stderr, "Invalid null list append result\n");
        exit(1);
    }

    /* Append list to empty */
    ptr = JEMCC_EnvStrBufferAppendSet((JNIEnv *) &envData, "1", "2", "34",
                                      "5", "678", "9", (char *) NULL);
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on null list append\n");
            exit(1);
        }
        JEMCC_Free(envData.envBuffer);
        return;
    }
    if (strcmp(ptr, "123456789") != 0) {
        (void) fprintf(stderr, "Invalid empty list append result\n");
        exit(1);
    }

    ptr = JEMCC_EnvStrBufferDup((JNIEnv *) &envData);
    if (ptr == NULL) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on envstr dup\n");
            exit(1);
        }
        JEMCC_Free(envData.envBuffer);
        return;
    }
    if (strcmp(ptr, "123456789") != 0) {
        (void) fprintf(stderr, "Invalid envstr dup result\n");
        exit(1);
    }

    JEMCC_Free(envData.envBuffer);
    JEMCC_Free(ptr);
}

jint hashEntryCB(JNIEnv *env, JEMCC_HashTable *table, void *key, 
                 void *obj, void *userData) {
    JEMCC_HashScanRemoveEntry(env, table, key, 
                              JEMCC_StrHashFn, JEMCC_StrEqualsFn);
    JEMCC_Free(key);
    return ((userData == (void *) 0) ? JNI_OK : JNI_ERR);
}

void doValidHashScan(jboolean fullsweep) {
    JEMCC_HashTable hashTable, dupHashTable;
    char testKey[1024], *testObject = "object";
    char *compKey = "abcdefghijklmnopqrstuvwxyz", *compObject = "hello world";
    void *oldKey, *oldObject;
    jint rc;

    /* Hashtable exercises */
    if (JEMCC_HashInitTable(NULL, &hashTable, -1) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on table init\n");
            exit(1);
        }
        return;
    }
    JEMCC_HashDestroyTable(&hashTable);
    if (JEMCC_HashInitTable(NULL, &hashTable, -1) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on second table init\n");
            exit(1);
        }
        return;
    }

    (void) sprintf(testKey, compKey);
    if (JEMCC_HashPutEntry(NULL, &hashTable, testKey, testObject,
                           NULL, NULL, JEMCC_StrHashFn, 
                           JEMCC_StrEqualsFn) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on hash put\n");
            exit(1);
        }
        JEMCC_HashDestroyTable(&hashTable);
        return;
    }
    if (JEMCC_HashGetEntry(NULL, &hashTable, compKey, 
                           JEMCC_StrHashFn, JEMCC_StrEqualsFn) != testObject) {
        (void) fprintf(stderr, "Unexpected hash mismatch\n");
        exit(1);
    }
    if (JEMCC_HashGetEntry(NULL, &hashTable, "notachance", 
                           JEMCC_StrHashFn, JEMCC_StrEqualsFn) != NULL) {
        (void) fprintf(stderr, "Unexpected hash get on non-key\n");
        exit(1);
    }
    if (JEMCC_HashRemoveEntry(NULL, &hashTable, "nokey", NULL, NULL,
                              JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_ERR) {
        (void) fprintf(stderr, "Unexpected hash remove on non-key\n");
        exit(1);
    }
    if (JEMCC_HashRemoveEntry(NULL, &hashTable, compKey, &oldKey, &oldObject,
                              JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_OK) {
        (void) fprintf(stderr, "First hash remove failed\n");
        exit(1);
    }
    if ((oldKey != testKey) || (oldObject != testObject)) {
        (void) fprintf(stderr, "Hash remove not returning originals\n");
        exit(1);
    }

    if (JEMCC_HashPutEntry(NULL, &hashTable, testKey, testObject,
                           &oldKey, &oldObject, 
                           JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on hash put\n");
            exit(1);
        }
        JEMCC_HashDestroyTable(&hashTable);
        return;
    }
    if ((oldKey != NULL) || (oldObject != NULL)) {
        (void) fprintf(stderr, "New insertion returning an original?\n");
        exit(1);
    }
    if (JEMCC_HashPutEntry(NULL, &hashTable, compKey, compObject,
                           &oldKey, &oldObject, 
                           JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on hash re-put\n");
            exit(1);
        }
        JEMCC_HashDestroyTable(&hashTable);
        return;
    }
    if ((oldKey != testKey) || (oldObject != testObject)) {
        (void) fprintf(stderr, "New insertion not returning real original\n");
        exit(1);
    }
    if (JEMCC_HashGetFullEntry(NULL, &hashTable, testKey, NULL, NULL,
                               JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_OK) {
        (void) fprintf(stderr, "Full get did not find entry\n");
        exit(1);
    }
    if (JEMCC_HashGetFullEntry(NULL, &hashTable, testKey, &oldKey, &oldObject,
                               JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_OK) {
        (void) fprintf(stderr, "Full get did not find object\n");
        exit(1);
    }
    if ((oldKey != compKey) || (oldObject != compObject)) {
        (void) fprintf(stderr, "Full get did not get correct key/object\n");
        exit(1);
    }

    rc = JEMCC_HashInsertEntry(NULL, &hashTable, "dummy", "entry", NULL, NULL,
                               JEMCC_StrHashFn, JEMCC_StrEqualsFn);
    if (rc == JNI_ENOMEM) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: mem error on hash insert\n");
            exit(1);
        }
        JEMCC_HashDestroyTable(&hashTable);
        return;
    }
    if (rc != JNI_OK) {
        (void) fprintf(stderr, "Insert failed with new entry\n");
        exit(1);
    }
    if (JEMCC_HashInsertEntry(NULL, &hashTable, "dummy", "entry", NULL, NULL,
                              JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_ERR) {
        (void) fprintf(stderr, "Insert succeeded with duplicate\n");
        exit(1);
    }
    if (JEMCC_HashInsertEntry(NULL, &hashTable, testKey, "nodata",  
                              &oldKey, &oldObject,
                              JEMCC_StrHashFn, JEMCC_StrEqualsFn) != JNI_ERR) {
        (void) fprintf(stderr, "2nd insert succeeded with duplicate\n");
        exit(1);
    }
    if ((oldKey != compKey) || (oldObject != compObject)) {
        (void) fprintf(stderr, "Insert did not return original\n");
        exit(1);
    }

    if (JEMCC_HashDuplicate(NULL, &dupHashTable, &hashTable, 
                            JEMCC_StrDupFn) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on hash duplicate\n");
            exit(1);
        }
        JEMCC_HashDestroyTable(&hashTable);
        JEMCC_HashScan(NULL, &dupHashTable, hashEntryCB, (void *) 0);
        JEMCC_HashDestroyTable(&dupHashTable);
        return;
    }
    if (JEMCC_HashGetEntry(NULL, &hashTable, compKey, 
                           JEMCC_StrHashFn, JEMCC_StrEqualsFn) != compObject) {
        (void) fprintf(stderr, "Unexpected duplicate mismatch\n");
        exit(1);
    }

    JEMCC_HashDestroyTable(&hashTable);
    JEMCC_HashScan(NULL, &dupHashTable, hashEntryCB, (void *) 1);
    JEMCC_HashScan(NULL, &dupHashTable, hashEntryCB, (void *) 0);
    JEMCC_HashDestroyTable(&dupHashTable);

    /* Hashtable exercises with classname comparision functions */
    if (JEMCC_HashInitTable(NULL, &hashTable, -1) != JNI_OK) return;
    if (JEMCC_HashPutEntry(NULL, &hashTable, "java/lang/Object", "class",
                           NULL, NULL, JEMCC_ClassNameHashFn, 
                           JEMCC_ClassNameEqualsFn) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on classname put\n");
            exit(1);
        }
        JEMCC_HashDestroyTable(&hashTable);
        return;
    }
    if (JEMCC_HashGetEntry(NULL, &hashTable, "java.lang.Object", 
                           JEMCC_ClassNameHashFn, 
                           JEMCC_ClassNameEqualsFn) == NULL) {
        (void) fprintf(stderr, "Class name get mismatch\n");
        exit(1);
    }
    if (JEMCC_HashGetEntry(NULL, &hashTable, "java!lang!Object", 
                           JEMCC_ClassNameHashFn, 
                           JEMCC_ClassNameEqualsFn) != NULL) {
        (void) fprintf(stderr, "Unexpected class name get match\n");
        exit(1);
    }
    if (JEMCC_HashRemoveEntry(NULL, &hashTable, "java/lang/Object", NULL, NULL,
                              JEMCC_ClassNameHashFn, 
                              JEMCC_ClassNameEqualsFn) != JNI_OK) {
        (void) fprintf(stderr, "Class name remove failed\n");
        exit(1);
    }

    if (JEMCC_HashPutEntry(NULL, &hashTable, "java.lang.Object", "class",
                           NULL, NULL, JEMCC_ClassNameHashFn, 
                           JEMCC_ClassNameEqualsFn) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: error on 2nd classname put\n");
            exit(1);
        }
        JEMCC_HashDestroyTable(&hashTable);
        return;
    }
    if (JEMCC_HashGetEntry(NULL, &hashTable, "java/lang/Object", 
                           JEMCC_ClassNameHashFn, 
                           JEMCC_ClassNameEqualsFn) == NULL) {
        (void) fprintf(stderr, "2 - Class name get mismatch\n");
        exit(1);
    }
    if (JEMCC_HashGetEntry(NULL, &hashTable, "java!lang!Object", 
                           JEMCC_ClassNameHashFn, 
                           JEMCC_ClassNameEqualsFn) != NULL) {
        (void) fprintf(stderr, "2 - Unexpected class name get match\n");
        exit(1);
    }
    if (JEMCC_HashRemoveEntry(NULL, &hashTable, "java.lang.Object", NULL, NULL,
                              JEMCC_ClassNameHashFn, 
                              JEMCC_ClassNameEqualsFn) != JNI_OK) {
        (void) fprintf(stderr, "2 - Class name remove failed\n");
        exit(1);
    }

    JEMCC_HashDestroyTable(&hashTable);
    if (JEMCC_ClassNameEqualsFn(NULL, "java.lang.Object", 
                                      "java/lang/Object") == 0) {
        (void) fprintf(stderr, "Separator classname compare failed\n");
        exit(1);
    }
    if (JEMCC_ClassNameEqualsFn(NULL, "java.lang/Object", 
                                      "java!lang!Object") != 0) {
        (void) fprintf(stderr, "Separator classname compare succeeded\n");
        exit(1);
    }
    if (JEMCC_ClassNameEqualsFn(NULL, "java!lang!Object", 
                                      "java.lang/Object") != 0) {
        (void) fprintf(stderr, "Separator classname compare succeeded\n");
        exit(1);
    }
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
