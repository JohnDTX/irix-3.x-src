#
#
include ${ROOT}/usr/include/make/commondefs


LCDEFS= -O  
LCINCS=  -I${ROOT}/usr/include/bsd -I${ROOT}/usr/include
LLDLIBS=  -lsun -lbsd -ldbm

MAKEPATH= .:..:../../ypserv

TARGETS= makedbm yppush stdhosts ypmapname ypset ypxfr yppoll
SCRIPTS= ypinit ypxfr_1pd ypxfr_2pd ypxfr_1ph

all: ${TARGETS}

.c:
	${CC} ${CFLAGS} -o $@ $? ${LDFLAGS}
.o:
	${CC} ${CFLAGS} -o $@ $? ${LDFLAGS}

ypxfr: ypxfr.o ypdbname.o
	${CC} ${CFLAGS} -o $@ ypxfr.o ypdbname.o ${LDFLAGS}

ypmapname: ypmapname.o ypdbname.o
	${CC} ${CFLAGS} -o $@ ypmapname.o ypdbname.o ${LDFLAGS}

install:	all
	${INSTALL} -m 755 -F /usr/etc/yp ${IDBTAG} "${TARGETS}"
	${INSTALL} -src ../make.script -F /usr/etc/yp  ${IDBTAG} Makefile
	for i in ${SCRIPTS}; \
	do \
		${INSTALL} -m 755 -src "../$$i.sh" -F /usr/etc/yp \
			${IDBTAG} $$i ;\
	done

clobber clean:
	rm -f ${TARGETS} a.out core *.o
