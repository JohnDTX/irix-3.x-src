#
# Makefile for constructing the IP2 standalone cmds
#

all: mdfex ipfex sifex

clean clobber install:
	cd mdfex; make $@
	cd ipfex; make $@
	cd sifex; make $@

mdfex: anything
	cd mdfex; make

ipfex: anything
	cd ipfex; make

sifex: anything
	cd sifex; make

anything:
