# AutoMakefile for compiling the java.lang.reflect classes.
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
noinst_LTLIBRARIES = libjemreflect.la

# Source files which are needed by the library generator
libjemreflect_la_SOURCES = array.c constructor.c exceptions.c field.c \
                           member.c method.c modifier.c reflect.c

# Include files associated with this distribution
INCLUDES = -I../../../../include -I../../include

# Removes the automake/autoconf generated files for ultra-clean source tree
MAINTAINERCLEANFILES = Makefile.in
