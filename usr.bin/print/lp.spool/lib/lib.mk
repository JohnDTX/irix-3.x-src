#
#ident	"\%W\%"
#
# Notes:
#	makefile for workstation print directory
#	$Source: /d2/3.7/src/usr.bin/print/lp.spool/lib/RCS/lib.mk,v $
#	@(#)$Revision: 1.1 $
#	$Date: 89/03/27 18:16:42 $

#

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
I_T_FLAGS =-idb "trans.sw.trans"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "std.sw.unix config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "std.sw.unix config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
LOG_FLAGS=-m 755 -u lp -g lp $(NOUPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(I_FLAGS)
TRANS_FLAGS=-m 644 -u bin -g bin $(I_T_FLAGS)

TRANSFILES = psinterface
FILES	= centface netface
LOGFILE = log.rotate
PUTDIR	= /usr/spool/lp/etc/lib

#
# Targets/Rules
#

default:    all

clean: FRC

clobber:FRC

FRC:

#
# Specific Target/Rules follow
#
all:

install:
	$(INSTALL) $(DAT_FLAGS) -F $(PUTDIR) "$(FILES)"
	$(INSTALL) $(TRANS_FLAGS) -F $(PUTDIR) "$(TRANSFILES)"
	$(INSTALL) $(LOG_FLAGS) -F $(PUTDIR) $(LOGFILE)
