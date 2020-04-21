# make Sun commands from /bin
# "$Header: /d2/3.7/src/sun/bin/RCS/Makefile,v 1.1 89/03/27 17:21:09 root Exp $"

include ${ROOT}/usr/include/make/commondefs

LCOPTS= -O 
LCINCS=-I${INCLDLR}/bsd
LLDLIBS=-lsun -lbsd
TARGETS= domainname 
MAKEPATH= .:RCS

all: ${TARGETS}

${TARGETS}: $$@.c
	$(CCF) -o $@ $@.c $(LDFLAGS)

install:	all
	${INSTALL} -u bin -g bin -F /usr/bin -idb "std.sw.unix" "${TARGETS}"

clean:
	rm -rf *.o a.out core
clobber:clean
	rm -f ${TARGETS} 

tags: 
	ctags ${SRCS}

fluff: ${SRCS} 
	lint -Dlint ${CFLAGS} ${SRCS} > fluff
