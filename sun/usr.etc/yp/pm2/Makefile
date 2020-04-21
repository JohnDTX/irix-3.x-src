#
#
include ${ROOT}/usr/include/make/commondefs


CFLAGS= -O  -I${ROOT}/usr/include/bsd -I${ROOT}/usr/include
LIBS=  -lyp -lrpc -lbsd -ldbm

MAKEPATH= .:..:../../ypserv

TARGETS= makedbm yppush stdhosts ypmapname ypset ypxfr yppoll
SCRIPTS= ypinit ypxfr_1pd ypxfr_2pd ypxfr_1ph

all: ${TARGETS}

.c:
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $? ${LIBS}
.o:
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $? ${LIBS}

ypxfr: ypxfr.o ypdbname.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ypxfr.o ypdbname.o ${LIBS}

ypmapname: ypmapname.o ypdbname.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ypmapname.o ypdbname.o ${LIBS}

install:	all
	${INSTALL} -m 755 -F /usr/etc/yp \
	-idb "std.sw.unix mr" "${TARGETS}"
	${INSTALL} -src ../make.script -F /usr/etc/yp \
	-idb "std.sw.unix mr" Makefile
	for i in ${SCRIPTS}; \
	do \
		cp ../$$i.sh ${ROOT}/usr/etc/yp/$$i; \
		chmod 755 ${ROOT}/usr/etc/yp/$$i; \
	done

clobber clean:
	rm -f ${TARGETS} a.out core *.o
