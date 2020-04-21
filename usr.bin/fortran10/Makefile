BINDIRS	= fortran code jlinker pascal
LIBDIRS = bin lib flib

all install: svslib
	for i in $(BINDIRS) $(LIBDIRS); do ( cd $$i; $(MAKE) $@ ); done

clean:
	for i in $(BINDIRS) $(LIBDIRS); do ( cd $$i; $(MAKE) clean ); done

bootbins:	svslib.o
	for i in $(BINDIRS); do ( cd $$i; $(MAKE) install ); done

bootlibs:	bin/asm68k
	for i in $(LIBDIRS); do ( cd $$i; $(MAKE) install ); done

#
#	boot makes the libs according to $$machine, the executables according to 
#	$$HOST.
#
boot:	bin/asm68k
	if test $$HOST = I && test $$machine = juniper;\
	then \
		(CC_OPTS="${CC_OPTS} -ZI" machine=iris make bootbins);\
	elif test $$HOST = J && test $$machine = iris;\
	then \
		(CC_OPTS="${CC_OPTS} -ZJ" machine=juniper make bootbins);\
	else \
		make bootbins;\
	fi
	make bootlibs;

clobber:
	rm -f *.a *.o bin/asm68k
	for i in $(BINDIRS) $(LIBDIRS) util; do ( cd $$i; $(MAKE) clobber); done

svslib: bin/asm68k svslib.o

svslib.o:
	cd lib; make svslib

bin/asm68k:
	cd util; make install

