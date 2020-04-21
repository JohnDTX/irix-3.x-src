#
#ident	"\%W\%"
#
# Notes:
# This is the top of the tree of files that fit into the spooling system.

#	makefile for workstation print directory
#	$Source: /d2/3.7/src/usr.bin/print/lp.spool/RCS/lp.spool.mk,v $
#	@(#)$Revision: 1.1 $
#	$Date: 89/03/27 18:16:17 $

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#

#
# Local Definitions
#
#
I_FLAGS	=-idb "std.sw.unix"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "std.sw.unix config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "trans.sw.trans config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
LP_FLAGS=-m 755 -u lp -g lp $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)


#
# Targets/Rules
#
DIRS	= util lib drivers patterns

default:    all

clean: FRC
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@); done

clobber:FRC
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@); done


FRC:

#
# Specific Target/Rules follow
#

FILES	= config printer
PUTDIR	= /usr/spool/lp/etc

all:
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@); done

install: 
	$(INSTALL) $(I_FLAGS) -dir $(PUTDIR) "/usr/lib/print /usr/lib/print/patterns"
	$(INSTALL) $(I_FLAGS) -u lp -g lp -m 755 -dir $(PUTDIR)/log
	$(INSTALL) $(LP_FLAGS) -F $(PUTDIR) "$(FILES)"
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@); done
