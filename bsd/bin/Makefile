# "$Header: /d2/3.7/src/bsd/bin/RCS/Makefile,v 1.1 89/03/27 14:58:13 root Exp $"
#
include ${ROOT}/usr/include/make/commondefs

BINLIST= hostname
USRBINLIST= hostid pagesize
TARGETS= $(BINLIST) $(USRBINLIST) rcp

IDBTAG	=-idb "std.sw.unix mr" 
INSCMD	=${INSTALL} ${IDBTAG} -u root -g sys

MAKEPATH=	.:RCS

LCOPTS= -O 
LCDEFS= -DUSG 
LLDLIBS= -lsun -lbsd -ldbm 
LCINCS = -I${ROOT}/usr/include/bsd

all:	$(TARGETS)

$(TARGETS):	$$@.c
	$(CCF) -o $@ $@.c $(LDFLAGS) 

install:	all
	${INSCMD} -F /bin "${BINLIST}"
	${INSCMD} -m 4755 -F /bin rcp
	${INSCMD} -F /usr/bin "${USRBINLIST}"

clean:
	rm -f *.o a.out core
    
clobber:	clean
	rm -f $(TARGETS)
