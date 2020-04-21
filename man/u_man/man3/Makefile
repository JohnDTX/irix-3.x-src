# $Source: /d2/3.7/src/man/u_man/man3/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 16:57:27 $

M	= u
S	= 3
all clean clobber:

install:
	cp *.$S *.$S? $(DEST)/usr/man/$M_man/man$S/.
	cd $(DEST)/usr/man/$M_man/man$S;chmod 644 *.$S *.$S?
	cd $(DEST)/usr/man/$M_man/man$S;chown bin *.$S *.$S?
	cd $(DEST)/usr/man/$M_man/man$S;chgrp bin *.$S *.$S?
#
	cd $(GLHDWR) ; make $@

FRC:














