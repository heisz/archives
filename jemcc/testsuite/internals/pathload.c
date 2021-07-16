/**
 * JEMCC test program to validate path loading mechanisms.
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

/* Test pathway (separate for multi-os typing) */
static char *realPath = "::nofile.zip::./.libs:./libdata:./zipdata/ok.zip:/usr/lib/:./zipdata/nosig.zip:/no_such_dir::.:::";

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

/* Forward declarations */
void doValidScan(jboolean fullsweep);

/* Main program will send the zipfile routines through their paces */
int main(int argc, char *argv[]) {
    JEM_JNIEnv envData;
    JEM_PathEntryList path;
    JEM_DynaLibLoader loader;
    JEM_DynaLib libHandle;
    jbyte *readBuffer;
    jsize readSize;

    /* Build a basic test environment instance */
    envData.envBuffer = envData.envEndPtr = NULL;
    envData.envBufferLength = 0;
    envData.parentVM = NULL;

    /* Try some null conditions first */
    if (JEM_ParsePathList((JNIEnv *) &envData, &path, 
                          NULL, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Failed to parse NULL path\n");
        exit(1);
    }
    JEM_DestroyPathList(NULL, &path);
    if (JEM_ParsePathList((JNIEnv *) &envData, &path, 
                          "", JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Failed to parse empty path\n");
        exit(1);
    }
    JEM_DestroyPathList(NULL, &path);

    /* Try a real path, including a good-zip/bad-zip routine */
    if (JEM_ParsePathList((JNIEnv *) &envData, &path, 
                          realPath, JNI_TRUE) != JNI_OK) {
        (void) fprintf(stderr, "Failed to parse realpath instance\n");
        exit(1);
    }

    /* Attempt some direct file load operations */
    if (JEM_ReadPathFileContents((JNIEnv *) &envData, &path,
                                 "no_such_file_exists", &readBuffer,
                                 &readSize) != JNI_EINVAL) {
        (void) fprintf(stderr, "Incorrect return for invalid file read\n");
        exit(1);
    }
    if (JEM_ReadPathFileContents((JNIEnv *) &envData, &path,
                                 "pathload.c", &readBuffer,
                                 &readSize) != JNI_OK) {
        (void) fprintf(stderr, "Incorrect return for self read\n");
        exit(1);
    }
    JEMCC_Free(readBuffer);
    if (JEM_ReadPathFileContents((JNIEnv *) &envData, &path,
                                 "zipdata/ok.zip", &readBuffer,
                                 &readSize) != JNI_OK) {
        (void) fprintf(stderr, "Incorrect return for zip file read\n");
        exit(1);
    }
    if ((readSize != 979) || (*readBuffer != 0x50) ||
        (*(readBuffer + 1) != 0x4b) || (*(readBuffer + 2) != 0x03)) {
        (void) fprintf(stderr, "Invalid read data from ok.zip file\n");
        exit(1);
    }
    JEMCC_Free(readBuffer);

    /* And a file load or two from within a Zip file */
    if (JEM_ReadPathFileContents((JNIEnv *) &envData, &path,
                                 "ten", &readBuffer,
                                 &readSize) != JNI_OK) {
        (void) fprintf(stderr, "Incorrect return for uncompressed file read\n");
        exit(1);
    }
    if ((readSize != 10) || (*readBuffer != '0') ||
        (*(readBuffer + 1) != '1') || (*(readBuffer + 2) != '2')) {
        (void) fprintf(stderr, "Invalid zip read data for 'ten'\n");
        exit(1);
    }
    JEMCC_Free(readBuffer);
    if (JEM_ReadPathFileContents((JNIEnv *) &envData, &path,
                                 "hundred", &readBuffer,
                                 &readSize) != JNI_OK) {
        (void) fprintf(stderr, "Incorrect return for compressed file read\n");
        exit(1);
    }
    if ((readSize != 100) || (*readBuffer != '0') ||
        (*(readBuffer + 1) != '1') || (*(readBuffer + 2) != '2')) {
        (void) fprintf(stderr, "Invalid zip read data for 'hundred'\n");
        exit(1);
    }
    JEMCC_Free(readBuffer);

    /* Try some library loads as well */
    loader = JEM_DynaLibLoaderInit();
    if (JEM_LoadPathLibrary((JNIEnv *) &envData, loader, &path, 
                            "/no/such/library/exists",
                            NULL, &libHandle) != JNI_EINVAL) {
        (void) fprintf(stderr, "Invalid return for impossible abs lib load\n");
        exit(1);
    }
    if (JEM_LoadPathLibrary((JNIEnv *) &envData, loader, &path, 
                            "no_such_library_exists",
                            NULL, &libHandle) != JNI_EINVAL) {
        (void) fprintf(stderr, "Invalid return for impossible rel lib load\n");
        exit(1);
    }
    quietMode = JNI_FALSE;
    *exClassName = *exMsg = '\0';
    if (JEM_LoadPathLibrary((JNIEnv *) &envData, loader, &path, "/etc/hosts",
                            NULL, &libHandle) != JNI_ERR) {
        (void) fprintf(stderr, "Invalid return for abs non-lib load\n");
        exit(1);
    }
    checkException("UnsatisfiedLinkError", NULL, "invalid file");
    *exClassName = *exMsg = '\0';
    if (JEM_LoadPathLibrary((JNIEnv *) &envData, loader, &path, "libdfctv.so",
                            NULL, &libHandle) != JNI_ERR) {
        (void) fprintf(stderr, "Invalid return for rel non-lib load\n");
        exit(1);
    }
    checkException("UnsatisfiedLinkError", NULL, "invalid file");
    *exClassName = *exMsg = '\0';
    if (JEM_LoadPathLibrary((JNIEnv *) &envData, loader, &path, 
                            "/usr/lib/libm.so", NULL, &libHandle) != JNI_OK) {
        (void) fprintf(stderr, "Invalid return for abs library load\n");
        exit(1);
    }
    if (JEM_LoadPathLibrary((JNIEnv *) &envData, loader, &path, 
                            "libffitest.so", NULL, &libHandle) != JNI_OK) {
        (void) fprintf(stderr, "Invalid return for abs library load\n");
        exit(1);
    }
    JEM_DynaLibLoaderDestroy(loader);

    /* All done with the real path, clean up */
    JEM_DestroyPathList(NULL, &path);
    JEMCC_Free(envData.envBuffer);

    /* Memory/data failure scanning */
#ifdef ENABLE_ERRORSWEEP
    testFailureCurrentCount = 0;
    doValidScan(JNI_TRUE);
    failureTotal = testFailureCurrentCount;
    (void) fprintf(stderr, "Preparing to process %i processing failures\n",
                           failureTotal);
    for (testFailureCount = 1;
             testFailureCount <= failureTotal;
                 testFailureCount++) {
        testFailureCurrentCount = 0;
        doValidScan(JNI_FALSE);
    }
#else
    failureTotal = 0;
#endif

    (void) fprintf(stderr, "All tests passed successfully (%i forced)\n",
                           failureTotal);
    return 0;
}

void doValidScan(jboolean fullsweep) {
    JEM_JNIEnv envData;
    JEM_PathEntryList path;
    JEM_DynaLibLoader loader;
    JEM_DynaLib libHandle;
    jbyte *readBuffer;
    jsize readSize;

    /* Build a basic test environment instance */
    envData.envBuffer = envData.envEndPtr = NULL;
    envData.envBufferLength = 0;
    envData.parentVM = NULL;

    /* Parse a mixed path */
    if (JEM_ParsePathList((JNIEnv *) &envData, &path, 
                          realPath, JNI_TRUE) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: failed to parse path info\n");
            exit(1);
        }
        JEM_DestroyPathList(NULL, &path);
        JEMCC_Free(envData.envBuffer);
        return;
    }

    /* Read a direct file and a zip contents file */
    if (JEM_ReadPathFileContents((JNIEnv *) &envData, &path,
                                 "pathload.c", &readBuffer,
                                 &readSize) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: failed to read direct file\n");
            exit(1);
        }
        JEM_DestroyPathList(NULL, &path);
        JEMCC_Free(envData.envBuffer);
        return;
    }
    JEMCC_Free(readBuffer);
    if (JEM_ReadPathFileContents((JNIEnv *) &envData, &path,
                                 "hundred", &readBuffer,
                                 &readSize) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: failed to read zip file entry\n");
            exit(1);
        }
        JEM_DestroyPathList(NULL, &path);
        JEMCC_Free(envData.envBuffer);
        return;
    }
    JEMCC_Free(readBuffer);

    /* Try some dynamic library acquisition as well */
    loader = JEM_DynaLibLoaderInit();
    if (JEM_LoadPathLibrary((JNIEnv *) &envData, loader, &path, 
                            "libffitest.so", NULL, &libHandle) != JNI_OK) {
        if (fullsweep == JNI_TRUE) {
            (void) fprintf(stderr, "Prescan: failed to load library\n");
            exit(1);
        }
        JEM_DestroyPathList(NULL, &path);
        JEMCC_Free(envData.envBuffer);
        return;
    }
    JEM_DynaLibLoaderDestroy(loader);

    /* All done with the path, clean up */
    JEM_DestroyPathList(NULL, &path);
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

    if (idx == JEMCC_Class_IOException) {
        className = "java.io.IOException";
    } else if (idx == JEMCC_Class_OutOfMemoryError) {
        className = "java.lang.OutOfMemoryError";
    } else if (idx == JEMCC_Class_InternalError) {
        className = "java.lang.InternalError";
    } else if (idx == JEMCC_Class_UnsatisfiedLinkError) {
        className = "java.lang.UnsatisfiedLinkError";
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

void *JEMCC_StrDupFn(JNIEnv *env, void *key) {
    char *newKey = JEMCC_Malloc(env, strlen(key) + 1);

    if (newKey == NULL) return NULL;
    (void) strcpy(newKey, (char *) key);
    return (void *) newKey;
}
