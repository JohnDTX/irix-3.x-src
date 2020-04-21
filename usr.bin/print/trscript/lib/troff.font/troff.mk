#
#ident	"\%W\%"
#
# Notes:
# transcript/lib/troff.font/Makefile.sysv
#
# Copyright (C) 1985 Adobe Systems Incorporated
#
# Makefile for troff width tables and pscat correspondence tables
#
# See pscatmap(8) and the .map files for more information.
#
# If you add a font family to your PostScript/troff database,
# add it to the FAMILIES line in this makefile, then do a "make".

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
	cp ft* *.ct *.head /usr/lib/font/ps
	chown $$OWNER ft* *.ct *.head
	chgrp $$GROUP ft* *.ct *.head
	chmod 644 ft* *.ct *.head

clean:
	rm -f - *.ct *.head *.c *.o ft? ft?? *BAK *CKP .emacs_[0-9]*

clobber:	clean

FRC:

#
# Specific Target/Rules follow
#
install: default

.SUFFIXES: .ct .map

#######################################################################
# Here are the family names, the face names are gotten from the .map files

FAMILIES = Times.ct Helvetica.ct

#######################################################################

#install: ${FAMILIES}
#	-mkdir /usr/lib/font/ps
#	cp ft* *.ct *.head /usr/lib/font/ps
#	cd /usr/lib/font/ps ; chown $$OWNER . * ; chgrp $$GROUP . * ; \
#		chmod 755 . ; chmod 644 * ; 

.map.ct:
	../pscatmap $*.map
	awk -f head.awk $*.map >$*.head

