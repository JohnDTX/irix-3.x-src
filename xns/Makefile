#
# Makefile for constructing the xns library and its utilities
#

all: library servers utilities trace multicast

clean clobber install:
	cd lib; make $@
	cd servers; make $@
	cd cmd; make $@
#	cd trace; make $@
	cd multicast; make $@

library: anything
	cd lib; make

servers: library anything
	cd servers; make

utilities: library anything
	cd cmd; make

trace: anything
	cd trace; make

multicast: anything
	cd multicast; make

anything:
