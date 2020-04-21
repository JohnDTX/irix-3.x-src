#
#
include ${ROOT}/usr/include/make/commondefs


# makefile for dc
# $Source: /d2/3.7/src/usr.bin/dc/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:45:05 $

all:	dc

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" dc

clean:
	rm -f dc.o

clobber:clean
	rm -f dc

dc:	dc.c dc.h
	$(CCF) -o $@ $@.c $(LDFLAGS) 
