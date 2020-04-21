#
# Notes:
# transcript/sh/Makefile
#
# Copyright 1985 Adobe Systems Incorporated
#

include	$(ROOT)/usr/include/make/commondefs

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
UPDATE	=-idb "trans.sw.trans"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u $$OWNER -g $$GROUP $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

#
# Targets/Rules
#

default:    all

clean:
	-rm -f - ptroff psroff *.BAK *.CKP .emacs_[0-9]*

clobber:	clean

FRC:

#
# Specific Target/Rules follow
#

programs all: ptroff psroff


ptroff: ptroff.sh
	sed	-e s,TROFFFONTDIR,$$TROFFFONTDIR,g \
		ptroff.sh >ptroff

psroff: psroff.sh
	cp psroff.sh psroff

install: default
	$(INSTALL) $(I_FLAGS) -F $$BINDIR psroff
