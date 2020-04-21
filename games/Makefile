#	makefile for workstation /usr/games directory

include ${ROOT}/usr/include/make/commondefs

#
# $Source: /d2/3.7/src/games/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:42:32 $

DIRS	= adventure cribbage fortune mastermind quiz rogue trek

PROGS	= aliens alienslog arithmetic autorobots back bcd bj chase \
	  craps fish hangman life maze moo naliens number \
	  rain robots ttt twinkle worm worms wump

NULLS	= robotscore rogue_roll alienslog arscore

LLDLIBS	= -lsun -lbsd -ldbm
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -g games -u games
G	= /usr/games
L	= $G/lib

SKIP	= random

all:	$(PROGS) FRC
	for i in $(DIRS); do (echo $$i; cd $$i; make $@); done

install:$(PROGS) nulls FRC
	${INSTALL} ${GRPOWN} ${IDBTAG} -m 755 -F ${G}  "${PROGS}"
	for i in $(DIRS); do (echo $$i; cd $$i; make $@); done
	(cd lib; ${INSTALL} ${GRPOWN} ${IDBTAG} -m 600 -F ${L} "${NULLS}")

clean:
	rm -f *.o a.out core y.tab.c y.tab.h
	for i in $(DIRS); do (echo $$i; cd $$i; make $@); done

nulls: 
	(cd lib; touch ${NULLS})

clobber: clean
	rm -f $(PROGS)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) -lcurses -ltermcap -lm 

$(DIRS):FRC
	cd $@ ; make all

FRC:
