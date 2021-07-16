/**
 * Static JEMCC test program to execute the Mauve testcases.
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

/* Read the JNI/JEMCC internal details */
#include "jem.h"
#include "jemcc.h"

#ifdef ENABLE_ERRORSWEEP
int testFailureCurrentCount, testFailureCount;

int JEM_CheckErrorSweep(int sweepType) {
    testFailureCurrentCount++;
    if ((testFailureCount >= 0) &&
        (testFailureCurrentCount == testFailureCount)) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}
#endif

/* Forward declarations for local help/version programs */
static void usage(char *progName, int exitCode);

/* General exit handler which properly cleans up JVM instances */
static void javaExit(JavaVM *vm, JNIEnv *env, int exitCode);

/* Initializers defined by the various JEMCC VM libraries */
extern JEMCC_MethodData JEMCC_VMTestInterfaceMethods[];

/* Main program behaves just like the "java" program */
int main(int argc, char *argv[]) {
    JavaVM *vm;
    JNIEnv *env;
    JDK1_1InitArgs jvmArgs;
    char *ptr = NULL, *envStr, *target = NULL;
    int i, rc, index;

    jclass mainClass = NULL, strClass, objClass;
    jmethodID mainMethodID;
    jobject argList;
    jstring arg;

    /* Initialize the Java default argument information */
    jvmArgs.version = JNI_VERSION_1_1;
    if ((rc = JNI_GetDefaultJavaVMInitArgs(&jvmArgs)) != JNI_OK) {
        (void) fprintf(stderr, 
                       "Error: unable to initialize JEMCC VM arguments (%i).\n",
                       rc);
        exit(1);
    }

    /* Parse the primary command line arguments */
    jvmArgs.classpath = NULL;
    jvmArgs.libpath = NULL;
    for (index = 1; index < argc; index++) {
        /* Process arguments or find class/jar target */
        if ((strcmp(argv[index], "-cp") == 0) ||
            (strcmp(argv[index], "-classpath") == 0)) {
            if (index >= (argc - 1)) {
                (void) fprintf(stderr, 
                               "Missing argument to classpath option.\n");
                usage(argv[0], 1);
            }
            ptr = jvmArgs.classpath = strdup(argv[index + 1]);
            index++;
        } else if ((strcmp(argv[index], "-lp") == 0) ||
                   (strcmp(argv[index], "-libpath") == 0)) {
            if (index >= (argc - 1)) {
                (void) fprintf(stderr, 
                               "Missing argument to libpath option.\n");
                usage(argv[0], 1);
            }
            ptr = jvmArgs.libpath = strdup(argv[index + 1]);
            index++;
        } else if ((strcmp(argv[index], "-?") == 0) ||
                   (strcmp(argv[index], "-h") == 0) ||
                   (strcmp(argv[index], "-help") == 0)) {
            ptr = argv[index];
            usage(argv[0], 0);
        } else if (strncmp(argv[index], "-verbose", 8) == 0) {
            /* TODO */
        } else {
            ptr = target = argv[index++];
            break;
        }

        /* Catch unexpected memory violation */
        if (ptr == NULL) {
            (void) fprintf(stderr, 
                           "Error: Out of memory for command options.\n");
            exit(1);
        }
    }
    if (target == NULL) usage(argv[0], 1);
    argc -= index;

    /* Handle defaults */
    if (jvmArgs.classpath == NULL) {
        envStr = getenv("CLASSPATH");
        if (envStr == NULL) envStr = ".";

        jvmArgs.classpath = strdup(envStr);
        if (jvmArgs.classpath == NULL) {
            (void) fprintf(stderr, 
                           "Error: Out of memory for command options.\n");
            exit(1);
        }
    }
    if (jvmArgs.libpath == NULL) {
        envStr = getenv("LD_LIBRARY_PATH");
        if (envStr == NULL) envStr = ".";

        jvmArgs.libpath = strdup(envStr);
        if (jvmArgs.libpath == NULL) {
            (void) fprintf(stderr, 
                           "Error: Out of memory for command options.\n");
            exit(1);
        }
    }

    (void) fprintf(stderr, "JEMCCTestHarness starting %s\n", target);

    /* Create the Java VM instance */
    if ((rc = JNI_CreateJavaVM(&vm, &env, &jvmArgs)) != JNI_OK) {
        (void) fprintf(stderr, 
                       "Error: unable to create Jem VM instance (%i).\n", rc);
        exit(1);
    }

    if (JEMCC_LocateClass(env, NULL, "java.lang.Object", 
                          JNI_FALSE, (JEMCC_Class **) &objClass) != JNI_OK) {
        (void) fprintf(stderr, "Error: unable to locate Object class\n");
        exit(1);
    }

    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC | ACC_FINAL,
                              "wrdg.mauve.VMTestInterface", 
                              (JEMCC_Class *) objClass,
                              NULL, 0, JEMCC_VMTestInterfaceMethods, 4, NULL,
                              NULL, 0, NULL, 0, NULL, NULL);
    if (rc != JNI_OK) {
        (void) fprintf(stderr,
                       "Error: failure in VMTestInterface create (%i).\n", rc);
        exit(1);
    }

    ptr = strdup(target);
    target = ptr;
    while (*ptr != '\0') {
        if (*ptr == '.') *ptr = '/';
        ptr++;
    }
    mainClass = (*env)->FindClass(env, target);
    free(target);
    if (mainClass == NULL) javaExit(vm, env, 1);

    /* Locate the method */
    mainMethodID = (*env)->GetStaticMethodID(env, mainClass, "main",
                                             "([Ljava/lang/String;)V");
    if (mainMethodID == NULL) javaExit(vm, env, 1);

    /* Construct the program argument list strings */
    strClass = (*env)->FindClass(env, "java/lang/String");
    if (strClass == NULL) javaExit(vm, env, 1);
    argList = (*env)->NewObjectArray(env, (argc < 0) ? 0 : argc, 
                                     strClass, NULL);
    if (argList == NULL) javaExit(vm, env, 1);
    i = 0;
    while (argc > 0) {
        arg = (*env)->NewStringUTF(env, argv[index++]);
        if (arg == NULL) javaExit(vm, env, 1);
        (*env)->SetObjectArrayElement(env, argList, i++, arg);
        (*env)->DeleteLocalRef(env, arg);
        argc--;
    }

    /* Finally make the call */
    (*env)->CallStaticVoidMethod(env, mainClass, mainMethodID, argList);

    /* Clean up and bail out */
    javaExit(vm, env, 0);
    exit(0);
}

/* Clean up JVM and exit, in case of program termination or startup error */
static void javaExit(JavaVM *vm, JNIEnv *env, int exitCode) {
    /* Document any final JVM errors */
    if ((*env)->ExceptionOccurred(env) != NULL) (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);

    /* Ensure main thread detaches (cleans up) prior to JVM exit */
    if ((*vm)->DetachCurrentThread(vm) != JNI_OK) {
        (void) fprintf(stderr, "Error: unable to detach main VM thread.\n");
        exitCode = 1;
    }

    /* Destroy the Java VM to ensure memory cleanup/tests can complete */
    /* NOTE: Sun's VM always returns error here.... */
    if ((*vm)->DestroyJavaVM(vm) != JNI_OK) {
        (void) fprintf(stderr, "Error: failure during VM destruction.\n");
        exitCode = 1;
    }

    if (exitCode != 0) {
        (void) fprintf(stderr, "Errors have occurred during execution.\n");
    } else {
        (void) fprintf(stderr, "Test program completed successfully.\n");
    }
    exit(exitCode);
}

/* Provide assistance to the poor command line user */
static void usage(char *progName, int exitCode) {
    (void) fprintf(stderr, 
             "\nUsage: %s [-options] className [args]\n", progName);
    (void) fprintf(stderr, 
             "  or   %s -jar [-options] jarName [args]\n\n", progName);
    (void) fprintf(stderr, "Options:\n");
    (void) fprintf(stderr, 
             "    -cp <pathinfo>\n");
    (void) fprintf(stderr, 
             "    -classpath <pathinfo>    define search path for classes\n");
    (void) fprintf(stderr, 
             "    -D<name>=<value>         define a Java system property\n");
    (void) fprintf(stderr, 
             "    -?\n");
    (void) fprintf(stderr, 
             "    -h\n");
    (void) fprintf(stderr, 
             "    -help                    display this help information\n");
    (void) fprintf(stderr, 
             "    -verbose:[class|gc|jni]  enable verbose internals logging\n");
    exit(exitCode);
}
