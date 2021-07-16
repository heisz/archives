/**
 * System specific assembly code for IA-32 foreign function interface calls.
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

# No data in this method

.text

#
# Actual assembly method used to call JNI methods using FFI.
# Function prototype/stack mapping is as follows (recall that return
# address and %ebp storage take "first" 8 bytes of stack):
#
#    JEM_FFICallFn(void *fnRef,      [ebp-8]
#                  void *argStack,   [ebp-12]
#                  jint argSize,     [ebp-16]
#                  jint retType      [ebp-20] );
#     
# Note that the argStack will have sufficient storage area
# for the return value, regardless of argument depth.
#

.globl JEM_FFICallFn
    .type JEM_FFICallFn,@function

JEM_FFICallFn:
    # Store our current offset register and load current stack position
    pushl %ebp
    movl %esp, %ebp

    # Allocate stack space for the passed arguments
    movl 16(%ebp), %ecx
    subl %ecx, %esp

    # Copy preset argument stack into position
    pushl %ecx
    movl 12(%ebp), %eax
    pushl %eax
    movl %esp, %eax
    addl $8, %eax
    pushl %eax
    call memcpy
    addl $12, %esp

    # Make the call to the foreign function
    call *8(%ebp)

    # Clean up the stack space (we are responsible for removing arg space)
    movl 16(%ebp), %ecx
    addl %ecx, %esp

    # Handle the return types (void is easiest to do first)
    movl 20(%ebp), %ecx
    cmpl $0, %ecx
    je callcomplete

tryint:
    # Note that this covers all of the word "smaller" values
    cmpl $21, %ecx  # BASETYPE_Int
    jne trylong
    # Dump the integer value in %ecx to the temporary stack area
    movl 12(%ebp), %ecx
    movl %eax, (%ecx)
    jmp callcomplete

trylong:
    cmpl $22, %ecx  # BASETYPE_Long
    jne tryfloat
    # Dump the double word in %eax/%edx to the temporary stack area
    movl 12(%ebp), %ecx
    movl %eax, (%ecx)
    movl %edx, 4(%ecx)
    jmp callcomplete

tryfloat:
    cmpl $20, %ecx  # BASETYPE_Float
    jne trydouble
    # Dump the coprocessor float value to the temporary stack area
    movl 12(%ebp), %ecx
    fstps (%ecx)
    jmp callcomplete

trydouble:
    cmpl $19, %ecx  # BASETYPE_Double
    jne tryobject
    # Dump the coprocessor double value to the temporary stack area
    movl 12(%ebp), %ecx
    fstpl (%ecx)
    jmp callcomplete

tryobject:
    cmpl $32, %ecx  # DESCRIPTOR_ObjectType
    jne callcomplete
    # Similar to integer result (pointer in %eax)
    movl 12(%ebp), %ecx
    movl %eax, (%ecx)

    # Restore stack and offset register, then return
callcomplete:
    movl %ebp, %esp
    popl %ebp
    ret
