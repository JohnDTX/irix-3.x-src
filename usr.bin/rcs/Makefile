SUBDIR	= src/rcs src/rdiff src/rdiff3

all install clean clobber depend:
	for i in $(SUBDIR); do (echo $$i; cd $$i; make $@); done

boot:	install
