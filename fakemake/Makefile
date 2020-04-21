#
# Makefile for IRIS make wrapper. Converted to the new Clover style build
# regime, which requires the inclusion of several header files which contain
# common rules and declarations.  The header files are located in the
# $ROOT/usr/include/make directory. 
#
# Andre 3/28/1987...
#

include ${ROOT}/usr/include/make/commondefs
all: make

clean clobber:
	rm -f make

install: all
	if test ! -d $(ROOT)/make ;\
	then rm -rf $(ROOT)/make ; mkdir $(ROOT)/make ;\
	fi
	cp make $(ROOT)/make/make

make: make.c
	cc -O -o make make.c
