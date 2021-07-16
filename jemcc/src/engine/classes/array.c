/**
 * JEMCC definitions for elements available in the array primitive.
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

/**
 * Just one method for an array (as per the Java Language Specification).
 * Note that we could put this logic in the java.lang.Object clone() method,
 * but someone using reflection could detect the missing clone method
 * (and hence a problem could occur, however unlikely that would be).
 */
static jint JEMCC_ArrayPrim_clone(JNIEnv *env, 
                                  JEMCC_VMFrame *frame,
                                  JEMCC_ReturnValue *retVal) {
    JEMCC_Object *thisArr = JEMCC_LOAD_OBJECT(frame, 0);

    retVal->objVal = JEMCC_CloneObject(env, thisArr);
    if (retVal->objVal == NULL) return JEMCC_ERR;

    return JEMCC_RET_OBJECT;
}

JEMCC_MethodData JEMCC_ArrayPrimMethods[] = {
    { ACC_PROTECTED, 
         "clone", "()Ljava/lang/Object;", 
         JEMCC_ArrayPrim_clone }
};

/**
 * Just one field as well (the array length).  Note that the JEMCC_Array_Object
 * structure can be used to directly access the native data pointer and the
 * length element, without using the field accessor methods.  The offset is
 * predefined to skip over the native data element.
 */
JEMCC_FieldData JEMCC_ArrayPrimFields[] = {
    { ACC_PUBLIC | ACC_FINAL, "length", "I", sizeof(void *) }
};
