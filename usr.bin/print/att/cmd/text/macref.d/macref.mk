#
#ident	"\%W\%"
#
# Notes:
#
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	text subsystem macref make file
#
# DSL 2

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
IFLAG = -n
LDFLAGS = -s
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
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

SFILES = 
FFILES = macref.c macrform.c macrstat.c macrtoc.c main.c match.c
FILES = macref.o macrform.o macrstat.o macrtoc.o main.o match.o
#
# Targets/Rules
#

default:    stamp all

clean:
	rm -f $(FILES)

clobber:    clean 
		rm -f macref

FRC:

#
# Specific Target/Rules follow
#

stamp:
	pwd

install:	all
	$(INSTALL) $(I_FLAGS) -F /usr/bin macref

compile all:  macref
	:

macref:	$(FILES)
	- if vax || u3b || m68000  ; \
	then $(CC) $(LDFLAGS) $(IFLAG) $(FILES)  -o macref ; fi
	- if mips ; \
	then $(CC) $(LDFLAGS) $(FILES)   -o macref ; fi
