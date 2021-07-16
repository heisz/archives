/**
 * JEMCC system/environment functions to support dynamic library operations.
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
#include <dlfcn.h>

/* Read the structure/method details */
#include "jem.h"

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
JEM_DynaLibLoader JEM_DynaLibLoaderInit() {
    return (JEM_DynaLibLoader) 0x1;
}

/**
 * Release the dynamic library loader instance allocated by the
 * DynaLibLoaderInit method.  May release references to associated dynamic
 * library instances, depending on system capabilities.
 *
 * Parameters:
 *     loader - the allocated dynamic library loader to release
 */
void JEM_DynaLibLoaderDestroy(JEM_DynaLibLoader loader) {
}

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
JEM_DynaLib JEM_DynaLibLoad(JEM_DynaLibLoader loader, const char *libFileName,
                            void *attachment) {
    void *libEntry;

    (void) dlerror();
    libEntry = dlopen(libFileName, RTLD_GLOBAL | RTLD_LAZY);

    return (JEM_DynaLib) libEntry;
}

/**
 * Retrieve the reference to the attachment object specified in the library
 * load call (for classloader cross-reference).
 *
 * Returns:
 *     The attachment reference supplied in the DynaLibLoad method call.
 */
void *JEM_DynaLibGetAttachment(JEM_DynaLib library) {
    return NULL;
}

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
void JEM_DynaLibRelease(JEM_DynaLib library) {
}

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
JEM_DynaLibSymbol JEM_DynaLibGetSymbol(JEM_DynaLib library,
                                       const char *symName) {
    /* Clear the error prior to getting symbol to ensure correct reporting */
    (void) dlerror();
    return (JEM_DynaLibSymbol) dlsym((void *) library, (char *) symName);
}

/**
 * Obtain the message associated with the last error that occurred in the
 * dynamic library system.  Calling this method also clears the error
 * buffer (subsequent call would not show the error again).
 *
 * Returns:
 *     The text message associated with the last dynamic library access
 *     error, or NULL if no error has occurred.
 */
char *JEM_DynaLibErrorMsg() {
    return dlerror();
}

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
 */
char *JEM_MapLibraryName(JNIEnv *env, const char *libName) {
    char *retVal = (char *) JEMCC_Malloc(env, (juint) (strlen(libName) + 10));

    if (retVal == NULL) return NULL;
    (void) sprintf(retVal, "lib%s.so", libName);

    return retVal;
}
