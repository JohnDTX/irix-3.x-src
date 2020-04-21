# transcript/lib/Makefile.sysv
#
# Copyright (c) 1985 Adobe Systems Incorporated. All Rights Reserved. 
#
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/lib/RCS/Makefile,v 1.1 89/03/27 18:18:57 root Exp $

FILES = banner.pro enscript.pro pstext.pro \
	ps4014.pro ps630.pro ps4014.pro pscat.pro psplot.pro psdit.pro \
	ehandler.ps uartpatch.ps bogusmsg.ps \
	font.map

programs all:

install: 
	cd $$PSLIBDEST; rm -rf - ${FILES} *.afm
	cp ${FILES} *.afm $$PSLIBDEST
	cd $$PSLIBDEST; chown $$OWNER ${FILES} *.afm; \
		chgrp $$GROUP ${FILES} *.afm; chmod 644 ${FILES} *.afm
	cd ditroff.font; make ${MFLAGS} install

clean:
	rm -f - *.BAK *.CKP .emacs_[0-9]*
	cd troff.font; make ${MFLAGS} clean
	cd ditroff.font; make ${MFLAGS} clean
