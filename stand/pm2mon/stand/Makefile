#
#
include ${ROOT}/usr/include/make/commondefs


# makefile for standalone (sub-)library

#----- tool names -----
AR =		ar
CC =		cc
LD =		ld
RANLIB =	ranlib


#----- parameters -----
LCINCS  =	-I../include -I${ROOT}/usr/include
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

stand: $(OBJS) crtstand.o $(MAKEFILE)
	rm -f $(STANDLIB)
	$(AR) r $(STANDLIB) $(OBJS)
	$(RANLIB) $(STANDLIB)

install: stand
	${INSTALL} -F /usr/lib \
	-idb "std.sw.unix mr" "${STANDLIB}"
	${INSTALL} -F /usr/lib \
	-idb "std.sw.unix mr" crtstand.o

clean:
	rm -f *.[oa] glxt


#----- peculiar targets -----
glxt:	glxt.o loadck.o $(LIBS) $(MAKEFILE)
	$(CCSA) -o $@ glxt.o loadck.o $(LIBS) -lc
ab:	ab.o $(LIBS) $(MAKEFILE)
	$(CCSA) -o $@ ab.o $(LIBS) -lc
x:	x.o $(LIBS) $(MAKEFILE)
	$(CCSA) -o $@ x.o $(LIBS) -lc
