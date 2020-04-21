# Makefile for gen prom objects
#
include ${ROOT}/usr/include/make/commondefs

# must run ranlib afterwards!
.SUFFIXES: .c .do .no .o .r .s


#----- tool names -----
CC =		cc
AR =		ar
RANLIB =	ranlib


#----- parameters -----
LCINCS  =	-I../include
LCDEFS  =	-DMC68000 -DPM2 -DQPROM
LCOPTS =	-O 


#----- peculiar defines -----
PM2LIB =	../libpm2prom.a

SRCS = \
	Launch.s \
	appendstr.c \
	argcnt.c \
	awful.c \
	beep.c \
	drboot.c \
	dummy.c \
	getnline.c \
	getstr.c \
	gmalloc.c \
	illegal.c \
	isnum.c \
	joinspec.c \
	memread.c \
	memwrite.c \
	newline.c \
	numargs.c \
	panic.c \
	prf.c \
	readargs.c \
	sbrk.c \
	splitspec.c \
	spystr.c \
	strlcpy.c \
	warmboot.c \
	wdumpmem.c
GENOBJS = \
	Launch.do \
	appendstr.do \
	argcnt.do \
	awful.do \
	beep.do \
	drboot.do \
	dummy.do \
	getnline.do \
	getstr.do \
	gmalloc.do \
	illegal.do \
	isnum.do \
	joinspec.do \
	memread.do \
	memwrite.do \
	newline.do \
	numargs.do \
	panic.do \
	prf.do \
	readargs.do \
	sbrk.do \
	splitspec.do \
	spystr.do \
	strlcpy.do \
	warmboot.do \
	wdumpmem.do


#----- default rules -----
default: all
	:
.c.o:
	$(CC) -c $(CFLAGS) $*.c
.c.do:
	$(CC) -S $(CFLAGS) $*.c 
	mv $*.s temp
	sed 's/\.text/.data/' <temp > $*.s
	$(AS) $*.s -o $*.o
	-rm -f $*.s temp
	mv $*.o $*.do
.c.no:
	$(CC) -c $(DEFINES) $(INCLUDES) $*.c
	mv $*.o $*.no
.c.s:
	$(CC) -S $(CFLAGS) $*.c
.s.o:	
	$(AS) $*.s -o $*.o
.s.do:
	$(AS) $*.s -o $*.o
	mv $*.o $*.do


#----- required targets -----
all:	pm2
	:
install:
	:
clean:
	-rm -f *.o *.do *.no *.r
new: clean all
	:
snapshot:
release:
	$@ $(SRCS) $(MAKEFILE)



#----- peculiar targets -----
pm2: $(GENOBJS) $(MAKEFILE)
	$(AR) r $(PM2LIB) $(GENOBJS)
