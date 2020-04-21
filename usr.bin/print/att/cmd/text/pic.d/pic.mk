#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	makefile for pic.
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
LCFLAGS =  -c
IFLAG = -i
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
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

YFLAGS = -d
OFILES = main.o print.o misc.o symtab.o blockgen.o boxgen.o circgen.o \
	arcgen.o linegen.o movegen.o textgen.o \
	input.o for.o pltroff.o
CFILES = main.c print.c misc.c symtab.c blockgen.c boxgen.c circgen.c \
	arcgen.c linegen.c movegen.c textgen.c \
	input.c for.c pltroff.c
SRCFILES = picy.y picl.l pic.h $(CFILES)

INSDIR = /usr/bin

#
# Targets/Rules
#

default:    stamp all


clean:
	rm -f $(OFILES) picy.o picl.o y.tab.h picy.c picl.c lex.yy.c

clobber:	clean
	rm -f pic pltroff

FRC:

#
# Specific Target/Rules follow
#
stamp:
	pwd

install: default
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) pic

all:	pic

pic::	picy.o picl.o $(OFILES)
	- if vax || u3b || m68000  ; \
	then $(CC) $(IFLAG) $(FFLAG) $(LDFLAGS) picy.o picl.o \
	    $(OFILES) -lm -o pic ; fi
	- if mips ; \
	then $(CC) $(FFLAG) $(LDFLAGS) picy.o picl.o $(OFILES) -lm -o pic ; fi

$(OFILES):	pic.h
picy.c:	picy.y pic.h
picl.c:	picl.l pic.h

y.tab.h:	picy.o

pic.ydef:	y.tab.h
	-cmp -s y.tab.h pic.ydef || cp y.tab.h pic.ydef

pltroff:	driver.o pltroff.o
	$(CC)  $(IFLAG) pltroff.o driver.o -lm -o pltroff 
