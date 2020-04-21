# /usr/src/games/master makefile
# $Source: /d2/3.7/src/games/mastermind/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:43:43 $

include ${ROOT}/usr/include/make/commondefs

OBJECTS = Version.o abbrev.o ematch.o equal.o gentab.o guess.o imatch.o \
	  init.o instr.o main.o marksubs.o pack.o play1.o play2.o \
	  prune.o randint.o read1.o review.o rguess.o unpack.o \
	  yorn.o zcount.o

B	= /usr/games
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -g games -u games

all:	mastermind

install:all
	${INSTALL} ${GRPOWN} -m 755 ${IDBTAG} -F ${B} mastermind
	${INSTALL} ${GRPOWN} -m 444 ${IDBTAG} -F ${B}/mmhow mmhow.del

clean:
	rm -f *.o

clobber:clean
	rm -f mastermind

mastermind:$(OBJECTS)
	$(CCF) $(NFLAG) $(OBJECTS) -o $@ $(LDFLAGS) 

gentab.o:gentab.c max.h
guess.o:guess.c max.h
init.o:	init.c max.h
main.o:	main.c max.h
play2.o:play2.c max.h
prune.o:prune.c max.h
review.o:review.c max.h
rguess.o:rguess.c max.h
