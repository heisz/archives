/**
 * JEMCC methods to support the JNI exception interface methods.
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
#include "jnifunc.h"

/**
 * Throw the given exception in the current execution context.  Does not
 * collapse the execution frame stack (holds in pendingException) but
 * method should immediately return following this call.  Exception will
 * be processed (up to the appropriate higher-level handler) when the
 * current native method returns (frame collapses).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     obj - the java.lang.Throwable instance to be thrown
 *
 * Returns:
 *     JNI_OK if successful, one of the negative error codes on failure.
 *
 * Exceptions:
 *     Either the provided exception instance or an internal exception
 *     in the stack processing.
 */
jint JEMCC_Throw(JNIEnv *env, jthrowable obj) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;

    /* Safety check for exception overloads */
    if (jenv->pendingException != NULL) return JNI_EINVAL;

    /* Hold now, will throw on frame exit */
    jenv->pendingException = (JEMCC_Object *) obj;
    return JNI_OK;
}

/**
 * Throw an instance of the specified throwable class/msg.  Does not
 * collapse the execution frame stack (holds in pendingException) but
 * method should immediately return following this call.  Exception will
 * be processed (up to the appropriate higher-level handler) when the
 * current native method returns (frame collapses).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     clazz - the class of the throwable to be created/thrown
 *     msg - the message to include in the throwable instance
 *
 * Returns:
 *     JNI_OK if successful, one of the negative error codes on failure.
 *
 * Exceptions:
 *     Either the indicated exception instance or an internal exception
 *     related to creating/processing the exception.
 */
jint JEMCC_ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;

    /* Safety check for exception overloads */
    if (jenv->pendingException != NULL) return JNI_EINVAL;

    /* Use the standard method (assume bypass of complex constructor) */
    JEMCC_ThrowStdThrowable(env, (JEMCC_Class *) clazz, NULL, msg);

    /* No return code from above, look for captured result */
    return ((jenv->pendingException == NULL) ? JNI_ERR : JNI_OK);
}

/**
 * Method by which JNI native methods can determine what (if any) exception
 * has occurred.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     The pending exception instance if an exception occurred or NULL if
 *     no exceptions have been caught/captured.
 */
jthrowable JEMCC_ExceptionOccurred(JNIEnv *env) {
    return (jthrowable) ((JEM_JNIEnv *) env)->pendingException;
}

/* Stack trace dump method from throwable.c */
extern void JEM_Throwable_PrintStackTrace(JNIEnv *env, JEMCC_Object *throwable,
                                   jint handler(JNIEnv *env, JEMCC_Object *line,
                                                jboolean includeEOL,
                                                void *callData),
                                   void *callData);

/**
 * Dump a description/stack trace of the pending exception for debugging
 * purposes.  Does nothing if there is no pending exception.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 */
void JEMCC_ExceptionDescribe(JNIEnv *env) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;

    /* Safety check */
    if (jenv->pendingException == NULL) return;

    /* Passing NULL's to print method outputs to stderr */
    JEM_Throwable_PrintStackTrace(env, jenv->pendingException, NULL, NULL);
}

/**
 * Release/clear any pending exception in the current native context.  The
 * exception can then be ignored or handled directly by the native method.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 */
void JEMCC_ExceptionClear(JNIEnv *env) {
    /* TODO - notify GC of object release */
    ((JEM_JNIEnv *) env)->pendingException = NULL;
}

/**
 * Print the message, dump the pending exception (if applicable) and
 * terminate the execution of the virtual machine.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     msg - a message to dump to the standard error channel
 */
void JEMCC_FatalError(JNIEnv *env, const char *msg) {
    /* Print and dump */
    (void) fprintf(stderr, "Fatal VM Error: %s\n", msg);
    JEMCC_ExceptionDescribe(env);

    /* TODO - Kill the VM */
}
