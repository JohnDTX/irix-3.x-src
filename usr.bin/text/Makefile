#	text sub-system make file

OPS	= 

all default install clean clobber:
	-for i in roff.d macros.d shells.d eqn.d neqn.d tbl.d checkmm; do \
		echo "==== usr.bin/text/$$i"; cd $$i; $(MAKE) $@; cd ..; \
	done
