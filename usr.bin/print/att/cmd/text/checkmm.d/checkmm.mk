#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	checkmm make file
#
# for DSL 2

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
#
#LCFLAGS = -O $(FFLAG) $(B10)
LCFLAGS =
IFLAG = -n
ALLFLAGS = -s $(B10) $(IFLAG) $(CFLAGS) $(LDFLAGS)
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

B10 =

SOURCE = chekl.l  chekmain.c chekrout.c
FILES = chekl.o chekmain.o chekrout.o

SOURCE1 = chekl1.l chekmain1.c chekrout1.c
FILES1 = chekl1.o chekmain1.o chekrout1.o
PROGS = checkmm checkmm1

#
# Targets/Rules
#

default:    stamp all

clean:
	  rm -f $(FILES) $(FILES1) lex.yy.c

clobber:  clean
	  rm -f checkmm checkmm1

FRC:

#
# Specific Target/Rules follow
#
install: all
	$(INSTALL) $(I_FLAGS) -F /usr/bin "$(PROGS)"


stamp:
	pwd

compile all: $(PROGS)
	:

checkmm1:	$(FILES1)
	 $(CC) $(ALLFLAGS) $(FILES1) -ll -lPW -o checkmm1

checkmm:	$(FILES)
	$(CC) $(ALLFLAGS) $(FILES) -ll -lPW -o checkmm
