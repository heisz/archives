/**
 * JEMCC definitions of the java.lang.Class class.
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

/* Internal class, needs to access JEMCC internal details */
#include "jem.h"
#include "jnifunc.h"

static jint JEMCC_Class_forName_String(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_Class *loadClass, *currentClass = JEMCC_GetCallingClassInstance(env);
    JEMCC_Object *className = JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_Object *currentLoader = NULL;
    JEMCC_StringData *strData;

    if (className == NULL) return JEMCC_NULL_EXCEPTION;

    strData = (JEMCC_StringData *) ((JEMCC_ObjectExt *) className)->objectData;
    if (currentClass != NULL) {
        currentLoader = currentClass->classData->classLoader;
    }
    if (strData->length == 0) {
        /* XXX - no chance of this being a class */
        return JEMCC_ERR;
    }
    if (strData->length > 0) {
        /* XXX - UTF-8 it! */
    }
    if (JEMCC_LocateClass(env, currentLoader, (char *) &(strData->data),
                          JNI_FALSE, &loadClass) != JNI_OK) {
        return JEMCC_ERR;
    }

    /* As per JLS, this load always initializes */
    if (JEMCC_InitializeClass(env, loadClass) != JNI_OK) {
        return JEMCC_ERR;
    }

    retVal->objVal = (JEMCC_Object *) loadClass;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_getClassLoader(JNIEnv *env,
                                       JEMCC_VMFrame *frame,
                                       JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);

    retVal->objVal = thisClass->classData->classLoader;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_getClasses(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getComponentType(JNIEnv *env,
                                         JEMCC_VMFrame *frame,
                                         JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getConstructor_ClassArray(JNIEnv *env,
                                                  JEMCC_VMFrame *frame,
                                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getConstructors(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getDeclaredClasses(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getDeclaredConstructor_ClassArray(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getDeclaredConstructors(JNIEnv *env,
                                                JEMCC_VMFrame *frame,
                                                JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;
    JEMCC_ArrayObject *arrayObj;
    JEMCC_Class *ctorClass;
    JEMCC_ObjectExt *ctor;
    int i, cnt;

    /* TODO - check member access (declared) */

    /* Determine the number of constructors */
    for (i = 0, cnt = 0; i < classData->localMethodCount; i++) {
        if (strcmp(classData->localMethods[i].name, "<init>") == 0) cnt++;
    }

    /* Allocate the constructor array */
    if (JEMCC_LocateClass(env, NULL, "java.lang.reflect.Constructor",
                          JNI_FALSE, &ctorClass) != JNI_OK) return JEMCC_ERR;
    arrayObj = JEMCC_NewObjectArray(env, cnt, ctorClass, NULL);
    if (arrayObj == NULL) return JEMCC_ERR;

    /* Fill it in from the local field list */
    for (i = 0, cnt = 0; i < classData->localMethodCount; i++) {
        if (strcmp(classData->localMethods[i].name, "<init>") != 0) continue;

        ctor = (JEMCC_ObjectExt *) JEMCC_AllocateObject(env, ctorClass, 0);
        if (ctor == NULL) return JEMCC_ERR;

        ctor->objectData = &(classData->localMethods[i]);

        *(((JEMCC_Object **) arrayObj->arrayData) + cnt++) = 
                                                     (JEMCC_Object *) ctor;
    }

    retVal->objVal = (JEMCC_Object *) arrayObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_getDeclaredField_String(JNIEnv *env,
                                                JEMCC_VMFrame *frame,
                                                JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getDeclaredFields(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;
    JEMCC_ArrayObject *arrayObj;
    JEMCC_Class *fieldClass;
    JEMCC_ObjectExt *field;
    int i;

    /* TODO - check member access (declared) */

    /* Build the field array */
    if (JEMCC_LocateClass(env, NULL, "java.lang.reflect.Field",
                          JNI_FALSE, &fieldClass) != JNI_OK) return JEMCC_ERR;
    arrayObj = JEMCC_NewObjectArray(env, classData->localFieldCount,
                                    fieldClass, NULL);
    if (arrayObj == NULL) return JEMCC_ERR;

    /* Fill it in from the local field list */
    for (i = 0; i < classData->localFieldCount; i++) {
        field = (JEMCC_ObjectExt *) JEMCC_AllocateObject(env, fieldClass, 0);
        if (field == NULL) return JEMCC_ERR;

        field->objectData = &(classData->localFields[i]);

        *(((JEMCC_Object **) arrayObj->arrayData) + i) = (JEMCC_Object *) field;
    }

    retVal->objVal = (JEMCC_Object *) arrayObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_getDeclaredMethod_StringClassArray(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getDeclaredMethods(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;
    JEMCC_ArrayObject *arrayObj;
    JEMCC_Class *methodClass;
    JEMCC_ObjectExt *method;
    int i, cnt;

    /* TODO - check member access (declared) */

    /* Determine the number of methods */
    for (i = 0, cnt = 0; i < classData->localMethodCount; i++) {
        if ((strcmp(classData->localMethods[i].name, "<init>") != 0) &&
            (strcmp(classData->localMethods[i].name, "<clinit>") != 0)) cnt++;
    }

    /* Allocate the method array */
    if (JEMCC_LocateClass(env, NULL, "java.lang.reflect.Method",
                          JNI_FALSE, &methodClass) != JNI_OK) return JEMCC_ERR;
    arrayObj = JEMCC_NewObjectArray(env, cnt, methodClass, NULL);
    if (arrayObj == NULL) return JEMCC_ERR;

    /* Fill it in from the local field list */
    for (i = 0, cnt = 0; i < classData->localMethodCount; i++) {
        if (strcmp(classData->localMethods[i].name, "<init>") == 0) continue;

        method = (JEMCC_ObjectExt *) JEMCC_AllocateObject(env, methodClass, 0);
        if (method == NULL) return JEMCC_ERR;

        method->objectData = &(classData->localMethods[i]);

        *(((JEMCC_Object **) arrayObj->arrayData) + cnt++) = 
                                                     (JEMCC_Object *) method;
    }

    retVal->objVal = (JEMCC_Object *) arrayObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_getDeclaringClass(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getField_String(JNIEnv *env,
                                        JEMCC_VMFrame *frame,
                                        JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getFields(JNIEnv *env,
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getInterfaces(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;
    JEMCC_ArrayObject *arrayObj;
    JEMCC_Class **ifWalkPtr;
    int i;

    /* Build the class return array */
    arrayObj = JEMCC_NewObjectArray(env, classData->interfaceCount,
                                    thisClass->classReference, NULL);
    if (arrayObj == NULL) return JEMCC_ERR;

    /* Fill it in from primary interface assignment list */
    ifWalkPtr = classData->assignList + 1;
    for (i = 0; i < classData->interfaceCount; i++) {
        *(((JEMCC_Object **) arrayObj->arrayData) + i) =
                                         (JEMCC_Object *) *ifWalkPtr;
        ifWalkPtr++;
    }

    retVal->objVal = (JEMCC_Object *) arrayObj;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_getMethod_StringClassArray(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getMethods(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getModifiers(JNIEnv *env,
                                     JEMCC_VMFrame *frame,
                                     JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getName(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;

    retVal->objVal = JEMCC_NewStringUTF(env, classData->className);

    return (retVal->objVal == NULL) ? JEMCC_ERR : JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_getResource_String(JNIEnv *env,
                                           JEMCC_VMFrame *frame,
                                           JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getResourceAsStream_String(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getSigners(JNIEnv *env,
                                   JEMCC_VMFrame *frame,
                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_Class_getSuperclass(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;

    /* While interfaces are objects, they don't actually look that way */
    if ((classData->accessFlags & ACC_INTERFACE) == 0) {
        retVal->objVal = (JEMCC_Object *) *(classData->assignList);
    } else {
        retVal->objVal = (JEMCC_Object *) NULL;
    }

    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_isArray(JNIEnv *env,
                                JEMCC_VMFrame *frame,
                                JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;

    retVal->intVal = ((classData->accessFlags & ACC_ARRAY) != 0) ? JNI_TRUE:
                                                                   JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Class_isAssignableFrom_Class(JNIEnv *env,
                                               JEMCC_VMFrame *frame,
                                               JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_Class *subClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 1);

    retVal->intVal = JEMCC_IsAssignableFrom(env, (jclass) subClass, 
                                                 (jclass) thisClass);
    return JEMCC_RET_INT;
}

static jint JEMCC_Class_isInstance_Object(JNIEnv *env,
                                          JEMCC_VMFrame *frame,
                                          JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEMCC_Object *targObj = JEMCC_LOAD_OBJECT(frame, 1);

    retVal->intVal = JEMCC_IsAssignableFrom(env, 
                                            (jclass) targObj->classReference, 
                                            (jclass) thisClass);
    return JEMCC_RET_INT;
}

static jint JEMCC_Class_isInterface(JNIEnv *env,
                                    JEMCC_VMFrame *frame,
                                    JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *clData = thisClass->classData;

    retVal->intVal = ((clData->accessFlags & ACC_INTERFACE) != 0) ? JNI_TRUE:
                                                                    JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Class_isPrimitive(JNIEnv *env,
                                    JEMCC_VMFrame *frame,
                                    JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *clData = thisClass->classData;

    retVal->intVal = ((clData->accessFlags & ACC_PRIMITIVE) != 0) ? JNI_TRUE:
                                                                    JNI_FALSE;
    return JEMCC_RET_INT;
}

static jint JEMCC_Class_newInstance(JNIEnv *env,
                                    JEMCC_VMFrame *frame,
                                    JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);

    /* TODO checkMemberAccess on security, this and Member.PUBLIC */
    /* TODO checkPackageAccess */

    /* Use the convenience method, and return if successful */
    retVal->objVal = JEMCC_NewObjectInstance(env, frame, thisClass, "()V");
    if (retVal->objVal == NULL) return JEMCC_ERR;
    return JEMCC_RET_OBJECT;
}

static jint JEMCC_Class_toString(JNIEnv *env,
                                 JEMCC_VMFrame *frame,
                                 JEMCC_ReturnValue *retVal) {
    JEMCC_Class *thisClass = (JEMCC_Class *) JEMCC_LOAD_OBJECT(frame, 0);
    JEM_ClassData *classData = thisClass->classData;

    if ((classData->accessFlags & ACC_INTERFACE) != 0) {
        retVal->objVal = JEMCC_StringCatUTF(env, "interface ", 
                                            classData->className, NULL);
    } else if ((classData->accessFlags & ACC_PRIMITIVE) != 0) {
        retVal->objVal = JEMCC_NewStringUTF(env, classData->className);
    } else {
        retVal->objVal = JEMCC_StringCatUTF(env, "class ", 
                                            classData->className, NULL);
    }
    if (retVal->objVal == NULL) return JEMCC_ERR;

    return JEMCC_RET_OBJECT;
}

JEMCC_MethodData JEMCC_ClassMethods[] = {
    { ACC_PUBLIC | ACC_STATIC,
         "forName", "(Ljava/lang/String;)Ljava/lang/Class;",
         JEMCC_Class_forName_String },
    { ACC_PUBLIC,
         "getClassLoader", "()Ljava/lang/ClassLoader;",
         JEMCC_Class_getClassLoader },
    { ACC_PUBLIC,
         "getClasses", "()[Ljava/lang/Class;",
         JEMCC_Class_getClasses },
    { ACC_PUBLIC,
         "getComponentType", "()Ljava/lang/Class;",
         JEMCC_Class_getComponentType },
    { ACC_PUBLIC,
         "getConstructor", 
         "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;",
         JEMCC_Class_getConstructor_ClassArray },
    { ACC_PUBLIC,
         "getConstructors", "()[Ljava/lang/reflect/Constructor;",
         JEMCC_Class_getConstructors },
    { ACC_PUBLIC,
         "getDeclaredClasses", "()[Ljava/lang/Class;",
         JEMCC_Class_getDeclaredClasses },
    { ACC_PUBLIC,
         "getDeclaredConstructor", 
         "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;",
         JEMCC_Class_getDeclaredConstructor_ClassArray },
    { ACC_PUBLIC,
         "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;",
         JEMCC_Class_getDeclaredConstructors },
    { ACC_PUBLIC,
         "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;",
         JEMCC_Class_getDeclaredField_String },
    { ACC_PUBLIC,
         "getDeclaredFields", "()[Ljava/lang/reflect/Field;",
         JEMCC_Class_getDeclaredFields },
    { ACC_PUBLIC,
         "getDeclaredMethod", 
         "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;",
         JEMCC_Class_getDeclaredMethod_StringClassArray },
    { ACC_PUBLIC,
         "getDeclaredMethods", "()[Ljava/lang/reflect/Method;",
         JEMCC_Class_getDeclaredMethods },
    { ACC_PUBLIC,
         "getDeclaringClass", "()Ljava/lang/Class;",
         JEMCC_Class_getDeclaringClass },
    { ACC_PUBLIC,
         "getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;",
         JEMCC_Class_getField_String },
    { ACC_PUBLIC,
         "getFields", "()[Ljava/lang/reflect/Field;",
         JEMCC_Class_getFields },
    { ACC_PUBLIC,
         "getInterfaces", "()[Ljava/lang/Class;",
         JEMCC_Class_getInterfaces },
    { ACC_PUBLIC,
         "getMethod", 
         "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;",
         JEMCC_Class_getMethod_StringClassArray },
    { ACC_PUBLIC,
         "getMethods", "()[Ljava/lang/reflect/Method;",
         JEMCC_Class_getMethods },
    { ACC_PUBLIC,
         "getModifiers", "()I",
         JEMCC_Class_getModifiers },
    { ACC_PUBLIC,
         "getName", "()Ljava/lang/String;",
         JEMCC_Class_getName },
    { ACC_PUBLIC,
         "getResource", "(Ljava/lang/String;)Ljava/net/URL;",
         JEMCC_Class_getResource_String },
    { ACC_PUBLIC,
         "getResourceAsStream", "(Ljava/lang/String;)Ljava/io/InputStream;",
         JEMCC_Class_getResourceAsStream_String },
    { ACC_PUBLIC,
         "getSigners", "()[Ljava/lang/Object;",
         JEMCC_Class_getSigners },
    { ACC_PUBLIC,
         "getSuperclass", "()Ljava/lang/Class;",
         JEMCC_Class_getSuperclass },
    { ACC_PUBLIC,
         "isArray", "()Z",
         JEMCC_Class_isArray },
    { ACC_PUBLIC,
         "isAssignableFrom", "(Ljava/lang/Class;)Z",
         JEMCC_Class_isAssignableFrom_Class },
    { ACC_PUBLIC,
         "isInstance", "(Ljava/lang/Object;)Z",
         JEMCC_Class_isInstance_Object },
    { ACC_PUBLIC,
         "isInterface", "()Z",
         JEMCC_Class_isInterface },
    { ACC_PUBLIC,
         "isPrimitive", "()Z",
         JEMCC_Class_isPrimitive },
    { ACC_PUBLIC,
         "newInstance", "()Ljava/lang/Object;",
         JEMCC_Class_newInstance },
    { ACC_PUBLIC,
         "toString", "()Ljava/lang/String;",
         JEMCC_Class_toString }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC | ACC_FINAL,
                     "java.lang.Class",
                     NULL ** java/lang/Object **,
                     interfaces ** java/io/Serializable,  **, 1,
                     JEMCC_ClassMethods, 32, NULL,
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
