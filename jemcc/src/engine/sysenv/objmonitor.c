/**
 * JEMCC system/environment functions to support object monitors.
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

/* Read the cpu/arch dependent definitions/codebases */
#define FROM_OBJ_MONITOR_C 1
#include "sysctrl.h"

/* Lock state bitmask/sets for various object monitor conditions */
#define LOCK_AVAILABLE    0x0
#define LOCK_WAITING      0x1
#define LOCK_ACTIVE       0x2
#define LOCK_IN_PROGRESS  0x3

#define LOCK_STATE_MASK   0x3

/* State definitions for the object queue locking transfers */
#define STATETXFR_IDLE 0
#define STATETXFR_SEND_WAIT 1
#define STATETXFR_RECV_WAIT 2
#define STATETXFR_COMPLETE 3

#if defined HAS_COMPARE_AND_SWAP && ! defined USE_SPINLOCK_QUEUE_LOCK

/**
 * Local method used to lock the object queue, allowing the other
 * methods below to avoid conflicts during manipulation of the object
 * lock queues.
 *
 * Parameters:
 *     jenv - the VM environment which is currently in context (the associated
 *            thread may block if required)
 *     stateFieldAddr - the pointer to the object state information (offset
 *                      already calculated by the calling method)
 *
 * Returns:
 *     The object state information which existed prior to the queue 
 *     locking action.  Used to restore original state information or
 *     to interact with prior lock operations.
 *
 * Exceptions:
 *     None are thrown but will abort() if an internal monitor error occurs.
 */
static juint JEM_LockObjectQueue(JEM_JNIEnv *jenv, juint *stateFieldAddr) {
    JEM_JNIEnv *lastEnv;
    juint lastStateSet;

    /* Trade the current lock state with the in-progress queue lock state */
    lastStateSet = ((juint) jenv) | LOCK_IN_PROGRESS;
#ifdef HAS_FETCH_AND_STORE
    FETCH_AND_STORE(lastStateSet, stateFieldAddr);
#endif

    /* Based on prior state, we have the lock or must negotiate for it */
    if ((lastStateSet & LOCK_STATE_MASK) != LOCK_IN_PROGRESS) {
        return lastStateSet;
    }

    /* Wait or grab the lock from the previous locking thread */
    lastEnv = (JEM_JNIEnv *) (lastStateSet & (~((juint) LOCK_STATE_MASK)));
    JEMCC_EnterSysMonitor(lastEnv->objStateTxfrMonitor);
    switch (lastEnv->stateTxfrMode) {
        case STATETXFR_IDLE:
            lastEnv->stateTxfrMode = STATETXFR_RECV_WAIT;
            while (lastEnv->stateTxfrMode != STATETXFR_COMPLETE) {
                if (JEMCC_SysMonitorWait(lastEnv->objStateTxfrMonitor) 
                                                       != JEMCC_MONITOR_OK) {
                    abort(); /* purecov: deadcode */
                }
            }
            lastEnv->stateTxfrMode = STATETXFR_IDLE;
            lastStateSet = lastEnv->stateTxfrSet;
            break;
        case STATETXFR_SEND_WAIT:
            lastEnv->stateTxfrMode = STATETXFR_COMPLETE;
            lastStateSet = lastEnv->stateTxfrSet;
            if (JEMCC_SysMonitorNotify(lastEnv->objStateTxfrMonitor) 
                                                       != JEMCC_MONITOR_OK) {
                abort(); /* purecov: deadcode */
            }
            break;
        default:
            abort(); /* purecov: deadcode */
    }
    if (JEMCC_ExitSysMonitor(lastEnv->objStateTxfrMonitor) 
                                                  != JEMCC_MONITOR_OK) {
        abort(); /* purecov: deadcode */
    }
    return lastStateSet;
}

/**
 * Local method used to unlock the object queue, allowing other waiting threads
 * to manipulate the queue information.
 *
 * Parameters:
 *     jenv - the VM environment which is currently in context (the associated
 *            thread may block if required)
 *     stateFieldAddr - the pointer to the object state information (offset
 *                      already calculated by the calling method)
 *     newStateSet - the new object state information, which may be the
 *                   original state data or an indicator of the object
 *                   lock queue condition
 *
 * Exceptions:
 *     None are thrown but will abort() if an internal monitor error occurs.
 */
static void JEM_UnlockObjectQueue(JEM_JNIEnv *jenv, juint *stateFieldAddr,
                                  juint newStateSet) {
    juint compState;
#ifdef HAS_COMPARE_AND_SWAP
    register char casResult;
#endif

    /* Attempt to swap the in-progress state with new state (fast unlock) */
    compState = ((juint) jenv) | LOCK_IN_PROGRESS;
#ifdef HAS_COMPARE_AND_SWAP
    COMPARE_AND_SWAP(compState, newStateSet, stateFieldAddr, casResult);
    if (casResult != 0) return;
#endif

    /* Swap failed - wait or send the lock to the next locking thread */
    JEMCC_EnterSysMonitor(jenv->objStateTxfrMonitor);
    switch (jenv->stateTxfrMode) {
        case STATETXFR_IDLE:
            jenv->stateTxfrSet = newStateSet;
            jenv->stateTxfrMode = STATETXFR_SEND_WAIT;
            while (jenv->stateTxfrMode != STATETXFR_COMPLETE) {
                if (JEMCC_SysMonitorWait(jenv->objStateTxfrMonitor) 
                                                      != JEMCC_MONITOR_OK) {
                    abort(); /* purecov: deadcode */
                }
            }
            jenv->stateTxfrMode = STATETXFR_IDLE;
            break;
        case STATETXFR_RECV_WAIT:
            jenv->stateTxfrSet = newStateSet;
            jenv->stateTxfrMode = STATETXFR_COMPLETE;
            if (JEMCC_SysMonitorNotify(jenv->objStateTxfrMonitor) 
                                                      != JEMCC_MONITOR_OK) {
                abort(); /* purecov: deadcode */
            }
            break;
        default:
            abort(); /* purecov: deadcode */
    }
    if (JEMCC_ExitSysMonitor(jenv->objStateTxfrMonitor) 
                                               != JEMCC_MONITOR_OK) {
        abort(); /* purecov: deadcode */
    }
}

#else

/**
 * Local method used to lock the object queue, allowing the other
 * methods below to avoid conflicts during manipulation of the object
 * lock queues.
 *
 * Parameters:
 *     jenv - the VM environment which is currently in context (the associated
 *            thread may block if required)
 *     stateFieldAddr - the pointer to the object state information (offset
 *                      already calculated by the calling method)
 *
 * Returns:
 *     The object state information which existed prior to the queue 
 *     locking action.  Used to restore original state information or
 *     to interact with prior lock operations.
 *
 * Exceptions:
 *     None are thrown but will abort() if an internal monitor error occurs.
 */
static juint JEM_LockObjectQueue(JEM_JNIEnv *jenv, juint *stateFieldAddr) {
    jlong exponentialSleepPeriod = 0;
    juint lockStateValue;

    /* Traditional spinlock style test-and-set action */
    while (1) {
        /* Attempt to swap in the lock value */
        lockStateValue = 0xFFFFFFFF;
#ifdef HAS_FETCH_AND_STORE
        FETCH_AND_STORE(lockStateValue, stateFieldAddr);
#endif

        /* Return the last value if we have achieved the lock */
        if (lockStateValue != 0xFFFFFFFF) return lockStateValue;

        /* Perform a retry/recheck with exponential backoff */
        if (exponentialSleepPeriod == 0) {
            for (; exponentialSleepPeriod < 50; exponentialSleepPeriod += 10) {
                JEMCC_YieldCurrentThread();
                if (*stateFieldAddr != 0xFFFFFFFF) break;
            }
        } else {
            JEMCC_YieldCurrentThreadAndSleep(exponentialSleepPeriod);
            if (exponentialSleepPeriod < 1000) {
                exponentialSleepPeriod = exponentialSleepPeriod * 2;
            }
        }
    }
}

/**
 * Local method used to unlock the object queue, allowing other waiting threads
 * to manipulate the queue information.
 *
 * Parameters:
 *     jenv - the VM environment which is currently in context (the associated
 *            thread may block if required)
 *     stateFieldAddr - the pointer to the object state information (offset
 *                      already calculated by the calling method)
 *     newStateSet - the new object state information, which may be the
 *                   original state data or an indicator of the object
 *                   lock queue condition
 *
 * Exceptions:
 *     None are thrown but will abort() if an internal monitor error occurs.
 */
static void JEM_UnlockObjectQueue(JEM_JNIEnv *jenv, juint *stateFieldAddr,
                                  juint newStateSet) {
    /* Spinlock exit is very straightforward */
    *stateFieldAddr = newStateSet;
}

#endif

/**
 * Local method used by the enter and wait methods where several 
 * threads are actively attempting to obtain the active lock for a specific
 * object.  Will repeatedly wait on the object monitor until the
 * txfrActiveObjectLock method below hands off the active lock and "kicks"
 * the waiting thread back to life.
 *
 * NOTE: the object queue lock must be obtained prior to calling this
 *       method (the current lock state must be provided).  The object
 *       queue lock will be released prior to this method returning.
 *
 * Parameters:
 *     jenv - the VM environment which is currently in context (the associated
 *            thread may block if required)
 *     stateFieldAddr - the pointer to the object state information (offset
 *                      already calculated by the calling method)
 *     currentQueueLockState - the current state of the object lock queue (as
 *                             obtained from the lockObjectQueue method)
 *     targetEntry - the lock record associated with the current thread which
 *                   is waiting to be the active record
 *
 * Exceptions:
 *     None are thrown but will abort() if an internal monitor error occurs.
 */
static void JEM_WaitForActiveObjectLock(JEM_JNIEnv *jenv, juint *stateFieldAddr,
                                        juint currentQueueLockState,
                                        JEM_ObjLockQueueEntry *targetEntry) {
    JEM_ObjLockQueueEntry *lockEntry, *queueEntry;

    /* Do this forever */
    while (1) {
        /* If a thread is waiting for the lock, only two states possible */
        switch (currentQueueLockState & LOCK_STATE_MASK) {
            case LOCK_ACTIVE:
                lockEntry = (JEM_ObjLockQueueEntry*) 
                     (currentQueueLockState & (~((juint) LOCK_STATE_MASK)));
                if (lockEntry == targetEntry) {
                    /* Active lock has been directly passed, all done */
                    JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                                          currentQueueLockState);
                    return;
                }
                /* Need to wait for txfr method to pass active lock */
                JEMCC_EnterSysMonitor(jenv->objLockMonitor);
                JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                                      currentQueueLockState);
                if (JEMCC_SysMonitorWait(jenv->objLockMonitor) != 
                                                      JEMCC_MONITOR_OK) {
                    abort(); /* purecov: deadcode */
                }
                if (JEMCC_ExitSysMonitor(jenv->objLockMonitor) != 
                                                      JEMCC_MONITOR_OK) {
                    abort(); /* purecov: deadcode */
                }

                /* Quick check for direct handoff */
                currentQueueLockState = *stateFieldAddr;
                if ((currentQueueLockState & LOCK_STATE_MASK) == 
                                                        LOCK_ACTIVE) {
                    lockEntry = (JEM_ObjLockQueueEntry*) 
                                        (currentQueueLockState & 
                                             (~((juint) LOCK_STATE_MASK)));
                    if (lockEntry == targetEntry) return;
                }

                /* Reacquire the queue lock and try again */
                currentQueueLockState = JEM_LockObjectQueue(jenv, 
                                                            stateFieldAddr);
                break;
            case LOCK_WAITING:
                /* Thread is first to awaken, take lead of queue */
                lockEntry = (JEM_ObjLockQueueEntry*) 
                     (currentQueueLockState & (~((juint) LOCK_STATE_MASK)));
                if (lockEntry != targetEntry) {
                    /* Not the first entry, locate and move to lead */
                    queueEntry = lockEntry;
                    while (queueEntry->nextEntry != targetEntry) {
                        queueEntry = queueEntry->nextEntry;
                    }
                    queueEntry->nextEntry = targetEntry->nextEntry;
                    targetEntry->objStateSet = lockEntry->objStateSet;
                    targetEntry->nextEntry = lockEntry;
                }

                /* The thread now has the active object lock, mark it */
                JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                                      ((juint) targetEntry | LOCK_ACTIVE));
                return;
            default:
                abort(); /* purecov: deadcode */
        }
    }
}

/**
 * Local method used by the exit and wait methods to handle off the active
 * object lock to another thread which may be waiting.  Cleanly handles the
 * condition where all locked threads are in specific wait states.
 *
 * NOTE: the object queue lock must be obtained prior to calling this
 *       method (the current lock state must be provided).  The object
 *       queue lock will be released prior to this method returning.
 *
 * Parameters:
 *     jenv - the VM environment which is currently in context
 *     stateFieldAddr - the pointer to the object state information (offset
 *                      already calculated by the calling method)
 *     targetEntry - the first queue entry of the remaining lock records which
 *                   could be waiting for the active lock (basis of the
 *                   new object state record)
 *
 * Exceptions:
 *     None are thrown but will abort() if an internal monitor error occurs.
 */
static void JEM_TxfrActiveObjectLock(JEM_JNIEnv *jenv, juint *stateFieldAddr,
                                     JEM_ObjLockQueueEntry *targetEntry) {
    JEM_JNIEnv *nextActiveEnv;
    JEM_ObjLockQueueEntry *nextActiveEntry, *queueEntry;

    /* First, find the next thread which is waiting for the active lock */
    nextActiveEntry = targetEntry;
    while (nextActiveEntry != NULL) {
        if (nextActiveEntry->entryCount > 0) break;
        nextActiveEntry = nextActiveEntry->nextEntry;
    }
    if (nextActiveEntry == NULL) {
        /* No threads are waiting for the active lock, must just be waiting */
        JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                              ((juint) targetEntry) | LOCK_WAITING);
        return;
    }

    /* Move the new active lock record to the head of the queue */
    if (nextActiveEntry != targetEntry) {
        queueEntry = targetEntry;
        while (queueEntry->nextEntry != nextActiveEntry) {
            queueEntry = queueEntry->nextEntry;
        }
        queueEntry->nextEntry = nextActiveEntry->nextEntry;
        nextActiveEntry->objStateSet = targetEntry->objStateSet;
        nextActiveEntry->nextEntry = targetEntry;
    }

    /* Give the next active thread the lock and a kick */
    nextActiveEnv = nextActiveEntry->parentEnv;
    JEMCC_EnterSysMonitor(nextActiveEnv->objLockMonitor);
    JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                          ((juint) nextActiveEntry) | LOCK_ACTIVE);
    if (JEMCC_SysMonitorNotify(nextActiveEnv->objLockMonitor) != 
                                                         JEMCC_MONITOR_OK) {
        abort(); /* purecov: deadcode */
    }
    if (JEMCC_ExitSysMonitor(nextActiveEnv->objLockMonitor) != 
                                                         JEMCC_MONITOR_OK) {
        abort(); /* purecov: deadcode */
    }
}

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
jint JEMCC_EnterObjMonitor(JNIEnv *env, jobject obj) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint *stateFieldAddr = &(((JEMCC_Object *) obj)->objStateSet);
    JEM_ObjLockQueueEntry *queueEntry, *lockEntry;
    juint compState, lockState;
    int i;
#ifdef HAS_COMPARE_AND_SWAP
    register char casResult;
#endif

    /* Handle the relock case first (very simple data manipulation) */
    compState = *stateFieldAddr;
    if ((compState & LOCK_STATE_MASK) == LOCK_ACTIVE) {
        lockEntry = (JEM_ObjLockQueueEntry*) 
                                (compState & (~((juint) LOCK_STATE_MASK)));
        if (lockEntry->parentEnv == jenv) {
            lockEntry->entryCount++;
            return JNI_OK;
        }
    }

    /* Ensure adequate lock queue records (and pull one from the free list) */
    if (jenv->freeObjLockQueue == NULL) {
        for (i = 0; i < 4; i++) {
            lockEntry = (JEM_ObjLockQueueEntry *) JEMCC_Malloc(env,
                                        (juint) sizeof(JEM_ObjLockQueueEntry));
            if (lockEntry == NULL) return JNI_ENOMEM;
            lockEntry->parentEnv = jenv;
            lockEntry->entryCount = 1;
            lockEntry->nextEntry = jenv->freeObjLockQueue;
            jenv->freeObjLockQueue = lockEntry;
        }
    }
    lockEntry = jenv->freeObjLockQueue;
    jenv->freeObjLockQueue = lockEntry->nextEntry;
    lockEntry->nextEntry = NULL;

    /* Rapid lock sequence for uncontested lock states */
#ifdef HAS_COMPARE_AND_SWAP
    if ((compState & LOCK_STATE_MASK) == LOCK_AVAILABLE) {
        lockState = ((juint) lockEntry) | LOCK_ACTIVE;
        lockEntry->objStateSet = compState;
        COMPARE_AND_SWAP(compState, lockState, stateFieldAddr, casResult);
        if (casResult != 0) return JNI_OK;
    }
#endif

    /* Lock the object queue and get the current lock state */
    lockState = JEM_LockObjectQueue(jenv, stateFieldAddr);
    switch (lockState & LOCK_STATE_MASK) {
        case LOCK_AVAILABLE:
            /* Simply store the new lock record into the state set */
            lockEntry->objStateSet = lockState;
            JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                                  ((juint) lockEntry) | LOCK_ACTIVE);
            break;
        case LOCK_WAITING:
            /* Waiting thread locks present - grab and insert new lock record */
            queueEntry = (JEM_ObjLockQueueEntry*) 
                                    (lockState & (~((juint) LOCK_STATE_MASK)));
            lockEntry->nextEntry = queueEntry;
            lockEntry->objStateSet = queueEntry->objStateSet;
            JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                                  ((juint) lockEntry) | LOCK_ACTIVE);
            break;
        case LOCK_ACTIVE:
            /* Either increment the relock or insert in the queue and wait */
            queueEntry = (JEM_ObjLockQueueEntry*) 
                                    (lockState & (~((juint) LOCK_STATE_MASK)));
            if (queueEntry->parentEnv == jenv) {
                queueEntry->entryCount++;
                JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);

                /* Don't forget to return the queue entry to the free list */
                lockEntry->nextEntry = jenv->freeObjLockQueue;
                jenv->freeObjLockQueue = lockEntry;
            } else {
                /* Add the new record to the end of the queue */
                while (queueEntry->nextEntry != NULL) {
                    queueEntry = queueEntry->nextEntry;
                }
                queueEntry->nextEntry = lockEntry;

                /* Wait for it to move to the head of the queue (active) */
                JEM_WaitForActiveObjectLock(jenv, stateFieldAddr, 
                                            lockState, lockEntry);
            }
            break;
        default:
            abort(); /* purecov: deadcode */
    }

    return JNI_OK;
}

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
jint JEMCC_ExitObjMonitor(JNIEnv *env, jobject obj) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint *stateFieldAddr = &(((JEMCC_Object *) obj)->objStateSet);
    JEM_ObjLockQueueEntry *lockEntry;
    juint lockState, compState;
#ifdef HAS_COMPARE_AND_SWAP
    register char casResult;
#endif

    /* Handle quick uncontested lock conditions (no queue lock) */
    compState = *stateFieldAddr;
    if ((compState & LOCK_STATE_MASK) == LOCK_ACTIVE) {
        lockEntry = (JEM_ObjLockQueueEntry*) 
                                (compState & (~((juint) LOCK_STATE_MASK)));
        if (lockEntry->parentEnv == jenv) {
            if (lockEntry->entryCount > 1) {
                lockEntry->entryCount--;
                return JNI_OK;
#ifdef HAS_COMPARE_AND_SWAP
            } else {
                /* Can only handle the unqueued case without locking actions */
                if (lockEntry->nextEntry == NULL) {
                    COMPARE_AND_SWAP(compState, lockEntry->objStateSet, 
                                     stateFieldAddr, casResult);
                    if (casResult != 0) {
                        /* Make sure to recycle lock record before returning */
                        lockEntry->nextEntry = jenv->freeObjLockQueue;
                        jenv->freeObjLockQueue = lockEntry;
                        return JNI_OK;
                    }
                }
#endif
            }
        }
    }

    /* Grab control of the queue and validate lock condition */
    lockState = JEM_LockObjectQueue(jenv, stateFieldAddr);
    lockEntry = (JEM_ObjLockQueueEntry*) 
                                (lockState & (~((juint) LOCK_STATE_MASK)));
    if (((lockState & LOCK_STATE_MASK) != LOCK_ACTIVE) ||
                                          (lockEntry->parentEnv != jenv)) {
        JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IllegalMonitorStateException,
                                   NULL, "Monitor not entered for exit");
        return JNI_ERR;
    }

    /* Handle the different lock exit conditions */
    if (lockEntry->entryCount > 1) {
        lockEntry->entryCount--;
        JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);
    } else {
        if (lockEntry->nextEntry == NULL) {
            /* Just unlock the queue returning original state information */
            JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockEntry->objStateSet);
        } else {
            /* Pass the state data (and lock) to the next entry */
            lockEntry->nextEntry->objStateSet = lockEntry->objStateSet;
            JEM_TxfrActiveObjectLock(jenv, stateFieldAddr, 
                                     lockEntry->nextEntry);
        }
        /* Make sure to recycle lock record before returning */
        lockEntry->nextEntry = jenv->freeObjLockQueue;
        jenv->freeObjLockQueue = lockEntry;
    }

    return JNI_OK;
}

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
jint JEMCC_ObjMonitorWait(JNIEnv *env, jobject obj) {
    /* See java.lang.Object documentation for notation on this shortcut */
    return JEMCC_ObjMonitorNanoWait(env, obj, 0, 0);
}

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
jint JEMCC_ObjMonitorMilliWait(JNIEnv *env, jobject obj, jlong milli) {
    return JEMCC_ObjMonitorNanoWait(env, obj, milli, 0);
}

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
jint JEMCC_ObjMonitorNanoWait(JNIEnv *env, jobject obj, 
                              jlong milli, jint nano) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint *stateFieldAddr = &(((JEMCC_Object *) obj)->objStateSet);
    JEM_ObjLockQueueEntry *lockEntry;
    juint lockState;
    jint rc;

    /* Quick check of input parameters */
    if (milli < 0) {
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IllegalArgumentException,
                                   NULL, "Negative timeout given for wait");
        return JNI_ERR;
    }
    if ((nano < 0) || (nano > 999999)) {
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IllegalArgumentException,
                                   NULL, "Invalid nanosecond timeout for wait");
        return JNI_ERR;
    }

    /* Grab control of the queue and validate lock condition */
    lockState = JEM_LockObjectQueue(jenv, stateFieldAddr);
    lockEntry = (JEM_ObjLockQueueEntry*) 
                                (lockState & (~((juint) LOCK_STATE_MASK)));
    if (((lockState & LOCK_STATE_MASK) != LOCK_ACTIVE) ||
                                          (lockEntry->parentEnv != jenv)) {
        JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IllegalMonitorStateException,
                                   NULL, "Monitor not entered for wait");
        return JNI_ERR;
    }

    /* Grab environment wait monitor and update wait indicator */
    JEMCC_EnterSysMonitor(jenv->objLockMonitor);
    lockEntry->entryCount = -lockEntry->entryCount;

    /* Transfer control if others waiting, otherwise release wait queue */
    if (lockEntry->nextEntry == NULL) {
        JEM_UnlockObjectQueue(jenv, stateFieldAddr,
                              ((juint) lockEntry) | LOCK_WAITING);
    } else {
        JEM_TxfrActiveObjectLock(jenv, stateFieldAddr, lockEntry);
    }

    /* Perform the wait/exit of the local environment monitor */
    if ((milli == 0) && (nano == 0)) {
        /* See java.lang.Object documentation for notation on this behaviour */
        if (JEMCC_SysMonitorWait(jenv->objLockMonitor) != JEMCC_MONITOR_OK) {
            abort(); /* purecov: deadcode */
        }
    } else {
        rc = JEMCC_SysMonitorNanoWait(jenv->objLockMonitor, milli, nano);
        if ((rc != JEMCC_MONITOR_OK) && (rc != JEMCC_MONITOR_TIMEOUT)) {
            abort(); /* purecov: deadcode */
        }
    }
    if (lockEntry->entryCount < 0) {
        lockEntry->entryCount = -lockEntry->entryCount;
    }
    if (JEMCC_ExitSysMonitor(jenv->objLockMonitor) != JEMCC_MONITOR_OK) {
        abort(); /* purecov: deadcode */
    }

    /* Check for direct unlock/transfer */
    lockState = *stateFieldAddr;
    if ((lockState & LOCK_STATE_MASK) == LOCK_ACTIVE) {
        lockEntry = (JEM_ObjLockQueueEntry*) 
                                (lockState & (~((juint) LOCK_STATE_MASK)));
        if (lockEntry->parentEnv == jenv) return JNI_OK;
    }

    /* Regain queue control and wait for active transfer */
    lockState = JEM_LockObjectQueue(jenv, stateFieldAddr);
    JEM_WaitForActiveObjectLock(jenv, stateFieldAddr, lockState, lockEntry);

    return JNI_OK;
}

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
jint JEMCC_ObjMonitorNotify(JNIEnv *env, jobject obj) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint *stateFieldAddr = &(((JEMCC_Object *) obj)->objStateSet);
    JEM_ObjLockQueueEntry *lockEntry;
    juint lockState;

    /* Grab control of the queue and validate lock condition */
    lockState = JEM_LockObjectQueue(jenv, stateFieldAddr);
    lockEntry = (JEM_ObjLockQueueEntry*) 
                                (lockState & (~((juint) LOCK_STATE_MASK)));
    if (((lockState & LOCK_STATE_MASK) != LOCK_ACTIVE) ||
                                          (lockEntry->parentEnv != jenv)) {
        JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IllegalMonitorStateException,
                                   NULL, "Monitor not entered for notify");
        return JNI_ERR;
    }

    /* Find a waiting lock entry (if available) and send it along */
    lockEntry = lockEntry->nextEntry;
    while (lockEntry != NULL) {
        if (lockEntry->entryCount < 0) {
            lockEntry->entryCount = -lockEntry->entryCount;
            break;
        }
        lockEntry = lockEntry->nextEntry;
    }
    JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);

    return JNI_OK;
}

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
jint JEMCC_ObjMonitorNotifyAll(JNIEnv *env, jobject obj) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    juint *stateFieldAddr = &(((JEMCC_Object *) obj)->objStateSet);
    JEM_ObjLockQueueEntry *lockEntry;
    juint lockState;

    /* Grab control of the queue and validate lock condition */
    lockState = JEM_LockObjectQueue(jenv, stateFieldAddr);
    lockEntry = (JEM_ObjLockQueueEntry*) 
                                (lockState & (~((juint) LOCK_STATE_MASK)));
    if (((lockState & LOCK_STATE_MASK) != LOCK_ACTIVE) ||
                                          (lockEntry->parentEnv != jenv)) {
        JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);
        JEMCC_ThrowStdThrowableIdx(env, 
                                   JEMCC_Class_IllegalMonitorStateException,
                                   NULL, "Monitor not entered for notifyAll");
        return JNI_ERR;
    }

    /* Find all waiting lock entries (if available) and send them along */
    lockEntry = lockEntry->nextEntry;
    while (lockEntry != NULL) {
        if (lockEntry->entryCount < 0) {
            lockEntry->entryCount = -lockEntry->entryCount;
        }
        lockEntry = lockEntry->nextEntry;
    }
    JEM_UnlockObjectQueue(jenv, stateFieldAddr, lockState);

    return JNI_OK;
}
