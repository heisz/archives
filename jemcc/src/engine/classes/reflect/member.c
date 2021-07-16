/**
 * JEMCC definitions of the java.lang.reflect.Member class.
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

JEMCC_MethodData JEMCC_MemberMethods[] = {
    { ACC_PUBLIC | ACC_ABSTRACT,
         "getDeclaringClass", "()Ljava/lang/Class;",
         NULL },
    { ACC_PUBLIC | ACC_ABSTRACT,
         "getModifiers", "()I",
         NULL },
    { ACC_PUBLIC | ACC_ABSTRACT,
         "getName", "()Ljava/lang/String;",
         NULL }
};

JEMCC_FieldData JEMCC_MemberFields[] = {
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "PUBLIC", "I", -1 },
    { ACC_PUBLIC | ACC_STATIC | ACC_FINAL, "DECLARED", "I", -1 }
};

/*
JEMCC_CreateStdClass(env, loader,
                     ACC_PUBLIC | ACC_INTERFACE,
                     "java.lang.reflect.Member",
                     NULL ** java/lang/Object **,
                     interfaces, 0,
                     JEMCC_MemberMethods, 3, NULL,
                     JEMCC_MemberFields, 2,
                     NULL, 0, NULL, classInstance);
*/
