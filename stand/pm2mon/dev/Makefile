# Makefile for disk boot related objects
# must run ranlib afterwards!
#
include ${ROOT}/usr/include/make/commondefs
#
.SUFFIXES: .c .do .no .o .r .s


#----- tool names -----
AR =		ar
CC =		cc


#----- parameters -----
LCINCS  =	-I../include 
LCDEFS  =	-DMC68000 -DPM2 -DQPROM
LCOPTS  =	-O 
MAKEFILE =	Makefile


#----- peculiar defines -----
PM2LIB =	../libpm2prom.a
IO_SRCS = \
	i488.c \
	ikon.c \
	iprblk.c \
	mdrblk.c \
	nx.c \
	tapesub.c

PROTO_SRCS = \
	bell_file.c \
	bky_file.c \
	bootstr.c \
	cpio_sub.c \
	diskboot.c \
	efs_file.c \
	enet.c \
	file_sub.c \
	i488boot.c \
	ipboot.c \
	mdboot.c \
	memboot.c \
	mfboot.c \
	mmemboot.c \
	netboot.c \
	seq.c \
	serialio.c \
	tapeboot.c

IO_OBJS = \
	i488.do \
	ikon.do \
	iprblk.do \
	mdrblk.do \
	nx.do \
	tapesub.do

PROTO_OBJS = \
	bell_file.do \
	bky_file.do \
	bootstr.do \
	cpio_sub.do \
	diskboot.do \
	efs_file.o \
	enet.do \
	file_sub.do \
	i488boot.do \
	ipboot.do \
	mdboot.do \
	memboot.do \
	mfboot.do \
	mmemboot.do \
	netboot.do \
	seq.do \
	serialio.do \
	tapeboot.do

HDRS = \
	bell_dir.h \
	bell_filsys.h \
	bell_fs.h \
	bell_ino.h \
	bell_inode.h \
	bky_dir.h \
	bky_fs.h \
	bky_inode.h \
	bky_param.h \
	bky_types.h \
	cpiohdr.h \
	dklabel.h \
	dsdreg.h \
	ipreg.h \
	mdrblk.h \
	net.h \
	nx.h \
	safs.h

OBJS =	$(IO_OBJS) $(PROTO_OBJS)
SRCS =	$(IO_SRCS) $(PROTO_SRCS)


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
all:	dev
	:
clean:
	-rm -f *.o *.do *.no *.r
new: clean all
	:
release:
snapshot:
	$@ $(SRCS) $(HDRS) $(MAKEFILE)


#----- peculiar targets -----
dev:	$(OBJS) $(MAKEFILE)
	$(AR) r $(PM2LIB) $(OBJS)
