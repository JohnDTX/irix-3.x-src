#
#ident	"\%W\%"
#
# Notes:
# lib/ditroff.font/ditroff.mk
#
# Copyright (c) 1984 Adobe Systems Incorporated. All Rights Reserved.
# PostScript is a trademark of Adobe Systems Incorporated.
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/lib/ditroff.font/RCS/ditroff.mk,v 1.1 89/03/27 18:19:49 root Exp $

# see README for more information
# makedev_host is the ditroff program that builds device descriptions

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
I_FLAGS	=-idb "trans.sw.trans"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "trans.sw.trans config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "trans.sw.trans config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u $$OWNER -g $$GROUP $(UPDATE)
DAT_FLAGS=-m 644 -u $$OWNER -g $$GROUP $(I_FLAGS)


#
# Targets/Rules
#

default:    makedev all

clean:
	rm -f ${FONTNAMES} ${TEMPFILES} core DESC *.out *.font *.aux \
	    *.CKP *.BAK .emacs_[0-9]* temp*

clobber:	clean

FRC:

#
# Specific Target/Rules follow
#

LOADFONTS = R I B BI H HB C CB S SS
MOREFONTS = HO HD CO CD

FONTNAMES = ${LOADFONTS} ${MOREFONTS}
TEMPFILES = temp.header temp.spaces temp.trailer temp.aux

programs:

install: makedev all
	$(INSTALL) $(DAT_FLAGS) -F $$DITDEST/devpsc "*.aux *.map *.out DESC"

makedev:
	test -r $$MAKEDEV


all DESC.out: ${LOADFONTS} moreout
	$$MAKEDEV DESC

allfonts: ${FONTNAMES}

moreout: ${MOREFONTS}
	$$MAKEDEV $?

${LOADFONTS}:
	$(MAKE) -f ditroff.mk ${MFLAGS} $@.font

${MOREFONTS}:
	-rm -f $@.font
	$(MAKE) -f ditroff.mk ${MFLAGS} $@.font

${FONTNAMES}: DESC afmdit.awk ditroff.mk

DESC: afmdit.awk ditroff.mk charset devspecs
	rm -f DESC
	echo "# ditroff device description for PostScript" >> DESC
	echo "# PostScript is a trademark of Adobe Systems Incorporated">>DESC
	echo ${LOADFONTS} | awk '{print "fonts", NF, $$0}' >> DESC
	cat devspecs >>DESC
	echo "charset" >> DESC
	cat charset >> DESC


.SUFFIXES: .out .font .map

# how to make a .font from a .map
# note that .font is a placeholder,  the file of interest has
# the same name with no extension (this is input to makedev)

.map.font:
	/bin/rm -f - ${TEMPFILES}
	./afmdit $* ..
	touch $*.font
	/bin/rm -f -  ${TEMPFILES}

# how to make a .out from a .font (again, the .font is dropped)

.font.out: DESC
	$$MAKEDEV $*

# how to make a .out from a .map
# just do the above two steps

.map.out:
	$(MAKE) -f ditroff.mk ${MFLAGS} $@.font
	$(MAKE) -f ditroff.mk ${MFLAGS} $@.out
