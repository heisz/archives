# Main (auto)makefile for the JEMCC-Mauve testsuite tree.
# Copyright (C) 1999-2004 J.M. Heisz 
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# See the file named COPYRIGHT in the root directory of the source
# distribution for specific references to the GNU General Public License, 
# as well as further clarification on your rights to use this software.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

CLASSES = wrdg/mauve/JEMCCTestHarness.class \
          wrdg/mauve/VMTestInterface.class \
          wrdg/mauve/TestTestlet.class

# Definition of our static JEMCC test program (all libraries included)
noinst_PROGRAMS = jemcc-static
jemcc_static_SOURCES = jemcc-static.c vmtestinterface.c
jemcc_static_LDADD = ../../src/engine/.libs/libjemccvm.a \
                     @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl
INCLUDES = -I../../include -I../../src/engine/include

# On top of the test VM instance, build the test components
all : $(CLASSES)

# Removes the automake/autoconf generated files for ultra-clean source tree
MAINTAINERCLEANFILES = Makefile.in

# Cleanup needs to remove all class files
CLEANFILES = *.class $(CLASSES)

# Ruleset to create the java class instances
SUFFIXES = .java .class

.java.class:
	$(JAVAC) -g -classpath .:${MAUVE_HOME} $<
