# Makefile for pm2 prom objects
#
include ${ROOT}/usr/include/make/commondefs
# must run ranlib afterwards!
.SUFFIXES: .c .do .h .no .o .s


#----- tool names -----
CC =		cc
AR =		ar
RANLIB =	ranlib


#----- parameters -----
LCINCS  =	-I../include
LCDEFS  =	-DMC68000 -DPM2 -DQPROM
LCOPTS  =	-O 


#----- peculiar defines -----
PM2LIB =	../libpm2prom.a

SRCS = \
	Qglobals.c \
	clockpm2.c \
	common.c \
	defaulterr.c \
	delay.c \
	duart.c \
	dutranslate.c \
	error.c \
	getput.c \
	keymap.c \
	lowpm2.s \
	magic.c \
	map.c \
	mbmalloc.c \
	mem.c \
	probe.c \
	qdevinit.c \
	rawgetput.c \
	ringbufs.c \
	sched.c \
	serialpm2.c \
	setkbd.c \
	slaveboot.c \
	spls.s \
	squeek.c \
	sum.c \
	wag.c

PM2OBJS = \
	Qglobals.o \
	clockpm2.o \
	common.o \
	defaulterr.o \
	delay.do \
	duart.o \
	dutranslate.o \
	error.o \
	getput.o \
	keymap.o \
	lowpm2.o \
	magic.o \
	map.o \
	mbmalloc.o \
	mem.o \
	probe.o \
	qdevinit.o \
	rawgetput.o \
	ringbufs.o \
	sched.o \
	serialpm2.o \
	setjmp.o \
	setkbd.o \
	slaveboot.o \
	spls.o \
	squeek.o \
	sum.o \
	wag.o


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
setjmp:
	oas setjmp.s -o setjmp.o


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
pm2: setjmp $(PM2OBJS) $(MAKEFILE)
	$(AR) r $(PM2LIB) $(PM2OBJS)
