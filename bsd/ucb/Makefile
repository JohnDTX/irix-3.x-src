# "$Header: /d2/3.7/src/bsd/ucb/RCS/Makefile,v 1.1 89/03/27 15:00:24 root Exp $"
#
#
include ${ROOT}/usr/include/make/commondefs

INSCMD	=${INSTALL} -u root -g sys -idb "std.sw.unix mr"

DIRS	=ftp tftp
BINSUID	=rsh
USRBINSUID=rlogin
NORM	=ruptime rwho telnet
TARGETS	=${BINSUID} ${NORM} ${USRBINSUID}

LCOPTS= -O
LCDEFS= -DUSG
LCINCS= -I${ROOT}/usr/include/bsd
LLDLIBS= -lsun -lbsd -ldbm

all default clean clobber install: bsd_$$@
	for i in ${DIRS}; do cd $$i; make $@; cd ..; done

bsd_all bsd_default:	${TARGETS} anything

bsd_install:	bsd_default
	${INSCMD} -m 4755 -F /bin "${BINSUID}"
	${INSCMD} -m 4755 -F /usr/bin "${USRBINSUID}"
	${INSCMD} -F /usr/bin "${NORM}"

bsd_clean:
	rm -rf *.o core a.out

bsd_clobber:	bsd_clean
	rm -rf ${TARGETS}

${TARGETS}: $$@.c
	${CC} ${CFLAGS} -o $@ $@.c ${LDFLAGS} 

anything:
