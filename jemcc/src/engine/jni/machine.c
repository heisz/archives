/**
 * JEMCC methods to support the JNI machine interfaces.
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

jint JEMCC_RegisterNatives(JNIEnv *env, jclass clazz, 
                           const JNINativeMethod *methods, jint nMethods) {
    JEM_JNIEnv *jenv = (JEM_JNIEnv *) env;
    JEM_ClassData *clData = 
                     jenv->topFrame->currentMethod->parentClass->classData;
    JEMCC_Class *cls = JEMCC_GetCallingClassInstance(env);
    int i, idx;

    for (i = 0; i < nMethods; i++) {
        for (idx = 0; idx < clData->localMethodCount; idx++) {
            if ((strcmp(methods[i].name, 
                             clData->localMethods[idx].name) == 0) &&
                (strcmp(methods[i].signature, 
                             clData->localMethods[idx].descriptorStr) == 0)) {
                if ((clData->localMethods[idx].accessFlags & ACC_NATIVE) == 0) {
                    /* TODO - what to do here in this case */
                } else {
                    clData->localMethods[idx].method.ntvMethod = 
                                                          methods[i].fnPtr;
                }
                break;
            }
        }
        /* TODO - what to do in this case (no match) */
    }

    return JNI_OK;
}

jint JEMCC_UnregisterNatives(JNIEnv *env, jclass clazz) {
    return 0; /* TODO */
}
