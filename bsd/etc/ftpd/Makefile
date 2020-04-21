#
#
include ${ROOT}/usr/include/make/commondefs
OBJS=	ftpd.o ftpcmd.o glob.o vers.o
SRCS=	$(OBJS:.o=.c)
TARGETS= ftpd

LCOPTS=	-O
LLDLIBS = -lsun -lbsd -ldbm -lcrypt
LCDEFS=	-DUSG
LCINCS=	-I${ROOT}/usr/include/bsd

all:	${TARGETS}

ftpd:	${OBJS}
	${CC} -o ftpd ${OBJS} ${LDFLAGS} 

vers.o:	ftpd.c ftpcmd.y glob.c newvers.sh $(FRC)
	sh newvers.sh
	${CC} -c vers.c

install:	all
	${INSTALL} -o -F /usr/etc \
	-idb "std.sw.unix mr" "${TARGETS}"

clean:
	rm -f *.o ftpcmd.c vers.c a.out core

clobber:	clean
	rm -f ${TARGETS}

ftpd.o:	ftpd.c $(FRC)
ftpcmd.o:ftpcmd.y $(FRC)
glob.o:	glob.c $(FRC)

FRC:	
