#
# $Source: /d2/3.7/src/stand/mon/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 17:15:31 $
#
#
# makefile for various 1 shot proms
#
#

#
# Common definitions
#

# Path definitions
INCL = $(DEST)/usr/include
LN = /bin/ln
MV = /bin/mv

# Macro definitions
AR = /bin/ar r
ECHO = @/bin/echo

# Installation directories
BINDIR  = $(DESTDIR)/bin
UBINDIR = $(DESTDIR)/usr/bin
ETCDIR  = $(DESTDIR)/etc
LIBDIR  = $(DESTDIR)/lib
ULIBDIR = $(DESTDIR)/usr/lib
UMAN    = $(DESTDIR)/usr/man
UINCL   = $(DESTDIR)/usr/include
UGAMES  = $(DESTDIR)/usr/games

#
# Local definitions
#
OFILES = main.o loadobj.o parse.o subr.o init.o help.o \
	 version.o strs.o set.o ls.o
SFILES = locore.c locore_mem.c main.c loadobj.c parse.c subr.c init.c help.c \
	 version.c strs.c
HFILES = help.h parse.h strs.h

CFLAGS = -OS -DDEBUG

LOCORE = -DTESTING

LINCL  = ../include

TOOLS  = ../../tools

LD     = ld.new

AS     = as20

LLIB   = ../lib/libstand.a
CRT0 = ../lib/srt0.o

LIBS   = $(LLIB) -lc

#
# Rules follow
#

.c.o:
	$(CC) $(CFLAGS) -I$(LINCL) -c $<

.c.s:
	$(CC) $(CFLAGS) -S -I$(LINCL) -c $<

.c.i:
	$(CC) $(CFLAGS) -P -I$(LINCL) -c $<

all: ip2prom

install:	all
	:

world: ip2prom_mem high_mem  ip2prom

mem: ip2prom_mem high_mem higher_mem

clean clobber:
	rm -f *.o ip2prom ip2prom? high_mem ip2prom_mem ip2prom.nm higher_mem

erase:
	rm -f $(SFILES) $(HFILES)

# with this prom bss starts at megabyte 32
ip2prom:	locore.o $(OFILES) $(LLIB)
	$(LD) -x -X -LT 30000000  -LC 2100000 \
	locore.o $(OFILES) $(LIBS) -o $@.nm
	cp $@.nm junk
	strip junk
	dd if=junk of=$@ skip=1 bs=32
	dd if=$@ of=$@0 count=32 bs=1024
	dd if=$@ of=$@1 skip=32 count=32 bs=1024
	dd if=$@ of=$@2 skip=64 count=32 bs=1024
	dd if=$@ of=$@3 skip=96 count=32 bs=1024
	rm junk
	$(ECHO) $@ ready

# this prom is intended to run as another bootable program
ip2prom_mem:	locore_mem.o $(OFILES) $(LLIB)
	ld -T 20000000 -x -X -e start \
	locore_mem.o $(OFILES) $(LIBS) -o $@
	cp $@ $(DEST)/stand/sprom
	$(ECHO) $@ ready

# this prom is intended to run as another bootable program high up
high_mem:	locore_mem.o $(OFILES) $(LLIB)
	ld -T 20050000 -x -X -e start \
	locore_mem.o $(OFILES) $(LIBS) -o $@
	cp $@ $(DEST)/stand/sprom2
	$(ECHO) $@ ready

# this prom is intended to run as another bootable program even higher up
higher_mem:	locore_mem.o $(OFILES) $(LLIB)
	ld -T 200f0000 -x -X -e start \
	locore_mem.o $(OFILES) $(LIBS) -o $@
	cp $@ $(DEST)/stand/sprom3
	$(ECHO) $@ ready

hello:	hello.o
	ld -T 20010000 -x -X \
	hello.o $(LLIB) -lc -o $@
	cp $@ /stand
	$(ECHO) $@ ready

mem.nm:	ip2prom_mem
	nm -ng ip2prom_mem > $@

offset.h: $(LINCL)/common.h genoffset.c
	$(HOSTCC) cc -I$(LINCL) genoffset.c
	a.out > offset.h
	rm -f a.out

# Header file dependancies

help.o:	$(INCL)/sys/types.h strs.h parse.h

init.o:	$(INCL)/sys/types.h $(LINCL)/cpureg.h $(LINCL)/common.h $(INCL)/termio.h

loadobj.o:	$(INCL)/sys/types.h $(LINCL)/cpureg.h $(LINCL)/common.h $(INCL)/errno.h $(INCL)/a.out.h $(LINCL)/sysmacros.h $(LINCL)/dprintf.h

locore.o:	$(LINCL)/cpureg.h $(LINCL)/sr.h $(LINCL)/evec.h offset.h
	cc $(LOCORE) -DLOCORE -I$(LINCL) -E locore.c | egrep -v '^#' > locore.i
	$(AS) -o $@ locore.i
	rm -f locore.i

locore_mem.o:	$(LINCL)/cpureg.h $(LINCL)/sr.h $(LINCL)/evec.h offset.h
	cc $(LOCORE) -DLOCORE -I$(LINCL) -E locore_mem.c | egrep -v '^#' > locore_mem.i
	$(AS) -o $@ locore_mem.i
	rm -f locore_mem.i

main.o:	$(INCL)/sys/types.h $(LINCL)/cpureg.h $(LINCL)/common.h strs.h parse.h $(LINCL)/mbenv.h $(LINCL)/sysmacros.h $(INCL)/setjmp.h $(LINCL)/dprintf.h

parse.o:	$(INCL)/sys/types.h $(LINCL)/cpureg.h $(LINCL)/common.h parse.h strs.h


set.o:	$(INCL)/sys/types.h $(INCL)/setjmp.h $(LINCL)/cpureg.h $(LINCL)/common.h $(INCL)/ctype.h parse.h strs.h $(LINCL)/tod.h $(LINCL)/dprintf.h

subr.o:	$(INCL)/sys/types.h $(INCL)/setjmp.h $(LINCL)/cpureg.h $(LINCL)/trap.h $(LINCL)/common.h $(INCL)/ctype.h parse.h strs.h
