#
# Notes:
# transcript/etc/Makefile
#
# Copyright 1985 Adobe Systems Incorporated
#
# $Header: /d2/3.7/src/usr.bin/print/trscript/etc/RCS/etc.mk,v 1.1 89/03/27 18:18:23 root Exp $
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

clobber:	clean


clean:
	rm -f - *CKP *BAK .emacs_[0-9]*

FRC:

#
# Specific Target/Rules follow
#
nothing programs install:   default

