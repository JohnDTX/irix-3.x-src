#	text subsystem shells make file
#	SCCS:  @(#)shells.mk	1.14
# $Source: /d2/3.7/src/usr.bin/text/shells.d/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:28:58 $
 
include ${ROOT}/usr/include/make/commondefs

B	= /usr/bin
H	= /usr/lib/help
# man.sh is now in usr.bin
FILES	= mm.sh mmt.sh mvt.sh osdd.sh greek.sh term text
PROGS	= mm mmt osdd greek
LINKS	= mvt
HELPS	= term text
GRPOWN = -g bin -u bin
MODS   = -m 755
IDBTAG = -idb "std.sw.unix"

all:	$(PROGS) $(LINKS) $(HELPS)

install:all
	${INSTALL} ${GRPOWN} ${MODS} ${IDBTAG} -F ${B} "${PROGS}"
	${INSTALL} -ln ${B}/mmt ${IDBTAG} -F ${B} mvt
	${INSTALL} ${GRPOWN} ${MODS} ${IDBTAG} -F ${H} "${HELPS}"

mvt:	mmt
	rm -f $@
	ln $? $@

clean:

clobber:clean
	rm -f $(PROGS) $(LINKS)
