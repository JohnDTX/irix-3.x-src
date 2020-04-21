# $Source: /d2/3.7/src/man/u_man/man1/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 16:43:54 $

M	= u
S	= 1

all clean clobber:

install:
	cp *.$S *.$S[a-z] $(DEST)/usr/man/$M_man/man$S
	cd $(DEST)/usr/man/$M_man/man$S;chmod 644 *;chgrp bin *;chown bin *
#
	cd $(GLHDWR) ; make $@

clean:
	rm *.*.C* *.*.B* *.B* *.C*
