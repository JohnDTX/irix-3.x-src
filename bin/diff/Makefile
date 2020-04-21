# Make /bin/diff
# "$Header: /d2/3.7/src/bin/diff/RCS/Makefile,v 1.1 89/03/27 14:53:54 root Exp $"


include ${ROOT}/usr/include/make/commondefs

IDBTAG=-idb "root upd" 

default all:	diff diffh

install:all
	${INSTALL} ${IDBTAG} -u bin -g bin -m 755 -F /bin diff 
	${INSTALL} ${IDBTAG} -u bin -g bin -m 755 -F /usr/lib diffh 

clean:
	rm -f *.o a.out

clobber:clean
	rm -f diff diffh
