/* <jemcc_start> */
/**
 * Compiled Class function/structure definitions for the Java Embedded Machine.
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

#ifndef JEM_JEMCC_H
#define JEM_JEMCC_H 1

/* Ensure inclusion of JNI definitions */
#include "jni.h"

/*************** Core JEMCC Structure Definitions *****************/

/**
 * This is the core JEMCC object type definition, which corresponds to
 * a Java object instance (from which all things Java are derived).  It
 * has been defined here (in the base jemcc file) to support internal/external
 * JEMCC VM function prototyping.
 */

struct JEMCC_Object;
typedef struct JEMCC_Object JEMCC_Object;
typedef struct JEMCC_Class JEMCC_Class;

/**
 * Forward type definition for the JEMCC VM frame management.  See below
 * (or cpu.h) for macros/methods used in managing the VM frame information
 * for argument parsing and data return.
 */
struct JEMCC_VMFrame;
typedef struct JEMCC_VMFrame JEMCC_VMFrame;

/**
 * Union which defines the return storage reference used by JEMCC
 * method implementations to return data to the caller.  Note that
 * the smaller-than-an-int data types do not appear here, to simplify
 * the case conditions and remain consistent with the stack model of
 * the Java VM.
 */
typedef union JEMCC_ReturnValue {
    jint intVal;
    jlong longVal;
    jfloat fltVal;
    jdouble dblVal;
    JEMCC_Object *objVal;
} JEMCC_ReturnValue;

/*************** Package Management Information *****************/

/**
 * Type definition for the package initialization information carrier.
 */
struct JEMCC_PkgInitData;
typedef struct JEMCC_PkgInitData JEMCC_PkgInitData;

/**
 * Type definition for a JEMCC package initialization function.  These
 * initializers can serve three purposes; to bulk define an initial set of
 * classes for a package (e.g. core package classes and exceptions) in 
 * an efficient manner, perform some setup (reference lookups, initial
 * configuration, etc.) on initialization, or provide the definition of
 * an internal JEMCC package-based classloader (most common operation, 
 * see below).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the package initialization is to
 *              occur, NULL if the VM bootstrap loader is to be used
 *     pkgName - the name of the package being initialized.  Allows for
 *               a single initializer for multiple packages
 *     initData - the current package initializer definition, which
 *                should be updated by the initialization function with the
 *                next initialization state (assuming an error has not
 *                occurred)
 *
 * Returns:
 *     JNI_OK - this package initialization has completed successfully
 *     JNI_ERR - a format/link error has occurred.  Package initialization
 *               cannot be completed and a format/link exception has been
 *               thrown in the current environment
 *     JNI_ENOMEM - a memory error has occurred.  Package initialization may
 *                  be subsequently completed and a memory exception has been
 *                  thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed; this is always
 *                        thrown, but does not preclude subsequent package
 *                        initialization (in case of recovery)
 *     ClassFormatError - JEMCC initialization data is invalid and the
 *                        specified package initialization has been
 *                        terminated
 *     LinkageError - an external JEMCC reference could not be resolved and
 *                    the specified package initialization has been
 *                    terminated
 */
typedef jint (*JEMCC_PackageInitFn)(JNIEnv *env, JEMCC_Object *loader,
                                    const char *pkgName,
                                    JEMCC_PkgInitData *initData);

/**
 * Type definition for an internal JEMCC package-based classloader.  This 
 * roughly corresponds to a conventional Java classloader, but in this case 
 * it obtains the class through a JEMCC defined package manager (typically, the
 * class will be a JEMCC class instance as well).  Note that this method 
 * does not receive the initialization info structure (as for the PackageInit
 * method above) - in theory, the package classloader should never know when
 * it is "done" (it could keep track, but would be a waste of resources).
 * Also note that it is permissible for this loader to define more than one
 * class for a single request, where multiple dependencies are involved.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the class definition is to
 *              occur, NULL if the VM bootstrap loader is to be used
 *     className - the name of the class to be loaded.  This classname will
 *                 not be fully qualified (no leading package) as it is
 *                 assumed that the package definition is appropriate
 *     classInst - a reference pointer through which the class instance
 *                 (if recognized and defined) is returned
 *
 * Returns:
 *     JNI_OK - the requested class was recognized and created successfully
 *              (as returned through the classInst reference)
 *     JNI_ENOMEM - a memory allocation occurred during class creation and
 *                  a memory exception has been thrown in the current
 *                  environment
 *     JNI_ERR - a format or linkage error occurred during the creation of the
 *               class - the requested class was not created and an appropriate
 *               exception has been thrown in the current environment
 *     JNI_EINVAL - the given classname is invalid (not recognized by this
 *                  loader) and no class was loaded.  No exception should be
 *                  thrown as this is an internal action and another loader
 *                  instance may be able to provide the class
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed; this is always
 *                        thrown, but does not preclude subsequent class
 *                        reload (in case of recovery)
 *     ClassFormatError - JEMCC initialization data is invalid and the
 *                        class is not defined.  This is "recoverable" in
 *                        the sense that the classloader may be called again
 *                        for the same classname and throw the exception
 *                        again
 *     LinkageError - an external JEMCC reference could not be resolved and
 *                    the class is not defined.  This is also "recoverable"
 *                    as the classloader may be called again and the exception
 *                    is rethrown
 */
typedef jint (*JEMCC_PackageClassLdrFn)(JNIEnv *env, JEMCC_Object *loader,
                                        const char *className,
                                        JEMCC_Class **classInst);

/**
 * The package initialization state structure, used to manage the package
 * initializers inside the VM and to allow the bulk package initializers
 * to signal subsequent operational states.
 *
 * The following describes the data values and state transitions which are
 * valid for the package initialization handlers, according to the
 * "initial" state of the package initialization data:
 *
 * PKGLOAD_LIB_NAME
 *     This is an internal state of the VM package information, which
 *     corresponds to the original definition of the package library
 *     instances to load.  A package initialization function will never
 *     see this state, as the VM internally converts to the INIT_FN state
 *     before calling the first library initializer for the package.
 *
 * PKGLOAD_INIT_FN
 *     This state corresponds to a package initialization function which
 *     performs bulk class definition (as defined in the pkgInit member
 *     of the handler union).  On successful completion, the next bulk
 *     initializer in the chain may be defined, a package classloader
 *     may be specified or the package initialization may be marked as
 *     complete.
 *
 * PKGLOAD_CLASSLDR_FN
 *     This state corresponds to an internal package classloader which
 *     builds/provides classes by name.  Once an initialization definition
 *     reaches this state, the package initialization info structure will
 *     never again be sent to an initialization method and hence there will
 *     be no exit from this state (the classloader is permanently defined).
 *     The function which provides the package class definition/loading
 *     capabilities is defined in the pkgClassLdr member of the handler union.
 *
 * PKGLOAD_COMPLETED
 *     This state indicates that all JEMCC initialization options for this
 *     package are completed and no further JEMCC definitions can appear
 *     via this initialization structure.  Note that this does not preclude
 *     other packages from defining classes in this package context (as
 *     subpackages), although such a situation would be quite unusual.
 */

#define PKGLOAD_LIB_NAME    1
#define PKGLOAD_INIT_FN     2
#define PKGLOAD_CLASSLDR_FN 3
#define PKGLOAD_COMPLETED   4

struct JEMCC_PkgInitData {
    int initState;

    union {
        char *pkgLibName;
        JEMCC_PackageInitFn pkgInitFn;
        JEMCC_PackageClassLdrFn pkgClassLdrFn;
    } handler;
};

/**
 * Method by which a static implementation can register primary package
 * initializers or a package initializer can register a child (nested)
 * package initializer.  Note that this method cannot be used to alter 
 * the initialization sequence for a package which is already defined;
 * the modification of the PkgInitData structure is intended to handle that 
 * case (see the RegisterMultiPkgInitFn method below for other information).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the initializer definition is to
 *              be stored, NULL if the VM bootstrap loader is to be used
 *     pkgName - the name of the package to be defined (e.g. 'java.lang')
 *     initFn - the function reference to the initializer for this package
 *
 * Returns:
 *     JNI_OK - the package initializer definition occurred successfully
 *     JNI_ENOMEM - a memory allocation occurred during definition of the
 *                  initializer and a memory exception has been thrown in
 *                  the current environment
 *     JNI_ERR - a package initializer/classloader is already defined for the
 *               specified package.  No exception has been thrown
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_RegisterPkgInitFn(JNIEnv *env,
                                               JEMCC_Object *loader,
                                               const char *pkgName,
                                               JEMCC_PackageInitFn initFn);

/**
 * Method by which a static implementation can register a primary package
 * classloader or a package initializer can register a child (nested)
 * package classloader.  Note that this method cannot be used to alter 
 * the initialization sequence for a package that is already defined; 
 * the modification of the InitData structure is intended to handle that 
 * case for the current package initializer (see the RegisterMultiPkgInitFn 
 * method below for other information).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the initializer definition is to
 *              be stored, NULL if the VM bootstrap loader is to be used
 *     pkgName - the name of the package to be defined (e.g. 'java.lang')
 *     ldrFn - the function reference to the internal package classloader for
 *             this package
 *
 * Returns:
 *     JNI_OK - the package classloader definition occurred successfully
 *     JNI_ENOMEM - a memory allocation occurred during definition of the
 *                  initializer and a memory exception has been thrown in
 *                  the current environment
 *     JNI_ERR - a package initializer/classloader is already defined for the
 *               specified package.  No exception has been thrown
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_RegisterPkgClassLdrFn(JNIEnv *env,
                                             JEMCC_Object *loader,
                                             const char *pkgName,
                                             JEMCC_PackageClassLdrFn loaderFn);

/**
 * Data structure used to initialize libraries containing multiple packages 
 * or to bulk define a set of statically linked package initializers via
 * the RegisterMultiPkgInitFn method below.  Note that only one of the
 * initialization models (either initializer or classloader function) can
 * be defined for a specific package (the initFn and loaderFn references
 * below) and if both are NULL, the package is marked complete.
 */
typedef struct JEMCC_MultiPackageInitData {
    char *pkgName;
    JEMCC_PackageInitFn initFn;
    JEMCC_PackageClassLdrFn loaderFn;
} JEMCC_MultiPackageInitData;

/**
 * Method used to register a set of package initializer or classloader
 * functions.  Intended for use with libraries containing multiple
 * package instances or for the bulk definition of packages in static 
 * implementations.  Unlike the above register methods, this method can 
 * replace an already existing (but unloaded) package library definition
 * or the package initialization definition which is in progress.  All of
 * the package initialization is performed within a single synchronized
 * block to avoid race conditions with other initializers and lookups.
 * However, the operation is not all-or-none; if a memory failure occurs,
 * the already defined package instances will be left in place.
 *
 * Note: this method ignores definitions of other initialization functions 
 *       which are in progress.  This way, a general init function which 
 *       defines 'a', 'b' and 'c' that is simultaneously called through the 
 *       'a' and 'b' library entries will operate correctly.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader into which the initializer definitions are to
 *              be stored, NULL if the VM bootstrap loader is to be used
 *     initData - the multi-package/bulk initialization data to be loaded
 *     initCount - the number of entries in the initData array
 *
 * Returns:
 *     JNI_OK - the package definitions occurred successfully
 *     JNI_ENOMEM - a memory allocation occurred during definition of the
 *                  initializers and a memory exception has been thrown in
 *                  the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_RegisterMultiPkgInitFn(JNIEnv *env,
                                          JEMCC_Object *loader,
                                          JEMCC_MultiPackageInitData *initData,
                                          jsize initCount);

/*************** JEMCC Definition Structures/Methods ******************/

/**
 * Definitions of the possible return value types from a JEMCC class method
 * (as returned from the JEMCC_MethodFn prototype below).  These are an 
 * enumeration of error cases (where an execution exception is involved)
 * or a typed return value (which *must* be consistent with the signature of 
 * the method).
 *
 * While the typed return values should be obvious, the following describes
 * the error codes and details how the virtual machine handles the various
 * cases:
 *
 * JEMCC_ERR - an exception condition has occurred which is not being
 *             handled by the JEMCC method.  There are two conditions where
 *             this is returned.  First, the JEMCC method has directly thrown
 *             an exception and the VM has already unwound the stack to the
 *             appropriate "catcher" (whose frame will be the next execution
 *             context).  The other situation is where a call made from within
 *             this method has triggered an exception condition - the stack
 *             may have unwound from a lower-level frame to the frame
 *             associated with this method.  In this case, the exception will
 *             be re-thrown to a higher level handler while retaining its
 *             original stack context (for error reporting).  A common return
 *             code is used as sometimes it is not possible to know which
 *             type of exception condition has occurred.
 *
 * JEMCC_NULL_EXCEPTION - a convenience return code where a null reference
 *                        check has failed within the JEMCC method (to avoid
 *                        repetitions of the throw(NullPointerException) call).
 *                        Returning this code causes the virtual machine to
 *                        automatically throw the NullPointerException in the
 *                        context of the JEMCC method (e.g. prior to unwinding
 *                        the JEMCC execution frame).
 *
 * Note that none of these return codes overlap with the JNI return codes, to
 * properly capture any coding errors (e.g. using JNI_ERR instead of JEMCC_ERR).
 */
#define JEMCC_ERR 1
#define JEMCC_NULL_EXCEPTION 2

#define JEMCC_RET_VOID 10
#define JEMCC_RET_INT 11
#define JEMCC_RET_LONG 12
#define JEMCC_RET_FLOAT 13
#define JEMCC_RET_DOUBLE 14
#define JEMCC_RET_OBJECT 15
#define JEMCC_RET_ARRAY 15     /* Optional define for readability */

/**
 * The definition of a JEMCC class method body (native code implementation
 * of a Java method). Stored in the class structures in place of the
 * corresponding bytecode method definition.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     frame - the current virtual machine frame which has been set up
 *             for the execution of this method (providing for arguments
 *             and stack space for additional method calls)
 *     retVal - if the method has a non-void return value and no
 *              exception condition has occurred (see the return type
 *              code details), the value returned from this method is stored 
 *              in this reference
 *
 * Returns:
 *     Either one of the exception return codes, which indicates the method
 *     has not completed successfully and an exception has been/should be
 *     thrown in the current environment or one of the non-exception return 
 *     codes which indicate the type of return value (where retVal has been 
 *     set except in the case of a void method).
 *
 * Exceptions:
 *     Any number, depending on the available method exceptions and any
 *     runtime exceptions/errors which may unexpectedly arise.
 */
typedef jint (JNICALL *JEMCC_MethodFn)(JNIEnv *env, JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal);

/**
 * Static definition structure used to define the Java methods for a
 * JEMCC specified class instance.  Members are as follows:
 *
 *   accessFlags - the ACC_ flags which define the method permissions and
 *                 types (ACC_JEMCC is automatically set)
 *   name - the name of the method (without arguments or function syntax)
 *   descriptor - the Java encoded string descriptor of the method
 *   codeBody - the native function which provides the functionality for this
 *              method
 */
typedef struct JEMCC_MethodData {
    juint accessFlags;
    char *name;
    char *descriptor;
    JEMCC_MethodFn codeBody;
} JEMCC_MethodData;

/**
 * "Special" object management method for JEMCC class instances.  Provides
 * a functional interface for specialized VM operations such as garbage
 * collection, serialization, etc.  Utilizes a single function pointer with
 * a case indicator for flexibility and compactness in the class storage
 * structure - these management operations are "expensive" so the overhead of
 * the switch statement is minimal.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     action - the management action being requested (see the JEMCC_OM defines)
 *     obj - the object on which the management request is being made
 *     exObj - an extension object whose purpose depends on the management
 *             request; for serialization requests, this would be the object
 *             input/output stream
 * Returns:
 *     JNI_OK - the management request is supported and was completed 
 *              successfully.  If there are generic actions defined by the VM
 *              to support this request, they will *not* be executed
 *     JNI_ERR - the management request was supported but an error occurred
 *               in performing the request (an exception has been thrown
 *               in the current environment)
 *     JNI_EINVAL - the management request is not supported by this method.  If
 *                  there are generic actions defined by the VM to support this
 *                  request, they will be executed instead
 */
#define JEMCC_OM_GC_COLLECT 1
#define JEMCC_OM_SERIAL_READ 2
#define JEMCC_OM_SERIAL_WRITE 3

typedef jint (JNICALL *JEMCC_ObjMgmtFn)(JNIEnv *env, jint action,
                                        JEMCC_Object *obj, JEMCC_Object *exObj);

/**
 * Static definition structure used to define the Java fields for a
 * JEMCC specified class instance.  Note that in most cases, these
 * fields are all public, as private data may be more efficiently
 * managed as object attachment information.  Members are as follows:
 *
 *   accessFlags - the ACC_ flags which define the field permissions and
 *                 types (ACC_JEMCC is automatically set)
 *   name - the name of the field
 *   descriptor - the Java encoded string descriptor of the field
 *   offset - predefined field offset for direct structure access,
 *            -1 if offset should be determined in the standard fashion
 */
typedef struct JEMCC_FieldData {
    juint accessFlags;
    char *name;
    char *descriptor;
    jint offset;
} JEMCC_FieldData;

/**
 * Convenience macro for determining structural offset of field entries
 * from structures.
 */
#define JEMCC_OffsetOf(type, field) ((jint) ((jubyte *) &(((type *) 0)->field)))

/**
 * External method for locating class instances by name.  Will operate with
 * a specific classloader instance or the VM bootstrap classloader.  This
 * method is similar to the ClassLoader.loadClass() method (and does in fact
 * call it where appropriate), but there are a number of subtle differences:
 *
 *   - it always reads the JVM bootstrap classtable first, to allow rapid
 *     retrieval of bootstrap JEMCC classes and to prevent any possible
 *     overriding of the bootstrap specific classes
 *   - array classes are constructed within this method, but directly
 *     manipulate the classloader from which the component class is obtained
 *     (similar to utilizing the classloading chain without the calling 
 *     sequences)
 *   - the classloader's internal classtable is consulted for the class
 *     instance prior to any attempt to load/define the class (rather than
 *     relying on any loadClass() hashing instances)
 *   - the JEMCC package initializers are called before the loadClass()
 *     method on the classloader to permit native integration (and to
 *     avoid potential conflicts with concurrent bytecode class
 *     implementations)
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to supply the requested class.  NULL
 *              if the class is to be loaded from the VM bootstrap loader
 *     className - the fully qualified name of the class to be loaded (both
 *                 '/' and '.' separators are recognized, which are accepted
 *                 by the ClassLoader.loadClass() method)
 *     isLink - if JNI_TRUE, this class is being located as part of the
 *              construction of another class (linking) and should throw
 *              NoClassDefFoundError if the requested instance is not found.
 *              If JNI_FALSE, throw ClassNotFoundException instead (from a
 *              forName() call, for instance)
 *     classInst - a class instance reference through which the desired class
 *                 is returned (if found)
 *
 * Returns:
 *     JNI_OK - the class was successfully located/constructed and has been
 *              returned in the classInst reference
 *     JNI_ERR - an error has occurred in the loading/parsing/linking of
 *               the requested class (an exception has been thrown in the
 *               current environment, based on the isLink value)
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *     JNI_EINVAL - the requested class was not found or recognized and a
 *                  ClassNotFoundException or NoClassDefFoundError has been 
 *                  thrown in the current environment, depending on the isLink
 *                  value)
 *
 * Exceptions:
 *     Many exceptions are possibly thrown, all corresponding to some failure
 *     to load the class (format, data, memory, etc.).  Note that an exception
 *     is guaranteed to be thrown (often ClassNotFoundException or 
 *     NoClassDefFoundError) if no class instance is returned from this 
 *     method (and vice versa, no exception is thrown if a class is returned).
 */
JNIEXPORT jint JNICALL JEMCC_LocateClass(JNIEnv *env, JEMCC_Object *loader,
                                         const char *className, jboolean isLink,
                                         JEMCC_Class **classInst);

/**
 * Build a JEM Compiled Class instance.  The creation of a JEM compiled
 * class is done in three stages - the initial construction of the class
 * structure (this method), the linking of the class against class, field
 * and method references (the LinkClass method) and the definition of the
 * class instance in the classloader (the RegisterClass method).  Note that
 * the definition of JEMCC classes must be carefully handled to avoid reference
 * loops, as the external reference linking is performed prior to the actual
 * class definition operation (which may be altered by the definition).  In
 * cases where such loops exist, the RegisterClasses method is used to
 * "simultaneously" define a set of classes with such cross-linkages.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the constructed class
 *              instance.  Note that the class will not be inserted into
 *              the classloader tables at this time, only referenced
 *     accessFlags - the ACC_ flags which define the class permissions and
 *                   types (ACC_JEMCC is automatically set)
 *     className - the fully qualified name of the class being constructed
 *                 (period (.) package separators are used in this case)
 *     superClass - if non-NULL, the superclass of this class.  Note that
 *                  JEMCC classes should ideally have another JEMCC class as
 *                  the superclass and only core Object related classes
 *                  should have a NULL superclass
 *     interfaces - an array reference to a list of direct interfaces
 *                  implemented by this class
 *     interfaceCount - the number of interface instances in the interfaces
 *                      array
 *     methodInfo - an array of JEMCC method definitions for this class
 *     methodCount - the number of method records in the methodInfo array
 *     objMgmtFn - a reference to the object management method, NULL if
 *                 default management operations apply to this class
 *                 (no override function exists)
 *     fieldInfo - an array of JEMCC field definitions for this class
 *     fieldCount - the number of field records in the fieldInfo array
 *     classInst - a class instance reference through which the
 *                 constructed class instance is returned
 *
 * Returns:
 *     JNI_OK - the construction of the JEMCC class was successful, the 
 *              initial class instance is returned through the classInst
 *              reference
 *     JNI_ERR - a definition error occurred during the creation of the class.
 *               A ClassFormatError/AbstractMethodError will have been 
 *               thrown in the current environment and this class should 
 *               be considered permanently invalid
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory 
 *                  error has been thrown in the current environment (but the
 *                  class should be considered "reloadable")
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - provided JEMCC initialization data is invalid
 *     AbstractMethodError - this concrete class is missing a non-abstract
 *                           method definition
 */
JNIEXPORT jint JNICALL JEMCC_BuildClass(JNIEnv *env, JEMCC_Object *loader,
                                        juint accessFlags,
                                        const char *className,
                                        JEMCC_Class *superClass,
                                        JEMCC_Class **interfaces,
                                        jsize interfaceCount,
                                        JEMCC_MethodData *methodInfo,
                                        jsize methodCount,
                                        JEMCC_ObjMgmtFn *objMgmtFn,
                                        JEMCC_FieldData *fieldInfo,
                                        jsize fieldCount,
                                        JEMCC_Class **classInst);

/**
 * Linkage type indicators for the JEMCC_LinkData structure, as specified
 * in the linkType member.
 */
#define JEMCC_LINK_CLASS 1    /* Linkage is to a class/interface instance */
#define JEMCC_LINK_METHOD 2   /* Linkage is to a class/interface method */
#define JEMCC_LINK_FIELD 3    /* Linkage is to a class/interface field */

/**
 * Static definition structure used to initialize external linkages for
 * a JEMCC class instance.  This includes external classes/interfaces,
 * their methods and fields and can involve either JEMCC or "conventional"
 * Java classes.  From within a JEMCC class method, the linkage information 
 * can be retrieved through the GetXXXReference methods for use. Structure 
 * members are:
 *
 *   linkType - the type of external linkage to obtain (see JEMCC_LINK defines)
 *   extClassRef - the class instance to link against, either from a local
 *                 reference (e.g. during a package/group initialization), a 
 *                 VM core class instance or obtained using the LocateClass 
 *                 method.  NULL if this is a method or field linkage within 
 *                 this class
 *   fieldMethodName - the name of the referenced field or method
 *   fieldMethodDesc - the Java descriptor of the referenced field or method
 */
typedef struct JEMCC_LinkData {
    int linkType;
    JEMCC_Class *extClassRef;
    char *fieldMethodName;
    char *fieldMethodDesc;
} JEMCC_LinkData;

/**
 * Perform the linkage of a JEMCC class instance to external class
 * elements.  This method takes a list of class, field and method
 * string references and builds the class reference tables which can then
 * be retrieved by the JEMCC method body.  As a rule, the compiled
 * class code is not expected to test for missing references - the JEMCC
 * classes are supposed to be at the lowest level where other classes that
 * are depended on are fully available (and exist as defined).  As a result,
 * the linkage of a JEMCC class throws an exception during class definition
 * if a link is missing, rather than upon first use of the reference as a
 * standard bytecode class does.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     linkClass - the class to be linked (receives the final linkage
 *                 tables).  This is also the class used if the extClassRef
 *                 data member in the linkInfo structure is NULL
 *     linkInfo - an array of external LinkData definitions
 *     linkCount - the number of link definitions in the linkInfo array
 *
 * Returns:
 *     JNI_OK - the linkage was completed successfully
 *     JNI_ERR - an error (missing field or method) occurred during the 
 *               linking of the class.  A LinkageError will have been thrown
 *               in the current environment
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory 
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     LinkageError - one of the class linkages failed (reference not found)
 */
JNIEXPORT jint JNICALL JEMCC_LinkClass(JNIEnv *env, JEMCC_Class *linkClass,
                                       JEMCC_LinkData *linkInfo,
                                       jsize linkCount);

/**
 * Method by which static field information may be initialized for
 * a class definition.  The provided value array contains typed initial
 * values for the static members of the class, in the same order as
 * the field definitions in the class construction list (ignoring/skipping
 * over non-static entries).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class containing the static fields to be initialized
 *     initData - an array of initialization values for the class, in the
 *                same order/types as the field declarations.  This array
 *                must have at least as many entries as there are static
 *                field entries in the class
 */
JNIEXPORT void JNICALL JEMCC_InitializeStaticClassFields(JNIEnv *env,
                                                  JEMCC_Class *classInst,
                                                  JEMCC_ReturnValue *initData);

/**
 * Register an instance of the given JEMCC class into the specified classloader
 * (or the VM bootstrap loader).  Due to the "recoverable" capabilities of the
 * package initialization process, it is not an error to re-register an already
 * registered JEMCC class instance.  If the given class instance already appears
 * in the classloader tables, this method will discard the "new" instance and
 * return the original (already existing) instance, to maintain consistency
 * with potential class instances already created.  Note that this method
 * is only to be used for single classes with no concurrent definition
 * dependencies (as a result of this potential "redefinition").  For situations
 * where multiple classes with cross-dependencies are to be registered at the
 * same time, use the RegisterClasses method below.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the registered class (or
 *              from which the replacement class is to come).  If NULL, the
 *              VM bootstrap classloader is used instead
 *     regClass - a reference to the class instance to be registered.  If this
 *                class already exists in the classloader table, the provided
 *                instance is destroyed and the existing instance is returned
 *                via this reference (for coding simplicity)
 *
 * Returns:
 *     JNI_OK - the definition of the class was successful (or the already
 *              existing instance has been returned successfully).
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory 
 *                  error has been thrown in the current environment (the
 *                  provided class instance has been destroyed)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_RegisterClass(JNIEnv *env, JEMCC_Object *loader,
                                           JEMCC_Class **regClass);

/**
 * Register a set of JEMCC classes into the specified classloader (or the VM
 * bootstrap loader).  This is identical to the RegisterClass method above,
 * except that it handles the definition within a single classloader lock
 * to avoid any errors due to already existing class definitions or race
 * conditions.  Due to the complexities associated with relinking partial
 * matches, this operation is completely atomic - if any class is duplicated
 * the entire set is discarded and replaced with existing instances.  If a 
 * memory error occurs, all of the provided classes are destroyed and removed
 * from the classloader hashtable if partially stored.
 *
 * Note that this method should only be used in exactly reproducible
 * situations for the minimal set of cross-linked class instances.  It cannot
 * overcome instances where the class list is partially defined - in such
 * a situation, the updated array will contain NULL values.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the registered classes (or
 *              from which the replacement(s) are to come).  If NULL, the
 *              VM bootstrap classloader is used instead
 *     regClasses - a reference to an array of the classes to be registered.
 *                  If any class already exists, all are destroyed and the
 *                  references to the already existing instances are placed
 *                  in the array
 *     regClassCount - the number of classes specified in the regClasses array 
 *
 * Returns:
 *     JNI_OK - the definition/replacement of the classes was successful
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory 
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_RegisterClasses(JNIEnv *env, JEMCC_Object *loader,
                                             JEMCC_Class **regClasses,
                                             jsize regClassCount);

/**
 * Convenience method to define a single class instance which has no cross
 * dependencies on other classes being defined at the same time. Essentially a 
 * wrapper around the three steps of building/linking/registering a JEMCC 
 * class instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the constructed class
 *              instance (class will be registered)
 *     accessFlags - the ACC_ flags which define the class permissions and
 *                   types (ACC_JEMCC is automatically set)
 *     className - the fully qualified name of the class being constructed
 *                 (period (.) package separators are used in this case)
 *     superClass - if non-NULL, the superclass of this class.  Note that
 *                  JEMCC classes should ideally have another JEMCC class as
 *                  the superclass and only core Object related classes
 *                  should have a NULL superclass
 *     interfaces - an array reference to a list of direct interfaces
 *                  implemented by this class
 *     interfaceCount - the number of interface instances in the interfaces
 *                      array
 *     methodInfo - an array of JEMCC method definitions for this class
 *     methodCount - the number of method records in the methodInfo array
 *     objMgmtFn - a reference to the object management method, NULL if
 *                 default management operations apply to this class
 *                 (no override function exists)
 *     fieldInfo - an array of JEMCC field definitions for this class
 *     fieldCount - the number of field records in the fieldInfo array
 *     linkInfo - an array of external LinkData definitions
 *     linkCount - the number of link definitions in the linkInfo array
 *     initData - an array of initialization values for the class, in the
 *                same order/types as the field declarations.  This array
 *                must have at least as many entries as there are static
 *                field entries in the class
 *     classInst - a class instance reference through which the
 *                 constructed class instance is returned (or the already
 *                 existing class instance if a conflict occurs)
 *
 * Returns:
 *     JNI_OK - the building/linking/definition of the JEMCC class was
 *              successful
 *     JNI_ERR - a definition error occurred during the creation of the class.
 *               An ClassFormatError/AbstractMethodError/LinkageError will 
 *               have been thrown in the current environment
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - provided JEMCC initialization data is invalid
 *     AbstractMethodError - this concrete class is missing a non-abstract
 *                           method definition
 *     LinkageError - one of the class linkages failed (reference not found)
 */
JNIEXPORT jint JNICALL JEMCC_CreateStdClass(JNIEnv *env, JEMCC_Object *loader,
                                            juint accessFlags,
                                            const char *className,
                                            JEMCC_Class *superClass,
                                            JEMCC_Class **interfaces,
                                            jsize interfaceCount,
                                            JEMCC_MethodData *methodInfo,
                                            jsize methodCount,
                                            JEMCC_ObjMgmtFn *objMgmtFn,
                                            JEMCC_FieldData *fieldInfo,
                                            jsize fieldCount,
                                            JEMCC_LinkData *linkInfo,
                                            jsize linkCount,
                                            JEMCC_ReturnValue *initData,
                                            JEMCC_Class **classInst);

/**
 * Convenience method to define a "standard" throwable instance.  These are
 * classes which originate from another "standard" throwable class, define no
 * additional fields or methods and only have the basic constuctors.
 * Internally, the VM saves space with these classes by copying the root 
 * definition tables from the Throwable class itself.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to contain the constructed class
 *              instance (class will be registered)
 *     className - the fully qualified name of the class being constructed
 *                 (period (.) package separators are used in this case)
 *     superClass - if superclass of this class.  Must be another "standard"
 *                  throwable class instance
 *     classInst - a class instance reference through which the
 *                     constructed class instance is returned
 *
 * Returns:
 *     JNI_OK - the building/linking/definition of the standard throwable class
 *              was successful
 *     JNI_ERR - the superclass is not a standard throwable, so the class
 *               instance could not be created.  A LinkageError has been
 *               thrown in the current environment
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     LinkageError - the superclass is not a standard throwable and linkage
 *                    could not occur
 */
JNIEXPORT jint JNICALL JEMCC_CreateStdThrowableClass(JNIEnv *env,
                                                     JEMCC_Object *loader,
                                                     const char *className,
                                                     JEMCC_Class *superClass,
                                                     JEMCC_Class **classInst);

/**
 * Method which launches class initialization, if required.  This method
 * will block if another thread is initializing the class, until the
 * class initialization is complete.  Exposed to JEMCC class instances to
 * allow forced initialization of static data.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class to be initialized
 *
 * Returns:
 *     JNI_OK - the class initialization was successful
 *     JNI_ERR - an error occurred while initializing the class.  An exception
 *               has been thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory error occurred while handling an init
 *                        exception
 *     ExceptionInInitializerError - an exception occurred during the
 *                                   operation of the initialization method
 *     NoClassDefFoundError - the class was previously initialized and
 *                            an error had occurred at that time
 */
JNIEXPORT jint JNICALL JEMCC_InitializeClass(JNIEnv *env, 
                                             JEMCC_Class *classInst);

/*********** JEMCC Internal/External Reference Managers **************/

/**
 * Defining the following will insert code into each method or field access
 * call to validate the name and descriptor of the target.  Definitely
 * should be used during development to ensure proper linkage indexing.
 * Should definitely be disabled for a production build.
 */
#ifdef ENABLE_ERRORSWEEP
#define ENABLE_JEMCC_CROSS_CHECKING 1
#endif

/**
 * Convenience method to obtain the class associated with the currently
 * executing stack frame.  Essentially allows a JEMCC method to obtain
 * a handle on the class for which it is defined from within the method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The class instance associated with the current frame (the class
 *     of the 'this' object for an instance method or the parent class
 *     of a static method).
 */
JNIEXPORT JEMCC_Class *JNICALL JEMCC_GetCurrentClass(JNIEnv *env);

/**
 * NOTE: in all of the following get/push methods, there are two types
 *       of access that are available:
 *
 *         - indexed access to a method/field which is defined as part of the 
 *           specified class (DIRECT access); may include the current class
 *           if a target class is unspecified
 *         - indexed access to a method/field which has been specified in
 *           the LinkClass() method (LINKED access); in this case, the index
 *           refers to a counter which starts from zero and skips over
 *           reference links of other types (i.e. the third method reference,
 *           ignoring class and field link references is index 2)
 */

/**
 * Method to obtain a linked class reference, as defined through the
 * LinkClass() method.  In this case, the index refers to linked class 
 * references only (the GetCurrentClass method will retrieve the local
 * class instance).
 *
 * Parameters:
 *     env - the VM environment which is currently in context 
 *     classIdx - the index of the linked class reference to retrieve
 *
 * Returns:
 *     The requested class reference associated with the provided index.
 *     Note that this will always succeed, as a JEMCC class definition should 
 *     fail if any external reference links are invalid.
 */
JNIEXPORT JEMCC_Class *JNICALL JEMCC_GetLinkedClassReference(JNIEnv *env, 
                                                             jint classIdx);

/**
 * Method to obtain a field reference, from the local method list of the
 * indicated class (NULL indicates current class local fields).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     targClass - the target class to retrieve the field from - if NULL, use
 *                 the class associated with the currently executing method
 *     fieldIdx - the index of the local field reference to retrieve
 *
 * Returns:
 *     The requested local field reference associated with the provided
 *     class.  Note that this will always succeed, even if the index is 
 *     invalid (no verification is performed by this method, use the CHECK
 *     macros to test during development).
 */
JNIEXPORT jfieldID JNICALL JEMCC_GetFieldReference(JNIEnv *env, 
                                                   JEMCC_Class *targClass,
                                                   jint fieldIdx);

/**
 * Method to obtain a linked field reference which was defined in the
 * current class through the LinkClass() method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     fieldIdx - the index of the linked field reference to retrieve
 *
 * Returns:
 *     The requested external field reference associated with the provided
 *     index.  Note that this will always succeed, even if the index is 
 *     invalid (no verification is performed by this method, use the CHECK
 *     macros to test during development).
 */
JNIEXPORT jfieldID JNICALL JEMCC_GetLinkedFieldReference(JNIEnv *env, 
                                                         jint fieldIdx);

/**
 * Method to obtain the data pointer for a field, from the local method list 
 * of the indicated class (NULL indicates current class local fields).
 *
 * NOTE: this method, while more efficient than using the fieldId and the JNI
 * methods, is inherently more dangerous due to the direct manipulation of the 
 * object data area.  Pay careful attention to proper casting of the returned
 * pointer.  Also note that no type checking is performed on the validity
 * of the field reference and the class of the given target object.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     targObj - the target object to retrieve the data pointer from
 *               (only applies to non-static fields)
 *     targClass - the target class to retrieve the field offset from - if 
 *                 NULL, use the class associated with the currently 
 *                 executing method
 *     fieldIdx - the index of the field reference to retrieve
 *
 * Returns:
 *     The pointer to the field data slot corresponding to the given index or
 *     NULL if the field is inaccessible (null target object or static/instance
 *     conflict - an exception has been thrown in the current environment).
 *
 * Exceptions:
 *     NullPointerException - the given object instance was null (and the field
 *                            is not static)
 *     IncompatibleClassChangeError - an object instance was provided for a
 *                                    static field reference
 */
JNIEXPORT void *JNICALL JEMCC_GetFieldDataPtr(JNIEnv *env, 
                                              JEMCC_Object *targObj,
                                              JEMCC_Class *targClass,
                                              jint fieldIdx);

/**
 * Method to obtain the data pointer for a linked field which was defined in
 * the current class through the LinkClass() method.
 *
 * NOTE: this method, while more efficient than using the fieldId and the JNI
 * methods, is inherently more dangerous due to the direct manipulation of the 
 * object data area.  Pay careful attention to proper casting of the returned
 * pointer.  Also note that no type checking is performed on the validity
 * of the field reference and the class of the given target object.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     targObj - the target object to retrieve the data pointer from
 *               (only applies to non-static fields)
 *     fieldIdx - the index of the linked field reference to retrieve
 *
 * Returns:
 *     The pointer to the field data slot corresponding to the given index or
 *     NULL if the field is inaccessible (null target object or static/instance
 *     conflict - an exception has been thrown in the current environment).
 *
 * Exceptions:
 *     NullPointerException - the given object instance was null (and the field
 *                            is not static)
 *     IncompatibleClassChangeError - an object instance was provided for a
 *                                    static field reference
 */
JNIEXPORT void *JNICALL JEMCC_GetLinkedFieldDataPtr(JNIEnv *env, 
                                                    JEMCC_Object *targObj,
                                                    jint fieldIdx);

/**
 * Macro/check function for validating the expected calls for the 
 * above references.
 */
JNIEXPORT void JNICALL JEMCC_CheckFieldReference(JNIEnv *env, 
                                                 JEMCC_Class *targClass,
                                                 jint fieldIdx,
                                                 const char *fieldName,
                                                 const char *fieldDesc);
JNIEXPORT void JNICALL JEMCC_CheckLinkedFieldReference(JNIEnv *env, 
                                                       jint fieldIdx,
                                                       const char *className,
                                                       const char *fieldName,
                                                       const char *fieldDesc);
#ifdef ENABLE_JEMCC_CROSS_CHECKING
#define JEMCC_CHECK_FIELD_REFERENCE(env, targClass, fieldIdx, \
                                    fieldName, fieldDesc) \
    JEMCC_CheckFieldReference(env, targClass, fieldIdx, fieldName, fieldDesc);
#define JEMCC_CHECK_LINKED_FIELD_REFERENCE(env, fieldIdx, \
                                           className, fieldName, fieldDesc) \
    JEMCC_CheckLinkedFieldReference(env, fieldIdx, \
                                    className, fieldName, fieldDesc);
#else
#define JEMCC_CHECK_FIELD_REFERENCE(env, targClass, fieldIdx, \
                                    fieldName, fieldDesc)
#define JEMCC_CHECK_LINKED_FIELD_REFERENCE(env, fieldIdx, \
                                           className, fieldName, fieldDesc)
#endif

/**
 * Obtain the active execution frame for the environment currently
 * in context.  Used for general methods (e.g. classloading) where
 * source methods do not pass execution frame information.
 * 
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The VM frame currently being executed.
 */
JNIEXPORT JEMCC_VMFrame *JNICALL JEMCC_GetCurrentVMFrame(JNIEnv *env);

/**
 * Method used to create and execute a new frame for an interface method
 * reference call.  Automatically extracts the appropriate method reference
 * by index, from the local class method table of the indicated interface.
 * NOTE: the argument stack must  be fully "constructed" (including object 
 * instance) prior to calling this method and no limit or interface type 
 * checking is performed (null object will be caught).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     targIf - the target interface to obtain the method from
 *     methodIdx - the index of the local interface method to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the 
 *               invocation of the method (exception has been thrown in the 
 *               current environment, will have been caught in the current 
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, bad interface) (exception
 *                  has been thrown in the current environment, will not 
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     IncompatibleClassChangeError - the interface was unmappable to the
 *                                    provided object instance
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteInterfaceMethod(JNIEnv *env,
                                                    JEMCC_Object *obj, 
                                                    JEMCC_Class *targIf,
                                                    jint methodIdx,
                                                    JEMCC_ReturnValue *retVal);

/**
 * Method used to create and execute a new frame for a linked interface method 
 * that was defined through the LinkClass() method. NOTE: the argument stack
 * must be fully "constructed" (including object instance) prior to calling
 * this method and no limit or interface type checking is performed (null 
 * object will be caught).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     methodIdx - the index of the linked interface method reference to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, bad interface) (exception
 *                  has been thrown in the current environment, will not
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     IncompatibleClassChangeError - the interface was unmappable to the
 *                                    provided object instance
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteLinkedInterfaceMethod(JNIEnv *env,
                                                    JEMCC_Object *obj,
                                                    jint methodIdx,
                                                    JEMCC_ReturnValue *retVal);

/* Definitions for code readability */
#define JEMCC_NON_VIRTUAL_METHOD JNI_FALSE
#define JEMCC_VIRTUAL_METHOD JNI_TRUE

/**
 * Method used to create and execute a new frame for an instance method
 * reference call.  This may be a virtual method call which handles overrides
 * of the referenced class method or a non-virtual call which exactly executes
 * the referenced method.   Automatically extracts the appropriate method
 * reference by index, from the local method table of the indicated class
 * (NULL indicates current class instance). NOTE: the argument stack must be 
 * fully "constructed" (including object instance) prior to calling this method
 * and no setup checking is performed (null object will be caught).
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     targClass - the target class to obtain the method from - if NULL, use
 *                 the class associated with the currently executing method
 *     methodIdx - the index of the local class method to use
 *     isVirtual - if JEMCC_VIRTUAL_METHOD (JNI_TRUE), locate the virtual
 *                 class method (accounts for overridden methods in the class
 *                 of the object instance), otherwise, prepare to execute
 *                 the exact method referenced
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, abstract method) (exception
 *                  has been thrown in the current environment, will not
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteInstanceMethod(JNIEnv *env,
                                                   JEMCC_Object *obj,
                                                   JEMCC_Class *targClass,
                                                   jint methodIdx, 
                                                   jboolean isVirtual,
                                                   JEMCC_ReturnValue *retVal);

/**
 * Method used to create and execute a new frame for a linked method that was
 * defined through the LinkClass() method.  This may be a virtual method call 
 * which handles overrides of the reference class method or a non-virtual 
 * call which exactly executes the referenced method.  NOTE: the argument 
 * stack must be fully "constructed" (including object instance) prior to 
 * calling this method and no setup checking is  performed (null object 
 * will be caught).
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     obj - the object instance to execute the method against
 *     methodIdx - the index of the linked method reference to use
 *     isVirtual - if JEMCC_VIRTUAL_METHOD (JNI_TRUE), locate the virtual
 *                 class method (accounts for overridden methods in the class
 *                 of the object instance), otherwise, prepare to execute
 *                 the exact method referenced
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (null object, abstract method) (exception
 *                  has been thrown in the current environment, will not
 *                  have been caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     NullPointerException - the given object instance was null
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteLinkedInstanceMethod(JNIEnv *env,
                                                    JEMCC_Object *obj,
                                                    jint methodIdx, 
                                                    jboolean isVirtual,
                                                    JEMCC_ReturnValue *retVal);

/**
 * Method used to create and execute a new frame for a superclass method call,
 * similar to calling the ExecuteInstance method with a local, non-virtual
 * index corresponding to the currently executing class method.  NOTE: the
 * argument stack must be fully "constructed" (including object instance) prior
 * to calling this method and no setup or null checking is performed.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     superClass - the superclass instance to extract the method from
 *                  (may be from a lower level in the inheritance tree).  If
 *                  NULL, the immediate superclass of the current class is
 *                  used.
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *     JNI_EINVAL - a local exception occurred due to invalid method
 *                  information (abstract method) (exception has been 
 *                  thrown in the current environment, will not have been 
 *                  caught by the current frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     AbstractMethodError - the interface method is not implemented or
 *                           is abstract in the object class
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteSuperClassMethod(JNIEnv *env,
                                                     JEMCC_Class *superClass,
                                                     JEMCC_ReturnValue *retVal);

/**
 * Method used to create and execute a new frame for a superclass constructor
 * call, similar to calling the ExecuteInstance method with the immediate 
 * superclass instance and a local, non-virtual index corresponding to the 
 * desired constructor to be called.  NOTE: the argument stack must be fully 
 * "constructed" (including new object instance) prior to calling this 
 * method and no setup or null checking is performed.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     constIdx - the local index of the target constructor in the immediate
 *                superclass
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteSuperClassConstructor(JNIEnv *env,
                                                          jint constIdx);

/**
 * Method used to create and execute a new frame for a static method
 * reference call.   Automatically extracts the appropriate method reference
 * by index, from the local method table of the indicated class (NULL 
 * indicates current class instance).  NOTE: the argument stack must be
 * fully "constructed" (no this!) prior to calling this method and no setup 
 * checking is performed.
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     targClass - the target class to obtain the method from - if NULL, use
 *                 the class associated with the currently executing method
 *     methodIdx - the index of the local class method to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteStaticMethod(JNIEnv *env,
                                                 JEMCC_Class *targClass,
                                                 jint methodIdx,
                                                 JEMCC_ReturnValue *retVal);

/**
 * Method used to create and execute a new frame for a linked static method
 * that was defined through the LinkClass() method.  NOTE: the argument stack 
 * must be fully "constructed" (no this!) prior to calling this method and 
 * no setup checking is performed.
 *
 * Parameters
 *     env - the VM environment which is currently in context
 *     methodIdx - the index of the linked method reference to use
 *     retVal - if non-NULL, the value returned from the method call will
 *              be returned via this reference.  Value is indeterminate
 *              if an exception has occurred.
 *
 * Returns:
 *     JNI_OK - the frame creation was successful and the method executed
 *              without errors (retVal has been updated, if applicable)
 *     JNI_ENOMEM - a memory allocation failure occurred (exception has
 *                  been thrown in the current environment, will not have
 *                  been caught by the current frame)
 *     JNI_ERR - a non-memory related exception has occurred in the
 *               invocation of the method (exception has been thrown in the
 *               current environment, will have been caught in the current
 *               frame)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed in creating the frame
 *     Other exceptions as thrown by the method in question.
 */
JNIEXPORT jint JNICALL JEMCC_ExecuteLinkedStaticMethod(JNIEnv *env,
                                                    jint methodIdx,
                                                    JEMCC_ReturnValue *retVal);

/**
 * Macro/check function for validating the expected calls for the 
 * above references.
 */
JNIEXPORT void JNICALL JEMCC_CheckMethodReference(JNIEnv *env, 
                                                  JEMCC_Class *targClass,
                                                  jint methodIdx,
                                                  const char *methodName,
                                                  const char *methodDesc);
JNIEXPORT void JNICALL JEMCC_CheckLinkedMethodReference(JNIEnv *env, 
                                                        jint methodIdx,
                                                        const char *className,
                                                        const char *methodName,
                                                        const char *methodDesc);
#ifdef ENABLE_JEMCC_CROSS_CHECKING
#define JEMCC_CHECK_METHOD_REFERENCE(env, targClass, methodIdx, \
                                     methodName, methodDesc) \
    JEMCC_CheckMethodReference(env, targClass, methodIdx, \
                               methodName, methodDesc);
#define JEMCC_CHECK_LINKED_METHOD_REFERENCE(env, methodIdx, \
                                            className, methodName, methodDesc) \
    JEMCC_CheckLinkedMethodReference(env, methodIdx, \
                                     className, methodName, methodDesc);
#else
#define JEMCC_CHECK_METHOD_REFERENCE(env, targClass, methodIdx, \
                                     methodName, methodDesc)
#define JEMCC_CHECK_LINKED_METHOD_REFERENCE(env, methodIdx, \
                                            className, methodName, methodDesc)
#endif

/**
 * Test condition to determine if the indicated local method of the indicated
 * class (NULL indicates current class instance) has been "overridden" in the 
 * target object's class.  Used in JEMCC native code to identify situations 
 * where the default implementation is in effect and a frame execution is 
 * not required.
 *
 * NOTE: this does not in any way validate that the target object class is
 *       assignable from the specified class
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the target object to test for default method implementation
 *     targClass - the target class to check the method from - if NULL, use
 *                 the class associated with the currently executing method
 *     methodIdx - the index of the local class method to test against
 *
 * Returns:
 *     JNI_TRUE if the target and method references are identical (default
 *     implementation applies), JNI_FALSE otherwise.
 */
JNIEXPORT jboolean JNICALL JEMCC_IsDefaultMethod(JNIEnv *env, JEMCC_Object *obj,
                                                 JEMCC_Class *targClass,
                                                 jint methodIdx);

/* <jemcc_end> */

#endif
