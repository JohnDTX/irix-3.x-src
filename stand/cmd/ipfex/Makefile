#
#
include ${ROOT}/usr/include/make/commondefs
AS= as
#INCL= -I/usr/include -I../include
LCINCS= -I../../include -I/usr/include
LCOPTS= -O -DIPFEX -Djuniper -UPMII -DEFS
OBJECTS= ipfex.o data.o exercise.o sets.o copy.o ipsub.o \
	tape.o tapesub.o misc.o long.o setjmp.o conf.o netcopy.o
COFLAGS= -l
RCSDIR= ./RCS
TESTSRCS= Makefile conf.c copy.c data.c disk.h dsdreg.h exercise.c \
	iperrlist.h ipreg.h ipsub.c long.s ipfex.c misc.c \
	netcopy.c sets.c test.h tape.c tapesub.c types.h

LIBRARY= $(ROOT)/usr/lib
STANDLIB = $(LIBRARY)/libstand.a
PM2LIB = $(LIBRARY)/libpm2prom.a
GLXLIB = $(LIBRARY)/libglxprom.a
STARTUP = $(ROOT)/lib/crtstand.o
IP2LIBRARY= ../../lib/
IP2STANDLIB = $(IP2LIBRARY)/libstand.a
IP2LIBS = $(IP2STANDLIB) -lc
IP2STARTUP = $(IP2LIBRARY)/srt0.o

ipfex: $(OBJECTS)
	@./Makeversion
	@echo "ipfex with DSD tape support"
	case $(machine) in \
		juniper)	 \
			$(LD) -o ipfex -T 20000000 -X -x -e start $(IP2STARTUP) version.o $(OBJECTS) $(IP2LIBS) ;; \
		iris) \
			$(LD) -o $@ $(STARTUP) version.o $(OBJECTS) $(STANDLIB) $(GLXLIB) $(PM2LIB) -lc ;; \
	esac
	strip $@
	@echo complete

install: ipfex
	${INSTALL} -u bin -g bin -m 644 -F /stand \
	-idb "std.sw.unix mr" ipfex

oly:
	xcp -v ipfex olympus:/usr/local/boot/ipfex

clean:
	rm -f *.o ipfex makerrs version.c

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
