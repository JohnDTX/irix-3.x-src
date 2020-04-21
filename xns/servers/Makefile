#
#
include ${ROOT}/usr/include/make/commondefs
# Makefile for the xns servers
#
LCDEFS	= -DSYSTEMV

PROGS	= sgboot sgbounce xnsd

LIBS	= ../lib/libxns.a
LLDLIBS	= -lsun -lbsd -ldbm
IDBTAG = -idb "std.sw.unix"

all:	$(PROGS)

install: all
	${INSTALL} -u root -g sys -m 755 -F /etc  ${IDBTAG} "${PROGS}"

clean:
	rm -f *.o a.out core

clobber: clean
	rm -f $(PROGS) *.o a.out core

.c:
	$(CCF) -o $@ $< $(LIBS) $(LDFLAGS) 
