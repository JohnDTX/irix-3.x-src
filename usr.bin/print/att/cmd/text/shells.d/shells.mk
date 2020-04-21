#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	text subsystem shells make file
#
# DSL 2.

#   Note: I commented out the helpdir target, and much of the samples target.
#   They create directories and copy to them.  We don't do that in our
#   distributions anymore, not even to a DEST tree.  Glen W. 11/20/86

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
INSLIB = /usr/lib
HINSDIR = /usr/pub
FILES =  mm.sh mmt.sh 
SFILES = nroff.letter mm.report mm.sales mm.letter tbl.language tbl.bridges \
	tbl.pres eqn.stats troff.fonts troff.sizes troff.ad \
	troff.aeneid pic.forms

#
# Targets/Rules
#

default:    stamp all

clean mmclean mmtclean mvtclean:

clobber:  clean mmclobber mmtclobber
	rm -f mvt
FRC:

#
# Specific Target/Rules follow
#

stamp:
	pwd

install:	all
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) "mm mmt"
	$(INSTALL) $(I_FLAGS) -ln $(INSDIR)/mmt -F $(INSDIR) mvt
	$(INSTALL) $(I_FLAGS) -dir "$(INSLIB)/dwb $(INSLIB)/dwb/samples $(HINSDIR)"
	$(INSTALL) $(I_FLAGS) -F $(HINSDIR) terminals
	$(INSTALL) $(I_FLAGS) -F $(INSLIB)/dwb/samples "$(SFILES)"

compile all:  mm mmt mvt samples

mm:	mm.sh
	cp mm.sh mm

mmt:	mmt.sh
	cp mmt.sh mmt

mvt:	mmt

samples: $(SFILES)

mmclobber:   ;  rm -f mm
mmtclobber mvtclobber:  ;  rm -f mmt
