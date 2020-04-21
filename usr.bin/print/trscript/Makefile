# transcript/Makefile.sysv
#
# Copyright 1985 (C) Adobe Systems Incorporated
#
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/RCS/Makefile,v 1.1 89/03/27 18:17:21 root Exp $
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

explain:
	@cat doc/make.notes

programs:
	. ./config; cd lib; make ${MFLAGS} programs
	. ./config; cd man; make ${MFLAGS} programs
	. ./config; cd sh;  make ${MFLAGS} programs
	. ./config; cd src; make ${MFLAGS} programs

install:
	-. ./config; mv $$PSLIBDIR $$PSLIBDIR.old
	-. ./config; mkdir $$PSLIBDIR; chown $$OWNER $$PSLIBDIR; \
		chgrp $$GROUP $$PSLIBDIR; chmod 755 $$PSLIBDIR
	. ./config; cd src; make ${MFLAGS} install
	. ./config; cd sh;  make ${MFLAGS} install
	. ./config; cd lib; make ${MFLAGS} install
# The official location for the man pages is now on olympus
#	. ./config; cd man; make ${MFLAGS} install

clean:
	rm -f *BAK *CKP .emacs_[0-9]*

cleanall clobber: clean
	cd etc; make ${MFLAGS} clean
	cd lib; make ${MFLAGS} clean
	cd man; make ${MFLAGS} clean
	cd  sh; make ${MFLAGS} clean
	cd src; make ${MFLAGS} clean
