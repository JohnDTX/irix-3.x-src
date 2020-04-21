# transcript/sh/Makefile
#
# Copyright 1985 Adobe Systems Incorporated
#
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/sh/RCS/Makefile,v 1.1 89/03/27 18:20:23 root Exp $
# RCSLOG:
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  18:20:23  root
# Initial check-in for 3.7
# 
# Revision 1.3  87/05/04  00:33:38  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.1  86/10/22  10:03:22  glen
# Initial revision
# 
# Revision 2.1  85/11/24  12:31:56  shore
# Product Release 2.0
# 
#

programs all: ptroff psroff

ptroff: ptroff.sh
	sed	-e s,TROFFFONTDIR,$$TROFFFONTDIR,g \
		ptroff.sh >ptroff

psroff: psroff.sh
	cp psroff.sh psroff

install: psroff ptroff
#	cp ptroff psroff $$BINDIR
	cp psroff $$BINDIR
	chown $$OWNER $$BINDIR/ptroff $$BINDIR/psroff
	chgrp $$GROUP $$BINDIR/ptroff $$BINDIR/psroff
	chmod 755 $$BINDIR/ptroff $$BINDIR/psroff

clean:
	-rm -f - ptroff psroff *.BAK *.CKP .emacs_[0-9]*
