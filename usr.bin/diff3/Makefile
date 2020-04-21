#
#
include ${ROOT}/usr/include/make/commondefs


all:	diff3 diff3prog

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" diff3
	${INSTALL} -u bin -g bin -m 755 -F usr/lib \
	-idb "std.sw.unix mr" diff3prog

clean:
	rm -f *.o

clobber:clean
	rm -f diff3 diff3prog
