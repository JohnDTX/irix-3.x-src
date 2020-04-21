
#
#
include ${ROOT}/usr/include/make/commondefs

all:	tset

install:all
	${INSTALL} -g bin -u bin -m 755 -F /bin  -idb "root upd" tset

clean:
	rm -f *.o

clobber:clean
	rm -f tset

tset:	tset.c 
	$(CCF) tset.c -o $@ $(LDFLAGS) -ltermcap

