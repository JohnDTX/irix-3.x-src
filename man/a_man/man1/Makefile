# $Source: /d2/3.7/src/man/a_man/man1/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 16:19:51 $

M	= a
S	= 1

all clean clobber:

install:
	cp *.$S[a-z] $(DEST)/usr/man/$M_man/man$S
	cd $(DEST)/usr/man/$M_man/man$S;chmod 644 *;chgrp bin *;chown bin *

FRC:

clean:
	rm *.*.B* *.*.C* *.B* *.C*

