#
# Makefile for SVR0 /bin/make.
#
include ${ROOT}/usr/include/make/commondefs

TARGETS= newmake
CFILES= doname.c dosys.c dyndep.c files.c main.c misc.c prtmem.c rules.c \
        gram.c
LCOPTS = -O -DMKDEBUG 

default: ${TARGETS}

include ${COMMONRULES}

${TARGETS}: ${OBJECTS}
	${CCF} ${OBJECTS} ${LDFLAGS}  -o ${TARGETS}

install: default
	${INSTALL} -src newmake -g bin -u bin -m 755 -o -idb "root upd" \
	    -F /bin make
