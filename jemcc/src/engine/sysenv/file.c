/**
 * JEMCC system/environment functions to support file operations.
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
#include "errno.h"
#include <sys/ioctl.h> /* TODO - wrap appropriately */

/* Read the structure/method details */
#include "jem.h"

/* Character/string constants defined in sysenv.h */
char JEMCC_FileSeparator = '/';
char JEMCC_PathSeparator = ':';
char JEMCC_LineSeparator = '\n';
char *JEMCC_FileSeparatorStr = "/";
char *JEMCC_PathSeparatorStr = ":";
char *JEMCC_LineSeparatorStr = "\n";

/**
 * Determine if the given filename is a directory.
 *
 * Parameters:
 *     name - the name of the filesystem entity to test 
 *
 * Returns:
 *     JNI_TRUE if the filesystem entity associated with the name
 *     is a directory, JNI_FALSE if it is something else and JNI_ERR
 *     if the stat failed (entity does not exist).
 */
jint JEMCC_FileIsDirectory(const char *name) {
    struct stat stBuff;

    if (stat(name, &stBuff) < 0) return JNI_ERR;
#ifdef WIN32
    if ((stBuff.st_mode & _S_IFDIR) == _S_IFDIR) return JNI_TRUE;
#else
    if (S_ISDIR(stBuff.st_mode)) return JNI_TRUE;
#endif
    return JNI_FALSE;
}

/**
 * Determine if the given filename is absolute (bound to the root of
 * the filesystem) rather than relative (bound to the current working
 * directory).
 *
 * Parameters:
 *     name - the name of the filesystem entity to test
 *
 * Returns:
 *     JNI_TRUE if the filesystem name is an absolute path or JNI_FALSE
 *     if it is relative.
 */
jint JEMCC_FileIsAbsolute(const char *name) {
    if (*name == '/') return JNI_TRUE;

    return JNI_FALSE;
}

/**
 * Determine the size of the given filename.
 * 
 * Parameters:
 *     name - the name of the file to determine the size of
 *
 * Returns:
 *     The size of the specified file (in bytes, greater than or equal to 
 *     zero) or -1 if the stat of the file failed (does not exist or is
 *     not a file).
 */
jint JEMCC_GetFileSize(const char *name) {
    struct stat stBuff;

    if (stat(name, &stBuff) < 0) return -1;
    return (jsize) stBuff.st_size;
}

/**
 * Open a file by name, according to the provided flagsets.  Handles exception
 * cases appropriately and multi-threaded situations as best as possible.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     name - the name of the file to open
 *     flags - system flags (TBD)
 *     mode - permission modes (TBD)
 *     fd - pointer through which the file descriptor is returned
 *
 * Returns:
 *     JNI_OK if the file open was successful (file descriptor has been
 *     returned through the fd pointer), JNI_ERR otherwise (an exception
 *     will have been thrown in the current environment).
 *
 * Exceptions:
 *     FileNotFoundException - the indicated filename does not exist
 *     IOException - an error occurred while opening the file
 */
jint JEMCC_OpenFile(JNIEnv *env, const char *name, jint flags, jint mode, 
                    jint *fd) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable, NULL,
                               "TODO - not implemented");
    return JNI_ERR;
}

/**
 * Read a set of bytes from the indicated file descriptor.  Handles exception 
 * cases appropriately and multi-threaded situations as best as possible.
 * Will block until some bytes are read or an error occurs.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fd - the file descriptor to read from
 *     buff - the byte buffer to read into
 *     len - the number of bytes available in the buffer to read
 *     readLen - pointer through which the number of bytes read is returned.
 *               Zero bytes returned indicates EOF.
 *
 * Returns:
 *     JNI_OK if the read was successful (number of bytes read returned
 *     through readLen pointer), JNI_ERR if an error occurred (an exception
 *     will have been thrown in the current environment).
 *
 * Exceptions:
 *     IOException - an error occurred during the read action
 */
jint JEMCC_ReadFromFileDesc(JNIEnv *env, jint fd, void *buff, jint len, 
                            jint *readLen) {
    jint l = 0, errnum;
    char *msg;

    /* Repeat until some read or error occurs */
    while (len > 0) {
        l = read(fd, buff, len);
        if (l < 0) {
            /* Grab quick before modified (sometimes thread-local) */
            errnum = errno;
            if (errnum == EINTR) {
                /* This is OK, try the read again */
                continue;
            } else if ((errnum == EBADF) || (errnum == EINVAL) || 
                       (errnum == EISDIR)) {
                msg = "invalid file descriptor";
            } else {
                msg = NULL;
            }

            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, 
                                       NULL, msg);
            return JNI_ERR;
        } else {
            break;
        }
    }

    *readLen = l;
    return JNI_OK;
}

/**
 * Write a set of bytes to the indicated file descriptor.  Handles exception 
 * cases appropriately and multi-threaded situations as best as possible. This
 * method will block until all of the bytes are written or an error occurs.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fd - the file descriptor to write to
 *     buff - the buffer containing the bytes to be written
 *     len - the number of bytes to write from the buffer
 *
 * Returns:
 *     JNI_OK if the write was successfully completed, JNI_ERR if
 *     an error occurred (an exception will have been thrown in the
 *     current environment).
 *
 * Exceptions:
 *     IOException - an error occurred during the write action
 */
jint JEMCC_WriteToFileDesc(JNIEnv *env, jint fd, void *buff, jint len) {
    jint l, errnum;
    char *msg;

    /* Repeat until written or error occurs */
    while (len > 0) {
        l = write(fd, buff, len);
        if (l < 0) {
            /* Grab quick before modified (sometimes thread-local) */
            errnum = errno;
            if (errnum == EINTR) {
                /* This is OK, keep writing */
                continue;
            } else if ((errnum == EBADF) || (errnum == EINVAL)) {
                msg = "invalid file descriptor";
            } else {
                msg = NULL;
            }

            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException, 
                                       NULL, msg);
            return JNI_ERR;
        }

        buff += l;
        len -= l;
    }

    return JNI_OK;
}

/**
 * Obtain the number of bytes available to be read from the given file
 * descriptor.  Handles exception cases appropriately and multi-threaded 
 * situations as best as possible.  Note in some situations this method
 * may simply indicate "something available" (1).
 * 
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fd - the file descriptor to obtain the available details from
 *     avail - pointer through which the available number of bytes is returned
 *     
 * Returns:
 *     JNI_OK if the call was successful (number of bytes available returned
 *     through readLen pointer), JNI_ERR if an error occurred (an exception
 *     will have been thrown in the current environment).
 *
 * Exceptions:
 *     IOException - an error occurred during the query
 */
jint JEMCC_FileDescAvail(JNIEnv *env, jint fd, jint *avail) {
    jint rc, val, errnum;
    char *msg;

    /* TODO - wrap appropriately, IOCTL not always available (use select) */
    rc = ioctl(fd, FIONREAD, &val);
    if (rc < 0) {
        /* Grab quick before modified (sometimes thread-local) */
        errnum = errno;
        if ((errnum == EBADF) || (errnum == EINVAL)) {
            msg = "invalid file descriptor";
        } else {
            msg = NULL;
        }

        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException,
                                   NULL, msg);
        return JNI_ERR;
    }

    *avail = val;
    return JNI_OK;
}

/**
 * Reposition the offset for the indicate file descriptor instance.  Handles 
 * exception cases appropriately and multi-threaded situations as best as 
 * possible.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fd - the file descriptor to reposition
 *     offset - the amount to reposition the file offset by
 *     whence - the offset mode, based on the JEMCC_SEEK definitions below
 *     newPos - pointer through which the new file position is returned
 *
 * Returns:
 *     JNI_OK if the file was repositioned successfully (the new position is
 *     returned through the newPos pointer), JNI_ERR if an error occurred (an 
 *     exception will have been thrown in the current environment).
 *
 * Exceptions:
 *     IOException - an error occurred in the file reposition
 */
jint JEMCC_LSeekFileDesc(JNIEnv *env, jint fd, jlong offset, jint whence, 
                         jlong *newPos) {
    jint errnum;
    int nwhence;
    char *msg;

    /* Wherefore art thou, file position? */
    if (whence == JEMCC_SEEK_SET) nwhence = SEEK_SET;
    else if (whence == JEMCC_SEEK_CUR) nwhence = SEEK_CUR;
    else nwhence = SEEK_END;

    /* Just do it */
    if ((*newPos = lseek(fd, offset, nwhence)) < 0) {
        /* Grab quick before modified (sometimes thread-local) */
        errnum = errno;
        if ((errnum == EBADF) || (errnum == ESPIPE)) {
            msg = "invalid file descriptor";
        } else {
            msg = NULL;
        }

        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException,
                                   NULL, msg);
        return JNI_ERR;
    }

    return JNI_OK;
}

/**
 * Close the provided file descriptor instance.  Handles exception cases 
 * appropriately and multi-threaded situations as best as possible.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fd - the file descriptor to close
 *
 * Returns:
 *     JNI_OK if the file descriptor close was successful, JNI_ERR if
 *     an error occurred (an exception will have been thrown in the
 *     current environment).
 *
 * Exceptions:
 *     IOException - an error occurred in the file close
 */
jint JEMCC_CloseFileDesc(JNIEnv *env, jint fd) {
    jint errnum;
    char *msg;

    /* Just do it */
    if (close(fd) < 0) {
        /* Grab quick before modified (sometimes thread-local) */
        errnum = errno;
        if (errnum == EBADF) {
            msg = "invalid file descriptor";
        } else if (errnum == EINTR) {
            msg = "close interrupted";
        } else {
            msg = NULL;
        }

        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_IOException,
                                   NULL, msg);
        return JNI_ERR;
    }

    return JNI_OK;
}
