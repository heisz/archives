/**
 * JEMCC test program to validate Zip file access methods.
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

/* Test flags/data values for condition setups and tests */
int failureTotal;
#ifdef ENABLE_ERRORSWEEP
int testFailureCurrentCount, testFailureCount;
int disableSweep = JNI_FALSE;

int JEM_CheckErrorSweep(int sweepType) {
    /* Note: don't care about sweep type */
    if (disableSweep != JNI_FALSE) return JNI_FALSE;
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

/* Forward declaration, see below */
void doValidReadScan(jboolean fullsweep);
void doValidStreamScan(jboolean fullsweep);

/* Main program will send the zipfile routines through their paces */
int main(int argc, char *argv[]) {
    JEMCC_ZipFile *zf;
    JEMCC_ZipFileEntry entry;
    JEMCC_ZipFileStream *stream;
    jbyte streamBuffer[1024];
    int rc, mode;

    /* Data failure tests first, do not force internal errors */
#ifdef ENABLE_ERRORSWEEP
    testFailureCount = -1;
#endif

    /********** Zip File/Directory Failures ***********/

    /* Data failure cases, primary zip file header data */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "no_such_file", &zf, 
                          JNI_TRUE, JNI_FALSE) == JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected nofile open success\n");
        exit(1);
    }
    checkException("ZipException", "not found", "non-existent file");

    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/nosig.zip", &zf, 
                          JNI_TRUE, JNI_FALSE) == JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected zipfile open success\n");
        exit(1);
    }
    checkException("ZipException", "missing signature", "unsigned file");

    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/multipart.zip", &zf, 
                          JNI_TRUE, JNI_FALSE) == JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected multizip open success\n");
        exit(1);
    }
    checkException("ZipException", "Multi-part", "multi-part file");

    /* Data failure cases, zip file directory header data */
    /* In prescan, should be found on opening */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/inventsig.zip", &zf, 
                          JNI_TRUE, JNI_FALSE) == JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected corrupt zip open success\n");
        exit(1);
    }
    checkException("ZipException", "entry signature", "invalid entry sig");

    /* Otherwise, found during entry lookup */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/inventsig.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected entry zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "zero",
                               &entry, JNI_FALSE) != JNI_ERR) {
        (void) fprintf(stderr, "Error: Failed detect of bad dir signature\n");
        exit(1);
    }
    checkException("ZipException", "entry signature", "invalid entry sig");
    JEMCC_CloseZipFile(NULL, zf);

    /********** Bulk Read Failures ***********/

    /* Invalid data entry signature (prescan irrelevant) */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/invdatasig.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected data zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "hundred",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected entry lookup error\n");
        exit(1);
    }
    if (JEMCC_ReadZipFileEntry(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful read of corrupt data\n");
        exit(1);
    }
    checkException("ZipException", "data signature", "invalid data sig");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Invalid compression sizing (if empty) */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/badempty.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected empty zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "zero",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected empty entry lookup error\n");
        exit(1);
    }
    if (JEMCC_ReadZipFileEntry(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful read of mis-sized data\n");
        exit(1);
    }
    checkException("ZipException", "entry length", "invalid empty file");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Invalid compression method */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/invmethod.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected method zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "fifty",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected method entry lookup error\n");
        exit(1);
    }
    if (JEMCC_ReadZipFileEntry(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful read of odd method data\n");
        exit(1);
    }
    checkException("ZipException", "Unsupported", "invalid compression method");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Sizing overflow */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/overflow.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected flood zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "fifty",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected flood entry lookup error\n");
        exit(1);
    }
    if (JEMCC_ReadZipFileEntry(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful read of overflow data\n");
        exit(1);
    }
    checkException("ZipException", "Invalid data", "data overflow");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Or maybe just a nice corrupted file or two */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/mangledfile.zip", &zf, 
                          JNI_TRUE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected mangled data open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "fifty",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected mangled data lookup error\n");
        exit(1);
    }
    if (JEMCC_ReadZipFileEntry(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful read of mangled data\n");
        exit(1);
    }
    checkException("ZipException", "Invalid data", "mangled file");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/corruptfile.zip", &zf, 
                          JNI_TRUE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected corrupt data open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "fifty",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected corrupt data lookup error\n");
        exit(1);
    }
    if (JEMCC_ReadZipFileEntry(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful read of corrupted data\n");
        exit(1);
    }
    checkException("ZipException", "Corrupt", "corrupt file");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /********** Stream Read Failures ***********/

    /* Invalid data entry signature (prescan irrelevant) */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/invdatasig.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected data zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "hundred",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected entry lookup error\n");
        exit(1);
    }
    if (JEMCC_OpenZipFileStream(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful open of corrupt stream\n");
        exit(1);
    }
    checkException("ZipException", "data signature", "invalid data sig");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Invalid compression sizing (if empty) */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/badempty.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected empty zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "zero",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected empty entry lookup error\n");
        exit(1);
    }
    if (JEMCC_OpenZipFileStream(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful open of mis-sized stream\n");
        exit(1);
    }
    checkException("ZipException", "entry length", "invalid empty file");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Invalid compression method */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/invmethod.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected method zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "fifty",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected method entry lookup error\n");
        exit(1);
    }
    if (JEMCC_OpenZipFileStream(NULL, zf, &entry) != NULL) {
        (void) fprintf(stderr, "Error: Successful open of odd method stream\n");
        exit(1);
    }
    checkException("ZipException", "Unsupported", "invalid compression method");
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Sizing overflow */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/overflow.zip", &zf, 
                          JNI_FALSE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected flood zip open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "fifty",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected flood entry lookup error\n");
        exit(1);
    }
    stream = JEMCC_OpenZipFileStream(NULL, zf, &entry);
    if (stream == NULL) {
        (void) fprintf(stderr, "Error: Unable to open flood stream\n");
        exit(1);
    }
    while (1) {
        rc = JEMCC_ZipFileStreamRead(NULL, streamBuffer, 1024, stream);
        if (rc == 0) {
            (void) fprintf(stderr, "Error: Completed read from flood stream\n");
            exit(1);
        }
        if (rc < 0) break;
    }
    checkException("ZipException", "Insufficient data", "data overflow");
    rc = JEMCC_ZipFileStreamRead(NULL, streamBuffer, 1024, stream);
    if (rc >= 0) {
        (void) fprintf(stderr, "Error: Resume on error from flood stream\n");
        exit(1);
    }
    checkException("ZipException", "Re-read on failed", "data overflow rpt");
    JEMCC_CloseZipFileStream(NULL, stream);
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /* Or maybe just a nice corrupted file or two */
    *exClassName = *exMsg = '\0';
    if (JEMCC_OpenZipFile(NULL, "zipdata/mangledfile.zip", &zf, 
                          JNI_TRUE, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected mangled data open failure\n");
        exit(1);
    }
    if (JEMCC_FindZipFileEntry(NULL, zf, "fifty",
                               &entry, JNI_FALSE) != JNI_OK) {
        (void) fprintf(stderr, "Error: Unexpected mangled data lookup error\n");
        exit(1);
    }
    stream = JEMCC_OpenZipFileStream(NULL, zf, &entry);
    if (stream == NULL) {
        (void) fprintf(stderr, "Error: Unable to open mangled stream\n");
        exit(1);
    }
    while (1) {
        rc = JEMCC_ZipFileStreamRead(NULL, streamBuffer, 1024, stream);
        if (rc == 0) {
            (void) fprintf(stderr, "Error: End read from mangled stream\n");
            exit(1);
        }
        if (rc < 0) break;
    }
    checkException("ZipException", "Invalid data", "mangled file");
    rc = JEMCC_ZipFileStreamRead(NULL, streamBuffer, 1024, stream);
    if (rc >= 0) {
        (void) fprintf(stderr, "Error: Resume on error from mangled stream\n");
        exit(1);
    }
    checkException("ZipException", "Re-read on failed", "mangled file rpt");
    JEMCC_CloseZipFileStream(NULL, stream);
    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
    JEMCC_CloseZipFile(NULL, zf);

    /********** Other Failures ***********/

    /* Repeat the following failures with and without file prescan */
    for (mode = 0; mode < 2; mode++) {
        if (JEMCC_OpenZipFile(NULL, "zipdata/ok.zip", &zf, 
                              ((mode == 0) ? JNI_TRUE : JNI_FALSE), 
                              JNI_FALSE) != JNI_OK) {
            (void) fprintf(stderr, "Error opening zip file\n");
            exit(1);
        }
        if (zf->entryCount != 7) {
            (void) fprintf(stderr, "Incorrect entry count in ok.zip\n");
            exit(1);
        }

        if (JEMCC_FindZipFileEntry(NULL, zf, "no/data/found/here", 
                                   &entry, JNI_FALSE) != JNI_EINVAL) {
            (void) fprintf(stderr, "Incorrect return from invalid find\n");
            exit(1);
        }

        JEMCC_CloseZipFile(NULL, zf);
    }

    /* Perform forced internal failure scanning */
#ifdef ENABLE_ERRORSWEEP
    testFailureCurrentCount = 0;
    doValidReadScan(JNI_TRUE);
    doValidStreamScan(JNI_TRUE);
    failureTotal = testFailureCurrentCount;
    (void) fprintf(stderr, "Preparing to process %i forced failures\n",
                           failureTotal);
    for (testFailureCount = 1; 
             testFailureCount <= failureTotal;
                 testFailureCount++) {
        testFailureCurrentCount = 0;
        doValidReadScan(JNI_FALSE);
        doValidStreamScan(JNI_FALSE);
    }
#else
    failureTotal = 0;
    doValidReadScan(JNI_TRUE);
    doValidStreamScan(JNI_TRUE);
#endif

    (void) fprintf(stderr, "All tests passed successfully (%i forced)\n",
                           failureTotal);
    exit(0);
}

static char digits[] = { '0', '1', '2', '3', '4',
                         '5', '6', '7', '8', '9' };

static char *names[] = { "zero", "one", "five", "ten",
                         "twenty", "fifty", "hundred" };
static int sizes[] = { 0, 1, 5, 10, 20, 50, 100 };

/* Perform successful read operations for forced failure validation */
void doValidReadScan(jboolean fullsweep) {
    JEMCC_ZipFile *zf;
    JEMCC_ZipFileEntry entry;
    jbyte *data;
    int mode, idx, k;

    /* Repeat the following with and without prescan */
    for (mode = 0; mode < 2; mode++) {
        if (JEMCC_OpenZipFile(NULL, "zipdata/ok.zip", &zf, 
                              ((mode == 0) ? JNI_TRUE : JNI_FALSE), 
                              JNI_FALSE) != JNI_OK) {
            if (fullsweep == JNI_TRUE) {
                (void) fprintf(stderr, "Prescan: error opening zip file\n");
                exit(1);
            }
            return;
        }

        for (idx = 0; idx < 7; idx++) {
            if (JEMCC_FindZipFileEntry(NULL, zf, names[idx],
                                       &entry, JNI_FALSE) != JNI_OK) {
                if (fullsweep == JNI_TRUE) {
                    (void) fprintf(stderr, "Prescan: cannot find %s\n",
                                           names[idx]);
                    exit(1);
                }
                JEMCC_CloseZipFile(NULL, zf);
                return;
            }
            if (entry.uncompressedSize != sizes[idx]) {
                (void) fprintf(stderr, "Prescan: wrong entry size for %s\n",
                                       names[idx]);
                exit(1);
            }
            data = JEMCC_ReadZipFileEntry(NULL, zf, &entry);
            if (data == NULL) {
                if (fullsweep == JNI_TRUE) {
                    (void) fprintf(stderr, "Prescan: cannot read entry %s\n",
                                           names[idx]);
                    exit(1);
                }
                JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
                JEMCC_CloseZipFile(NULL, zf);
                return;
            }
            for (k = 0; k < sizes[idx] - 1; k++) {
                if (data[k] != digits[k % 10]) {
                    (void) fprintf(stderr, "Prescan: A invalid data for %s\n",
                                           names[idx]);
                    exit(1);
                }
            }
            if ((k != 0) && (data[k] != '\n')) {
                (void) fprintf(stderr, "Prescan: B invalid data for %s\n",
                                       names[idx]);
                exit(1);
            }
            JEMCC_Free(data);
            JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
        }

        JEMCC_CloseZipFile(NULL, zf);
    }
}

/* Perform successful stream operations for forced failure validation */
void doValidStreamScan(jboolean fullsweep) {
    JEMCC_ZipFile *zf;
    JEMCC_ZipFileEntry entry;
    JEMCC_ZipFileStream *streama, *streamb;
    jbyte *data, rdBuffer[1024];
    int cnt, mode, idx, k, offset, l;

    /* Repeat the following with and without prescan */
    for (mode = 0, cnt = 0; mode < 2; mode++) {
        for (idx = 0; idx < 7; idx++) {
            /* Note, multiple opens for dangling stream close tests */
#ifdef ENABLE_ERRORSWEEP
            disableSweep = JNI_TRUE;
#endif
            if (JEMCC_OpenZipFile(NULL, "zipdata/ok.zip", &zf, 
                                  ((mode == 0) ? JNI_TRUE : JNI_FALSE), 
                                  JNI_FALSE) != JNI_OK) {
                (void) fprintf(stderr, "Prescan: error opening zip file\n");
                exit(1);
            }

            if (JEMCC_FindZipFileEntry(NULL, zf, names[idx],
                                       &entry, JNI_FALSE) != JNI_OK) {
                (void) fprintf(stderr, "Prescan: cannot find %s\n",
                                       names[idx]);
                exit(1);
            }

            /* Conventional read for comparison */
            data = JEMCC_ReadZipFileEntry(NULL, zf, &entry);
            if (data == NULL) {
                (void) fprintf(stderr, "Prescan: cannot read comp entry %s\n",
                                       names[idx]);
                exit(1);
            }
#ifdef ENABLE_ERRORSWEEP
            disableSweep = JNI_FALSE;
#endif

            /* Two streams, reading big and small chunks of data */
            streama = JEMCC_OpenZipFileStream(NULL, zf, &entry);
            if (streama == NULL) {
                if (fullsweep == JNI_TRUE) {
                    (void) fprintf(stderr, "Prescan: streama open err on %s\n",
                                           names[idx]);
                    exit(1);
                }
                JEMCC_Free(data);
                JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
                JEMCC_CloseZipFile(NULL, zf);
                return;
            }
            streamb = JEMCC_OpenZipFileStream(NULL, zf, &entry);
            if (streamb == NULL) {
                if (fullsweep == JNI_TRUE) {
                    (void) fprintf(stderr, "Prescan: streamb open err on %s\n",
                                           names[idx]);
                    exit(1);
                }
                JEMCC_Free(data);
                JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
                JEMCC_CloseZipFileStream(NULL, streama);
                JEMCC_CloseZipFile(NULL, zf);
                return;
            }

            offset = 0;
            while ((l = JEMCC_ZipFileStreamRead(NULL, rdBuffer + offset,
                                                1024 - offset, streama)) > 0) {
                offset += l;
            }
            if (l < 0) {
                if (fullsweep == JNI_TRUE) {
                    (void) fprintf(stderr, "Prescan: streama read err on %s\n",
                                           names[idx]);
                    exit(1);
                }
            } else {
                if (offset != entry.uncompressedSize) {
                    (void) fprintf(stderr, "Prescan: bad streama len on %s\n",
                                           names[idx]);
                    exit(1);
                }
                for (k = 0; k < offset; k++) {
                    if (data[k] != rdBuffer[k]) {
                        (void) fprintf(stderr, "Prescan: invalid data for %s\n",
                                               names[idx]);
                        exit(1);
                    }
                }
                if (JEMCC_ZipFileStreamRead(NULL, rdBuffer, 
                                            1024, streama) != 0) {
                    if (fullsweep == JNI_TRUE) {
                        (void) fprintf(stderr, 
                                       "Prescan: streama end failure on %s\n",
                                       names[idx]);
                        exit(1);
                    }
                }
            }

            offset = 0;
            k = (int) (entry.uncompressedSize / 7);
            if (k < 7) k = 7;
            while ((l = JEMCC_ZipFileStreamRead(NULL, rdBuffer + offset, k,
                                                streamb)) > 0) {
                offset += l;
            }
            if (l < 0) {
                if (fullsweep == JNI_TRUE) {
                    (void) fprintf(stderr, "Prescan: streamb read err on %s\n",
                                           names[idx]);
                    exit(1);
                }
            } else {
                if (offset != entry.uncompressedSize) {
                    (void) fprintf(stderr, "Prescan: bad streamb len on %s\n",
                                           names[idx]);
                    exit(1);
                }
                for (k = 0; k < offset; k++) {
                    if (data[k] != rdBuffer[k]) {
                        (void) fprintf(stderr, "Prescan: invalid data for %s\n",
                                               names[idx]);
                        exit(1);
                    }
                }
                if (JEMCC_ZipFileStreamRead(NULL, rdBuffer, 
                                            1024, streamb) != 0) {
                    if (fullsweep == JNI_TRUE) {
                        (void) fprintf(stderr, 
                                       "Prescan: streamb end failure on %s\n",
                                       names[idx]);
                        exit(1);
                    }
                }
            }

            JEMCC_Free(data);
            /* Randomly vary the closing arrangements (test pending) */
            switch (cnt % 4) {
                case 0:
                    JEMCC_CloseZipFileStream(NULL, streama);
                    JEMCC_CloseZipFileStream(NULL, streamb);
                    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
                    JEMCC_CloseZipFile(NULL, zf);
                    break;
                case 1:
                    JEMCC_CloseZipFileStream(NULL, streamb);
                    JEMCC_CloseZipFileStream(NULL, streama);
                    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
                    JEMCC_CloseZipFile(NULL, zf);
                    break;
                case 2:
                    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
                    JEMCC_CloseZipFile(NULL, zf);
                    JEMCC_CloseZipFileStream(NULL, streama);
                    JEMCC_CloseZipFileStream(NULL, streamb);
                    break;
                case 3:
                    JEMCC_ReleaseZipFileEntry(NULL, zf, &entry);
                    JEMCC_CloseZipFileStream(NULL, streamb);
                    JEMCC_CloseZipFile(NULL, zf);
                    JEMCC_CloseZipFileStream(NULL, streama);
                    break;
            }
            cnt++;
        }
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

    if (idx == JEMCC_Class_IOException) {
        className = "java.io.IOException";
    } else if (idx == JEMCC_Class_OutOfMemoryError) {
        className = "java.lang.OutOfMemoryError";
    } else if (idx == JEMCC_Class_InternalError) {
        className = "java.lang.InternalError";
    } else {
        (void) fprintf(stderr, "Fatal error: unexpected exception index %i.\n",
                               idx);
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
    (void) fprintf(stderr, "Error[%s]: %s.\n", className, msg);

    (void) strcpy(exClassName, className);
    (void) strcpy(exMsg, msg);
}
