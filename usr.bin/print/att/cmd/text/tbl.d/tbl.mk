#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	text Development Support Library (DSL) tbl make file
#
# DSL 2

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
LDFLAGS = -s
IFLAG = -i
#LCFLAGS = -O
#
# Local Definitions
#

#
I_FLAGS	=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)


INSDIR = /usr/bin

#
# Targets/Rules
#

default:    stamp all

clean:
	rm -f *.o

clobber: clean
	rm -f tbl

FRC:

#
# Specific Target/Rules follow
#
stamp:
	pwd

install: default
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) tbl


SFILES = t..c t[0-9].c t[bcefgimrstuv].c
OFILES = t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o\
	te.o tf.o tg.o ti.o tm.o tr.o ts.o tt.o tu.o tv.o

compile all: tbl
	:

tbl:	$(OFILES) 
	- if vax || u3b || m68000  ; \
	then $(CC) $(IFLAG) $(CFLAGS) $(LDFLAGS) $(OFILES) -o tbl ; fi
	- if mips ; \
	then $(CC)  $(CFLAGS) $(LDFLAGS) $(OFILES) -o tbl ; fi

$(OFILES):: t..c
	:
