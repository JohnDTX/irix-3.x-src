#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	makefile for grap.

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
YFLAGS = -d -D
IFLAG = -i
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


# ALLOC = malloc.o
OFILES = main.o input.o print.o frame.o for.o coord.o ticks.o plot.o label.o misc.o $(ALLOC)
CFILES = main.c input.c print.c frame.c for.c coord.c ticks.c plot.c label.c misc.c
SRCFILES = grap.y grapl.l grap.h $(CFILES)
LIBDIR = /usr/lib/dwb


#
# Targets/Rules
#

default:    stamp all

clean:
	rm -f grap.o grapl.o $(OFILES) y.tab.h prevy.tab.h

clobber:	clean
	rm -f grap

FRC:

#
# Specific Target/Rules follow
#
stamp:
	pwd

all:	$(LIBDIR)/grap.defines grap

install:	all
	$(INSTALL) $(I_FLAGS) -F /usr/bin grap
	$(INSTALL) -dir $(LIBDIR)
	$(INSTALL) $(DAT_FLAGS) -F $(LIBDIR) grap.defines

grap:	grap.o grapl.o $(OFILES) grap.h 
	- if vax || u3b || m68000  ; \
	then $(CC) $(IFLAG) $(FFLAG) $(LDFLAGS) grap.o grapl.o \
	    $(OFILES) -lm -o grap ; fi
	- if mips ; \
	then $(CC) $(FFLAG) $(LDFLAGS) grap.o grapl.o \
	    $(OFILES) -lm -o grap; fi

$(LIBDIR)/grap.defines:	grap.defines

$(OFILES) grapl.o:	grap.h prevy.tab.h

grap.o:	grap.h

y.tab.h:	grap.o

prevy.tab.h:	y.tab.h
	-cmp -s y.tab.h prevy.tab.h || cp y.tab.h prevy.tab.h
