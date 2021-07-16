# AutoMakefile for compiling the 'core' engine functionality.
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

# Use automake/libtool to generate a local library instance
# (NOTE: this may be corrected in AutoMake Version > 1.4)
noinst_LTLIBRARIES = libjemcore.la

# Source files which are needed by the library generator
libjemcore_la_SOURCES = hash.c sundry.c classparser.c paths.c \
                        class.c jemcc.c vmclass.c classlinker.c \
                        classverifier.c string.c cpu.c exception.c \
                        memgc.c numerics.c

# Special compile for the internal test cases
all: memgc-inttst.o

# Include files associated with this distribution
INCLUDES = -I../../../include -I../include

# Removes the automake/autoconf generated files for ultra-clean source tree
MAINTAINERCLEANFILES = Makefile.in

# Rule for the internal test case compilation
memgc-inttst.o: memgc.c
	$(COMPILE) -o memgc-inttst.o -c -DTEST_INTERNAL=1 $<
