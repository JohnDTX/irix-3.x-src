all:	libg.a

#
#
include ${ROOT}/usr/include/make/commondefs

libg.a:	dbxxx.s
	$(AS) dbxxx.s -o $@

install: libg.a
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix" libg.a

clean:

clobber:clean
	rm -f libg.a
