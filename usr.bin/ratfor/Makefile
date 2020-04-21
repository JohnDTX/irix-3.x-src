# Makefile for ratfor
#
#
include ${ROOT}/usr/include/make/commondefs

# @(#)$Header: /d2/3.7/src/usr.bin/ratfor/RCS/Makefile,v 1.1 89/03/27 18:21:18 root Exp $

YACCRM	= rm
OFILES	= r0.o r1.o r2.o rio.o rlook.o rlex.o

all:	ratfor

ratfor:$(OFILES) r.g.o
	$(CC) $(OFILES) r.g.o $(LDFLAGS) -o ratfor -ly

$(OFILES):r.h r.g.h

r.g.c r.g.h:r.g.y
	$(YACC) -d r.g.y
	mv y.tab.c r.g.c
	mv y.tab.h r.g.h

test:
	rtest ratfor

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" ratfor

clean:
	rm -f *.o
	$(YACCRM) -f r.g.c r.g.h

clobber:clean
	rm -f ratfor
