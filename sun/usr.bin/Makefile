#
# "$Header: /d2/3.7/src/sun/usr.bin/RCS/Makefile,v 1.1 89/03/27 17:22:33 root Exp $"

include ${ROOT}/usr/include/make/commondefs

LCINCS=	-I$(ROOT)/usr/include/bsd
LCOPTS=	-O 
LLDLIBS= -lsun -lbsd -ldbm -lrpcsvc
PLAIN=	ypcat ypwhich ypmatch
TARGETS= $(PLAIN) yppasswd

all:	$(TARGETS)

install:	all
	${INSTALL} -u bin -g bin -F /usr/bin -idb "std.sw.unix" "${TARGETS}"

clean:
	rm -rf *.o a.out core
clobber:clean
	rm -rf $(TARGETS)

$(PLAIN):	$$@.c
	$(CCF) -o $@ $@.c $(LDFLAGS)

yppasswd:	yppasswd.c
	$(CCF) -o $@ $@.c $(LDFLAGS) -lcrypt
