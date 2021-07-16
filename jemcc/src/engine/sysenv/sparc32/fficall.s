/**
 * System specific assembly code for sparc32 foreign function interface calls.
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

! No data in this method

.text

!
! Actual assembly method used to call JNI methods using FFI.
! Function prototype/register mapping is as follows
!
!    JEM_FFICallFn(jint frameSize,   [%i0]
!                  void *fnRef,      [%i1]
!                  void *argStack,   [%i2]
!                  jint argSize,     [%i3]
!                  jint retType      [%i4]);
!     

! Cover both encoding forms for method names
.globl JEM_FFICallFn
.globl _JEM_FFICallFn

JEM_FFICallFn:
_JEM_FFICallFn:
    ! Perform the standard frame/window register setup
    ! Took a while to figure out that o0 isnt i0 until AFTER this is called
    save %sp, %o0, %sp

    ! Copy the provided argument set into the frame argument space
    ! Note that we offset from the current sp by 64 + 4 (start of argspace)
    add %sp, 68, %o0
    mov %i2, %o1
    mov %i3, %o2
    call memcpy, 0
    nop

    ! Preload the output/input arguments from the argspace
    ld [%i2], %o0
    ld [%i2 + 4], %o1
    ld [%i2 + 8], %o2
    ld [%i2 + 12], %o3
    ld [%i2 + 16], %o4
    ld [%i2 + 20], %o5

    ! Make the call to the foreign function (do nothing in delay slot)
    call %i1
    nop

    ! Handle the return types (void is easiest to handle first)
    tst %i4
    bne tryint
    nop
    ! All done, do the return/restore simultaneously (delay slot)
    ret
    restore

tryint:
    ! Note that this covers all of the word "smaller" values
    cmp %i4, 21  ! BASETYPE_Int
    bne trylong
    nop
    ! All done, store/return/restore (delay slot)
    st %o0, [%i2]
    ret
    restore

trylong:
    cmp %i4, 22  ! BASETYPE_Long
    bne tryfloat
    nop
    ! All done, store (in two parts), return/restore (delay slot)
    st %o0, [%i2]
    st %o1, [%i2 + 4]
    ret
    restore

tryfloat:
    cmp %i4, 20  ! BASETYPE_Float
    bne trydouble
    nop
    ! All done, store/return/restore (delay slot)
    st %f0, [%i2]
    ret
    restore

trydouble:
    cmp %i4, 19  ! BASETYPE_Double
    bne tryobject
    nop
    ! All done, store (in two parts), return/restore (delay slot)
    st %f0, [%i2]
    st %f1, [%i2 + 4]
    ret
    restore

tryobject:
    cmp %i4, 32  ! DESCRIPTOR_ObjectType
    bne callcomplete
    nop
    ! All done, store/return/restore (delay slot)
    st %o0, [%i2]
    ret
    restore

callcomplete:
    ! All done, do the return/restore simultaneously (delay slot)
    ret
    restore
