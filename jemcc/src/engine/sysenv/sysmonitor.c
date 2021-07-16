/**
 * JEMCC system/environment functions to support system (kernel) monitors.
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
#include <sys/time.h>
#include <errno.h>

/* Read the structure/method details */
#include "jem.h"

#define USE_PTHREADS 1 

/* System dependent thread management inclusions */
#ifdef USE_PTHREADS
#include <pthread.h>
#endif

/* Definition of the opaque system monitor definition */
typedef struct JEMCC_SysMonitorData {
    /* How many monitor enter conditions have occurred */
    jint reentryCount;

    /* Thread instance which currently owns this monitor */
    JEMCC_ThreadId owner;

    /* System dependent mutex/condition variables to support monitor */
#ifdef USE_PTHREADS
    pthread_cond_t condition;
    pthread_mutex_t mutex;
#endif
} JEMCC_SysMonitorData;

static JEMCC_SysMonitor *globalMonitor = NULL;

/**
 * Obtain exclusive use/lock of the global monitor.  Use for control of
 * library wide resources.
 *
 * Returns:
 *     JNI_OK if the global monitor was allocated and entered or JNI_ERR
 *     if there was a failure.  No exception can be thrown as this method
 *     will be called during VM initialization.
 */
jint JEMCC_EnterGlobalMonitor() {
    /* Yes, several could be made if multiple VM's created simultaneously. */
    /* No, there isn't anything to be done about it. */
    if (globalMonitor == NULL) {
        globalMonitor = JEMCC_CreateSysMonitor(NULL);
        if (globalMonitor == NULL) return JNI_ERR;
    }
    JEMCC_EnterSysMonitor(globalMonitor);

    return JNI_OK;
}

/**
 * Release the lock on the global monitor, allowing other threads to access
 * global information.
 */
void JEMCC_ExitGlobalMonitor() {
    if (globalMonitor != NULL) JEMCC_ExitSysMonitor(globalMonitor);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
JEMCC_SysMonitor *JEMCC_CreateSysMonitor(JNIEnv *env) {
    /* Allocate the monitor space */
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) JEMCC_Malloc(env, 
                                  (juint) sizeof(struct JEMCC_SysMonitorData));
    if (monData == NULL) return NULL;

    /* Initialize attachment information */
    monData->reentryCount = 0;
    monData->owner = 0;

    /* Initialize monitor condition/mutex information */
#ifdef USE_PTHREADS
    (void) pthread_cond_init(&(monData->condition), NULL);
    (void) pthread_mutex_init(&(monData->mutex), NULL);
#endif

    return (JEMCC_SysMonitor *) monData;
}

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
void JEMCC_DestroySysMonitor(JEMCC_SysMonitor *mon) {
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) mon;

    /* Destroy the condition/mutex variables */
#ifdef USE_PTHREADS
    if (pthread_cond_destroy(&(monData->condition))) {
        abort(); /* purecov: deadcode */
    }
    if (pthread_mutex_destroy(&(monData->mutex))) {
        abort(); /* purecov: deadcode */
    }
#endif

    /* Destroy the record structure */
    JEMCC_Free(monData);
}

/**
 * Enter a system monitor, usually to gain access to a piece of
 * thread sensitive code.  Only one thread at a time can be
 * "within" a particular monitor instance.  The system monitors track
 * multiple entries to the same monitor, so it is safe for a thread
 * to re-enter a monitor which it is already within (however, there must
 * be an exit for every enter).  This call will block if another thread
 * is within the monitor, until said thread exits and this thread is
 * able to enter.
 *
 * Parameters:
 *     mon - the monitor instance to be entered
 *
 * Exceptions:
 *     None are thrown in the current environment, but the program
 *     will abort if the mutex lock fails (should never happen).
 */
void JEMCC_EnterSysMonitor(JEMCC_SysMonitor *mon) {
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) mon;
    JEMCC_ThreadId currentThreadId = JEMCC_GetCurrentThread();

    /* If this thread owns the monitor, just increment the count */
    if (JEMCC_IsSameThread(monData->owner, currentThreadId)) {
        monData->reentryCount++;
        return;
    }

    /* Grab the monitor mutex */
#ifdef USE_PTHREADS
    if (pthread_mutex_lock(&monData->mutex)) {
        abort(); /* purecov:deadcode */
    }
#endif

    monData->owner = currentThreadId;
    monData->reentryCount = 0;
}

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
jint JEMCC_ExitSysMonitor(JEMCC_SysMonitor *mon) {
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) mon;
    JEMCC_ThreadId currentThreadId = JEMCC_GetCurrentThread();

    /* Cannot exit the thread if the current thread does not own it */
    if (!JEMCC_IsSameThread(monData->owner, currentThreadId)) {
        return JEMCC_MONITOR_NOT_OWNER;
    }

    /* Of course, maybe there have been a number of enters */
    if (monData->reentryCount > 0) {
        monData->reentryCount--;
        return JEMCC_MONITOR_OK;
    }

    /* Enter/exit count matches, release the lock */
    monData->owner = 0;
#ifdef USE_PTHREADS
    if (pthread_mutex_unlock(&monData->mutex)) {
        abort(); /* purecov: deadcode */
    }
#endif

    return JEMCC_MONITOR_OK;
}

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
jint JEMCC_SysMonitorWait(JEMCC_SysMonitor *mon) {
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) mon;
    JEMCC_ThreadId currentThreadId = JEMCC_GetCurrentThread();
    int lastReentryCount = monData->reentryCount;

    /* Cannot exit the thread if the current thread does not own it */
    if (!JEMCC_IsSameThread(monData->owner, currentThreadId)) {
        return JEMCC_MONITOR_NOT_OWNER;
    }

    /* "Release" the monitor temporarily to allow external wait access */
    monData->reentryCount = 0;
    monData->owner = 0;

    /* Drop into the wait condition */
#ifdef USE_PTHREADS
    (void) pthread_cond_wait(&monData->condition, &monData->mutex);
#endif

    /* We have been notified and have the monitor - restore tracking data */
    monData->reentryCount = lastReentryCount;
    monData->owner = currentThreadId;

    return JEMCC_MONITOR_OK;
}

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
 *                      notified this thread to restart
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 *     JEMCC_MONITOR_TIMEOUT - the wait was successful and the specified time
 *                           period has elapsed without a notification from
 *                           another thread
 */
jint JEMCC_SysMonitorMilliWait(JEMCC_SysMonitor *mon, jlong milli) {
    return JEMCC_SysMonitorNanoWait(mon, milli, 0);
}

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
 *                      notified this thread to restart
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 *     JEMCC_MONITOR_TIMEOUT - the wait was successful and the specified time
 *                           period has elapsed without a notification from
 *                           another thread
 */
jint JEMCC_SysMonitorNanoWait(JEMCC_SysMonitor *mon, jlong milli, jint nano) {
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) mon;
    JEMCC_ThreadId currentThreadId = JEMCC_GetCurrentThread();
    int rc, lastReentryCount = monData->reentryCount;
    struct timespec ts;
    struct timeval tv;

    /* See java.lang.Object documentation for notation on this behaviour */
    if ((milli == 0) && (nano == 0)) {
        return JEMCC_SysMonitorWait(mon);
    }

    /* Cannot exit the thread if the current thread does not own it */
    if (!JEMCC_IsSameThread(monData->owner, currentThreadId)) {
        return JEMCC_MONITOR_NOT_OWNER;
    }

    /* "Release" the monitor temporarily to allow external wait access */
    monData->reentryCount = 0;
    monData->owner = 0;

    /* Drop into the wait condition, for the given time */
    (void) gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + (int) (milli / 1000); /* No nano - zero roundoff */
    ts.tv_nsec = tv.tv_usec * 1000 + (milli % 1000) * 1000000 + 
                                     (nano % 1000000);
#ifdef USE_PTHREADS
    rc = pthread_cond_timedwait(&monData->condition, &monData->mutex, &ts);
#endif

    /* We have been notified and have the monitor - restore tracking data */
    monData->reentryCount = lastReentryCount;
    monData->owner = currentThreadId;

#ifdef USE_PTHREADS
    return (rc == ETIMEDOUT) ? JEMCC_MONITOR_TIMEOUT : JEMCC_MONITOR_OK;
#endif
}

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
 *                      there were no threads waiting on the monitor
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 */
jint JEMCC_SysMonitorNotify(JEMCC_SysMonitor *mon) {
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) mon;
    JEMCC_ThreadId currentThreadId = JEMCC_GetCurrentThread();

    /* Cannot exit the thread if the current thread does not own it */
    if (!JEMCC_IsSameThread(monData->owner, currentThreadId)) {
        return JEMCC_MONITOR_NOT_OWNER;
    }

    /* Send the notification to one (random) thread waiting on this monitor */
#ifdef USE_PTHREADS
    (void) pthread_cond_signal(&monData->condition);
#endif

    return JEMCC_MONITOR_OK;
}

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
 *                      there were no threads waiting on the monitor
 *     JEMCC_MONITOR_NOT_OWNER - the current thread has not entered the monitor
 */
jint JEMCC_SysMonitorNotifyAll(JEMCC_SysMonitor *mon) {
    JEMCC_SysMonitorData *monData = (JEMCC_SysMonitorData *) mon;
    JEMCC_ThreadId currentThreadId = JEMCC_GetCurrentThread();

    /* Cannot exit the thread if the current thread does not own it */
    if (!JEMCC_IsSameThread(monData->owner, currentThreadId)) {
        return JEMCC_MONITOR_NOT_OWNER;
    }

    /* Send the notification to all threads waiting on this monitor */
#ifdef USE_PTHREADS
    (void) pthread_cond_broadcast(&monData->condition);
#endif

    return JEMCC_MONITOR_OK;
}
