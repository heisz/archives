/**
 * JEMCC methods to support the JNI class interface methods.
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
#include "jnifunc.h"

jclass JEMCC_DefineClass(JNIEnv *env, jobject loader, const jbyte *buff, 
                         jsize buffLen) {
    JEM_ParsedClassData *classData;
    JEMCC_Class *classInstance;

    classData = JEM_ParseClassData(env, buff, buffLen);
    if (classData == NULL) return NULL;
    classInstance = JEM_DefineAndResolveClass(env, loader, classData);
    return (jclass) classInstance;
}

jclass JEMCC_FindClass(JNIEnv *env, const char *name) {
    JEM_JavaVM *jvm = (JEM_JavaVM *) ((JEM_JNIEnv *) env)->parentVM;
    JEM_VMFrameExt *frame = ((JEM_JNIEnv *) env)->topFrame;
    JEMCC_Object *classLoader = NULL;
    JEMCC_Class *classInst;
    jint rc;

    /* Determine the classloader context for the current environment frame */
    if ((frame->opFlags & FRAME_TYPE_MASK) == FRAME_ROOT) {
        /* Root frames have no object context, use system classloader */
        classLoader = jvm->systemClassLoader;
    } else {
    }

    /* Now, locate the class, potentially using loader context */
    rc = JEMCC_LocateClass(env, classLoader, (char *) name,
                           JNI_FALSE, &classInst);
    if (rc != JNI_OK) return NULL;
    return classInst;
}

jclass JEMCC_GetSuperclass(JNIEnv *env, jclass clazz) {
    return (jclass) *(((JEMCC_Class *) clazz)->classData->assignList);
}

jboolean JEMCC_IsAssignableFrom(JNIEnv *env, jclass clazza, jclass clazzb) {
    JEM_ClassData *classData = ((JEMCC_Class *) clazza)->classData;
    JEMCC_Class **compClassPtr;

    /* TODO Catch the array comparison case! */

    /* Are they the same class, i.e. same pointer in this context */
    if (clazza == clazzb) return JNI_TRUE;

    /* Is clazzb a superclass or interface of clazza */
    compClassPtr = classData->assignList;
    while (*compClassPtr != NULL) {
        if (*compClassPtr == (JEMCC_Class *) clazzb) return JNI_TRUE;
        compClassPtr++;
    }

    /* All tests have failed */
    return JNI_FALSE;
}
