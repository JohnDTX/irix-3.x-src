include ${ROOT}/usr/include/make/commondefs

PROGDIRS=jcode jlinker pascal fortran bin
LIBDIRS=lib flib
DIRS= $(LIBDIRS) $(PROGDIRS)
IDBTAG = -idb "std.si.unix"

all: libs
	for i in $(PROGDIRS);\
	do\
		cd $$i;\
		$(MAKE) -$(MAKEFLAGS) all;\
		cd ..;\
	done

bootbins:	svslib.o
	for i in $(PROGDIRS); do ( cd $$i; $(MAKE) install ); done

bootlibs:	bin/asm68k
	for i in $(LIBDIRS); do ( cd $$i; $(MAKE) install ); done

boot:	bin/asm68k
	${CC} ${CFLAGS} make bootbins
	make bootlibs

libs: svslib
	for i in $(LIBDIRS);\
	do\
		cd $$i;\
		$(MAKE) -$(MAKEFLAGS) all;\
		cd ..;\
	done

install: installlibs
	ranlib ${ROOT}/usr/lib/libp.a
	for i in $(PROGDIRS);\
	do\
		cd $$i;\
		$(MAKE) -$(MAKEFLAGS) install;\
		cd ..;\
	done

installlibs: svslib
	for i in $(LIBDIRS);\
	do\
		cd $$i;\
		$(MAKE) -$(MAKEFLAGS) install;\
		cd ..;\
	done

clean:
	for i in $(DIRS);\
	do\
		cd $$i;\
		$(MAKE) -$(MAKEFLAGS) clean;\
		cd ..;\
	done

clobber:
	for i in $(DIRS) util;\
	do\
		cd $$i;\
		$(MAKE) -$(MAKEFLAGS) clobber;\
		cd ..;\
	done
	rm -f *.a *.o bin/asm68k

svslib: bin/asm68k svslib.o

svslib.o:
	cd lib; make svslib

bin/asm68k:
	cd util; make install

