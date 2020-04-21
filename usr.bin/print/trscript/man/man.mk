#
#ident	"\%W\%"
#
# Notes:
# transcript/man/Makefile
#
# Copyright 1985 Adobe Systems Incorporated
#
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/man/RCS/man.mk,v 1.1 89/03/27 18:20:07 root Exp $
# RCSLOG:
# Revision 1.2  86/11/06  11:54:30  paulm
# Change manual section numbers from BSD-flavored to SYSV-flavored.
# 
# Revision 1.1  86/08/11  10:26:33  paulm
# Initial revision
# 
# Revision 2.1  85/11/24  12:38:23  shore
# Product Release 2.0
# 
# 
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

clean:
	rm -f - ${PAGES} *.BAK *.CKP .emacs_[0-9]*

clobber:	clean

FRC:

#
# Specific Target/Rules follow
#

install: default

.SUFFIXES: .1 .1p .4 .4p .5 .5p

PAGES1 = enscript.1 ps4014.1 ps630.1 psdit.1 psplot.1 psrev.1 \
	psroff.1 ptroff.1  pscat.1 
PAGES4 = afm.4 postscript.4
PAGES5 = transcript.5 #pscatmap.5 

PAGES = ${PAGES1} ${PAGES4} ${PAGES5}

programs all: ${PAGES}

.1p.1 .4p.4 .5p.5:
	sed	-e s,XPSLIBDIRX,$$PSLIBDIR,g \
		-e s,XTROFFFONTDIRX,$$TROFFFONTDIR,g \
		-e s,XPSTEMPDIR,$$PSTEMPDIR,g \
		-e s,XDITDIRX,$$DITDIR,g \
		$? > $@

#install: ${PAGES}
#	cd ${MANDIR}; rm -f ${PAGES}
#	./installman 1 $$MAN1
#	./installman 4 $$MAN4
#	./installman 5 $$MAN5

