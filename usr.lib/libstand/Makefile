# makefile for standalone (sub-)library

#
#
include ${ROOT}/usr/include/make/commondefs

#----- tool names -----
AS =		as
AR =		ar
CC =		cc
LD =		ld
RANLIB =	ranlib


#----- parameters -----
MAKEFILE =	Makefile
LCINCS  = 	-I./include
LCDEFS  =	-DMC68000 -DPM2
LCOPTS  =	-O 

STANDLIB =	libstand.a
PM2LIB =	libpm2prom.a
GLXLIB =	libglxprom.a
LIBS =		$(STANDLIB) $(GLXLIB) $(PM2LIB)
CCSA =		$(LD) $(STARTUP)


#----- peculiar defines -----
STARTUP =	crtstand.o

SRCS = \
	cgetput.c \
	clearscreen.c \
	cstart.c \
	getld.s \
	defaulterr.c \
	getkbd.c \
	getput.c \
	rawgetput.c \
	setkbd.c
OBJS = \
	cgetput.o \
	clearscreen.o \
	cstart.o \
	getld.o \
	defaulterr.o \
	getkbd.o \
	getput.o \
	rawgetput.o \
	setkbd.o

#----- required targets -----
all: stand glxt
	:

stand: $(OBJS) crtstand.o $(MAKEFILE) sp_crtstand.o
	rm -f $(STANDLIB)
	$(AR) r $(STANDLIB) $(OBJS)
	$(RANLIB) $(STANDLIB)

install: stand
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" ${STANDLIB}
	ranlib ${ROOT}/usr/lib/${STANDLIB}
	${INSTALL} -u bin -g bin -m 644 -F /lib \
	-idb "std.sw.unix mr" crtstand.o
	${INSTALL} -u bin -g bin -m 644 -F /lib \
	-idb "std.sw.unix mr" sp_crtstand.o

clobber clean:
	rm -f *.[oa] ${STANDLIB}


#----- peculiar targets -----
glxt:	glxt.o loadck.o $(LIBS) $(MAKEFILE)
	$(CCSA) -o $@ glxt.o loadck.o $(LIBS) -lc
ab:	ab.o $(LIBS) $(MAKEFILE)
	$(CCSA) -o $@ ab.o $(LIBS) -lc
x:	x.o $(LIBS) $(MAKEFILE)
	$(CCSA) -o $@ x.o $(LIBS) -lc
