DESTDIR=
AS= as
SYSTEM= SGI
#INCL= -I/usr/include -I../include
INCL= -I/usr/include
CFLAGS= -O -DIPFEX -D$(SYSTEM) -DVERBOSE $(INCL)
OBJECTS= ipfex.o data.o exercise.o sets.o copy.o ipsub.o \
	tape.o tapesub.o misc.o long.o setjmp.o
COFLAGS= -l
RCSDIR= ./RCS
TESTSRCS= Makefile copy.c data.c disk.h dsdreg.h exercise.c \
	iperrlist.h ipreg.h ipsub.c long.s ipfex.c misc.c \
	sets.c test.h tape.c tapesub.c types.h

LIBRARY= /usr/lib
STANDLIB = $(LIBRARY)/libstand.a
PM2LIB = $(LIBRARY)/libpm2prom.a
GLXLIB = $(LIBRARY)/libglxprom.a
STARTUP = /lib/crtstand.o

ipfex: $(OBJECTS)
	@Makeversion
	@echo "ipfex with DSD tape support"
	$(LD) -o $@ $(STARTUP) version.o $(OBJECTS) $(STANDLIB) $(GLXLIB) $(PM2LIB) -lc
	@echo complete

install: ipfex
	cp ipfex $(DESTDIR)/stand
	xcp ipfex olympus:/usr/local/boot/ipfex
	xcp ipfex olympus:/staff/chase/boot/ipfex

clean:
	rm -f *.o ipfex makerrs

rcs:	
	for i in $(TESTSRCS); do \
		ci $$i; \
		co $(COFLAGS) $$i; \
	done

ipfex.o: ipreg.h disk.h test.h ipfex.c

exercise.o: disk.h ipreg.h dsdreg.h test.h

tape.o: dsdreg.h disk.h

tapesub.o: disk.h dsdreg.h

data.o:	disk.h ipreg.h data.c

disk.o: disk.h ipreg.h test.h disk.c

sets.o:	ipreg.h disk.h test.h sets.c

ipsub.o: disk.h ipreg.h test.h iperrlist.h ipsub.c

copy.o: ipreg.h disk.h test.h copy.c

misc.o: ipreg.h disk.h test.h misc.c

setjmp.o:
	ar xv /lib/libc.a setjmp.o
