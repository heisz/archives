/**
 * JEMCC system/environment functions to support Zip file reading.
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

#ifdef TEST_NOMMAP
#undef HAVE_MMAP 
#endif

/* Standard and local includes */
#include "jeminc.h"
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#include <fcntl.h>
#include <errno.h>

/* Read the structure/method details */
#include "jem.h"

/* Of course, we require the zlib methods */
#include "zlib.h"

struct ZipFileStreamData;

/* Definition of the internal ZipFile management structure */
typedef struct ZipFileData {
    /* Match the external definition */
    JEMCC_ZipFile ezf;

    /* Pre-scan data */
    JEMCC_ZipFileEntry *entries;

    /* File descriptor/size and map access information */
    int fd;
    jint fileSize, dirStartOffset;
#ifdef HAVE_MMAP
    jbyte *mmapData;
#endif

    /* Link list entry point for streams open against this file */
    jboolean closePending;
    struct ZipFileStreamData *streams;
} ZipFileData;

/**
 * Forced read method, which ensures that exactly the requested
 * number of bytes/characters is read.  Returns JNI_OK if the
 * requested data was read, JNI_ERR if a read error occurred 
 * during input (an IOException will have been thrown in the
 * current environment).
 */
#ifndef HAVE_MMAP
static jint readFromFile(JNIEnv *env, int fd, jbyte *buffer, 
                         jint readLength, jboolean quietMode) {
    int n;

    while (readLength > 0) {
        n = read(fd, buffer, readLength);
        if (n <= 0) break;
        buffer += n;
        readLength -= n;
    }

    if (ERROR_SWEEP(ES_DATA, readLength > 0)) {
        if (quietMode == JNI_FALSE) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                       "Unexpected EOF in zip file");
        }
        return JNI_ERR;
    }

    return JNI_OK;
}
#endif

/**
 * Internal method to obtain a zip file entry.  Will initialize using as
 * little memory as possible.  Returns directory entry length, JNI_ERR if
 * directory entry read/processing failed (exception will be thrown if
 * not in quiet mode), or JNI_ENOMEM if a memory failure occurred.
 */
static jint readZipFileEntry(JNIEnv *env, ZipFileData *zipFile,
                             jint entryOffset, JEMCC_ZipFileEntry *entry,
                             jboolean quietMode) {
#ifdef HAVE_MMAP
    jubyte *scanPtr = zipFile->mmapData + entryOffset;
    juint dataOffset;
#else
    jubyte *scanPtr, readBuff[256];
    juint dataOffset;

    /* Move into position and read header */
    if (ERROR_SWEEP(ES_DATA, lseek(zipFile->fd, entryOffset, SEEK_SET) < 0)) {
        if (quietMode == JNI_FALSE) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                       "Zip file seek failed");
        }
        return JNI_ERR;
    }
    if (readFromFile(env, zipFile->fd, readBuff, 
                     46, quietMode) != JNI_OK) return JNI_ERR;
    scanPtr = readBuff;
#endif

    /* Check signature */
    if ((*scanPtr != 0x50) || (*(scanPtr + 1) != 0x4B) ||
        (*(scanPtr + 2) != 0x01) || (*(scanPtr + 3) != 0x02)) {
        if (quietMode == JNI_FALSE) {
            JEMCC_ThrowStdThrowableByName(env, NULL,
                                          "java.util.zip.ZipException", NULL,
                                          "Invalid Zip entry signature");
        }
        return JNI_ERR;
    }

    /* Read the core zipfile entry information */
    entry->compressionMethod = ((juint) *(scanPtr + 10)) |
                               ((juint) *(scanPtr + 11)) << 8;
    entry->fileModTime = 0;
    entry->crc32 = ((juint) *(scanPtr + 16)) |
                   ((juint) *(scanPtr + 17)) << 8 |
                   ((juint) *(scanPtr + 18)) << 16 |
                   ((juint) *(scanPtr + 19)) << 24;
    entry->compressedSize = ((juint) *(scanPtr + 20)) |
                            ((juint) *(scanPtr + 21)) << 8 |
                            ((juint) *(scanPtr + 22)) << 16 |
                            ((juint) *(scanPtr + 23)) << 24;
    entry->uncompressedSize = ((juint) *(scanPtr + 24)) |
                              ((juint) *(scanPtr + 25)) << 8 |
                              ((juint) *(scanPtr + 26)) << 16 |
                              ((juint) *(scanPtr + 27)) << 24;
    entry->fileNameLength = ((juint) *(scanPtr + 28)) |
                            ((juint) *(scanPtr + 29)) << 8;
    entry->extraFieldLength = ((juint) *(scanPtr + 30)) |
                              ((juint) *(scanPtr + 31)) << 8;
    entry->fileCommentLength = ((juint) *(scanPtr + 32)) |
                               ((juint) *(scanPtr + 33)) << 8;
    dataOffset = ((juint) *(scanPtr + 42)) |
                 ((juint) *(scanPtr + 43)) << 8 |
                 ((juint) *(scanPtr + 44)) << 16 |
                 ((juint) *(scanPtr + 45)) << 24;

#ifdef HAVE_MMAP
    /* Just initialize the memory pointers */
    entry->fileName = scanPtr + 46;
    entry->entryData = zipFile->mmapData + dataOffset;
#else
    /* Copy the filename, but keep the others as offsets */
    entry->fileName = (jbyte *) JEMCC_Malloc(env, entry->fileNameLength + 1);
    if (entry->fileName == NULL) return JNI_ENOMEM;
    /* NOTE: exact read of 46 has us in position for filename */
    if (readFromFile(env, zipFile->fd, entry->fileName, 
                     entry->fileNameLength, quietMode) != JNI_OK) {
        JEMCC_Free(entry->fileName);
        entry->fileName = NULL;
        return JNI_ERR;
    }
    entry->entryData = (jbyte *) dataOffset;
    entry->extraFieldData = (jbyte *) entryOffset + 46 + entry->fileNameLength;
#endif

    return entry->fileNameLength + entry->extraFieldLength +
           entry->fileCommentLength + 46;
}

/**
 * Open a Zip/Jar file instance for reading.  Can prescan the file instance
 * and determine the internal file entries, providing efficient lookups at the
 * cost of initial scanning time and memory overhead.  Returned structure
 * instance is allocated and owned by the caller, but must be released
 * using the CloseZipFile method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fileName - the name of the file to be opened for processing.  Depending
 *                on system support, may be memory mapped or opened as a
 *                conventional seek/read file descriptor.
 *     zipFile - the reference to return the open ZipFile structure through
 *     preScan - if JNI_TRUE, the Zip directory structure will be preloaded
 *               into memory (faster lookup)
 *     quietMode - if JNI_TRUE, no exceptions other than OutOfMemory will
 *                 be thrown (used for classloaders, etc.)
 *
 * Returns:
 *     JNI_OK - the Zip file was opened/parsed successfully
 *     JNI_ERR - an error occurred opening/reading/parsing the Zip file
 *               contents (an exception will have been thrown in the current
 *               environment if quietMode is false)
 *     JNI_ENOMEM - an memory allocation failed and an OutOfMemoryError has
 *                  been thrown in the current environment
 *
 * Exceptions
 *     ZipException - a Zipfile specific error occurred (invalid signature,
 *                    record information, etc.)
 *     IOException - a general file access exception occurred
 *     OutOfMemoryError - a memory allocation for the file structures failed
 */
jint JEMCC_OpenZipFile(JNIEnv *env, const char *fileName, 
                       JEMCC_ZipFile **zipFile, jboolean preScan, 
                       jboolean quietMode) {
    ZipFileData *retFile;
#ifdef HAVE_MMAP
    jubyte *scanPtr, *limPtr;
#else
    jint readStart, readSize;
    jubyte *scanPtr, readBuff[256];
#endif
    jint i, entryLen, offset;

    /* Create the data structure and open the file for reading/mapping */
    retFile = (ZipFileData *) JEMCC_Malloc(env, sizeof(ZipFileData));
    if (retFile == NULL) return JNI_ENOMEM;

    retFile->fd = open(fileName, O_RDONLY);
    if (retFile->fd < 0) {
        if (quietMode == JNI_FALSE) {
            JEMCC_ThrowStdThrowableByName(env, NULL,
                                          "java.util.zip.ZipException", NULL,
                                          "Requested file not found");
        }
        JEMCC_Free(retFile);
        return JNI_ERR;
    }

    /* Need the size of the file for mapping/bound checks */
    retFile->fileSize = JEMCC_GetFileSize(fileName);
    if (ERROR_SWEEP(ES_DATA, retFile->fileSize < 0)) {
        if (quietMode == JNI_FALSE) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                       "File size/stat failed");
        }
        close(retFile->fd);
        JEMCC_Free(retFile);
        return JNI_ERR;
    }

#ifdef HAVE_MMAP
    /* Map the file into our memory space */
    retFile->mmapData = (jbyte *) mmap(NULL, retFile->fileSize, PROT_READ,
                                       MAP_SHARED, retFile->fd, 0);
    if (ERROR_SWEEP(ES_DATA, retFile->mmapData == ((jbyte *) -1))) {
        if (quietMode == JNI_FALSE) {
            if (errno == ENOMEM) {
                /* purecov: begin inspected */
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_OutOfMemoryError,
                                           NULL, "Zip file map out of memory");
            } else {
                /* purecov: end */
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                           "Zip file memory map failed");
            }
        }
        close(retFile->fd);
        JEMCC_Free(retFile);
        return JNI_ERR;
    }
    close(retFile->fd);
    retFile->fd = -1;
#endif

    /* Validate the ZipFile structure and extract the entry count */
#ifdef HAVE_MMAP
    /* Locate the central directory end signature (skip global comment) */
    limPtr = retFile->mmapData;
    if (retFile->fileSize > 0xFFFF) limPtr += retFile->fileSize - 0xFFFF;
    scanPtr = retFile->mmapData + retFile->fileSize - 4;
    while (scanPtr >= limPtr) {
        if ((*scanPtr == 0x50) && (*(scanPtr + 1) == 0x4B) &&
            (*(scanPtr + 2) == 0x05) && (*(scanPtr + 3) == 0x06)) break;
        scanPtr--;
    }
    if (scanPtr < limPtr) {
#else
    /* Locate the central directory end signature (skip global comment) */
    readSize = 32;
    scanPtr = NULL;
    readStart = retFile->fileSize - readSize;
    while (readStart > (retFile->fileSize - 0xFFFF - 256)) {
        /* Move into location */
        if (readStart < 0) readStart = 0;
        if (ERROR_SWEEP(ES_DATA, lseek(retFile->fd, readStart, SEEK_SET) < 0)) {
            if (quietMode == JNI_FALSE) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                           "Zip file seek failed");
            }
            JEMCC_CloseZipFile(env, (JEMCC_ZipFile *) retFile);
            return JNI_ERR;
        }

        /* Fill the buffer */
        if (readFromFile(env, retFile->fd, readBuff,
                         readSize, quietMode) != JNI_OK) {
            JEMCC_CloseZipFile(env, (JEMCC_ZipFile *) retFile);
            return JNI_ERR;
        }

        /* Look for the signature */
        readSize -= 4;
        scanPtr = readBuff;
        while (readSize >= 0) {
            if ((*scanPtr == 0x50) && (*(scanPtr + 1) == 0x4B) &&
                (*(scanPtr + 2) == 0x05) && (*(scanPtr + 3) == 0x06)) break;
            scanPtr++;
            readSize--;
        }
        if (readSize >= 0) break;
        scanPtr = NULL;

        /* Jump back (by 256 - 21 byte overlap) */
        if (readStart == 0) break;
        readStart -= 235;
        readSize = 256;
    }
    if (scanPtr == NULL) {
#endif
        if (quietMode == JNI_FALSE) {
            JEMCC_ThrowStdThrowableByName(env, NULL,
                                          "java.util.zip.ZipException", NULL,
                                          "Zip file missing signature");
        }
        JEMCC_CloseZipFile(env, (JEMCC_ZipFile *) retFile);
        return JNI_ERR;
    }

    /* Only support one segment disk files (all indices are zero) */
    if ((*(scanPtr + 4) != 0x00) || (*(scanPtr + 5) != 0x00) ||
        (*(scanPtr + 6) != 0x00) || (*(scanPtr + 7) != 0x00)) {
        if (quietMode == JNI_FALSE) {
            JEMCC_ThrowStdThrowableByName(env, NULL,
                                          "java.util.zip.ZipException", NULL,
                                          "Multi-part Zip file found");
        }
        JEMCC_CloseZipFile(env, (JEMCC_ZipFile *) retFile);
        return JNI_ERR;
    }

    /* Grab the central directory entry count and start point */
    retFile->ezf.entryCount = ((juint) *(scanPtr + 10)) |
                              (((juint) *(scanPtr + 11)) << 8);

    retFile->dirStartOffset = ((juint) *(scanPtr + 16)) |
                              (((juint) *(scanPtr + 17)) << 8) |
                              (((juint) *(scanPtr + 18)) << 16) |
                              (((juint) *(scanPtr + 19)) << 24);

    if (preScan != JNI_FALSE) {
        retFile->entries = (JEMCC_ZipFileEntry *)
                  JEMCC_Malloc(env, (unsigned int) retFile->ezf.entryCount *
                                                   sizeof(JEMCC_ZipFileEntry));
        if (retFile->entries == NULL) {
            JEMCC_CloseZipFile(env, (JEMCC_ZipFile *) retFile);
            return JNI_ENOMEM;
        }

        offset = retFile->dirStartOffset;
        for (i = 0; i < retFile->ezf.entryCount; i++) {
            entryLen = readZipFileEntry(env, retFile, offset,
                                        &(retFile->entries[i]), quietMode);
            if (entryLen < 0) {
                JEMCC_CloseZipFile(env, (JEMCC_ZipFile *) retFile);
                return JNI_ERR;
            }
            offset += entryLen;
        }
    }

    *zipFile = (JEMCC_ZipFile *) retFile;
    return JNI_OK;
}

/**
 * Closes a previously opened Zip/Jar file.  May or may not close/deallocate
 * the actual file instance as entries may still be open, but the provided
 * ZipFile instance should not be referenced after this call.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipFile - the Zip/Jar file instance to be closed
 */
void JEMCC_CloseZipFile(JNIEnv *env, JEMCC_ZipFile *zipFile) {
    ZipFileData *zFile = (ZipFileData *) zipFile;
#ifndef HAVE_MMAP
    int i;
#endif

    /* Check for hanging zip stream entries */
    if (zFile == NULL) return;
    if ((zFile->streams != NULL) || (zFile->closePending != JNI_FALSE)) { 
        zFile->closePending = JNI_TRUE;
        return;
    }

    /* Clean up the prescanned entry information */
    if (zFile->entries != NULL) {
#ifndef HAVE_MMAP
        for (i = 0; i < zFile->ezf.entryCount; i++) {
            JEMCC_Free(zFile->entries[i].fileName);
        }
#endif
        JEMCC_Free(zFile->entries);
    }

    /* Shutdown the map/file handle */
#ifdef HAVE_MMAP
     (void) munmap(zFile->mmapData, zFile->fileSize);
#else
     (void) close(zFile->fd);
#endif
     JEMCC_Free(zFile);
}

/**
 * Locates a specific entry within an open Zip/Jar file.  Much faster in the
 * prescanned directory case (note that an external user should not directly
 * access the ZipFile entry name data).  The returned entry may or may not
 * be allocated - use the FreeEntry method to properly release the returned
 * entry.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipFile - the Zip/Jar file instance to be searched
 *     fileName - the name of the Zip entry file to be located
 *     zipEntry - the reference to return the located ZipEntry structure through
 *     quietMode - if JNI_TRUE, no exceptions other than OutOfMemory will
 *                 be thrown (used for classloaders, etc.)
 *
 * Returns:
 *     JNI_OK - the Zip file entry was found and parsed correctly
 *     JNI_ERR - an error occurred opening/reading/parsing the Zip file/entry
 *               contents (an exception will have been thrown in the current
 *               environment if quietMode is false)
 *     JNI_ENOMEM - an memory allocation failed and an OutOfMemoryError has
 *                  been thrown in the current environment
 *     JNI_EINVAL - no such entry was found in the Zip file directory
 *
 * Exceptions
 *     ZipException - a Zipfile specific error occurred (invalid signature,
 *                    record information, etc.)
 *     IOException - a general file access exception occurred
 *     OutOfMemoryError - a memory allocation for the file structures failed
 */
jint JEMCC_FindZipFileEntry(JNIEnv *env, JEMCC_ZipFile *zipFile,
                            const char *fileName, JEMCC_ZipFileEntry *zipEntry,
                            jboolean quietMode) {
    ZipFileData *zFile = (ZipFileData *) zipFile;
    jint i, entryLen, offset, fileNameLength = strlen(fileName);
    JEMCC_ZipFileEntry *ePtr;

    /* Scan the entries list if we have one */
    if (zFile->entries != NULL) {
        ePtr = zFile->entries;
        for (i = 0; i < zFile->ezf.entryCount; i++, ePtr++) {
            if ((ePtr->fileNameLength == fileNameLength) &&
                (memcmp(ePtr->fileName, fileName, fileNameLength) == 0)) {
                *zipEntry = *ePtr;
                return JNI_OK;
            }
        }
    } else {
        offset = zFile->dirStartOffset;
        for (i = 0; i < zFile->ezf.entryCount; i++) {
            entryLen = readZipFileEntry(env, zFile, offset,
                                        zipEntry, quietMode);
            if (entryLen < 0) return entryLen;
            if ((zipEntry->fileNameLength == fileNameLength) &&
                (memcmp(zipEntry->fileName, fileName, fileNameLength) == 0)) {
                return JNI_OK;
            }
#ifndef HAVE_MMAP
            JEMCC_Free(zipEntry->fileName);
#endif
            offset += entryLen;
        }
    }

    return JNI_EINVAL;
}

/**
 * Method used to "release" a located Zip file entry as returned from
 * the method above.  Does not deallocate the passed Zip file entry instance
 * (not allocated by the find method) but may deallocate internals where
 * appropriate.  In addition, this method does not close any input
 * streams which were opened against this entry (they will remain functional).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipFile - the Zip/Jar file instance from which the entry originated
 *     zipEntry - the entry instance to be released
 */
void JEMCC_ReleaseZipFileEntry(JNIEnv *env, JEMCC_ZipFile *zipFile,
                               JEMCC_ZipFileEntry *zipEntry) {
#ifndef HAVE_MMAP
    ZipFileData *zFile = (ZipFileData *) zipFile;

    /* Only free it where allocated during scanning search */
    if (zFile->entries == NULL) JEMCC_Free(zipEntry->fileName);
#endif
}

/**
 * Validate a zip file entry.  Cross checks the size, crc and other
 * information and updates the local extra field data information.
 * Returns a message if the entry is corrupted, otherwise NULL.
 */
static char *JEM_CheckZipFileEntry(JEMCC_ZipFileEntry *zipEntry,
                                   jubyte *checkBuffer) {
    if ((*checkBuffer != 0x50) || (*(checkBuffer + 1) != 0x4B) ||
               (*(checkBuffer + 2) != 0x03) || (*(checkBuffer + 3) != 0x04)) {
        return "Invalid Zip data signature";
    }

    zipEntry->extraFieldLength = ((juint) *(checkBuffer + 28)) |
                                 ((juint) *(checkBuffer + 29)) << 8;

    return NULL;
}

/**
 * Load the contents of a Zip file entry (fully inflated if required).
 * NOTE: this method does not allow for exception suppression - if a Zip
 * file is consistent enough to find the entry, then the user most likely
 * expects the file to read (avoids confusion).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipFile - the Zip/Jar file instance from which the entry originated
 *     zipEntry - the Zip/Jar file entry to read the contents of
 *
 * Returns:
 *     The data contents of the requested entry (conventionally allocated,
 *     must be freed by the caller) or NULL if a typing/inflation error
 *     occurred (an exception will be thrown in the current environment).
 *
 * Exceptions:
 *     ZipException - a Zipfile specific error occurred (invalid signature,
 *                    record information, etc.)
 *     IOException - a general file access exception occurred
 *     InternalError - an unexpected zlib library error occurred
 *     OutOfMemoryError - a memory allocation for the file structures failed
 */
jbyte *JEMCC_ReadZipFileEntry(JNIEnv *env, JEMCC_ZipFile *zipFile,
                              JEMCC_ZipFileEntry *zipEntry) {
    jbyte *inBuff, *outBuff;
    z_stream inflateStream;
    char *errMsg;
    juint checksum;
    jint rc;
#ifndef HAVE_MMAP
    ZipFileData *zFile = (ZipFileData *) zipFile;
    jint offset = (jint) zipEntry->entryData;
    jbyte checkBuff[30];
#endif

    /* Watch for zero length entries */
    if (zipEntry->compressedSize == 0) {
        if (zipEntry->uncompressedSize == 0) {
            return (jbyte *) JEMCC_Malloc(env, 1);
        }
        JEMCC_ThrowStdThrowableByName(env, NULL,
                                      "java.util.zip.ZipException", NULL,
                                      "Invalid Zip entry length");
        return NULL;
    }

    /* Move to or read the entry contents */
#ifdef HAVE_MMAP
    inBuff = zipEntry->entryData;
    errMsg = JEM_CheckZipFileEntry(zipEntry, inBuff);
    if (ERROR_SWEEP(ES_DATA, errMsg != NULL)) {
        JEMCC_ThrowStdThrowableByName(env, NULL, "java.util.zip.ZipException",
                                      NULL, errMsg);
        return NULL;
    }
    inBuff += 30 + zipEntry->fileNameLength + zipEntry->extraFieldLength;
#else
    if (ERROR_SWEEP(ES_DATA, lseek(zFile->fd, offset, SEEK_SET) < 0)) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                   "Zip file seek failed");
        return NULL; 
    }
    if (readFromFile(env, zFile->fd, checkBuff, 30, JNI_FALSE) != JNI_OK) {
        return NULL;
    }
    errMsg = JEM_CheckZipFileEntry(zipEntry, checkBuff);
    if (ERROR_SWEEP(ES_DATA, errMsg != NULL)) {
        JEMCC_ThrowStdThrowableByName(env, NULL, "java.util.zip.ZipException",
                                      NULL, errMsg);
        return NULL;
    }

    offset += 30 + zipEntry->fileNameLength + zipEntry->extraFieldLength;
    if (ERROR_SWEEP(ES_DATA, lseek(zFile->fd, offset, SEEK_SET) < 0)) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                   "Zip file seek failed");
        return NULL; 
    }
    inBuff = (jbyte *) JEMCC_Malloc(env, zipEntry->compressedSize + 1);
    if (inBuff == NULL) return NULL;
    if (readFromFile(env, zFile->fd, inBuff, zipEntry->compressedSize + 1, 
                     JNI_FALSE) != JNI_OK) {
        JEMCC_Free(inBuff);
        return NULL;
    }
#endif

    /* Handle the entry compression modes */
    switch (zipEntry->compressionMethod) {
        case 0:
            /* No compression in place, just straight copy */
#ifdef HAVE_MMAP
            outBuff = (jbyte *) JEMCC_Malloc(env, zipEntry->compressedSize);
            if (outBuff == NULL) return NULL;
            (void) memcpy(outBuff, inBuff, zipEntry->compressedSize);
            return outBuff;
#else
            return inBuff;
#endif
        case Z_DEFLATED:
            inflateStream.zalloc = NULL;
            inflateStream.zfree = NULL;
            inflateStream.opaque = NULL;
            rc = inflateInit2(&inflateStream, -MAX_WBITS);
            if (ERROR_SWEEP(ES_MEM, rc == Z_MEM_ERROR)) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_OutOfMemoryError,
                                           NULL, NULL);
#ifndef HAVE_MMAP
                JEMCC_Free(inBuff);
#endif
                (void) inflateEnd(&inflateStream);
                return NULL;
            }
            if (ERROR_SWEEP(ES_DATA, rc != Z_OK)) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError, NULL,
                                           (inflateStream.msg != NULL) ?
                                               inflateStream.msg :
                                               "Zlib inflate init failed");
#ifndef HAVE_MMAP
                JEMCC_Free(inBuff);
#endif
                (void) inflateEnd(&inflateStream);
                return NULL;
            }
            outBuff = (jbyte *) JEMCC_Malloc(env, zipEntry->uncompressedSize);
            if (outBuff == NULL) {
#ifndef HAVE_MMAP
                JEMCC_Free(inBuff);
#endif
                (void) inflateEnd(&inflateStream);
                return NULL;
            }

            /* Strange glitch, zlib needs one extra input byte for Z_FINISH */
            inflateStream.next_in = inBuff;
            inflateStream.avail_in = zipEntry->compressedSize + 1;
            inflateStream.next_out = outBuff;
            inflateStream.avail_out = zipEntry->uncompressedSize;
            rc = inflate(&inflateStream, Z_FINISH);
#ifndef HAVE_MMAP
            JEMCC_Free(inBuff);
#endif
            if (ERROR_SWEEP(ES_MEM, rc == Z_MEM_ERROR)) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_OutOfMemoryError,
                                           NULL, NULL);
                (void) inflateEnd(&inflateStream);
                JEMCC_Free(outBuff);
                return NULL;
            }
            if (ERROR_SWEEP(ES_DATA, rc != Z_STREAM_END)) {
                JEMCC_ThrowStdThrowableByName(env, NULL,
                                              "java.util.zip.ZipException",
                                              NULL, "Invalid data to inflate");
                (void) inflateEnd(&inflateStream);
                JEMCC_Free(outBuff);
                return NULL;
            }
            checksum = (juint) crc32((uLong) 0, NULL, 0);
            checksum = (juint) crc32((uLong) checksum, outBuff, 
                                     zipEntry->uncompressedSize);
            if (ERROR_SWEEP(ES_DATA, checksum != zipEntry->crc32)) {
                JEMCC_ThrowStdThrowableByName(env, NULL,
                                              "java.util.zip.ZipException",
                                              NULL, "Corrupt Zip entry data");
                (void) inflateEnd(&inflateStream);
                JEMCC_Free(outBuff);
                return NULL;
            }

            rc = inflateEnd(&inflateStream);
            if (ERROR_SWEEP(ES_DATA, rc != Z_OK)) {
                JEMCC_ThrowStdThrowableByName(env, NULL,
                                              "java.util.zip.ZipException",
                                              NULL,
                                              (inflateStream.msg != NULL) ?
                                                 inflateStream.msg :
                                                 "Zlib close failed (corrupt)");
                JEMCC_Free(outBuff);
                return NULL;
            }

            return outBuff;
    }

    JEMCC_ThrowStdThrowableByName(env, NULL, "java.util.zip.ZipException",
                                  NULL, "Unsupported Zip entry format");
#ifndef HAVE_MMAP
    JEMCC_Free(inBuff);
#endif

    return NULL;
}

/* Definition of the internal stream information */
#define STRM_RD_SZ 8
typedef struct ZipFileStreamData {
    /* Match the external definition */
    JEMCC_ZipFileStream ezfs;

    /* Parent ZipFile instance (for data access and cleanup) */
    ZipFileData *parentZipFile;

    /* Required information from entry (concurrent release) */
    juint compressionMethod, compressedSize, uncompressedSize;
    juint dataOffset;

    /* Working buffer and inflater for compressed entries */
    z_stream inflateStream;

    /* Input/output buffers and cursors for reading */
    jint sourceCursor;
#ifndef HAVE_MMAP
    jbyte inputBuffer[STRM_RD_SZ];
#endif

    /* Linked list used to manage stream set */
    struct ZipFileStreamData *nextStream;
} ZipFileStreamData;

/**
 * Open an input stream for the given entry.  This allows the contents
 * of the entry to be read without necessarily loading the contents into
 * memory.  Note that this stream may remain open for reading even if the
 * parent ZipFile is closed or the given ZipEntry is released (the ZipFile
 * only "officially" closes when all of the child streams are closed as
 * well).  While the returned stream instance is owned by the caller,
 * it can only be freed using the CloseStream method below.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipFile - the Zip/Jar file instance from which the entry originated
 *     zipEntry - the Zip/Jar file entry to open an input stream against
 *
 * Returns:
 *     The input stream instance which is open for reading or NULL if
 *     a typing/inflation error occurred (an exception will be thrown in
 *     the current environment).
 *
 * Exceptions:
 *     ZipException - a Zipfile specific error occurred (invalid signature,
 *                    record information, etc.)
 *     IOException - a general file access exception occurred
 *     InternalError - an unexpected zlib library error occurred
 *     OutOfMemoryError - a memory allocation for the file structures failed
 */
JEMCC_ZipFileStream *JEMCC_OpenZipFileStream(JNIEnv *env,
                                             JEMCC_ZipFile *zipFile,
                                             JEMCC_ZipFileEntry *zipEntry) {
    ZipFileStreamData *retStrm, *scanStrm;
    ZipFileData *zFile = (ZipFileData *) zipFile;
    char *errMsg;
    jint rc;
#ifndef HAVE_MMAP
    jbyte checkBuff[30];
#endif

    /* Allocate the stream instance and initialize entry/zip information */
    retStrm = (ZipFileStreamData *) JEMCC_Malloc(env, 
                                                 sizeof(ZipFileStreamData));
    if (retStrm == NULL) return NULL;
    retStrm->parentZipFile = zFile;
    retStrm->sourceCursor = 0;

    /* Capture zero length entries */
    if (zipEntry->compressedSize == 0) {
        if (zipEntry->uncompressedSize != 0) {
            JEMCC_ThrowStdThrowableByName(env, NULL,
                                          "java.util.zip.ZipException", NULL,
                                          "Invalid Zip entry length");
            JEMCC_Free(retStrm);
            return NULL;
        }
        retStrm->dataOffset = retStrm->compressionMethod = 0;
        retStrm->compressedSize = retStrm->uncompressedSize = 0;
    } else {
        /* Validate entry and determine offset */
#ifdef HAVE_MMAP
        retStrm->dataOffset = (zipEntry->entryData - zFile->mmapData);
        errMsg = JEM_CheckZipFileEntry(zipEntry, zipEntry->entryData);
        if (ERROR_SWEEP(ES_DATA, errMsg != NULL)) {
            JEMCC_ThrowStdThrowableByName(env, NULL, 
                                          "java.util.zip.ZipException", NULL,
                                          errMsg);
            JEMCC_Free(retStrm);
            return NULL;
        }
        retStrm->dataOffset += 30 + zipEntry->fileNameLength + 
                                    zipEntry->extraFieldLength;
#else
        retStrm->dataOffset = (jint) zipEntry->entryData;
        if (ERROR_SWEEP(ES_DATA, lseek(zFile->fd, retStrm->dataOffset, 
                                                              SEEK_SET) < 0)) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                       "Zip file seek failed");
            JEMCC_Free(retStrm);
            return NULL; 
        }
        if (readFromFile(env, zFile->fd, checkBuff, 30, JNI_FALSE) != JNI_OK) {
            JEMCC_Free(retStrm);
            return NULL;
        }
        errMsg = JEM_CheckZipFileEntry(zipEntry, checkBuff);
        if (ERROR_SWEEP(ES_DATA, errMsg != NULL)) {
            JEMCC_ThrowStdThrowableByName(env, NULL, 
                                          "java.util.zip.ZipException", NULL,
                                          errMsg);
            JEMCC_Free(retStrm);
            return NULL;
        }
        retStrm->dataOffset += 30 + zipEntry->fileNameLength + 
                                    zipEntry->extraFieldLength;
#endif
        retStrm->compressedSize = zipEntry->compressedSize;
        retStrm->uncompressedSize = zipEntry->uncompressedSize;

        /* Initialize according to compression mode */
        switch (zipEntry->compressionMethod) {
            case 0:
                /* No compression in place, just straight copy */
                retStrm->compressionMethod = 0;
                break;
            case Z_DEFLATED:
                retStrm->compressionMethod = Z_DEFLATED;
                retStrm->inflateStream.zalloc = NULL;
                retStrm->inflateStream.zfree = NULL;
                retStrm->inflateStream.opaque = NULL;
                rc = inflateInit2(&(retStrm->inflateStream), -MAX_WBITS);
                if (ERROR_SWEEP(ES_MEM, rc == Z_MEM_ERROR)) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                               JEMCC_Class_OutOfMemoryError,
                                               NULL, NULL);
                    (void) inflateEnd(&(retStrm->inflateStream));
                    JEMCC_Free(retStrm);
                    return NULL;
                }
                if (ERROR_SWEEP(ES_DATA, rc != Z_OK)) {
                    errMsg = retStrm->inflateStream.msg;
                    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError,
                                               NULL,
                                               (errMsg != NULL) ? errMsg :
                                                   "Zlib inflate init failed");
                    (void) inflateEnd(&(retStrm->inflateStream));
                    JEMCC_Free(retStrm);
                    return NULL;
                }
                break;
            default:
                JEMCC_ThrowStdThrowableByName(env, NULL, 
                                              "java.util.zip.ZipException",
                                              NULL,
                                              "Unsupported Zip entry format");
                JEMCC_Free(retStrm);
                return NULL;
        }
    }

    /* Push the open stream onto the zipfile list */
    if (zFile->streams == NULL) {
        zFile->streams = retStrm;
    } else {
        scanStrm = (ZipFileStreamData *) zFile->streams;
        while (scanStrm->nextStream != NULL) scanStrm = scanStrm->nextStream;
        scanStrm->nextStream = retStrm;
    }

    return (JEMCC_ZipFileStream *) retStrm;
}

/**
 * Closes a previously opened Zip file input stream.  If this is the last
 * open stream and the Zip file has been closed, the ZipFile instance will
 * also be cleaned up/released.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipStream - the Zip file entry stream to be closed
 */
void JEMCC_CloseZipFileStream(JNIEnv *env, JEMCC_ZipFileStream *zipStream) {
    ZipFileStreamData *zStrm = (ZipFileStreamData *) zipStream;
    ZipFileStreamData *scanStrm = NULL;
    ZipFileData *zFile = zStrm->parentZipFile;

    /* Remove stream from parent zipfile tracking (possible close) */
    if (zStrm == zFile->streams) {
        zFile->streams = zStrm->nextStream;
    } else {
        scanStrm = zFile->streams;
        while (scanStrm->nextStream != zStrm) scanStrm = scanStrm->nextStream;
        scanStrm->nextStream = zStrm->nextStream;
    }

    /* Capture final stream close on closed file */
    if ((zFile->streams == NULL) && (zFile->closePending != JNI_FALSE)) {
        zFile->closePending = JNI_FALSE;
        JEMCC_CloseZipFile(env, (JEMCC_ZipFile *) zFile);
    }

    /* Final cleanup */
    if (zStrm->compressionMethod == Z_DEFLATED) {
        (void) inflateEnd(&(zStrm->inflateStream));
    }
    JEMCC_Free(zStrm);
}

/**
 * Perform a read operation from a Zip file input stream, similar
 * to an "fread" on a FILE instance.  Conceptually, this read will
 * block for input, but there is no non-blocking support as the ZipFile
 * is always a local file instance which should not block adversely.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     buffer - the buffer into which the stream contents are to be read
 *     bufferLength - the size of the provided read buffer (maximum number
 *                    of bytes to be read)
 *     zipStream - the Zip file entry stream to be read from
 *
 * Returns:
 *     The number of characters read from the stream (non-zero), zero if
 *     the end of the stream has been reached and -1 if an error has
 *     occurred (an exception will be thrown in the current environment).
 *
 * Exceptions:
 *     ZipException - a Zipfile specific error occurred (invalid signature,
 *                    record information, etc.)
 *     IOException - a general file access exception occurred
 *     OutOfMemoryError - a memory allocation for the file structures failed
 */
jint JEMCC_ZipFileStreamRead(JNIEnv *env, jbyte *buffer, int bufferLength,
                             JEMCC_ZipFileStream *zipStream) {
    ZipFileStreamData *zStrm = (ZipFileStreamData *) zipStream;
    ZipFileData *zFile = zStrm->parentZipFile;
    int rc, readLen, outputLen;

    /* Perform the simple copy/read case first */
    if (zStrm->compressionMethod == 0) {
        if (zStrm->sourceCursor >= zStrm->uncompressedSize) return 0;
        readLen = zStrm->uncompressedSize - zStrm->sourceCursor;
        if (bufferLength < readLen) readLen = bufferLength;

#ifdef HAVE_MMAP
        (void) memcpy(buffer, zFile->mmapData + zStrm->dataOffset + 
                              zStrm->sourceCursor, readLen);
#else
        if (ERROR_SWEEP(ES_DATA, lseek(zFile->fd, 
                                       zStrm->dataOffset + zStrm->sourceCursor, 
                                                               SEEK_SET) < 0)) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                       "Zip file seek failed");
            return -1; 
        }
        readLen = read(zFile->fd, buffer, readLen);
        if (ERROR_SWEEP(ES_DATA, readLen <= 0)) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                       "Unexpected EOF in zip file");
            return -1;
        }
#endif
        zStrm->sourceCursor += readLen;
        return readLen;
    }

    /* Inflate case is a little more complicated */

    /* Note: need to repeat until data result, error occurs or finished */
    outputLen = 0;
    zStrm->inflateStream.next_out = buffer;
    zStrm->inflateStream.avail_out = bufferLength;
    while (zStrm->sourceCursor >= 0)  {
        if (zStrm->inflateStream.avail_in > 0) {
            /* Input data is in position, process into output */
            rc = inflate(&(zStrm->inflateStream), Z_SYNC_FLUSH);
            if (ERROR_SWEEP(ES_MEM, rc == Z_MEM_ERROR)) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_OutOfMemoryError,
                                           NULL, NULL);
                zStrm->sourceCursor = -1;
                return -1;
            }
            if (ERROR_SWEEP(ES_DATA, ((rc != Z_STREAM_END) && (rc != Z_OK)))) {
                JEMCC_ThrowStdThrowableByName(env, NULL,
                                              "java.util.zip.ZipException", 
                                              NULL, "Invalid data to inflate");
                zStrm->sourceCursor = -1;
                return -1;
            }
            outputLen = bufferLength - zStrm->inflateStream.avail_out;
            if (outputLen != 0) return outputLen;
            if (rc == Z_STREAM_END) {
                /* TODO - do we need consistency checks here? */
                return 0;
            }
        }

#ifdef HAVE_MMAP
        if (zStrm->sourceCursor == 0) {
            zStrm->inflateStream.next_in = zFile->mmapData + zStrm->dataOffset;
            zStrm->inflateStream.avail_in = zStrm->compressedSize + 1;
            zStrm->sourceCursor = zStrm->compressedSize;
        } else {
            JEMCC_ThrowStdThrowableByName(env, NULL,
                                          "java.util.zip.ZipException", NULL,
                                          "Insufficient data to inflate");
            zStrm->sourceCursor = -1;
            return -1;
        }
#else
        if (zStrm->inflateStream.avail_in == 0) {
            readLen = zStrm->compressedSize + 1 - zStrm->sourceCursor;
            if (readLen <= 0) {
                JEMCC_ThrowStdThrowableByName(env, NULL,
                                              "java.util.zip.ZipException",
                                              NULL,
                                              "Insufficient data to inflate");
                zStrm->sourceCursor = -1;
                return -1;
            }
            if (readLen > STRM_RD_SZ) readLen = STRM_RD_SZ;
            if (ERROR_SWEEP(ES_DATA, lseek(zFile->fd, zStrm->dataOffset + 
                                                      zStrm->sourceCursor, 
                                                               SEEK_SET) < 0)) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                           "Zip file seek failed");
                /* Can recover from seek error */
                return -1; 
            }
            readLen = read(zFile->fd, zStrm->inputBuffer, readLen);
            if (ERROR_SWEEP(ES_DATA, readLen <= 0)) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, NULL,
                                           "Unexpected EOF in zip file");
                /* Can recover from read error */
                return -1;
            }
            zStrm->sourceCursor += readLen;
            zStrm->inflateStream.next_in = zStrm->inputBuffer;
            zStrm->inflateStream.avail_in = readLen;
        }
#endif
    }

    JEMCC_ThrowStdThrowableByName(env, NULL, "java.util.zip.ZipException", NULL,
                                  "Re-read on failed Zip input stream");
    return -1;
}

/******************* DATE *********************/

/*
    uLong uDate;
    uDate = (uLong)(ulDosDate>>16);
    ptm->tm_mday = (uInt)(uDate&0x1f) ;
    ptm->tm_mon =  (uInt)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (uInt)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (uInt) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (uInt) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (uInt) (2*(ulDosDate&0x1f)) ;
*/
