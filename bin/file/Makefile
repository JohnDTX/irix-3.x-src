# "$Header: /d2/3.7/src/bin/file/RCS/Makefile,v 1.1 89/03/27 14:54:02 root Exp $"
#
#
include ${ROOT}/usr/include/make/commondefs

default all:	file

install: all
	${INSTALL} -g bin -u bin -m 755 -F /bin -idb "root upd" file
	${INSTALL} -g bin -u bin -m 644 -F /etc -idb "root upd" magic

clean:
	rm -f *.o a.out

clobber:clean
	rm -f file
