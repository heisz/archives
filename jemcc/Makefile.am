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
SUBDIRS = src tools @TESTSUITE_DIR@

# Generate the aggregated jemcc definitions from the engine internals.  Not
# run automatically - used to generate the jemcc.h file for check-in.
jemcc.h:
	rm -f include/jemcc.h
	cat src/engine/include/jemcc.h \
	    src/engine/include/jnifunc.h \
	    src/engine/include/sysenv.h \
	    src/engine/include/jem.h \
	    src/engine/include/cpu.h \
	    src/engine/include/class.h \
	    src/engine/include/jvmenv.h \
	    src/engine/include/vmclasses.h | \
	    awk -f compjemcc.awk > include/jemcc.h

# Removes the automake/autoconf generated files for ultra-clean source tree
MAINTAINERCLEANFILES = config.cache config.log Makefile.in \
                       aclocal.m4 configure libtool include/jnitypes.h
