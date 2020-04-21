# makefile for usr.bin/calendar directory
#	$Header: /d2/3.7/src/usr.bin/calendar/RCS/Makefile,v 1.1 89/03/27 17:43:34 root Exp $

include ${ROOT}/usr/include/make/commondefs
INSCMD	=${INSTALL} -idb "std.sw.unix" -u bin -g bin

BINPROGS=calendar 
LIBPROGS=calprog calscript calnames
TARGETS	=${BINPROGS} ${LIBPROGS}
LLDLIBS	=-lsun -lbsd

all:	${TARGETS}

install:all
	${INSTALL} -F /usr/lib "${LIBPROGS}"
	${INSTALL} -F /usr/bin "${BINPROGS}"

clean:
	rm -f *.o

clobber:clean
	rm -f ${TARGETS}

.sh:
	cp $< $@
	chmod 555 $@

.c:
	${CCF} $< -o $@ ${LDFLAGS}
