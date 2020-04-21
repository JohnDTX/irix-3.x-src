#
# Makefile for IRIS 2-3K mkfs(1m).
#
include ${ROOT}/usr/include/make/commondefs

LCINCS=		-ZN -I../toyfs 
LCDEFS= 	-DSYSTEM5
TOYLIBDIR=	../toyfs
TOYLIB=		${TOYLIBDIR}/libtoyfs.a

COMMONCFILES=	efs_mkfs.c getdklabel.c
CFILES=		${COMMONCFILES} mkfs.c
COMMONOBJS=	${COMMONCFILES:.c=.o}
STDTARGETS=	mkfs
OPTTARGETS=	mkfs1b mkfs2b mkfs4b
TARGETS=	${STDTARGETS} ${OPTTARGETS}
IDBTAG=		-idb "std.sw.unix mr"

all:	${STDTARGETS}

include ${COMMONRULES}

install:all
	${INSTALL} ${IDBTAG} -u root -g sys -F /etc mkfs
	${INSTALL} ${IDBTAG} -ln /etc/mkfs -F /etc mkfs.bell

mkfs:	mkfs2b
	-mv -f mkfs2b $@

${OPTTARGETS}: $$@.o ${COMMONOBJS} ${TOYLIB}
	${CCF} ${LDFLAGS} $@.o ${COMMONOBJS} ${TOYLIB} -o $@

mkfs1b.o: mkfs.c
	${CCF} -c -DFsTYPE=1 mkfs.c -o $@

mkfs2b.o: mkfs.c
	${CCF} -c -DFsTYPE=2 mkfs.c -o $@

mkfs4b.o: mkfs.c
	${CCF} -c -DFsTYPE=4 mkfs.c -o $@
