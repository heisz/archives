/**
 * System specific monitor definitions for the Sparc-32 CPU architecture.
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

#define HAS_FETCH_AND_STORE 1

/**
 * The SWAP instruction swaps the contents of the memory address (targAddr)
 * and the given register value (swapVal).  Note that the input/output
 * definitions are aligned (the "0" constraint) to allow direct access to
 * the original value at the given memory address.
 */
#define FETCH_AND_STORE(swapVal, targAddr) \
    __asm__ __volatile__("swap %2, %0" \
                         : "=r" (swapVal) \
                         : "0" (swapVal), "m" (*targAddr) \
                         : "memory");
