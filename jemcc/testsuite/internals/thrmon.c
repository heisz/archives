/**
 * JEMCC test program to test threading and monitor system methods.
 * Copyright (C) 1999-2004 J.M. Heisz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU General Public License,
 * as well as further clarification on your rights to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "jeminc.h"
#include <time.h>

/* Read the jni/jem internal details */
#include "jem.h"

/* Storage buffer for exception class and message (failure verification) */
char exClassName[256], exMsg[4096];

void checkException(const char *checkClassName, const char *checkMsg,
                    const char *tstName) {
    if ((checkClassName != NULL) &&
        (strstr(exClassName, checkClassName) == NULL)) {
        (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
        (void) fprintf(stderr, "Expected %s, got %s instead.\n",
                               checkClassName, exClassName);
        exit(1);
    }
    if ((checkMsg != NULL) && (strstr(exMsg, checkMsg) == NULL)) {
        (void) fprintf(stderr, "Fatal exception failure on %s\n", tstName);
        (void) fprintf(stderr, "Expected '%s', got '%s' instead.\n",
                               checkMsg, exMsg);
        exit(1);
    }
    *exClassName = *exMsg = '\0';
}

/* Test flags/data values for condition setups and tests */
static int mallocFailFlag = 0;
static char *testObj = "x";
static JEMCC_ThreadId primThreadId;
static JEMCC_SysMonitor *topSysMonitor;
static JEMCC_Object *topObjMonitor;

/* Spinning thread test program used by the multi-CPU test sequence */
void *threadSpinFn(JNIEnv *env, void *userArg) {
    int pigs_can_fly = 0;
    while (pigs_can_fly == 0);

    return (void *) 0;
}

/* Thread method for testing primary thread operations */
void *primThreadTestFn(JNIEnv *env, void *userArg) {
    char *msg;

    /* Wait for global monitor to ensure setup is correct */
    JEMCC_EnterGlobalMonitor();

    /* Handle basic thread method test cases */
    if (JEMCC_IsSameThread(JEMCC_GetCurrentThread(), 
                           primThreadId) != JNI_TRUE) {
        (void) fprintf(stderr, "Error: current thread is not core thread.\n");
        exit(1);
    }
    msg = JEMCC_AssociateThreadValue((void *) testObj);
    if (msg != NULL) {
        (void) fprintf(stderr, "Error: failed to attach thread value %s.\n",
                               msg);
        exit(1);
    }
    if (JEMCC_RetrieveThreadValue() != (void *) testObj) {
        (void) fprintf(stderr, "Error: attached thread value is invalid %p.\n",
                               JEMCC_RetrieveThreadValue());
        exit(1);
    }

    /* All done, return happy */
    JEMCC_ExitGlobalMonitor();

    return (void *) 0;
}

/* Thread method for testing system wait/notify operations */
static int sysWaitCount = 0;
void *sysWaitTestFn(JNIEnv *env, void *userArg) {
    /* Grab the wait test monitor */
    JEMCC_EnterSysMonitor(topSysMonitor);

    /* Perform an immediate wait for the launcher to trigger */
    (void) fprintf(stdout, "Thread %i entering initial wait state\n",
                           JEMCC_GetCurrentThread());
    JEMCC_SysMonitorNanoWait(topSysMonitor, 0, 0);

    /* Update the working count and wait/notify appropriately */
    sysWaitCount++;
    if ((sysWaitCount == 1) || (sysWaitCount >= 3)) {
        (void) fprintf(stdout, "Thread %i sending notify\n",
                               JEMCC_GetCurrentThread());
        JEMCC_SysMonitorNotify(topSysMonitor);
    } else {
        (void) fprintf(stdout, "Thread %i entering wait state\n",
                               JEMCC_GetCurrentThread());
        JEMCC_SysMonitorWait(topSysMonitor);
    }

    /* Thread test completed, release the wait test monitor */
    if (JEMCC_ExitSysMonitor(topSysMonitor) != JEMCC_MONITOR_OK) {
        (void) fprintf(stderr, "Error: system monitor release failed.\n");
        exit(1);
    }
    (void) fprintf(stdout, "Thread %i terminating\n", JEMCC_GetCurrentThread());

    return (void *) 0;
}

/* Methods for creating/destroying local environment instances */
JEM_JNIEnv *createTempEnv() {
    JEM_JNIEnv *jenv;

    jenv = (JEM_JNIEnv *) JEMCC_Malloc(NULL, sizeof(JEM_JNIEnv));
    if (jenv == NULL) {
        (void) fprintf(stderr, "Error: env alloc failed.\n");
        exit(1);
    }
    jenv->freeObjLockQueue = NULL;
    jenv->objStateTxfrMonitor = JEMCC_CreateSysMonitor(NULL);
    if (jenv->objStateTxfrMonitor == NULL) {
        (void) fprintf(stderr, "Error: env system monitor failed.\n");
        exit(1);
    }
    jenv->objLockMonitor = JEMCC_CreateSysMonitor(NULL);
    if (jenv->objLockMonitor == NULL) {
        (void) fprintf(stderr, "Error: env lock monitor failed.\n");
        exit(1);
    }

    return jenv;
}
void destroyTempEnv(JEM_JNIEnv *jenv) {
    JEM_ObjLockQueueEntry *lockEntry, *nextEntry;

    if (jenv->objStateTxfrMonitor != NULL) {
        JEMCC_DestroySysMonitor(jenv->objStateTxfrMonitor);
    }
    if (jenv->objLockMonitor != NULL) {
        JEMCC_DestroySysMonitor(jenv->objLockMonitor);
    }
    lockEntry = jenv->freeObjLockQueue;
    while (lockEntry != NULL) {
        nextEntry = lockEntry->nextEntry;
        JEMCC_Free(lockEntry);
        lockEntry = nextEntry;
    }
    JEMCC_Free(jenv);
}

/* Thread method for testing object monitor enter/exit criteria */
static jlong objCheckCount = 0, objExitCount = 0;
void *objMonitorTestFn(JNIEnv *env, void *userArg) {
    int passCount = 0, index = (int) userArg;

    /* These tests need a JNIEnv instance for object lock management */
    env = (JNIEnv *) createTempEnv();

    (void) fprintf(stdout, "Thread %i beginning race operations\n",
                           JEMCC_GetCurrentThread());
    while (passCount < 5000) {
        /* Grab monitor and modify count */
        if (JEMCC_EnterObjMonitor(env, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object monitor lock failed.\n");
            exit(1);
        }
        if ((index % 2) != 0) {
            if (JEMCC_EnterObjMonitor(env, topObjMonitor) != JNI_OK) {
                (void) fprintf(stderr, "Error: object monitor relock fail.\n");
                exit(1);
            }
        }

        /* Perform complex modifications to ensure thread collisions */
        if (objCheckCount == 0) {
            JEMCC_YieldCurrentThread();
            objCheckCount += 0xFFFFFFFFFFFL;
            JEMCC_YieldCurrentThread();
        } else if (objCheckCount == 0xFFFFFFFFFFFL) {
            JEMCC_YieldCurrentThread();
            objCheckCount += 0x1CCCCCCCCCL;
            JEMCC_YieldCurrentThread();
        } else if (objCheckCount == 0x101CCCCCCCCBL) {
            JEMCC_YieldCurrentThread();
            objCheckCount += 0x535353535353L;
            JEMCC_YieldCurrentThread();
        } else if (objCheckCount == 0x63702020201EL) {
            JEMCC_YieldCurrentThread();
            objCheckCount = 0;
            JEMCC_YieldCurrentThread();
        } else {
            (void) fprintf(stderr,
                           "Error: object locked value change failed %llx.\n", 
                           objCheckCount);
            exit(1);
        }

        if ((index % 2) != 0) {
            if (JEMCC_ExitObjMonitor(env, topObjMonitor) != JNI_OK) {
                (void) fprintf(stderr, "Error: object monitor release fail.\n");
                exit(1);
            }
        }
        if (JEMCC_ExitObjMonitor(env, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object monitor release failed.\n");
            exit(1);
        }

        JEMCC_YieldCurrentThread();
        passCount++;
    }
    (void) fprintf(stdout, "Thread %i terminating\n", JEMCC_GetCurrentThread());
    if (JEMCC_EnterObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor lock failed.\n");
        exit(1);
    }
    objExitCount++;
    if (JEMCC_ObjMonitorNotify(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor notify failed.\n");
        exit(1);
    }
    if (JEMCC_ExitObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor release failed.\n");
        exit(1);
    }

    /* Clean up the local environment instance */
    destroyTempEnv((JEM_JNIEnv *) env);

    return (void *) 0;
}

/* Thread method for testing object monitor wait/notify operations */
static int objWaitCount = 0;
void *objWaitTestFn(JNIEnv *env, void *userArg) {
    /* These tests need a JNIEnv instance for object lock management */
    env = (JNIEnv *) createTempEnv();

    /* Grab the wait test monitor */
    if (JEMCC_EnterObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor lock failed.\n");
        exit(1);
    }

    /* Perform an immediate wait for the launcher to notify */
    (void) fprintf(stdout, "Thread %i entering initial obj wait state\n",
                           JEMCC_GetCurrentThread());
    if (JEMCC_ObjMonitorNanoWait(env, topObjMonitor, 0, 0) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor wait failed.\n");
        exit(1);
    }

    /* Update the working count and wait/notify appropriately */
    objWaitCount++;
    if ((objWaitCount == 1) || (objWaitCount >= 3)) {
        (void) fprintf(stdout, "Thread %i sending obj notify\n",
                               JEMCC_GetCurrentThread());
        if (JEMCC_ObjMonitorNotify(env, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object monitor notify failed.\n");
            exit(1);
        }
    } else {
        (void) fprintf(stdout, "Thread %i entering obj wait state\n",
                               JEMCC_GetCurrentThread());
        if (JEMCC_ObjMonitorWait(env, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object monitor wait failed.\n");
            exit(1);
        }
    }

    /* Thread test completed, release the wait test monitor */
    if (JEMCC_ExitObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor release failed.\n");
        exit(1);
    }
    (void) fprintf(stdout, "Thread %i terminating\n", JEMCC_GetCurrentThread());

    /* Clean up the local environment instance */
    destroyTempEnv((JEM_JNIEnv *) env);

    return (void *) 0;
}

/* Thread method for testing object monitor timed wait/queueing operations */
void *objTimedWaitTestFn(JNIEnv *env, void *userArg) {
    jint waitPeriod = (jint) userArg;

    /* These tests need a JNIEnv instance for object lock management */
    env = (JNIEnv *) createTempEnv();

    /* Grab the wait test monitor */
    if (JEMCC_EnterObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor lock failed.\n");
        exit(1);
    }

    /* Wait for the indicated period of time */
    (void) fprintf(stdout, "Thread %i entering wait period %i\n",
                           JEMCC_GetCurrentThread(), waitPeriod);
    if (JEMCC_ObjMonitorNanoWait(env, topObjMonitor, 
                                 waitPeriod * 1000, 0) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor timed wait failed.\n");
        exit(1);
    }

    /* Thread test completed, release the wait test monitor */
    if (JEMCC_ExitObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor release failed.\n");
        exit(1);
    }
    (void) fprintf(stdout, "Thread %i terminating\n", JEMCC_GetCurrentThread());

    /* Clean up the local environment instance */
    destroyTempEnv((JEM_JNIEnv *) env);

    return (void *) 0;
}

/* Thread method for testing multiple object monitor notify operations */
void *objNotifyWaitTestFn(JNIEnv *env, void *userArg) {
    /* These tests need a JNIEnv instance for object lock management */
    env = (JNIEnv *) createTempEnv();

    /* Grab the wait test monitor */
    if (JEMCC_EnterObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor lock failed.\n");
        exit(1);
    }

    /* Wait for the indicated period of time */
    (void) fprintf(stdout, "Thread %i entering notify wait\n",
                           JEMCC_GetCurrentThread());
    if (JEMCC_ObjMonitorWait(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor wait failed.\n");
        exit(1);
    }

    /* Thread test completed, release the wait test monitor */
    if (JEMCC_ExitObjMonitor(env, topObjMonitor) != JNI_OK) {
        (void) fprintf(stderr, "Error: object monitor release failed.\n");
        exit(1);
    }
    (void) fprintf(stdout, "Thread %i terminating\n", JEMCC_GetCurrentThread());

    /* Clean up the local environment instance */
    destroyTempEnv((JEM_JNIEnv *) env);

    return (void *) 0;
}

#define TEST_THREAD 1
#define TEST_SYSTEM 2
#define TEST_OBJECT 4

/* Main program will send the threading/monitor routines through their paces */
int main(int argc, char *argv[]) {
    int i, timeDiff, multiCheck = 0, testFlags = 0;
    JEMCC_ThreadId threadId;
    JEM_ObjLockQueueEntry *lockEntry, *nextEntry;
    JEM_JNIEnv *jenv;
    time_t testTime;

    /* Collect options */
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-all") == 0) {
            testFlags = 0xFF;
        } else if (strcmp(argv[i], "-checkmulti") == 0) {
            multiCheck = 1;
        } else if ((strcmp(argv[i], "-h") == 0) ||
                   (strcmp(argv[i], "-help") == 0)) {
            (void) fprintf(stderr, "Usage: %s [options]\n", argv[0]);
            (void) fprintf(stderr, "\nOptions:\n");
            (void) fprintf(stderr, "    -all          all tests [default]\n");
            (void) fprintf(stderr, "    -checkmulti   run multi-CPU test\n");
            (void) fprintf(stderr, "    -object       test object monitors\n");
            (void) fprintf(stderr, "    -system       test system monitors\n");
            (void) fprintf(stderr, "    -thread       test basic threads\n");
            (void) fprintf(stderr, "\n");
            exit(0);
        } else if (strcmp(argv[i], "-object") == 0) {
            testFlags |= TEST_OBJECT;
        } else if (strcmp(argv[i], "-system") == 0) {
            testFlags |= TEST_SYSTEM;
        } else if (strcmp(argv[i], "-thread") == 0) {
            testFlags |= TEST_THREAD;
        }
    }
    if (testFlags == 0) testFlags = 0xFF;

    /* Do these tests first to avoid MT global monitor error */
    mallocFailFlag = 1;
    if (JEMCC_EnterGlobalMonitor() == JNI_OK) {
        (void) fprintf(stderr,
                       "Error: Unexpected global monitor create success.\n");
        exit(1);
    }
    mallocFailFlag = 0;
    if (JEMCC_EnterGlobalMonitor() != JNI_OK) {
        (void) fprintf(stderr,
                       "Error: Failure during global monitor allocation\n");
        exit(1);
    }
    JEMCC_ExitGlobalMonitor();

    /* Try the quick and dirty multi-CPU test (N-spinning threads) */
    if (multiCheck != 0) {
        /* Clogging up four CPU's should be plenty! */
        for (i = 0; i < 4; i++) {
            threadId = JEMCC_CreateThread(NULL, threadSpinFn, NULL, 1);
            if (threadId == 0) {
                (void) fprintf(stderr,
                               "Error: failed on creation of thread %i.\n", i);
                break;
            }
            (void) fprintf(stderr, "Thread %i: Id: %x\n", i, threadId);
        }
        /* Let them spin for a while, but not too long */
        sleep(60);
        exit(0);
    }

    if ((testFlags & TEST_THREAD) != 0) {
        /* Conventional test sequences - handle malloc thread failures */
        mallocFailFlag = 1;
        threadId = JEMCC_CreateThread(NULL, threadSpinFn, NULL, 1);
        if (threadId != 0) {
            (void) fprintf(stderr,
                       "Error: unexpected thread create success (mem fail).\n");
            exit(1);
        }

        /* Check core thread methods (using global monitor to sequence) */
        (void) fprintf(stdout,
                       "Performing core thread and global monitor test\n");
        mallocFailFlag = 0;
        JEMCC_EnterGlobalMonitor();
        primThreadId = JEMCC_CreateThread(NULL, primThreadTestFn, NULL, 1);
        if (primThreadId == 0) {
            (void) fprintf(stderr, "Error: core test thread create failure.\n");
            exit(1);
        }
        JEMCC_ExitGlobalMonitor();

        /* Allow time for thread to complete */
        sleep(5);
    }

    if ((testFlags & TEST_SYSTEM) != 0) {
        /* Try local monitor wait/notify operations */
        (void) fprintf(stdout, "Performing local monitor wait/notify test\n");
        topSysMonitor = JEMCC_CreateSysMonitor(NULL);
        if (topSysMonitor == NULL) {
            (void) fprintf(stderr, "Error: test sys monitor create failed.\n");
            exit(1);
        }

        /* Try lock failure cases */
        if (JEMCC_ExitSysMonitor(topSysMonitor) != JEMCC_MONITOR_NOT_OWNER) {
            (void) fprintf(stderr, "Error: exit test allowed non-owner.\n");
            exit(1);
        }
        if (JEMCC_SysMonitorMilliWait(topSysMonitor, 10) !=
                                                 JEMCC_MONITOR_NOT_OWNER) {
            (void) fprintf(stderr, "Error: millWait test allowed non-owner.\n");
            exit(1);
        }

        /* Try a basic timed wait */
        JEMCC_EnterSysMonitor(topSysMonitor);
        testTime = time((time_t *) NULL);
        if (JEMCC_SysMonitorMilliWait(topSysMonitor, 5000) !=
                                                  JEMCC_MONITOR_TIMEOUT) {
            (void) fprintf(stderr, "Error: millWait test did not timeout.\n");
            exit(1);
        }
        timeDiff = time((time_t *) NULL) - testTime;
        if ((timeDiff < 4) || (timeDiff > 7)) {
            (void) fprintf(stderr,
                           "Error: millWait test gave strange result %i.\n",
                           timeDiff);
            exit(1);
        }
        if (JEMCC_ExitSysMonitor(topSysMonitor) != JEMCC_MONITOR_OK) {
            (void) fprintf(stderr,
                           "Error: millWait test did not exit monitor.\n");
            exit(1);
        }

        /* Try a single notify */
        if (JEMCC_SysMonitorWait(topSysMonitor) != JEMCC_MONITOR_NOT_OWNER) {
            (void) fprintf(stderr, "Error: wait test allowed non-owner.\n");
            exit(1);
        }
        if (JEMCC_SysMonitorNotify(topSysMonitor) != JEMCC_MONITOR_NOT_OWNER) {
            (void) fprintf(stderr, "Error: notify test allowed non-owner.\n");
            exit(1);
        }
        if (JEMCC_SysMonitorNotifyAll(topSysMonitor) !=
                                              JEMCC_MONITOR_NOT_OWNER) {
            (void) fprintf(stderr,
                           "Error: notifyAll test allowed non-owner.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, sysWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single wait thread create failure.\n");
            exit(1);
        }
        sleep(2);
        JEMCC_EnterSysMonitor(topSysMonitor);
        /* Kick thread, wait for it to kick back, kick it again to complete */
        (void) fprintf(stdout, "Main thread sending notifyAll\n");
        if (JEMCC_SysMonitorNotifyAll(topSysMonitor) != JEMCC_MONITOR_OK) {
            (void) fprintf(stderr, "Error: single notifyAll failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread entering wait state\n");
        if (JEMCC_SysMonitorMilliWait(topSysMonitor,
                                      10000) != JEMCC_MONITOR_OK) {
            (void) fprintf(stderr, "Error: single wait failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread sending notifyAll\n");
        if (JEMCC_SysMonitorNotifyAll(topSysMonitor) != JEMCC_MONITOR_OK) {
            (void) fprintf(stderr, "Error: single notifyAll failed.\n");
            exit(1);
        }
        JEMCC_ExitSysMonitor(topSysMonitor);
        sleep(2);

        /* Now handle a contention case */
        (void) fprintf(stdout, "\n");
        threadId = JEMCC_CreateThread(NULL, sysWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: multi wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, sysWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: multi wait thread create failure.\n");
            exit(1);
        }
        sleep(2);
        JEMCC_EnterSysMonitor(topSysMonitor);
        /* Do a single thread kick and wait for the kickback */
        (void) fprintf(stdout, "Main thread sending notify\n");
        if (JEMCC_SysMonitorNotify(topSysMonitor) != JEMCC_MONITOR_OK) {
            (void) fprintf(stderr, "Error: first multi notify failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread entering wait state\n");
        if (JEMCC_SysMonitorMilliWait(topSysMonitor, 2000) !=
                                                       JEMCC_MONITOR_TIMEOUT) {
            (void) fprintf(stderr, "Error: first multi timed wait failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread sending notifyAll\n");
        if (JEMCC_SysMonitorNotifyAll(topSysMonitor) != JEMCC_MONITOR_OK) {
            (void) fprintf(stderr, "Error: second multi notify failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread entering wait state\n");
        if (JEMCC_SysMonitorMilliWait(topSysMonitor,
                                      10000) != JEMCC_MONITOR_OK) {
            (void) fprintf(stderr, "Error: second multi wait failed.\n");
            exit(1);
        }
        JEMCC_ExitSysMonitor(topSysMonitor);

        /* Ensure timeout of MT thread cases and clean up */
        sleep(2);
        JEMCC_DestroySysMonitor(topSysMonitor);
    }

    if ((testFlags & TEST_OBJECT) != 0) {
        /* Build an object monitor for the test case */
        topObjMonitor = (JEMCC_Object *) JEMCC_Malloc(NULL,
                                                      sizeof(JEMCC_Object));
        if (topObjMonitor == NULL) {
            (void) fprintf(stderr, "Error: test obj monitor create failed.\n");
            exit(1);
        }
        topObjMonitor->objStateSet = 0xE2A8;

        /* Try the linear/uncontested cases first (need local env instance) */
        (void) fprintf(stdout,
                       "Performing basic uncontested object monitor tests\n");
        jenv = (JEM_JNIEnv *) JEMCC_Malloc(NULL, sizeof(JEM_JNIEnv));
        if (jenv == NULL) {
            (void) fprintf(stderr, "Error: env alloc failed.\n");
            exit(1);
        }
        jenv->freeObjLockQueue = NULL;
        jenv->objStateTxfrMonitor = JEMCC_CreateSysMonitor(NULL);
        if (jenv->objStateTxfrMonitor == NULL) {
            (void) fprintf(stderr, "Error: env system monitor failed.\n");
            exit(1);
        }
        jenv->objLockMonitor = JEMCC_CreateSysMonitor(NULL);
        if (jenv->objLockMonitor == NULL) {
            (void) fprintf(stderr, "Error: env lock monitor failed.\n");
            exit(1);
        }

        if (JEMCC_EnterObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: 1st object monitor lock failed.\n");
            exit(1);
        }
        if (JEMCC_EnterObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: 2nd object monitor lock failed.\n");
            exit(1);
        }
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: 1st obj monitor unlock failed.\n");
            exit(1);
        }
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: 2nd obj monitor unlock failed.\n");
            exit(1);
        }
        if (topObjMonitor->objStateSet != 0xE2A8) {
            (void) fprintf(stderr, "Object monitor state did not reset\n");
            exit(1);
        }

        /* Test lock failure cases */
        (void) fprintf(stdout, 
                       "Checking basic object monitor wait/notify errors\n");
        *exClassName = *exMsg = '\0';
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_ERR) {
            (void) fprintf(stderr, 
                           "Error: exit object test allowed non-owner.\n");
            exit(1);
        }
        checkException("IllegalMonitorState", "not entered for exit",
                       "exit without enter");
        if (JEMCC_ObjMonitorMilliWait((JNIEnv *) jenv, 
                                      topObjMonitor, 10) != JNI_ERR) {
            (void) fprintf(stderr, 
                           "Error: millWait object test allowed non-owner.\n");
            exit(1);
        }
        checkException("IllegalMonitorState", "not entered for wait",
                       "wait without enter");
        if (JEMCC_ObjMonitorNanoWait((JNIEnv *) jenv, 
                                     topObjMonitor, -1, 0) != JNI_ERR) {
            (void) fprintf(stderr, 
                           "Error: nanoWait object test allowed negative.\n");
            exit(1);
        }
        checkException("IllegalArgument", "Negative timeout",
                       "negative wait time");
        if (JEMCC_ObjMonitorNanoWait((JNIEnv *) jenv, 
                                     topObjMonitor, 1, -1) != JNI_ERR) {
            (void) fprintf(stderr, 
                           "Error: nanoWait object test allowed invalid.\n");
            exit(1);
        }
        checkException("IllegalArgument", "Invalid nanosecond timeout",
                       "invalid nanotime");
        if (JEMCC_ObjMonitorWait((JNIEnv *) jenv, topObjMonitor) != JNI_ERR) {
            (void) fprintf(stderr, "Error: obj wait test allowed non-owner.\n");
            exit(1);
        }
        checkException("IllegalMonitorState", "not entered for wait",
                       "wait without enter");
        if (JEMCC_ObjMonitorNotify((JNIEnv *) jenv, 
                                   topObjMonitor) != JNI_ERR) {
            (void) fprintf(stderr, 
                           "Error: obj notify test allowed non-owner.\n");
            exit(1);
        }
        checkException("IllegalMonitorState", "not entered for notify",
                       "notify without enter");
        if (JEMCC_ObjMonitorNotifyAll((JNIEnv *) jenv, 
                                      topObjMonitor) != JNI_ERR) {
            (void) fprintf(stderr,
                           "Error: obj notifyAll test allowed non-owner.\n");
            exit(1);
        }
        checkException("IllegalMonitorState", "not entered for notifyAll",
                       "notifyAll without enter");

        /* Try a basic timed wait */
        (void) fprintf(stdout, 
                       "Performing basic object monitor timed wait test\n");
        if (JEMCC_EnterObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object enter failed.\n");
            exit(1);
        }
        testTime = time((time_t *) NULL);
        if (JEMCC_ObjMonitorMilliWait((JNIEnv *) jenv, 
                                      topObjMonitor, 5000) != JNI_OK) {
            (void) fprintf(stderr, 
                           "Error: millWait object test did not timeout.\n");
            exit(1);
        }
        timeDiff = time((time_t *) NULL) - testTime;
        if ((timeDiff < 4) || (timeDiff > 7)) {
            (void) fprintf(stderr,
                           "Error: millWait obj test gave strange result %i.\n",
                           timeDiff);
            exit(1);
        }
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr,
                           "Error: millWait obj test did not exit monitor.\n");
            exit(1);
        }
        if (topObjMonitor->objStateSet != 0xE2A8) {
            (void) fprintf(stderr, "Object monitor state did not reset\n");
            exit(1);
        }

        /* Try a single notify */
        (void) fprintf(stdout, 
                       "Performing basic object monitor wait/notify test\n");
        threadId = JEMCC_CreateThread(NULL, objWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        sleep(2);
        if (JEMCC_EnterObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object enter failed.\n");
            exit(1);
        }
        /* Kick thread, wait for it to kick back, kick it again to complete */
        (void) fprintf(stdout, "Main thread sending object notifyAll\n");
        if (JEMCC_ObjMonitorNotifyAll((JNIEnv *) jenv, 
                                      topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: single obj notifyAll failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread entering wait state\n");
        if (JEMCC_ObjMonitorMilliWait((JNIEnv *) jenv, 
                                      topObjMonitor, 10000) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj single wait failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread sending obj notifyAll\n");
        if (JEMCC_ObjMonitorNotifyAll((JNIEnv *) jenv, 
                                      topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj single notifyAll failed.\n");
            exit(1);
        }
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj monitor exit failed.\n");
            exit(1);
        }
        sleep(2);
        if (topObjMonitor->objStateSet != 0xE2A8) {
            (void) fprintf(stderr, "Object wait monitor state did not reset\n");
            exit(1);
        }

        /* Now a notify sequence */
        (void) fprintf(stdout, 
                       "Performing multiple object monitor wait/notify test\n");
        threadId = JEMCC_CreateThread(NULL, objNotifyWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: notify obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objNotifyWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: notify obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objNotifyWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: notify obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objNotifyWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: notify obj wait thread create failure.\n");
            exit(1);
        }
        sleep(2);
        if (JEMCC_EnterObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object enter failed.\n");
            exit(1);
        }
        if (JEMCC_ObjMonitorNotify((JNIEnv *) jenv, 
                                   topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj notify failed.\n");
            exit(1);
        }
        if (JEMCC_ObjMonitorNotify((JNIEnv *) jenv, 
                                   topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj notify failed.\n");
            exit(1);
        }
        if (JEMCC_ObjMonitorNotifyAll((JNIEnv *) jenv, 
                                      topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj notify failed.\n");
            exit(1);
        }
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj monitor exit failed.\n");
            exit(1);
        }
        sleep(5);
        if (topObjMonitor->objStateSet != 0xE2A8) {
            (void) fprintf(stderr, "Object wait monitor state did not reset\n");
            exit(1);
        }

        /* How about a complex set of timeouts (queue management) */
        (void) fprintf(stdout, 
                       "Performing multiple object monitor timed wait tests\n");
        threadId = JEMCC_CreateThread(NULL, objTimedWaitTestFn, (void *) 3, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objTimedWaitTestFn, (void *) 5, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objTimedWaitTestFn, (void *) 7, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objTimedWaitTestFn, (void *) 4, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objTimedWaitTestFn, (void *) 6, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objTimedWaitTestFn, (void *) 4, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objTimedWaitTestFn, (void *) 3, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: single obj wait thread create failure.\n");
            exit(1);
        }
        sleep(10);

        /* Now handle a wait contention case */
        (void) fprintf(stdout, 
                       "Performing basic object monitor contention tests\n");
        threadId = JEMCC_CreateThread(NULL, objWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: multi obj wait thread create failure.\n");
            exit(1);
        }
        threadId = JEMCC_CreateThread(NULL, objWaitTestFn, NULL, 1);
        if (threadId == 0) {
            (void) fprintf(stderr,
                           "Error: multi obj wait thread create failure.\n");
            exit(1);
        }
        sleep(2);
        if (JEMCC_EnterObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj monitor enter failed.\n");
            exit(1);
        } 
        /* Do a single thread kick and wait for the kickback */
        (void) fprintf(stdout, "Main thread sending obj notify\n");
        if (JEMCC_ObjMonitorNotify((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: first obj multi notify failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread entering obj wait state\n");
        if (JEMCC_ObjMonitorMilliWait((JNIEnv *) jenv, 
                                      topObjMonitor, 2000) != JNI_OK) {
            (void) fprintf(stderr, "Error: first obj multi timed wait fail.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread sending notifyAll\n");
        if (JEMCC_ObjMonitorNotifyAll((JNIEnv *) jenv, 
                                      topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: second obj multi notify failed.\n");
            exit(1);
        }
        (void) fprintf(stdout, "Main thread entering wait state\n");
        if (JEMCC_ObjMonitorMilliWait((JNIEnv *) jenv, 
                                      topObjMonitor, 10000) != JNI_OK) {
            (void) fprintf(stderr, "Error: second obj multi wait failed.\n");
            exit(1);
        }
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: obj monitor exit failed.\n");
            exit(1);
        }
        sleep(2);
        if (topObjMonitor->objStateSet != 0xE2A8) {
            (void) fprintf(stderr, "Object monitor state did not reset\n");
            exit(1);
        }

        /* Finally, turn loose a bunch of racing threads with collisions */
        (void) fprintf(stdout,
                       "Performing fully contested object monitor tests\n");
        for (i = 0; i < 4; i++) {
            threadId = JEMCC_CreateThread(NULL, objMonitorTestFn, 
                                          (void *) i, 1);
            if (threadId == 0) {
                (void) fprintf(stderr, 
                               "Error: object conflict thread %i failed.\n", i);
                exit(1);
            }
        }
        if (JEMCC_EnterObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object monitor lock failed.\n");
            exit(1);
        }
        while (objExitCount < i) {
            if (JEMCC_ObjMonitorWait((JNIEnv *) jenv, 
                                     topObjMonitor) != JNI_OK) {
                (void) fprintf(stderr, "Error: object monitor wait failed.\n");
                exit(1);
            }
        }
        if (JEMCC_ExitObjMonitor((JNIEnv *) jenv, topObjMonitor) != JNI_OK) {
            (void) fprintf(stderr, "Error: object monitor release failed.\n");
            exit(1);
        }
        if (topObjMonitor->objStateSet != 0xE2A8) {
            (void) fprintf(stderr, "Object monitor state did not reset\n");
            exit(1);
        }

        sleep(5);

        /* Clean up to ensure truly complete memcheck */
        JEMCC_Free(topObjMonitor);
        lockEntry = jenv->freeObjLockQueue;
        while (lockEntry != NULL) {
            nextEntry = lockEntry->nextEntry;
            JEMCC_Free(lockEntry);
            lockEntry = nextEntry;
        }
        JEMCC_DestroySysMonitor(jenv->objLockMonitor);
        JEMCC_DestroySysMonitor(jenv->objStateTxfrMonitor);
        JEMCC_Free(jenv);
    }

    exit(0);
}

/* Local methods to avoid full library inclusion */
void *JEMCC_Malloc(JNIEnv *env, juint size) {
    if (mallocFailFlag != 0) {
        (void) fprintf(stderr, "Error[local]: Simulated malloc failure\n");
        return NULL;
    }
    return calloc(1, size);
}

void JEMCC_Free(void *block) {
    free(block);
}

void JEMCC_ThrowStdThrowableIdx(JNIEnv *env, JEMCC_VMClassIndex idx,
                                JEMCC_Object *causeThrowable, const char *msg) {
    char *className = "unknown";
    if (msg == NULL) msg = "(null)";

    if (idx == JEMCC_Class_IllegalMonitorStateException) {
        className = "java.lang.IllegalMonitorStateException";
    } else if (idx == JEMCC_Class_IllegalArgumentException) {
        className = "java.lang.IllegalArgumentException";
    } else {
        (void) fprintf(stderr, "Fatal error: unexpected exception index %i.\n",
                               idx);
        exit(1);
    }
    (void) fprintf(stderr, "Error[%s]: %s.\n", className, msg);

    (void) strcpy(exClassName, className);
    (void) strcpy(exMsg, msg);
}
