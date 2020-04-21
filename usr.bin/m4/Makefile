#
#
include ${ROOT}/usr/include/make/commondefs

all:	m4

install :all
	${INSTALL} -o -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" m4

m4:	m4.o m4ext.o m4macs.o m4y.o
	$(CC) $(LDFLAGS) -o $@ m4.o m4ext.o m4macs.o m4y.o $ \
	${ROOT}/usr/lib/liby.a

clean:
	rm -f *.o m4y.c 

clobber:clean
	rm -f m4
