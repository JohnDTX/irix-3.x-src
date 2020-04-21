# makefile for /usr/bin/bc

#
#
include ${ROOT}/usr/include/make/commondefs
# $Source: /d2/3.7/src/usr.bin/bc/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:43:21 $

all:	bc

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" bc

clean:
	rm -f *.o

clobber:clean
	rm -f bc

bc:	$$@.y
	yacc $?
	$(CCF) -o $@ y.tab.c $(LDFLAGS) 
	rm -f y.tab.c

.sh:
	${INSTALL} -sh $<.sh -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" $@

