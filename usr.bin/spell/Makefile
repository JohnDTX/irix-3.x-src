#
#
include ${ROOT}/usr/include/make/commondefs
#

# spell makefile for vax->68000 system v
#

L	= /usr/lib/spell
D	= /usr/dict
IDBTAGS = -idb "std.sw.unix"

PROGS	= spell spellprog spellin hashcheck hashmake history

all:	$(PROGS)

install:all
	${INSTALL} ${IDBTAGS} -u bin -g bin -dir "${L} ${D}"
	${INSTALL} ${IDBTAGS} -u bin -g bin -m 755 -F /usr/bin spell
	${INSTALL} ${IDBTAGS} -u bin -g bin -m 666 -F ${L} history
	${INSTALL} ${IDBTAGS} -u bin -g bin -m 755 -F ${L} \
		"spellprog spellin hashcheck hashmake "
	${INSTALL} ${IDBTAGS} -u bin -g bin -m 644 -F ${L} \
		 "hlista hlistb hstop british american extra local stop list "
	${INSTALL} ${IDBTAGS} -ln /usr/lib/spell/list -F ${D} words

clean:
	rm -f *.o

clobber:clean
	rm -f $(PROGS)

spellprog:spellprog.o hash.o hashlook.o huff.o malloc.o
	$(CCF) spellprog.o hash.o hashlook.o huff.o malloc.o ${LDFLAGS} -o $@

spellin:spellin.o huff.o
	$(CCF) spellin.o huff.o ${LDFLAGS} -o $@

hashcheck:hashcheck.o hash.o huff.o
	$(CCF) hashcheck.o hash.o huff.o ${LDFLAGS} -o $@

hashmake:hashmake.o hash.o
	$(CCF) hashmake.o hash.o ${LDFLAGS} -o $@

huff.o:	huff.h huff.c

history: history
	touch history
