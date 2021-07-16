# Automake file for the internal testuite programs/libraries.
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

# List of programs to be built as part of the testsuite
noinst_PROGRAMS = zipfile zipnommap utility descriptor classparser thrmon \
                  ffi pathload jemcc package classlinker vlinktbl classmgmt \
                  string cpu

# Dynamically linked elements of test programs
noinst_LTLIBRARIES = libpkg.la

# Special compile to ensure testing library construction
all: libffitest.la libpkgtesta.la libpkgtestb.la libpkgtestc.la

# Things to do as part of the testsuite check operations
check: libffitest.la libpkgtesta.la libpkgtestb.la libpkgtestc.la
	./zipfile
	./zipnommap
	./utility
	./descriptor
	./classparser
	./thrmon
	./ffi
	./pathload
	./jemcc
	./package
	./classlinker
	./vlinktbl
	./classmgmt
	./string
	./cpu

# Include files associated with this distribution
INCLUDES = -I../../include -I ../../src/engine/include

# Removes the automake/autoconf generated files for ultra-clean source tree
MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *.la

# Workaround for fact that libtool cannot make shared libraries (.so) without
# installing them as well (do compile automatically, link manually)
libffitest.la: ffitest.lo
	$(LIBTOOL) --mode=link $(COMPILE) -o libffitest.la -rpath /tmp/jemtest ffitest.lo
libpkgtesta.la: pkgtesta.lo
	$(LIBTOOL) --mode=link $(COMPILE) -o libpkgtesta.la -rpath /tmp/jemtest pkgtesta.lo -lpkg
libpkgtestb.la: pkgtestb.lo
	$(LIBTOOL) --mode=link $(COMPILE) -o libpkgtestb.la -rpath /tmp/jemtest pkgtestb.lo -lpkg
libpkgtestc.la: pkgtestc.lo
	$(LIBTOOL) --mode=link $(COMPILE) -o libpkgtestc.la -rpath /tmp/jemtest pkgtestc.lo -lpkg

# Build rules for creating the rational instrumented test programs
purify: zipfile-purify zipnommap-purify utility-purify \
        descriptor-purify classparser-purify thrmon-purify \
        ffi-purify pathload-purify jemcc-purify package-purify \
        classlinker-purify vlinktbl-purify classmgmt-purify \
        string-purify cpu-purify
quantify: zipfile-quantify zipnommap-quantify utility-quantify \
          descriptor-quantify classparser-quantify thrmon-quantify \
          ffi-quantify pathload-quantify jemcc-quantify package-quantify \
          classlinker-quantify vlinktbl-quantify classmgmt-quantify \
          string-quantify cpu-quantify
purecov: zipfile-purecov zipnommap-purecov utility-purecov \
         descriptor-purecov classparser-purecov thrmon-purecov \
         ffi-purecov pathload-purecov jemcc-purecov package-purecov \
         classlinker-purecov vlinktbl-purecov classmgmt-purecov \
         string-purecov cpu-purecov

# Definitions for the zipfile (interface) test programs
ZIPOBJ = ../../src/engine/zlib/adler32.o \
         ../../src/engine/zlib/crc32.o \
         ../../src/engine/zlib/inflate.o \
         ../../src/engine/zlib/infblock.o \
         ../../src/engine/zlib/inftrees.o \
         ../../src/engine/zlib/infcodes.o \
         ../../src/engine/zlib/infutil.o \
         ../../src/engine/zlib/inffast.o \
         ../../src/engine/zlib/zutil.o 

zipfile_SOURCES = zipfile.c
zipfile_LDADD = ../../src/engine/sysenv/zipfile.o \
                ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                @EFENCE_LIB@ -lm -ldl

zipnommap_SOURCES = zipfile.c
zipnommap_LDADD = ../../src/engine/sysenv/zipfile-nommap.o \
                  ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                  @EFENCE_LIB@ -lm -ldl

zipfile-purecov:
	purecov gcc -g -o ../../../../rational/zipfile-purecov \
                    zipfile.o ../../src/engine/sysenv/zipfile.o  \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    -lm -ldl
zipnommap-purecov:
	purecov gcc -g -o ../../../../rational/zipnommap-purecov \
                    zipfile.o ../../src/engine/sysenv/zipfile-nommap.o  \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    -lm -ldl

zipfile-quantify:
	quantify gcc -g -o ../../../../rational/zipfile-quantify \
                    zipfile.o ../../src/engine/sysenv/zipfile.o  \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    -lm -ldl
zipnommap-quantify:
	quantify gcc -g -o ../../../../rational/zipnommap-quantify \
                    zipfile.o ../../src/engine/sysenv/zipfile-nommap.o  \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    -lm -ldl

zipfile-purify:
	purify gcc -g -o ../../../../rational/zipfile-purify \
                    zipfile.o ../../src/engine/sysenv/zipfile.o  \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    -lm -ldl
zipnommap-purify:
	purify gcc -g -o ../../../../rational/zipnommap-purify \
                    zipfile.o ../../src/engine/sysenv/zipfile-nommap.o  \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    -lm -ldl

# Definitions for the utility routines test program
utility_SOURCES = utility.c
utility_LDADD = ../../src/engine/core/hash.o \
                ../../src/engine/core/sundry.o @EFENCE_LIB@ -lm 

utility-purecov:
	purecov gcc -g -o ../../../../rational/utility-purecov \
                    utility.o ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o -lm

utility-quantify:
	quantify gcc -g -o ../../../../rational/utility-quantify \
                    utility.o ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o -lm

utility-purify:
	purify gcc -g -o ../../../../rational/utility-purify \
                    utility.o ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o -lm

# Definitions for the descriptor management test program
descriptor_SOURCES = descriptor.c
descriptor_LDADD = ../../src/engine/core/classparser.o \
                   ../../src/engine/core/sundry.o \
                   @EFENCE_LIB@ -lm -ldl

descriptor-purecov:
	purecov gcc -g -o ../../../../rational/descriptor-purecov \
                    descriptor.o ../../src/engine/core/classparser.o \
                    ../../src/engine/core/sundry.o -lm

descriptor-quantify:
	quantify gcc -g -o ../../../../rational/descriptor-quantify \
                    descriptor.o ../../src/engine/core/classparser.o \
                    ../../src/engine/core/sundry.o -lm

descriptor-purify:
	purify gcc -g -o ../../../../rational/descriptor-purify \
                    descriptor.o ../../src/engine/core/classparser.o \
                    ../../src/engine/core/sundry.o -lm

# Definitions for the classparsing test program
classparser_SOURCES = classparser.c
classparser_LDADD = ../../src/engine/core/classparser.o \
                   ../../src/engine/core/sundry.o \
                   @EFENCE_LIB@ -lm -ldl

classparser-purecov:
	purecov gcc -g -o ../../../../rational/classparser-purecov \
                    classparser.o ../../src/engine/core/classparser.o \
                    ../../src/engine/core/sundry.o -lm

classparser-quantify:
	quantify gcc -g -o ../../../../rational/classparser-quantify \
                    classparser.o ../../src/engine/core/classparser.o \
                    ../../src/engine/core/sundry.o -lm

classparser-purify:
	purify gcc -g -o ../../../../rational/classparser-purify \
                    classparser.o ../../src/engine/core/classparser.o \
                    ../../src/engine/core/sundry.o -lm

# Definitions for the thread/monitor test program
thrmon_SOURCES = thrmon.c
thrmon_LDADD = ../../src/engine/sysenv/thread.o \
               ../../src/engine/sysenv/sysmonitor.o \
               ../../src/engine/sysenv/objmonitor.o \
               @THREAD_LIB@ @EFENCE_LIB@ -lm

thrmon-purecov:
	purecov gcc -g -o ../../../../rational/thrmon-purecov \
                    thrmon.o ../../src/engine/sysenv/thread.o  \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    @THREAD_LIB@ -lposix4 -lm

thrmon-quantify:
	quantify gcc -g -o ../../../../rational/thrmon-quantify \
                    thrmon.o ../../src/engine/sysenv/thread.o  \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    @THREAD_LIB@ -lposix4 -lm

thrmon-purify:
	purify gcc -g -o ../../../../rational/thrmon-purify \
                    thrmon.o ../../src/engine/sysenv/thread.o  \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    @THREAD_LIB@ -lposix4 -lm

# Definitions for the dynamic library and foreign function interfaces
ffi_SOURCES = ffi.c
ffi_LDADD = ../../src/engine/sysenv/dynalib.o \
            ../../src/engine/sysenv/ffi.o \
            ../../src/engine/sysenv/fficall.o \
            @EFENCE_LIB@ -lm -ldl

ffi-purecov:
	purecov gcc -g -o ../../../../rational/ffi-purecov \
                    ffi.o ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/sysenv/ffi.o \
                    ../../src/engine/sysenv/fficall.o \
                    -lm -ldl

ffi-quantify:
	quantify gcc -g -o ../../../../rational/ffi-quantify \
                    ffi.o ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/sysenv/ffi.o \
                    ../../src/engine/sysenv/fficall.o \
                    -lm -ldl

ffi-purify:
	purify gcc -g -o ../../../../rational/ffi-purify \
                    ffi.o ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/sysenv/ffi.o \
                    ../../src/engine/sysenv/fficall.o \
                    -lm -ldl

# Definitions for the path parsing and content loading tests
pathload_SOURCES = pathload.c
pathload_LDADD = ../../src/engine/core/paths.o \
                 ../../src/engine/sysenv/zipfile.o \
                 ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                 ../../src/engine/sysenv/dynalib.o \
                 ../../src/engine/core/sundry.o \
                 @EFENCE_LIB@ -lm -ldl

pathload-purecov:
	purecov gcc -g -o ../../../../rational/pathload-purecov \
                    pathload.o ../../src/engine/core/paths.o \
                    ../../src/engine/sysenv/zipfile.o \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/core/sundry.o \
                    -lm -ldl

pathload-quantify:
	quantify gcc -g -o ../../../../rational/pathload-quantify \
                    pathload.o ../../src/engine/core/paths.o \
                    ../../src/engine/sysenv/zipfile.o \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/core/sundry.o \
                    -lm -ldl

pathload-purify:
	purify gcc -g -o ../../../../rational/pathload-purify \
                    pathload.o ../../src/engine/core/paths.o \
                    ../../src/engine/sysenv/zipfile.o \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/core/sundry.o \
                    -lm -ldl

# Definitions for the JEM compiled class mechanisms
jemcc_SOURCES = uvminit.c jemcc.c
jemcc_LDADD = ../../src/engine/core/paths.o \
              ../../src/engine/core/classparser.o \
              ../../src/engine/core/class.o \
              ../../src/engine/core/jemcc.o \
              ../../src/engine/core/hash.o \
              ../../src/engine/core/sundry.o \
              ../../src/engine/sysenv/dynalib.o \
              ../../src/engine/sysenv/thread.o \
              ../../src/engine/sysenv/sysmonitor.o \
              ../../src/engine/sysenv/objmonitor.o \
              ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
              ../../src/engine/sysenv/file.o \
              @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl

jemcc-purecov:
	purecov gcc -g -o ../../../../rational/jemcc-purecov \
                    jemcc.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/sysenv/zipfile.o \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/thread.o  \
                    ../../src/engine/sysenv/sysmonitor.o  \
                    ../../src/engine/sysenv/objmonitor.o  \
                    ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/core/sundry.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

jemcc-quantify:
	quantify gcc -g -o ../../../../rational/jemcc-quantify \
                    jemcc.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/sysenv/zipfile.o \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/thread.o  \
                    ../../src/engine/sysenv/sysmonitor.o  \
                    ../../src/engine/sysenv/objmonitor.o  \
                    ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/core/sundry.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

jemcc-purify:
	purify gcc -g -o ../../../../rational/jemcc-purify \
                    jemcc.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/sysenv/zipfile.o \
                    ../../src/engine/sysenv/file.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/thread.o  \
                    ../../src/engine/sysenv/sysmonitor.o  \
                    ../../src/engine/sysenv/objmonitor.o  \
                    ../../src/engine/sysenv/dynalib.o  \
                    ../../src/engine/core/sundry.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

# Definitions for the JEMCC packaging tests
libpkg_la_SOURCES = libpkg.c
libpkg_la_LIBADD = ../../src/engine/core/paths.lo \
                   ../../src/engine/core/classparser.lo \
                   ../../src/engine/core/class.lo \
                   ../../src/engine/core/jemcc.lo \
                   ../../src/engine/core/hash.lo \
                   ../../src/engine/core/sundry.lo \
                   ../../src/engine/sysenv/dynalib.lo \
                   ../../src/engine/sysenv/thread.lo \
                   ../../src/engine/sysenv/sysmonitor.lo \
                   ../../src/engine/sysenv/objmonitor.lo \
                   ../../src/engine/sysenv/zipfile.lo \
                   ../../src/engine/sysenv/file.lo \
                   ../../src/engine/zlib/adler32.lo \
                   ../../src/engine/zlib/crc32.lo \
                   ../../src/engine/zlib/deflate.lo \
                   ../../src/engine/zlib/trees.lo \
                   ../../src/engine/zlib/zutil.lo \
                   ../../src/engine/zlib/inflate.lo \
                   ../../src/engine/zlib/infblock.lo \
                   ../../src/engine/zlib/inftrees.lo \
                   ../../src/engine/zlib/infcodes.lo \
                   ../../src/engine/zlib/infutil.lo \
                   ../../src/engine/zlib/inffast.lo

package_SOURCES = uvminit.c package.c
package_LDADD = libpkg.la @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl

package-purecov:
	libtool --mode=link purecov \
            gcc -g -o ../../../../rational/package-purecov \
                    package.o uvminit.o \
                    -lpkg @THREAD_LIB@ -lposix4 -lm -ldl

package-quantify:
	libtool --mode=link quantify \
            gcc -g -o ../../../../rational/package-quantify \
                    package.o uvminit.o \
                    -lpkg @THREAD_LIB@ -lposix4 -lm -ldl

package-purify:
	libtool --mode=link purify \
            gcc -g -o ../../../../rational/package-purify \
                    package.o uvminit.o \
                    -lpkg @THREAD_LIB@ -lposix4 -lm -ldl

# Definitions for the bytecode linking test cases
classlinker_SOURCES = uvminit.c classlinker.c
classlinker_LDADD = ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/classlinker.o \
                    ../../src/engine/core/classverifier.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o \
                    ../../src/engine/sysenv/dynalib.o \
                    ../../src/engine/sysenv/thread.o \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/file.o \
                    @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl

classlinker-purecov:
	purecov gcc -g -o ../../../../rational/classlinker-purecov \
                    classlinker.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/classlinker.o \
                    ../../src/engine/core/classverifier.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o \
                    ../../src/engine/sysenv/dynalib.o \
                    ../../src/engine/sysenv/thread.o \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/file.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

classlinker-quantify:
	quantify gcc -g -o ../../../../rational/classlinker-quantify \
                    classlinker.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/classlinker.o \
                    ../../src/engine/core/classverifier.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o \
                    ../../src/engine/sysenv/dynalib.o \
                    ../../src/engine/sysenv/thread.o \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/file.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

classlinker-purify:
	purify gcc -g -o ../../../../rational/classlinker-purify \
                    classlinker.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/classlinker.o \
                    ../../src/engine/core/classverifier.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o \
                    ../../src/engine/sysenv/dynalib.o \
                    ../../src/engine/sysenv/thread.o \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/file.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

# Definitions for the bytecode hierarchical linkage test cases
vlinktbl_SOURCES = uvminit.c vlinktbl.c
vlinktbl_LDADD = ../../src/engine/core/paths.o \
                 ../../src/engine/core/classparser.o \
                 ../../src/engine/core/classlinker.o \
                 ../../src/engine/core/classverifier.o \
                 ../../src/engine/core/class.o \
                 ../../src/engine/core/jemcc.o \
                 ../../src/engine/core/hash.o \
                 ../../src/engine/core/sundry.o \
                 ../../src/engine/sysenv/dynalib.o \
                 ../../src/engine/sysenv/thread.o \
                 ../../src/engine/sysenv/sysmonitor.o \
                 ../../src/engine/sysenv/objmonitor.o \
                 ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                 ../../src/engine/sysenv/file.o \
                 @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl

vlinktbl-purecov:
	purecov gcc -g -o ../../../../rational/vlinktbl-purecov \
                    vlinktbl.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/classlinker.o \
                    ../../src/engine/core/classverifier.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o \
                    ../../src/engine/sysenv/dynalib.o \
                    ../../src/engine/sysenv/thread.o \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/file.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

vlinktbl-quantify:
	quantify gcc -g -o ../../../../rational/vlinktbl-quantify \
                    vlinktbl.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/classlinker.o \
                    ../../src/engine/core/classverifier.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o \
                    ../../src/engine/sysenv/dynalib.o \
                    ../../src/engine/sysenv/thread.o \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/file.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

vlinktbl-purify:
	purify gcc -g -o ../../../../rational/vlinktbl-purify \
                    vlinktbl.o uvminit.o ../../src/engine/core/paths.o \
                    ../../src/engine/core/classparser.o \
                    ../../src/engine/core/classlinker.o \
                    ../../src/engine/core/classverifier.o \
                    ../../src/engine/core/class.o \
                    ../../src/engine/core/jemcc.o \
                    ../../src/engine/core/hash.o \
                    ../../src/engine/core/sundry.o \
                    ../../src/engine/sysenv/dynalib.o \
                    ../../src/engine/sysenv/thread.o \
                    ../../src/engine/sysenv/sysmonitor.o \
                    ../../src/engine/sysenv/objmonitor.o \
                    ../../src/engine/sysenv/zipfile.o $(ZIPOBJ) \
                    ../../src/engine/sysenv/file.o \
                    @THREAD_LIB@ -lposix4 -lm -ldl

# Definitions for the class management test program
JEMCCOBJ = ../../src/engine/core/paths.o \
           ../../src/engine/core/classlinker.o \
           ../../src/engine/core/classparser.o \
           ../../src/engine/core/classverifier.o \
           ../../src/engine/core/vmclass.o \
           ../../src/engine/core/jemcc.o \
           ../../src/engine/core/hash.o \
           ../../src/engine/core/sundry.o \
           ../../src/engine/core/cpu.o \
           ../../src/engine/core/string.o \
           ../../src/engine/core/exception.o \
           ../../src/engine/core/memgc-inttst.o \
           ../../src/engine/core/numerics.o \
           ../../src/engine/jni/array.o \
           ../../src/engine/jni/object.o \
           ../../src/engine/jni/class.o \
           ../../src/engine/classes/init.o \
           ../../src/engine/classes/object.o \
           ../../src/engine/classes/class.o \
           ../../src/engine/classes/array.o \
           ../../src/engine/classes/runnable.o \
           ../../src/engine/classes/classloader.o \
           ../../src/engine/classes/string.o \
           ../../src/engine/classes/sysclassloader.o \
           ../../src/engine/classes/throwable.o \
           ../../src/engine/classes/exceptions.o \
           ../../src/engine/classes/reflect/array.o \
           ../../src/engine/classes/reflect/constructor.o \
           ../../src/engine/classes/reflect/exceptions.o \
           ../../src/engine/classes/reflect/field.o \
           ../../src/engine/classes/reflect/member.o \
           ../../src/engine/classes/reflect/method.o \
           ../../src/engine/classes/reflect/modifier.o \
           ../../src/engine/classes/reflect/reflect.o \
           ../../src/engine/sysenv/dynalib.o \
           ../../src/engine/sysenv/thread.o \
           ../../src/engine/sysenv/sysmonitor.o \
           ../../src/engine/sysenv/objmonitor.o \
           ../../src/engine/sysenv/zipfile.o \
           ../../src/engine/sysenv/file.o
classmgmt_SOURCES = uvminit.c classmgmt.c
classmgmt_LDADD = $(JEMCCOBJ) $(ZIPOBJ) \
                  @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl

classmgmt-purecov:
	purecov gcc -g -o ../../../../rational/classmgmt-purecov \
                    classmgmt.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

classmgmt-quantify:
	quantify gcc -g -o ../../../../rational/classmgmt-quantify \
                    classmgmt.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

classmgmt-purify:
	purify gcc -g -o ../../../../rational/classmgmt-purify \
                    classmgmt.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

# Definitions for the character/string support test cases
string_SOURCES = string.c uvminit.c
string_LDADD = $(JEMCCOBJ) $(ZIPOBJ) \
               @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl

string-purecov:
	purecov gcc -g -o ../../../../rational/string-purecov \
                    string.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

string-quantify:
	quantify gcc -g -o ../../../../rational/string-quantify \
                    string.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

string-purify:
	purify gcc -g -o ../../../../rational/string-purify \
                    string.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

# Definitions for the bytecode interpreter test cases
cpu_SOURCES = cpu.c uvminit.c
cpu_LDADD = $(JEMCCOBJ) $(ZIPOBJ) \
            @THREAD_LIB@ @EFENCE_LIB@ -lm -ldl

cpu-purecov:
	purecov gcc -g -o ../../../../rational/cpu-purecov \
                    cpu.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

cpu-quantify:
	quantify gcc -g -o ../../../../rational/cpu-quantify \
                    cpu.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl

cpu-purify:
	purify gcc -g -o ../../../../rational/cpu-purify \
                    cpu.o uvminit.o $(JEMCCOBJ) $(ZIPOBJ) \
                    @THREAD_LIB@ -lposix4 -lm -ldl
