#	makefile for "more" directory
#
#
include ${ROOT}/usr/include/make/commondefs

LIBFILES= more.help
all:	more more.help page

install:all
	${INSTALL} -u bin -g bin -m 755 -F /bin \
	-idb "root upd" more
	${INSTALL} -ln /bin/more -F /bin \
	-idb "root upd" page
	${INSTALL} -u root -g sys -m 755 -F /usr/lib \
	-idb "root upd" ${LIBFILES}

more:	more.c
	$(CC) $(CFLAGS) more.c -o $@ $(LDFLAGS) -ltermcap

page:	more
	rm -f page ; ln more page

clean:
	rm -f *.o

clobber:clean
	rm -f more page
