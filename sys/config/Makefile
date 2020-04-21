#
#
include ${ROOT}/usr/include/make/commondefs

# $Header: /d2/3.7/src/sys/config/RCS/Makefile,v 1.1 89/03/27 17:27:27 root Exp $


CSRC	= main.c mkioconf.c mkmakefile.c mkubglue.c mkheaders.c \
		mkswapconf.c sgiglue.c
YSRC	= config.y
LSRC	= token.l
SRC	= ${CSRC} ${YSRC} ${LSRC} Makefile
CFILES	= ${CSRC} ${YSRC:.y=.c} ${LSRC:.l=.c}
OBJS	= ${CSRC:.c=.o} ${YSRC:.y=.o} ${LSRC:.l=.o}

YFLAGS	=-d
LLDLIBS	=-ll

all:	config

config: ${OBJS}
	${CCF} ${OBJS} -o $@ ${LDFLAGS}

lex.yy.c:config.l
	lex config.l

y.tab.h:${YSRC:.y=.c}
	@#do nothing

clean:
	rm -f ${YSRC:.y=.c} ${LSRC:.l=.c}
	rm -f ${OBJS} *.o lex.yy.c y.tab.c y.tab.h tags a.out core
	
clobber:clean
	rm -f config

install:all
	${INSTALL} -idb "sys" -u root -g sys -dir /usr/sys/config
	${INSTALL} -idb "sys" -u root -g sys -F /usr/sys/config "${SRC}"

lint:	${CFILES}
	lint ${CFILES}

y.tab.o:	config.h
main.o:		y.tab.h config.h
lex.yy.o:	y.tab.h config.h
mkioconf.o:	y.tab.h config.h
mkmakefile.o:	y.tab.h config.h
mkubglue.o:	config.h y.tab.h
mkheaders.o:	config.h y.tab.h
mkswapconf.o:	config.h
sgiglue.o:	config.h y.tab.h
