/**
 * Initialization routines for the JEMCC java.lang.reflect package.
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
#include "jemcc.h"

/* Definitions appearing in all of the other "class" files */
extern JEMCC_MethodData JEMCC_ArrayMethods[];
extern JEMCC_MethodData JEMCC_ConstructorMethods[];
extern JEMCC_MethodData JEMCC_FieldMethods[];
extern JEMCC_MethodData JEMCC_InvocationTargetExceptionMethods[];
extern JEMCC_MethodData JEMCC_MemberMethods[];
extern JEMCC_FieldData JEMCC_MemberFields[];
extern JEMCC_MethodData JEMCC_MethodMethods[];
extern JEMCC_MethodData JEMCC_ModifierMethods[];
extern JEMCC_FieldData JEMCC_ModifierFields[];

/* Package classloader declaration (defined during initialization) */
jint JEM_reflect_ClassLoader(JNIEnv *env, JEMCC_Object *loader,
                             const char *className, JEMCC_Class **classInst) {
    JEMCC_Class *superClass;
    JEMCC_Class *interfaces[2];
    JEMCC_ReturnValue staticInitVals[15];
    const char *ptr = className;
    juint hashSum = 0;
    jint rc;

    /* Build a numerical case shortcut for the classname comparisons */
    while (*ptr != '\0') hashSum += (int) *(ptr++);

    switch (hashSum) {
        case 511:
            if (strcmp(className, "Array") == 0) {
                superClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Object);

                return JEMCC_CreateStdClass(env, loader,
                                            ACC_PUBLIC | ACC_FINAL,
                                            "java.lang.reflect.Array",
                                            superClass, NULL, 0,
                                            JEMCC_ArrayMethods, 21, 
                                            NULL, NULL, 0,
                                            NULL, 0, NULL, classInst);
            }
            break;
        case 1190:
            if (strcmp(className, "Constructor") == 0) {
                superClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Object);

                rc = JEMCC_LocateClass(env, loader, "java.lang.reflect.Member",
                                       JNI_FALSE, &interfaces[0]);
                if (rc != JNI_OK) return rc;

                return JEMCC_CreateStdClass(env, loader,
                                            ACC_PUBLIC | ACC_FINAL |
                                                             ACC_NATIVE_DATA,
                                            "java.lang.reflect.Constructor",
                                            superClass, interfaces, 1,
                                            JEMCC_ConstructorMethods, 9, 
                                            NULL, NULL, 0,
                                            NULL, 0, NULL, classInst);
            }
            break;
        case 484:
            if (strcmp(className, "Field") == 0) {
                superClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Object);

                rc = JEMCC_LocateClass(env, loader, "java.lang.reflect.Member",
                                       JNI_FALSE, &interfaces[0]);
                if (rc != JNI_OK) return rc;

                return JEMCC_CreateStdClass(env, loader,
                                            ACC_PUBLIC | ACC_FINAL |
                                                             ACC_NATIVE_DATA,
                                            "java.lang.reflect.Field",
                                            superClass, interfaces, 1,
                                            JEMCC_FieldMethods, 25, 
                                            NULL, NULL, 0,
                                            NULL, 0, NULL, classInst);
            }
            break;
        case 2608:
            if (strcmp(className, "InvocationTargetException") == 0) {
                superClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Exception);

                return JEMCC_CreateStdClass(env, loader, ACC_PUBLIC,
                                 "java.lang.reflect.InvocationTargetException",
                                 superClass, NULL, 0,
                                 JEMCC_InvocationTargetExceptionMethods, 4, 
                                 NULL, NULL, 0,
                                 NULL, 0, NULL, classInst);
            }
            break;
        case 600:
            if (strcmp(className, "Member") == 0) {
                superClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Object);

                staticInitVals[0].intVal = 0;
                staticInitVals[1].intVal = 1;

                return JEMCC_CreateStdClass(env, loader,
                                            ACC_PUBLIC | ACC_INTERFACE,
                                            "java.lang.reflect.Member",
                                            superClass, NULL, 0,
                                            JEMCC_MemberMethods, 3, NULL,
                                            JEMCC_MemberFields, 2,
                                            NULL, 0, staticInitVals, 
                                            classInst);
            }
            break;
        case 609:
            if (strcmp(className, "Method") == 0) {
                superClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Object);

                rc = JEMCC_LocateClass(env, loader, "java.lang.reflect.Member",
                                       JNI_FALSE, &interfaces[0]);
                if (rc != JNI_OK) return rc;

                return JEMCC_CreateStdClass(env, loader,
                                            ACC_PUBLIC | ACC_FINAL |
                                                             ACC_NATIVE_DATA,
                                            "java.lang.reflect.Method",
                                            superClass, interfaces, 1,
                                            JEMCC_MethodMethods, 10,
                                            NULL, NULL, 0,
                                            NULL, 0, NULL, classInst);
            }
            break;
        case 815:
            if (strcmp(className, "Modifier") == 0) {
                superClass = JEMCC_GetCoreVMClass(env, JEMCC_Class_Object);

                staticInitVals[0].intVal = ACC_PUBLIC;
                staticInitVals[1].intVal = ACC_PRIVATE;
                staticInitVals[2].intVal = ACC_PROTECTED;
                staticInitVals[3].intVal = ACC_STATIC;
                staticInitVals[4].intVal = ACC_FINAL;
                staticInitVals[5].intVal = ACC_SYNCHRONIZED;
                staticInitVals[6].intVal = ACC_VOLATILE;
                staticInitVals[7].intVal = ACC_TRANSIENT;
                staticInitVals[8].intVal = ACC_NATIVE;
                staticInitVals[9].intVal = ACC_INTERFACE;
                staticInitVals[10].intVal = ACC_ABSTRACT;

                return JEMCC_CreateStdClass(env, loader, ACC_PUBLIC,
                                            "java.lang.reflect.Modifier",
                                            superClass, NULL, 0,
                                            JEMCC_ModifierMethods, 13, NULL,
                                            JEMCC_ModifierFields, 11,
                                            NULL, 0, staticInitVals, 
                                            classInst);
            }
            break;
    }

    return JNI_EINVAL;
}
