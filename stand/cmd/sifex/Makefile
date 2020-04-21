#
#
include ${ROOT}/usr/include/make/commondefs
TESTER=
SYSTEM= -DSGI
CONTROLLER= -DSTORAGER
INCLDIR= $(ROOT)/usr/include/sys
CFLAGS= -O $(CONTROLLER) -Djuniper -DSTFEX $(SYSTEM) $(TESTER) -I../../include
LIBC= -lc
OBJECTS= sifex.o data.o exercise.o sets.o sisub.o misc.o netcopy.o conf.o \
	sterrs.o copy.o long.o setjmp.o dumpblk.o sitape.o sitapesub.o

SOURCES= sifex.c data.c exercise.c sets.c sisub.c misc.c sitape.c \
	sterrs.c copy.c long.s setjmp.c sitapesub.c netcopy.c conf.c \
	streg.h fex.h disk.h

COFLAGS= -l
CIFLAGS=
RCSDIR= ./RCS

LIBRARY= $(ROOT)/usr/lib
STANDLIB = $(LIBRARY)/libstand.a
PM2LIB = $(LIBRARY)/libpm2prom.a
GLXLIB = $(LIBRARY)/libglxprom.a
STARTUP = $(ROOT)/lib/crtstand.o
IP2LIBRARY= ../../lib/
IP2STANDLIB = $(IP2LIBRARY)/libstand.a
IP2LIBS = $(IP2STANDLIB) -lc
IP2STARTUP = $(IP2LIBRARY)/srt0.o


sifex: $(OBJECTS)
	@./Makeversion
	case $(machine) in \
		juniper)	 \
			$(LD) -o sifex -T 20000000 -X -x -e start $(IP2STARTUP) version.o $(OBJECTS) $(IP2LIBS) ;; \
		iris) \
			$(LD) -o sifex $(STARTUP) version.o $(OBJECTS) $(STANDLIB) $(GLXLIB) $(PM2LIB) -lc ;; \
	esac
	strip sifex
	@echo complete

install: sifex
	${INSTALL} -u bin -g bin -m 644 -F /stand \
	-idb "std.sw.unix mr" sifex

clean:
	rm -f *.o sifex makerrs

rcs:	
	for i in $(SOURCES); do \
		ci $$i $(RCSDIR)/$$i,v; \
		co $(COFLAGS) $$i; \
	done

sifex.o: $(INCLDIR)/dklabel.h streg.h disk.h fex.h sifex.c

exercise.o: disk.h streg.h $(INCLDIR)/dklabel.h fex.h

data.o:	$(INCLDIR)/dklabel.h disk.h streg.h data.c

sets.o:	$(INCLDIR)/dklabel.h streg.h disk.h fex.h sets.c

sisub.o: $(INCLDIR)/dklabel.h disk.h streg.h fex.h sisub.c sterrs.c

copy.o: $(INCLDIR)/dklabel.h streg.h disk.h fex.h copy.c

misc.o: streg.h disk.h fex.h misc.c

setjmp.o:
	ar xv /lib/libc.a setjmp.o
