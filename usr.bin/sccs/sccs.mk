#	@(#)sccs.mk	1.5 -- SCCS build

BIN = /usr/bin
HELPLIB = /usr/lib/help
ARGS =
INSTR =

inssrc:
	cd lib; $(MAKE)
	cd src; $(MAKE) $(ARGS) BIN=$(BIN)

install:
	cd lib; $(MAKE) INSTR=install
	cd src; $(MAKE) $(ARGS) BIN=$(BIN); $(MAKE) owner BIN=$(BIN) ARGS="$(ARGS)"
	if [ ! "$(ARGS)" ]; \
	then cd help.d; $(MAKE) HELPLIB=$(HELPLIB); \
	fi

clobber:
	cd lib; $(MAKE) clobber
	cd src; $(MAKE) clobber BIN=$(BIN)
