#ident	"\%W\%"
#
# Notes:
# transcript/lib/Makefile.sysv
#
# Copyright (c) 1985 Adobe Systems Incorporated. All Rights Reserved. 
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs
#

#
# Compile Time Options
#

#
# Local Definitions
#
#
I_FLAGS	=-idb "trans.sw.trans"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "trans.sw.trans config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "trans.sw.trans config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u $$OWNER -g $$GROUP $(UPDATE)
DAT_FLAGS=-m 644 -u $$OWNER -g $$GROUP $(I_FLAGS)

FILES = banner.pro enscript.pro pstext.pro \
	ps630.pro ps4014.pro pscat.pro psplot.pro psdit.pro \
	ehandler.ps uartpatch.ps bogusmsg.ps \
	font.map Notice

#
# Targets/Rules
#

default:    all

clean:
	rm -f - *.BAK *.CKP .emacs_[0-9]*
	cd troff.font; $(MAKE) -f troff.mk ${MFLAGS} clean
	cd ditroff.font; $(MAKE) -f ditroff.mk ${MFLAGS} clean

clobber:	clean

FRC:

#
# Specific Target/Rules follow
#
programs all:
	cd ditroff.font; $(MAKE) -f ditroff.mk ${MFLAGS} default


install: 
	$(INSTALL) $(DAT_FLAGS) -F $$PSLIBDEST "*.afm"
	$(INSTALL) $(DAT_FLAGS) -F $$PSLIBDEST "$(FILES)"

	cd ditroff.font; $(MAKE) ${MFLAGS}  -f ditroff.mk install

