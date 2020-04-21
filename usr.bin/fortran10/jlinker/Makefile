#
#
include ${ROOT}/usr/include/make/commondefs


.SUFFIXES: .o .p

SRC20=../../fortran20

SRCS = jlinker10.p jmisc jphase.1 jphase.2a jphase.2b

OBJS=jlinker10.o ../svslib.o buildstab.o debug.o getbytes.o lines.o\
	names.o psym.o readj.o symbols.o

IDIR=.

INCS=$(IDIR)/symbols.h $(IDIR)/machine.h $(IDIR)/tree.h $(IDIR)/names.h\
	$(IDIR)/languages.h $(IDIR)/lists.h $(IDIR)/operators.h \
	$(IDIR)/events.h $(IDIR)/readj.h $(IDIR)/getbytes.h $(IDIR)/defs.h\
	$(IDIR)/printsym.h

CFLAGS=-I$(IDIR) -DGB_FIXES -Dsgi -DMAINTAINER="jimt"\
	'-Ddouble=long float' '-Datof=_latof'

all:	jlinker10

install:	all
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" jlinker10
	if test $$machine = iris;\
	then \
		${INSTALL} -ln /usr/lib/jlinker10 -F /usr/lib \
		-idb "std.sw.unix mr" jlinker ;\
	fi

jlinker10:	$(OBJS)
	$(PC) $(PCFLAGS) -o jlinker10 $(OBJS)

.p.o:
	$(PC) $(PCFLAGS) -c $<

jlinker10.o: $(SRCS) 

../svslib.o:
	cd ..; $(MAKE) svslib

clean:
	rm -f jlinker10.o *.obj 

clobber:	clean
	rm -f jlinker10
