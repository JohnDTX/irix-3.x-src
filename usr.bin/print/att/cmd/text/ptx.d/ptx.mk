#
#ident	"\%W\%"
#
# Notes:
#
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
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
DAT_FLAGS=-m 644 -u bin -g bin $(I_FLAGS)

INSLIB = /usr/lib
INSDIR = /usr/bin
FILES = ptx.c eign.sh
#
# Targets/Rules
#

default:    stamp all

clean:
	rm -f *.o

clobber: clean
	rm  -f eign ptx

FRC:

#
# Specific Target/Rules follow
#
stamp:
	pwd

install: default
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) ptx
	$(INSTALL) $(DAT_FLAGS) -F $(INSLIB) eign

all: ptx eign

ptx:	ptx.c
	- if vax || u3b || m68000  ; \
	then $(CC) -O $(LDFLAGS) ptx.c -o ptx ;  fi
	- if mips ; \
	then $(CC) $(CFLAGS) $(LDFLAGS) ptx.c -o ptx ;  fi

eign:	eign.sh
	cp eign.sh eign
