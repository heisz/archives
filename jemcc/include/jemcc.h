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


/************************ Direct JNI Functions ***********************/
/* TODO - Move comment blocks from code implementations to this file */

JNIEXPORT jint JNICALL JEMCC_GetVersion(JNIEnv *env);
JNIEXPORT jclass JNICALL JEMCC_DefineClass(JNIEnv *env, jobject loader,
                                           const jbyte *buff, jsize buffLen);
JNIEXPORT jclass JNICALL JEMCC_FindClass(JNIEnv *env, const char *name);

JNIEXPORT jclass JNICALL JEMCC_GetSuperclass(JNIEnv *env, jclass clazz);
JNIEXPORT jboolean JNICALL JEMCC_IsAssignableFrom(JNIEnv *env,
                                                  jclass clazza, jclass clazzb);

JNIEXPORT jint JNICALL JEMCC_Throw(JNIEnv *env, jthrowable obj);
JNIEXPORT jint JNICALL JEMCC_ThrowNew(JNIEnv *env, jclass clazz,
                                      const char *msg);
JNIEXPORT jthrowable JNICALL JEMCC_ExceptionOccurred(JNIEnv *env);
JNIEXPORT void JNICALL JEMCC_ExceptionDescribe(JNIEnv *env);
JNIEXPORT void JNICALL JEMCC_ExceptionClear(JNIEnv *env);
JNIEXPORT void JNICALL JEMCC_FatalError(JNIEnv *env, const char *msg);

JNIEXPORT jobject JNICALL JEMCC_NewGlobalRef(JNIEnv *env, jobject obj);
JNIEXPORT void JNICALL JEMCC_DeleteGlobalRef(JNIEnv *env, jobject globalRef);
JNIEXPORT void JNICALL JEMCC_DeleteLocalRef(JNIEnv *env, jobject localRef);
JNIEXPORT jboolean JNICALL JEMCC_IsSameObject(JNIEnv *env,
                                              jobject obja, jobject objb);

JNIEXPORT jobject JNICALL JEMCC_AllocObject(JNIEnv *env, jclass clazz);
JNIEXPORT jobject JNICALL JEMCC_NewObject(JNIEnv *env, jclass class,
                                          jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_NewObjectV(JNIEnv *env, jclass class,
                                           jmethodID methodID, va_list args);
JNIEXPORT jobject JNICALL JEMCC_NewObjectA(JNIEnv *env, jclass class,
                                           jmethodID methodID, jvalue *args);

JNIEXPORT jclass JNICALL JEMCC_GetObjectClass(JNIEnv *env, jobject obj);
JNIEXPORT jboolean JNICALL JEMCC_IsInstanceOf(JNIEnv *env, jobject obj,
                                              jclass clazz);

JNIEXPORT jmethodID JNICALL JEMCC_GetMethodID(JNIEnv *env, jclass clazz,
                                              const char *name,
                                              const char *sig);

JNIEXPORT jobject JNICALL JEMCC_CallObjectMethod(JNIEnv *env, jobject obj,
                                                 jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_CallObjectMethodV(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  va_list args);
JNIEXPORT jobject JNICALL JEMCC_CallObjectMethodA(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  jvalue *args);

JNIEXPORT jboolean JNICALL JEMCC_CallBooleanMethod(JNIEnv *env, jobject obj,
                                                   jmethodID methodID, ...);
JNIEXPORT jboolean JNICALL JEMCC_CallBooleanMethodV(JNIEnv *env, jobject obj,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jboolean JNICALL JEMCC_CallBooleanMethodA(JNIEnv *env, jobject obj,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jbyte JNICALL JEMCC_CallByteMethod(JNIEnv *env, jobject obj,
                                             jmethodID methodID, ...);
JNIEXPORT jbyte JNICALL JEMCC_CallByteMethodV(JNIEnv *env, jobject obj,
                                              jmethodID methodID, va_list args);
JNIEXPORT jbyte JNICALL JEMCC_CallByteMethodA(JNIEnv *env, jobject obj,
                                              jmethodID methodID, jvalue *args);

JNIEXPORT jchar JNICALL JEMCC_CallCharMethod(JNIEnv *env, jobject obj,
                                             jmethodID methodID, ...);
JNIEXPORT jchar JNICALL JEMCC_CallCharMethodV(JNIEnv *env, jobject obj,
                                              jmethodID methodID, va_list args);
JNIEXPORT jchar JNICALL JEMCC_CallCharMethodA(JNIEnv *env, jobject obj,
                                              jmethodID methodID, jvalue *args);

JNIEXPORT jshort JNICALL JEMCC_CallShortMethod(JNIEnv *env, jobject obj,
                                               jmethodID methodID, ...);
JNIEXPORT jshort JNICALL JEMCC_CallShortMethodV(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jshort JNICALL JEMCC_CallShortMethodA(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jint JNICALL JEMCC_CallIntMethod(JNIEnv *env, jobject obj,
                                           jmethodID methodID, ...);
JNIEXPORT jint JNICALL JEMCC_CallIntMethodV(JNIEnv *env, jobject obj,
                                            jmethodID methodID, va_list args);
JNIEXPORT jint JNICALL JEMCC_CallIntMethodA(JNIEnv *env, jobject obj,
                                            jmethodID methodID, jvalue *args);

JNIEXPORT jlong JNICALL JEMCC_CallLongMethod(JNIEnv *env, jobject obj,
                                             jmethodID methodID, ...);
JNIEXPORT jlong JNICALL JEMCC_CallLongMethodV(JNIEnv *env, jobject obj,
                                              jmethodID methodID, va_list args);
JNIEXPORT jlong JNICALL JEMCC_CallLongMethodA(JNIEnv *env, jobject obj,
                                              jmethodID methodID, jvalue *args);

JNIEXPORT jfloat JNICALL JEMCC_CallFloatMethod(JNIEnv *env, jobject obj,
                                               jmethodID methodID, ...);
JNIEXPORT jfloat JNICALL JEMCC_CallFloatMethodV(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jfloat JNICALL JEMCC_CallFloatMethodA(JNIEnv *env, jobject obj,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jdouble JNICALL JEMCC_CallDoubleMethod(JNIEnv *env, jobject obj,
                                                 jmethodID methodID, ...);
JNIEXPORT jdouble JNICALL JEMCC_CallDoubleMethodV(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  va_list args);
JNIEXPORT jdouble JNICALL JEMCC_CallDoubleMethodA(JNIEnv *env, jobject obj,
                                                  jmethodID methodID,
                                                  jvalue *args);

JNIEXPORT void JNICALL JEMCC_CallVoidMethod(JNIEnv *env, jobject obj,
                                            jmethodID methodID, ...);
JNIEXPORT void JNICALL JEMCC_CallVoidMethodV(JNIEnv *env, jobject obj,
                                             jmethodID methodID, va_list args);
JNIEXPORT void JNICALL JEMCC_CallVoidMethodA(JNIEnv *env, jobject obj,
                                             jmethodID methodID, jvalue *args);


JNIEXPORT jobject JNICALL JEMCC_CallNonvirtualObjectMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_CallNonvirtualObjectMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jobject JNICALL JEMCC_CallNonvirtualObjectMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jboolean JNICALL JEMCC_CallNonvirtualBooleanMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jboolean JNICALL JEMCC_CallNonvirtualBooleanMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jboolean JNICALL JEMCC_CallNonvirtualBooleanMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jbyte JNICALL JEMCC_CallNonvirtualByteMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jbyte JNICALL JEMCC_CallNonvirtualByteMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jbyte JNICALL JEMCC_CallNonvirtualByteMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jchar JNICALL JEMCC_CallNonvirtualCharMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jchar JNICALL JEMCC_CallNonvirtualCharMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jchar JNICALL JEMCC_CallNonvirtualCharMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jshort JNICALL JEMCC_CallNonvirtualShortMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jshort JNICALL JEMCC_CallNonvirtualShortMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jshort JNICALL JEMCC_CallNonvirtualShortMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jint JNICALL JEMCC_CallNonvirtualIntMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jint JNICALL JEMCC_CallNonvirtualIntMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jint JNICALL JEMCC_CallNonvirtualIntMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jlong JNICALL JEMCC_CallNonvirtualLongMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jlong JNICALL JEMCC_CallNonvirtualLongMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jlong JNICALL JEMCC_CallNonvirtualLongMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jfloat JNICALL JEMCC_CallNonvirtualFloatMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jfloat JNICALL JEMCC_CallNonvirtualFloatMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jfloat JNICALL JEMCC_CallNonvirtualFloatMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT jdouble JNICALL JEMCC_CallNonvirtualDoubleMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT jdouble JNICALL JEMCC_CallNonvirtualDoubleMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT jdouble JNICALL JEMCC_CallNonvirtualDoubleMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);

JNIEXPORT void JNICALL JEMCC_CallNonvirtualVoidMethod(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID, ...);
JNIEXPORT void JNICALL JEMCC_CallNonvirtualVoidMethodV(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                va_list args);
JNIEXPORT void JNICALL JEMCC_CallNonvirtualVoidMethodA(JNIEnv *env,
                                                jobject obj, jclass clazz,
                                                jmethodID methodID,
                                                jvalue *args);


JNIEXPORT jfieldID JNICALL JEMCC_GetFieldID(JNIEnv *env, jclass clazz,
                                            const char *name, const char *sig);

JNIEXPORT jobject JNICALL JEMCC_GetObjectField(JNIEnv *env, jobject obj,
                                               jfieldID fieldID);
JNIEXPORT jboolean JNICALL JEMCC_GetBooleanField(JNIEnv *env, jobject obj,
                                                 jfieldID fieldID);
JNIEXPORT jbyte JNICALL JEMCC_GetByteField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID);
JNIEXPORT jchar JNICALL JEMCC_GetCharField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID);
JNIEXPORT jshort JNICALL JEMCC_GetShortField(JNIEnv *env, jobject obj,
                                             jfieldID fieldID);
JNIEXPORT jint JNICALL JEMCC_GetIntField(JNIEnv *env, jobject obj,
                                         jfieldID fieldID);
JNIEXPORT jlong JNICALL JEMCC_GetLongField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID);
JNIEXPORT jfloat JNICALL JEMCC_GetFloatField(JNIEnv *env, jobject obj,
                                             jfieldID fieldID);
JNIEXPORT jdouble JNICALL JEMCC_GetDoubleField(JNIEnv *env, jobject obj,
                                               jfieldID fieldID);

JNIEXPORT void JNICALL JEMCC_SetObjectField(JNIEnv *env, jobject obj,
                                            jfieldID fieldID, jobject val);
JNIEXPORT void JNICALL JEMCC_SetBooleanField(JNIEnv *env, jobject obj,
                                             jfieldID fieldID, jboolean val);
JNIEXPORT void JNICALL JEMCC_SetByteField(JNIEnv *env, jobject obj,
                                          jfieldID fieldID, jbyte val);
JNIEXPORT void JNICALL JEMCC_SetCharField(JNIEnv *env, jobject obj,
                                          jfieldID fieldID, jchar val);
JNIEXPORT void JNICALL JEMCC_SetShortField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID, jshort val);
JNIEXPORT void JNICALL JEMCC_SetIntField(JNIEnv *env, jobject obj,
                                         jfieldID fieldID, jint val);
JNIEXPORT void JNICALL JEMCC_SetLongField(JNIEnv *env, jobject obj,
                                          jfieldID fieldID, jlong val);
JNIEXPORT void JNICALL JEMCC_SetFloatField(JNIEnv *env, jobject obj,
                                           jfieldID fieldID, jfloat val);
JNIEXPORT void JNICALL JEMCC_SetDoubleField(JNIEnv *env, jobject obj,
                                            jfieldID fieldID, jdouble val);


JNIEXPORT jmethodID JNICALL JEMCC_GetStaticMethodID(JNIEnv *env, jclass clazz,
                                                    const char *name,
                                                    const char *sig);

JNIEXPORT jobject JNICALL JEMCC_CallStaticObjectMethod(JNIEnv *env,
                                                       jclass clazz,
                                                       jmethodID methodID, ...);
JNIEXPORT jobject JNICALL JEMCC_CallStaticObjectMethodV(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        va_list args);
JNIEXPORT jobject JNICALL JEMCC_CallStaticObjectMethodA(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        jvalue *args);

JNIEXPORT jboolean JNICALL JEMCC_CallStaticBooleanMethod(JNIEnv *env,
                                                         jclass clazz,
                                                         jmethodID methodID,
                                                         ...);
JNIEXPORT jboolean JNICALL JEMCC_CallStaticBooleanMethodV(JNIEnv *env,
                                                          jclass clazz,
                                                          jmethodID methodID,
                                                          va_list args);
JNIEXPORT jboolean JNICALL JEMCC_CallStaticBooleanMethodA(JNIEnv *env,
                                                          jclass clazz,
                                                          jmethodID methodID,
                                                          jvalue *args);

JNIEXPORT jbyte JNICALL JEMCC_CallStaticByteMethod(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID, ...);
JNIEXPORT jbyte JNICALL JEMCC_CallStaticByteMethodV(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jbyte JNICALL JEMCC_CallStaticByteMethodA(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jchar JNICALL JEMCC_CallStaticCharMethod(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID, ...);
JNIEXPORT jchar JNICALL JEMCC_CallStaticCharMethodV(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jchar JNICALL JEMCC_CallStaticCharMethodA(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jshort JNICALL JEMCC_CallStaticShortMethod(JNIEnv *env, jclass clazz,
                                                     jmethodID methodID, ...);
JNIEXPORT jshort JNICALL JEMCC_CallStaticShortMethodV(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      va_list args);
JNIEXPORT jshort JNICALL JEMCC_CallStaticShortMethodA(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      jvalue *args);

JNIEXPORT jint JNICALL JEMCC_CallStaticIntMethod(JNIEnv *env, jclass clazz,
                                                 jmethodID methodID, ...);
JNIEXPORT jint JNICALL JEMCC_CallStaticIntMethodV(JNIEnv *env, jclass clazz,
                                                  jmethodID methodID,
                                                  va_list args);
JNIEXPORT jint JNICALL JEMCC_CallStaticIntMethodA(JNIEnv *env, jclass clazz,
                                                  jmethodID methodID,
                                                  jvalue *args);

JNIEXPORT jlong JNICALL JEMCC_CallStaticLongMethod(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID, ...);
JNIEXPORT jlong JNICALL JEMCC_CallStaticLongMethodV(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    va_list args);
JNIEXPORT jlong JNICALL JEMCC_CallStaticLongMethodA(JNIEnv *env, jclass clazz,
                                                    jmethodID methodID,
                                                    jvalue *args);

JNIEXPORT jfloat JNICALL JEMCC_CallStaticFloatMethod(JNIEnv *env, jclass clazz,
                                                     jmethodID methodID, ...);
JNIEXPORT jfloat JNICALL JEMCC_CallStaticFloatMethodV(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      va_list args);
JNIEXPORT jfloat JNICALL JEMCC_CallStaticFloatMethodA(JNIEnv *env, jclass clazz,
                                                      jmethodID methodID,
                                                      jvalue *args);

JNIEXPORT jdouble JNICALL JEMCC_CallStaticDoubleMethod(JNIEnv *env,
                                                       jclass clazz,
                                                       jmethodID methodID, ...);
JNIEXPORT jdouble JNICALL JEMCC_CallStaticDoubleMethodV(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        va_list args);
JNIEXPORT jdouble JNICALL JEMCC_CallStaticDoubleMethodA(JNIEnv *env,
                                                        jclass clazz,
                                                        jmethodID methodID,
                                                        jvalue *args);

JNIEXPORT void JNICALL JEMCC_CallStaticVoidMethod(JNIEnv *env, jclass clazz,
                                                  jmethodID methodID, ...);
JNIEXPORT void JNICALL JEMCC_CallStaticVoidMethodV(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID,
                                                   va_list args);
JNIEXPORT void JNICALL JEMCC_CallStaticVoidMethodA(JNIEnv *env, jclass clazz,
                                                   jmethodID methodID,
                                                   jvalue *args);

JNIEXPORT jfieldID JNICALL JEMCC_GetStaticFieldID(JNIEnv *env, jclass clazz,
                                                  const char *name,
                                                  const char *sig);

JNIEXPORT jobject JNICALL JEMCC_GetStaticObjectField(JNIEnv *env, jclass clazz,
                                                     jfieldID fieldID);
JNIEXPORT jboolean JNICALL JEMCC_GetStaticBooleanField(JNIEnv *env,
                                                       jclass clazz,
                                                       jfieldID fieldID);
JNIEXPORT jbyte JNICALL JEMCC_GetStaticByteField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID);
JNIEXPORT jchar JNICALL JEMCC_GetStaticCharField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID);
JNIEXPORT jshort JNICALL JEMCC_GetStaticShortField(JNIEnv *env, jclass clazz,
                                                   jfieldID fieldID);
JNIEXPORT jint JNICALL JEMCC_GetStaticIntField(JNIEnv *env, jclass clazz,
                                               jfieldID fieldID);
JNIEXPORT jlong JNICALL JEMCC_GetStaticLongField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID);
JNIEXPORT jfloat JNICALL JEMCC_GetStaticFloatField(JNIEnv *env, jclass clazz,
                                                   jfieldID fieldID);
JNIEXPORT jdouble JNICALL JEMCC_GetStaticDoubleField(JNIEnv *env, jclass clazz,
                                                     jfieldID fieldID);

JNIEXPORT void JNICALL JEMCC_SetStaticObjectField(JNIEnv *env, jclass clazz,
                                                  jfieldID fieldID,
                                                  jobject val);
JNIEXPORT void JNICALL JEMCC_SetStaticBooleanField(JNIEnv *env, jclass clazz,
                                                   jfieldID fieldID,
                                                   jboolean val);
JNIEXPORT void JNICALL JEMCC_SetStaticByteField(JNIEnv *env, jclass clazz,
                                                jfieldID fieldID, jbyte val);
JNIEXPORT void JNICALL JEMCC_SetStaticCharField(JNIEnv *env, jclass clazz,
                                                jfieldID fieldID, jchar val);
JNIEXPORT void JNICALL JEMCC_SetStaticShortField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID, jshort val);
JNIEXPORT void JNICALL JEMCC_SetStaticIntField(JNIEnv *env, jclass clazz,
                                               jfieldID fieldID, jint val);
JNIEXPORT void JNICALL JEMCC_SetStaticLongField(JNIEnv *env, jclass clazz,
                                                jfieldID fieldID, jlong val);
JNIEXPORT void JNICALL JEMCC_SetStaticFloatField(JNIEnv *env, jclass clazz,
                                                 jfieldID fieldID, jfloat val);
JNIEXPORT void JNICALL JEMCC_SetStaticDoubleField(JNIEnv *env, jclass clazz,
                                                  jfieldID fieldID,
                                                  jdouble val);

JNIEXPORT jstring JNICALL JEMCC_NewString(JNIEnv *env, const jchar *unicode,
                                          jsize len);
JNIEXPORT jsize JNICALL JEMCC_GetStringLength(JNIEnv *env, jstring str);
JNIEXPORT const jchar *JNICALL JEMCC_GetStringChars(JNIEnv *env, jstring str,
                                                    jboolean *isCopy);
JNIEXPORT void JNICALL JEMCC_ReleaseStringChars(JNIEnv *env, jstring str,
                                                const jchar *chars);

JNIEXPORT jstring JNICALL JEMCC_NewStringUTF(JNIEnv *env, const char *utf);
JNIEXPORT jsize JNICALL JEMCC_GetStringUTFLength(JNIEnv *env, jstring str);
JNIEXPORT const char *JNICALL JEMCC_GetStringUTFChars(JNIEnv *env, jstring str,
                                                      jboolean *isCopy);
JNIEXPORT void JNICALL JEMCC_ReleaseStringUTFChars(JNIEnv *env, jstring str,
                                                   const char *chars);

JNIEXPORT jsize JNICALL JEMCC_GetArrayLength(JNIEnv *env, jarray array);
JNIEXPORT jobjectArray JNICALL JEMCC_NewObjectArray(JNIEnv *env, jsize len,
                                                    jclass clazz, jobject init);
JNIEXPORT jobject JNICALL JEMCC_GetObjectArrayElement(JNIEnv *env,
                                                      jobjectArray array,
                                                      jsize index);
JNIEXPORT void JNICALL JEMCC_SetObjectArrayElement(JNIEnv *env,
                                                   jobjectArray array,
                                                   jsize index, jobject val);

JNIEXPORT jbooleanArray JNICALL JEMCC_NewBooleanArray(JNIEnv *env, jsize len);
JNIEXPORT jbyteArray JNICALL JEMCC_NewByteArray(JNIEnv *env, jsize len);
JNIEXPORT jcharArray JNICALL JEMCC_NewCharArray(JNIEnv *env, jsize len);
JNIEXPORT jshortArray JNICALL JEMCC_NewShortArray(JNIEnv *env, jsize len);
JNIEXPORT jintArray JNICALL JEMCC_NewIntArray(JNIEnv *env, jsize len);
JNIEXPORT jlongArray JNICALL JEMCC_NewLongArray(JNIEnv *env, jsize len);
JNIEXPORT jfloatArray JNICALL JEMCC_NewFloatArray(JNIEnv *env, jsize len);
JNIEXPORT jdoubleArray JNICALL JEMCC_NewDoubleArray(JNIEnv *env, jsize len);

JNIEXPORT jboolean *JNICALL JEMCC_GetBooleanArrayElements(JNIEnv *env,
                                                          jbooleanArray array,
                                                          jboolean *isCopy);
JNIEXPORT jbyte *JNICALL JEMCC_GetByteArrayElements(JNIEnv *env,
                                                    jbyteArray array,
                                                    jboolean *isCopy);
JNIEXPORT jchar *JNICALL JEMCC_GetCharArrayElements(JNIEnv *env,
                                                    jcharArray array,
                                                    jboolean *isCopy);
JNIEXPORT jshort *JNICALL JEMCC_GetShortArrayElements(JNIEnv *env,
                                                      jshortArray array,
                                                      jboolean *isCopy);
JNIEXPORT jint *JNICALL JEMCC_GetIntArrayElements(JNIEnv *env,
                                                  jintArray array,
                                                  jboolean *isCopy);
JNIEXPORT jlong *JNICALL JEMCC_GetLongArrayElements(JNIEnv *env,
                                                    jlongArray array,
                                                    jboolean *isCopy);
JNIEXPORT jfloat *JNICALL JEMCC_GetFloatArrayElements(JNIEnv *env,
                                                      jfloatArray array,
                                                      jboolean *isCopy);
JNIEXPORT jdouble *JNICALL JEMCC_GetDoubleArrayElements(JNIEnv *env,
                                                        jdoubleArray array,
                                                        jboolean *isCopy);

JNIEXPORT void JNICALL JEMCC_ReleaseBooleanArrayElements(JNIEnv *env,
                                                         jbooleanArray array,
                                                         jboolean *elems,
                                                         jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseByteArrayElements(JNIEnv *env,
                                                      jbyteArray array,
                                                      jbyte *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseCharArrayElements(JNIEnv *env,
                                                      jcharArray array,
                                                      jchar *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseShortArrayElements(JNIEnv *env,
                                                       jshortArray array,
                                                       jshort *elems,
                                                       jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseIntArrayElements(JNIEnv *env,
                                                     jintArray array,
                                                     jint *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseLongArrayElements(JNIEnv *env,
                                                      jlongArray array,
                                                      jlong *elems, jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseFloatArrayElements(JNIEnv *env,
                                                       jfloatArray array,
                                                       jfloat *elems,
                                                       jint mode);
JNIEXPORT void JNICALL JEMCC_ReleaseDoubleArrayElements(JNIEnv *env,
                                                        jdoubleArray array,
                                                        jdouble *elems,
                                                        jint mode);

JNIEXPORT void JNICALL JEMCC_GetBooleanArrayRegion(JNIEnv *env,
                                                   jbooleanArray array,
                                                   jsize start, jsize len,
                                                   jboolean *buff);
JNIEXPORT void JNICALL JEMCC_GetByteArrayRegion(JNIEnv *env,
                                                jbyteArray array,
                                                jsize start, jsize len,
                                                jbyte *buff);
JNIEXPORT void JNICALL JEMCC_GetCharArrayRegion(JNIEnv *env,
                                                jcharArray array,
                                                jsize start, jsize len,
                                                jchar *buff);
JNIEXPORT void JNICALL JEMCC_GetShortArrayRegion(JNIEnv *env,
                                                 jshortArray array,
                                                 jsize start, jsize len,
                                                 jshort *buff);
JNIEXPORT void JNICALL JEMCC_GetIntArrayRegion(JNIEnv *env,
                                               jintArray array,
                                               jsize start, jsize len,
                                               jint *buff);
JNIEXPORT void JNICALL JEMCC_GetLongArrayRegion(JNIEnv *env,
                                                jlongArray array,
                                                jsize start, jsize len,
                                                jlong *buff);
JNIEXPORT void JNICALL JEMCC_GetFloatArrayRegion(JNIEnv *env,
                                                 jfloatArray array,
                                                 jsize start, jsize len,
                                                 jfloat *buff);
JNIEXPORT void JNICALL JEMCC_GetDoubleArrayRegion(JNIEnv *env,
                                                  jdoubleArray array,
                                                  jsize start, jsize len,
                                                  jdouble *buff);

JNIEXPORT void JNICALL JEMCC_SetBooleanArrayRegion(JNIEnv *env,
                                                   jbooleanArray array,
                                                   jsize start, jsize len,
                                                   jboolean *buff);
JNIEXPORT void JNICALL JEMCC_SetByteArrayRegion(JNIEnv *env,
                                                jbyteArray array,
                                                jsize start, jsize len,
                                                jbyte *buff);
JNIEXPORT void JNICALL JEMCC_SetCharArrayRegion(JNIEnv *env,
                                                jcharArray array,
                                                jsize start, jsize len,
                                                jchar *buff);
JNIEXPORT void JNICALL JEMCC_SetShortArrayRegion(JNIEnv *env,
                                                 jshortArray array,
                                                 jsize start, jsize len,
                                                 jshort *buff);
JNIEXPORT void JNICALL JEMCC_SetIntArrayRegion(JNIEnv *env,
                                               jintArray array,
                                               jsize start, jsize len,
                                               jint *buff);
JNIEXPORT void JNICALL JEMCC_SetLongArrayRegion(JNIEnv *env,
                                                jlongArray array,
                                                jsize start, jsize len,
                                                jlong *buff);
JNIEXPORT void JNICALL JEMCC_SetFloatArrayRegion(JNIEnv *env,
                                                 jfloatArray array,
                                                 jsize start, jsize len,
                                                 jfloat *buff);
JNIEXPORT void JNICALL JEMCC_SetDoubleArrayRegion(JNIEnv *env,
                                                  jdoubleArray array,
                                                  jsize start, jsize len,
                                                  jdouble *buff);

JNIEXPORT jint JNICALL JEMCC_RegisterNatives(JNIEnv *env, jclass clazz,
                                             const JNINativeMethod *methods,
                                             jint nMethods);
JNIEXPORT jint JNICALL JEMCC_UnregisterNatives(JNIEnv *env, jclass clazz);

/* NOTE: MonitorEnter/MonitorExit are directly defined by sysenv.h */

JNIEXPORT jint JNICALL JEMCC_GetJavaVM(JNIEnv *env, JavaVM **vm);


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


/*********************** Coverage Test Methods *************************/

/**
 * Macros and counter/test methods which provide for code coverage testing
 * by "simulating" extraneous error conditions through a sequence counter.
 */
#ifdef ENABLE_ERRORSWEEP

#define ES_MEM 1
#define ES_DATA 2
#define ES_EXC 4
JNIEXPORT int JNICALL JEM_CheckErrorSweep(int sweepType);
#define ERROR_SWEEP(t, x) ((JEM_CheckErrorSweep(t) == JNI_TRUE) || (x))

#else

#define ERROR_SWEEP(t, x) (x)

#endif

/********************* Memory/Object Management ************************/

/**
 * Allocate a block of memory for storage purposes.  This method will
 * automatically trigger GC object cleanups as required, as well as
 * providing a central point for the memory failure test conditions.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     size - the number of bytes to allocate
 *
 * Returns:
 *     NULL if memory allocation failed, otherwise the allocated storage block.
 *     The allocated block of memory will be initialized to zero (calloc).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT void *JNICALL JEMCC_Malloc(JNIEnv *env, juint size);

/*
 * The main object (java.lang.Object) structure.  Every JEMCC object consists
 * of the common fixed structural data (class reference and state bitsets)
 * followed by the dynamic elements (class field data).  Note that the latter
 * structure definition is more commonly used by JEMCC instances which
 * directly access attached data elements and the final structure definition
 * provides convenient access to class definition data.
 */
struct JEMCC_Object {
    JEMCC_Class *classReference;
    juint objStateSet;
};

typedef struct JEMCC_ObjectExt {
    JEMCC_Class *classReference;
    juint objStateSet;

    /*
     * This element doesn't necessarily exist.  It just simplifies the
     * referencing of the dynamic object elements.
     */
    void *objectData;
} JEMCC_ObjectExt;

/* Convienence structural definitions for "core" typed VM objects */

/**
 * A standard class instance is just a base Object with native reference to
 * the class definition information (opaque) and the static data area
 * reference.
 */
struct JEMCC_Class {
    JEMCC_Class *classReference;
    juint objStateSet;

    struct JEM_ClassData *classData;
    void *staticData;
};

/**
 * An array contains an opaque reference to the native contents and the
 * defined length of the native array.
 */
typedef struct JEMCC_ArrayObject {
    JEMCC_Class *classReference;
    juint objStateSet;

    void *arrayData;
    jint arrayLength;
} JEMCC_ArrayObject;

/**
 * An array class maps to the base class definition with additional
 * members for depth and reference information.
 */
typedef struct JEMCC_ArrayClass {
    JEMCC_Class *classReference;
    juint objStateSet;

    /* In this case, the native data reference is the class information */
    /* and the array details are appended manually */
    struct JEM_ClassData *classData;
    juint typeDepthInfo;
    JEMCC_Class *referenceClass;
} JEMCC_ArrayClass;

/**
 * Allocate the memory block for a java.lang.Object based instance.  Also
 * provides the auto-allocation of internal attachment data which can be
 * used by the VM and JEMCC classes.  This does NOT call any <init>
 * constructors of the given class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class of the Object instance to be allocated (determines
 *                 the trailing dynamic element size and permits basic
 *                 initialization of the object)
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
 *     ExceptionInInitializerError - the initialization of the given class
 *                                   failed in some respect
 *     NoClassDefFoundError - the specified class has had a previous
 *                            initialization error and is now unusable
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_AllocateObject(JNIEnv *env,
                                                     JEMCC_Class *classInst,
                                                     juint objDataSize);

/**
 * Perform a clone operation on the indicated object - essentially a 
 * direct call of the Object.clone() method.  As described in that method,
 * this creates a new object of the same class and makes an exact copy
 * of the field contents of the object - it does not perform a "deep"
 * clone of the object.  JEMCC implementations which utilize native
 * object data will need to implement a clone() method to ensure that
 * a duplicate is made of native data area.
 *
 * The only exception to the above is arrays - if an array object is
 * provided to this method, a copy will also be made of the native
 * storage area automatically.
 *
 * Note: this method does not validate that the given object is, in fact,
 * cloneable (implements the Cloneable interface).  Nor does it perform a
 * NULL check on the provided object.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to be cloned
 *
 * Returns:
 *     NULL if memory allocation failed, otherwise the cloned object instance.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_CloneObject(JNIEnv *env,
                                                  JEMCC_Object *obj);

/**
 * Release an object instance (immediate GC).  This method should only
 * be used where an internal processing failure has occurred and the
 * given object has NEVER appeared outside of C local variables (no
 * verification is made that no other object is referencing this object).
 * This is equivalent to calling free() on the object but also removes
 * the object instance from the GC reference tracking tables as it can
 * immediately be destroyed.
 *
 * NOTE: this method will NOT call the finalize() method on the given
 *       object.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     object - the object instance to be released and removed from the
 *              GC tracking tables
 */
JNIEXPORT void JNICALL JEMCC_ReleaseObject(JNIEnv *env, JEMCC_Object *object);

/**
 * Method used to "mark" an object when it is promoted to a non-local
 * context (stored in a field of another object).  Once this mark has
 * been made, full referential GC must be performed on the object to
 * ensure that it is not in use in other environments.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     object - the object instance which is to be marked non-local
 */
JNIEXPORT void JNICALL JEMCC_MarkNonLocalObject(JNIEnv *env,
                                                JEMCC_Object *object);

/**
 * Create a new instance of an object, calling the specified constructor
 * instance (essentially the Java "new" operator).  Note that the provided
 * frame instance must have the constructor arguments properly initialized
 * for the constructor call, not including the "this" object which will be
 * pushed onto the stack by this method.
 *
 * Note: this will create the object as a local frame object, which will
 * be immediately GC'd upon the exit of the current frame, unless the 
 * MarkNonLocalObject call is made (or the JNI field storage methods are
 * called).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     frame - the executing frame which contains the constructor arguments
 *             (if required)
 *     classInst - the class of the object to be instantiated
 *     desc - the descriptor of the constructor <init> method to be called
 *
 * Returns:
 *     The newly instantiated object of the requested class, or NULL if
 *     an error occurred in creating the object (an exception will be thrown
 *     in the current environment). 
 *
 * Exceptions:
 *     Any which may arise from the comparable Java "new" operator (class
 *     initialization errors, resolution errors, constructor opcode errors,
 *     memory errors and others).
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_NewObjectInstance(JNIEnv *env,
                                                        JEMCC_VMFrame *frame, 
                                                        JEMCC_Class *classInst,
                                                        const char *desc);

/********************* Array Utilities *********************************/

/**
 * Convenience method to validate the accessibility of the array member
 * defined by the given index.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     array - the array instance to validate against
 *     index - the array index to validate
 *     excIdx - the VM exception index to throw (sometimes IndexOutOfBounds,
 *              sometimes ArrayIndexOutOfBounds).  If negative, the
 *              ArrayIndexOutOfBounds exception will be used by default.
 *
 * Returns:
 *     JNI_OK if the array index is valid, JNI_ERR otherwise (the
 *     indicated exception will be thrown in the current environment).
 *
 * Exceptions:
 *     As determined by the excIdx value.
 */
JNIEXPORT jint JNICALL JEMCC_CheckArrayLimits(JNIEnv *env,
                                              JEMCC_ArrayObject *array,
                                              jsize index, jint excIdx);

/**
 * Convenience method to validate the accessibility of the array members
 * in the region offset -> offset + len - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     array - the array instance to validate against
 *     offset - the array index of the first member in the region
 *     len - the number of array members in the region
 *     excIdx - the VM exception index to throw (sometimes IndexOutOfBounds,
 *              sometimes ArrayIndexOutOfBounds).  If negative, the
 *              ArrayIndexOutOfBounds exception will be used by default.
 *
 * Returns:
 *     JNI_OK if the array region is valid, JNI_ERR otherwise (the
 *     indicated exception will be thrown in the current environment).
 *
 * Exceptions:
 *     As determined by the excIdx value.
 */
JNIEXPORT jint JNICALL JEMCC_CheckArrayRegion(JNIEnv *env,
                                              JEMCC_ArrayObject *array,
                                              jsize offset, jsize len,
                                              jint excIdx);

/**
 * Convenience method to validate the accessibility of the array members
 * in the segment from start to end - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     array - the array instance to validate against
 *     start - the index of the first array member
 *     end - the index of the last array member plus one
 *     excIdx - the VM exception index to throw (sometimes IndexOutOfBounds,
 *              sometimes ArrayIndexOutOfBounds).  If negative, the
 *              ArrayIndexOutOfBounds exception will be used by default.
 *
 * Returns:
 *     JNI_OK if the array index is valid, JNI_ERR otherwise (the
 *     indicated exception will be thrown in the current environment).
 *
 * Exceptions:
 *     As determined by the excIdx value.
 */
JNIEXPORT jint JNICALL JEMCC_CheckArraySegment(JNIEnv *env,
                                               JEMCC_ArrayObject *array,
                                               jsize start, jsize end,
                                               jint excIdx);

/********************* String Functions *********************************/

/**
 * Convenience method to concatenate a NULL terminated set of char*
 * text pointers into a single char* result, handling memory exceptions
 * appropriately.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the list
 *
 * Returns:
 *     The character string resulting from the concatentation of the given
 *     character sequences or NULL if a memory allocation has occurred
 *     (an OutOfMemoryError has been thrown in the current environment).
 *     The caller is responsible for free'ing this string result.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT char *JNICALL JEMCC_StrCatFn(JNIEnv *env, ...);

/**
 * Attachment data structure associated with java.lang.String instances.
 * In all cases, the allocation of memory associated with this structure
 * is variable, as the byte information for the string is allocated as
 * part of the structure and begins with the "data" character.
 *
 * Note that the JEMCC implementation stores strings in ASCII or Unicode,
 * to save memory for pure ASCII situations.  If the length is less than
 * zero, the byte information beginning at the data marker is 7-bit ASCII
 * text (of the given negative length) which is always NULL terminated.
 * If the length is greater than zero, the byte information beginning at
 * the data marker is 16-bit (jchar) Unicode information of the given
 * length.
 */
typedef struct JEMCC_StringData {
    jsize length;

    char data;
} JEMCC_StringData;

/**
 * Scans the provided UTF-8 string information to determine if the contents
 * are ASCII (7-bit characters).
 *
 * Parameters:
 *     utfStrData - the UTF-8 encoded string data to scan (must be terminated
 *                  by the '\0' character)
 *
 * Returns:
 *     JNI_TRUE if the contents of the UTF-8 encoded string are ASCII
 *     characters only.
 */
JNIEXPORT jboolean JNICALL JEMCC_UTFStrIsAscii(const char *utfStrData);

/**
 * Scans the provided Unicode string information (16-bit) to determine if the
 * contents are ASCII (7-bit characters).
 *
 * Parameters:
 *     uniStrData - the Unicode text to scan
 *     len - the number of Unicode characters in the text
 *
 * Returns:
 *     JNI_TRUE if the contents of the Unicode string are ASCII
 *     characters only.
 */
JNIEXPORT jboolean JNICALL JEMCC_UnicodeStrIsAscii(const jchar *uniStrData,
                                                   jsize len);

/**
 * Generate/retrieve an "interned" string instance based on the provided
 * UTF-8 encoded string information.  If the given character sequence is found
 * in the intern()'ed String hashtable associated with the VM, that String is
 * returned.  Otherwise, a new String is created with the given characters,
 * stored in the VM hashtable and returned.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     utfStrData - the UTF-8 encoded string data to internalize (must be
 *                  terminated by the '\0' character)
 *
 * Returns:
 *     The interned String instance (either found or created) or NULL if
 *     a String creation was attempted but failed due to a memory error
 *     (an OutOfMemoryError has been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_GetInternStringUTF(JNIEnv *env,
                                                        const char *utfStrData);

/**
 * Generate/retrieve an "interned" string instance based on the provided
 * Unicode string information.  If the given character sequence is found
 * in the intern()'ed String hashtable associated with the VM, that String is
 * returned.  Otherwise, a new String is created with the given characters,
 * stored in the VM hashtable and returned.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     uniStrData - the Unicode text to internalize
 *     len - the number of characters in the Unicode text
 *
 * Returns:
 *     The interned String instance (either found or created) or NULL if
 *     a String creation was attempted but failed due to a memory error
 *     (an OutOfMemoryError has been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_GetInternString(JNIEnv *env,
                                                      const jchar *uniStrData,
                                                      jsize len);

/**
 * Method provided for the java.lang.String class implementation to access
 * the internalized String table in the VM.  Locates the intern()'ed String
 * instance with the same character sequence as the given String, or inserts
 * the given String into the internalized String table and returns the given
 * instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     string - the String instance containing the character sequence to locate
 *              the internalized String instance for
 *
 * Returns:
 *     The intern()'ed String instance associated with the provided String
 *     instance (will be the same instance if not found in the table) or
 *     NULL if a memory error occurred while inserting the String into the
 *     VM hashtable (an OutOfMemoryError has been thrown in the current
 *     environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_FindInternString(JNIEnv *env,
                                                       JEMCC_Object *string);

/**
 * Convenience method to construct a String instance from a NULL
 * terminated list of UTF-8 encoded elements.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - a series of char* UTF-8 encoded character sequences (each ending
 *           with the '\0' character), ending with a (char *) NULL pointer 
 *           marking the end of the list
 *
 * Returns:
 *     A java.lang.String instance containing the concatentation of the given
 *     character sequences or NULL if a memory allocation has occurred
 *     (an OutOfMemoryError has been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_StringCatUTF(JNIEnv *env, ...);

/**
 * Hash generation function which operates with the java.lang.String
 * attachment data structure (the provided key is the JEMCC_StringData
 * structure, NOT the java.lang.String object reference).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the JEMCC_StringData reference to be hashed
 *
 * Returns:
 *     The numerical hashcode for the provided String contents.
 */
JNIEXPORT juint JNICALL JEMCC_StringHashFn(JNIEnv *env, void *key);

/**
 * Comparison function which operates with the java.lang.String
 * attachment data structure (the provided keys are the JEMCC_StringData
 * structures, NOT the java.lang.String object references).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the JEMCC_StringData references to be compared
 *
 * Returns:
 *     JNI_TRUE if the two string instances are equal (same character
 *     sequences) otherwise JNI_FALSE.
 */
JNIEXPORT jboolean JNICALL JEMCC_StringEqualsFn(JNIEnv *env, void *keya,
                                                void *keyb);

/**
 * Dump the contents of a String instance, either to the standard error
 * or output channel.  Used for debugging purposes.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     string - the String instance to dump
 *     useError - if JNI_TRUE, use the standard error channel, if JNI_FALSE,
 *                use standard output instead
 */
JNIEXPORT void JNICALL JEMCC_DumpString(JNIEnv *env, JEMCC_Object *string,
                                        jboolean useError);

/**
 * Convenience method to validate the accessibility of the indexed String
 * character.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the String data structure to validate against
 *     index - the character index to validate
 *
 * Returns:
 *     JNI_OK if the String index is valid, JNI_ERR otherwise (an
 *     exception will be thrown in the current environment).
 *
 * Exceptions:
 *     StringIndexOutOfBoundsException - an invalid index was given
 */
JNIEXPORT jint JNICALL JEMCC_CheckStringLimits(JNIEnv *env,
                                               JEMCC_StringData *strData,
                                               jsize index);

/**
 * Convenience method to validate the accessibility of the String characters
 * in the region offset -> offset + len - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the String data structure to validate against
 *     offset - the character index of the first member in the region
 *     len - the number of characters in the region
 *
 * Returns:
 *     JNI_OK if the array region is valid, JNI_ERR otherwise (an
 *     exception will be thrown in the current environment).
 *
 * Exceptions:
 *     StringIndexOutOfBoundsException - an invalid region was given
 */
JNIEXPORT jint JNICALL JEMCC_CheckStringRegion(JNIEnv *env,
                                               JEMCC_StringData *strData,
                                               jsize offset, jsize len);
/**
 * Convenience method to validate the accessibility of the String characters
 * in the segment from start to end - 1.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the String data structure to validate against
 *     start - the index of the first String character
 *     end - the index of the last character plus one
 *
 * Returns:
 *     JNI_OK if the array index is valid, JNI_ERR otherwise (an
 *     exception will be thrown in the current environment).
 *
 * Exceptions:
 *     StringIndexOutOfBoundsException - an invalid segment was given
 */
JNIEXPORT jint JNICALL JEMCC_CheckStringSegment(JNIEnv *env,
                                                JEMCC_StringData *strData,
                                                jsize start, jsize end);

/**
 * Common method for use with java.lang.String, java.lang.StringBuffer
 * and any other method requiring rapid access to the character array
 * contents of the StringData structure.  Does not perform any range checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the JEMCC_StringData structure containing the String data
 *     begin - the index of the first character to extract
 *     end - the index of the last character to extract plus one (e.g.
 *           extract occurs from begin to end - 1)
 *     buff - the char[] buffer to extract the String contents into
 */
JNIEXPORT void JNICALL JEMCC_StringGetChars(JNIEnv *env, 
                                            JEMCC_StringData *strData,
                                            jint begin, jint end,
                                            jchar *buff);

/**
 * Common method for use with java.lang.String, java.lang.StringBuffer
 * and any other method requiring rapid creation of substrings based on
 * a parent String instance.  Does not perform any range checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     strData - the JEMCC_StringData structure containing the String data
 *     begin - the index of the first character to extract
 *     end - the index of the last character to extract plus one (e.g.
 *           extract occurs from begin to end - 1)
 *
 * Returns:
 *     NULL if a memory allocation failure occurred (no substring created).
 *     Otherwise, the substring as requested.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_StringSubstring(JNIEnv *env, 
                                                      JEMCC_StringData *strData,
                                                      jint begin, jint end);

/**
 * Common method for use with java.lang.String, java.lang.StringBuffer
 * and any other method performing subString searches within another String
 * (like strstr()).  Does not perform any range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the JEMCC_StringData structure containing the search string
 *     fromIdx - the index to begin searching at (if invalid, assumes 0)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the first 
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
JNIEXPORT jint JNICALL JEMCC_StringString(JNIEnv *env, 
                                          JEMCC_StringData *haystack,
                                          JEMCC_StringData *needle,
                                          jint fromIdx);

/**
 * Like the above, but performs the search in reverse order starting from
 * the given index (for lastIndexOf() calls).  Does not perform any 
 * range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the JEMCC_StringData structure containing the search string
 *     fromIdx - the index to begin searching at (if invalid, assumes end)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the last 
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
JNIEXPORT jint JNICALL JEMCC_LastStringString(JNIEnv *env, 
                                              JEMCC_StringData *haystack,
                                              JEMCC_StringData *needle,
                                              jint fromIdx);

/**
 * Common method for use with any method performing character searches 
 * within a String (like strchr()).  Does not perform any range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the character to search for
 *     fromIdx - the index to begin searching at (if invalid, assumes 0)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the first 
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
JNIEXPORT jint JNICALL JEMCC_StringChar(JNIEnv *env, 
                                        JEMCC_StringData *haystack,
                                        jchar needle, jint fromIdx);

/**
 * Like the above, but performs the search in reverse order starting from
 * the given index (for lastIndexOf() calls).  Does not perform any 
 * range/null checks.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     haystack - the JEMCC_StringData structure containing the string to search
 *     needle - the character to search for
 *     fromIdx - the index to begin searching at (if invalid, assumes end)
 *
 * Returns:
 *     -1 if the needle was not found, otherwise the index of the last
 *     occurence of the needle in the haystack starting at the fromIdx.
 */
JNIEXPORT jint JNICALL JEMCC_LastStringChar(JNIEnv *env, 
                                            JEMCC_StringData *haystack,
                                            jchar needle, jint fromIdx);

/********************* Hashtable Management ************************/

/*
 * Hashtable data management.  Generic hashing structure and methods for
 * maintaining keyed data records.  Note that this is used internally by
 * the engine routines and is also used by the Hashtable implementation.
 */
typedef struct JEMCC_HashTable {
    juint entryCount, occupied;
    juint tableMask;
    struct JEM_HashEntry *entries;
} JEMCC_HashTable;

/**
 * Hash generation function prototype for use with the hashtable methods
 * below.  This function must always return the same hashcode for keys
 * which are "equal", but unequal keys are allowed to generate the same
 * hashcode (although it is potentially faster if they don't).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the key associated with the object to be hashed
 *
 * Returns:
 *     The numeric hashcode for the given key instance.
 */
typedef juint (*JEMCC_KeyHashFn)(JNIEnv *env, void *key);

/**
 * Comparison function for locating key matches in hashtables for use
 * with the hashtable methods below.  This function may do a simple
 * pointer comparison or a more complex equals comparison based on data
 * contained within the key structures.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the two keys to be compared
 *
 * Returns:
 *     JNI_TRUE if the values of the two keys are "equal", JNI_FALSE otherwise.
 */
typedef jboolean (*JEMCC_KeyEqualsFn)(JNIEnv *env, void *keya, void *keyb);

/**
 * Duplication function to create a new key based on the given key.  This
 * may be a simple pointer copy (easiest case) or a full duplication of the
 * information represented by the key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the key instance to be copied
 *
 * Returns:
 *     The "copy" of the provided key, or NULL if a memory allocation
 *     failed in the key duplication (and exception must be thrown in this
 *     case).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
typedef void *(*JEMCC_KeyDupFn)(JNIEnv *env, void *key);

/**
 * Callback method to enumerate entries within a hashtable.  Allows for
 * termination of the scanning through the return code.  NOTE: the hashtable
 * should only be modified during scanning using the "safe" methods such
 * as HashScanRemoveEntry below.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the table which is currently being scanned
 *     key - the key associated with the currently scanned entry
 *     object - the object associated with the currently scanned entry
 *     userData - the caller provided information attached to the scan
 *                request
 *
 * Returns:
 *     JNI_OK - continue with scanning of hashtable
 *     JNI_ERR - terminate scanning of hashtable
 */
typedef jint (*JEMCC_EntryScanCB)(JNIEnv *env, JEMCC_HashTable *table,
                                  void *key, void *obj, void *userData);

/**
 * Initialize a hash table instance to the given number of base hash
 * points.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - pointer to an existing instance of the hashtable to
 *             be initialized.  Already existing entries in the table
 *             will not be cleaned up
 *     startSize - the number of hash blocks to initially allocate in the
 *                 table.  If <= 0, an appropriate start size will be selected.
 *
 * Returns:
 *     JNI_OK - hashtable was initialized
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashInitTable(JNIEnv *env, JEMCC_HashTable *table,
                                           jsize startSize);
/**
 * Store an object into a hashtable.  Hashtable will expand as necessary,
 * and object will replace an already existing object with an equal key.
 * If an existing object is replaced, it is not destroyed but the key/object
 * pair is returned to allow the caller to clean up.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to put the key->value pair into
 *     key - the key associated with the entry
 *     object - the object to store in the hashtable according to the given key
 *     lastKey - if this pointer is non-NULL, the previous key is returned if
 *               the put entry replaces one in the hashtable or NULL is
 *               returned if the entry is new
 *     lastObject - if this pointer is non-NULL, the previous object is
 *                  returned if the put entry replaces one in the hashtable or
 *                  NULL is returned if the entry is new
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - hashtable insertion was successfuly
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashPutEntry(JNIEnv *env, JEMCC_HashTable *table,
                                          void *key, void *object,
                                          void **lastKey, void **lastObject,
                                          JEMCC_KeyHashFn keyHashFn,
                                          JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Almost identical to the HashPutEntry method, this method stores an
 * key->object entry into a hashtable unless there already exists an entry
 * in the hashtable with an "equal" key (i.e. this method will not replace
 * already existing hashtable entries where HashPutEntry does).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to put the key->value pair into
 *     key - the key associated with the entry
 *     object - the object to store in the hashtable according to the given key
 *     lastKey - if this pointer is non-NULL, the existing key is returned if
 *               the insert did not happen (no replace) or NULL if the entry
 *               is new and was inserted
 *     lastObject - if this pointer is non-NULL, the existing key is returned
 *                  if the insert did not happen (no replace) or NULL if the
 *                  entry is new and was inserted
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - hashtable insertion was successful
 *     JNI_ERR - an entry already exists in the hashtable for the given key
 *               and no action was taken (no exception has been thrown either)
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashInsertEntry(JNIEnv *env,
                                             JEMCC_HashTable *table,
                                             void *key, void *object,
                                             void **lastKey, void **lastObject,
                                             JEMCC_KeyHashFn keyHashFn,
                                             JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Remove an entry from the hashtable.  This does not destroy the removed
 * object/key, only the reference to them.  The original key/object pair
 * can be returned to the caller for cleanup purposes.  NOTE: this method
 * is not "safe" for use during hashtable scanning - use HashScanRemoveEntry
 * instead.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to remove the entry from
 *     key - the key of the pair entry to be removed
 *     origKey - if this pointer is non-NULL and an entry is removed, the
 *               original key of the entry is returned here
 *     origObject - if this pointer is non-NULL and an entry is removed, the
 *                  object associated with the entry is returned here
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - a matching entry was found and has been removed
 *     JNI_ERR - no entry matching the specified key was found (no exception
 *               is thrown as it may not really be an error)
 */
JNIEXPORT jint JNICALL JEMCC_HashRemoveEntry(JNIEnv *env,
                                             JEMCC_HashTable *table,
                                             void *key,
                                             void **origKey, void **origObject,
                                             JEMCC_KeyHashFn keyHashFn,
                                             JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Retrieve an object from the hashtable according to the specified key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to retrieve the entry from
 *     key - the key of the object to be obtained
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     NULL if no object entry has a matching key, otherwise the matching
 *     object reference.
 */
JNIEXPORT void *JNICALL JEMCC_HashGetEntry(JNIEnv *env, JEMCC_HashTable *table,
                                           void *key,
                                           JEMCC_KeyHashFn keyHashFn,
                                           JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Similar to the HashGetEntry method, this retrieves entry information
 * for the provided key, but obtains both the object and the key associated
 * with the hashtable entry.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to retrieve the entry from
 *     key - the key of the object to be obtained
 *     retKey - if non-NULL, the entry key is returned if a matching entry
 *              was found, otherwise NULL is returned
 *     retObject - if non-NULL, the entry object is returned if a matching
 *                 entry was found, otherwise NULL is returned
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - an entry matching the key was found and the data was returned
 *     JNI_ERR - no entry matching the key was found
 */
JNIEXPORT jint JNICALL JEMCC_HashGetFullEntry(JNIEnv *env,
                                              JEMCC_HashTable *table,
                                              void *key,
                                              void **retKey, void **retObject,
                                              JEMCC_KeyHashFn keyHashFn,
                                              JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Duplicate the given hashtable.  This will create copies of the internal
 * management structures of the hashtable and may possibly create duplicates
 * of the entry keys, if a duplication function is provided.  It does not
 * duplicate the object instances, only the references to the objects.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     dest - the hashtable to copy information into.  Any entries in this
 *            table will be lost without any sort of cleanup
 *     source - the hashtable containing the information to be copied
 *     keyDupFn - if non-NULL, this function will be called to duplicate
 *                the key instances between the tables
 *
 * Returns:
 *     JNI_OK - hashtable was duplicated
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment.  The duplicate hashtable may be partially
 *                  filled, if the memory failure occurred during key
 *                  duplication.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEMCC_HashDuplicate(JNIEnv *env, JEMCC_HashTable *dest,
                                           JEMCC_HashTable *source,
                                           JEMCC_KeyDupFn keyDupFn);

/**
 * Scan through all entries in a hashtable, calling the specified
 * callback function for each valid hashtable entry.  NOTE: only the
 * "safe" methods (such as HashScanRemoveEntry below) should be used while
 * a hashtable scan is in progress.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable containing the entries to be scanned
 *     entryCB - a function reference which is called for each valid
 *               entry in the hashtable
 *     userData - a caller provided data set which is included in the
 *                scan arguments
 */
JNIEXPORT void JNICALL JEMCC_HashScan(JNIEnv *env, JEMCC_HashTable *table,
                                      JEMCC_EntryScanCB entryCB,
                                      void *userData);

/**
 * Identical to the HashRemoveEntry method, this removes an entry from
 * the hashtable but is "safe" to use while a scan of the hashtable is
 * in progress.  This does not destroy the removed object/key - only the
 * reference to them.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to remove the entry from
 *     key - the key of the pair entry to be removed
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - a matching entry was found and has been removed
 *     JNI_ERR - no entry matching the specified key was found (no exception
 *               is thrown as it may not really be an error)
 */
JNIEXPORT void JNICALL JEMCC_HashScanRemoveEntry(JNIEnv *env,
                                                 JEMCC_HashTable *table,
                                                 void *key,
                                                 JEMCC_KeyHashFn keyHashFn,
                                                 JEMCC_KeyEqualsFn keyEqualsFn);

/**
 * Destroy the internals of a hashtable instance.  Does NOT destroy the
 * objects contained in the hashtable or the hashtable structure itself.
 *
 * Parameters:
 *     table - the hashtable instance to be internally destroyed
 */
JNIEXPORT void JNICALL JEMCC_HashDestroyTable(JEMCC_HashTable *table);

/* * * * * * * * * * * Convenience Hash Methods * * * * * * * * * * */

/**
 * Generate the hashcode value for a char sequence of characters (may
 * be ASCII or UTF-8 encoded Unicode) given in the key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the character sequence to be hashed (must be \0 terminated)
 *
 * Returns:
 *     The numerical hashcode associated with the given characters.
 */
JNIEXPORT juint JNICALL JEMCC_StrHashFn(JNIEnv *env, void *key);

/**
 * Perform a comparison of the character information contained in the
 * two given keys.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the two character sequences to be compared
 *                  (must be \0 terminated)
 *
 * Returns:
 *     JNI_TRUE if the two character sequences are identical (including
 *     null character), JNI_FALSE otherwise.
 */
JNIEXPORT jboolean JNICALL JEMCC_StrEqualsFn(JNIEnv *env, void *keya,
                                             void *keyb);

/**
 * Generate a duplicate of the character information provided in the given
 * key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the character sequence to be duplicated (must be \0 terminated)
 *
 * Returns:
 *     An allocated copy of the provided character sequence or NULL if
 *     the memory allocation failed (an OutOfMemoryError will have been
 *     thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT void *JNICALL JEMCC_StrDupFn(JNIEnv *env, void *key);

/**
 * Generate the hashcode value for a fully qualified Java class name.  In
 * this case, the hash result is the same whether the '.' or '/' package
 * separator is used (i.e. 'java/lang/Object' and 'java.lang.Object' will
 * generate the same hashcode).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the fully qualified classname to be hashed (must be \0 terminated)
 *
 * Returns:
 *     The numerical hashcode for the classname, "ignoring" package
 *     separators.
 */
JNIEXPORT juint JNICALL JEMCC_ClassNameHashFn(JNIEnv *env, void *key);

/**
 * Perform a string comparison of two fully qualified Java class names,
 * ignoring any differences in the '.' or '/' package separators
 * (i.e. 'java/lang/Object' and 'java.lang.Object' are identical according
 * to this method).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya - the fully qualified classname to be compared
 *     keyb - the fully qualified classname to compare against
 *            (both classnames must be \0 terminated)
 *
 * Returns:
 *     JNI_TRUE if the two classnames are equal ignoring separator differences,
 *     JNI_FALSE if they are different.
 */
JNIEXPORT jboolean JNICALL JEMCC_ClassNameEqualsFn(JNIEnv *env,
                                                   void *keya, void *keyb);


/************************ Throwable Management ****************************/

/**
 * Structure which defines the private throwable details in a more
 * "program-friendly" fashion.  Methods within exception handlers can
 * access this structure using the following cast:
 *
 *   (JEMCC_ThrowableData *) &(object->objectData)
 */
typedef struct JEMCC_ThrowableData {
    JEMCC_Object *message;
    JEMCC_Object *causeThrowable;

    /* The following are intended to be "opaque" to non-VM entities */
    struct JEMCC_StackTraceEntryData *stackTrace;
    jint stackTraceDepth;
} JEMCC_ThrowableData;

/*
 * Standard throwable handling.  Most exceptions in Java are a basic class
 * which extends Throwable and simply carry a message and (as of 1.4)
 * possibly a cause exception.  These methods simply the throwing of
 * exceptions by instantiating an exception instance and defining this
 * basic information without the overhead of constructor invocations.
 * Of course, these methods should NOT be used in cases where more exact
 * constructors are to be used to initialize other information.
 */

/* See vmclasses.h for similar methods using VM core indices */

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * name.  Performs a simple class lookup for the provided name against the
 * classloader associated with the given object or the class to which the
 * currently executing method belongs.  See the 'jemcc.h' file for more 
 * details on what constitutes a "standard" throwable instance in JEMCC 
 * (essentially, a Throwable subclass which does nothing more than provide 
 * a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     srcObj - if non-NULL, load the throwable class from the classloader
 *              associated with the given object.  If NULL, use the classloader
 *              which is associated with the currently executing method
 *     className - the classname for the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError or ClassNotFoundException) due to difficulties
 *     in loading or creating the throwable instance.
 *     
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableByName(JNIEnv *env,
                                                  JEMCC_Object *srcObj,
                                                  const char *className,
                                                  JEMCC_Object *causeThrowable,
                                                  const char *msg);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * name.  Performs a simple class lookup for the provided name against the
 * classloader associated with the given object or the class to which the
 * currently executing method belongs.  See the 'jemcc.h' file for more 
 * details on what constitutes a "standard" throwable instance in JEMCC 
 * (essentially, a Throwable subclass which does nothing more than provide 
 * a new classname).
 *
 * This method is identical to the above method except that it allows for
 * vararg message construction.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     srcObj - if non-NULL, load the throwable class from the classloader
 *              associated with the given object.  If NULL, use the classloader
 *              which is associated with the currently executing method
 *     className - the classname for the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError or ClassNotFoundException) due to difficulties
 *     in loading or creating the throwable instance.
 *     
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableByNameV(JNIEnv *env,
                                                  JEMCC_Object *srcObj,
                                                  const char *className,
                                                  JEMCC_Object *causeThrowable,
                                                  ...);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * instance.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     msg - the text message for the standard throwable instance
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowable(JNIEnv *env,
                                               JEMCC_Class *throwclass,
                                               JEMCC_Object *causeThrowable,
                                               const char *msg);

/**
 * Construct and throw a "standard" VM throwable based on the provided class
 * instance.  See the 'jemcc.h' file for more details on what constitutes a
 * "standard" throwable instance in JEMCC (essentially, a Throwable subclass
 * which does nothing more than provide a new classname).
 *
 * This method is identical to the above method except that it allows for
 * vararg message construction.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the standard throwable to throw
 *     causeThrowable - if non-NULL, attach this throwable to the thrown
 *                      instance as the "cause" of this throwable
 *     ... - a series of char* encoded character sequences, ending
 *           with a (char *) NULL pointer marking the end of the message
 *
 * Exceptions:
 *     Either the requested exception instance, or a different exception
 *     (such as OutOfMemoryError) due to allocation difficulties.
 */
JNIEXPORT void JNICALL JEMCC_ThrowStdThrowableV(JNIEnv *env,
                                                JEMCC_Class *throwclass,
                                                JEMCC_Object *causeThrowable,
                                                ...);

/**
 * "Catch" an instance of a throwable class (based on an outstanding exception 
 * in the current environment) for the external class reference established 
 * during linking.  Will return and clear the pending exception if an exception
 * exists which is assignable to the indicated reference class instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classIdx - the index of the external throwable class reference to
 *                catch
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_CatchThrowableRef(JNIEnv *env,
                                                        jint classIdx);

/**
 * "Catch" an instance of a throwable class (based on an outstanding exception 
 * in the current environment).  Will return and clear the pending exception if
 * an exception exists which is assignable to the provided class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     throwClass - the class of the throwable to be caught
 *
 * Returns:
 *     NULL if there is no pending exception or the pending exception is not
 *     an instance of the indicated class, otherwise the outstanding instance
 *     of the exception in the current environment (which will be cleared).
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_CatchThrowable(JNIEnv *env,
                                                     JEMCC_Class *throwClass);


/********************** CPU/Engine Operations **************************/

/**
 * This defines the type union for both local variables and operand stacks.
 * While it is exposed, it should only be used indirectly through the
 * JEMCC_* macros defined below.
 */
typedef union JEM_FrameEntry {
    jint i;
    jfloat f;
#ifdef DBL_SNGL_FRAME_MISALIGNED
    jlong l;
    jdouble d;
#endif
    JEMCC_Object *obj;
    /* Note that retAddr is equivalent to object, as VM doesn't differentiate */
    void *retAddr;
} JEM_FrameEntry;

/**
 * The structure definition of the base references of a method frame
 * instance (the operand stack and the local variable set).  This structure
 * should not be used directly but rather the JEMCC_* macros defined
 * below should be used to manipulate the frame information.
 */
struct JEMCC_VMFrame {
    JEM_FrameEntry *operandStackTop;
    JEM_FrameEntry *localVars;
};

/**
 * Structure definition to overlay double/long dual stack record entries.
 * Note - this needs to be exactly twice the size of JEM_FrameEntry, to
 * properly handle the entry indexing as defined in the Java virtual machine
 * specification.  Also note that this should not be used directly by
 * JEMCC method instances but the JEMCC_* macros defined below should be
 * used instead.
 */
typedef union JEM_DblFrameEntry {
    /* Dummy union entry to ensure sizeof two frame units */
    struct dblrecord {
        JEM_FrameEntry one;
        JEM_FrameEntry two;
    } dblrec;

    /* Actual frame data elements which require two stack index units */
    jlong l;
    jdouble d;
} JEM_DblFrameEntry;

/**
 * The following macros define the "methods" by which a JEMCC method
 * can manipulate the operand stack (for method calls) or the local
 * variable store (for calling arguments).
 */
#define JEMCC_STORE_INT(frame, index, val) frame->localVars[index].i = val;
#define JEMCC_LOAD_INT(frame, index) frame->localVars[index].i

#define JEMCC_STORE_FLOAT(frame, index, val) frame->localVars[index].f = val;
#define JEMCC_LOAD_FLOAT(frame, index) frame->localVars[index].f

#define JEMCC_STORE_OBJECT(frame, index, val) frame->localVars[index].obj = val;
#define JEMCC_LOAD_OBJECT(frame, index) frame->localVars[index].obj

#define JEMCC_STORE_LONG(frame, index, val) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->l = val;
#define JEMCC_LOAD_LONG(frame, index) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->l

#define JEMCC_STORE_DOUBLE(frame, index, val) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->d = val;
#define JEMCC_LOAD_DOUBLE(frame, index) \
               ((JEM_DblFrameEntry *) (frame->localVars + index))->d

/* Macros for manipulating operand stack frame entries */
#define JEMCC_PUSH_STACK_INT(frame, val) \
               ((frame->operandStackTop)++)->i = val;
#define JEMCC_POP_STACK_INT(frame) \
               (--(frame->operandStackTop))->i

#define JEMCC_PUSH_STACK_FLOAT(frame, val) \
               ((frame->operandStackTop)++)->f = val;
#define JEMCC_POP_STACK_FLOAT(frame) \
               (--(frame->operandStackTop))->f

#define JEMCC_PUSH_STACK_OBJECT(frame, val) \
               ((frame->operandStackTop)++)->obj = val;
#define JEMCC_POP_STACK_OBJECT(frame) \
               (--(frame->operandStackTop))->obj

#define JEMCC_PUSH_STACK_LONG(frame, val) \
               (((JEM_DblFrameEntry *) (frame->operandStackTop))++)->l = val;
#define JEMCC_POP_STACK_LONG(frame) \
               (--((JEM_DblFrameEntry *) (frame->operandStackTop)))->l

#define JEMCC_PUSH_STACK_DOUBLE(frame, val) \
               (((JEM_DblFrameEntry *) (frame->operandStackTop))++)->d = val;
#define JEMCC_POP_STACK_DOUBLE(frame) \
               (--((JEM_DblFrameEntry *) (frame->operandStackTop)))->d


/**
 * Process the specified throwable in the context of the current environment.
 * This will search for an appropriate "catcher" of the exception, be it
 * a bytecode defined exception handler or a calling native/JEMCC method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context and in which
 *           the exception is to be thrown
 *     throwable - the Throwable instance which is to be thrown.  If NULL,
 *                 the "native" exception which is contained in the given
 *                 environment will be rethrown (pass-on mechanism).
 *
 * Exceptions:
 *     It is possible that another more critical exception (such as
 *     OutOfMemoryError) may be thrown in place of the specified throwable,
 *     if an error occurs in processing the exception.
 */
JNIEXPORT void JNICALL JEMCC_ProcessThrowable(JNIEnv *env,
                                              JEMCC_Object *throwable);

/**
 * Seek out the "previous" object which called the currently executing
 * method (associated with the current environment frame).  This previous
 * object must be different than the object instance currently associated
 * with the executing method (i.e. will skip over any method calls made with
 * 'this').  If the currently executing method is static, then this method
 * will return the most "recent" frame which has an object instance associated
 * with it.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The previous object instance which was responsible for making the
 *     call to this object instance method, or NULL if no such object
 *     could be found.
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_GetCallingObjectInstance(JNIEnv *env);

/**
 * Seek out the previous class instance which called the currently executing
 * method (associated with the current environment frame).  This previous
 * class must be different than the class associated with the currently
 * executing method (i.e. will skip over method calls made with 'this' or
 * a static class of the current class).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The previous class instance which was responsible for making the
 *     call to the currently executing method or NULL if no such class
 *     could be found (root of the excuting frame).
 */
JNIEXPORT JEMCC_Class *JNICALL JEMCC_GetCallingClassInstance(JNIEnv *env);


/************************ Class Information ****************************/

/* The following are directly from Chapter 4 of the JVM specification */
#define ACC_PUBLIC       0x0001 /* accessable outside of package/class */
#define ACC_PRIVATE      0x0002 /* method/field only available within class */
#define ACC_PROTECTED    0x0004 /* method/field only available within package */
#define ACC_STATIC       0x0008 /* method/field is static to the class */
#define ACC_FINAL        0x0010 /* no subclassing, overriding or assignment */
#define ACC_SUPER        0x0020 /* invokespecial backwards compatibility flag */
#define ACC_SYNCHRONIZED 0x0020 /* method must be monitor locked */
#define ACC_VOLATILE     0x0040 /* no caching permitted */
#define ACC_TRANSIENT    0x0080 /* no read/write from persistent object mngr */
#define ACC_NATIVE       0x0100 /* not implemented in pure java */
#define ACC_INTERFACE    0x0200 /* class data is actually an interface */
#define ACC_ABSTRACT     0x0400 /* abstract class/method - must be provided */
#define ACC_STRICT       0x0800 /* floating point is fp-strict */

/* And these are for the JVM implementation record-keeping */
#define ACC_JEMCC       0x01000 /* class/method is a jemcc implementation */
#define ACC_NATIVE_DATA 0x02000 /* jemcc class has native data element */
#define ACC_SYNTHETIC   0x04000 /* special flag for fields/methods */

#define ACC_ARRAY       0x10000 /* class is an array instance */
#define ACC_PRIMITIVE   0x20000 /* class is a primitive object class */
#define ACC_THROWABLE   0x40000 /* class is a throwable subclass */
#define ACC_STD_THROW   0x80000 /* class is a "standard" throwable subclass */

#define ACC_RESOLVE_ERROR 0x100000 /* class data element has an error */

/* Type definitions for primitives, with gap for array depth info */
#define PRIMITIVE_BOOLEAN  0x0100
#define PRIMITIVE_BYTE     0x0200
#define PRIMITIVE_CHAR     0x0300
#define PRIMITIVE_SHORT    0x0400
#define PRIMITIVE_INT      0x0500
#define PRIMITIVE_FLOAT    0x0600
#define PRIMITIVE_LONG     0x0700
#define PRIMITIVE_DOUBLE   0x0800
#define PRIMITIVE_VOID     0x0900

#define PRIMITIVE_TYPE_MASK 0x0F00

/**
 * Convenience method to obtain the "internal" name of the class.  This
 * is the raw character className, may be UTF encoded.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     class - the class instance to retrieve the name for
 *
 * Returns:
 *     The raw character representation of the fully qualified name of
 *     the specified class.  May be UTF encoded.
 */
JNIEXPORT char *JNICALL JEMCC_GetClassName(JNIEnv *env, 
                                           JEMCC_Class *class);


/****************** Environment Buffer Management *******************/

/*
 * All of the following methods are designed for "short-term" working
 * buffer management, without incurring additional overhead from allocating
 * and releasing working memory.  Each "environment" in the JEMCC VM has
 * a "static" buffer which can be utilized through the following methods.
 * There are no multi-threading issues, as each environment instance is tied
 * to a single thread only.  These buffers are also only able to grow in
 * size - while this may waste memory for occasional large buffers, it
 * optimizes the size for repetitive operations.
 *
 * For the more Java oriented, these methods are equivalent to similar
 * methods against a StringBuffer instance attached to the current 
 * environment.
 */

/**
 * Ensure that the buffer has at least the specified number of bytes
 * allocated for use.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     length - the total number of bytes which the buffer must allocate to
 *              be available for subsequent use
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
JNIEXPORT jbyte *JNICALL JEMCC_EnvEnsureBufferCapacity(JNIEnv *env,
                                                       jsize length);

/**
 * Convenience method to prepare the buffer for string append operations.
 * Ensures that the buffer has preallocated at least the given number of
 * bytes and initializes the buffer to a zero-length string.  Note that
 * this will initialize the buffer for either str (char *) or full string
 * append operations (assumes ASCII initially).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     length - the total number of bytes which the buffer must allocate to
 *              be available for subsequent use
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferInit(JNIEnv *env, jsize length);

/**
 * Append the given character string to the end of the current buffer.
 * If required, the buffer will expand to accomodate the space requirements
 * of the resulting string.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     str - the character string to append to the current buffer contents
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferAppend(JNIEnv *env, char *str);

/**
 * Append the given set of character strings to the end of the current buffer.
 * The list of append strings must be terminated by a NULL string reference.
 * If required, the buffer will expand to accomodate the space requirements
 * of the resulting string.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - the set of character strings to be appended to the buffer
 *           (NULL terminated)
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferAppendSet(JNIEnv *env, ...);

/**
 * Obtain a copy of the current contents of the string buffer.  The buffer
 * must be at least initialized before calling this method or indeterminate
 * results may occur.  Note: the returned buffer may not be '\0' terminated
 * if Unicode characters were introduced through the String appends.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     Either a pointer to the character string copy of the buffer contents
 *     or NULL if the allocation of the requested memory has failed (an
 *     OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
JNIEXPORT char *JNICALL JEMCC_EnvStrBufferDup(JNIEnv *env);

/**
 * Append the given java.lang.String to the end of the current buffer.
 * If required, the buffer will expand to accomodate the space requirements
 * of the resulting string and an ASCII buffer will be converted to Unicode
 * if the String contains such characters.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     string - the String instance to append to the current buffer contents
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL if the allocation of the requested memory has failed
 *     (an OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 */
JNIEXPORT char *JNICALL JEMCC_EnvStringBufferAppend(JNIEnv *env,
                                                    JEMCC_Object *string);

/**
 * Append the given Object to the end of the current buffer - this will make
 * a toString() call on the object.  If required, the buffer will expand to 
 * accomodate the space requirements of the resulting string and an ASCII 
 * buffer will be convereted to Unicode if the String contains such 
 * characters.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the object instance to append the string representation of
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL a memory allocation occurred (exception will be thrown
 *     in the current environment) or a toString() method call threw an
 *     exception.
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 *     Other exceptions as thrown in the toString() call.
 */
JNIEXPORT char *JNICALL JEMCC_EnvStringBufferAppendObj(JNIEnv *env,
                                                       JEMCC_Object *obj);

/**
 * Append the set of elements to the end of the current buffer.  This will
 * take a combination of char * strings and Strings/Objects.  The provided
 * list must be terminated by a NULL reference.  If required, the buffer 
 * will expand to accomodate the space requirements of the resulting string
 * and an ASCII  buffer will be converted to Unicode if the String contains
 * such characters.
 * 
 * NOTE: to properly mark Java Objects and Strings, they must be "wrapped"
 *       by the STR_OBJECT() macro, e.g.
 *
 *  JEMCC_EnvStringBufferAppendSet(env, "class=(", STR_OBJECT(clz), ")", NULL);
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     ... - the set of elements to be appended to the buffer (NULL terminated)
 *
 * Returns:
 *     Either a pointer to the beginning of the allocated buffer memory
 *     block or NULL a memory allocation occurred (exception will be thrown
 *     in the current environment) or a toString() method call threw an
 *     exception.
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the buffer failed
 *     Other exceptions as thrown in the toString() call.
 */
#define STR_OBJECT(obj) (void *) 0xdecaf, obj
JNIEXPORT char *JNICALL JEMCC_EnvStringBufferAppendSet(JNIEnv *env, ...);

/**
 * Convert the contents of the environment string buffer into a full
 * java.lang.String object (e.g. env.StringBuffer.toString()).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     Either a pointer to the String representation of the buffer contents
 *     or NULL if the allocation of the String object has failed (an
 *     OutOfMemoryError will have been thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - the memory allocation for the String failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEMCC_EnvStringBufferToString(JNIEnv *env);


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

#endif
