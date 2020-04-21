#
#ident	"\%W\%"
#
# Notes:
#
# transcript/Makefile.sysv
#
# Copyright 1985 (C) Adobe Systems Incorporated
#
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/RCS/trscript.mk,v 1.1 89/03/27 18:17:32 root Exp $
#
# to install transcript:
#
#	(Once for your system)
#		sysv
#		edit config & printer
#		make programs
#
#		(become super-user)
#		make install		( install files in system dirs )
#
#	(For each PostScript/TranScript printer)
#		cd etc
#		(become super-user)
#		mkprinter PRINTER TTY
#


#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#

#
# Local Definitions
#

#
# Targets/Rules
#

default:
	. ./config; cd src; make ${MFLAGS} -f src.mk
	. ./config; cd sh;  make ${MFLAGS} -f sh.mk
	. ./config; cd lib; make ${MFLAGS} -f lib.mk
# The official location for the man pages is now on olympus
#	. ./config; cd man; make ${MFLAGS} install

boot:
	. ./config; cd src; make -f src.mk boot

clean:
	rm -f *BAK *CKP .emacs_[0-9]*

cleanall clobber: clean
	cd etc; make ${MFLAGS} -f etc.mk clean
	cd lib; make ${MFLAGS} -f lib.mk clean
	cd man; make ${MFLAGS} -f man.mk clean
	cd  sh; make ${MFLAGS} -f sh.mk clean
	cd src; make ${MFLAGS} -f src.mk clean

FRC:

#
# Specific Target/Rules follow
#
install:
	. ./config; cd src; make ${MFLAGS} -f src.mk install
	. ./config; cd sh;  make ${MFLAGS} -f sh.mk install
	. ./config; cd lib; make ${MFLAGS} -f lib.mk install

explain:
	@cat doc/make.notes

programs:
	. ./config; cd lib; make ${MFLAGS} programs
	. ./config; cd man; make ${MFLAGS} programs
	. ./config; cd sh;  make ${MFLAGS} programs
	. ./config; cd src; make ${MFLAGS} programs

#install:
#	-. ./config; mv $$PSLIBDIR $$PSLIBDIR.old
#	-. ./config; mkdir $$PSLIBDIR; chown $$OWNER $$PSLIBDIR; \
#		chgrp $$GROUP $$PSLIBDIR; chmod 755 $$PSLIBDIR
#	. ./config; cd src; make ${MFLAGS} install
#	. ./config; cd sh;  make ${MFLAGS} install
#	. ./config; cd lib; make ${MFLAGS} install
# The official location for the man pages is now on olympus
#	. ./config; cd man; make ${MFLAGS} install

