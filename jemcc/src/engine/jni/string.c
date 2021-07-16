/**
 * JEMCC methods to support the JNI string interfaces.
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

jsize JEMCC_GetStringLength(JNIEnv *env, jstring str) {
    JEMCC_StringData *strData = (JEMCC_StringData *) 
                                         ((JEMCC_ObjectExt *) str)->objectData;
    int len = strData->length;
    if (len < 0) len = -len;

    return len;
}

const jchar *JEMCC_GetStringChars(JNIEnv *env, jstring str, jboolean *isCopy) {
    return NULL; /* TODO */
}

void JEMCC_ReleaseStringChars(JNIEnv *env, jstring str, const jchar *chars) {
    /* TODO */
}

jsize JEMCC_GetStringUTFLength(JNIEnv *env, jstring str) {
    return 0; /* TODO */
}

const char *JEMCC_GetStringUTFChars(JNIEnv *env, jstring str,
                                    jboolean *isCopy) {
    return NULL; /* TODO */
}

void JEMCC_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *chars) {
    /* TODO */
}
