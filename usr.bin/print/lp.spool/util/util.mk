#
#ident	"\%W\%"
#
# Notes:
#	makefile for workstation print directory
#	$Source: /d2/3.7/src/usr.bin/print/lp.spool/util/RCS/util.mk,v $
#	@(#)$Revision: 1.1 $
#	$Date: 89/03/27 18:17:17 $

FILES	= addclient mkcentpr mknetpr rmprinter rhostfix preset
TRANSFILES = mkPS

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
I_T_FLAGS =-idb "trans.sw.trans"
I_FLAGS	=-idb "std.sw.unix"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
TRANS_FLAGS=-m 755 -u lp -g lp $(I_T_FLAGS)
STD_FLAGS=-m 755 -u bin -g bin $(I_FLAGS)

PUTDIR	= /usr/spool/lp/etc/util

#
# Targets/Rules
#

default:    all

clean: FRC

clobber:    clean
	rm -f $(FILES)


FRC:

#
# Specific Target/Rules follow
#

all:
	cp addclient.sh addclient
	cp mkcentpr.sh mkcentpr
	cp mknetpr.sh mknetpr
	cp mkPS.sh mkPS
	cp rmprinter.sh rmprinter
	cp preset.sh preset
	cp rhostfix.sh rhostfix


install:    all
	$(INSTALL) $(STD_FLAGS) -F $(PUTDIR) "$(FILES)"
	$(INSTALL) $(TRANS_FLAGS) -F $(PUTDIR) "$(TRANSFILES)"
