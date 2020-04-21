#	Copyright (c) 1984 AT&T
#	  All Rights Reserved


#	text Development Support Library (DSL) macros make file
#
# DSL 2
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

#
# Local Definitions
I_FLAGS	=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(I_FLAGS)

#
LIST = lp
MINSLIB = /usr/lib/macros
TINSLIB = /usr/lib/tmac
TMACFILES = tmac.an tmac.m tmac.ptx tmac.v

#
# Targets/Rules
#

default:    stamp all

clean mmnclean mmtclean vmcaclean ptxclean stringsclean:

clobber:  clean mmnclobber mmtclobber vmcaclobber \
		ptxclobber stringsclobber manclobber

mmnclobber:  ;  rm -f mmn
mmtclobber:  ;  rm -f mmt
vmcaclobber: ;  rm -f vmca
ptxclobber:  ;  rm -f ptx
manclobber:  ;  rm -f an
stringsclobber:	;  rm -f strings.mm strings.mm.src

FRC:

#
# Specific Target/Rules follow
#

compile all:	mmn mmt vmca ptx tmac man

stamp:
	pwd

install:	all
	$(INSTALL) $(I_FLAGS) -dir $(MINSLIB)
	$(INSTALL) $(I_FLAGS) -dir $(TINSLIB)
	$(INSTALL) $(DAT_FLAGS) -F $(MINSLIB) "mmn mmt vmca ptx an strings.mm"
	$(INSTALL) $(DAT_FLAGS) -F $(TINSLIB) "$(TMACFILES)"

mmn:	mmn.src strings.mm
	sh ./macrunch mmn

strings.mm:	strings.src
# I renamed strings.mm.src to strings.src because strings.mm.src won't RCS,
# however macrunch requires a name of the form foo.src; hence the link.
	rm -f strings.mm.src
	- ln strings.src strings.mm.src
	sh ./macrunch strings.mm

mmt:	mmt.src strings.mm
	sh ./macrunch mmt

vmca:	vmca.src
	sh ./macrunch vmca

man:	an.src
	sh ./macrunch an

ptx:	ptx.src
	sh ./macrunch ptx

tmac:	$(TMACFILES)

listing:    listmmn listmmt listvmca listman listptx

listmmn: ;  nl -ba mmn.src | pr -h "mmn.src" | $(LIST)
	    macref -s -t mmn.src | pr -h "macref of mmn.src" | $(LIST)
listmmt: ;  nl -ba mmt.src | pr -h "mmt.src" | $(LIST)
	    macref -s -t mmt.src | pr -h "macref of mmt.src" | $(LIST)
listvmca: ; nl -ba vmca.src | pr -h "vmca.src" | $(LIST)
	    macref -s -t vmca.src | pr -h "macref of vmca.src" | $(LIST)
listman: ;  nl -ba an.src | pr -h "an.src" | $(LIST)
	    macref -s -t an.src | pr -h "macref of an.src" | $(LIST)
listptx: ;  nl -ba ptx.src | pr -h "ptx.src" | $(LIST)
	    macref -s -t ptx.src | pr -h "macref of ptx.src" | $(LIST)
