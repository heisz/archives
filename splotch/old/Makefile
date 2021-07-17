#******************************************************************
#                          sPLOTch!
#
#  Makefile - makefile for construction and cleanup
# 
#*******************************************************************
#
#  These settings should be obvious.  Adjust them to match the 
#  appropriate command pathways for your machine.
#
MAKE=make
CC=gcc
INSTALL=cp
SHELL=/bin/sh
#
#  The following line should contain specific C compilation directives.
#  Usually, this is either -g (for testing), or -O (for optimization).
#
COPTS=
#
#  All binary executables will be installed in BINDIR, man pages in MANDIR,
#  and support files (fonts, for example) will be installed in SUPPORTDIR.
#
#  These are (by default) all keyed to MASTERDIR....this is probably the
#  only one that needs to be changed.
#
#  Note: executables can be moved later, but the support files must remain 
#        in the directory defined by SUPPORTDIR.
#
MASTERDIR=/users/home/jheisz/.local
BINDIR=$(MASTERDIR)/bin
MANDIR=$(MASTERDIR)/man/man1
SUPPORTDIR=$(MASTERDIR)/lib
#
#  Add specific entries to DRIVER_LIST from the following list.  See
#  the README.drivers file for more information on these drivers, as
#  well as other driver specific setup instructions.
#
#       dev.x - X Windows driver
#       dev.ps - PostScript driver
#       dev.vec - vector devices (currently Tek and HPGL)
#       dev.bm - bitmap/pixmap based formats
#       dev.tty - tty output test driver
#
DRIVER_LIST=dev.x dev.ps dev.vec dev.bm dev.tty 
#
#    If dev.x is selected, the following entries should indicate the
#  directories containing X11 libraries and inclusion files.  Note that
#  these include the -I and -L directives...if the directories are
#  available by default, delete the directives as well (ie. completely 
#  blank).
#
# X11LIBS=-L/usr/X11R5/lib
# X11INCLUDES=-I/usr/X11R5/include
X11LIBS=
X11INCLUDES=
#
#    PRINTER should be the system print spool controller, while
#  POSTSCRIPT is the name of the postscript printer.  It is important
#  that the PRINTER command be correct, as it is used by several
#  drivers.
#
#PRINTER=/usr/ucb/lpr
PRINTER=/usr/bin/lpr
POSTSCRIPT=sparc
#
###########################################################################
#  Nothing below here should need to be changed.
###########################################################################
#
X11OPTIONS=$(X11INCLUDES) $(X11LIBS)

all: drivers splotch

drivers::
	(cd drivers; $(MAKE) all 'DRIVERS=$(DRIVER_LIST)' 'COPTS=$(COPTS)' \
         'CC=$(CC)' 'MAKE=$(MAKE)' 'X11OPTS=$(X11OPTIONS)' \
         'PRINTER=$(PRINTER)' 'PSTSCPT=$(POSTSCRIPT)' )

splotch::
	(cd src; $(MAKE) splotch 'CC=$(CC)' 'COPTS=$(COPTS)' \
          'SUPPORTDIR=$(SUPPORTDIR)' )

install: drivers splotch
	if test ! -d $(SUPPORTDIR) ; \
	then mkdir $(SUPPORTDIR) ; \
	fi
	if test ! -d $(BINDIR) ; \
	then mkdir $(BINDIR) ; \
	fi
	(cd src; $(MAKE) install 'SUPPORTDIR=$(SUPPORTDIR)' \
           'BINDIR=$(BINDIR)' 'INSTALL=$(INSTALL)' )
	(cd drivers; $(MAKE) install 'BINDIR=$(BINDIR)'  \
           'INSTALL=$(INSTALL)' )

install_man: install.man

install.man::
	if test ! -d $(MANDIR) ; \
	then mkdir $(MANDIR) ; \
	fi
	(cd src; $(MAKE) install.man 'MANDIR=$(MANDIR)' \
            'INSTALL=$(INSTALL)' )
	(cd drivers; $(MAKE) install_man 'MANDIR=$(MANDIR)' \
            'INSTALL=$(INSTALL)' )

clean::
	(cd drivers; make clean)
	(cd src; make clean)
	(cd examples; touch clean.log; rm -f *.log *.sdvi *.ps)

dist::
	(cd drivers; make dist)
	(cd src; make dist)
	(cd examples; touch clean.log; rm -f *.log *.sdvi *.ps)
