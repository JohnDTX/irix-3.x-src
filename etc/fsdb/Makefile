#
#
include ${ROOT}/usr/include/make/commondefs

all:	fsdb


install:all
	${INSTALL} -u root -g sys -m 755 -F /etc \
	-idb "std.sw.unix mr" fsdb

clean:
	rm -f *.o

clobber:clean
	rm -f fsdb #fsdb1b

fsdb:	fsdb.c
	$(CCF) $(CFLAGS) $(LDFLAGS) -DFsTYPE=2 fsdb.c -o fsdb

#fsdb1b:fsdb1b.c
#	$(CCF) $(CFLAGS) $(LDFLAGS) -DFsTYPE=1 fsdb1b.c -o fsdb1b
