# Main (auto)makefile for the JEMCC development tree.
# Copyright (C) 1999-2004 J.M. Heisz 
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# See the file named COPYRIGHT in the root directory of the source
# distribution for specific references to the GNU Lesser General Public
# License, as well as further clarification on your rights to use this
# software.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# Just one step in the path to the source packages
SUBDIRS = zlib sysenv core classes jni

# NOTE: AutoMake Version 1.4 does not support clean subdirectory makes.
# Instead, we make sub-libraries and then merge them at this level
lib_LTLIBRARIES = libjemccvm.la

# Need at least one local source file to generate the library
libjemccvm_la_SOURCES = jemccvm.c

# Subdirectory compiles to be included in this directory
libjemccvm_la_LIBADD = core/classlinker.lo core/classparser.lo \
                       core/classverifier.lo core/cpu.lo core/exception.lo \
                       core/hash.lo core/jemcc.lo core/memgc.lo \
                       core/numerics.lo core/paths.lo core/string.lo \
                       core/sundry.lo core/vmclass.lo \
                       classes/array.lo classes/class.lo classes/init.lo \
                       classes/object.lo classes/runnable.lo \
                       classes/classloader.lo classes/string.lo \
                       classes/sysclassloader.lo classes/throwable.lo \
                       classes/exceptions.lo \
                       classes/reflect/array.lo classes/reflect/constructor.lo \
                       classes/reflect/exceptions.lo classes/reflect/field.lo \
                       classes/reflect/member.lo classes/reflect/method.lo \
                       classes/reflect/modifier.lo classes/reflect/reflect.lo \
                       jni/array.lo jni/class.lo \
                       jni/exception.lo jni/field.lo jni/invoke.lo \
                       jni/machine.lo jni/method.lo jni/object.lo \
                       jni/stdargfn.lo jni/string.lo \
                       sysenv/dynalib.lo sysenv/fficall.lo sysenv/ffi.lo \
                       sysenv/file.lo sysenv/objmonitor.lo \
                       sysenv/sysmonitor.lo sysenv/thread.lo sysenv/zipfile.lo \
                       zlib/adler32.lo zlib/crc32.lo zlib/deflate.lo \
                       zlib/trees.lo zlib/zutil.lo zlib/inflate.lo \
                       zlib/infblock.lo zlib/inftrees.lo zlib/infcodes.lo \
                       zlib/infutil.lo zlib/inffast.lo 

# Removes the automake/autoconf generated files for ultra-clean source tree
MAINTAINERCLEANFILES = Makefile.in
