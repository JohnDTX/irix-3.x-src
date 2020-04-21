
install: install.$(GLHDWR)

install.both:
	cd obj.gl1 ; GLHDWR=gl1 $(MAKE) install
	cd obj.gl2 ; GLHDWR=gl2 $(MAKE) install

install.gl1 install.gl2:
	cd obj.$(GLHDWR) ; $(MAKE) install

clean:
	cd obj.gl1 ; $(MAKE) clean
	cd obj.gl2 ; $(MAKE) clean
