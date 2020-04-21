#
#
include ${ROOT}/usr/include/make/commondefs

#	Makefile for ed
#	"edfun" is used on pdp-11s to reduce data space by magically
#	putting some constand data, such as strings, in text space and
#	running separate I & D. Not useful for m68000s and vaxen.

PGMS	=ed
IDBTAG	=-idb "root upd" 

default all:	$(PGMS)

install:all
	${INSTALL} ${IDBTAG} -u bin -g bin -m 755 -F /bin $(PGMS)

ed:	ed.c
	$(CCF) -DPWB -o $@ $? $(LDFLAGS) 

edcrypt:ed.c
	$(CCF) -DCRYPT -DPWB -o $@ $? $(LDFLAGS) -lcrypt

clean:
	rm -f *.o

clobber:clean
	rm -f ed edcrypt
