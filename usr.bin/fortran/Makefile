
all: pascal code library ulinker fortran dbg

pascal:
	cc -o pascal pascal.o

code: 	code.o
	cc -o code code.o

library:
	cc -o library library.o

dbg:
	cc -o dbg dbg.o

ulinker:
	cc -o ulinker ulinker.o

fortran:
	cc -o fortran fortran.o

install: all
	strip code ulinker fortran dbg library pascal
	cp code ulinker fortran pascal $(DEST)/usr/lib
	cp library dbg $(DEST)/usr/bin
	cd $(DEST)/usr/lib ; chgrp bin code ulinker fortran pascal ;\
		chmod 755 code ulinker fortran pascal ;\
		chown bin code ulinker fortran pascal
	cd $(DEST)/usr/bin ; chgrp bin dbg library ;\
		chmod 755 dbg library ;\
		chown bin dbg library
	cp ftncterrs ftnrterrs pascterrs $(DEST)/usr/lib
	cd $(DEST)/usr/lib ; chgrp bin ftncterrs ftnrterrs pascterrs;\
		chmod 755 ftncterrs ftnrterrs pascterrs;\
		chown bin ftncterrs ftnrterrs pascterrs
	cp ftnlib.obj paslib.obj sky.paslib.obj $(DEST)/usr/lib
	cd $(DEST)/usr/lib ; chgrp bin ftnlib.obj paslib.obj sky.paslib.obj ;\
		chmod 755 ftnlib.obj paslib.obj sky.paslib.obj ;\
		chown bin ftnlib.obj paslib.obj sky.paslib.obj
	rm -f $(DEST)/usr/lib/hwfp.paslib.obj
	ln $(DEST)/usr/lib/sky.paslib.obj $(DEST)/usr/lib/hwfp.paslib.obj

clean:
	rm -f code library ulinker fortran pascal dbg
