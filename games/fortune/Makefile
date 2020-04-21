# makefile for fortune
# $Source: /d2/3.7/src/games/fortune/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:43:37 $

include ${ROOT}/usr/include/make/commondefs

B	= /usr/games
L	= $B/lib
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -g games -u games

all:	fortune fortunes

install:all
	${INSTALL} ${GRPOWN} ${IDBTAG} -F ${B} -m 755 fortune
	${INSTALL} ${GRPOWN} ${IDBTAG} -F ${L} -m 644 fortunes

clean:
	rm -f *.o
	
clobber:clean
	rm -f fortune fortunes

fortunes:fortunes.dat
	cp $? $@

FRC:
