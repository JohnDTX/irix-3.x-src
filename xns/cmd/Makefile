#
#
include ${ROOT}/usr/include/make/commondefs
# Makefile for constructing the xns utilities
#
# $Source: /d2/3.7/src/xns/cmd/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 18:41:19 $
#
RMPROGS = xx xlogin xcp
PROGS	= xcp xx
LCDEFS	= -DSYSTEMV -DSYSTEM5
LIBS	= ../lib/libxns.a 
LLDLIBS	= -lsun -lbsd -ldbm
IDBTAG  = -idb "std.sw.unix"

all:	${PROGS}

install:all
	${INSTALL} -u bin -g bin -m 755 -F /bin ${IDBTAG} "${PROGS}"
	${INSTALL} -ln /bin/xx -F /bin ${IDBTAG} xlogin

clean:
	rm -f *.o a.out core

clobber:clean
	rm -f ${PROGS}

.c:
	${CCF} -o $@ $< ${LDFLAGS} $(LIBS)

.c.o:
	${CCF} -c $*.c

xcp:	xcp.o clstat.o
	${CCF} -o $@ xcp.o clstat.o ${LDFLAGS} $(LIBS)

xx:	xx.o
	${CCF} -o $@ xx.o ${LDFLAGS} $(LIBS)
