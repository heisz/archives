/**
 * Methods for bundling library extension for package test program.
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

/* All of these common methods have been moved here for access by test libs */

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
