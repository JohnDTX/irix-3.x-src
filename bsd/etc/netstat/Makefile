# Make bsd 'netstat'
#
# "$Header"

include ${ROOT}/usr/include/make/commondefs

OBJS=	host.o if.o inet.o main.o mbuf.o ns.o route.o unix.o
TARGETS= netstat

LCOPTS=	-O
LCDEFS=	-DUSG
LCINCS=	-I${ROOT}/usr/include/bsd
LLDLIBS = -lbsd -ldbm
IDBTAG = -idb "std.sw.unix"

default all:	${TARGETS}

netstat:${OBJS}
	${CCF} -o netstat ${OBJS} ${LDFLAGS} 

install:	all
	${INSTALL} -u root -g sys -m 2755 -F /usr/etc ${IDBTAG} "${TARGETS}"

clean:
	rm -rf *.o a.out core

clobber:	clean
	rm -rf ${TARGETS}
