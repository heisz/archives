/**
 * Path parsing and content loading methods (split for testsuite linkage).
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
#include "jeminc.h"

/* Read the VM structure/method definitions */
#include "jem.h"

/**
 * Parse a path list string according to the Java CLASSPATH model.  Splits
 * entries based on a system dependent character (: or ;) and constructs a
 * path list of valid entries (either directories or zip files).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     list - the path entry set to be populated with the parse results
 *     path - the path list string to be parsed
 *     allowZip - if JNI_TRUE, path entries which are zip or jar files
 *                are included in the final list.  If JNI_FALSE, such
 *                files are skipped (list contains directories only).
 *
 * Returns:
 *     JNI_OK - path list has been successfully parsed
 *     JNI_ENOMEM - a memory error has occurred (an OutOfMemory exception
 *                  will have been thrown in the current environment)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEM_ParsePathList(JNIEnv *env, JEM_PathEntryList *list, 
                       const char *path, jboolean allowZip) {
    int i, rc, cnt;
    char *str, *ptr, *lastPtr;

    if ((path == NULL) || (strlen(path) == 0)) {
        list->entryCount = 0;
        list->entries = NULL;
    } else {
        /* Count the number of entries (don't worry about empty yet) */
        cnt = 1;
        for (i = strlen(path) - 1; i >= 0; i--) {
            if (path[i] == JEMCC_PathSeparator) cnt++;
        }

        /* Allocate the array */
        list->entries = (struct JEM_PathEntry *) JEMCC_Malloc(env, 
                                         cnt * sizeof(struct JEM_PathEntry));
        if (list->entries == NULL) return JNI_ENOMEM;
        list->entryCount = cnt;

        /* Split up the classpath entry data - first entry contains string */
        list->entries[0].path = (char *) JEMCC_StrDupFn(env, (void *) path);
        if (list->entries[0].path == NULL) {
            JEMCC_Free(list->entries);
            list->entries = NULL;
            list->entryCount = 0;
            return JNI_ENOMEM;
        }
        cnt = 0;
        lastPtr = ptr = list->entries[0].path;
        while (*ptr != '\0') {
            if (*ptr == JEMCC_PathSeparator) {
                /* Remove trailing file separators */
                str = ptr - 1;
                while ((str >= lastPtr) && (*str == JEMCC_FileSeparator)) {
                    *(str--) = '\0';
                }

                /* Trim and store the entry */
                *ptr = '\0';
                list->entries[cnt++].path = lastPtr;
                lastPtr = ptr + 1;
            }
            ptr++;
        }
        list->entries[cnt++].path = lastPtr;

        /* Scan the entry set, determining data type */
        for (i = 0; i < list->entryCount; i++) {
            ptr = list->entries[i].path;
            str = ptr + strlen(ptr) - 4;
            if (strlen(ptr) == 0) {
                list->entries[i].type = JEM_PATH_INV;
            } else if (JEMCC_FileIsDirectory(ptr) > 0) {
                list->entries[i].type = JEM_PATH_DIR;
            } else if ((str > ptr) && (allowZip != JNI_FALSE) &&
                       ((strcmp(str, ".jar") == 0) ||
                                       (strcmp(str, ".zip") == 0))) {
                rc = JEMCC_OpenZipFile(env, ptr, &(list->entries[i].zipFile),
                                       JNI_TRUE, JNI_TRUE);
                if (rc == JNI_ENOMEM) {
                    /* Clean up any already open zip files */
                    JEM_DestroyPathList(env, list);
                    list->entries = NULL;
                    list->entryCount = 0;
                    return JNI_ENOMEM;
                }
                if (rc != JNI_OK) {
                    list->entries[i].type = JEM_PATH_INV;
                } else {
                    list->entries[i].type = JEM_PATH_JARZIP;
                }
            } else {
                list->entries[i].type = JEM_PATH_INV;
            }
        }
    }

    return JNI_OK;
}

/**
 * Destroy and clean up the path list allocated by the ParsePathList method
 * above.
 * 
 * Parameters:
 *     env - the VM environment which is currently in context
 *     list - the parsed path list instance to be destroyed
 */
void JEM_DestroyPathList(JNIEnv *env, JEM_PathEntryList *list) {
    int i;

    if (list->entries != NULL) {
        /* Need to flush all open zipfile instances */
        for (i = 0; i < list->entryCount; i++) {
            if (list->entries[i].type == JEM_PATH_JARZIP) {
                JEMCC_CloseZipFile(env, list->entries[i].zipFile);
            }
        }

        /* Recall that the first entry contains all strings sequentially */
        JEMCC_Free(list->entries[0].path);
        JEMCC_Free(list->entries);
    }
}

/**
 * Read the contents of a file located in the specified path list.
 * Scans each path location in order until it finds the requested file,
 * whereupon it reads it (expanding if necessary from a Zip file).  Note
 * that only the first instance of the file is tried - if an error occurs
 * during reading of the file, the search does not resume.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     pathList - the path entry set on which the file is located
 *     fileName - the name of the file to be read
 *     rawFileData - an allocated buffer containing the contents of the
 *                   file (the caller must free the buffer)
 *     rawFileSize - the size of the file (and hence the buffer contents)
 *
 * Returns:
 *     JNI_OK - the file was found and the read completed successfully
 *     JNI_EINVAL - the specified file was not found on the path
 *     JNI_ERR - the file was found and a read error occurred or a memory
 *               failure occurred while *reading* the file (an exception    
 *               will be thrown in the current environment)
 *     JNI_ENOMEM - a memory failure occurred while *searching* for the
 *                  file (an exception will be thrown in the current 
 *                  environment)
 *
 * Exceptions:
 *     ZipException - a Zipfile specific error occurred (invalid signature,
 *                    record information, etc.)
 *     IOException - a general file access exception occurred
 *     InternalError - an unexpected zlib library error occurred
 *     OutOfMemoryError - a memory allocation failed (search or read)
 */
jint JEM_ReadPathFileContents(JNIEnv *env, JEM_PathEntryList *pathList,
                              const char *fileName, jbyte **rawFileData,
                              jsize *rawFileSize) {
    JEMCC_ZipFileEntry zfentry;
    int i, rc, offset;
    char *targName;
    jsize len;
    FILE *fp;

    /* Search the path list for the file, depending on entry type */
    for (i = 0; i < pathList->entryCount; i++) {
        switch (pathList->entries[i].type) {
            case JEM_PATH_DIR:
                /* Determine the proper filename and verify its existence */
                if (JEMCC_EnvStrBufferInit(env, 100) == NULL) return JNI_ENOMEM;
                targName = JEMCC_EnvStrBufferAppendSet(env,
                                              pathList->entries[i].path,
                                              JEMCC_FileSeparatorStr, fileName,
                                              (char *) NULL);
                if (targName == NULL) return JNI_ENOMEM;
                if ((len = JEMCC_GetFileSize(targName)) <= 0) break;

                /* Read the contents of the file into memory */
                *rawFileData = (jbyte *) JEMCC_Malloc(env, len + 4);
                if (*rawFileData == NULL) return JNI_ENOMEM;
                fp = fopen(targName, "rb");
                if (ERROR_SWEEP(ES_DATA, fp == NULL)) {
                    JEMCC_Free(*rawFileData);
                    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException,
                                               NULL, "Path file open failed");
                    return JNI_ERR;
                }
                offset = 0;
                while ((rc = fread(*rawFileData + offset, 1,
                                   len - offset + 1, fp)) != 0) offset += rc;
                if (ERROR_SWEEP(ES_DATA, (feof(fp) == 0) || (offset != len))) {
                    /* Read error or file size has changed */
                    (void) fclose(fp);
                    JEMCC_Free(*rawFileData);
                    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException,
                                               NULL, "Path file read failed");
                    return JNI_ERR;
                }
                (void) fclose(fp);
                *rawFileSize = len;
                return JNI_OK;
            case JEM_PATH_JARZIP:
                rc = JEMCC_FindZipFileEntry(env, pathList->entries[i].zipFile,
                                            fileName, &zfentry, JNI_TRUE);
                if (rc == JNI_ENOMEM) return rc;
                if (ERROR_SWEEP(ES_DATA, rc == JNI_ERR)) {
                    /* Zip file is corrupt, stop using it */
                    pathList->entries[i].type = JEM_PATH_INV;
                    /* But make sure to clean up the references */
                    JEMCC_CloseZipFile(env, pathList->entries[i].zipFile);
                    break;
                }
                if (rc == JNI_OK) {
                    len = zfentry.uncompressedSize;
                    *rawFileData = JEMCC_ReadZipFileEntry(env,
                                                   pathList->entries[i].zipFile,
                                                   &zfentry);
                    JEMCC_ReleaseZipFileEntry(env, pathList->entries[i].zipFile,
                                              &zfentry);
                    if (rawFileData == NULL) return JNI_ERR;
                    *rawFileSize = len;
                    return JNI_OK;
                }
                break;
            default:
                /* Do nothing here, just ignore the entry */
                break;
        }
    }

    return JNI_EINVAL;
}

/**
 * Load a dynamic library through a specified source path (similar to
 * LD_LIBRARY_PATH).  Searches each directory entry in the path for the
 * indicated filename and attempts to load/link the library if found.
 * Note that only the first instance of the library is tried - if an error
 * occurs during linking, the search does not resume.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the dynamic loader instance through which the library is
 *              loaded.  If NULL, the main virtual machine loader is used.
 *     pathList - the path entry set on which the library is located.  If NULL,
 *            the virtual machine libPath is used.
 *     libFileName - the name of the library to load/link.  This must be
 *                   the system qualified library name, as returned from
 *                   the MapLibraryName method.  It can also be an absolute
 *                   filename, in which case the path entries are ignored.
 *     attachment - an opaque data object to associate with the dynamically
 *                  loaded library (for classloader cross-reference)
 *     libHandle - a reference pointer into which the linked library instance
 *                 will be returned, if the search/link action was successful
 *
 * Returns:
 *     JNI_OK - the library was found and linked successfully
 *     JNI_EINVAL - the specified library was not found on the vm path
 *     JNI_ERR - the library was found and a memory or data error occurred
 *               while reading/linking the library (an exception will be thrown
 *               in the current environment)
 *     JNI_ENOMEM - a memory failure occurred while searching for the
 *                  library (an exception will be thrown in the current
 *                  environment)
 *
 * Exceptions:
 *     IOException - a general file access exception occurred
 *     OutOfMemoryError - a memory allocation failed (search or read)
 *     UnsatisfiedLinkError - the library was found but the load/link failed
 */
jint JEM_LoadPathLibrary(JNIEnv *env, JEM_DynaLibLoader loader,
                         JEM_PathEntryList *pathList, const char *libFileName,
                         void *attachment, JEM_DynaLib *libHandle) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    char *targName;
    int i;

    /* Handle the null cases */
    if (loader == NULL) loader = jvm->libLoader;
    if (pathList == NULL) pathList = &(jvm->libPath);

    /* If filename is absolute, just load */
    if (JEMCC_FileIsAbsolute(libFileName)) {
        if (JEMCC_GetFileSize(libFileName) < 0) return JNI_EINVAL;
        *libHandle = JEM_DynaLibLoad(loader, libFileName, attachment);
        if (*libHandle == NULL) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_UnsatisfiedLinkError,
                                       NULL, JEM_DynaLibErrorMsg());
            return JNI_ERR;
        }
        return JNI_OK;
    }

    /* Otherwise, scan library path list for library instance */
    for (i = 0; i < pathList->entryCount; i++) {
        if (pathList->entries[i].type != JEM_PATH_DIR) continue;

        if (JEMCC_EnvStrBufferInit(env, 100) == NULL) return JNI_ENOMEM;
        targName = JEMCC_EnvStrBufferAppendSet(env, pathList->entries[i].path,
                                               JEMCC_FileSeparatorStr, 
                                               libFileName, (char *) NULL);
        if (targName == NULL) return JNI_ENOMEM;
        if (JEMCC_GetFileSize(targName) < 0) continue;
        *libHandle = JEM_DynaLibLoad(loader, targName, attachment);
        if (*libHandle == NULL) {
            /* TODO - is NULL error a memory failure? */
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_UnsatisfiedLinkError,
                                       NULL, JEM_DynaLibErrorMsg());
            return JNI_ERR;
        }
        return JNI_OK;
    }
    return JNI_EINVAL;
}
