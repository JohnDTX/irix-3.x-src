#ident	"\%W\%"
#	text sub-system make file
#
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#
#
# Notes:
#
#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
LLDFLAGS = -s
#
# Local Definitions
#
I_FLAGS	=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)

PROGS = diffmk hyphen
DIRS  = checkmm eqn grap macref macros neqn pic ptx roff shells subndx tbl
#
# Targets/Rules
#

default:    all

clean:
	for i in $(DIRS); do (cd $$i.d; $(MAKE) -f $$i.mk $@ ); done
	rm -f hyphen.o

# Don't depend on clean here!
clobber:
	for i in $(DIRS); do (cd $$i.d; $(MAKE) -f $$i.mk $@ ); done
	rm -f diffmk

FRC:

#
# Specific Target/Rules follow
#
install:	$(PROGS)
	$(INSTALL) $(I_FLAGS) -F /usr/bin "$(PROGS)"
	for i in $(DIRS); do (cd $$i.d; $(MAKE) -f $$i.mk $@ ); done

compile all:	 $(PROGS)
	for i in $(DIRS); do (cd $$i.d; $(MAKE) -f $$i.mk $@ ); done

diffmk:	diffmk.sh
	cp diffmk.sh diffmk

hyphen:	hyphen.c
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) hyphen.c -o hyphen ; fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) hyphen.c -o hyphen ; fi
