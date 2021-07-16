/**
 * JEMCC test program to test the dynalib and foreign function system methods.
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

/* Read the jni/jem internal details */
#include "jem.h"

/* Test flags/data values for condition setups and tests */
#ifdef ENABLE_ERRORSWEEP
int testFailureCurrentCount, testFailureCount;

int JEM_CheckErrorSweep(int sweepType) {
    /* Note: don't care about sweep type */
    testFailureCurrentCount++;
    if ((testFailureCount >= 0) &&
        (testFailureCurrentCount == testFailureCount)) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}
#endif

/* Test flags/data values for condition setups and tests */
typedef void (*voidTestFn)();

/* Main program will send the dynalib/ffi interfaces through their paces */
int main(int argc, char *argv[]) {
    JEM_DynaLibLoader libLoader;
    JEM_DynaLib libHandle;
    JEM_DynaLibSymbol fnHandle;
    union JEM_DescriptorInfo methodDesc, retDesc, argDesc[255];
    JEMCC_ReturnValue retVal, argList[16];
    char *libName;

    libLoader = JEM_DynaLibLoaderInit();
    if (libLoader == NULL) {
        (void) fprintf(stderr, "Error: dynaLibLoaderInit has failed\n");
        exit(1);
    }

#ifdef ENABLE_ERRORSWEEP
    /* Try the basic memory test failures */
    testFailureCurrentCount = 0;
    testFailureCount = 1;
    if (JEM_MapLibraryName(NULL, "testlib") != NULL) {
        (void) fprintf(stderr, "Error: expected mapLibraryName() to fail\n");
        exit(1);
    }
/*
    testFailureCurrentCount = 0;
    if (JEM_DynaLibLoad(libLoader, "libffitest.so", NULL) != NULL) {
        (void) fprintf(stderr, "Error: expected dynaLibLoad() to fail\n");
        exit(1);
    }
*/
#endif

    /* Try a library failure */
    if (JEM_DynaLibLoad(libLoader, "nocando.so", NULL) != NULL) {
        (void) fprintf(stderr, "Error: expected dynaLibLoad() to fail\n");
        exit(1);
    }

    /* Try a success */
    libName = JEM_MapLibraryName(NULL, "ffitest");
    if (libName == NULL) {
        (void) fprintf(stderr, "Error: mapLibraryName() has failed\n");
        exit(1);
    }
    libHandle = JEM_DynaLibLoad(libLoader, libName, NULL);
    if (libHandle == NULL) {
        (void) fprintf(stderr, "Error: dynaLibLoad of %s has failed\n", 
                               libName);
        exit(1);
    }
    JEMCC_Free(libName);

    /* Check out the function handle grabber */
    if (JEM_DynaLibGetSymbol(libHandle, "notachance") != NULL) {
        (void) fprintf(stderr, "Error: expected dynaLibGetSymbol() to fail\n");
        exit(1);
    }
    libName = JEM_DynaLibErrorMsg();
    (void) fprintf(stdout, "Expected dynalib failure message: %s\n",
                           ((libName != NULL) ? libName : "(null)"));

    /* Try a basic (direct) function call */
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "voidTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, "Error: dynaLibGetSymbol() has failed\n");
        exit(1);
    }
    (*((voidTestFn) fnHandle))();

    /* Now try some FFI calls - first the void(void) method */
    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, NULL) != JNI_OK) {
        (void) fprintf(stderr, "Error: voidTestFn() ffi call failed\n");
        exit(1);
    }

    /* Try all of the various return types */
    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = BASETYPE_Boolean;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "boolRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(boolRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: boolRetTestFn() ffi call failed\n");
        exit(1);
    }
    if (retVal.intVal != 1) {
        (void) fprintf(stderr, 
                       "Error: boolRetTestFn() returned wrong value\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = BASETYPE_Byte;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "byteRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(byteRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: byteRetTestFn() ffi call failed\n");
        exit(1);
    }
    if (retVal.intVal != 12) {
        (void) fprintf(stderr, 
                       "Error: byteRetTestFn() returned wrong value\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = BASETYPE_Short;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "shortRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(shortRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: shortRetTestFn() ffi call failed\n");
        exit(1);
    }
    if (retVal.intVal != 1234) {
        (void) fprintf(stderr, 
                       "Error: shortRetTestFn() returned wrong value\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = BASETYPE_Int;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "intRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(intRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: intRetTestFn() ffi call failed\n");
        exit(1);
    }
    if (retVal.intVal != 1234567) {
        (void) fprintf(stderr, 
                       "Error: intRetTestFn() returned wrong value\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = BASETYPE_Long;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "longRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(longRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: longRetTestFn() ffi call failed\n");
        exit(1);
    }
    if (retVal.longVal != 12345678901L) {
        (void) fprintf(stderr, 
                       "Error: longRetTestFn() returned wrong value\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = BASETYPE_Float;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "fltRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(fltRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: fltRetTestFn() ffi call failed\n");
        exit(1);
    }
    if ((retVal.fltVal <= 1.19999) || (retVal.fltVal >= 1.20001)) {
        (void) fprintf(stderr, 
                       "Error: fltRetTestFn() returned wrong value\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = BASETYPE_Double;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "dblRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(dblRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: dblRetTestFn() ffi call failed\n");
        exit(1);
    }
    if ((retVal.dblVal <= 9.87654320) || (retVal.dblVal >= 9.87654322)) {
        (void) fprintf(stderr, 
                       "Error: dblRetTestFn() returned wrong value\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    argDesc[0].generic.tag = DESCRIPTOR_EndOfList;
    retDesc.generic.tag = DESCRIPTOR_ObjectType;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "objRetTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(objRetTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                NULL, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: objRetTestFn() ffi call failed\n");
        exit(1);
    }
    if (retVal.objVal != (JEMCC_Object *) 0xDEAD) {
        (void) fprintf(stderr, 
                       "Error: objRetTestFn() returned wrong value\n");
        exit(1);
    }

    /* Try different argument types */
    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = BASETYPE_Boolean;
    argDesc[1].generic.tag = BASETYPE_Boolean;
    argDesc[2].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].intVal = 0;
    argList[1].intVal = 1;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "boolArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(boolArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: boolArgTestFn() ffi call failed\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = BASETYPE_Byte;
    argDesc[1].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].intVal = 12;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "byteArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(byteArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: byteArgTestFn() ffi call failed\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = BASETYPE_Short;
    argDesc[1].generic.tag = BASETYPE_Short;
    argDesc[2].generic.tag = BASETYPE_Short;
    argDesc[3].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].intVal = 1;
    argList[1].intVal = 2;
    argList[2].intVal = 3;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "shortArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(shortArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: shortArgTestFn() ffi call failed\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = BASETYPE_Int;
    argDesc[1].generic.tag = BASETYPE_Int;
    argDesc[2].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].intVal = 12;
    argList[1].intVal = 6;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "intArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(intArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: intArgTestFn() ffi call failed\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = BASETYPE_Long;
    argDesc[1].generic.tag = BASETYPE_Long;
    argDesc[2].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].longVal = 12345678901L;
    argList[1].longVal = 98765432109L;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "longArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(longArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: longArgTestFn() ffi call failed\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = BASETYPE_Float;
    argDesc[1].generic.tag = BASETYPE_Float;
    argDesc[2].generic.tag = BASETYPE_Float;
    argDesc[3].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].fltVal = 1.0;
    argList[1].fltVal = 2.0;
    argList[2].fltVal = 3.0;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "fltArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(fltArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: fltArgTestFn() ffi call failed\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = BASETYPE_Double;
    argDesc[1].generic.tag = BASETYPE_Double;
    argDesc[2].generic.tag = BASETYPE_Double;
    argDesc[3].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].dblVal = 1.0;
    argList[1].dblVal = 6.0;
    argList[2].dblVal = 12.0;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "dblArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(dblArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: dblArgTestFn() ffi call failed\n");
        exit(1);
    }

    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = NULL;
    argDesc[0].generic.tag = DESCRIPTOR_ObjectType;
    argDesc[1].generic.tag = DESCRIPTOR_ObjectType;
    argDesc[2].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].objVal = (jobject) 0xDEAD;
    argList[1].objVal = (jobject) 0xCAFE;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "objArgTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(objArgTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, NULL, fnHandle, &methodDesc, 
                                argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: objArgTestFn() ffi call failed\n");
        exit(1);
    }

    /* Mix and match */
    methodDesc.method_info.tag = DESCRIPTOR_MethodType;
    methodDesc.method_info.paramDescriptor = argDesc;
    methodDesc.method_info.returnDescriptor = &retDesc;
    retDesc.generic.tag = BASETYPE_Double;
    argDesc[0].generic.tag = BASETYPE_Short;
    argDesc[1].generic.tag = BASETYPE_Float;
    argDesc[2].generic.tag = BASETYPE_Boolean;
    argDesc[3].generic.tag = BASETYPE_Long;
    argDesc[4].generic.tag = BASETYPE_Int;
    argDesc[5].generic.tag = BASETYPE_Double;
    argDesc[6].generic.tag = DESCRIPTOR_ObjectType;
    argDesc[7].generic.tag = BASETYPE_Boolean;
    argDesc[8].generic.tag = DESCRIPTOR_EndOfList;
    argList[0].intVal = 12;
    argList[1].fltVal = 6.0;
    argList[2].intVal = JNI_FALSE;
    argList[3].longVal = 99999999999L;
    argList[4].intVal = 6;
    argList[5].dblVal = 1.0;
    argList[6].objVal = (jobject) 0xDEADCAFE;
    argList[7].intVal = 0;
    fnHandle = JEM_DynaLibGetSymbol(libHandle, "instMixedTestFn");
    if (fnHandle == NULL) {
        (void) fprintf(stderr, 
                       "Error: dynaLibGetSymbol(instMixedTestFn) failed\n");
        exit(1);
    }
    if (JEM_CallForeignFunction(NULL, (jobject) 0xAABBCCDD, fnHandle, 
                                &methodDesc, argList, &retVal) != JNI_OK) {
        (void) fprintf(stderr, "Error: instMixedTestFn() ffi call failed\n");
        exit(1);
    }
    if (retVal.dblVal != -12.0) {
        (void) fprintf(stderr, "Error: instMixedTestFn() bad return value\n");
        exit(1);
    }

    exit(0);
}

/* Local methods to avoid full library inclusion */
void *JEMCC_Malloc(JNIEnv *env, juint size) {
#ifdef ENABLE_ERRORSWEEP
    if (JEM_CheckErrorSweep(ES_MEM) == JNI_TRUE) {
        (void) fprintf(stderr, "Error[local]: Simulated malloc failure\n");
        return NULL;
    }
#endif
    return calloc(1, size);
}

void JEMCC_Free(void *block) {
    free(block);
}

void JEMCC_ThrowStdThrowableIdx(JNIEnv *env, JEMCC_VMClassIndex idx,
                                JEMCC_Object *causeThrowable, const char *msg) {
/*
    char *className = "unknown";
    if (msg == NULL) msg = "(null)";

    if (idx == JEMCC_Class_IOException) {
        className = "java.io.IOException";
    } else if (idx == JEMCC_Class_OutOfMemoryError) {
        className = "java.lang.OutOfMemoryError";
    } else if (idx == JEMCC_Class_InternalError) {
        className = "java.lang.InternalError";
    } else {
        (void) fprintf(stderr, "Fatal error: unexpected exception index %i.\n",
                               idx);
        exit(1);
    }
    (void) fprintf(stderr, "Error[%s]: %s.\n", className, msg);

    (void) strcpy(exClassName, className);
    (void) strcpy(exMsg, msg);
*/
}
