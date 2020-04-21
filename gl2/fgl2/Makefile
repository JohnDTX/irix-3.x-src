#
#
include ${ROOT}/usr/include/make/commondefs

GLHDWR = gl2
INCS = fget.h fgl.h fdevice.h
LIBPRIM = ${ROOT}/usr/lib/$(GLHDWR)/lib.prim

INSCMD=${INSTALL} -idb "std.sw.unix" -u bin -g bin 

.f.o:
	$(CC) ${CFLAGS} f77 -c $*.f

all:	libfgl.a fgldat.o

lib.i: flibgen.awk $(LIBPRIM)
	awk -f flibgen.awk $(LIBPRIM) > awk.out 2>&1

$(LIBPRIM):
	cd ../tpe2/term; make install-common

libfgl.a: lib/lastdone
	rm -f libfgl.a
	cd lib; ar cr ../libfgl.a *.o
	ranlib libfgl.a

lib/lastdone: lib.i
	rm -rf lib; mkdir lib
	ex - lib.i < fsplit.ex
	cd lib; $(MAKE) -f ../lib.Makefile "SRCS=`echo *.fc`"
	cd bugfixes; $(MAKE)
	cp bugfixes/*.o lib

clobber clean:
	rm -f *.i libfgl.a *.junk *.out *.j lib

install: all
	strip libfgl.a
	${INSCMD} -m 644 -src libfgl.a -F /usr/lib libfgl2.a
	ranlib ${ROOT}/usr/lib/libfgl2.a
	${INSCMD} -ln /usr/lib/libfgl2.a -F /usr/lib libfgl.a
	${INSCMD} -m 444 -F /usr/include/gl2 "$(INCS)"
	for i in $(INCS); do \
		${INSCMD} -ln /usr/include/${GLHDWR}/$$i -F /usr/include $$i ;\
	done
	${INSCMD} -m 444 -F /usr/lib fgldat.o

.SUFFIXES:
.SUFFIXES: .a .o .c .fc .f

