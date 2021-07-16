/**
 * JEMCC virtual machine functions to support Java class parsing.
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

/* Read the VM structure/method definitions */
#include "jem.h"

static char *endOfDataMsg = "Unexpected end of data while parsing class";

/* Local methods to construct u1/u2/u4 data from the class stream */
static u1 read_u1(const jubyte **ptr) {
    return (u1) *((*ptr)++);
}
static u2 read_u2(const jubyte **ptr) {
    return ((u2) (read_u1(ptr) << 8)) | read_u1(ptr);
}
static u4 read_u4(const jubyte **ptr) {
    return ((u4) (read_u1(ptr) << 24)) | (read_u1(ptr) << 16) |
                 (read_u1(ptr) << 8) | read_u1(ptr);
}

/* Buffer sizes for the various constant pool record.  Includes the tag! */
static int tagCheckSize[] = { 0,  /* Nothing */
                              3,  /* Utf8 */
                              0,  /* Nothing */
                              5,  /* Integer */
                              5,  /* Float */
                              9,  /* Long */
                              9,  /* Double */
                              3,  /* Class */
                              3,  /* String */
                              5,  /* Fieldref */
                              5,  /* Methodref */
                              5,  /* InterfaceMethodref */
                              5,  /* NameAndType */
                            };

/**
 * Read the constant pool information from the provided data buffer.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the parsing structure into which the constant pool
 *                 data is to be populated 
 *     buffPtr - the pointer reference to the binary class data
 *     buffLen - the number of bytes remaining in the binary buffer
 *
 * Returns:
 *     JNI_OK if parsing was successful, JNI_ERR otherwise (an exception
 *     will have been thrown in the current environment)
 *
 * Exceptions:
 *     ClassFormatError - the constant pool data is invalid or corrupted
 *     OutOfMemoryError - a memory allocation failed
 *
 * TODO - properly handle IEEE754 bit to float/double conversions, and long
 *        endian conversions (system dependent).
 */
static jint readConstantPool(JNIEnv *env, JEM_ParsedClassData *classData, 
                             const jubyte **buffPtr, jsize *buffLen) {
    jsize constantPoolCount;
    juint checkLen;
    u1 constantTag;
    JEM_ConstantPoolData *poolPtr;
    struct { u4 t1; u4 t2; } converter;
    int i;

    /* Get the count and allocate the primary record storage */
    constantPoolCount = read_u2(buffPtr);
    *buffLen -= 2;
    if (constantPoolCount < 2) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                   "Invalid class constant pool count");
        return JNI_ERR;
    }
    classData->constantPool = (JEM_ConstantPoolData *)
                                  JEMCC_Malloc(env, 
                                               (constantPoolCount - 1) * 
                                                sizeof(JEM_ConstantPoolData));
    if (classData->constantPool == NULL) {
        classData->constantPoolCount = 0;
        return JNI_ERR;
    }
    classData->constantPoolCount = constantPoolCount;

    /* Read in the set of pool entries */
    poolPtr = classData->constantPool;
    for (i = 1; i < constantPoolCount; i++) {
        /* Read the type, checking for sizes along the way */
        if ((checkLen = *buffLen) < 1) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, 
                                       NULL, endOfDataMsg);
            return JNI_ERR;
        }
        constantTag = read_u1(buffPtr);
        if (constantTag > 12) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                       "Unknown constant pool entry type");
            return JNI_ERR;
        }
        if (checkLen < tagCheckSize[constantTag]) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                       NULL, endOfDataMsg);
            return JNI_ERR;
        }
        *buffLen -= tagCheckSize[constantTag];

        /* Read the data as required */
        switch (constantTag) {
            case CONSTANT_Class:
                poolPtr->class_info.nameIndex = read_u2(buffPtr);
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
                poolPtr->ref_info.classIndex = read_u2(buffPtr);
                poolPtr->ref_info.nameAndTypeIndex = read_u2(buffPtr);
                break;
            case CONSTANT_String:
                poolPtr->string_info.stringIndex = read_u2(buffPtr);
                break;
            case CONSTANT_Integer:
                converter.t1 = read_u4(buffPtr);
                poolPtr->integer_info.constTblIndex = -1;
                poolPtr->integer_info.value = *((jint *) (&(converter.t1)));
                break;
            case CONSTANT_Float:
                converter.t1 = read_u4(buffPtr);
                poolPtr->float_info.constTblIndex = -1;
                poolPtr->float_info.value = *((jfloat *) (&(converter.t1)));
                break;
            case CONSTANT_Long:
                converter.t2 = read_u4(buffPtr);
                converter.t1 = read_u4(buffPtr);
                poolPtr->long_info.constTblIndex = -1;
                poolPtr->long_info.value = *((jlong *) (&converter));
                break;
            case CONSTANT_Double:
                converter.t2 = read_u4(buffPtr);
                converter.t1 = read_u4(buffPtr);
                poolPtr->double_info.constTblIndex = -1;
                poolPtr->double_info.value = *((jdouble *) (&converter));
                break;
            case CONSTANT_NameAndType:
                poolPtr->nameandtype_info.nameIndex = read_u2(buffPtr);
                poolPtr->nameandtype_info.descIndex = read_u2(buffPtr);
                break;
            case CONSTANT_Utf8:
                checkLen = read_u2(buffPtr);
                if (*buffLen < checkLen) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                               JEMCC_Class_ClassFormatError,
                                               NULL, endOfDataMsg);
                    return JNI_ERR;
                }
                /* Note that this auto-terminates the utf8 string (if ascii) */
                poolPtr->utf8_info.bytes = 
                                 (unsigned char *) JEMCC_Malloc(env, 
                                                                checkLen + 1);
                if (poolPtr->utf8_info.bytes == NULL) return JNI_ERR;
                (void) memcpy(poolPtr->utf8_info.bytes, *buffPtr, checkLen);
                *buffLen -= checkLen;
                *buffPtr += checkLen;
                break;
            default:
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                     NULL, "Unknown constant pool entry type");
                return JNI_ERR;
        }

        /* Only save the type once everything is complete */
        poolPtr->generic.tag = constantTag;
        poolPtr++;
        if ((constantTag == CONSTANT_Long) || 
            (constantTag == CONSTANT_Double)) { 
            i++;
            if (i >= constantPoolCount) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                     NULL, "Incorrect long/double end offset");
                return JNI_ERR;
            }
            poolPtr++;
        }
    }

    return JNI_OK;
}

/**
 * Destroy all constant pool information contained in the parsed class
 * data.  Safe to call at any time during parsing.
 *
 * Parameters:
 *     classData - the parsing structure containing the constant pool
 *                 information to be deleted
 */
static void destroyConstantPool(JEM_ParsedClassData *classData) {
    int i;

    /* Free the constant pool internals (utf8 allocations only) */
    for (i = 0; i < classData->constantPoolCount - 1; i++) {
        if (classData->constantPool[i].generic.tag == CONSTANT_Utf8) {
            if (classData->constantPool[i].utf8_info.bytes != NULL) {
                JEMCC_Free(classData->constantPool[i].utf8_info.bytes);
            }
        }
    }
    JEMCC_Free(classData->constantPool);
}

/**
 * Validate the constant pool information at the given index.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the parsing structure containing the constant pool data
 *     index - the constant pool index to validate (class data basis)
 *     checkTag - if greater than zero, validate the constant pool entry
 *                at the given index against this type
 *     typeMsg - the message to include in the exception if the validation
 *               fails
 *
 * Returns:
 *     JNI_OK if validation passed, JNI_ERR otherwise (an exception
 *     will have been thrown in the current environment)
 *
 * Exceptions:
 *     ClassFormatError - the index or the constant pool type is invalid
 */
static jint checkConstantPoolType(JNIEnv *env, JEM_ParsedClassData *classData,
                                  jint index, jint checkTag, char *typeMsg) {
    if ((index < 1) || (index >= classData->constantPoolCount)) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                   "Invalid constant pool index");
        return JNI_ERR;
    }

    if (checkTag > 0) {
        index--;
        if ((classData->constantPool[index].generic.tag & 0x0F) != checkTag) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, 
                                       NULL, typeMsg);
            return JNI_ERR;
        }
    }

    return JNI_OK;
}

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
int JEM_ReadConstantPoolIndex(JNIEnv *env, const jbyte **buffer,
                              JEM_ParsedClassData *classData,
                              jint checkTag, char *typeMsg) {
    jint index = read_u2((const jubyte **) buffer);

    if (checkTag <= 0) {
        if (index == 0) return 0;
        if (checkConstantPoolType(env, classData, index, 
                                  -checkTag, typeMsg) != JNI_OK) return -1;
    } else {
        if (checkConstantPoolType(env, classData, index, 
                                  checkTag, typeMsg) != JNI_OK) return -1;
    }
    return index;
}

/**
 * Read the class interface information from the provided data buffer.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the parsing structure into which the interface
 *                 data is to be populated 
 *     buffPtr - the pointer reference to the binary class data
 *     buffLen - the number of bytes remaining in the binary buffer
 *
 * Returns:
 *     JNI_OK if parsing was successful, JNI_ERR otherwise (an exception
 *     will have been thrown in the current environment)
 *
 * Exceptions:
 *     ClassFormatError - the interface information could not be read
 *     OutOfMemoryError - a memory allocation failed
 */
static jint readInterfaces(JNIEnv *env, JEM_ParsedClassData *classData, 
                           const jubyte **buffPtr, jsize *buffLen) {
    int i;

    /* Get the interface count and allocate the storage array */
    jsize interfacesCount = read_u2(buffPtr);
    *buffLen -= 2;
    if (*buffLen < (interfacesCount * 2)) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfDataMsg);
        return JNI_ERR;
    }

    if (interfacesCount != 0) {
        classData->interfaces = (jint *) 
                                  JEMCC_Malloc(env, 
                                               interfacesCount * sizeof(jint));
        if (classData->interfaces == NULL) {
            classData->interfacesCount = 0;
            return JNI_ERR;
        }
    } else {
        classData->interfaces = NULL;
    }
    classData->interfacesCount = interfacesCount;

    /* Read in the interface list */
    for (i = 0; i < interfacesCount; i++) {
        classData->interfaces[i] = read_u2(buffPtr);
    }
    *buffLen -= interfacesCount * 2;

    return JNI_OK;
}

/**
 * Destroy the interface references contained in the parsed class
 * data.  Safe to call at any time during parsing.
 *
 * Parameters:
 *     classData - the parsing structure containing the interface
 *                 information to be deleted
 */
static void destroyInterfaces(JEM_ParsedClassData *classData) {
    /* Just free the allocated array */
    JEMCC_Free(classData->interfaces);
}

/**
 * Read the attribute information from the provided data buffer.  Note that
 * this reader is used for class attributes, method attributes, field
 * attributes and method code attributes.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     attrData - the parsing structure into which the attributes are to
 *                be populated
 *     buffPtr - the pointer reference to the binary class data
 *     buffLen - the number of bytes remaining in the binary buffer
 *
 * Returns:
 *     JNI_OK if parsing was successful, JNI_ERR otherwise (an exception
 *     will have been thrown in the current environment)
 *
 * Exceptions:
 *     ClassFormatError - the attribute information could not be read
 *     OutOfMemoryError - a memory allocation failed
 */
static jint readAttributes(JNIEnv *env, JEM_ParsedAttributeData *attrData, 
                           const jubyte **buffPtr, jsize *buffLen) {
    jsize attributesCount;
    juint attributeLen;
    struct ATTRIBUTE_info *attPtr;
    int i;

    /* Obtain the count and allocate the primary array */
    attributesCount = read_u2(buffPtr);
    *buffLen -= 2;
    if (*buffLen < attributesCount * 6) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfDataMsg);
        return JNI_ERR;
    }

    if (attributesCount != 0) {
        attrData->attributes = (struct ATTRIBUTE_info *) 
                                  JEMCC_Malloc(env, attributesCount *
                                               sizeof(struct ATTRIBUTE_info));
        if (attrData->attributes == NULL) {
            attrData->attributesCount = 0;
            return JNI_ERR;
        }
    } else {
        attrData->attributes = NULL;
    }
    attrData->attributesCount = attributesCount;

    attPtr = attrData->attributes;
    for (i = 0; i < attributesCount; i++) {
        if (*buffLen < 6) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                       NULL, endOfDataMsg);
            return JNI_ERR;
        }
        attPtr->attributeNameIndex = read_u2(buffPtr);
        attributeLen = attPtr->attributeLength = read_u4(buffPtr);
        *buffLen -= 6;
        if (*buffLen < attributeLen) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                       NULL, endOfDataMsg);
            return JNI_ERR;
        }
        attPtr->info = (jbyte *) JEMCC_Malloc(env, attributeLen + 1);
        if (attPtr->info == NULL) return JNI_ERR;
        (void) memcpy(attPtr->info, *buffPtr, attributeLen);
        *buffPtr += attributeLen;
        *buffLen -= attributeLen;
        attPtr++;
    }

    return JNI_OK;
}

/**
 * Destroy the provided attribute information storage.  Safe to call 
 * at any time during parsing.
 *
 * Parameters:
 *     attrData - the parsing structure containing the attribute
 *                information to be deleted
 */
static void destroyAttributes(JEM_ParsedAttributeData *attrData) {
    int i;

    /* Free the attribute internals */
    for (i = 0; i < attrData->attributesCount; i++) {
        JEMCC_Free(attrData->attributes[i].info);
    }
    JEMCC_Free(attrData->attributes);
}

/**
 * Read the class field definition information from the provided data buffer.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the parsing structure into which the field information is
 *                 to be read
 *     buffPtr - the pointer reference to the binary class data
 *     buffLen - the number of bytes remaining in the binary buffer
 *
 * Returns:
 *     JNI_OK if parsing was successful, JNI_ERR otherwise (an exception
 *     will have been thrown in the current environment)
 *
 * Exceptions:
 *     ClassFormatError - the class field data is invalid or corrupted
 *     OutOfMemoryError - a memory allocation failed
 */
static jint readFieldInfo(JNIEnv *env, JEM_ParsedClassData *classData, 
                          const jubyte **buffPtr, jsize *buffLen) {
    jsize fieldsCount;
    JEM_ParsedFieldData *fieldPtr;
    int i;

    /* Get the count and allocate the primary record storage */
    fieldsCount = read_u2(buffPtr);
    *buffLen -= 2;
    if (fieldsCount != 0) {
        classData->fields = (JEM_ParsedFieldData *)
                 JEMCC_Malloc(env, fieldsCount * sizeof(JEM_ParsedFieldData));
        if (classData->fields == NULL) {
            classData->fieldsCount = 0;
            return JNI_ERR;
        }
    } else {
        classData->fields = NULL;
    }
    classData->fieldsCount = fieldsCount;

    /* Read in the set of field entries */
    fieldPtr = classData->fields;
    for (i = 0; i < fieldsCount; i++) {
        if (*buffLen < 8) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                       NULL, endOfDataMsg);
            return JNI_ERR;
        }
        fieldPtr->accessFlags = read_u2(buffPtr);
        fieldPtr->nameIndex = read_u2(buffPtr);
        fieldPtr->descIndex = read_u2(buffPtr);
        *buffLen -= 6;

        /* Grab the standard attribute data */
        if (readAttributes(env, 
                      (JEM_ParsedAttributeData *) &(fieldPtr->attributesCount),
                       buffPtr, buffLen) != JNI_OK) {
            return JNI_ERR;
        }

        fieldPtr++;
    }

    return JNI_OK;
}

/**
 * Destroy the field information contained in the parsed class
 * data.  Safe to call at any time during parsing.
 *
 * Parameters:
 *     classData - the parsing structure containing the field
 *                 information to be deleted
 */
static void destroyFieldInfo(JEM_ParsedClassData *classData) {
    int i;

    /* Free the field data internals */
    for (i = 0; i < classData->fieldsCount; i++) {
        destroyAttributes((JEM_ParsedAttributeData *) 
                          &(classData->fields[i].attributesCount));
    }
    JEMCC_Free(classData->fields);
}

/**
 * Read the class method definition information from the provided data buffer.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the parsing structure into which the method information is
 *                 to be read
 *     buffPtr - the pointer reference to the binary class data
 *     buffLen - the number of bytes remaining in the binary buffer
 *
 * Returns:
 *     JNI_OK if parsing was successful, JNI_ERR otherwise (an exception
 *     will have been thrown in the current environment)
 *
 * Exceptions:
 *     ClassFormatError - the class method data is invalid or corrupted
 *     OutOfMemoryError - a memory allocation failed
 */
static jint readMethodInfo(JNIEnv *env, JEM_ParsedClassData *classData, 
                           const jubyte **buffPtr, jsize *buffLen) {
    jsize methodsCount;
    JEM_ParsedMethodData *methodPtr;
    int i;

    /* Get the count and allocate the primary record storage */
    methodsCount = read_u2(buffPtr);
    *buffLen -= 2;
    if (methodsCount != 0) {
        classData->methods = (JEM_ParsedMethodData *)
                JEMCC_Malloc(env, methodsCount * sizeof(JEM_ParsedMethodData));
        if (classData->methods == NULL) {
            classData->methodsCount = 0;
            return JNI_ERR;
        }
    } else {
        classData->methods = NULL;
    }
    classData->methodsCount = methodsCount;

    /* Read in the set of method entries */
    methodPtr = classData->methods;
    for (i = 0; i < methodsCount; i++) {
        if (*buffLen < 8) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                       NULL, endOfDataMsg);
            return JNI_ERR;
        }
        methodPtr->accessFlags = read_u2(buffPtr);
        methodPtr->nameIndex = read_u2(buffPtr);
        methodPtr->descIndex = read_u2(buffPtr);
        *buffLen -= 6;

        /* Grab the standard attribute data */
        if (readAttributes(env, 
                     (JEM_ParsedAttributeData *) &(methodPtr->attributesCount),
                     buffPtr, buffLen) != JNI_OK) {
            return JNI_ERR;
        }

        methodPtr++;
    }

    return JNI_OK;
}

/**
 * Destroy the method information contained in the parsed class
 * data.  Safe to call at any time during parsing.
 *
 * Parameters:
 *     classData - the parsing structure containing the method
 *                 information to be deleted
 */
static void destroyMethodInfo(JEM_ParsedClassData *classData) {
    int i;

    /* Free the method data internals */
    for (i = 0; i < classData->methodsCount; i++) {
        destroyAttributes((JEM_ParsedAttributeData *) 
                          &(classData->methods[i].attributesCount));
    }
    JEMCC_Free(classData->methods);
}

/**
 * String comparison function which ignores separator differences in the
 * Java packaging information.
 *
 * Parameters:
 *     ptr - the string to compare against
 *     str - the string to be compared
 *
 * Returns:
 *     Non-zero if strings are equal except for / or . package separators.
 */
static int strcmpsep(char *ptr, char *str) {
    while (1) {
        if ((*ptr != *str) &&
                (((*ptr != '.') && (*ptr != '/')) ||
                 ((*str != '.') && (*str != '/')))) return 1;
        if (*ptr == '\0') return 0;
        ptr++; str++;
    }
}

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
JEM_ParsedClassData *JEM_ParseClassData(JNIEnv *env, const jbyte *buff, 
                                        jsize buffLen) {
    const jubyte *buffPtr = (const jubyte*) buff;
    u2 majorVersion, minorVersion;
    JEM_ParsedClassData *classData;
    int i, idx, j, k, cnt, state = JNI_OK, flags;
    struct ATTRIBUTE_info *attr;
    char *ptr, *str;
    jubyte *bptr;
    jint index;

    /* Must at least have enough data to read the signatures and counts */
    if (buffLen < 24) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfDataMsg);
        return NULL;
    }

    /* Check for some magic here */
    if (read_u4(&buffPtr) != 0xCAFEBABE) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                   "Class data has invalid magic number");
        return NULL;
    }

    /* Verify version information */
    minorVersion = read_u2(&buffPtr);
    majorVersion = read_u2(&buffPtr);
    if ((majorVersion < 45) || (majorVersion > 55)) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                   "Unsupported class version for JEMCC VM");
        return NULL;
    }
    buffLen -= 8;

    /* Now that the niceties are over, we can start building the class */
    if ((classData = (JEM_ParsedClassData *)
                     JEMCC_Malloc(env, sizeof(JEM_ParsedClassData))) == NULL) {
        return NULL;
    }

    /* Read the constant pool information */
    if (readConstantPool(env, classData, &buffPtr, &buffLen) != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Must still have enough data to read counts */
    if (buffLen < 14) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfDataMsg);
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Read and validate the class access indicators */
    classData->classAccessFlags = (juint) read_u2(&buffPtr);
    if ((classData->classAccessFlags & ACC_INTERFACE) != 0) {
        if ((classData->classAccessFlags | ACC_PUBLIC) != 
                            (ACC_PUBLIC | ACC_INTERFACE | ACC_ABSTRACT)) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                     "Interface class must also be abstract.");
            JEM_DestroyParsedClassData(classData);
            return NULL;
        }
    }
    if (((classData->classAccessFlags & ACC_FINAL) != 0) &&
        ((classData->classAccessFlags & ACC_ABSTRACT) != 0)) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                   "Final class cannot also be abstract.");
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Process the primary class definition information */
    classData->classIndex = read_u2(&buffPtr);
    classData->superClassIndex = read_u2(&buffPtr);
    buffLen -= 6;

    /* Read the class interface information */
    if (readInterfaces(env, classData, &buffPtr, &buffLen) != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Must still have enough data to read counts */
    if (buffLen < 6) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfDataMsg);
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Read the class field information */
    if (readFieldInfo(env, classData, &buffPtr, &buffLen) != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Must still have enough data to read counts */
    if (buffLen < 4) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfDataMsg);
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Read the class method information */
    if (readMethodInfo(env, classData, &buffPtr, &buffLen) != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Must still have enough data to read counts */
    if (buffLen < 2) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfDataMsg);
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Read the attribute information */
    if (readAttributes(env, 
                    (JEM_ParsedAttributeData *) &(classData->attributesCount),
                    &buffPtr, &buffLen) != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /* Must be an exact fit */
    if (buffLen != 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                   "Extra information following class data");
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    /*** Integrity checks of the class pool indices ***/

    /* Validate the type references of the constant pool internals */
    for (i = 0; i < classData->constantPoolCount - 1; i++) {
        state = JNI_OK;
        switch (classData->constantPool[i].generic.tag) {
            case CONSTANT_Class:
                idx = classData->constantPool[i].class_info.nameIndex;
                state = checkConstantPoolType(env, classData, idx,
                                          CONSTANT_Utf8, 
                                          "Invalid class constant name index");
                if (state == JNI_OK) {
                    bptr = classData->constantPool[idx - 1].utf8_info.bytes;
                    /* Quick check to avoid internal errors */
                    /* TODO - need to formalize this */
                    if ((*bptr <= 0x2F) || 
                           ((*bptr >= 0x3A) && (*bptr <= 0x40)) ||
                               ((*bptr >= 0x5C) && (*bptr <= 0x60)) ||
                                   ((*bptr >= 0x7B) && (*bptr <= 0x7F))) {
                        JEMCC_ThrowStdThrowableIdx(env, 
                                           JEMCC_Class_ClassFormatError, NULL,
                                           "Invalid classname lead character");
                        state = JNI_ERR;
                    }
                }
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
                state = checkConstantPoolType(env, classData,
                               classData->constantPool[i].ref_info.classIndex,
                               CONSTANT_Class, 
                               "Invalid reference constant class index");
                if (state == JNI_OK) {
                    state = checkConstantPoolType(env, classData,
                           classData->constantPool[i].ref_info.nameAndTypeIndex,
                           CONSTANT_NameAndType, 
                           "Invalid reference constant name and type index");
                }
                break;
            case CONSTANT_String:
                state = checkConstantPoolType(env, classData,
                             classData->constantPool[i].string_info.stringIndex,
                             CONSTANT_Utf8, 
                             "Invalid string constant data index");
                break;
            case CONSTANT_NameAndType:
                state = checkConstantPoolType(env, classData,
                          classData->constantPool[i].nameandtype_info.nameIndex,
                          CONSTANT_Utf8, 
                          "Invalid name/type constant name index");
                if (state == JNI_OK) {
                    state = checkConstantPoolType(env, classData,
                          classData->constantPool[i].nameandtype_info.descIndex,
                          CONSTANT_Utf8,
                          "Invalid name/type constant descriptor index");
                }
                break;
        }
        if (state != JNI_OK) {
            JEM_DestroyParsedClassData(classData);
            return NULL;
        }
    }

    /* Validate the class cross-references */
    if (checkConstantPoolType(env, classData, classData->classIndex,
                              CONSTANT_Class,
                              "Invalid primary class index") != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }
    if (classData->superClassIndex == 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                 "All classes must have a superclass defined");
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }
    if (checkConstantPoolType(env, classData, classData->superClassIndex,
                              CONSTANT_Class,
                              "Invalid super class index") != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }
    for (i = 0; i < classData->interfacesCount; i++) {
        index = classData->interfaces[i];
        if (checkConstantPoolType(env, classData, index, CONSTANT_Class,
                                  "Invalid interface class index") != JNI_OK) {
            JEM_DestroyParsedClassData(classData);
            return NULL;
        }

        /* Cross-check that there isn't a duplicate entry */
        index = classData->constantPool[index - 1].class_info.nameIndex;
        ptr = classData->constantPool[index - 1].utf8_info.bytes;
        for (j = 0; j < i; j++) {
            index = classData->interfaces[j];
            index = classData->constantPool[index - 1].class_info.nameIndex;
            str = classData->constantPool[index - 1].utf8_info.bytes;
            if (strcmpsep(ptr, str) == 0) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, "Repeated interface instance (name)");
                JEM_DestroyParsedClassData(classData);
                return NULL;
            }
        }
    }

    /* Verify that an interface has superclass Object and at most one superif */
    if ((classData->classAccessFlags & ACC_INTERFACE) != 0) {
        index = classData->superClassIndex;
        index = classData->constantPool[index - 1].class_info.nameIndex;
        ptr = classData->constantPool[index - 1].utf8_info.bytes;
        if (strcmpsep(ptr, "java/lang/Object") != 0) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                        "Interfaces must have superclass of java.lang.Object");
            JEM_DestroyParsedClassData(classData);
            return NULL;
        }
    }

    /* Validate the type indices and access flags for the field data */
    for (i = 0; i < classData->fieldsCount; i++) {
        state = JNI_OK;
        state = checkConstantPoolType(env, classData,
                                      classData->fields[i].nameIndex,
                                      CONSTANT_Utf8, 
                                      "Invalid field name index");
        if (state == JNI_OK) {
            state = checkConstantPoolType(env, classData,
                                          classData->fields[i].descIndex,
                                          CONSTANT_Utf8, 
                                          "Invalid field descriptor index");
        }
        for (j = 0; j < classData->fields[i].attributesCount; j++) {
            if (state == JNI_OK) {
                state = checkConstantPoolType(env, classData,
                          classData->fields[i].attributes[j].attributeNameIndex,
                          CONSTANT_Utf8, 
                          "Invalid field attribute name index");
            }
        }

        flags = classData->fields[i].accessFlags;
        if (state == JNI_OK) {
            cnt = 0;
            if ((flags & ACC_PUBLIC) != 0) cnt++;
            if ((flags & ACC_PROTECTED) != 0) cnt++;
            if ((flags & ACC_PRIVATE) != 0) cnt++;
            if (cnt > 1) {
                JEMCC_ThrowStdThrowableIdx(env, 
                        JEMCC_Class_ClassFormatError, NULL,
                        "Fields must be only one of public/protected/private");
                state = JNI_ERR;
            }
        }
        if ((state == JNI_OK) && ((flags & ACC_FINAL) != 0) && 
            ((flags & ACC_VOLATILE) != 0)) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                                       "Fields cannot be final and volatile");
            state = JNI_ERR;
        }
        if ((state == JNI_OK) && 
            ((classData->classAccessFlags & ACC_INTERFACE) != 0) &&
            ((flags & 0xFFF) != (ACC_PUBLIC | ACC_FINAL | ACC_STATIC))) {
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, NULL,
                               "Interface fields must be public/final/static");
            state = JNI_ERR;
        }

        if (state != JNI_OK) {
            JEM_DestroyParsedClassData(classData);
            return NULL;
        }
    }

    /* Validate the type indices and access flags for the method data */
    for (i = 0; i < classData->methodsCount; i++) {
        state = JNI_OK;
        state = checkConstantPoolType(env, classData,
                                      classData->methods[i].nameIndex,
                                      CONSTANT_Utf8, 
                                      "Invalid method name index");
        if (state == JNI_OK) {
            state = checkConstantPoolType(env, classData,
                                          classData->methods[i].descIndex,
                                          CONSTANT_Utf8, 
                                          "Invalid method descriptor index");
        }
        for (j = 0; j < classData->methods[i].attributesCount; j++) {
            attr = &(classData->methods[i].attributes[j]);
            if (state == JNI_OK) {
                state = checkConstantPoolType(env, classData,
                                      attr->attributeNameIndex, CONSTANT_Utf8, 
                                      "Invalid method attribute name index");
            }
            if (state == JNI_OK) {
                /* Validate exceptions definition */
                ptr = classData->constantPool[attr->attributeNameIndex
                                                           - 1].utf8_info.bytes;
                if (strcmp(ptr, "Exceptions") == 0) {
                    if (attr->attributeLength < 2) {
                        JEMCC_ThrowStdThrowableIdx(env, 
                                     JEMCC_Class_ClassFormatError, NULL,
                                     "Unexpected end of exceptions attribute");
                        state = JNI_ERR;
                    } else {
                        buffPtr = attr->info;
                        cnt = read_u2(&buffPtr);
                        if (attr->attributeLength != (2 + cnt * 2)) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                             JEMCC_Class_ClassFormatError, NULL,
                                             "Invalid exceptions list length");
                            state = JNI_ERR;
                        } else {
                            for (k = 0; ((k < cnt) && (state == JNI_OK)); k++) {
                                index = read_u2(&buffPtr);
                                state = checkConstantPoolType(env, classData,
                                              index, CONSTANT_Class, 
                                              "Invalid exceptions list entry");
                            }
                        }
                    }
                }
            }
        }

        ptr = classData->constantPool[classData->methods[i].nameIndex 
                                                          - 1].utf8_info.bytes;
        if ((state == JNI_OK) && (strcmp(ptr, "<clinit>") != 0)) {
            cnt = 0;
            flags = classData->methods[i].accessFlags;
            if ((flags & ACC_PUBLIC) != 0) cnt++;
            if ((flags & ACC_PROTECTED) != 0) cnt++;
            if ((flags & ACC_PRIVATE) != 0) cnt++;
            if (cnt > 1) {
                JEMCC_ThrowStdThrowableIdx(env, 
                       JEMCC_Class_ClassFormatError, NULL,
                       "Methods must be only one of public/protected/private");
                state = JNI_ERR;
            }
            if ((state == JNI_OK) && ((flags & ACC_ABSTRACT) != 0) &&
                ((flags & (ACC_FINAL | ACC_NATIVE | ACC_PRIVATE | ACC_STATIC |
                           ACC_STRICT | ACC_SYNCHRONIZED)) != 0)) {
                JEMCC_ThrowStdThrowableIdx(env, 
                   JEMCC_Class_ClassFormatError, NULL,
                   "Abstract methods cannot be fnl/ntv/prvt/synch/sttc/strct");
                state = JNI_ERR;
            }
            if ((state == JNI_OK) && (strcmp(ptr, "<init>") == 0) &&
                ((flags & (0xFFF & (~(ACC_PRIVATE | ACC_PROTECTED |
                                      ACC_PUBLIC | ACC_STRICT)))) != 0)) {
                JEMCC_ThrowStdThrowableIdx(env, 
                       JEMCC_Class_ClassFormatError, NULL,
                       "Initialization methods cannot be native/synch/static");
                state = JNI_ERR;
            }
            if ((state == JNI_OK) && 
                ((classData->classAccessFlags & ACC_INTERFACE) != 0) &&
                ((flags & 0xFFF) != (ACC_PUBLIC | ACC_ABSTRACT))) {
                JEMCC_ThrowStdThrowableIdx(env, 
                                  JEMCC_Class_ClassFormatError, NULL,
                                  "Interface methods must be public/abstract");
                state = JNI_ERR;
            }
        }

        if (state != JNI_OK) {
            JEM_DestroyParsedClassData(classData);
            return NULL;
        }
    }

    /* Finally, validate the class attributes */
    state = JNI_OK;
    for (j = 0; j < classData->attributesCount; j++) {
        if (state == JNI_OK) {
            state = checkConstantPoolType(env, classData,
                                   classData->attributes[j].attributeNameIndex,
                                   CONSTANT_Utf8, 
                                   "Invalid class attribute name index");
        }
    }
    if (state != JNI_OK) {
        JEM_DestroyParsedClassData(classData);
        return NULL;
    }

    return classData;
}

/**
 * Destroy the parsed class information, as returned from the ParseClassData
 * method above.
 *
 * Parameters:
 *     data - the parsing structure containing the class information to be
 *            destroyed
 */
void JEM_DestroyParsedClassData(JEM_ParsedClassData *data) {
    if (data == NULL) return;

    destroyConstantPool(data);
    destroyInterfaces(data);
    destroyFieldInfo(data);
    destroyMethodInfo(data);
    destroyAttributes((JEM_ParsedAttributeData *) &(data->attributesCount));
    JEMCC_Free(data);
}

static char *endOfCodeMsg = "Unexpected end of code attribute information";

/* Note: this method appears here due to u2 read dependencies */
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
JEM_BCMethod *JEM_ParseMethodCode(JNIEnv *env, const jbyte *buffer,
                                  jsize buffLen, 
                                  JEM_ParsedClassData *classData) {
    JEM_ParsedAttributeData codeAttributes;
    const jubyte *buffPtr = (const jubyte *) buffer;
    JEM_BCMethod *retVal;
    jint index;
    char *ptr;
    int i, j;

    /* Check/read the stack information */
    if (buffLen < 12) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfCodeMsg);
        return NULL;
    }
    retVal = (JEM_BCMethod *) JEMCC_Malloc(env, sizeof(JEM_BCMethod));
    if (retVal == NULL) return NULL;
    retVal->maxStack = read_u2(&buffPtr);
    retVal->maxLocals = read_u2(&buffPtr);

    /* Check/read the code information */
    retVal->codeLength = read_u4(&buffPtr);
    if (retVal->codeLength == 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, "No code body provided for method");
        JEM_DestroyMethodCode(retVal);
        return NULL;
    }
    buffLen -= 8 + retVal->codeLength;
    if (buffLen < 4) {
        retVal->codeLength = 0;
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfCodeMsg);
        JEM_DestroyMethodCode(retVal);
        return NULL;
    }
    retVal->code = JEMCC_Malloc(env, retVal->codeLength);
    if (retVal->code == NULL) {
        JEM_DestroyMethodCode(retVal);
        return NULL;
    }
    (void) memcpy(retVal->code, buffPtr, retVal->codeLength);
    buffPtr += retVal->codeLength;

    /* Check/read the exception block information */
    retVal->exceptionTableLength = read_u2(&buffPtr);
    buffLen -= retVal->exceptionTableLength * 8 + 2;
    if (buffLen < 2) {
        retVal->exceptionTableLength = 0;
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, endOfCodeMsg);
        JEM_DestroyMethodCode(retVal);
        return NULL;
    }
    if (retVal->exceptionTableLength != 0) {
        retVal->exceptionTable = (JEM_MethodExceptionBlock *)
            JEMCC_Malloc(env, retVal->exceptionTableLength *
                                         sizeof(JEM_MethodExceptionBlock));
        if (retVal->exceptionTable == NULL) {
            JEM_DestroyMethodCode(retVal);
            return NULL;
        }
    }
    for (i = 0; i < retVal->exceptionTableLength; i++) {
        retVal->exceptionTable[i].startPC = read_u2(&buffPtr);
        retVal->exceptionTable[i].endPC = read_u2(&buffPtr);
        retVal->exceptionTable[i].handlerPC = read_u2(&buffPtr);
        index = read_u2(&buffPtr);
        if (index == 0) {
            /* No class - this is a finally clause */
        } else {
            /* Must be a throwable class */
            if (checkConstantPoolType(env, classData, index, CONSTANT_Class,
                                  "Invalid exception class index") != JNI_OK) {
                JEM_DestroyMethodCode(retVal);
                return NULL;
            }
        }
        retVal->exceptionTable[i].exceptionClass.index = index;
    }

    /* Check/read the attribute information */
    if (readAttributes(env, &codeAttributes, &buffPtr, &buffLen) != JNI_OK) {
        /* Note that the standard parse behaviour leaves cleanup to here */
        destroyAttributes(&codeAttributes);
        JEM_DestroyMethodCode(retVal);
        return NULL;
    }
    for (i = 0; i < codeAttributes.attributesCount; i++) {
        index = codeAttributes.attributes[i].attributeNameIndex;
        if (checkConstantPoolType(env, classData, index, CONSTANT_Utf8, 
                              "Invalid code attribute name index") != JNI_OK) {
            JEM_DestroyMethodCode(retVal);
            destroyAttributes(&codeAttributes);
            return NULL;
        }
        ptr = classData->constantPool[index - 1].utf8_info.bytes;
#ifndef NO_JVM_DEBUG
        if (strcmp(ptr, "LineNumberTable") == 0) {
            if (codeAttributes.attributes[i].attributeLength < 2) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                           NULL, endOfCodeMsg);
                JEM_DestroyMethodCode(retVal);
                destroyAttributes(&codeAttributes);
                return NULL;
            }
            buffPtr = (jubyte *) codeAttributes.attributes[i].info;
            retVal->lineNumberTableLength = read_u2(&buffPtr);
            if (codeAttributes.attributes[i].attributeLength != 
                                 (2 + retVal->lineNumberTableLength * 4)) {
                retVal->lineNumberTableLength = 0;
                JEMCC_ThrowStdThrowableIdx(env, 
                             JEMCC_Class_ClassFormatError, NULL,
                             "Invalid code line number attribute information");
                JEM_DestroyMethodCode(retVal);
                destroyAttributes(&codeAttributes);
                return NULL;
            }
            if (retVal->lineNumberTableLength != 0) {
                retVal->lineNumberTable = (JEM_LineNumberEntry *)
                           JEMCC_Malloc(env, retVal->lineNumberTableLength *
                                                  sizeof(JEM_LineNumberEntry));
                if (retVal->lineNumberTable == NULL) {
                    retVal->lineNumberTableLength = 0;
                    JEM_DestroyMethodCode(retVal);
                    destroyAttributes(&codeAttributes);
                    return NULL;
                }
            }
            for (j = 0; j < retVal->lineNumberTableLength; j++) {
                retVal->lineNumberTable[j].startPC = read_u2(&buffPtr);
                retVal->lineNumberTable[j].lineNumber = read_u2(&buffPtr);
            }
        } else if (strcmp(ptr, "LocalVariableTable") == 0) {
            if (codeAttributes.attributes[i].attributeLength < 2) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                           NULL, endOfCodeMsg);
                JEM_DestroyMethodCode(retVal);
                destroyAttributes(&codeAttributes);
                return NULL;
            }
            buffPtr = (jubyte *) codeAttributes.attributes[i].info;
            retVal->localVariableTableLength = read_u2(&buffPtr);
            if (codeAttributes.attributes[i].attributeLength != 
                               (2 + retVal->localVariableTableLength * 10)) {
                retVal->localVariableTableLength = 0;
                JEMCC_ThrowStdThrowableIdx(env, 
                          JEMCC_Class_ClassFormatError, NULL,
                          "Invalid code local variable attribute information");
                JEM_DestroyMethodCode(retVal);
                destroyAttributes(&codeAttributes);
                return NULL;
            }
            if (retVal->localVariableTableLength != 0) {
                retVal->localVariableTable = (JEM_LocalVariableEntry *)
                           JEMCC_Malloc(env, retVal->localVariableTableLength *
                                               sizeof(JEM_LocalVariableEntry));
                if (retVal->localVariableTable == NULL) {
                    retVal->localVariableTableLength = 0;
                    JEM_DestroyMethodCode(retVal);
                    destroyAttributes(&codeAttributes);
                    return NULL;
                }
            }
            for (j = 0; j < retVal->localVariableTableLength; j++) {
                retVal->localVariableTable[j].startPC = read_u2(&buffPtr);
                retVal->localVariableTable[j].length = read_u2(&buffPtr);

                index = read_u2(&buffPtr);
                if (checkConstantPoolType(env, classData, index, CONSTANT_Utf8,
                               "Invalid local variable name index") != JNI_OK) {
                    JEM_DestroyMethodCode(retVal);
                    destroyAttributes(&codeAttributes);
                    return NULL;
                }
                ptr = classData->constantPool[index - 1].utf8_info.bytes;
                retVal->localVariableTable[j].name = JEMCC_StrDupFn(env, ptr);
                if (retVal->localVariableTable[j].name == NULL) {
                    JEM_DestroyMethodCode(retVal);
                    destroyAttributes(&codeAttributes);
                    return NULL;
                }

                index = read_u2(&buffPtr);
                if (checkConstantPoolType(env, classData, index, CONSTANT_Utf8, 
                               "Invalid local variable desc index") != JNI_OK) {
                    JEM_DestroyMethodCode(retVal);
                    destroyAttributes(&codeAttributes);
                    return NULL;
                }
                ptr = classData->constantPool[index - 1].utf8_info.bytes;
                retVal->localVariableTable[j].descriptor = 
                                                   JEMCC_StrDupFn(env, ptr);
                if (retVal->localVariableTable[j].descriptor == NULL) {
                    JEM_DestroyMethodCode(retVal);
                    destroyAttributes(&codeAttributes);
                    return NULL;
                }

                retVal->localVariableTable[j].index = read_u2(&buffPtr);
            }
        }
#endif
        /* Everything else is ignorable */
    }
    destroyAttributes(&codeAttributes);

    /* That should be a perfect fit */
    if (buffLen != 0) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, "Extra information in code attribute");
        JEM_DestroyMethodCode(retVal);
        return NULL;
    }

    return retVal;
}

/**
 * Destroy the parsed method code information, as returned from the
 * ParseMethodCode method above.
 *
 * Parameters:
 *     method - the bytecode method structure containing the information
 *              to be destroyed
 */
void JEM_DestroyMethodCode(JEM_BCMethod *method) {
#ifndef NO_JVM_DEBUG
    int i;
#endif

    JEMCC_Free(method->code);
    JEMCC_Free(method->exceptionTable);
#ifndef NO_JVM_DEBUG
    JEMCC_Free(method->lineNumberTable);
    for (i = 0; i < method->localVariableTableLength; i++) {
        JEMCC_Free(method->localVariableTable[i].name);
        JEMCC_Free(method->localVariableTable[i].descriptor);
    }
    JEMCC_Free(method->localVariableTable);
#endif

    JEMCC_Free(method);
}

/* * * * * * * * Descriptor Handling Methods * * * * * * * */

static char *descExtraMsg = "Extra text at end of descriptor";
static char *descInvalidMsg = "Invalid character in descriptor";

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
JEM_DescriptorData *JEM_ParseDescriptor(JNIEnv *env, const char *descriptor,
                                        JEM_DescriptorData *descBuffer,
                                        const char **nextPtr, 
                                        jboolean isStatic) {
    JEM_DescriptorData wrkVal, *retVal, paramArray[257];
    const char *cptr;
    char *ptr;
    int i = 0;

    /* Quick test for empty case */
    if (*descriptor == '\0') {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, "Provided descriptor is empty");
        return NULL;
    }

    /* Parse the descriptor information onto the local stack */
    switch (*descriptor) {
        case 'B':
            wrkVal.base_info.tag = BASETYPE_Byte;
            break;
        case 'C':
            wrkVal.base_info.tag = BASETYPE_Char;
            break;
        case 'D':
            wrkVal.base_info.tag = BASETYPE_Double;
            break;
        case 'F':
            wrkVal.base_info.tag = BASETYPE_Float;
            break;
        case 'I':
            wrkVal.base_info.tag = BASETYPE_Int;
            break;
        case 'J':
            wrkVal.base_info.tag = BASETYPE_Long;
            break;
        case 'S':
            wrkVal.base_info.tag = BASETYPE_Short;
            break;
        case 'Z':
            wrkVal.base_info.tag = BASETYPE_Boolean;
            break;
        case 'L':
            wrkVal.object_info.tag = DESCRIPTOR_ObjectType;
            wrkVal.object_info.className = (char *)
                                   JEMCC_Malloc(env, strlen(descriptor));
            if (wrkVal.object_info.className == NULL) return NULL;
            (void) strcpy(wrkVal.object_info.className, descriptor + 1);
            JEM_SlashToDot(wrkVal.object_info.className);
            ptr = wrkVal.object_info.className;
            while ((*ptr != '\0') && (*ptr != ')')) {
                if (*ptr == ';') break;
                ptr++;
            }
            if (*ptr != ';') {
                JEMCC_ThrowStdThrowableIdx(env, 
                             JEMCC_Class_ClassFormatError, NULL,
                             "Missing semi-colon from object type descriptor");
                JEMCC_Free(wrkVal.object_info.className);
                return NULL;
            }
            *(ptr++) = '\0';
            if (nextPtr == NULL) {
                if (*ptr != '\0') {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                               JEMCC_Class_ClassFormatError, 
                                               NULL, descExtraMsg);
                    JEMCC_Free(wrkVal.object_info.className);
                    return NULL;
                }
            } else {
                *nextPtr = descriptor + 
                                (ptr - wrkVal.object_info.className) + 1;
            }
            break;
        case '[':
            cptr = descriptor + 1;
            if (*cptr == '\0') {
                JEMCC_ThrowStdThrowableIdx(env, 
                                           JEMCC_Class_ClassFormatError, NULL,
                                           "Unterminated array descriptor");
                return NULL;
            }
            wrkVal.array_info.tag = DESCRIPTOR_ArrayType;
            wrkVal.array_info.componentType = 
                       JEM_ParseDescriptor(env, cptr, NULL, nextPtr, isStatic);
            if (wrkVal.array_info.componentType == NULL) return NULL;
            break;
        case '(':
            if (nextPtr != NULL) {
                JEMCC_ThrowStdThrowableIdx(env, 
                                      JEMCC_Class_ClassFormatError, NULL,
                                      "Method descriptors cannot be embedded");
                return NULL;
            }
            wrkVal.method_info.tag = DESCRIPTOR_MethodType;

            /* Parse the method parameter argument list */
            cptr = descriptor + 1;
            while ((*cptr != '\0') && (*cptr != ')')) {
                paramArray[i].generic.tag = DESCRIPTOR_EndOfList;
                if (JEM_ParseDescriptor(env, cptr, &(paramArray[i]), 
                                        &cptr, isStatic) == NULL) {
                    while (i >= 0) JEM_DestroyDescriptor(&(paramArray[i--]), 
                                                         JNI_FALSE);
                    return NULL;
                }
                if (((isStatic == JNI_FALSE) && (i >= 254)) || (i >= 255)) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                  JEMCC_Class_ClassFormatError, NULL,
                                  "Too many parameters for method descriptor");
                    while (i >= 0) JEM_DestroyDescriptor(&(paramArray[i--]),
                                                         JNI_FALSE);
                    return NULL;
                }
                i++;
            }
            paramArray[i].generic.tag = DESCRIPTOR_EndOfList;

            if (*cptr == '\0') {
                JEMCC_ThrowStdThrowableIdx(env, 
                                           JEMCC_Class_ClassFormatError, NULL,
                                           "Unterminated method descriptor");
                while (i >= 0) JEM_DestroyDescriptor(&(paramArray[i--]),
                                                     JNI_FALSE);
                return NULL;
            }

            /* Parse the return value information */
            if (*(++cptr) == 'V') {
                wrkVal.method_info.returnDescriptor = NULL;
                cptr++;
            } else {
                wrkVal.method_info.returnDescriptor = 
                           JEM_ParseDescriptor(env, cptr, NULL, &cptr, 
                                               isStatic);
                if (wrkVal.method_info.returnDescriptor == NULL) {
                    while (i >= 0) JEM_DestroyDescriptor(&(paramArray[i--]),
                                                         JNI_FALSE);
                    return NULL;
                }
            }
            if (*cptr != '\0') {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, 
                                           NULL, descExtraMsg);
                while (i >= 0) JEM_DestroyDescriptor(&(paramArray[i--]),
                                                     JNI_FALSE);
                if (wrkVal.method_info.returnDescriptor != NULL) {
                    JEM_DestroyDescriptor(wrkVal.method_info.returnDescriptor, 
                                          JNI_TRUE);
                }
                return NULL;
            }

            /* Duplicate the argument list */
            wrkVal.method_info.paramDescriptor = (JEM_DescriptorData *)
                               JEMCC_Malloc(env, (i + 1) * 
                                                   sizeof(JEM_DescriptorData));
            if (wrkVal.method_info.paramDescriptor == NULL) {
                while (i >= 0) JEM_DestroyDescriptor(&(paramArray[i--]),
                                                     JNI_FALSE);
                if (wrkVal.method_info.returnDescriptor != NULL) {
                    JEM_DestroyDescriptor(wrkVal.method_info.returnDescriptor,
                                          JNI_TRUE);
                }
                return NULL;
            }
            (void) memcpy(wrkVal.method_info.paramDescriptor, paramArray,
                          (i + 1) * sizeof(JEM_DescriptorData));
            break;
        default:
            JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                       NULL, descInvalidMsg);
            return NULL;
    }
    
    /* Check for proper termination of common descriptor elements */
    if ((wrkVal.generic.tag & 0xF0) == DESCRIPTOR_BaseType) {
        if (nextPtr == NULL) {
            if (*(descriptor + 1) != '\0') {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError, 
                                           NULL, descExtraMsg);
                return NULL;
            }
        } else {
            *nextPtr = descriptor + 1;
        }
    }

    /* If ok, allocate a new copy or dump it into the buffer */
    if (descBuffer == NULL) {
        retVal = (JEM_DescriptorData *) 
                             JEMCC_Malloc(env, sizeof(JEM_DescriptorData));
        if (retVal == NULL) {
            JEM_DestroyDescriptor(&wrkVal, JNI_FALSE);
            return NULL;
        }
    } else {
        retVal = descBuffer;
    }
    *retVal = wrkVal;

    return retVal;
}

/**
 * Destroys the descriptor instance and any nested descriptors (for methods).
 *
 * Parameters:
 *     data - the parsed descriptor instance to be destroyed
 *     freeStruct - if JNI_TRUE, free the main descriptor structure - this
 *                  should always be the case unless a descriptor buffer
 *                  was used in the parsing of the descriptor
 */
void JEM_DestroyDescriptor(JEM_DescriptorData *data, jboolean freeStruct) {
    JEM_DescriptorData *dptr;
    int i;

    switch (data->generic.tag) {
        case DESCRIPTOR_ObjectType:
            JEMCC_Free(data->object_info.className);
            break;
        case DESCRIPTOR_ArrayType:
            JEM_DestroyDescriptor(data->array_info.componentType, 1);
            break;
        case DESCRIPTOR_MethodType:
            if (data->method_info.returnDescriptor)
                JEM_DestroyDescriptor(data->method_info.returnDescriptor, 1);
            dptr = data->method_info.paramDescriptor;
            for (i = 0; i < 256; i++) {
                if (dptr->generic.tag == DESCRIPTOR_EndOfList) break;
                JEM_DestroyDescriptor(dptr, 0);
                dptr++;
            }
            JEMCC_Free(data->method_info.paramDescriptor);
            break;
        default:
            /* Nothing to do, nothing was allocated */
            break;
    }
    if (freeStruct != JNI_FALSE) JEMCC_Free(data);
}

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
jint JEM_ValidateDescriptor(JNIEnv *env, const char *descriptor,
                            jboolean isMethod) {
    jboolean inArray = JNI_FALSE, inReturn = JNI_FALSE;
    jint paramCount = 0;

    /* Quick test for empty or invalid method descriptor */
    if (*descriptor == '\0') {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, "Provided descriptor is empty");
        return JNI_ERR;
    }
    if ((isMethod == JNI_TRUE) && (*(descriptor++) != '(')) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, descInvalidMsg);
        return JNI_ERR;
    }

    /* Scan descriptor; once for field, repeatedly for method arguments */
    while (*descriptor != '\0') {
        switch (*descriptor) {
            case 'B':
            case 'C':
            case 'D':
            case 'F':
            case 'I':
            case 'J':
            case 'S':
            case 'Z':
                paramCount++;
                inArray = JNI_FALSE;
                break;
            case 'V':
                if ((inReturn != JNI_TRUE) || (inArray == JNI_TRUE)) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                               JEMCC_Class_ClassFormatError,
                                               NULL, descInvalidMsg);
                    return JNI_ERR;
                }
                break;
            case 'L':
                while ((*descriptor != '\0') && (*descriptor!= ')')) {
                    if (*descriptor== ';') break;
                    descriptor++;
                }
                if (*descriptor != ';') {
                    JEMCC_ThrowStdThrowableIdx(env, 
                             JEMCC_Class_ClassFormatError, NULL,
                             "Missing semi-colon from object type descriptor");
                    return JNI_ERR;
                }
                paramCount++;
                inArray = JNI_FALSE;
                break;
            case '[':
                inArray = JNI_TRUE;
                break;
            case '(':
                JEMCC_ThrowStdThrowableIdx(env, 
                                      JEMCC_Class_ClassFormatError, NULL,
                                      "Method descriptors cannot be embedded");
                return JNI_ERR;
            case ')':
                if (isMethod != JNI_TRUE) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                               JEMCC_Class_ClassFormatError,
                                               NULL, descInvalidMsg);
                    return JNI_ERR;
                }
                if (paramCount > 255) {
                    JEMCC_ThrowStdThrowableIdx(env, 
                                  JEMCC_Class_ClassFormatError, NULL,
                                  "Too many parameters for method descriptor");
                    return JNI_ERR;
                }
                break;
            default:
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                           NULL, descInvalidMsg);
                return JNI_ERR;
        }

        /* Need to catch this outside to avoid conflict with below */
        if (*(descriptor++) == ')') {
            inReturn = JNI_TRUE;
            continue;
        }

        /* Fields and return values have only have one descriptor element */
        if ((inArray == JNI_FALSE) && 
            ((isMethod == JNI_FALSE) || (inReturn == JNI_TRUE))) {
            if (*descriptor != '\0') {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                           NULL, descExtraMsg);
                return JNI_ERR;
            }
        }
    }

    /* Catch the unterminated cases */
    if ((isMethod == JNI_TRUE) && (inReturn == JNI_FALSE)) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, "Unterminated method descriptor");
        return JNI_ERR;
    }
    if (inArray == JNI_TRUE) {
        JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_ClassFormatError,
                                   NULL, "Unterminated array descriptor");
        return JNI_ERR;
    }

    return JNI_OK;
}

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
 *     of the descriptor.  If nestedEntry was zero, the return string must
 *     be freed by the caller.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
char *JEM_ConvertDescriptor(JNIEnv *env, JEM_DescriptorData *data,
                            jboolean nestedEntry) {
    char *ptr = NULL;
    int i;

    /* Initialize the string if we aren't in a nested call */
    if (nestedEntry == JNI_FALSE) {
        if (JEMCC_EnvStrBufferInit(env, -1) == NULL) return NULL;
    }

    /* Re-encode based on the generic descriptor type */
    switch (data->generic.tag) {
        case BASETYPE_Byte:
            ptr = "B";
            break;
        case BASETYPE_Char:
            ptr = "C";
            break;
        case BASETYPE_Double:
            ptr = "D";
            break;
        case BASETYPE_Float:
            ptr = "F";
            break;
        case BASETYPE_Int:
            ptr = "I";
            break;
        case BASETYPE_Long:
            ptr = "J";
            break;
        case BASETYPE_Short:
            ptr = "S";
            break;
        case BASETYPE_Boolean:
            ptr = "Z";
            break;
        case DESCRIPTOR_ObjectType:
            if (JEMCC_EnvStrBufferAppendSet(env, "L", 
                                            data->object_info.className,
                                            ";", NULL) == NULL) return NULL;
            break;
        case DESCRIPTOR_ArrayType:
            if (JEMCC_EnvStrBufferAppend(env, "[") == NULL) return NULL;
            if (JEM_ConvertDescriptor(env, data->array_info.componentType, 
                                      1) == NULL) return NULL;
            break;
        case DESCRIPTOR_MethodType:
            if (JEMCC_EnvStrBufferAppend(env, "(") == NULL) return NULL;
            for (i = 0; i < 256; i++) {
                if (data->method_info.paramDescriptor[i].generic.tag ==
                                               DESCRIPTOR_EndOfList) break;
                if (JEM_ConvertDescriptor(env, 
                                          &data->method_info.paramDescriptor[i],
                                          1) == NULL) return NULL;
            }
            if (JEMCC_EnvStrBufferAppend(env, ")") == NULL) return NULL;
            if (data->method_info.returnDescriptor == NULL) {
                if (JEMCC_EnvStrBufferAppend(env, "V") == NULL) return NULL;
            } else {
                if (JEM_ConvertDescriptor(env, 
                                          data->method_info.returnDescriptor, 
                                          1) == NULL) return NULL;
            }
            break;
        default:
            ptr = "X";
            break;
    }
    if (ptr != NULL) {
        if (JEMCC_EnvStrBufferAppend(env, ptr) == NULL) return NULL;
    }

    if (nestedEntry == JNI_FALSE) return JEMCC_EnvStrBufferDup(env);
    return (char *) 0xdead;
} 

/**
 * Internal method for providing a human readable description of a 
 * method parameter/return value.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     data - the parsed descriptor information to generation the parameter
 *            description from
 *
 * Returns:
 *     NULL if a memory failure occurred, otherwise the parameter value is
 *     appended onto the environment working string buffer and a non-NULL
 *     (and non-useable) value is returned.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
static char *JEM_ConvertParameter(JNIEnv *env, JEM_DescriptorData *data) {
    int arrayCount;
    char *ptr = NULL;

    /* Re-encode based on the generic descriptor type */
    switch (data->generic.tag) {
        case BASETYPE_Byte:
            ptr = "byte";
            break;
        case BASETYPE_Char:
            ptr = "char";
            break;
        case BASETYPE_Double:
            ptr = "double";
            break;
        case BASETYPE_Float:
            ptr = "float";
            break;
        case BASETYPE_Int:
            ptr = "int";
            break;
        case BASETYPE_Long:
            ptr = "long";
            break;
        case BASETYPE_Short:
            ptr = "short";
            break;
        case BASETYPE_Boolean:
            ptr = "boolean";
            break;
        case DESCRIPTOR_ObjectType:
            ptr = data->object_info.className;
            if (JEMCC_EnvStrBufferAppend(env, ptr) == NULL) return NULL;
            ptr = NULL;
            break;
        case DESCRIPTOR_ArrayType:
            /* Count the number of arrays */
            arrayCount = 0;
            while (data->generic.tag == DESCRIPTOR_ArrayType) {
                data = data->array_info.componentType;
                arrayCount++;
            }
            if (JEM_ConvertParameter(env, data) == NULL) return NULL;
            while (arrayCount != 0) {
                if (JEMCC_EnvStrBufferAppend(env, "[]") == NULL) return NULL;
                arrayCount--;
            }
            break;
        default:
            ptr = "X";
            break;
    }
    if (ptr != NULL) {
        if (JEMCC_EnvStrBufferAppend(env, ptr) == NULL) return NULL;
    }

    return (char *) 0xdead;
}

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
char *JEM_ConvertMethodDescriptor(JNIEnv *env, char *methodName,
                                  JEM_DescriptorData *data) {
    int i;

    if (JEMCC_EnvStrBufferInit(env, -1) == NULL) return NULL;

    /* Verify that we are looking at a method */
    if (data->generic.tag != DESCRIPTOR_MethodType) {
        return JEMCC_EnvStrBufferDup(env);
    }

    /* Return information/method name first */
    if (data->method_info.returnDescriptor == NULL) {
        if (JEMCC_EnvStrBufferAppend(env, "void ") == NULL) return NULL;
    } else {
        if (JEM_ConvertParameter(env, 
                     data->method_info.returnDescriptor) == NULL) return NULL;
        if (JEMCC_EnvStrBufferAppend(env, " ") == NULL) return NULL;
    }

    /* Argument set */
    if (JEMCC_EnvStrBufferAppendSet(env, methodName, 
                                    "(", NULL) == NULL) return NULL;
    for (i = 0; i < 256; i++) {
        if (data->method_info.paramDescriptor[i].generic.tag ==
                                               DESCRIPTOR_EndOfList) break;
        if (i != 0) {
            if (JEMCC_EnvStrBufferAppend(env, ",") == NULL) return NULL;
        }
        if (JEM_ConvertParameter(env, 
                    &data->method_info.paramDescriptor[i]) == NULL) return NULL;
    }
    if (JEMCC_EnvStrBufferAppend(env, ")") == NULL) return NULL;
    return JEMCC_EnvStrBufferDup(env);
}

/**
 * Convenience method to convert the internal class package directory
 * notation to the programmatic package dot notation.
 *
 * Parameters:
 *     buff - the string buffer containing the class name to be converted
 */
void JEM_SlashToDot(char *buff) {
    while (*buff != '\0') {
        if (*buff == '/') *buff = '.';
        buff++;
    }
}
