# Makefile for monitor related objects
# NOT CHECKED IN TO RCS AS OF 3/28/1987....
#
include ${ROOT}/usr/include/make/commondefs
#
.SUFFIXES: .c .do .no .o .r .s


#----- tool names -----
AR =		ar
CC =		cc
RANLIB =	ranlib


#----- parameters -----
LCINCS  =	-I../include 
LCDEFS  =	-DMC68000 -DPM2 -DQPROM
LCOPTS  =	-O 
MAKEFILE =	Makefile


#----- peculiar definitions -----
LIBMON = ../libmon.a

HDRS = irisboot.h

CMD_SRCS = \
	adv_net.c \
	adv_snoop.c \
	adv_xx.c \
	advent.c \
	boot.c \
	bootsw.c \
	cmdsw.c \
	debug.c \
	debug_sub.c \
	download.c \
	help.c \
	ls.c \
	restart.c \
	swit.c \
	termulate.c

MON_SRCS = \
	main.c \
	monitor.c

CMD_OBJS = \
	adv_net.o \
	adv_snoop.o \
	adv_xx.o \
	advent.o \
	boot.o \
	bootsw.o \
	cmdsw.o \
	debug.o \
	debug_sub.o \
	download.o \
	help.o \
	ls.o \
	restart.o \
	swit.o \
	termulate.o

MON_OBJS = \
	main.o \
	monitor.o

OBJS = $(MON_OBJS) $(CMD_OBJS)

SRCS = $(MON_SRCS) $(CMD_SRCS)



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
all:	mon
	:
clean:
	-rm -f *.o *.do *.no *.r
new:	clean all
	:
release:
snapshot:
	$@ $(SRCS) $(HDRS) $(MAKEFILE)


#----- peculiar targets -----
mon:	$(LIBMON)
	:

$(LIBMON): $(OBJS) $(MAKEFILE)
	-rm -f $@
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@
