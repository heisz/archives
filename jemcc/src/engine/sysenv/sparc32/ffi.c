/**
 * JEMCC system/environment functions to support foreign function (JNI) calls.
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

/* Prototype method defined in the fficall.s object file */
extern void JEM_FFICallFn(jint frameSize, void *fnRef, void *argStack, 
                          jint argSize, jint retType);

/**
 * Method by which foreign functions (not directly integrated into the
 * JEMCC VM) are called.  Used exclusively by the JNI method calling
 * methods.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     thisObj - if non-NULL, method is non-static and this is the
 *               'this' object reference
 *     fnRef - the reference to the foreign function to be called
 *     fnDesc - the Java descriptor of the JNI method being called.  Used
 *              to parse/format both the function arguments and the return
 *              information.
 *     argList - an array of the arguments to the foreign function.  Must
 *               contain at least the number of members indicated by the
 *               function descriptor.
 *     retVal - reference to the structure which is to contain the return
 *              value from the foreign function.  May be NULL for void
 *              functions.
 *
 * Returns:
 *     JNI_OK if the method setup and call was successful, JNI_ERR if
 *     a memory allocation or other error occurred (an exception will
 *     have been thrown in the current environment).  Note that the
 *     native method may itself throw an exception - this will not be
 *     caught by this method (should be managed by the caller).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     InternalError - an invalid argument condition occurred
 *     StackOverflowError - unable to allocate stack segment for foreign call
 *     Any other exception thrown within the foreign function.
 */
jint JEM_CallForeignFunction(JNIEnv *env, JEMCC_Object *thisObj,
                             void *fnRef, union JEM_DescriptorInfo *fnDesc,
                             JEMCC_ReturnValue *argList,
                             JEMCC_ReturnValue *retVal) {

    jint index, argSize, frameSize, retType = 0;
    union JEM_DescriptorInfo *descPtr;
    void *argStack = NULL;
    unsigned char *ptr;

    /* Scan the method information and determine the argument stack size */
    argSize = 4;
    if (thisObj != NULL) argSize += 4;
    descPtr = fnDesc->method_info.paramDescriptor;
    while (descPtr->generic.tag != DESCRIPTOR_EndOfList) {
        switch (descPtr->generic.tag) {
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                argSize += 4;
                break;
            case BASETYPE_Byte:
            case BASETYPE_Char:
            case BASETYPE_Int:
            case BASETYPE_Short:
            case BASETYPE_Boolean:
            case BASETYPE_Float:
                argSize += 4;
                break;
            case BASETYPE_Long:
            case BASETYPE_Double:
                argSize += 8;
                break;
            default:
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError,
                                           NULL, "Invalid ffi parameter type");
                return JNI_ERR;
        }
        descPtr++;
    }

    /* Always need space for at least 6 arguments (guarantees return) */
    if (argSize < 24) argSize = 24;

    /* Determine SPARC frame size - must be at least 96 and a multiple of 8 */
    frameSize = 64 + 4 + argSize;
    if (frameSize < 96) {
        frameSize = 96;
    } else {
        if ((frameSize & 0x7) != 0) {
            frameSize = (frameSize & 0xFFFFFFF8) + 8;
        }
    }

    /* Allocate a temporary array on the local stack */
    argStack = alloca(argSize);
    if (argStack == NULL) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_StackOverflowError,
                                   NULL, "Ffi stack allocation failed");
        return JNI_ERR;
    }

    /* Build the function argument stack (local copy) */
    index = 0;
    ptr = (unsigned char *) argStack;
    *((JNIEnv **) ptr) = env;
    ptr += 4;
    if (thisObj != NULL) {
        *((JEMCC_Object **) ptr) = thisObj;
        ptr += 4;
    }
    descPtr = fnDesc->method_info.paramDescriptor;
    while (descPtr->generic.tag != DESCRIPTOR_EndOfList) {
        switch (descPtr->generic.tag) {
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                *((jobject *) ptr) = argList[index].objVal;
                ptr += 4;
                break;
            case BASETYPE_Byte:
            case BASETYPE_Char:
            case BASETYPE_Int:
            case BASETYPE_Short:
            case BASETYPE_Boolean:
                *((jint *) ptr) = argList[index].intVal;
                ptr += 4;
                break;
            case BASETYPE_Long:
                (void) memcpy(ptr, &(argList[index].longVal), 8);
                ptr += 8;
                break;
            case BASETYPE_Float:
                *((jfloat *) ptr) = argList[index].fltVal;
                ptr += 4;
                break;
            case BASETYPE_Double:
                (void) memcpy(ptr, &(argList[index].dblVal), 8);
                ptr += 8;
                break;
            default:
                abort(); /* purecov: deadcode */
        }
        descPtr++;
        index++;
    }

    /* Determine the return type indicator */
    if (fnDesc->method_info.returnDescriptor == NULL) {
        retType = 0;
    } else {
        switch (fnDesc->method_info.returnDescriptor->generic.tag) {
            case DESCRIPTOR_ObjectType:
            case DESCRIPTOR_ArrayType:
                retType = DESCRIPTOR_ObjectType;
                break;
            case BASETYPE_Byte:
            case BASETYPE_Char:
            case BASETYPE_Int:
            case BASETYPE_Short:
            case BASETYPE_Boolean:
                /* These are all the same in sparc32 function returns */
                retType = BASETYPE_Int;
                break;
            case BASETYPE_Long:
                retType = BASETYPE_Long;
                break;
            case BASETYPE_Float:
                retType = BASETYPE_Float;
                break;
            case BASETYPE_Double:
                retType = BASETYPE_Double;
                break;
            default:
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_InternalError,
                                           NULL, "Invalid ffi return type");
                return JNI_ERR;
        }
    }

    /* Time to really make the call */
    JEM_FFICallFn(-frameSize, fnRef, argStack, argSize, retType);

    /* Based on the return type, fill in the return value */
    if (retVal != NULL) {
        switch (retType) {
            case 0:
                /* Do nothing, void return type */
                break;
            case DESCRIPTOR_ObjectType:
                retVal->objVal = *((JEMCC_Object **) argStack);
                break;
            case BASETYPE_Int:
                retVal->intVal = *((jint *) argStack);
                break;
            case BASETYPE_Long:
                retVal->longVal = *((jlong *) argStack);
                break;
            case BASETYPE_Float:
                retVal->fltVal = *((jfloat *) argStack);
                break;
            case BASETYPE_Double:
                retVal->dblVal = *((jdouble *) argStack);
                break;
            default:
                abort(); /* purecov: deadcode */
        }
    }

    return JNI_OK;
}
