/**
 * JEMCC system/environment functions to support thread management.
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

/* Read the structure/method details */
#include "jem.h"

#define USE_PTHREADS 1

/* System dependent monitor management inclusions */
#ifdef USE_PTHREADS
#include <pthread.h>
#include <sched.h>
#endif

/* Static thread initialization flags/data */
static jboolean threadDataInitialized = JNI_FALSE;
static pthread_key_t assocThreadValKey;

/**
 * Initialize any static thread information (keys, etc).
 *
 * Returns:
 *     JNI_OK if initialization was successful, JNI_FALSE otherwise.  Cannot
 *     throw an exception as this method is most likely called during the
 *     creation of the first VM instance.
 */
static int checkInitThread() {
    int retCode = JNI_OK;

    /* Rare, but possible to create two VM's simultaneously from two threads */
    JEMCC_EnterGlobalMonitor();

    /* Only handle this once */
    if (threadDataInitialized == JNI_FALSE) {
        threadDataInitialized = JNI_TRUE;

        /* Construct the thread value keyset */
#ifdef USE_PTHREADS
        if (pthread_key_create(&assocThreadValKey, NULL) != 0) {
            retCode = JNI_ERR; /* purecov: deadcode */
        }
#endif
    }

    /* Clean up once we are finished */
    JEMCC_ExitGlobalMonitor();

    return retCode;
}

/**
 * Carrier structure to pass environment and argument information between
 * the CreateThread method and the various thread initialization functions
 * below.
 */
typedef struct ThreadInitDataCarrier {
    JNIEnv *requestEnv;
    JEMCC_ThreadStartFunction *startFn;
    void *userArg;
    jint priority;
} ThreadInitDataCarrier;

/**
 * Launch method(s) used in the creation of a new JVM thread instance.
 * Will initialize/create a JNIEnv instance, bind it to the thread and
 * call the thread start method (with the associated userArg) as given
 * to the CreateThread() method below.
 */
#ifdef USE_PTHREADS
static void *threadInitFn(void *arg) {
    ThreadInitDataCarrier *carrier = (ThreadInitDataCarrier *) arg;
    JEM_JNIEnv *env = NULL;
    void * threadRetCode;
    JavaVM *vm;
    jint rc;

    /* First, attach an environment instance to the thread */
    /* XXX - need attachment arguments! */
    if (carrier->requestEnv != NULL) {
        vm = (JavaVM *) ((JEM_JNIEnv *) carrier->requestEnv)->parentVM;
        rc = (*vm)->AttachCurrentThread(vm, (JNIEnv **) &env, NULL);
        if (rc != JNI_OK) return (void *) rc;
    }

    /* Handle priority setting for local thread specific instances */

    /* Make the call and capture the internal return code for later return */
    threadRetCode = carrier->startFn((JNIEnv *) env, carrier->userArg);

    /* XXX - Clean up the environment associated with this thread */
    JEMCC_Free(carrier);

    return threadRetCode;
}
#endif

/**
 * Create a new native/user thread instance which will execute the specified
 * start function with the given priority.  A new JNIEnv instance will be
 * created for and attached to the new thread and will be provided to the
 * specified start function.
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
JEMCC_ThreadId JEMCC_CreateThread(JNIEnv *env,
                                  JEMCC_ThreadStartFunction startFn,
                                  void *userArg, jint priority) {
    ThreadInitDataCarrier *carrier;
    JEMCC_ThreadId retThreadId;

    /* Verify threading system initialization */
    if (checkInitThread() != JNI_OK) {
        /* We have the current environment, toss out an exception */
        /* purecov: begin inspected */
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError, NULL,
                                   "Error during threading initialization");
        return 0;
        /* purecov: end */
    }

    /* Build the carrier data to allow "in-thread" initialization */
    carrier = (ThreadInitDataCarrier *) 
                       JEMCC_Malloc(env, sizeof(ThreadInitDataCarrier));
    if (carrier == NULL) return 0;
    carrier->requestEnv = env;
    carrier->startFn = startFn;
    carrier->userArg = userArg;
    carrier->priority = priority;

    /* Create the thread */
#ifdef USE_PTHREADS
    if (pthread_create((pthread_t *) &retThreadId, NULL, 
                       threadInitFn, carrier) != 0) {
        /* purecov: begin inspected */
        JEMCC_Free(carrier);
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError, NULL,
                                   "Insufficient resources for thread create");
        retThreadId = 0;
        /* purecov: end */
    }
#endif

    return retThreadId;
}

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
jboolean JEMCC_IsSameThread(JEMCC_ThreadId threada, 
                            JEMCC_ThreadId threadb) {
#ifdef USE_PTHREADS
    return (pthread_equal((pthread_t) threada, 
                          (pthread_t) threadb) != 0) ? JNI_TRUE : JNI_FALSE;
#endif
}

/**
 * Obtain the thread identifier for the current thread.
 *
 * Returns:
 *     The thread identifier for the currently executing thread.
 */
JEMCC_ThreadId JEMCC_GetCurrentThread() {
#ifdef USE_PTHREADS
    return (JEMCC_ThreadId) pthread_self();
#endif
}

/**
 * Yield control of the current thread, allowing any threads waiting
 * for use of this processor to continue.  This method will return
 * when the thread scheduler re-activates this thread (may occur at
 * any time).
 */
void JEMCC_YieldCurrentThread() {
#ifdef USE_PTHREADS
    sched_yield();
#endif
}

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
void JEMCC_YieldCurrentThreadAndSleep(jlong nano) {
#ifdef USE_PTHREADS
    usleep(nano/1000);
#endif
}

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
void JEMCC_YieldCurrentThreadAgainstFd(int fd) {
}

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
char *JEMCC_AssociateThreadValue(void *data) {
    /* Verify threading system initialization */
    if (checkInitThread() != JNI_OK) {
        return "Error during threading initialization"; /* purecov: deadcode */
    }

#ifdef USE_PTHREADS
    if (pthread_setspecific(assocThreadValKey, data) != 0) {
        return "Unable to set thread specific data "; /* purecov: deadcode */
    }
#endif

    return NULL;
}

/**
 * Retrieve the data object previously associated with the current thread.
 * Typically used to retrieve the JNIEnv instance attached to the thread
 * instance.
 *
 * Returns:
 *     The previously defined data object associated with the given thread
 *     or NULL if the indicated thread has no associated data.
 */
void *JEMCC_RetrieveThreadValue() {
    /* Verify threading system initialization */
    if (checkInitThread() != JNI_OK) return NULL;

#ifdef USE_PTHREADS
    return pthread_getspecific(assocThreadValKey);
#endif
}
