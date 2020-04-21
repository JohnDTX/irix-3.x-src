#
#
include ${ROOT}/usr/include/make/commondefs

.SUFFIXES: .o .p

SRCS = jlinker.p jmisc jphase.1 jphase.2a jphase.2b

OBJS=jlinker.o ../svslib.o buildstab.o debug.o getbytes.o lines.o\
	names.o psym.o readj.o symbols.o

IDIR=.

INCS=$(IDIR)/symbols.h $(IDIR)/machine.h $(IDIR)/tree.h $(IDIR)/names.h\
	$(IDIR)/languages.h $(IDIR)/lists.h $(IDIR)/operators.h \
	$(IDIR)/events.h readj.h 

CFLAGS=-I$(IDIR) -I${ROOT}/usr/include -DGB_FIXES -Djuniper -Dsgi \
	-DMAINTAINER="jimt" '-Ddouble=long float' '-Datof=_latof'

all:	jlinker

install:	all
	${INSTALL} -src jlinker -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" jlinker20
	if test $$machine = juniper;\
	then \
		${INSTALL} -ln /usr/lib/jlinker20 -F /usr/lib \
		-idb "std.sw.unix mr" jlinker ;\
	fi

jlinker:	$(OBJS)
	$(PC) $(PCFLAGS) -o jlinker $(OBJS)

../svslib.o:
	cd ../lib; $(MAKE) -$(MAKEFLAGS) svslib

jlinker.o: $(SRCS)

.p.o:
	$(PC) $(PCFLAGS) -c $<

clean:
	rm -f *.o *.obj *.j

clobber:	clean
	rm -f jlinker
