/**
 * JEMCC definitions of the wrdg.jemcc.SysClassLoader class.
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

static jint JEMCC_SysClassLoader_init(JNIEnv *env,
                                      JEMCC_VMFrame *frame,
                                      JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_SysClassLoader_getResource_String(JNIEnv *env,
                                                    JEMCC_VMFrame *frame,
                                                    JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_SysClassLoader_getResourceAsStream_String(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_SysClassLoader_loadClass_String(JNIEnv *env,
                                                  JEMCC_VMFrame *frame,
                                                  JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

static jint JEMCC_SysClassLoader_loadClass_StringZ(JNIEnv *env,
                                                   JEMCC_VMFrame *frame,
                                                   JEMCC_ReturnValue *retVal) {
    JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_Throwable,
                               NULL, "TODO - not implemented");
    return JEMCC_ERR;
}

JEMCC_MethodData JEMCC_SysClassLoaderMethods[] = {
    { ACC_PROTECTED,
         "<init>", "()V",
         JEMCC_SysClassLoader_init },
    { ACC_PUBLIC,
         "getResource", "(Ljava/lang/String;)Ljava/net/URL;",
         JEMCC_SysClassLoader_getResource_String },
    { ACC_PUBLIC,
         "getResourceAsStream", "(Ljava/lang/String;)Ljava/io/InputStream;",
         JEMCC_SysClassLoader_getResourceAsStream_String },
    { ACC_PUBLIC,
         "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;",
         JEMCC_SysClassLoader_loadClass_String },
    { ACC_PROTECTED | ACC_SYNCHRONIZED,
         "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;",
         JEMCC_SysClassLoader_loadClass_StringZ }
};

/*
JEMCC_CreateStdClass(env, loader, ACC_PUBLIC | ACC_FINAL,
                     "wrdg.jemcc.SysClassLoader",
                     NULL ** java/lang/ClassLoader **,
                     interfaces, 0,
                     JEMCC_SysClassLoaderMethods, 5, NULL
                     NULL, 0,
                     NULL, 0, NULL, classInstance);
*/
