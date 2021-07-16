#
# Main (auto)makefile for the JEMCC-Indigo testsuite tree.
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

# Construction list for the Indigo test scenarios
CLASSES = wrdg/indigo/parser/Descriptor.class \
          wrdg/indigo/parser/ClassDefinition.class \
          wrdg/indigo/parser/ClassAttributes.class \
          wrdg/indigo/parser/ConstantPool.class \
          wrdg/indigo/parser/TruncatedConstantPool.class \
          wrdg/indigo/parser/Interfaces.class \
          wrdg/indigo/parser/Fields.class \
          wrdg/indigo/parser/FieldAttributes.class \
          wrdg/indigo/parser/Methods.class \
          wrdg/indigo/parser/MethodAttributes.class \
          wrdg/indigo/parser/MethodCode.class \
          wrdg/indigo/linker/Hierarchies.class \
          wrdg/indigo/linker/Circularity.class \
          wrdg/indigo/linker/Inheritance.class \
          wrdg/indigo/runtime/Initialization.class \
          wrdg/indigo/runtime/Cloning.class

# Definition of our static JEMCC test program (all libraries included)
noinst_PROGRAMS = jemcc-static
jemcc_static_SOURCES = jemcc-static.c
jemcc_static_LDADD = ../../src/engine/.libs/libjemccvm.a \
                     @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl
INCLUDES = -I../../include -I../../src/engine/include

# On top of the test VM instance, build the test components
all : $(CLASSES)

# Run the check sequences
TESTVM = ./jemcc-static
check:
	$(TESTVM) wrdg.indigo.parser.Descriptor
	$(TESTVM) wrdg.indigo.parser.ClassDefinition
	$(TESTVM) wrdg.indigo.parser.ClassAttributes
	$(TESTVM) wrdg.indigo.parser.ConstantPool
	$(TESTVM) wrdg.indigo.parser.TruncatedConstantPool
	$(TESTVM) wrdg.indigo.parser.Interfaces
	$(TESTVM) wrdg.indigo.parser.Fields
	$(TESTVM) wrdg.indigo.parser.FieldAttributes
	$(TESTVM) wrdg.indigo.parser.Methods
	$(TESTVM) wrdg.indigo.parser.MethodAttributes
	$(TESTVM) wrdg.indigo.parser.MethodCode
	$(TESTVM) wrdg.indigo.linker.Hierarchies
	$(TESTVM) wrdg.indigo.linker.Circularity
	$(TESTVM) wrdg.indigo.linker.Inheritance
	$(TESTVM) wrdg.indigo.runtime.Initialization
javacheck:
	$(JAVA) wrdg.indigo.parser.Descriptor
	$(JAVA) wrdg.indigo.parser.ClassDefinition
	$(JAVA) wrdg.indigo.parser.ClassAttributes
	$(JAVA) wrdg.indigo.parser.ConstantPool
	$(JAVA) wrdg.indigo.parser.TruncatedConstantPool
	$(JAVA) wrdg.indigo.parser.Interfaces -sun
	$(JAVA) wrdg.indigo.parser.Fields
	$(JAVA) wrdg.indigo.parser.FieldAttributes
	$(JAVA) wrdg.indigo.parser.Methods
	$(JAVA) wrdg.indigo.parser.MethodAttributes
	$(JAVA) wrdg.indigo.parser.MethodCode
	$(JAVA) wrdg.indigo.linker.Hierarchies
	$(JAVA) wrdg.indigo.linker.Circularity
	$(JAVA) wrdg.indigo.linker.Inheritance
	$(JAVA) wrdg.indigo.runtime.Initialization

# Removes the automake/autoconf generated files for ultra-clean source tree
MAINTAINERCLEANFILES = Makefile.in

# Cleanup needs to remove all class files
CLEANFILES = *.class $(CLASSES)

# Ruleset to create the java class instances
SUFFIXES = .java .class

.java.class:
	$(JAVAC) -g $<
