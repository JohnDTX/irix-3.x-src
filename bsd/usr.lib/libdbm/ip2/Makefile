#
#
include ${ROOT}/usr/include/make/commondefs

# $Source: /d2/3.7/src/bsd/usr.lib/libdbm/ip2/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 15:02:26 $
#
INCL= -I${ROOT}/usr/include/bsd -I${ROOT}/usr/include
DEFS= -DUSG
CFLAGS= -O ${DEFS} ${INCL}
OBJS= dbm.o ndbm.o 
SRCS= dbm.c ndbm.c 
TARGETS= libdbm.a 

all: ${TARGETS}

install:	all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" libdbm.a
	ranlib ${ROOT}/usr/lib/libdbm.a

clobber: clean
	rm -f ${TARGETS} 

clean:
	rm -f ${OBJS} core a.out

libdbm.a: ${OBJS}
	@rm -rf libdbm.a
	ar cu libdbm.a ${OBJS}
	ranlib libdbm.a

dbm.o: ../common/dbm.c
	${CC} ${CFLAGS} -c ../common/dbm.c

ndbm.o: ../common/ndbm.c
	${CC} ${CFLAGS} -c ../common/ndbm.c
