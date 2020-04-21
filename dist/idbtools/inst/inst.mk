# ident "$Header: "
#
# Notes: Makefile for inst
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
IDIR	= ../include
LCINCS	= -I$(IDIR)

CMDS	= inst genprod genimage distcp versions
SCRIPTS	= instdriver

LIBS	= ../libinst/libinst.a ../libidb/libidb.a

#
# Targets/Rules
#

default:	$(CMDS) $(SCRIPTS)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(CMDS) $(SCRIPTS)

FRC:

#
# Specific Target/Rules follow
#
install: default
	$(INSTALL) -F /usr/sbin -idb "mr std.sw.unix" "inst versions"
	$(INSTALL) -F /usr/lib/inst -idb "mr std.sw.unix" help
	$(INSTALL) -F /usr/sbin -idb mr instdriver
	$(INSTALL) -F /usr/sbin -idb noship "genimage genprod"

local: default
	cp $(CMDS) $(LOCAL)

$(CMDS): $$@.o $(LIBS)
	$(CCF) -o $@ $@.o $(LIBS) -lbsd $(LDFLAGS)

instdriver: instdr.sh
	cp instdr.sh instdriver
	chmod 755 instdriver

$(LIBS):
	cd $(@D); $(MAKE) -f $(@:.a=.mk) $(@F)
