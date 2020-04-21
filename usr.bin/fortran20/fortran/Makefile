#
#
include ${ROOT}/usr/include/make/commondefs


.SUFFIXES: .o .p

SRCS = f.assign f.call f.dump.1 f.dump.2 f.exec.1 f.exec.2 \
	   f.exec.3 f.exec.4 f.expr f.init f.misc f.opt f.scan.1 \
	   f.scan.2 f.spec f.state f.symb fortran.p

OBJS=fortran.o ../svslib.o

PCFLAGS = -DFOR_68020

all:	fortran

install:	all
	${INSTALL} -src fortran -u bin -g bin -m 755 -F /usr/lib \
		-idb "std.sw.unix mr" fortran20
	if test $$machine = juniper;\
	then \
		${INSTALL} -ln /usr/lib/fortran20 -F /usr/lib \
			-idb "std.sw.unix mr" fortran ;\
	fi

fortran:	$(OBJS)
	$(PC) $(PCFLAGS) -o fortran $(OBJS)

../svslib.o:
	cd ../lib; $(MAKE) -$(MAKEFLAGS) svslib

fortran.o: $(SRCS)

.p.o:
	$(PC) $(PCFLAGS) $(PCFLAGS) -c $<


clean:
	rm -f fortran.o *.obj *.j

clobber:	clean
	rm -f fortran
