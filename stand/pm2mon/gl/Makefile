# Makefile for prom gl directory
#
include ${ROOT}/usr/include/make/commondefs
#
.SUFFIXES: .c .do .no .o .r .s


#----- tool names -----
CC =		cc
AR =		ar
RANLIB =	ranlib


#----- parameters -----
LCDEFS  =	-DMC68000 -DPM2 -DQPROM
LCINCS  =	-I../include
LCOPTS  =	-O 
MAKEFILE =	Makefile


#----- peculiar definitions -----
GLXLIB =	../libglxprom.a

GLXSRCS = \
	glbox.c \
	glchar.c \
	glclear.c \
	glcolor.c \
	glcurs.c \
	glfill.c \
	glinit.c \
	glxfont.c \
	term.c \
	screen.c scrinit.c
GLXOBJS = \
	glbox.o \
	glchar.o \
	glclear.o \
	glcolor.o \
	glcurs.o \
	glfill.o \
	glinit.o \
	glxfont.o \
	term.o \
	screen.o scrinit.o
GL1SRCS =	gl1gl.c
GL2SRCS =	gl2gl.c
GL1OBJS =	gl1gl.o
GL2OBJS=	gl2gl.o
GL1HDRS = \
	gl1cmds.h \
	gl1dcdev.h \
	gl1gfdev.h \
	gl1screen.h \
	gl1uctest.h
GL2HDRS = \
	gl2cmds.h \
	gl2dcdev.h \
	gl2gfdev.h \
	gl2screen.h \
	gl2uc4.h \
	gl2uctest.h
GLXHDR =	glx.h

SYSOBJS = \
	$(GLXOBJS) $(GL1OBJS) $(GL2OBJS)
SRCS = \
	$(GLXSRCS) $(GL1SRCS) $(GL2SRCS)
HDRS = \
	$(GL1HDRS) $(GL2HDRS) $(GLXHDR)


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
$(SYSOBJS): $(GLXHDR)
all: glx
	:
clean:  
	-rm -f *.o *.do *.no *.r
new:	clean all 
	:
release:
snapshot:
	$@ $(SRCS) $(HDRS) $(MAKEFILE)



#----- peculiar targets -----
glx:	$(GLXLIB)
	:
$(GLXLIB): $(SYSOBJS) $(MAKEFILE) $(GLXHDR)
	-rm -f $@
	$(AR) r $@ $(SYSOBJS)
	$(RANLIB) $@
