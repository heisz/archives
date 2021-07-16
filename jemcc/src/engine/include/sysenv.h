/**
 * Prototypes for the common JEMCC system/environment interfaces.
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

#ifndef JEM_SYSENV_H
#define JEM_SYSENV_H 1

/* NOTE: this is auto-included by jem.h, so it shouldn't be directly used */

/**
 * All of the interfaces defined below are, apart from the JEMCC naming and
 * data typing, completely independent of the actual JEM virtual machine.
 * They can (conceivably) be compiled, tested and packaged as a separate
 * component and should remain that way.  Of course, the only 'exception' is
 * the exception handling mechanism (pun intended), which is simulated in 
 * the test programs.
 */

/* <jemcc_start> */

/******************* Filesystem Management **********************/

/**
 * System dependent separator characters, both in character and char * form.
 * Note that separators can be multi-character, in which case the single
 * character form is the first character in the sequence.
 */
JNIEXTERN char JEMCC_FileSeparator;
JNIEXTERN char JEMCC_PathSeparator;
JNIEXTERN char JEMCC_LineSeparator;
JNIEXTERN char *JEMCC_FileSeparatorStr;
JNIEXTERN char *JEMCC_PathSeparatorStr;
JNIEXTERN char *JEMCC_LineSeparatorStr;

/**
 * Determine if the given filename is a directory.
 *
 * Parameters:
 *     name - the name of the filesystem entity to test
 *
 * Returns:
 *     JNI_TRUE if the filesystem entity associated with the name
 *     is a directory, JNI_FALSE if it is something else and JNI_ERR
 *     if the stat() call failed (entity does not exist).
 */
JNIEXPORT jint JNICALL JEMCC_FileIsDirectory(const char *name);

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
JNIEXPORT jint JNICALL JEMCC_FileIsAbsolute(const char *name);

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
JNIEXPORT jint JNICALL JEMCC_GetFileSize(const char *name);

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
JNIEXPORT jint JNICALL JEMCC_OpenFile(JNIEnv *env, const char *name,
                                      jint flags, jint mode, jint *fd);

/* 
 * NOTE: the following methods will apply equally well to other file
 *       descriptor instances (from networking, for example).
 */

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
JNIEXPORT jint JNICALL JEMCC_ReadFromFileDesc(JNIEnv *env, jint fd, void *buff,
                                              jint len, jint *readLen);

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
JNIEXPORT jint JNICALL JEMCC_WriteToFileDesc(JNIEnv *env, jint fd, void *buff,
                                             jint len);

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
JNIEXPORT jint JNICALL JEMCC_FileDescAvail(JNIEnv *env, jint fd, jint *avail);

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
#define JEMCC_SEEK_SET 1
#define JEMCC_SEEK_CUR 2
#define JEMCC_SEEK_END 3
JNIEXPORT jint JNICALL JEMCC_LSeekFileDesc(JNIEnv *env, jint fd, jlong offset,
                                           jint whence, jlong *newPos);

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
JNIEXPORT jint JNICALL JEMCC_CloseFileDesc(JNIEnv *env, jint fd);

/******************* Zip/Jar File Management **********************/

/**
 * Structure which contains the access information for a file entry within
 * a Zip file.
 */
typedef struct JEMCC_ZipFileEntry {
    /* Zip entry constants, available to external applications */
    juint fileNameLength, extraFieldLength, fileCommentLength, crc32;
    juint compressionMethod, compressedSize, uncompressedSize;
    jlong fileModTime;

    /* Access values (for internal use only) */
    jbyte *fileName, *entryData, *extraFieldData;
} JEMCC_ZipFileEntry;

/**
 * Exposed structure definition for an opened Zip file instance (system
 * specific details are opaque).
 */
typedef struct JEMCC_ZipFile {
    jint entryCount;
} JEMCC_ZipFile;

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
JNIEXPORT jint JNICALL JEMCC_OpenZipFile(JNIEnv *env, const char *fileName,
                                         JEMCC_ZipFile **zipFile,
                                         jboolean preScan, jboolean quietMode);

/**
 * Closes a previously opened Zip/Jar file.  May or may not close/deallocate
 * the actual file instance as entries may still be open, but the provided
 * ZipFile instance should not be referenced after this call.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipFile - the Zip/Jar file instance to be closed
 */
JNIEXPORT void JNICALL JEMCC_CloseZipFile(JNIEnv *env, JEMCC_ZipFile *zipFile);

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
JNIEXPORT jint JNICALL JEMCC_FindZipFileEntry(JNIEnv *env, 
                                              JEMCC_ZipFile *zipFile,
                                              const char *fileName,
                                              JEMCC_ZipFileEntry *zipEntry,
                                              jboolean quietMode);

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
JNIEXPORT void JNICALL JEMCC_ReleaseZipFileEntry(JNIEnv *env,
                                                 JEMCC_ZipFile *zipFile,
                                                 JEMCC_ZipFileEntry *zipEntry);

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
JNIEXPORT jbyte *JNICALL JEMCC_ReadZipFileEntry(JNIEnv *env,
                                                JEMCC_ZipFile *zipFile,
                                                JEMCC_ZipFileEntry *zipEntry);

/**
 * Structure placeholder for an input stream driven from a Zip file entry.
 */
typedef struct JEMCC_ZipFileStream {
    /* This structure is completely opaque */
    int dummy;
} JEMCC_ZipFileStream;

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
JNIEXPORT JEMCC_ZipFileStream *JNICALL JEMCC_OpenZipFileStream(JNIEnv *env,
                                                JEMCC_ZipFile *zipFile,
                                                JEMCC_ZipFileEntry *zipEntry);

/**
 * Closes a previously opened Zip file input stream.  If this is the last
 * open stream and the Zip file has been closed, the ZipFile instance will
 * also be cleaned up/released.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     zipStream - the Zip file entry stream to be closed
 */
JNIEXPORT void JNICALL JEMCC_CloseZipFileStream(JNIEnv *env, 
                                                JEMCC_ZipFileStream *zipStream);

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
JNIEXPORT jint JNICALL JEMCC_ZipFileStreamRead(JNIEnv *env, jbyte *buffer,
                                               int bufferLength,
                                               JEMCC_ZipFileStream *zipStream);

/******************* Thread Management **********************/

/**
 * For complete "opaqueness", threads are referenced within the JEMCC
 * framework by a 32-bit integer identifier.  Note that this identifier
 * is not necessarily a start-from-zero counter, but an arbitrary bitfield
 * optimized for thread/monitor usage in a specific environment.
 */
typedef jint JEMCC_ThreadId;

/**
 * Method prototype for thread start/initialization methods.  Passed to the
 * following CreateThread method as the initial function called a newly
 * created thread instance.
 *
 * Parameters:
 *     env - a new VM environment which is associated with the newly
 *           created thread instance (in context for the new thread)
 *     userArg - the user data provided to the CreateThread method
 */
typedef void *JEMCC_ThreadStartFunction(JNIEnv *env, void *userArg);

/**
 * Create a new native/user thread instance which will execute the specified
 * start function with the given priority.  A new JNIEnv instance will be
 * created for and attached to the new thread and will be provided to the
 * specified start function.
 *
 *
 * Note: in general, the priority is often ignored in native thread
 *       implementations.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     startFn - the initial function which is to be executed at the start of
 *               the thread
 *     userArg - user data to be passed to the thread start function
 *     priority - the priority of the thread to be created (from 1 to 10)
 *
 * Returns:
 *     The thread reference handle or 0 if the thread creation failed (an
 *     exception will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation for the new thread failed
 *     InternalError - some other thread resouce failure occurred
 */
JNIEXPORT JEMCC_ThreadId JNICALL
                    JEMCC_CreateThread(JNIEnv *env,
                                       JEMCC_ThreadStartFunction startFn,
                                       void *userArg, jint priority);

/**
 * Determines if two thread identifiers are identical.  This takes into
 * account only the bitfields which identify the thread - other bits may
 * be different and such differences will not affect the outcome of this
 * method.
 *
 * Parameters:
 *     threada, threadb - the two thread identifiers to compare
 *
 * Returns:
 *     JNI_TRUE if the two threads are equal, JNI_FALSE otherwise.
 */
JNIEXPORT jboolean JNICALL JEMCC_IsSameThread(JEMCC_ThreadId threada,
                                              JEMCC_ThreadId threadb);

/**
 * Obtain the thread identifier for the current thread.
 *
 * Returns:
 *     The thread identifier for the currently executing thread.
 */
JNIEXPORT JEMCC_ThreadId JNICALL JEMCC_GetCurrentThread();

/**
 * Yield control of the current thread, allowing any threads waiting
 * for use of this processor to continue.  This method will return
 * when the thread scheduler re-activates this thread (may occur at
 * any time).
 */
JNIEXPORT void JNICALL JEMCC_YieldCurrentThread();

/**
 * Yield control of the current thread and sleep for a given period,
 * allowing other threads waiting on the processor to continue. This
 * method will return when the sleep period expires and the thread
 * scheduler re-activates this thread.
 *
 * Note: this method will round down to the finest granularity of the
 *       sleep periods provided by the system.
 *
 * Parameters:
 *     nano - the number of nanoseconds to wait for
 */
JNIEXPORT void JNICALL JEMCC_YieldCurrentThreadAndSleep(jlong nano);

/**
 * Yield control of the current thread based on activity on the given
 * file descriptor index, allowing other threads waiting on the processor
 * to continue without potentially blocking other user level threads
 * in the kernel.  This method will return when the thread scheduler
 * re-activates this thread and there is activity on the given file
 * descriptor.
 *
 * Parameters:
 *     fd - the file descriptor number to yield the thread against
 */
JNIEXPORT void JNICALL JEMCC_YieldCurrentThreadAgainstFd(int fd);

/* <jemcc_end> */

/**
 * Associate the given data object with the current thread in context.
 * This is typically used to attach the JNIEnv instance to the thread
 * it corresponds with.  Note that this will replace an already associated
 * data value without deleting it.
 *
 * Parameters:
 *     data - the data object to associate with the current thread
 *
 * Returns:
 *     NULL if the association was successful or a text message describing
 *     the reason for failure.
 */
JNIEXPORT char *JNICALL JEMCC_AssociateThreadValue(void *data);

/**
 * Retrieve the data object previously associated with the current thread.
 * Typically used to retrieve the JNIEnv instance attached to the thread
 * instance.
 *
 * Returns:
 *     The previously defined data object associated with the given thread
 *     or NULL if the indicated thread has no associated data.
 */
JNIEXPORT void *JNICALL JEMCC_RetrieveThreadValue();

/* <jemcc_start> */

/******************* Monitor (Condition) Management **********************/

/**
 * Obtain exclusive use/lock of the global monitor.  Use for control of
 * library wide resources.
 *
 * Returns:
 *     JNI_OK if the global monitor was allocated and entered or JNI_ERR
 *     if there was a failure.  No exception can be thrown as this method
 *     will be called during VM initialization.
 */
JNIEXPORT jint JNICALL JEMCC_EnterGlobalMonitor();

/**
 * Release the lock on the global monitor, allowing other threads to access
 * global information.
 */
JNIEXPORT void JNICALL JEMCC_ExitGlobalMonitor();

/**
 * NOTE: there are two levels of monitors defined here - Sys (system) monitors
 * and Java object monitors.  The system level monitors are heavyweight and
 * tied to the OS level mutex/condition implementations.  The object level
 * monitors are more lightweight using object-centric spin-locks.
 */

/* The system monitor data structure is opaque to the end user */
struct JEMCC_SysMonitor;
typedef struct JEMCC_SysMonitor JEMCC_SysMonitor;

/**
 * Create a new system monitor instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     A new system monitor instance or NULL if a memory allocation failed
 *     (an OutOfMemoryError exception will have been thrown in the current
 *     environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_SysMonitor *JNICALL JEMCC_CreateSysMonitor(JNIEnv *env);

/**
 * Destroy a system monitor instance which was generated through the
 * CreateSysMonitor method.  There should be no threads "in" the
 * monitor when this is called.
 *
 * Parameters:
 *     mon - the monitor instance to be destroyed
 *
 * Exceptions:
 *     None are directly thrown, but will abort if an internal
 *     mutex/condition destroy fails (indicative of an external
 *     problem).
 */
JNIEXPORT void JNICALL JEMCC_DestroySysMonitor(JEMCC_SysMonitor *mon);

/* The following have return codes from the list defined below */
#define JEMCC_MONITOR_OK 0
#define JEMCC_MONITOR_NOT_OWNER 1
#define JEMCC_MONITOR_TIMEOUT 2

/**
 * Enter a system monitor, usually to gain access to a piece of
 * thread sensitive code.  Only one thread at a time can be
 * "within" a particular monitor instance.  The system monitors track
 * multiple entries to the same monitor, so it is safe for a thread
 * to re-enter a monitor which it is already within (however, there must
 * be an exit for every enter).  This call will block if another thread
 * is within the monitor, until said the other thread exits and this thread 
 * is able to enter.
 *
 * Parameters:
 *     mon - the monitor instance to be entered
 *
 * Exceptions:
 *     None are thrown in the current environment, but the program
 *     will abort if the mutex lock fails (should never happen).
 */
JNIEXPORT void JNICALL JEMCC_EnterSysMonitor(JEMCC_SysMonitor *mon);

/**
 * Exit a system monitor.  This will release the monitor mutex for
 * another thread only if the number of exits matches the number of
 * enters.
 *
 * Parameters:
 *     mon - the monitor instance to be exited
 *
 * Returns:
 *     JEMCC_MONITOR_OK - the exit completed successfully (there is no
 *                        external indication if the mutex has actually been
 *                        released)
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 *
 * Exceptions:
 *     None are thrown in the current environment, but the program
 *     will abort if the mutex unlock fails (should never happen).
 */
JNIEXPORT jint JNICALL JEMCC_ExitSysMonitor(JEMCC_SysMonitor *mon);

/**
 * Perform a wait operation against a monitor.  This will block the current
 * thread instance until another thread issues a notify request against
 * the monitor.  The current thread must have entered (gained control of)
 * the specified monitor instance.
 *
 * Parameters:
 *     mon - the monitor instance to be waited against
 *
 * Returns:
 *     JEMCC_MONITOR_OK - the wait was successful and another thread has
 *                        notified this thread to restart
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 */
JNIEXPORT jint JNICALL JEMCC_SysMonitorWait(JEMCC_SysMonitor *mon);

/**
 * Perform a wait operation against a monitor with a timeout.  This will
 * block the current thread instance until another thread issues a notify
 * request against the monitor or the requested amount of time has elapsed.
 * The current thread must have entered (gained control of) the specified
 * monitor instance.
 *
 * Parameters:
 *     mon - the monitor instance to be waited against
 *     milli - the timeout period, in milliseconds
 *
 * Returns:
 *     JEMCC_MONITOR_OK - the wait was successful and another thread has
 *                        notified this thread to restart
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 *     JEMCC_MONITOR_TIMEOUT - the wait was successful and the specified time
 *                             period has elapsed without a notification from
 *                             another thread
 */
JNIEXPORT jint JNICALL JEMCC_SysMonitorMilliWait(JEMCC_SysMonitor *mon,
                                                 jlong milli);

/**
 * Perform a wait operation against a monitor with a timeout.  This will
 * block the current thread instance until another thread issues a notify
 * request against the monitor or the requested amount of time has elapsed.
 * The current thread must have entered (gained control of) the specified
 * monitor instance.  This method is identical to the above method but
 * supports a finer timeout resolution (although not all platforms may
 * support a nanosecond granularity).
 *
 * Parameters:
 *     mon - the monitor instance to be waited against
 *     milli - the milliseconds component of the timeout period
 *     nano - the nanoseconds component of the timeout period
 *
 * Returns:
 *     JEMCC_MONITOR_OK - the wait was successful and another thread has
 *                        notified this thread to restart
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 *     JEMCC_MONITOR_TIMEOUT - the wait was successful and the specified time
 *                             period has elapsed without a notification from
 *                             another thread
 */
JNIEXPORT jint JNICALL JEMCC_SysMonitorNanoWait(JEMCC_SysMonitor *mon,
                                                jlong milli, jint nano);

/**
 * Notify a single thread currently waiting on the specified monitor
 * to "awaken".  There is no ordering guarantee on thread waits and
 * notifications - if multiple threads are currently waiting on this
 * monitor, it should be assumed that the single thread to notify is
 * selected randomly.  The current thread must have entered (gained
 * control of) the specified monitor instance.
 *
 * If no threads are currently waiting on the monitor, this method does
 * nothing.
 *
 * Parameters:
 *     mon - the monitor instance to notify a waiting thread against
 *
 * Returns:
 *     JEMCC_MONITOR_OK - a single thread notification was successful or
 *                        there were no threads waiting on the monitor
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 */
JNIEXPORT jint JNICALL JEMCC_SysMonitorNotify(JEMCC_SysMonitor *mon);

/**
 * Notify all threads currently waiting on the specified monitor
 * to "awaken".  There is no ordering guarantee on thread waits and
 * notifications - if multiple threads are currently waiting on this
 * monitor, it should be assumed that the waiting threads will restart
 * randomly.  The current thread must have entered (gained control of)
 * the specified monitor instance.
 *
 * If no threads are currently waiting on the monitor, this method does
 * nothing.
 *
 * Parameters:
 *     mon - the monitor instance to notify all waiting threads against
 *
 * Returns:
 *     JEMCC_MONITOR_OK - all thread notifications were successful or
 *                        there were no threads waiting on the monitor
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 */
JNIEXPORT jint JNICALL JEMCC_SysMonitorNotifyAll(JEMCC_SysMonitor *mon);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Enter an object monitor, to acquire exclusive access to a section
 * of critical code.  This method uses the lightweight monitors attached
 * to the JEMCC_Object bitmask and corresponds to the use of the
 * synchronized{} statement within the JLS.  As a result, the standard
 * Java operations associated with the synchronization of Object instances
 * applies to this method.  Note: this method corresponds to linkage 217
 * of the JNI specification.
 *
 * Parameters:
 *     env - the VM environment which is currently in context (the associated
 *           thread may block if required)
 *     obj - the Object instance which contains the bitfield tagging word
 *           whose monitor is to be entered
 *
 * Returns:
 *     JNI_OK - the monitor entry was successful (this may return after
 *              an arbitrary blocking period)
 *     JNI_ENOMEM - a data structure allocation failed and an OutOfMemoryError
 *                  exception has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_EnterObjMonitor(JNIEnv *env, jobject obj);

/**
 * Exit an object monitor.  This is identical to reaching the end of a
 * synchronize{} block in the JLS.  This will release the "first"
 * environment thread which is waiting on this monitor if the owner
 * entry count reaches zero.  Naturally, the current thread must be the
 * current owner of the monitor to exit it.  Note: this method corresponds
 * to linkage 218 of the JNI specification.
 *
 * Parameters:
 *     env - the VM environment which is currently in context (which must
 *           own the monitor)
 *     obj - the Object instance which contains the bitfield tagging word
 *           whose monitor is to be exited
 *
 * Returns:
 *     JNI_OK - the monitor exit operation completed successfully
 *     JNI_ERR - the monitor exit failed and an exception has been thrown
 *               in the current environment
 *
 * Exceptions:
 *     IllegalMonitorStateException - the current environment/thread does
 *                                    not own the monitor
 */
JNIEXPORT jint JNICALL JEMCC_ExitObjMonitor(JNIEnv *env, jobject obj);

/**
 * Perform a wait operation against an object monitor.  This method corresponds
 * exactly to the Java Object.wait() method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context (which must
 *           own the monitor)
 *     obj - the Object instance which contains the bitfield tagging word
 *           to wait against
 *
 * Returns:
 *     JNI_OK - the wait completed successfully (and a notify occurred)
 *     JNI_ERR - an error occurred while executing the wait (related to
 *               the initialization of the wait state - an exception
 *               will have been thrown in the current environment)
 *     JNI_EINVAL - the thread was interrupted without a notify occurring
 *                  (an InterruptedException will be thrown in the current
 *                  environment)
 *
 * Exceptions:
 *     IllegalMonitorStateException - the current environment/thread does
 *                                    not own the monitor
 *     InterruptedException - another thread has issued an interrupt against
 *                            this thread
 */
JNIEXPORT jint JNICALL JEMCC_ObjMonitorWait(JNIEnv *env, jobject obj);

/**
 * Perform a wait operation against an object monitor, with a timeout period
 * specified.  This method corresponds exactly to the Java Object.wait(J)
 * method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context (which must
 *           own the monitor)
 *     obj - the Object instance which contains the bitfield tagging word
 *           to wait against
 *     milli - the timeout period, in milliseconds
 *
 * Returns:
 *     JNI_OK - the wait completed successfully (and a notify occurred)
 *     JNI_ERR - an error occurred while executing the wait (related to
 *               the initialization of the wait state - an exception
 *               will have been thrown in the current environment)
 *     JNI_EINVAL - the thread was interrupted without a notify occurring
 *                  (an InterruptedException will be thrown in the current
 *                  environment)
 *
 * Exceptions:
 *     IllegalArgumentException - the timeout period was negative
 *     IllegalMonitorStateException - the current environment/thread does
 *                                    not own the monitor
 *     InterruptedException - another thread has issued an interrupt against
 *                            this thread
 */
JNIEXPORT jint JNICALL JEMCC_ObjMonitorMilliWait(JNIEnv *env, jobject obj,
                                                 jlong milli);

/**
 * Perform a wait operation against an object monitor, with a timeout period
 * specified of a finer grain than the above method.  Note that not all
 * platforms will support granularity down to the nanosecond.  This method
 * corresponds exactly to the Java Object.wait(JI) method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context (which must
 *           own the monitor)
 *     obj - the Object instance which contains the bitfield tagging word
 *           to wait against
 *     milli - the milliseconds component of the timeout period
 *     nano - the nanoseconds component of the timeout period
 *
 * Returns:
 *     JNI_OK - the wait completed successfully (and a notify occurred)
 *     JNI_ERR - an error occurred while executing the wait (related to
 *               the initialization of the wait state - an exception
 *               will have been thrown in the current environment)
 *     JNI_EINVAL - the thread was interrupted without a notify occurring
 *                  (an InterruptedException will be thrown in the current
 *                  environment)
 *
 * Exceptions:
 *     IllegalArgumentException - the timeout period was negative or the
 *                                nanosecond parameter was not in the range
 *                                0-999999
 *     IllegalMonitorStateException - the current environment/thread does
 *                                    not own the monitor
 *     InterruptedException - another thread has issued an interrupt against
 *                            this thread
 */
JNIEXPORT jint JNICALL JEMCC_ObjMonitorNanoWait(JNIEnv *env, jobject obj,
                                                jlong milli, jint nano);

/**
 * Notify a single thread currently waiting on the specified object monitor
 * to "awaken".  This method corresponds exactly to the Object.notify()
 * method.
 *
 * If no threads are currently waiting on the object monitor, this method
 * does nothing.
 *
 * Parameters:
 *     env - the VM environment which is currently in context (which must
 *           own the monitor)
 *     obj - the Object instance which contains the bitfield tagging word
 *           to notify against
 *
 * Returns:
 *     JNI_OK - the monitor notify operation completed successfully
 *     JNI_ERR - the monitor notify failed and an exception has been thrown
 *               in the current environment
 *
 * Exceptions:
 *     IllegalMonitorStateException - the current environment/thread does
 *                                    not own the monitor
 */
JNIEXPORT jint JNICALL JEMCC_ObjMonitorNotify(JNIEnv *env, jobject obj);

/**
 * Notify all threads currently waiting on the specified object monitor
 * to "awaken".  This method corresponds exactly to the Object.notifyAll()
 * method.
 *
 * If no threads are currently waiting on the object monitor, this method
 * does nothing.
 *
 * Parameters:
 *     env - the VM environment which is currently in context (which must
 *           own the monitor)
 *     obj - the Object instance which contains the bitfield tagging word
 *           to notify against
 *
 * Returns:
 *     JNI_OK - the monitor notify operation completed successfully
 *     JNI_ERR - the monitor notify failed and an exception has been thrown
 *               in the current environment
 *
 * Exceptions:
 *     IllegalMonitorStateException - the current environment/thread does
 *                                    not own the monitor
 */
JNIEXPORT jint JNICALL JEMCC_ObjMonitorNotifyAll(JNIEnv *env, jobject obj);

/* <jemcc_end> */

/******************* Dynamic Library Management **********************/

/*
 * Methods and structures for dynamic library loading and method reference.
 * Note that this supports multiple instances of the dynamic loaders (for
 * different virtual machine configurations) as well as library attachments
 * for tracking classloaders (see the JNI specification regarding dynamic
 * library conflicts).
 */
typedef void *JEM_DynaLibLoader;
typedef void *JEM_DynaLib;
typedef void *JEM_DynaLibSymbol;

/**
 * Create/initialize a dynamic library loader instance.  Currently does
 * nothing, but provides for future implementation of multi-VM initialization 
 * support.
 *
 * Returns:
 *     An opaque reference to the dynamic loader management instance, or
 *     NULL if the initialization has failed (use the DynaLibErrorMsg
 *     method below to obtain details).
 */
JNIEXPORT JEM_DynaLibLoader JNICALL JEM_DynaLibLoaderInit();

/**
 * Release the dynamic library loader instance allocated by the 
 * DynaLibLoaderInit method.  May release references to associated dynamic 
 * library instances, depending on system capabilities.
 *
 * Parameters:
 *     loader - the allocated dynamic library loader to release
 */
JNIEXPORT void JNICALL JEM_DynaLibLoaderDestroy(JEM_DynaLibLoader loader);

/**
 * Load a dynamic library instance, with support for cross-referencing
 * an opaque data structure (attachment) for associated classloader management.
 *
 * Note: the mechanism for searching for the requested library instance (along
 * with whether or not absolute filenames are valid) is system dependent.
 * Refer to appropriate system documentation for details on the dynamic library
 * loading constraints.
 *
 * Parameters:
 *     loader - the dynamic library loader instance
 *     libFileName - the filename of the dynamic library to be loaded.  This
 *                   filename should be generated using the MapLibraryName
 *                   method below.
 *     attachment - a tracking data structure which can subsequently be
 *                  retrieved through the GetAttachment method below.
 *
 * Returns:
 *     A reference to the dynamic library instance, or NULL if the 
 *     initialization has failed (use the DynaLibErrorMsg method below 
 *     to obtain details).
 */
JNIEXPORT JEM_DynaLib JNICALL JEM_DynaLibLoad(JEM_DynaLibLoader loader,
                                              const char *libFileName,
                                              void *attachment);

/**
 * Retrieve the reference to the attachment object specified in the library
 * load call (for classloader cross-reference).
 *
 * Returns:
 *     The attachment reference supplied in the DynaLibLoad method call.
 */
JNIEXPORT void *JNICALL JEM_DynaLibGetAttachment(JEM_DynaLib library);

/**
 * Release a dynamic library instance.  At a minimum, this will delete
 * the internal structures used to track the library attachment.  Whether the
 * actual dynamic library instance is detached is system dependent.
 *
 * Note: this method does not delete the attachment object supplied during
 * the DynaLibLoad call.  It is the responsibility of the client to manage
 * the attachment instance.
 *
 * Parameters:
 *     library - the dynamic library instance to be released
 */
JNIEXPORT void JNICALL JEM_DynaLibRelease(JEM_DynaLib library);

/**
 * Retrieve a named symbol from a dynamic library instance (in this system,
 * a JNI or JEMCC method instance).
 *
 * Parameters:
 *     library - the dynamic library instance to pull the reference symbol from
 *     symName - the (method) name of the symbol to retrieve
 *
 * Returns:
 *     The resolved symbol reference, or NULL if the symbol is not found (note 
 *     that it is possible to obtain NULL symbols from dynamic libraries, but
 *     for JNI/JEMCC methods that will not be the case).
 */
JNIEXPORT JEM_DynaLibSymbol JNICALL JEM_DynaLibGetSymbol(JEM_DynaLib library,
                                                         const char *symName);

/**
 * Obtain the message associated with the last error that occurred in the 
 * dynamic library system.  Calling this method also clears the error
 * buffer (subsequent call would not show the error again).
 *
 * Returns:
 *     The text message associated with the last dynamic library access
 *     error, or NULL if no error has occurred.
 */
JNIEXPORT char *JNICALL JEM_DynaLibErrorMsg();

/**
 * Convert a library basename (e.g. the name used in linking) to the system
 * dependent filename (e.g. in UNIX, lib***.so).  Generally used to construct
 * the "proper" filename for the DynaLibLoad call above.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     libName - the basename of the library to construct the full name for
 *
 * Returns:
 *     The system specific library filename or NULL if an error occurred
 *     (an exception will have been thrown in the current environment).
 *     The returned name must be free'd by the caller.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed for the result
 */
JNIEXPORT char *JNICALL JEM_MapLibraryName(JNIEnv *env, const char *libName);

/******************* Foreign Function Interfaces **********************/

/* Forward declaration for function definition */
union JEM_DescriptorInfo;

/**
 * Method by which foreign functions (not directly integrated into the
 * JEMCC VM) are called.  Used exclusively by the JNI method calling
 * methods.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     thisObj - if non-NULL, method is non-static and this is the
 *               'this' object reference
 *     fnRef - the reference to the foreign function to be called
 *     fnDesc - the Java descriptor of the JNI method being called.  Used
 *              to parse/format both the function arguments and the return
 *              information.
 *     argList - an array of the arguments to the foreign function.  Must
 *               contain at least the number of members indicated by the
 *               function descriptor.
 *     retVal - reference to the structure which is to contain the return
 *              value from the foreign function.  May be NULL for void
 *              functions.
 *
 * Returns:
 *     JNI_OK if the method setup and call was successful, JNI_ERR if 
 *     a memory allocation or other error occurred (an exception will 
 *     have been thrown in the current environment).  Note that the
 *     native method may itself throw an exception - this will not be
 *     caught by this method (should be managed by the caller).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     InternalError - an invalid argument condition occurred
 *     StackOverflowError - unable to allocate stack segment for foreign call
 *     Any other exception thrown within the foreign function.
 */
JNIEXPORT jint JNICALL JEM_CallForeignFunction(JNIEnv *env, 
                                               JEMCC_Object *thisObj,
                                               void *fnRef,
                                               union JEM_DescriptorInfo *fnDesc,
                                               JEMCC_ReturnValue *argList,
                                               JEMCC_ReturnValue *retVal);

#endif
