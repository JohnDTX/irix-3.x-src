#AS= as -i
AS= as
DESTDIR=
SYSTEM= SGI
INCL= -I.
CFLAGS= -O -DMDFEX -D$(SYSTEM) $(INCL) -DVERBOSE
OBJECTS= mdfex.o data.o exercise.o sets.o copy.o disksub.o \
	tape.o tapesub.o misc.o long.o \
	memtest.o kill.o

CIFLAGS= -l
RCSDIR= ./RCS
TESTSRCS= copy.c data.c exercise.c disksub.c mdfex.c misc.c \
	sets.c tape.c tapesub.c memtest.c probe.c long.s\
	dsdreg.h Makefile disk.h kill.c

CFILES= copy.c data.c disksub.c exercise.c mdfex.c \
	memtest.c misc.c probe.c sets.c tape.c tapesub.c
HFILES= date.h disk.h dsdreg.h fex.h 

LIBRARY= /usr/lib
STANDLIB = $(LIBRARY)/libstand.a
PM2LIB = $(LIBRARY)/libpm2prom.a
GLXLIB = $(LIBRARY)/libglxprom.a
STARTUP = /lib/crtstand.o
#STARTUP = $(LIBRARY)/sp_crtstand.o

mdfex: $(OBJECTS)
	@Makeversion
	$(LD) -o $@ $(STARTUP) version.o $(OBJECTS) $(STANDLIB) $(GLXLIB) $(PM2LIB) -lc
	@echo complete

install: mdfex
	cp mdfex $(DESTDIR)/stand
	xcp mdfex olympus:/usr/local/boot/mdfex
	xcp mdfex olympus:/staff/chase/boot/mdfex

clean:
	rm -f *.o mdfex makerrs

Crcs:	
	for i in $(CFILES); do \
		ci $$i ;\
		co -l $$i; \
	done

Hrcs:	
	for i in $(HFILES); do \
		ci $$i ;\
		co -l $$i; \
	done

nm:	
	for i in $(OBJECTS); do \
		nm -g $$i > $$i.nm; \
	done

fex.o:	dsdreg.h disk.h fex.c

exercise.o: disk.h dsdreg.h dsdreg.h

tape.o: dsdreg.h disk.h

tapesub.o: disk.h dsdreg.h

data.o:	disk.h dsdreg.h data.c

disk.o: disk.h dsdreg.h disk.c

sets.o:	dsdreg.h disk.h sets.c

disksub.o: disk.h dsdreg.h disksub.c

copy.o: dsdreg.h disk.h copy.c

misc.o: dsdreg.h disk.h misc.c

setjmp.o:
	ar xv /lib/libc.a setjmp.o
