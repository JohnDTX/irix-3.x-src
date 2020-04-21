#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved


#	nroff/troff make file (text subsystem)
#
# DSL 2

#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
#LCFLAGS = -O
LCFLAGS = -g
LLDFLAGS = -s

#
# Local Definitions
#
INCORE = -DINCORE
#
I_FLAGS	=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)


#
# Targets/Rules
#

default:    stamp all

clean:
	cd nroff.d;  $(MAKE) -f nroff.mk clean
	cd troff.d;  $(MAKE) -f troff.mk clean

clobber:
	cd nroff.d;  $(MAKE) -f nroff.mk clobber
	cd troff.d;  $(MAKE) -f troff.mk clobber

FRC:

#
# Specific Target/Rules follow
#
stamp:
	pwd

install:
	cd nroff.d; $(MAKE) -f nroff.mk insnroff "INCORE=$(INCORE)"
	cd nroff.d; $(MAKE) -f nroff.mk insterms
	cd troff.d; $(MAKE) -f troff.mk instroff "INCORE=$(INCORE)"
	cd troff.d; $(MAKE) -f troff.mk insfonts "INCORE=$(INCORE)"


compile all:  nroff terms troff fonts

nroff:
	- if u3b2 || u3b5 || u3b15 ; \
	then cd nroff.d;   $(MAKE) -f nroff.mk nroff INCORE= ; fi
	- if vax || u3b || m68000 || mips ; \
	then cd nroff.d;   $(MAKE) -f nroff.mk nroff "INCORE=$(INCORE)" ; fi

troff:
	- if u3b2 || u3b5 || u3b15 ; \
	then cd troff.d;   $(MAKE) -f troff.mk troff INCORE= ; fi
	- if vax || u3b || m68000 || mips ; \
	then cd troff.d;   $(MAKE) -f troff.mk troff INCORE=$(INCORE) ; fi

terms:
	cd nroff.d;  $(MAKE) -f nroff.mk terms

fonts:
	cd troff.d;  $(MAKE) -f troff.mk fonts

insnroff:
	$(MAKE) -f roff.mk INS=cp ROOT=$(ROOT) INCORE=$(INCORE) nroff
instroff:
	$(MAKE) -f roff.mk INS=cp ROOT=$(ROOT) INCORE=$(INCORE) troff
