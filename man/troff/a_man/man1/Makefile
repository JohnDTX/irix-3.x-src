# $Source: 
# @(#)$Revision:
# $Date:

M	= a
S	= 1

all clean clobber:

install:
	cp *.$S* $(DEST)/../print/usr/man/$M_man/man$S
	cd $(DEST)/../print/usr/man/$M_man/man$S; \
			     chmod 644 *;chgrp bin *;chown bin *
