#
#ident	"\%W\%"
#
# Notes:
#
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved


#	nroff terminal driving tables make file
#
# DSL 2.


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
I_FLAGS	=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

INSDIR = /usr/lib/nterm
FILES = tab.8510 tab.2631 tab.2631-c tab.2631-e tab.300 tab.300-12 tab.300s \
	tab.300s-12 tab.37 tab.382 tab.4000a tab.450 \
	tab.450-12 tab.832 tab.lp tab.tn300 tab.X
IFILES = $(FILES) tab.300S tab.300S-12 tab.4000A

#
# Targets/Rules
#

default:    all

clean:
	rm -f maketerms

clobber:  clean
	rm -f ${FILES}

FRC:

#
# Specific Target/Rules follow
#
install: default
	$(INSTALL) -dir $(INSDIR) 
	$(INSTALL) $(DAT_FLAGS) -F $(INSDIR) "$(FILES)"
	$(INSTALL) -ln $(INSDIR)/tab.300s -F $(INSDIR) tab.300S
	$(INSTALL) -ln $(INSDIR)/tab.300s-12 -F $(INSDIR) tab.300S-12
	$(INSTALL) -ln $(INSDIR)/tab.4000a -F $(INSDIR) tab.4000A

all:	$(FILES)

tab.2631:	a.2631 b.lp
	cat a.2631 b.lp >tab.2631
tab.2631-c:	a.2631-c b.lp
	cat a.2631-c b.lp >tab.2631-c
tab.2631-e:	a.2631-e b.lp
	cat a.2631-e b.lp >tab.2631-e
tab.300:	a.300 b.300
	cat a.300 b.300 >tab.300
tab.300-12:	a.300-12 b.300
	cat a.300-12 b.300 >tab.300-12
tab.300s:	a.300s b.300
	cat a.300s b.300 >tab.300s
tab.300s-12:	a.300s-12 b.300
	cat a.300s-12 b.300 >tab.300s-12
tab.37:	ab.37
	cat ab.37 >tab.37
tab.382:	a.382 b.300
	cat a.382 b.300 >tab.382
tab.4000a:	a.4000a b.300
	cat a.4000a b.300 >tab.4000a
tab.450:	a.450 b.300
	cat a.450 b.300 >tab.450
tab.450-12:	a.450-12 b.300
	cat a.450-12 b.300 >tab.450-12
tab.832:	a.832 b.300
	cat a.832 b.300 >tab.832
tab.8510:	ab.8510
	cat ab.8510 >tab.8510
tab.X:	ab.X
	cat ab.X >tab.X
tab.lp:	a.lp b.lp
	cat a.lp b.lp >tab.lp
tab.tn300:	ab.tn300
	cat ab.tn300 >tab.tn300
