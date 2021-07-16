/**
 * Definition list and access methods for the "core" virtual machine classes.
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

#ifndef JEM_VMCLASSES_H
#define JEM_VMCLASSES_H 1

/* NOTE: this is auto-read by jem.h, so it shouldn't be directly read */

/**
 * Method by which the "core" classes (those required by the JEMCC virtual
 * machine internals) are initially created.  Should only be called once with
 * the primary (initial) environment instance for the new virtual machine.
 *
 * Parameters:
 *     env - the initial environment for a new virtual machine instance
 *
 * Returns:
 *     JNI_OK - all initialization completed successfully, all VM core
 *              classes are now available
 *     JNI_ERR - an initialization error occurred (should not happen) and
 *               the virtual machine is effectively "invalid" (an exception
 *               will have been thrown in the environment, if possible)
 *     JNI_ENOMEM - a memory allocation error has occurred and the virtual
 *                  machine is effectively "invalid" (an exception will have
 *                  been thrown in the environment, if possible)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - JEMCC initialization information is invalid
 *     LinkageError - one of the class linkages failed
 */
JNIEXPORT jint JNICALL JEM_InitializeVMClasses(JNIEnv *env);

/* <jemcc_start> */

/************************ Core VM Classes ****************************/

/**
 * Enumeration set of the "core" classes defined in the JEMCC virtual machine
 * initialization.  External entities can use this enumeration with the
 * following methods to access these classes without name lookups, etc.
 * Note that internal methods can use the VM_CLASS macro to directly read
 * the class storage table in the VM structures.
 */
typedef enum {
    JEMCC_Primitive_Boolean = 0, JEMCC_Primitive_Byte, JEMCC_Primitive_Char,
    JEMCC_Primitive_Short, JEMCC_Primitive_Int, JEMCC_Primitive_Float,
    JEMCC_Primitive_Long, JEMCC_Primitive_Double, JEMCC_Primitive_Void,
    JEMCC_Primitive_Array,

    JEMCC_Array_Boolean, JEMCC_Array_Byte, JEMCC_Array_Char,
    JEMCC_Array_Short, JEMCC_Array_Int, JEMCC_Array_Float,
    JEMCC_Array_Long, JEMCC_Array_Double,

    JEMCC_Class_Object, JEMCC_Class_Serializable, JEMCC_Class_Class,

    JEMCC_Class_Cloneable, JEMCC_Class_Runnable, JEMCC_Class_ClassLoader, 
    JEMCC_Class_String, JEMCC_Class_Throwable,

    JEMCC_Class_ArithmeticException, JEMCC_Class_ArrayIndexOutOfBoundsException,
    JEMCC_Class_ArrayStoreException, JEMCC_Class_ClassCastException,
    JEMCC_Class_ClassNotFoundException, JEMCC_Class_CloneNotSupportedException,
    JEMCC_Class_Exception, JEMCC_Class_IllegalAccessException,
    JEMCC_Class_IllegalArgumentException, 
    JEMCC_Class_IllegalMonitorStateException, JEMCC_Class_IllegalStateException,
    JEMCC_Class_IllegalThreadStateException,
    JEMCC_Class_IndexOutOfBoundsException, JEMCC_Class_InstantiationException,
    JEMCC_Class_InterruptedException, JEMCC_Class_NegativeArraySizeException,
    JEMCC_Class_NoSuchFieldException, JEMCC_Class_NoSuchMethodException,
    JEMCC_Class_NullPointerException, JEMCC_Class_NumberFormatException,
    JEMCC_Class_RuntimeException, JEMCC_Class_SecurityException,
    JEMCC_Class_StringIndexOutOfBoundsException,

    JEMCC_Class_AbstractMethodError, JEMCC_Class_ClassCircularityError, 
    JEMCC_Class_ClassFormatError, JEMCC_Class_Error, 
    JEMCC_Class_ExceptionInInitializerError, JEMCC_Class_IllegalAccessError, 
    JEMCC_Class_IncompatibleClassChangeError, JEMCC_Class_InstantiationError, 
    JEMCC_Class_InternalError, JEMCC_Class_LinkageError, 
    JEMCC_Class_NoClassDefFoundError, JEMCC_Class_NoSuchFieldError, 
    JEMCC_Class_NoSuchMethodError, JEMCC_Class_OutOfMemoryError, 
    JEMCC_Class_StackOverflowError, JEMCC_Class_ThreadDeath, 
    JEMCC_Class_UnknownError, JEMCC_Class_UnsatisfiedLinkError,
    JEMCC_Class_VerifyError, JEMCC_Class_VirtualMachineError, 

    JEMCC_Class_IOException,

    JEMCC_VM_CLASS_TBL_SIZE

} JEMCC_VMClassIndex;

/**
 * Obtain the class reference for the VM core class based on the index
 * value.  This method is provided for external entities which do not have
 * the VM_CLASS macro available.
 *
 * Parameters:
 *     env - an environment instance associated with the VM core
 *     index - the index of the VM core class to retrieve
 *
 * Returns:
 *     The requested VM class instance - NULL will never occur as
 *     the VM initialization will completely fail if any class cannot
 *     be created.
 */
JNIEXPORT JEMCC_Class *JNICALL JEMCC_GetCoreVMClass(JNIEnv *env, 
                                                    JEMCC_VMClassIndex index);

/**
 * The following methods match the allocate/throw/catch methods defined 
 * in jem.h but utilize the VM class index for efficiency.
 */

/**
 * Allocate the memory block for a java.lang.Object based instance, including
 * the internal attachment data, based on the core index value.  This does
 * NOT call any <init> constructors of the given class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to create an instance of
 *     objDataSize - if greater than zero, allocate a native object data
 *                   block of this size in bytes.  The object class must
 *                   be a JEMCC class instance which has defined a native
 *                   reference pointer
 *
 * Returns:
 *     NULL if memory allocation failed, otherwise the object instance.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_AllocateObjectIdx(JNIEnv *env,
                                                     JEMCC_VMClassIndex idx,
                                                     juint objDataSize);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * index.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to be created and thrown
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableIdx(JNIEnv *env,
                                                  JEMCC_VMClassIndex idx,
                                                  JEMCC_Object *causeThrowable,
                                                  const char *msg);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * index.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * This method is identical to the above method except that it allows for
 * vararg message construction.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to be created and thrown
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableIdxV(JNIEnv *env,
                                                   JEMCC_VMClassIndex idx,
                                                   JEMCC_Object *causeThrowable,
                                                   ...);

/**
 * "Catch" an instance of a core VM throwable class (based on an outstanding
 * exception in the current environment).  Will return and clear the pending
 * exception if an exception exists which is assignable to the indicated core 
 * class instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     idx - the index of the core VM class to be caught
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_CatchThrowableIdx(JNIEnv *env,
                                                        JEMCC_VMClassIndex idx);

/* <jemcc_end> */

/**
 * Load a "system" class (available via the CLASSPATH definitions in the
 * VM properties).  This method is not intended for general consumption -
 * it is equivalent to calling ClassLoader.findSystemClass() (e.g. load through
 * the system classloader) but does NOT perform the associated checks for
 * already loaded classes or classes available through the bootstrap loader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     className - the fully qualified name of the class to load
 *     classInst - a class instance reference through which the desired class
 *                 is returned (if found)
 *
 * Returns:
 *     JNI_OK - the class was successfully located/constructed and has been
 *              returned in the classInst reference
 *     JNI_ERR - an error has occurred in the loading/parsing/linking of
 *               the requested class (an exception has been thrown in the
 *               current environment)
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *     JNI_EINVAL - the requested class was not found or recognized - no
 *                  exception is thrown in this instance
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     <other class errors> - a failure occurred in the parsing, linking
 *                            or verification of the class (if found)
 */
JNIEXPORT jint JNICALL JEM_GetSystemClass(JNIEnv *env, const char *className,
                                          JEMCC_Class **classInst);

#endif
