#
include ${ROOT}/usr/include/make/commondefs

# Makefile for constructing the xns tools and servers
#
# $Source: /d2/3.7/src/xns/trace/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 18:41:55 $
#
PROGS	= xstat
LCDEFS	= -DSYSTEMV
IDBTAG  = -idb "std.sw.unix"

all:	${PROGS}

install: all
	${INSTALL} -u bin -g bin -m 755 -F /etc ${IDBTAG}  "${PROGS}"

clean:
	rm -f *.o a.out core

clobber:clean
	rm -f ${PROGS}

.c:
	${CCF} -o $@ $< $(LDFLAGS) ../lib/libxns.a
