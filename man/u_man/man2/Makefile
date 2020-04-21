# $Source: /d2/3.7/src/man/u_man/man2/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 16:53:14 $

M	= u
S	= 2

all clean clobber:

install:
	cp *.$S* $(DEST)/usr/man/$M_man/man$S
	cd $(DEST)/usr/man/$M_man/man$S;chmod 644 *;chgrp bin *;chown bin *

FRC:

clean:
	rm *.*.B* *.*.C* *.B* *.C*
