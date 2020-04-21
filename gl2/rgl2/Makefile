##########################################################################
#									 #
# 		 Copyright (C) 1983, Silicon Graphics, Inc.		 #
#									 #
#  These coded instructions, statements, and computer programs  contain  #
#  unpublished  proprietary  information of Silicon Graphics, Inc., and  #
#  are protected by Federal copyright law.  They  may  not be disclosed  #
#  to  third  parties  or copied or duplicated in any form, in whole or  #
#  in part, without the prior written consent of Silicon Graphics, Inc.  #
#									 #
##########################################################################

#
#	Makefile	This file
#
#	lib.c		C library source after preprocessing
#	io.c		C io source
#	librgl2.a	C remote graphics library	
#	rlibgen.awk	Awk script for making lib.c from lib.prim
#	lib.prim	Specs file for graphics primitives
#
include ${ROOT}/usr/include/make/commondefs

VCFLAGS = -O -DSYSTEM5 -DMC68000
SRC = rlibgen.awk io.c decl.h
LIBPRIM = $(ROOT)/usr/lib/$(GLHDWR)/lib.prim

all: librgl2.a librgl2b.a librgl2dbg.a

librgl2.a: lib.c lib/lastdone io.o 
	rm -f librgl2.a
	cd lib; ar cr ../librgl2.a *.o ../io.o
	strip librgl2.a
	ranlib librgl2.a

# 'big' librgl2.a -- made quickly without the splitter
librgl2b.a: lib.o io.o
	ar cr librgl2b.a lib.o io.o
	ranlib librgl2b.a

# 'debug' librgl2.a -- prints out a listing of all graphics i/o into the file
#   protsave (or into $PROTSAVE if $PROTSAVE is defined in the environment)
librgl2dbg.a: lib.o iodbg.o
	ar cr librgl2dbg.a lib.o iodbg.o
	ranlib librgl2dbg.a

io.o: rpc.h
lib.o: decl.h 

iodbg.o: io.c 
	rm -f iodbg.c; ln io.c iodbg.c
	cc -c $(CFLAGS) -DIO_DEBUG iodbg.c
	rm -f iodbg.c

lib.c mandefs.cr: rlibgen.awk $(LIBPRIM)
	awk -f rlibgen.awk $(LIBPRIM)

rpc.h: $(ROOT)/usr/include/$(GLHDWR)/rpc.h
	cp $(ROOT)/usr/include/$(GLHDWR)/rpc.h .

lib/lastdone: lib.c 
	rm -rf lib; mkdir lib
	ex - lib.c < rsplit.ex
	cd lib; $(MAKE) -f ../lib.Makefile "SRCS=`echo *.c`"

install: librgl2.a
	${INSTALL} -m 644 -g bin -u bin -idb "gifts" -F /usr/lib librgl2.a
	ranlib ${ROOT}/usr/lib/librgl2.a

clobber clean: 
	rm -f *.o *.a lib.c splitter.junk iodbg.c mandefs.cr rpc.h
	rm -rf lib
