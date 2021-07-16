/**
 * Supporting methods for JEMCC class packaging management.
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
#include <ctype.h>

/* This file is not compiled, but read into class.c (static method access) */

/**
 * Internal method, identical to the ClassNameHashFn method in that it hashes
 * the package name without differentiating the two separators, this method
 * also ignores the first "state marker" character in the package name
 * key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the package name key to be hashed
 *
 * Returns:
 *     The numerical hashcode for the package name, "ignoring" package
 *     separators and the first marker character.
 */
static juint JEM_PackageNameHashFn(JNIEnv *env, void *key) {
    char *ptr = (char *) key;

    if (*(ptr++) == '\0') return 0;
    return JEMCC_ClassNameHashFn(env, (void *) ptr);
}

/**
 * Internal method, identical to the ClassNameEqualsFn method in that it 
 * compares the package names without differentiating the two separators, 
 * this method also ignores the first "state marker" character in the 
 * package name keys.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya - the package name key to be compared
 *     keyb - the package name key to compare against
 *
 * Returns:
 *     JNI_TRUE if the two package names are equal ignoring separator 
 *     differences and the first character, JNI_FALSE if they are different.
 */
static jboolean JEM_PackageNameEqualsFn(JNIEnv *env, void *keya, void *keyb) {
    char *ptra = (char *) keya, *ptrb = (char *) keyb;

    if ((*(ptra++) == '\0') || (*(ptrb++) == '\0')) return JNI_FALSE;
    return JEMCC_ClassNameEqualsFn(env, (void *) ptra, (void *) ptrb);
}

/**
 * Internal structure definition for the JEMCC package initialization
 * tracker.  Partially composed of the externally visible JEMCC_PkgInitData
 * structure and internal elements used by the package management system.
 */
typedef struct JEM_PkgData {
    JEMCC_PkgInitData initData;
    JEMCC_ThreadId initThread;
} JEM_PkgData;

/**
 * Parse the library definition file for the VM included JEMCC packages.
 * Populates the preconfigured package loading and initialization structures
 * in the bootstrap classloader of the VM.  NOTE: this method is only intended
 * to be used during VM initialization and is not MT-safe.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     pkgFileData - the contents of the VM/JEMCC package/library definition
 *                   file (this string will be modified during parsing)
 *     pkgFileLen - the number of bytes contained in the file data
 *
 * Returns:
 *     JNI_OK - the package definitions were processed successfully
 *     JNI_ENOMEM - a memory allocation of the package information failed
 *                  (a memory exception has been thrown in the current
 *                  environment)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEM_ParseLibPackageInfo(JNIEnv *env, char *pkgFileData, int pkgFileLen) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;
    char *ptr, *pkgKey, *lineStart, *lineEnd, *libPtr;
    JEM_PkgData *pkgData;
    jint rc;

    /* Scan the provided file contents line by line */
    ptr = pkgFileData;
    while (pkgFileLen > 0) {
        /* Parse/read a line from the configuration contents */
        lineStart = ptr;
        while (pkgFileLen > 0) {
            if (*ptr == '#') *ptr = '\0';
            if (*ptr == JEMCC_LineSeparator) {
                *(ptr++) = '\0';
                pkgFileLen--;
                break;
            }
            ptr++;
            pkgFileLen--;
        }

        /* Trim the line contents, dumping empty lines */
        while (isspace(*lineStart)) lineStart++;
        lineEnd = lineStart + strlen(lineStart) - 1;
        while ((lineEnd >= lineStart) && isspace(*lineEnd)) *(lineEnd--) = '\0';
        if (strlen(lineStart) == 0) continue;

        /* Split out the package and library names */
        libPtr = lineStart;
        while ((*libPtr != '\0') && (!isspace(*libPtr))) libPtr++;
        if (*libPtr == '\0') continue;
        *(libPtr++) = '\0';
        libPtr = lineEnd;
        while ((*libPtr != '\0') && (!isspace(*libPtr))) libPtr--;
        libPtr++;
        pkgKey = (char *) JEMCC_StrCatFn(env, "+", lineStart, (char *) NULL);
        if (pkgKey == NULL) {
            return JNI_ENOMEM;
        }

        /* Build the initialization information carrier */
        pkgData = (JEM_PkgData *) JEMCC_Malloc(env, sizeof(JEM_PkgData));
        if (pkgData == NULL) {
            JEMCC_Free(pkgKey);
            return JNI_ENOMEM;
        }
        pkgData->initData.initState = PKGLOAD_LIB_NAME;
        pkgData->initData.handler.pkgLibName = 
                             (char *) JEMCC_StrDupFn(env, (void *) libPtr);
        if (pkgData->initData.handler.pkgLibName == NULL) {
            JEMCC_Free(pkgData);
            JEMCC_Free(pkgKey);
            return JNI_ENOMEM;
        }

        /* Define the package library name - nullified on load attempt */
        /* NOTE: this is only called during VM create, so no MT conflicts */
        rc = JEMCC_HashInsertEntry(env, &(jvm->jemccClassPackageTable), 
                                   (void *) pkgKey, (void *) pkgData, 
                                   NULL, NULL,
                                   JEM_PackageNameHashFn, 
                                   JEM_PackageNameEqualsFn);
        if (rc == JNI_ENOMEM) {
            JEMCC_Free(pkgData->initData.handler.pkgLibName);
            JEMCC_Free(pkgData);
            JEMCC_Free(pkgKey);
            return JNI_ENOMEM;
        }
        if (rc != JNI_OK) {
            /* Redefinition of the package - don't override it */
            JEMCC_Free(pkgData->initData.handler.pkgLibName);
            JEMCC_Free(pkgData);
            JEMCC_Free(pkgKey);
        }
    }

    return JNI_OK;
}

/**
 * Internal method to perform definition of package initialization or
 * classloader function (very common definition).  See the two following
 * methods for parameter and return/exception information.
 */
static jint JEM_RegisterPkgFn(JNIEnv *env, JEMCC_Object *loader,
                              const char *pkgName, JEMCC_PackageInitFn initFn,
                              JEMCC_PackageClassLdrFn ldrFn) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    JEM_PkgData *pkgData, *origPkgData;
    char *pkgKey, *origPkgKey;
    jint rc;

    /* First off, construct the registration information */
    pkgKey = JEMCC_StrCatFn(env, "-", pkgName, (char *) NULL);
    if (pkgKey == NULL) return JNI_ENOMEM;

    pkgData = (JEM_PkgData *) JEMCC_Malloc(env, sizeof(JEM_PkgData));
    if (pkgData == NULL) {
        JEMCC_Free(pkgKey);
        return JNI_ENOMEM;
    }
    if (initFn != NULL) {
        pkgData->initData.initState = PKGLOAD_INIT_FN;
        pkgData->initData.handler.pkgInitFn = initFn;
    } else {
        pkgData->initData.initState = PKGLOAD_CLASSLDR_FN;
        pkgData->initData.handler.pkgClassLdrFn = ldrFn;
    }

    /* If loader is specified, work with the loader namespace */
    if (loader != NULL) {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
    }

    /* Prevent multithread conflicts on the hashtable */
    if (JEM_LockClassLoader(env, loader) != JNI_OK) return JNI_ENOMEM;

    /* Push the package definition */
    rc = JEMCC_HashInsertEntry(env, hash, 
                               (void *) pkgKey, (void *) pkgData, 
                               (void **) &origPkgKey, (void **) &origPkgData, 
                               JEM_PackageNameHashFn, JEM_PackageNameEqualsFn);

    /* Release the lock now that we have worked with the hashtable */
    JEM_UnlockClassLoader(env, loader);

    /* Handle the return information */
    if (rc != JNI_OK) {
        JEMCC_Free(pkgData);
        JEMCC_Free(pkgKey);
    }

    return rc;
}

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
jint JEMCC_RegisterPkgInitFn(JNIEnv *env, JEMCC_Object *loader,
                             const char *pkgName, JEMCC_PackageInitFn initFn) {
    return JEM_RegisterPkgFn(env, loader, pkgName, initFn, NULL);
}

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
jint JEMCC_RegisterPkgClassLdrFn(JNIEnv *env, JEMCC_Object *loader,
                                 const char *pkgName, 
                                 JEMCC_PackageClassLdrFn ldrFn) {
    return JEM_RegisterPkgFn(env, loader, pkgName, NULL, ldrFn);
}

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
jint JEMCC_RegisterMultiPkgInitFn(JNIEnv *env, JEMCC_Object *loader,
                                  JEMCC_MultiPackageInitData *initData,
                                  jsize initCount) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_JavaVM *jvm = (JEM_JavaVM *) jenv->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    JEMCC_ThreadId currThreadId = jenv->envThread;
    JEM_PkgData *pkgData, *origPkgData;
    char *pkgKey, *origPkgKey;
    jint i, rc;


    /* If loader is specified, work with the loader namespace */
    if (loader != NULL) {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
    }

    /* Prevent multithread conflicts on the hashtable */
    if (currThreadId == 0) currThreadId = JEMCC_GetCurrentThread();
    if (JEM_LockClassLoader(env, loader) != JNI_OK) return JNI_ENOMEM;

    /* Repeat for all of the initialization definitions */
    for (i = 0; i < initCount; i++) {
        /* First off, construct the registration information */
        pkgKey = JEMCC_StrCatFn(env, "-", initData[i].pkgName, (char *) NULL);
        if (pkgKey == NULL) {
            JEM_UnlockClassLoader(env, loader);
            return JNI_ENOMEM;
        }

        pkgData = (JEM_PkgData *) JEMCC_Malloc(env, sizeof(JEM_PkgData));
        if (pkgData == NULL) {
            JEMCC_Free(pkgKey);
            JEM_UnlockClassLoader(env, loader);
            return JNI_ENOMEM;
        }
        if (initData[i].initFn != NULL) {
            pkgData->initData.initState = PKGLOAD_INIT_FN;
            pkgData->initData.handler.pkgInitFn = initData[i].initFn;
        } else if (initData[i].loaderFn != NULL) {
            pkgData->initData.initState = PKGLOAD_CLASSLDR_FN;
            pkgData->initData.handler.pkgClassLdrFn = initData[i].loaderFn;
        } else {
            *pkgKey = '@';
            pkgData->initData.initState = PKGLOAD_COMPLETED;
        }

        /* Push the package definition */
        rc = JEMCC_HashInsertEntry(env, hash, 
                               (void *) pkgKey, (void *) pkgData,
                               (void **) &origPkgKey, (void **) &origPkgData,
                               JEM_PackageNameHashFn, JEM_PackageNameEqualsFn);

        /* Handle replacement instances */
        /* Note that we can modify the indicator character safely */
        if (rc == JNI_ERR) {
            if (*origPkgKey == '+') {
                /* Replacement of library load information */
                JEMCC_Free(origPkgData->initData.handler.pkgLibName);
                rc = JNI_OK;
            } else if (*origPkgKey == '-') {
                /* Replacement of previously defined (static) initializer */
                rc = JNI_OK;
            } else if (((*origPkgKey == '?') || (*origPkgKey == '!')) && 
                          (((JEM_PkgData *) origPkgData)->initThread == 
                                                               currThreadId)) {
                /* Update of currently initializing package */
                *pkgKey = *origPkgKey;
                rc = JNI_OK;
            }

            /* Perform the replacement where appropriate */
            if (rc == JNI_OK) {
                *origPkgKey = *pkgKey;
                *((JEMCC_PkgInitData *) origPkgData) =
                                             *((JEMCC_PkgInitData *) pkgData);
                JEMCC_Free(pkgData);
                JEMCC_Free(pkgKey);
            }
        } else if (rc == JNI_ENOMEM) {
            /* Memory failure, cleanup and return */
            JEMCC_Free(pkgData);
            JEMCC_Free(pkgKey);
            JEM_UnlockClassLoader(env, loader);
            return rc;
        }
    }
    
    /* All finished, release and return success */
    JEM_UnlockClassLoader(env, loader);
    return JNI_OK;
}

/* Return codes from the JEMCC package/class loading mechanism */
#define PKG_INIT_ERROR 1
#define PKG_MEM_ERROR 2
#define PKG_NOT_FOUND 3
#define PKG_COMPLETE 4
#define PKG_TRY_AGAIN 5
#define PKG_CLASS_ERROR 6
#define PKG_CLASS_NOT_FOUND 7
#define PKG_CLASS_OK 8

/**
 * Common method for calling a package interface functions (initializers and
 * loaders).  Wraps the proper sequences for package initialization and manages
 * the package state indicators (both key characters and state values).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader in which the package initialization or class
 *              loading is to take place, NULL if the VM bootstrap loader is 
 *              to be used
 *     entryKey - the hashtable key associated with the package initialization
 *                data (used to set the key character state indicator)
 *     pkgData - the package initialization information
 *     className - the name of a class to be loaded, with packaging 
 *                 information removed.  Used only where the package is
 *                 associated with an internal package classloader
 *     classInst - if the className is found via an internal package 
 *                 classloader, the instance is returned through this reference
 *     exitTypeCh - the next state indicator of the initialization data, based
 *                  on actions taken within
 *
 * Returns:
 *     The package load status return indicator (see the comments on the
 *     LoadPackageClass method for more complete information on the return
 *     status enumeration).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed;  this is always
 *                        thrown, but does not preclude subsequent package
 *                        initialization (in case of recovery)
 *     ClassFormatError - JEMCC initialization data is invalid and the
 *                        specified package initialization or class load has 
 *                        been terminated
 *     LinkageError - a JEMCC reference to an external class or class 
 *                    method/field was invalid or not found (during class load)
 */
static jint JEM_CallPackageInterface(JNIEnv *env, JEMCC_Object *loader,
                                     char *entryKey, JEMCC_PkgInitData *pkgData,
                                     char *className, JEMCC_Class **classInst,
                                     char *exitTypeCh) {
    jint rc = 0, pkgInitStatus = PKG_INIT_ERROR;
    JEMCC_PackageInitFn pkgInitFn;
    JEMCC_Class *clInst = NULL;

    /* Switch modes based on initialization function type */
    if (pkgData->initState == PKGLOAD_INIT_FN) {
        /* Call the bulk initializer and handle the result */
        pkgInitFn = pkgData->handler.pkgInitFn;
        rc = (*(pkgInitFn))(env, loader, (entryKey + 1), pkgData);
        if (rc != JNI_OK) {
            /* Initialization routine will have thrown exception */
            if (rc == JNI_ENOMEM) {
                /* Memory error during initialization is recoverable */
                *exitTypeCh = '-';
                pkgInitStatus = PKG_MEM_ERROR;
            } else {
                /* Other errors are not recoverable */
                *exitTypeCh = '@';
                pkgData->initState = PKGLOAD_COMPLETED;
                pkgData->handler.pkgLibName = NULL;
                pkgInitStatus = PKG_INIT_ERROR;
            }
        } else {
            /* Handle the case where the init function is lazy (avoid loop) */
            if (pkgInitFn == pkgData->handler.pkgInitFn) {
                pkgData->initState = PKGLOAD_COMPLETED;
            }

            /* Mark package state according to initializer state */
            if (pkgData->initState != PKGLOAD_COMPLETED) {
                *exitTypeCh = '-';
            } else {
                *exitTypeCh = '@';
                pkgData->handler.pkgLibName = NULL;
            }

            /* In either case, our class may have loaded */
            pkgInitStatus = PKG_TRY_AGAIN;
        }
    } else if (pkgData->initState == PKGLOAD_CLASSLDR_FN) {
        /* Note that a local class reference is used due to external call */
        rc = (*(pkgData->handler.pkgClassLdrFn))(env, loader, 
                                                 className, &clInst);
        /* Handle the various class results */
        if (rc == JNI_OK) {
            *classInst = clInst;
            if (clInst == NULL) {
               JEMCC_ThrowStdThrowableIdxV(env, JEMCC_Class_LinkageError, NULL,
                                           "JEMCC classloader missing ",
                                           className, NULL);
                pkgInitStatus = PKG_CLASS_ERROR;
            } else {
                pkgInitStatus = PKG_CLASS_OK;
            }
        } else if (rc == JNI_ENOMEM) {
            *classInst = NULL;
            pkgInitStatus = PKG_MEM_ERROR;
        } else if (rc == JNI_ERR) {
            *classInst = NULL;
            pkgInitStatus = PKG_CLASS_ERROR;
        } else {
            *classInst = NULL;
            pkgInitStatus = PKG_CLASS_NOT_FOUND;
        }

        /* In any case, the classloader persists forever */
        *exitTypeCh = '-';
    } else {
        /* Inconsistent state reached, destroy the package */
        *exitTypeCh = '@';
        pkgData->initState = PKGLOAD_COMPLETED;
        pkgData->handler.pkgLibName = NULL;
        pkgInitStatus = PKG_INIT_ERROR;
    }

    return pkgInitStatus;
}

/* Prototype definition from the JNI library specification */
typedef jint (*JNI_OnLoad)(JavaVM *vm, void *reserved);

/**
 * Core method used for the management of JEMCC class packaging.  Supports
 * group package initializers, as well as internal package classloaders. This
 * method is called repeatedly by the LocateClassByPackage method below, as
 * some class requests may require multiple levels of package initialization,
 * (nested child packages).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader in which the package initialization or class
 *              loading is to take place.  If NULL, the VM bootstrap loader
 *              is to be used.  This loader is used to contain the package
 *              initialization information as well as the target class instance
 *     pkgKey - the fully qualified name of the class to be loaded, with
 *              an extra leading and ignored first character (for hashing).
 *              Passed rather than generated for the sake of efficiency in
 *              recursive initialization
 *     rootClassName - the name of the class to be loaded, with packaging 
 *                     information removed.  Used only where the package is
 *                     associated with an internal package classloader
 *     classInst - if the className is found via an internal package
 *                 classloader, the instance is returned through this reference
 *
 * Returns:
 *     PKG_INIT_ERROR - a package-fatal format error has occurred.  Package
 *                      initialization cannot be recovered and format
 *                      exception has been thrown in current environment.
 *     PKG_MEM_ERROR - a memory error has occurred in the package initializer.
 *                     Package is still valid (but incomplete) and a memory
 *                     exception has been thrown in current environment.
 *     PKG_NOT_FOUND - no record has been found for the indicated package in
 *                     the classloader packaging lists
 *     PKG_COMPLETE - there is no further package initialization available.
 *                    No further class instances can arise from this
 *                    package initialization definition.
 *     PKG_TRY_AGAIN - some package initialization has occurred and the class
 *                     may have been defined.  Look for the class and try this
 *                     method again if it is not found.
 *     PKG_CLASS_ERROR - an internal package classloader is in scope for this 
 *                       package and an error (format or memory) occurred 
 *                       while loading the class.  The exception has been 
 *                       thrown in the current environment.  This may be 
 *                       recoverable depending on the error condition, but 
 *                       a retry will always be valid.
 *     PKG_CLASS_NOT_FOUND - an internal package classloader is in scope for 
 *                           this package and the class was not found.  No 
 *                           exception has been thrown (will be thrown by the 
 *                           higher level method, if necessary).
 *     PKG_CLASS_OK - an internal package classloader is in scope for this 
 *                    package and the class for the specified name was 
 *                    defined (and has been returned through the classInst 
 *                    pointer).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed;  this is always
 *                        thrown, but does not preclude subsequent package
 *                        initialization (in case of recovery)
 *     ClassFormatError - JEMCC initialization data is invalid and the
 *                        specified class or package initialization has been
 *                        terminated
 *     LinkageError - a JEMCC reference to an external class or class 
 *                    method/field was invalid or not found
 *     UnsatisfiedLinkError - a referenced package library was found but
 *                            could not be loaded/linked into the application
 */
static jint JEM_LoadPackageClass(JNIEnv *env, JEMCC_Object *loader,
                                 char *pkgKey, char *rootClassName,
                                 JEMCC_Class **classInst) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_JavaVM *jvm = (JEM_JavaVM *) jenv->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    JEMCC_ThreadId currThreadId = jenv->envThread;
    JEM_DynaLib libHandle;
    JEM_DynaLibSymbol sym, pkgLibInitSym;
    int rc, pkgInitStatus = PKG_COMPLETE;
    char typeCh, exitTypeCh, *entryKey;
    char *libFileName, *pkgLibName, *pkgLibInitName;
    void *entryObj;
    JEMCC_PkgInitData *pkgData;

    /* If loader is specified, work with the loader namespace */
    if (currThreadId == 0) currThreadId = JEMCC_GetCurrentThread();
    if (loader != NULL) {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
    }

    /* Prevent multithread conflicts on the hashtable */
    if (JEM_LockClassLoader(env, loader) != JNI_OK) return PKG_MEM_ERROR;

    /* Attempt to locate package definition information */
    if (JEMCC_HashGetFullEntry(env, hash, pkgKey,
                               (void **) &entryKey, &entryObj,
                               JEM_PackageNameHashFn, 
                               JEM_PackageNameEqualsFn) != JNI_OK) {
        /* No package available, try elsewhere */
        JEM_UnlockClassLoader(env, loader);
        return PKG_NOT_FOUND;
    }

    /* Ensure that package initialization is only attempted once */
    typeCh = *entryKey;
    switch (typeCh) {
        case '!':
            if (((JEM_PkgData *) entryObj)->initThread != currThreadId) {
                JEM_WaitOnClassLoader(env, loader);
                JEM_UnlockClassLoader(env, loader);
                return PKG_TRY_AGAIN;
            }
            break;
        case '?':
            if (((JEM_PkgData *) entryObj)->initThread != currThreadId) {
                /* Mark threads waiting on loader and do so */
                *entryKey = '!';
                JEM_WaitOnClassLoader(env, loader);
                JEM_UnlockClassLoader(env, loader);
                return PKG_TRY_AGAIN;
            }
            break;
        case '@':
            /* Package has already been processed, nothing further to do */
            JEM_UnlockClassLoader(env, loader);
            return PKG_COMPLETE;
        default:
            /* All other states, take control of package initialization */
            *entryKey = '?';
            ((JEM_PkgData *) entryObj)->initThread = currThreadId;
            break;
    }
    exitTypeCh = '@';

    /* Release the lock now that we have control of the tag */
    JEM_UnlockClassLoader(env, loader);

    /* Handle load/initialize operation based on package leader character */
    pkgData = (JEMCC_PkgInitData *) entryObj;
    switch (typeCh) {
        case '+':
            /* Library name - load and initialize library package */
            pkgLibName = pkgData->handler.pkgLibName;
            libFileName = JEM_MapLibraryName(env, pkgLibName);
            if (libFileName == NULL) {
                /* Restore state, as next loader may have more luck */
                exitTypeCh = '+';
                pkgInitStatus = PKG_MEM_ERROR;
            } else {
                libHandle = NULL;
                rc = JEM_LoadPathLibrary(env, jvm->libLoader, &(jvm->libPath),
                                         libFileName, NULL, &libHandle);
                JEMCC_Free(libFileName);
                if (rc == JNI_ENOMEM) {
                    /* Restore state, as next loader may have more luck */
                    exitTypeCh = '+';
                    pkgInitStatus = PKG_MEM_ERROR;
                    break;
                }
                if ((rc != JNI_OK) || (libHandle == NULL)) {
                    /* Library was found but not linked (exception thrown) */
                    exitTypeCh = '@';
                    pkgData->initState = PKGLOAD_COMPLETED;
                    pkgData->handler.pkgLibName = NULL;
                    JEMCC_Free(pkgLibName);
                    pkgInitStatus = PKG_INIT_ERROR;
                    break;
                }

                /* Library is loaded, try to obtain/run init methods */
                /* Build library init name first as memory may be low */
                if (JEMCC_EnvStrBufferInit(env, 256) == NULL) {
                    /* Restore state, as next loader may have more luck */
                    exitTypeCh = '+';
                    pkgInitStatus = PKG_MEM_ERROR;
                    break;
                }
                pkgLibInitName = JEMCC_EnvStrBufferAppendSet(env, "JEMCC_", 
                                                             pkgLibName,
                                                             "_Init",
                                                             (char *) NULL);
                if (pkgLibInitName == NULL) {
                    /* Restore state, as next loader may have more luck */
                    exitTypeCh = '+';
                    pkgInitStatus = PKG_MEM_ERROR;
                    break;
                }

                /* Grab the symbol in case init functions alter buffer */
                pkgLibInitSym = JEM_DynaLibGetSymbol(libHandle,
                                                     pkgLibInitName);

                /* Look for the JNI load hook */
                sym = JEM_DynaLibGetSymbol(libHandle, "JNI_OnLoad");
                if (sym != NULL) {
                    rc = (*((JNI_OnLoad) sym))((JavaVM *) jvm, NULL);
                    if (rc != JNI_VERSION_1_1) {
                        /* According to JNI, library can't load */
                        /* TODO - capture existing exception??? */
                        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_UnsatisfiedLinkError, NULL,
                                   "JEMCC package init: JNI_OnLoad failure");

                        /* It's not going to work again */
                        exitTypeCh = '@';
                        pkgData->initState = PKGLOAD_COMPLETED;
                        pkgData->handler.pkgLibName = NULL;
                        JEMCC_Free(pkgLibName);
                        pkgInitStatus = PKG_INIT_ERROR;
                        break;
                    }

                    /* This init will not alter package init status */
                }

                /* Look for the common JEMCC initialization method */
                sym = JEM_DynaLibGetSymbol(libHandle, "JEMCC_Init");
                if (sym != NULL) {
                    /* Set up init function in package data structure */
                    pkgData->initState = PKGLOAD_INIT_FN;
                    pkgData->handler.pkgInitFn = (JEMCC_PackageInitFn) sym;
                    JEMCC_Free(pkgLibName);

                    /* Call the common initializer handler */
                    pkgInitStatus = JEM_CallPackageInterface(env, loader,
                                                             entryKey, pkgData,
                                                             rootClassName,
                                                             classInst,
                                                             &exitTypeCh);

                    /* Only one of the JEMCC initializers may be called */
                    break;
                }

                /* Finally, try the library specific initializer */
                if (pkgLibInitSym != NULL) {
                    /* Set up init function in package data structure */
                    pkgData->initState = PKGLOAD_INIT_FN;
                    pkgData->handler.pkgInitFn = 
                                       (JEMCC_PackageInitFn) pkgLibInitSym;
                    JEMCC_Free(pkgLibName);

                    /* Call the common initializer handler */
                    pkgInitStatus = JEM_CallPackageInterface(env, loader,
                                                             entryKey, pkgData,
                                                             rootClassName,
                                                             classInst,
                                                             &exitTypeCh);

                    /* Did get an initializer at the last try */
                    break;
                }

                /* Got this far, no JEMCC initializer was found */
                JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_UnsatisfiedLinkError, NULL,
                                   "Package library has no JEMCC initializer");
                exitTypeCh = '@';
                pkgData->initState = PKGLOAD_COMPLETED;
                pkgData->handler.pkgLibName = NULL;
                JEMCC_Free(pkgLibName);
                pkgInitStatus = PKG_INIT_ERROR;
            }
            break;
        case '-':
            /* Already defined initializer, handle specific init mode */
            pkgInitStatus = JEM_CallPackageInterface(env, loader,
                                                     entryKey, pkgData,
                                                     rootClassName,
                                                     classInst, &exitTypeCh);
            break;
        case '?':
        case '!':
            /* Recursive call is only allowed for specific classdefiner */
            if (pkgData->initState == PKGLOAD_CLASSLDR_FN) {
                pkgInitStatus = JEM_CallPackageInterface(env, loader,
                                                         entryKey, pkgData,
                                                         rootClassName,
                                                         classInst, 
                                                         &exitTypeCh);
            } else {
                /* This class initializer is invalid */
                exitTypeCh = '@';
                pkgData->initState = PKGLOAD_COMPLETED;
                pkgData->handler.pkgLibName = NULL;
                pkgInitStatus = PKG_INIT_ERROR;
            }
            break;
        case '@':
            /* Package has already been processed, nothing further to do */
            pkgInitStatus = PKG_COMPLETE;
            break;
        default:
            abort();
    }

    /* Update exit state and notify any threads waiting on classloader */
    if (JEM_LockClassLoader(env, loader) != JNI_OK) {
        /* Note that the classloader waits timeout to handle this case */
        *entryKey = exitTypeCh;
        return PKG_MEM_ERROR;
    }
    if (*entryKey == '!') {
        JEM_NotifyWaitingOnClassLoader(env, loader);
    }
    *entryKey = exitTypeCh;
    JEM_UnlockClassLoader(env, loader);

    return pkgInitStatus;
}

/**
 * Internal method to handle the JEMCC class loading by package.  This
 * method is essentially a wrapper to multiple calls to the LoadPackageClass 
 * method, in order to handle situations where multiple bulk initializers
 * need to be called to obtain the required class.  Note that this method may 
 * throw format exceptions but does not throw the NoClassDefFound exception 
 * (handled by a higher level caller).
 *
 * Note: this method is externally exposed for testing purposes only!
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader from which the class is to be obtained
 *              (and the package initializer is to operate).  If NULL,
 *              the class is loaded in the VM boostrap classloader
 *     className - the fully qualified name of the class to be loaded, with
 *                 either of the '.' or '/' package separators supported
 *     classInst - an instance reference by which the package initialized
 *                 or loaded class is returned
 *
 * Returns:
 *     JNI_OK - no errors occurred during package initialization - the 
 *              class has been loaded successfully if the classInst value 
 *              is non-NULL, otherwise the initializer for this package
 *              cannot provide the requested class
 *     JNI_ERR - a format or linkage error has occurred during package
 *               initialization - the class instance is not valid and an
 *               exception has been thrown in the current environment
 *     JNI_ENOMEM - a memory allocation has failed during initialization - the
 *                  class instance is not valid and an exception has been 
 *                  thrown in the current environment
 *
 * Exceptions (in all cases, the class may or may not have been loaded):
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - some JEMCC initialization data was invalid
 *     LinkageError - a JEMCC reference to an external class or class
 *                    method/field was invalid or not found
 */
jint JEM_LocateClassByPackage(JNIEnv *env, JEMCC_Object *loader,
                              const char *className, JEMCC_Class **classInst) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_HashTable *hash = &(jvm->jemccClassPackageTable);
    const char *ptr, *lastPtr = NULL;
    char *rootClassName, *pkgKey;
    int rc, pkgNameLen = 0;
    JEMCC_Class *clInst;

    /* First, split the package name from the root classname */
    ptr = className;
    while (*ptr != '\0') {
        if ((*ptr == '.') || (*ptr == '/')) lastPtr = ptr;
        ptr++;
    }
    if (lastPtr != NULL) pkgNameLen = (int) (lastPtr - className);
    if (pkgNameLen == 0) {
        *classInst = NULL;
        return JNI_OK;
    }
    pkgKey = (char *) JEMCC_Malloc(env, pkgNameLen + 2);
    if (pkgKey == NULL) {
        *classInst = NULL;
        return JNI_ENOMEM;
    }
    *pkgKey = ' ';
    (void) strncpy(pkgKey + 1, className, pkgNameLen);
    pkgKey[pkgNameLen + 1] = '\0';
    rootClassName = (char *) (lastPtr + 1);

    /* If loader is specified, work with the loader namespace */
    if (loader != NULL) {
        hash = (JEMCC_HashTable *) &(((JEMCC_ObjectExt *) loader)->objectData);
    }

    /* This process will repeat for multiple initializers */
    while (1) {
        /* Call the nested method which manages the package initializers */
        rc = JEM_LoadPackageClass(env, loader, pkgKey, rootClassName,
                                  classInst);

        /* Handle the various packaging results */
        if ((rc == PKG_NOT_FOUND) || (rc == PKG_COMPLETE) || 
                                             (rc == PKG_CLASS_NOT_FOUND)) {
            /* No error, but no class defined either */
            JEMCC_Free(pkgKey);
            *classInst = NULL;
            return JNI_OK;
        }
        if (rc == PKG_CLASS_OK) {
            /* Class has been loaded and is already in the return reference */
            JEMCC_Free(pkgKey);
            return JNI_OK;
        }
        if ((rc == PKG_INIT_ERROR) || (rc == PKG_CLASS_ERROR)) {
            /* Fatal error during package initialization, exit from mainloop */
            break;
        }
        if (rc == PKG_MEM_ERROR) {
            /* Memory failure during initialization, pass it on */
            JEMCC_Free(pkgKey);
            return JNI_ENOMEM;
        }

        /* All that is left is try again - see if the class is there yet */
        rc = JEM_LockClassLoader(env, loader);
        if (rc != JNI_OK) {
            JEMCC_Free(pkgKey);
            return rc;
        }
        clInst = (JEMCC_Class *) JEMCC_HashGetEntry(env, hash, 
                                                    (void *) className,
                                                    JEMCC_ClassNameHashFn, 
                                                    JEMCC_ClassNameEqualsFn);
        JEM_UnlockClassLoader(env, loader);
        if (clInst != NULL) {
            JEMCC_Free(pkgKey);
            *classInst = clInst;
            return JNI_OK;
        }
    }

    JEMCC_Free(pkgKey);
    return JNI_ERR;
}
