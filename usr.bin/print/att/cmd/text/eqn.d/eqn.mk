#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	eqn make file (text subsystem)
#
# for DSL 2

#
#ident	"\%W\%"
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
# comment out local c flags on mips until we're sure the optimizer works
#LCFLAGS = -O
LDFLAGS = -s
YFLAGS = -d
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

INSDIR = /usr/bin
PINSDIR = /usr/pub

SOURCE = e.y e.h main.c diacrit.c eqnbox.c font.c fromto.c funny.c \
glob.c integral.c io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
 pile.c shift.c size.c sqrt.c text.c

OFILES =  main.o diacrit.o eqnbox.o font.o fromto.o funny.o \
glob.o integral.o io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
 pile.o shift.o size.o sqrt.o text.o
FILES =  $(OFILES) e.o

#
# Targets/Rules
#

default:    stamp all

clean:
	  rm -f *.o y.tab.h e.def

clobber:  clean
	  rm -f eqn

FRC:

#
# Specific Target/Rules follow
#
stamp:
	pwd

install:	all
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) eqn
	$(INSTALL) $(DAT_FLAGS) -F $(PINSDIR) "apseqnchar cateqnchar"
#	$(INSTALL) $(DAT_FLAGS) -ln $(PINSDIR)/eqnchar -F $(PINSDIR) apseqnchar
	$(INSTALL) $(DAT_FLAGS) -ln $(PINSDIR)/apseqnchar -F $(PINSDIR) eqnchar


compile all: eqn apseqnch cateqnch
	:

eqn:	$(FILES)
	$(CC) $(FFLAG) $(LDFLAGS) $(FILES) -ly -o eqn 

$(OFILES): e.h e.def

e.o:	e.h

e.def:	  y.tab.h
	  -cmp -s y.tab.h e.def || cp y.tab.h e.def

y.tab.h:  e.o
	:

apseqnch:	apseqnchar

cateqnch:	cateqnchar
