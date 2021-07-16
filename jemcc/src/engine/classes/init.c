/**
 * Initialization routines for the core JEMCC virtual machine class instances.
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

/*
 * None of the classes are static, as new instances are created for
 * each VM.  They are, however, "static" from the VM perspective and
 * this macro provides such access.
 */
#define JVM_CLASS(idx) (jvm->coreClassTbl[(idx)])

/* Definitions appearing in the other class files */
extern JEMCC_MethodData JEMCC_ObjectMethods[];
extern JEMCC_MethodData JEMCC_ClassMethods[];

/**
 * Create the truly "core" classes - Object and Class.  Note that this
 * construction sequence performs internal manipulations of the JEMCC
 * information to handle the 'chicken and egg' problem (an Object needs
 * a Class and a Class is an Object).
 */
static jint JEM_InitializeCoreClasses(JNIEnv *env) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_Class *interfaces[1];
    JEMCC_Class *objectClass, *serializableClass, *classClass;
    jint rc;

    /* First, construct the "defective" Object/Serializable/Class instances */
    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC, 
                              "java.lang.Object", 
                              NULL, NULL, 0, 
                              JEMCC_ObjectMethods, 12, NULL, NULL, 0,
                              NULL, 0, NULL, &objectClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_Object) = objectClass;

    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC | ACC_INTERFACE,
                              "java.io.Serializable", 
                              objectClass, NULL, 0, 
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &serializableClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_Serializable) = serializableClass;

    interfaces[0] = serializableClass;
    rc = JEMCC_CreateStdClass(env, NULL, 
                              ACC_PUBLIC | ACC_FINAL | ACC_NATIVE_DATA,
                              "java.lang.Class", 
                              objectClass, interfaces, 1,
                              JEMCC_ClassMethods, 32, NULL, NULL, 0,
                              NULL, 0, NULL, &classClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_Class) = classClass;

    /* Now, fix them */
    objectClass->classReference = classClass;
    serializableClass->classReference = classClass;
    classClass->classReference = classClass;

    /* Special adjustment for additional elements of Class instances */
    classClass->classData->packedFieldSize = 
                                 sizeof(JEMCC_Class) - sizeof(JEMCC_Object);

    return JNI_OK;
}

/* Primitive array data definitions */
extern JEMCC_MethodData JEMCC_ArrayPrimMethods[];
extern JEMCC_FieldData JEMCC_ArrayPrimFields[];

/**
 * Construct a primitive array instance, for each of the primitive type
 * instances.  Manually constructs the class due to the requirements of
 * the array classes (and the association with the primitive instances).
 * Returns NULL if the construction fails for any reason.
 */
static JEMCC_Class *JEM_BuildPrimitiveArrayClass(JNIEnv *env, 
                                                 JEMCC_Class *arrayPrimitive,
                                                 JEMCC_Class *primClass, 
                                                 juint primitiveType,
                                                 char *arrayName) {
    JEMCC_ArrayClass *arrayClass;

    /* Note that array names are constructed/duplicated */
    arrayName = JEMCC_StrDupFn(env, arrayName);
    if (arrayName == NULL) return NULL;

    /* Create the class instance.  Not standard as componentType ref needed  */
    arrayClass = (JEMCC_ArrayClass *) JEMCC_Malloc(env, 
                                                   sizeof(JEMCC_ArrayClass));
    if (arrayClass == NULL) {
        JEMCC_Free(arrayName);
        return NULL;
    }
    arrayClass->classData = JEMCC_Malloc(env, sizeof(JEM_ClassData));
    if (arrayClass->classData == NULL) {
        JEMCC_Free(arrayClass);
        JEMCC_Free(arrayName);
        return NULL;
    }
    arrayClass->classReference = VM_CLASS(JEMCC_Class_Class);
    arrayClass->objStateSet = 0;

    /* Clone the primitive array class definitions, except for className */
    (void) memcpy(arrayClass->classData, arrayPrimitive->classData,
                  sizeof(JEM_ClassData));
    arrayClass->classData->accessFlags =
              ACC_PUBLIC | ACC_FINAL | ACC_ARRAY | ACC_JEMCC | ACC_NATIVE_DATA;
    arrayClass->classData->className = arrayName;

    /* Indicate array of depth 1 and associated primitive type */
    arrayClass->typeDepthInfo = primitiveType | 1;
    arrayClass->referenceClass = (JEMCC_Class *) primClass;

    /* Store it in the JEMCC registry */
    if (JEM_ClassNameSpaceStore(env, NULL, (JEMCC_Class *) arrayClass, 
                                NULL, JNI_TRUE) != JNI_OK) {
        JEMCC_Free(arrayClass->classData);
        JEMCC_Free(arrayClass);
        JEMCC_Free(arrayName);
        return NULL;
    }

    return (JEMCC_Class *) arrayClass;
}

/**
 * Construct the primitive class instances.  These are not the concrete
 * classes associated with the primitives (as exposed in the Java API) but
 * instead are the "internal" classes directly associated with the primitives.
 * Also constructs the array instances for these primitives.
 */
static jint JEM_InitializePrimitiveClasses(JNIEnv *env) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_Class *interfaces[2];
    JEMCC_Class *booleanPrimitive, *bytePrimitive, *charPrimitive;
    JEMCC_Class *shortPrimitive, *intPrimitive, *floatPrimitive, *longPrimitive;
    JEMCC_Class *doublePrimitive, *voidPrimitive, *arrayPrimitive;
    JEMCC_Class *cloneableClass, *objectClass = JVM_CLASS(JEMCC_Class_Object);
    JEMCC_Class *tmpClass;
    juint accessFlags = ACC_PUBLIC | ACC_FINAL | ACC_PRIMITIVE;
    jint rc;

    /* 
     * According to some test Java code, the primitive classes in the
     * Sun VM do NOT have an Object parent.  Same goes here for compatibility.
     */
    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "boolean", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &booleanPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Boolean) = booleanPrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "byte", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &bytePrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Byte) = bytePrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "char", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &charPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Char) = charPrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "short", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &shortPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Short) = shortPrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "int", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &intPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Int) = intPrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "float", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &floatPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Float) = floatPrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "long", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &longPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Long) = longPrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "double", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &doublePrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Double) = doublePrimitive;

    rc = JEMCC_CreateStdClass(env, NULL, accessFlags,
                              "void", NULL, NULL, 0,
                              NULL, 0, NULL, NULL, 0,
                              NULL, 0, NULL, &voidPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Void) = voidPrimitive;

    /* Need to create this now to support array cloneability */
    rc = JEMCC_CreateStdClass(env, NULL, ACC_PUBLIC | ACC_INTERFACE,
                              "java.lang.Cloneable", 
                               objectClass, NULL, 0, 
                               NULL, 0, NULL, NULL, 0,
                               NULL, 0, NULL, &cloneableClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_Cloneable) = cloneableClass;

    /* 
     * This class does not "exist" in the VM.  It is used as a copy basis
     * for the dynamic array class generators.
     */
    interfaces[0] = cloneableClass;
    interfaces[1] = JVM_CLASS(JEMCC_Class_Serializable);
    rc = JEMCC_CreateStdClass(env, NULL, accessFlags | ACC_NATIVE_DATA,
                              "array", objectClass, interfaces, 2,
                              JEMCC_ArrayPrimMethods, 1, NULL,
                              JEMCC_ArrayPrimFields, 1,
                              NULL, 0, NULL, &arrayPrimitive);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Primitive_Array) = arrayPrimitive;

    /*
     * Create the primitive array class definitions.
     */
    JVM_CLASS(JEMCC_Array_Boolean) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive,
                                                booleanPrimitive, 
                                                PRIMITIVE_BOOLEAN, "[Z");
    if (tmpClass == NULL) return JNI_ENOMEM;

    JVM_CLASS(JEMCC_Array_Byte) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive, 
                                                bytePrimitive, 
                                                PRIMITIVE_BYTE, "[B");
    if (tmpClass == NULL) return JNI_ENOMEM;

    JVM_CLASS(JEMCC_Array_Char) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive, 
                                                charPrimitive, 
                                                PRIMITIVE_CHAR, "[C");
    if (tmpClass == NULL) return JNI_ENOMEM;

    JVM_CLASS(JEMCC_Array_Short) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive, 
                                                shortPrimitive, 
                                                PRIMITIVE_SHORT, "[S");
    if (tmpClass == NULL) return JNI_ENOMEM;

    JVM_CLASS(JEMCC_Array_Int) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive, 
                                                intPrimitive, 
                                                PRIMITIVE_INT, "[I");
    if (tmpClass == NULL) return JNI_ENOMEM;

    JVM_CLASS(JEMCC_Array_Float) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive, 
                                                floatPrimitive, 
                                                PRIMITIVE_FLOAT, "[F");
    if (tmpClass == NULL) return JNI_ENOMEM;

    JVM_CLASS(JEMCC_Array_Long) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive, 
                                                longPrimitive, 
                                                PRIMITIVE_LONG, "[J");
    if (tmpClass == NULL) return JNI_ENOMEM;

    JVM_CLASS(JEMCC_Array_Double) = 
        tmpClass = JEM_BuildPrimitiveArrayClass(env, arrayPrimitive,
                                                doublePrimitive, 
                                                PRIMITIVE_DOUBLE, "[D");
    if (tmpClass == NULL) return JNI_ENOMEM;

    return JNI_OK;
}

/* More definitions appearing in the other class files */
extern JEMCC_MethodData JEMCC_RunnableMethods[];

extern JEMCC_MethodData JEMCC_ClassLoaderMethods[];
extern JEMCC_FieldData JEMCC_ClassLoaderFields[];
extern JEMCC_MethodData JEMCC_StringMethods[];
extern JEMCC_MethodData JEMCC_StringBufferMethods[];

extern JEMCC_MethodData JEMCC_ThrowableMethods[];
extern JEMCC_FieldData JEMCC_ThrowableFields[];
extern JEMCC_MethodData JEMCC_ArrayIndexOutOfBoundsExceptionMethods[];
extern JEMCC_MethodData JEMCC_StringIndexOutOfBoundsExceptionMethods[];
extern JEMCC_MethodData JEMCC_ExceptionInInitializerErrorMethods[];

/**
 * Construct the core system classes (ClassLoader, String[Buffer], associated
 * interfaces and java.lang Exceptions/Errors.  These are the main classes
 * required to complete the JVM.
 */
static jint JEM_InitializeSystemClasses(JNIEnv *env) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_Class *interfaces[5];
    JEMCC_Class *objectClass = JVM_CLASS(JEMCC_Class_Object);
    JEMCC_Class *throwableClass, *exceptionClass, *errorClass;
    JEMCC_Class *runtimeExceptionClass, *illegalArgumentExceptionClass;
    JEMCC_Class *indexOutOfBoundsExceptionClass;
    JEMCC_Class *linkageErrorClass, *incClassChangeErrorClass;
    JEMCC_Class *vmErrorClass, *classFormatErrorClass;
    jint rc;

    /* Interfaces */

    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_INTERFACE,
                              "java.lang.Runnable",
                              objectClass, interfaces, 0, 
                              JEMCC_RunnableMethods, 1, NULL,
                              NULL, 0, NULL, 0, NULL, 
                              &(JVM_CLASS(JEMCC_Class_Runnable)));
    if (rc != JNI_OK) return rc;

    /* Classes */

    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_ABSTRACT,
                              "java.lang.ClassLoader",
                              objectClass, interfaces, 0,
                              JEMCC_ClassLoaderMethods, 13, NULL,
                              JEMCC_ClassLoaderFields, 1, NULL, 0, NULL,
                              &(JVM_CLASS(JEMCC_Class_ClassLoader)));
    if (rc != JNI_OK) return rc;

    interfaces[0] = JVM_CLASS(JEMCC_Class_Serializable);
    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_FINAL | ACC_NATIVE_DATA,
                              "java.lang.String",
                              objectClass, interfaces, 1,
                              JEMCC_StringMethods, 66, NULL,
                              NULL, 0, NULL, 0, NULL, 
                              &(JVM_CLASS(JEMCC_Class_String)));
    if (rc != JNI_OK) return rc;

    interfaces[0] = JVM_CLASS(JEMCC_Class_Serializable);
    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_FINAL | ACC_NATIVE_DATA,
                              "java.lang.StringBuffer",
                              objectClass, interfaces, 1,
                              JEMCC_StringBufferMethods, 43, NULL,
                              NULL, 0, NULL, 0, NULL, NULL);
    if (rc != JNI_OK) return rc;

    /* Throwables/Errors/Exceptions */

    interfaces[0] = JVM_CLASS(JEMCC_Class_Serializable);
    rc = JEMCC_CreateStdClass(env, NULL,
                              ACC_PUBLIC | ACC_THROWABLE | ACC_STD_THROW,
                              "java.lang.Throwable",
                              objectClass, interfaces, 1,
                              JEMCC_ThrowableMethods, 9, NULL,
                              JEMCC_ThrowableFields, 1,
                              NULL, 0, NULL, &throwableClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_Throwable) = throwableClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL, "java.lang.Exception",
                                       throwableClass, &exceptionClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_Exception) = exceptionClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                         "java.lang.ClassNotFoundException",
                         exceptionClass,
                         &(JVM_CLASS(JEMCC_Class_ClassNotFoundException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                         "java.lang.CloneNotSupportedException",
                         exceptionClass,
                         &(JVM_CLASS(JEMCC_Class_CloneNotSupportedException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                             "java.lang.IllegalAccessException",
                             exceptionClass,
                             &(JVM_CLASS(JEMCC_Class_IllegalAccessException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                             "java.lang.InstantiationException",
                             exceptionClass,
                             &(JVM_CLASS(JEMCC_Class_InstantiationException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                               "java.lang.InterruptedException",
                               exceptionClass,
                               &(JVM_CLASS(JEMCC_Class_InterruptedException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                               "java.lang.NoSuchFieldException",
                               exceptionClass,
                               &(JVM_CLASS(JEMCC_Class_NoSuchFieldException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                              "java.lang.NoSuchMethodException",
                              exceptionClass,
                              &(JVM_CLASS(JEMCC_Class_NoSuchMethodException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL, 
                                       "java.lang.RuntimeException",
                                       exceptionClass, 
                                       &runtimeExceptionClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_RuntimeException) = runtimeExceptionClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                "java.lang.ArithmeticException",
                                runtimeExceptionClass,
                                &(JVM_CLASS(JEMCC_Class_ArithmeticException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                "java.lang.ArrayStoreException",
                                runtimeExceptionClass,
                                &(JVM_CLASS(JEMCC_Class_ArrayStoreException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                "java.lang.ClassCastException",
                                runtimeExceptionClass,
                                &(JVM_CLASS(JEMCC_Class_ClassCastException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.IllegalArgumentException",
                                       runtimeExceptionClass,
                                       &illegalArgumentExceptionClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_IllegalArgumentException) = 
                                      illegalArgumentExceptionClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                        "java.lang.IllegalThreadStateException",
                        illegalArgumentExceptionClass,
                        &(JVM_CLASS(JEMCC_Class_IllegalThreadStateException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                              "java.lang.NumberFormatException",
                              illegalArgumentExceptionClass,
                              &(JVM_CLASS(JEMCC_Class_NumberFormatException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                       "java.lang.IllegalMonitorStateException",
                       runtimeExceptionClass,
                       &(JVM_CLASS(JEMCC_Class_IllegalMonitorStateException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                              "java.lang.IllegalStateException",
                              runtimeExceptionClass,
                              &(JVM_CLASS(JEMCC_Class_IllegalStateException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.IndexOutOfBoundsException",
                                       runtimeExceptionClass,
                                       &indexOutOfBoundsExceptionClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_IndexOutOfBoundsException) =
                                      indexOutOfBoundsExceptionClass;

    rc = JEMCC_CreateStdClass(env, NULL,
                     ACC_PUBLIC | ACC_THROWABLE,
                     "java.lang.ArrayIndexOutOfBoundsException",
                     indexOutOfBoundsExceptionClass, interfaces, 0,
                     JEMCC_ArrayIndexOutOfBoundsExceptionMethods, 3, NULL,
                     NULL, 0, NULL, 0, NULL,
                     &(JVM_CLASS(JEMCC_Class_ArrayIndexOutOfBoundsException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdClass(env, NULL,
                    ACC_PUBLIC | ACC_THROWABLE,
                    "java.lang.StringIndexOutOfBoundsException",
                    indexOutOfBoundsExceptionClass, interfaces, 0,
                    JEMCC_StringIndexOutOfBoundsExceptionMethods, 3, NULL,
                    NULL, 0, NULL, 0, NULL,
                    &(JVM_CLASS(JEMCC_Class_StringIndexOutOfBoundsException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                         "java.lang.NegativeArraySizeException",
                         runtimeExceptionClass,
                         &(JVM_CLASS(JEMCC_Class_NegativeArraySizeException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                               "java.lang.NullPointerException",
                               runtimeExceptionClass,
                               &(JVM_CLASS(JEMCC_Class_NullPointerException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                  "java.lang.SecurityException",
                                  runtimeExceptionClass,
                                  &(JVM_CLASS(JEMCC_Class_SecurityException)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL, "java.lang.Error",
                                       throwableClass, &errorClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_Error) = errorClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.LinkageError",
                                       errorClass, &linkageErrorClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_LinkageError) = linkageErrorClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                              "java.lang.ClassCircularityError",
                              linkageErrorClass,
                              &(JVM_CLASS(JEMCC_Class_ClassCircularityError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.ClassFormatError",
                                       linkageErrorClass,
                                       &classFormatErrorClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_ClassFormatError) = classFormatErrorClass;

    rc = JEMCC_CreateStdClass(env, NULL,
                        ACC_PUBLIC | ACC_THROWABLE,
                        "java.lang.ExceptionInInitializerError",
                        linkageErrorClass, interfaces, 0,
                        JEMCC_ExceptionInInitializerErrorMethods, 4, NULL,
                        NULL, 0, NULL, 0, NULL,
                        &(JVM_CLASS(JEMCC_Class_ExceptionInInitializerError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.IncompatibleClassChangeError",
                                       linkageErrorClass,
                                       &incClassChangeErrorClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_IncompatibleClassChangeError) = 
                                             incClassChangeErrorClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                "java.lang.AbstractMethodError",
                                incClassChangeErrorClass,
                                &(JVM_CLASS(JEMCC_Class_AbstractMethodError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                 "java.lang.IllegalAccessError",
                                 incClassChangeErrorClass,
                                 &(JVM_CLASS(JEMCC_Class_IllegalAccessError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                 "java.lang.InstantiationError",
                                 incClassChangeErrorClass,
                                 &(JVM_CLASS(JEMCC_Class_InstantiationError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                   "java.lang.NoSuchFieldError",
                                   incClassChangeErrorClass,
                                   &(JVM_CLASS(JEMCC_Class_NoSuchFieldError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                  "java.lang.NoSuchMethodError",
                                  incClassChangeErrorClass,
                                  &(JVM_CLASS(JEMCC_Class_NoSuchMethodError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                               "java.lang.NoClassDefFoundError",
                               linkageErrorClass,
                               &(JVM_CLASS(JEMCC_Class_NoClassDefFoundError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                               "java.lang.UnsatisfiedLinkError",
                               linkageErrorClass,
                               &(JVM_CLASS(JEMCC_Class_UnsatisfiedLinkError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.VerifyError",
                                       linkageErrorClass,
                                       &(JVM_CLASS(JEMCC_Class_VerifyError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.ThreadDeath",
                                       errorClass,
                                       &(JVM_CLASS(JEMCC_Class_ThreadDeath)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.VirtualMachineError",
                                       errorClass, &vmErrorClass);
    if (rc != JNI_OK) return rc;
    JVM_CLASS(JEMCC_Class_VirtualMachineError) = vmErrorClass;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.InternalError",
                                       vmErrorClass,
                                       &(JVM_CLASS(JEMCC_Class_InternalError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                   "java.lang.OutOfMemoryError",
                                   vmErrorClass,
                                   &(JVM_CLASS(JEMCC_Class_OutOfMemoryError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                 "java.lang.StackOverflowError",
                                 vmErrorClass,
                                 &(JVM_CLASS(JEMCC_Class_StackOverflowError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.lang.UnknownError",
                                       vmErrorClass,
                                       &(JVM_CLASS(JEMCC_Class_UnknownError)));
    if (rc != JNI_OK) return rc;

    rc = JEMCC_CreateStdThrowableClass(env, NULL,
                                       "java.io.IOException",
                                       exceptionClass,
                                       &(JVM_CLASS(JEMCC_Class_IOException)));
    if (rc != JNI_OK) return rc;

    return JNI_OK;
}

/* Create the basic VM instances required for the system operation */
extern jint JEM_ClassLoader_Init(JNIEnv *env, JEMCC_Object *loader,
                                 JEMCC_Object *parentLoader);
extern JEMCC_MethodData JEMCC_SysClassLoaderMethods[];

static jint JEM_InitializeCoreInstances(JNIEnv *env) {
    JEM_JavaVM *jvm = ((JEM_JNIEnv *) env)->parentVM;
    JEMCC_Class *interfaces[5], *sysClassLoaderClass;
    JEMCC_Object *sysClassLoaderInst;
    jint rc;

    /* First, the system classloader */
    rc = JEMCC_CreateStdClass(env, NULL, 
                              ACC_PUBLIC | ACC_FINAL,
                              "wrdg.jemcc.SysClassLoader",
                              JVM_CLASS(JEMCC_Class_ClassLoader), interfaces, 0,
                              JEMCC_SysClassLoaderMethods, 5, NULL,
                              NULL, 0, NULL, 0, NULL, &sysClassLoaderClass);
    if (rc != JNI_OK) return rc;

    /* Directly initialize the system classloader instance */
    sysClassLoaderInst = JEMCC_AllocateObject(env, sysClassLoaderClass, 0);
    if (sysClassLoaderInst == NULL) return JNI_ENOMEM;
    rc = JEM_ClassLoader_Init(env, sysClassLoaderInst, NULL);
    if (rc != JNI_OK) {
        /* TODO - clean it up! */
        return JNI_ENOMEM;
    }
    jvm->systemClassLoader = sysClassLoaderInst;

    return JNI_OK;
}

/* Reflection loader from "down-below" */
extern jint JEM_reflect_ClassLoader(JNIEnv *env, JEMCC_Object *loader,
                                    const char *className,
                                    JEMCC_Class **classInst);

/**
 * Method by which the "core" classes (those required by the JEMCC virtual
 * machine internals) are initially created.  Should only be called once with
 * the primary (initial) environment instance for the new virtual machine.
 *
 * Parameters:
 *     env - the initial environment for a new virtual machine instance
 *
 * Returns:
 *     JNI_OK - all initialization completed successfully, all VM core
 *              classes are now available
 *     JNI_ERR - an initialization error occurred (should not happen) and
 *               the virtual machine is effectively "invalid" (an exception
 *               will have been thrown in the environment, if possible)
 *     JNI_ENOMEM - a memory allocation error has occurred and the virtual
 *                  machine is effectively "invalid" (an exception will have
 *                  been thrown in the environment, if possible)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - JEMCC initialization information is invalid
 *     LinkageError - one of the class linkages failed
 */
jint JEM_InitializeVMClasses(JNIEnv *env) {
    jint rc;

    /* Load 'em up! */
    rc = JEM_InitializeCoreClasses(env);
    if (rc != JNI_OK) return rc;

    rc = JEM_InitializePrimitiveClasses(env);
    if (rc != JNI_OK) return rc;

    rc = JEM_InitializeSystemClasses(env);
    if (rc != JNI_OK) return rc;

    rc = JEM_InitializeCoreInstances(env);
    if (rc != JNI_OK) return rc;

    /* Hook in the reflection class loader */
    rc = JEMCC_RegisterPkgClassLdrFn(env, NULL, "java.lang.reflect",
                                     JEM_reflect_ClassLoader);
    if (rc != JNI_OK) return rc;

    return JNI_OK;
}

/**
 * Obtain the class reference for the VM core class based on the index
 * value.  This method is provided for external entities which do not have
 * the VM_CLASS macro available.
 *
 * Parameters:
 *     env - an environment instance associated with the VM core
 *     index - the index of the VM core class to retrieve
 *
 * Returns:
 *     The requested VM class instance - NULL will never occur as
 *     the VM initialization will completely fail if any class cannot
 *     be created.
 */
JEMCC_Class *JEMCC_GetCoreVMClass(JNIEnv *env, JEMCC_VMClassIndex index) {
    return ((JEM_JNIEnv *) env)->parentVM->coreClassTbl[index];
}
