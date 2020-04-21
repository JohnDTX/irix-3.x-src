
include ${ROOT}/usr/include/make/commondefs

B	= /usr/games
L	= $B/lib
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -g games -u games

OBJS	= crib.o support.o cards.o score.o io.o
TOBJS	= test.o cards.o score.o io.o

all:	cribbage crib.instr

install:all
	${INSTALL} ${GRPOWN} -F ${B} ${IDBTAG} -m 755 cribbage
	${INSTALL} ${GRPOWN} -F ${L} ${IDBTAG} -m 644 crib.instr

clean:
	rm -f *.o crib.instr

clobber:clean
	rm -f cribbage 

cribbage:$(OBJS)
	$(CCF) -o $@ $(OBJS) $(LDFLAGS) 

$(OBJS):deck.h cribbage.h

$(TOBJS):deck.h cribbage.h

crib.instr:macro cribbage.n
	nroff cribbage.n > $@

test:	$(TOBJS)
	$(CCF) -o $@ $(TOBJS) $(LDFLAGS) 
