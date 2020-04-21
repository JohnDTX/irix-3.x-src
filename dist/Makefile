all install:
	cd idbtools ; $(MAKE) -f idbtools.mk install
	cd tools ; $(MAKE) install

clean clobber:
	cd idbtools ; $(MAKE) -f idbtools.mk clobber
	cd tools ; $(MAKE) clobber

.DEFAULT:
	cd tools ; $(MAKE) $@
