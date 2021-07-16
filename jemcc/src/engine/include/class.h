/**
 * Internal function/structure definitions for JEMCC class data management.
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

#ifndef JEM_CLASS_H
#define JEM_CLASS_H 1

/* NOTE: this is auto-read by jem.h, so it shouldn't be directly read */

/* <jemcc_start> */

/************************ Class Information ****************************/

/* The following are directly from Chapter 4 of the JVM specification */
#define ACC_PUBLIC       0x0001 /* accessable outside of package/class */
#define ACC_PRIVATE      0x0002 /* method/field only available within class */
#define ACC_PROTECTED    0x0004 /* method/field only available within package */
#define ACC_STATIC       0x0008 /* method/field is static to the class */
#define ACC_FINAL        0x0010 /* no subclassing, overriding or assignment */
#define ACC_SUPER        0x0020 /* invokespecial backwards compatibility flag */
#define ACC_SYNCHRONIZED 0x0020 /* method must be monitor locked */
#define ACC_VOLATILE     0x0040 /* no caching permitted */
#define ACC_TRANSIENT    0x0080 /* no read/write from persistent object mngr */
#define ACC_NATIVE       0x0100 /* not implemented in pure java */
#define ACC_INTERFACE    0x0200 /* class data is actually an interface */
#define ACC_ABSTRACT     0x0400 /* abstract class/method - must be provided */
#define ACC_STRICT       0x0800 /* floating point is fp-strict */

/* And these are for the JVM implementation record-keeping */
#define ACC_JEMCC       0x01000 /* class/method is a jemcc implementation */
#define ACC_NATIVE_DATA 0x02000 /* jemcc class has native data element */
#define ACC_SYNTHETIC   0x04000 /* special flag for fields/methods */

#define ACC_ARRAY       0x10000 /* class is an array instance */
#define ACC_PRIMITIVE   0x20000 /* class is a primitive object class */
#define ACC_THROWABLE   0x40000 /* class is a throwable subclass */
#define ACC_STD_THROW   0x80000 /* class is a "standard" throwable subclass */

#define ACC_RESOLVE_ERROR 0x100000 /* class data element has an error */

/* Type definitions for primitives, with gap for array depth info */
#define PRIMITIVE_BOOLEAN  0x0100
#define PRIMITIVE_BYTE     0x0200
#define PRIMITIVE_CHAR     0x0300
#define PRIMITIVE_SHORT    0x0400
#define PRIMITIVE_INT      0x0500
#define PRIMITIVE_FLOAT    0x0600
#define PRIMITIVE_LONG     0x0700
#define PRIMITIVE_DOUBLE   0x0800
#define PRIMITIVE_VOID     0x0900

#define PRIMITIVE_TYPE_MASK 0x0F00

/**
 * Convenience method to obtain the "internal" name of the class.  This
 * is the raw character className, may be UTF encoded.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     class - the class instance to retrieve the name for
 *
 * Returns:
 *     The raw character representation of the fully qualified name of
 *     the specified class.  May be UTF encoded.
 */
JNIEXPORT char *JNICALL JEMCC_GetClassName(JNIEnv *env, 
                                           JEMCC_Class *class);

/* <jemcc_end> */

/************* Raw Class Parsing Structures/Definitions/Methods *************/

/* Defines/structures for class constant pool data - odd order follows spec */
#define CONSTANT_Class 7
#define CONSTANT_Fieldref 9
#define CONSTANT_Methodref 10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_String 8
#define CONSTANT_Integer 3
#define CONSTANT_Float 4
#define CONSTANT_Long 5
#define CONSTANT_Double 6
#define CONSTANT_NameAndType 12
#define CONSTANT_Utf8 1

/* Internal constants used during resolution phase */
#define CONSTANT_ResolvedClass 135                   /* 128 + 7 */
#define CONSTANT_ResolvedClassErr 391                /* 128 + 256 + 7 */
#define CONSTANT_ResolvedFieldref 137                /* 128 + 9 */
#define CONSTANT_ResolvedFieldrefErr 393             /* 128 + 256 + 9 */
#define CONSTANT_ResolvedMethodref 138               /* 128 + 10 */
#define CONSTANT_ResolvedMethodrefErr 394            /* 128 + 256 + 10 */
#define CONSTANT_ResolvedInterfaceMethodref 139      /* 128 + 11 */
#define CONSTANT_ResolvedInterfaceMethodrefErr 395   /* 128 + 256 + 11 */
#define CONSTANT_ResolvedString 136                  /* 128 + 8 */

/* Data structure for carrying class constant pool records */
typedef union {
    struct CONSTANT_Generic_info {
        jint tag;
    } generic;

    struct CONSTANT_Class_info {
        jint tag;
        jint nameIndex;
    } class_info;

    struct RESOLVED_Class_info {
        jint tag;
        jint refTblIndex;
        JEMCC_Object *extClassRef;
    } res_class_info;

    /* These represent a Fieldref, Methodref or InterfaceMethodref */
    struct CONSTANT_Ref_info {
        jint tag;
        jint classIndex;
        jint nameAndTypeIndex;
    } ref_info;

    struct RESOLVED_Ref_info {
        jint tag;
        jint tableIndex;
    } res_ref_info;

    struct CONSTANT_String_info {
        jint tag;
        jint stringIndex;
    } string_info;

    struct RESOLVED_String_info {
        jint tag;
        jint constTblIndex;
        JEMCC_Object *stringRef;
    } res_string_info;

    struct CONSTANT_Integer_info {
        jint tag;
        jint constTblIndex;
        jint value;
    } integer_info;

    struct CONSTANT_Float_info {
        jint tag;
        jint constTblIndex;
        jfloat value;
    } float_info;

    struct CONSTANT_Long_info {
        jint tag;
        jint constTblIndex;
        jlong value;
    } long_info;

    struct CONSTANT_Double_info {
        jint tag;
        jint constTblIndex;
        jdouble value;
    } double_info;

    struct CONSTANT_NameAndType_info {
        jint tag;
        jint nameIndex;
        jint descIndex;
    } nameandtype_info;

    struct CONSTANT_Utf8_info {
        jint tag;
        jbyte *bytes;
    } utf8_info;
} JEM_ConstantPoolData;

/* Storage structure for class, field and method (code) attribute sets */
typedef struct {
    jsize attributesCount;

    struct ATTRIBUTE_info {
        jint attributeNameIndex;
        juint attributeLength;
        jbyte *info;
    } *attributes;
} JEM_ParsedAttributeData;

/* Storage structure for class field definitions */
typedef struct {
    jint accessFlags;
    jint nameIndex;
    jint descIndex;
    jsize attributesCount;
    struct ATTRIBUTE_info *attributes;
} JEM_ParsedFieldData;

/* Storage structure for class method definitions */
typedef struct {
    jint accessFlags;
    jint nameIndex;
    jint descIndex;
    jsize attributesCount;
    struct ATTRIBUTE_info *attributes;
} JEM_ParsedMethodData;

/* Storage structure for fully parsed binary class data */
typedef struct JEM_ParsedClassData {
    jint classAccessFlags;
    jint classIndex;
    jint superClassIndex;

    jsize constantPoolCount;
    JEM_ConstantPoolData *constantPool;

    jsize interfacesCount;
    jint *interfaces;

    jsize fieldsCount;
    JEM_ParsedFieldData *fields;

    jsize methodsCount;
    JEM_ParsedMethodData *methods;

    jsize attributesCount;
    struct ATTRIBUTE_info *attributes;
} JEM_ParsedClassData;

/**
 * Parse the provided binary class data, which initializes the constant
 * pool, interface, field, method and attribute information.  Also does
 * some preliminary validation of the integrity of the class data (most
 * of the Pass 1 and 2 verification tests as defined by the Java VM spec are
 * performed here).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     buff - the binary data associated with the class to be parsed
 *     buffLen - the number of bytes in the provided binary data buffer
 *
 * Returns:
 *     NULL if a parsing error occurred, otherwise the structure instance
 *     which contains the parsed class data.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - the class data is invalid or corrupted (in any one
 *                        of a number of possible ways)
 *     UnsupportedClassVersionError - binary class data is not supported by
 *                                    this VM
 */
JNIEXPORT JEM_ParsedClassData *JNICALL JEM_ParseClassData(JNIEnv *env,
                                                          const jbyte *buff,
                                                          jsize buffLen);

/**
 * Destroy the parsed class information, as returned from the ParseClassData
 * method above.
 *
 * Parameters:
 *     data - the parsing structure containing the class information to be
 *            destroyed
 */
JNIEXPORT void JNICALL JEM_DestroyParsedClassData(JEM_ParsedClassData *data);

/**
 * Read a constant pool index from the binary class data and validate
 * it against the parsed constant pool set.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     buffer - the pointer reference to the binary class data
 *     classData - the parsing structure containing constant pool data
 *     checkTag - if greater than zero, validate the constant pool entry
 *                at the given index against this type.  If zero or negative,
 *                index is passed verbatim if zero (none or type check).
 *     typeMsg - the message to include in the exception if the validation
 *               fails
 *
 * Returns:
 *     -1 if the constant pool index or type was invalid or the read index
 *     value (never zero if checkTag positive).
 *
 * Exceptions:
 *     ClassFormatError - the constant pool index or type is invalid
 */
JNIEXPORT int JNICALL JEM_ReadConstantPoolIndex(JNIEnv *env,
                                                const jbyte **buffer,
                                                JEM_ParsedClassData *classData,
                                                jint checkTag, char *typeMsg);

/**
 * Convenience method to convert the internal class package directory
 * notation to the programmatic package dot notation.
 *
 * Parameters:
 *     buff - the string buffer containing the class name to be converted
 */
JNIEXPORT void JNICALL JEM_SlashToDot(char *buff);

/************ Descriptor Parsing Structures/Definitions/Methods ************/

/* High byte indicates descriptor type, low byte is the base type */
#define DESCRIPTOR_EndOfList 0x00
#define DESCRIPTOR_BaseType 0x10
#define DESCRIPTOR_ObjectType 0x20
#define DESCRIPTOR_ArrayType 0x30
#define DESCRIPTOR_MethodType 0x40

#define BASETYPE_Byte 0x11
#define BASETYPE_Char 0x12
#define BASETYPE_Double 0x13
#define BASETYPE_Float 0x14
#define BASETYPE_Int 0x15
#define BASETYPE_Long 0x16
#define BASETYPE_Short 0x17
#define BASETYPE_Boolean 0x18

/* Carrier for Java field and method descriptor information (recursive) */
typedef union JEM_DescriptorInfo JEM_DescriptorData;
union JEM_DescriptorInfo {
    struct DESCRIPTOR_Generic_info {
        jint tag;
    } generic;

    struct DESCRIPTOR_BaseType_info {
        jint tag;
    } base_info;

    struct DESCRIPTOR_ObjectType_info {
        jint tag;
        char *className;
    } object_info;

    struct DESCRIPTOR_ArrayType_info {
        jint tag;
        JEM_DescriptorData *componentType;
    } array_info;

    struct DESCRIPTOR_MethodType_info {
        jint tag;
        JEM_DescriptorData *paramDescriptor;
        JEM_DescriptorData *returnDescriptor;
    } method_info;
};

/**
 * Parse the Java descriptor, which may be a class, field or method
 * reference.  Used recursively for method descriptor parsing, this
 * method can parse into a provided buffer and return indicators to
 * the "next" descriptor element.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     descriptor - the Java field/method descriptor to be parsed
 *     descBuffer - if non-NULL, the parsed descriptor information is
 *                  placed here; otherwise, a new parsed descriptor
 *                  instance is created for return
 *     nextPtr - if non-NULL, the arguments of a method are being
 *               parsed - different consistency checks occur and the
 *               pointer to the "next" descriptor in the master
 *               descriptor is returned here
 *     isStatic - only applicable for method descriptor parsing - if JNI_TRUE,
 *                this is a static method descriptor and can have 255 arguments,
 *                otherwise it is an instance method and can only have 254
 *                arguments
 *
 * Returns:
 *     NULL if a parsing error occurred (an exception will have been thrown
 *     in the current environment, if quiet mode is false), otherwise the 
 *     descriptor instance.  Equal to descBuffer if descBuffer is non-NULL, 
 *     otherwise a new instance which must be destroyed by the caller using 
 *     the DestroyDescriptor method.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - the descriptor information is invalid (note that
 *                        different/additional tests occur if nextPtr is
 *                        non-NULL)
 */
JNIEXPORT JEM_DescriptorData *JNICALL JEM_ParseDescriptor(JNIEnv *env,
                                                const char *descriptor,
                                                JEM_DescriptorData *descBuffer,
                                                const char **nextPtr,
                                                jboolean isStatic);

/**
 * Destroys the descriptor instance and any nested descriptors (for methods).
 *
 * Parameters:
 *     data - the parsed descriptor instance to be destroyed
 *     freeStruct - if JNI_TRUE, free the main descriptor structure - this
 *                  should always be the case unless a descriptor buffer
 *                  was used in the parsing of the descriptor
 */
JNIEXPORT void JNICALL JEM_DestroyDescriptor(JEM_DescriptorData *data,
                                             jboolean freeStruct);

/**
 * Validate a descriptor instance, in a minimal fashion (no memory allocations
 * except for error conditions).  This only validates the format, it does
 * not check the validity of any specified classnames.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     descriptor - the Java field/method descriptor to be parsed
 *     isMethod - if JNI_TRUE, the provided descriptor must be a valid
 *                method descriptor, if JNI_FALSE, must be a field instance
 *
 * Returns:
 *     JNI_OK if the provided descriptor is valid, JNI_ERR if an error
 *     was found (a ClassFormatError is thrown in the current environment).
 *
 * Exceptions:
 *     ClassFormatError - the descriptor information is invalid 
 */
JNIEXPORT jint JNICALL JEM_ValidateDescriptor(JNIEnv *env,
                                              const char *descriptor,
                                              jboolean isMethod);

/**
 * "Unparse" a descriptor, converting the parsed descriptor structure
 * back into the internal Java string representation of the descriptor.
 * This method uses the environment working string buffer.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     data - the parsed descriptor information to generation the string
 *            representation from
 *     nestedEntry - if JNI_TRUE, append descriptor information to the
 *                   environment working buffer; otherwise, reset the buffer,
 *                   convert the information and return a string duplicate
 *                   of the result
 *
 * Returns:
 *     NULL if a memory failure occurred, otherwise the string representation
 *     of the descriptor.  If nestedEntry was false, the return string must
 *     be freed by the caller.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT char *JNICALL JEM_ConvertDescriptor(JNIEnv *env,
                                              JEM_DescriptorData *data,
                                              jboolean nestedEntry);

/**
 * "Unparse" a method descriptor, converting the parsed descriptor structure
 * into a human readable descriptor of the method (i.e. 'void func(int, char[])'
 * instead of 'func(I[C)V'). This method uses the environment working string
 * buffer.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     methodName - the name of the method to appear in the description
 *     data - the parsed descriptor information to generation the method
 *            description from
 *
 * Returns:
 *     NULL if a memory failure occurred, otherwise a copy of the human
 *     readable method description (which must be freed by the caller).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT char *JNICALL JEM_ConvertMethodDescriptor(JNIEnv *env,
                                                    char *methodName,
                                                    JEM_DescriptorData *data);

/********* Resolved/Linked Class Structures/Definitions/Methods *********/

typedef struct JEM_MethodExceptionBlock {
    jint startPC;
    jint endPC;
    jint handlerPC;

    union {
        jint index;
        JEMCC_Class *instance;
    } exceptionClass;
} JEM_MethodExceptionBlock;

#ifndef NO_JVM_DEBUG
typedef struct JEM_LineNumberEntry {
    jint startPC;
    jint lineNumber;
} JEM_LineNumberEntry;

typedef struct JEM_LocalVariableEntry {
    jint startPC;
    jint length;
    jint index;
    char *name;
    char *descriptor;
} JEM_LocalVariableEntry;
#endif

typedef struct JEM_BCMethod {
    jsize maxStack;
    jsize maxLocals;
    jsize codeLength;
    jubyte *code;
    jsize exceptionTableLength;
    JEM_MethodExceptionBlock *exceptionTable;

#ifndef NO_JVM_DEBUG
    jsize lineNumberTableLength;
    JEM_LineNumberEntry *lineNumberTable;
    jsize localVariableTableLength;
    JEM_LocalVariableEntry *localVariableTable;
#endif
} JEM_BCMethod;

/**
 * Parse the method code attribute information.  This will extract the
 * bytecode and stack information for the method, as well as parsing
 * internal method attributes.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     buffer - the attribute buffer containing the binary code information
 *     buffLen - the number of bytes in the provided binary data buffer
 *     classData - the parsed class information associated with this method
 *                 code, used for retrieving reference information
 *
 * Returns:
 *     NULL if a parsing error occurred, otherwise the structure instance
 *     which contains the parsed bytecode method data.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - the method data is invalid or corrupted (in any one
 *                        of a number of possible ways)
 */
JNIEXPORT JEM_BCMethod *JNICALL JEM_ParseMethodCode(JNIEnv *env,
                                            const jbyte *buffer, jsize buffLen,
                                            JEM_ParsedClassData *classData);

/**
 * Destroy the parsed method code information, as returned from the
 * ParseMethodCode method above.
 *
 * Parameters:
 *     method - the bytecode method structure containing the information
 *              to be destroyed
 */
JNIEXPORT void JNICALL JEM_DestroyMethodCode(JEM_BCMethod *method);

typedef struct JEM_ClassMethodData {
    juint accessFlags;
    char *name;
    char *descriptorStr;
    JEM_DescriptorData *descriptor;
    jint methodIndex;
    jsize stackConsumeCount;

    union {
        JEM_BCMethod *bcMethod;
        JEMCC_MethodFn ccMethod;
        void *ntvMethod;
    } method;

    JEMCC_Class *parentClass;
} JEM_ClassMethodData;

typedef struct JEM_ClassMethodRefError {
    juint accessFlags;
    char *name;
    JEM_DescriptorData *descriptor;
} JEM_ClassMethodRefError;

typedef struct JEM_ClassFieldData {
    juint accessFlags;
    char *name;
    char *descriptorStr;
    JEM_DescriptorData *descriptor;
    jint fieldOffset;
    JEMCC_Class *parentClass;
} JEM_ClassFieldData;

typedef struct JEM_ClassFieldRefError {
    juint accessFlags;
    char *name;
    JEM_DescriptorData *descriptor;
} JEM_ClassFieldRefError;

typedef union JEM_ClassConstant {
    struct {
        jint tag;
    } generic;

    struct {
        jint tag;
        jobject stringRef;
    } string_const;

    struct {
        jint tag;
        jint value;
    } integer_const;

    struct {
        jint tag;
        jfloat value;
    } float_const;

    struct {
        jint tag;
        jlong value;
    } long_const;

    struct {
        jint tag;
        jdouble value;
    } double_const;
} JEM_ClassConstant;

/* Resolution states */
#define JEM_CLASS_CONSTRUCTING        0
#define JEM_CLASS_RESOLVE_IN_PROGRESS 1
#define JEM_CLASS_RESOLVE_ERROR       2
#define JEM_CLASS_RESOLVE_COMPLETE    3
#define JEM_CLASS_INIT_IN_PROGRESS    4
#define JEM_CLASS_INIT_FAILED         5
#define JEM_CLASS_INIT_COMPLETE       6
#define JEM_CLASS_ABSTRACT_ERROR      7

typedef struct JEM_ClassData {
    /* Class flag information (see ACC_ defines) */
    juint accessFlags;

    /* Reference to classloader from which this class originates */
    JEMCC_Object *classLoader;

    /* Class name and pending parsed data information */
    char *className;
    JEM_ParsedClassData *parseData;

    /* Resolution/initialization flag handler */
    jint resolveInitState;
    JEMCC_ThreadId resolveInitThread;

    /* SuperClass/SuperInterface information for assignment verification */
    JEMCC_Class **assignList;
    jsize interfaceCount, assignmentCount;

    /* Reference tables for class/interface method vtables */
    JEM_ClassMethodData ***methodLinkTables;
    jsize classMethodCount, syntheticMethodCount;

    /* Storage regions for local field/method data arrays */
    JEM_ClassMethodData *localMethods;
    jsize localMethodCount;
    JEM_ClassFieldData *localFields;
    jsize localFieldCount;

    /* Field packing information */
    juint packedFieldSize;

    /* Storage array for the fixed class constants */
    JEM_ClassConstant *localConstants;

    /* Storage arrays for external class/field/method references */
    JEMCC_Class **classRefs;
    JEM_ClassMethodData **classMethodRefs;
    JEM_ClassFieldData **classFieldRefs;

    /* Miscellaneous bits and pieces */
    char *sourceFile;
} JEM_ClassData;

/**
 * Allocate the memory block for a java.lang.Class instance.  This method
 * is similar to JEMCC_AllocateObject, but does not create the Class/Object
 * instance in the standard (garbage-collectible) allocator.  Class instances
 * must be manually destroyed (through DestroyClassInstance), garbage
 * collection of classes (if enabled) only occurs on the destruction of a
 * ClassLoader instance, by definition.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *
 * Returns:
 *     NULL if a memory allocation failed, otherwise the base class structure.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT JEMCC_Object *JNICALL JEM_AllocateClass(JNIEnv *env);

/**
 * Internal method to "mangle" the class hierarchy information (superclasses
 * and interfaces) and the class method information to build the inheritance
 * chain information and the method/interface mapping tables.
 *
 * Note: this method behaves differently based on a JEMCC or bytecode
 * class instance.  For JEMCC classes, an abstract/interface linkage error
 * is cause for immediate failure.  For a bytecode class, this method does 
 * not throw an exception or return an error if an abstract method 
 * implementation is missing (non-abstract class), but sets the 
 * resolveInitState of the class to JEM_CLASS_ABSTRACT_ERROR to force the 
 * throwing of AbstractMethodError during initialization (as per the JVM
 * specification).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class instance being constructed which will contain
 *                 the inheritance chain and method mapping tables
 *     superClass - the direct superclass of this class.  Must only be NULL
 *                  for the java.lang.Object class
 *     interfaces - an array of direct interfaces for this class
 *     interfaceCount - the number of interfaces in the interfaces array
 *     methods - an array of "local" method definitions for this class
 *     methodCount - the number of method definitions in the methods array
 *
 * Returns:
 *     JNI_OK - the package hierarchy data has been successfully created
 *     JNI_ERR - this is a JEMCC class instance and an abstract method
 *               linkage failure has occurred (an AbstractMethodError has
 *               been thrown in the current environment)
 *     JNI_ENOMEM - a memory allocation has failed during the creation of the
 *                  chain/linkage tables and an OutOfMemory error has been
 *                  thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     AbstractMethodError - a non-abstract JEMCC class is missing an "abstract"
 *                           method implementation
 */
JNIEXPORT jint JNICALL JEM_BuildClassHierData(JNIEnv *env,
                               JEMCC_Class *classInst, JEMCC_Class *superClass,
                               JEMCC_Class **interfaces, jsize interfaceCount,
                               JEM_ClassMethodData *methods, jsize methodCount);

/**
 * Internal method to pack the class field information, determining the
 * appropriate offsets for each field element to ensure proper byte
 * alignment of the various field data types.  Handles both the instance
 * fields and the static fields, as well as creating the static field
 * storage area.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class for which the fields are being packed/allocated
 *     fields - an array of field definitions for this class
 *     fieldCount - the number of field definitions in the fields array
 *
 * Returns:
 *     JNI_OK - the packing of the field information completed successfully
 *     JNI_ERR - an alignment/size error occurred using predefined JEMCC
 *               class field offsets (ClassFormatError will have been thrown
 *               in the current environment)
 *     JNI_ENOMEM - a memory allocation has failed during the creation of the
 *                  field tables and an OutOfMemory error has been thrown
 *                  in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - a predefined field offset was invalid
 */
JNIEXPORT jint JNICALL JEM_PackClassFieldData(JNIEnv *env, 
                                              JEMCC_Class *classInst,
                                              JEM_ClassFieldData *fields, 
                                              jsize fieldCount);

/**
 * Store/define a class instance in the given classloader.  This method
 * will not replace an already existing class, but handles such an error
 * case differently depending on whether it is an internal (JEMCC) or 
 * bytecode class definition.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to receive the class instance.  If
 *              NULL, the VM bootstrap classloader is used
 *     classInst - the class to be stored into the classloader
 *     existingClass - if non-NULL and the class being stored is already in
 *                     the classloader, this reference is used to return the 
 *                     already existing class
 *     internalClass - if JNI_TRUE, this is an internal VM class (either
 *                     an array or JEMCC class) and no exception is thrown
 *                     on a duplication
 *
 * Returns:
 *     JNI_OK - the class definition in the classloader table was successful
 *     JNI_ERR - a class already exists with the given classname and no
 *               insertion occurred (the existing class instance is returned 
 *               through the existingClass reference, if defined).  If the 
 *               internalClass flag is JNI_FALSE, a LinkageError exception 
 *               has been thrown in the current environment
 *     JNI_ENOMEM - a memory allocation has failed during the insertion of
 *                  the class and an OutOfMemory error has been thrown in the
 *                  current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     LinkageError - a class already exists in the classloader with the given
 *                    name.  This is only thrown if the internalClass flag is
 *                    JNI_FALSE
 */
JNIEXPORT jint JNICALL JEM_ClassNameSpaceStore(JNIEnv *env,
                                               JEMCC_Object *loader,
                                               JEMCC_Class *classInst,
                                               JEMCC_Class **existingClass,
                                               jboolean internalClass);

/**
 * Remove the given class instance from the classloader it belongs to.
 * There is no effect/exception if the given class instance no longer 
 * belongs to the owner classloader.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class to be removed from its owner classloader
 */
JNIEXPORT void JNICALL JEM_ClassNameSpaceRemove(JNIEnv *env,
                                                JEMCC_Class *classInst);

/**
 * Locate a method instance in the given class definition, using the
 * name and descriptor of the method in question.  Only scans the primary
 * linked method table (inheritance and interfaces accounted for), not
 * the local method table (i.e. this method will not locate the <init> or
 * <clinit> methods).
 *
 * Parameters:
 *     classData - the class definition to search for the method
 *     name - the name of the method to find
 *     descriptor - the descriptor of the method to find
 *
 * Returns:
 *     The method definition reference if the method was found, NULL otherwise.
 */
JNIEXPORT JEM_ClassMethodData *JNICALL
                      JEM_LocateClassMethod(JEM_ClassData *classData,
                                            const char *name,
                                            const char *descriptor);

/**
 * Locate a field instance in the given class definition, using the
 * name and descriptor of the field in question.  Will scan local field
 * definitions, then recurse over superinterfaces and superclasses
 * according to the sequence defined in the Java VM specification.
 *
 * Parameters:
 *     classData - the class definition to search for the field
 *     name - the name of the field to find
 *     descriptor - the descriptor of the field to find
 *
 * Returns:
 *     The method definition reference if the method was found, NULL otherwise.
 */
JNIEXPORT JEM_ClassFieldData *JNICALL
                          JEM_LocateClassField(JEM_ClassData *classData,
                                               const char *name,
                                               const char *descriptor);

/**
 * Define and resolve a bytecode class instance based on the provided
 * parse information.  This will create and store the class instance in the
 * provided classloader, resolving all classes required to construct the
 * hierarchy tables.   While this method does validate the linkage information
 * (for format, etc.), it does not perform the actual external linking and
 * bytecode verification - this occurs during initialization.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader which is to receive the class instance.  If
 *              NULL, the VM bootstrap classloader is used
 *     pData - the parsed bytecode class information.  Once passed to this
 *             method, this parse information is "owned" by the class (caller
 *             must not free/destroy it)
 *
 * Returns:
 *     The fully defined and resolved bytecode class or NULL if a fatal
 *     processing error occurred.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatError - a data error occurred while parsing the method
 *                        code information
 *     NoClassDefFoundError - a class required for resolving was not found or
 *                            was not loadable by the VM
 */
JNIEXPORT JEMCC_Class *JNICALL JEM_DefineAndResolveClass(JNIEnv *env,
                                                   JEMCC_Object *loader,
                                                   JEM_ParsedClassData *pData);

/**
 * Construct the external class, method and field linkages as required by
 * the bytecode class.  Moved into a separate method to simplify multiple
 * exit (error) points and associated class cleanup.
 *
 * Note: some error conditions result in immediate failure of this method
 *       (and hence the class instance).  Other missing class/method/field
 *       exceptions are held until the reference is used (late binding).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the compiled bytecode class information, for which the
 *                 linkage information is obtained
 *
 * Returns:
 *     JNI_OK - the linkage information was successfully constructed
 *     JNI_ENOMEM - a memory allocation error occurred
 *     JNI_ERR - a memory or immediately fatal error occurred (an exception
 *               has been thrown in the current environment)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassFormatException - an invalid reference descriptor was found
 *     <OtherClassErrors> - an immediately fatal condition occurred in the
 *                          loading of an exception class
 */
JNIEXPORT jint JNICALL JEM_LinkClassReferences(JNIEnv *env, 
                                               JEM_ClassData *classData);


/**
 * Perform the verification tests on the method bytecode as described
 * in the Java VM specifications.  While the Pass One and Pass Two validations
 * are mainly handled by the parsing and linking operations, this method
 * performs the algorithmically more complex Pass Three and Four verifications.
 *
 * Note: this method also performs a compaction on the class constant
 *       information, removing entries which exist for class definition and
 *       only leaving those required by the executing bytecode.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the partially resolved/linked class instance information
 *
 * Returns:
 *     JNI_OK - the class method bytecode verification was successful
 *     JNI_ERR - a verification error has occurred.  A VerifyError has been
 *               thrown in the current environment
 *     JNI_ENOMEM - a memory allocation has failed during the creation of the
 *                  verification tables and an OutOfMemory error has been
 *                  thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     VerifyError - the bytecode verification has failed
 */
JNIEXPORT jint JNICALL JEM_VerifyClassByteCode(JNIEnv *env,
                                               JEM_ClassData *classData);

/**
 * Internal method to construct array class instances on demand.  This method
 * manually constructs the JEMCC_ArrayClass instance (which overlays the
 * standard JEMCC_Class structure), utilizing the base class method/field
 * structures from the primitive array instance.  Note that the generated
 * array class instance for object-based arrays are stored in the
 * component classloader (while primitive-based arrays are stored in the
 * VM loader), to reduce the number of definitions of the array class.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - the classloader from which the array class is to be defined.  
 *              NULL if the VM bootstrap loader is to be used
 *     arrayName - the name of the array instances (Java descriptor).  Parsed
 *                 for depth as well as primitive/component class information
 *     isLink - if JNI_TRUE, this array is being defined as part of the 
 *              construction of another class (linking) and should throw
 *              NoClassDefFoundError on build failure.  If JNI_FALSE, should
 *              throw ClassNotFoundException instead (from a forName() call,
 *              for instance)
 *     localOnly - determines how the component class is "located" for 
 *                 object-based arrays.  If JNI_TRUE, only look in the
 *                 hashtable of the provided classloader (used for manual
 *                 classloading).  If JNI_FALSE, use the LocateClass method
 *                 to search for the class instance (used for linking).  Only
 *                 the latter case will throw an exception if the class is
 *                 not found.
 *     classInst - a class instance reference through which the constructed
 *                 array class instance is returned (if successful)
 *
 * Returns:
 *     JNI_OK - the array class was successfully constructed and the class
 *              instance has been returned through the classInst reference
 *     JNI_ERR - an error was found in the specification of the array
 *               name (invalid depth or bad component descriptor - an
 *               exception has been thrown in the current environment, based
 *               on the isLink value) or a component class load error occurred
 *               (with exception)
 *     JNI_ENOMEM - a memory allocation error has occurred and an OutOfMemory
 *                  error has been thrown in the current environment
 *     JNI_EINVAL - the requested array was object based and the associated
 *                  component class was not found.  An exception has been
 *                  thrown in the current environment, based on the isLink
 *                  value, if the localOnly value was false (linking requires
 *                  class to be found).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     NoClassDefFoundError - the array specification was invalid or the
 *                            associated component class was not found
 *     ClassNotFoundException - the array specification was invalid or the
 *                              associated component class was not found
 */
JNIEXPORT jint JNICALL JEM_BuildArrayClass(JNIEnv *env, JEMCC_Object *loader, 
                                           const char *arrayName, 
                                           jboolean isLink, jboolean localOnly,
                                           JEMCC_Class **classInst);

/**
 * Parse the library definition file for the VM included JEMCC packages.
 * Populates the preconfigured package loading and initialization structures
 * in the bootstrap classloader of the VM.  NOTE: this method is only intended
 * to be used during VM initialization and is not MT-safe.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     pkgFileData - the contents of the VM/JEMCC package/library definition
 *                   file (this string will be modified during parsing)
 *     pkgFileLen - the number of bytes contained in the file data
 *
 * Returns:
 *     JNI_OK - the classloader lock was successful
 *     JNI_ENOMEM - a memory allocation of the package information failed
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
JNIEXPORT jint JNICALL JEM_ParseLibPackageInfo(JNIEnv *env, char *pkgFileData, 
                                               int pkgFileLen);

/**
 * Collection method used to destroy class instances and attachments.  Used
 * for cleanup operations following errors during class creation and for
 * final destruction of a class instance from the garbage collector.  NOTE:
 * this method does not verify if a class is in use or not - it is intended
 * for internal use by the VM management components only.
 *
 * This method will release both the main attachment data as well as the
 * JEMCC_Object instance.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classInst - the class instance to be destroyed
 */
JNIEXPORT void JNICALL JEM_DestroyClassInstance(JNIEnv *env,
                                                JEMCC_Class *classInst);

/**
 * Convenience method for dumping internal class information (for debugging
 * purposes).  All output goes to the stderr channel.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     dbgClass - the class instance to be debugged
 */
JNIEXPORT void JNICALL JEM_DumpDebugClass(JNIEnv *env, JEMCC_Class *dbgClass);

/**
 * Convenience method to grab a class instance from a given classloader.
 * Essentially does a lock/get/unlock to retrieve from the local hashtable
 * for the loader (i.e. does NOT perform the full Java classload sequence,
 * use JEMCC_LocateClass for that).  Note that this method will also detect
 * and wait on a class instance which is defined but in the progress of
 * being resolved.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     loader - if non-NULL, lock and examine the associated loader hashtable,
 *              otherwise utilize the bootstrap hashtable in the VM
 *     className - the name of the class to be retrieved
 *     classInst - a class reference through which the requested class is
 *                 returned (if found)
 *
 * Returns:
 *     JNI_OK - the lookup occurred successfully and the requested class was
 *              found
 *     JNI_ERR - a circular class resolution case was detected (an exception
 *               has been thrown in the current environment)
 *     JNI_ENOMEM - a memory allocation error occurred
 *     JNI_EINVAL - the lookup was successful but the requested class was
 *                  not found (no exception is thrown in this case)
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     ClassCircularityError - the requested class is being resolved by
 *                             the current thread
 */
JNIEXPORT jint JNICALL JEM_RetrieveClass(JNIEnv *env, JEMCC_Object *loader,
                                         const char *className, 
                                         JEMCC_Class **classInst);

#endif
