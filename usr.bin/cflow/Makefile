

#
#
include ${ROOT}/usr/include/make/commondefs

# makefile for cflow
# $Source: /d2/3.7/src/usr.bin/cflow/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:43:40 $

FILES	= makefile README cflow.sh dag.c lpfx.c nmf.c flip.c
PROGS	= dag lpfx nmf flip cflow
LINT	= ../lint
MIP	= ../../lib/ccom


all:	$(PROGS)

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" cflow
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" "  dag lpfx nmf flip "

lpfx.o:	lpfx.c $(LINT)/lmanifest \
	  $(MIP)/manifest.h
	$(CCF) -c $(CFLAGS) -I$(LINT) -I$(MIP) lpfx.c

lpfx:	$$@.o
	$(CCF) $(LDFLAGS) -o $@ $?

clean:
	rm -f *.o *.out core

clobber:clean
	rm -f $(PROGS)
