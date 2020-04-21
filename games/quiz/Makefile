# makefile for quiz
# $Source: /d2/3.7/src/games/quiz/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:44:12 $

include ${ROOT}/usr/include/make/commondefs

B	= /usr/games
L	= $B/lib/quiz
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -g games -u games
QUIZZES = africa airport america areas arith asia babies bard chinese \
	  collectives editor elements europe greeklang inca index latin \
	  locomotive midearth morse mult murders poetry posneg pres province \
	  rockgroup seq-easy seq-hard sexes sov spell state trek ucc 

all:	quiz

install:all 
	${INSTALL} -dir ${L} ${GRPOWN} ${IDBTAG} -m 755 
	${INSTALL} ${GRPOWN} ${IDBTAG} -F ${B} -m 755 quiz
	cd quizzes; ${INSTALL} ${GRPOWN} ${IDBTAG} -F ${L} -m 644 "${QUIZZES}"

clean:
	rm -f *.o
	
clobber:clean
	rm -f quiz

FRC:
