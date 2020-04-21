#
#
include ${ROOT}/usr/include/make/commondefs
SYSTEM= SGI
LCINCS= -I. -I../../include -I${ROOT}/usr/include
LCOPTS= -O -DMDFEX -D$(SYSTEM) -DVERBOSE -DEFS -Djuniper
OBJECTS= mdfex.o data.o exercise.o sets.o copy.o disksub.o \
	tape.o tapesub.o misc.o long.o conf.o netcopy.o \
	memtest.o  setjmp.o kill.o

CIFLAGS= -l
RCSDIR= ./RCS
TESTSRCS= copy.c data.c exercise.c disksub.c mdfex.c misc.c \
	sets.c tape.c tapesub.c memtest.c probe.c long.s\
	dsdreg.h Makefile disk.h kill.c

CFILES= conf.c copy.c data.c disksub.c exercise.c mdfex.c \
	memtest.c misc.c netcopy.c probe.c sets.c tape.c tapesub.c
HFILES= date.h disk.h dsdreg.h fex.h 

LIBRARY= $(ROOT)/usr/lib
STANDLIB = $(LIBRARY)/libstand.a
PM2LIB = $(LIBRARY)/libpm2prom.a
GLXLIB = $(LIBRARY)/libglxprom.a
STARTUP = $(ROOT)/lib/crtstand.o
IP2LIBRARY= ../../lib/
IP2STANDLIB = $(IP2LIBRARY)/libstand.a
IP2LIBS=	$(IP2STANDLIB) -lc
IP2STARTUP = $(IP2LIBRARY)/srt0.o

mdfex: $(OBJECTS)
	@./Makeversion
	case $(machine) in \
		juniper)	 \
			$(LD) -o $@ -T 20000000 -X -x -e start $(IP2STARTUP) version.o $(OBJECTS) $(IP2LIBS) ;; \
		iris) \
			$(LD) -o $@ $(STARTUP) version.o $(OBJECTS) $(STANDLIB) $(GLXLIB) $(PM2LIB) -lc ;; \
	esac
	strip $@
	@echo complete

install: mdfex
	${INSTALL} -u bin -g bin -m 644 -F /stand \
	-idb "std.sw.unix mr" mdfex

clean:
	rm -f *.o

clobber: clean
	rm -rf mdfex makerrs version.c

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
