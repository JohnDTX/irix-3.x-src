# 'make all'			makes test programs
# 'make clean'			cleans directories
# 'make copy DESTDIR=path'	copies test programs to path
# 'make dist DESTDIR=path'	copies sources to path

MAKE=make
DESTDIR=/no/such/path
COPYFILES=runtests runbasic server README Testitems

all:
	cd basic; $(MAKE)
	cd general; $(MAKE)
	cd special; $(MAKE)
#	cd tools; $(MAKE)

clean clobber:
	cd basic; $(MAKE) clean
	cd general; $(MAKE) clean
	cd special; $(MAKE) clean
#	cd tools; $(MAKE) clean

copy dist: mknewdirs
	cp $(COPYFILES) $(DESTDIR)
	if test $@ = dist; then cp Makefile $(DESTDIR); fi
	for d in basic general special; do \
		(cd $$d; $(MAKE) $@ DESTDIR=$(DESTDIR)/$$d) \
	done

mknewdirs:
	-mkdir $(DESTDIR)/basic $(DESTDIR)/general $(DESTDIR)/special \
	       $(DESTDIR)/tools
