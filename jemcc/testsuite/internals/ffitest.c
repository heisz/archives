/**
 * Test methods for the libffi test program (foreign function instances).
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

/* Need to test the various JNI type related methods */
#include "jni.h"

void voidTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Void test method has been called.\n");
}

jboolean boolRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Boolean return test method has been called.\n");
    return 1;
}

jbyte byteRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Byte return test method has been called.\n");
    return 12;
}

jshort shortRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Short return test method has been called.\n");
    return 1234;
}

jint intRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Int return test method has been called.\n");
    return 1234567;
}

jlong longRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Long return test method has been called.\n");
    return 12345678901L;
}

jfloat fltRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Float return test method has been called.\n");
    return 1.2;
}

jdouble dblRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Double return test method has been called.\n");
    return 9.87654321;
}

jobject objRetTestFn(JNIEnv *env) {
    (void) fprintf(stdout, "Object return test method has been called.\n");
    return (jobject) 0xDEAD;
}

void boolArgTestFn(JNIEnv *env, jboolean argA, jboolean argB) {
    (void) fprintf(stdout, "Boolean argument test method has been called.\n");
    if (!argB) {
        (void) fprintf(stderr, "Error: invalid second boolean argument\n");
        exit(1);
    }
    if (argA) {
        (void) fprintf(stderr, "Error: invalid first boolean argument\n");
        exit(1);
    }
}

void byteArgTestFn(JNIEnv *env, jbyte argA) {
    (void) fprintf(stdout, "Byte argument test method has been called.\n");
    if (argA != 12) {
        (void) fprintf(stderr, "Error: invalid byte argument\n");
        exit(1);
    }
}

void shortArgTestFn(JNIEnv *env, jshort argA, jshort argB, jshort argC) {
    (void) fprintf(stdout, "Short argument test method has been called.\n");
    if (argA != 1) {
        (void) fprintf(stderr, "Error: invalid first short argument\n");
        exit(1);
    }
    if (argB != 2) {
        (void) fprintf(stderr, "Error: invalid second short argument\n");
        exit(1);
    }
    if (argC != 3) {
        (void) fprintf(stderr, "Error: invalid third short argument\n");
        exit(1);
    }
}

void intArgTestFn(JNIEnv *env, jint argA, jint argB) {
    (void) fprintf(stdout, "Int argument test method has been called.\n");
    if (argA != 12) {
        (void) fprintf(stderr, "Error: invalid first int argument\n");
        exit(1);
    }
    if (argB != 6) {
        (void) fprintf(stderr, "Error: invalid second int argument\n");
        exit(1);
    }
}

void longArgTestFn(JNIEnv *env, jlong argA, jlong argB) {
    (void) fprintf(stdout, "Long argument test method has been called.\n");
    if (argA != 12345678901L) {
        (void) fprintf(stderr, "Error: invalid first long argument\n");
        exit(1);
    }
    if (argB != 98765432109L) {
        (void) fprintf(stderr, "Error: invalid second long argument\n");
        exit(1);
    }
}

void fltArgTestFn(JNIEnv *env, jfloat argA, jfloat argB, jfloat argC) {
    (void) fprintf(stdout, "Float argument test method has been called.\n");
    if ((argA <= 0.999999) || (argA >= 1.000001)) {
        (void) fprintf(stderr, "Error: invalid first float argument\n");
        exit(1);
    }
    if ((argB <= 1.999999) || (argB >= 2.000001)) {
        (void) fprintf(stderr, "Error: invalid second float argument\n");
        exit(1);
    }
    if ((argC <= 2.999999) || (argC >= 3.000001)) {
        (void) fprintf(stderr, "Error: invalid third float argument\n");
        exit(1);
    }
}

void dblArgTestFn(JNIEnv *env, jdouble argA, jdouble argB, jdouble argC) {
    (void) fprintf(stdout, "Double argument test method has been called.\n");
    if ((argA <= 0.999999) || (argA >= 1.000001)) {
        (void) fprintf(stderr, "Error: invalid first double argument\n");
        exit(1);
    }
    if ((argB <= 5.999999) || (argB >= 6.000001)) {
        (void) fprintf(stderr, "Error: invalid second double argument\n");
        exit(1);
    }
    if ((argC <= 11.999999) || (argC >= 12.000001)) {
        (void) fprintf(stderr, "Error: invalid third double argument\n");
        exit(1);
    }
}

void objArgTestFn(JNIEnv *env, jobject argA, jobject argB) {
    (void) fprintf(stdout, "Object argument test method has been called.\n");
    if (argA != (jobject) 0xDEAD) {
        (void) fprintf(stderr, "Error: invalid first object argument\n");
        exit(1);
    }
    if (argB != (jobject) 0xCAFE) {
        (void) fprintf(stderr, "Error: invalid second object argument\n");
        exit(1);
    }
}

jint staticMixedTestFn(JNIEnv *env, jboolean argA, jdouble argB, jshort argC,
                       jlong argD, jfloat argE, jint argF, jobject argG) {
    (void) fprintf(stdout, "Mixed static test method has been called.\n");
    if (!argA) {
        (void) fprintf(stderr, "Error: invalid first mixed static argument\n");
        exit(1);
    }
    if ((argB <= 5.999999) || (argB >= 6.000001)) {
        (void) fprintf(stderr, "Error: invalid second mixed static argument\n");
        exit(1);
    }
    if (argC != 12) {
        (void) fprintf(stderr, "Error: invalid third mixed static argument\n");
        exit(1);
    }
    if (argD != 99999999999L) {
        (void) fprintf(stderr, "Error: invalid fourth mixed static argument\n");
        exit(1);
    }
    if ((argE <= 0.999999) || (argE >= 1.000001)) {
        (void) fprintf(stderr, "Error: invalid fifth mixed static argument\n");
        exit(1);
    }
    if (argF != 6) {
        (void) fprintf(stderr, "Error: invalid sixth mixed static argument\n");
        exit(1);
    }
    if (argG != (jobject) 0xDEADCAFE) {
        (void) fprintf(stderr, "Error: invalid 7th mixed static argument\n");
        exit(1);
    }

    return -12;
}

jdouble instMixedTestFn(JNIEnv *env, jobject inst, jshort argA, jfloat argB, 
                        jboolean argC, jlong argD, jint argE, jdouble argF,
                        jobject argG, jboolean argH) {
    (void) fprintf(stdout, "Mixed instance test method has been called.\n");
    if (inst != (jobject) 0xAABBCCDD) {
        (void) fprintf(stderr, "Error: invalid instance argument\n");
        exit(1);
    }
    if (argA != 12) {
        (void) fprintf(stderr, "Error: invalid first mixed inst argument\n");
        exit(1);
    }
    if ((argB <= 5.999999) || (argB >= 6.000001)) {
        (void) fprintf(stderr, "Error: invalid second mixed inst argument\n");
        exit(1);
    }
    if (argC) {
        (void) fprintf(stderr, "Error: invalid third mixed inst argument\n");
        exit(1);
    }
    if (argD != 99999999999L) {
        (void) fprintf(stderr, "Error: invalid fourth mixed inst argument\n");
        exit(1);
    }
    if (argE != 6) {
        (void) fprintf(stderr, "Error: invalid fifth mixed inst argument\n");
        exit(1);
    }
    if ((argF <= 0.999999) || (argF >= 1.000001)) {
        (void) fprintf(stderr, "Error: invalid sixth mixed inst argument\n");
        exit(1);
    }
    if (argG != (jobject) 0xDEADCAFE) {
        (void) fprintf(stderr, "Error: invalid 7th mixed inst argument\n");
        exit(1);
    }
    if (argH) {
        (void) fprintf(stderr, "Error: invalid 8th mixed inst argument\n");
        exit(1);
    }

    return -12.0;
}
