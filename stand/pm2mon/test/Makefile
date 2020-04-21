# Makefile for pm2 proms

#
include ${ROOT}/usr/include/make/commondefs
#
.SUFFIXES: .c .o .r .s .no .ls .ci .rlog .c,v


#----- tool names -----
CC =		cc
AS =		as
LD =		ld
ADB =		adb
RANLIB =	ranlib
NM =		nm
STRIP =		strip
REV =		rev
MAKEBOOT =	makeboot


#----- parameters -----
LCINCS  =	-I../include
LCDEFS  =	-DMC68000 -DPM2 -DQPROM
LCOPTS  =	-O 
MAKEFILE =	Makefile
BOOTDIR =	/usr/tmp


#----- peculiar defines -----
PM2LIB =	../libpm2prom.a
GLXLIB =	../libglxprom.a
STANDLIBS =	$(GLXLIB) $(PM2LIB)


#----- default rules -----
default: all
	:
.c.ls:
	$(CC) -L $(DEFINES) $(INCLUDES) $<
.c.o:
	$(CC) -c $(CFLAGS) $*.c
.c.no:
	$(CC) -c $(DEFINES) $(INCLUDES) $*.c
	mv $*.o $*.no


#----- required targets -----
all:	s t
	:
new:	clean all
	:
clean:
	-rm -f *.o *.r *.no *.ls *.nm a.*
install:
	:
release:
snapshot:
	cd include; make $@
	$@ $(SRCS) $(MAKEFILE)


#----- peculiar targets -----
t:	t.o
	$(CC) -T 80000 -o $@ $@.o \
		$(STANDLIBS) -lc
	$(MAKEBOOT) -l 80000 $@ $(BOOTDIR)/$@
s:	s.o
	$(CC) -s -T 40000 -o $@ $@.o \
		$(STANDLIBS) $(STANDLIBS) -lc
	$(MAKEBOOT) -l 40000 $@ $(BOOTDIR)/$@
