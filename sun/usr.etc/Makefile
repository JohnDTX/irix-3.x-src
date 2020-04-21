# makefile for making SUN usr/etc files
#
#
include ${ROOT}/usr/include/make/commondefs

all boot install:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make $@; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make $@; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make $@; \
	fi
	cd yp; make $@
	cd ypserv; make $@

clean clobber:
	cd ip2; make $@
	cd pm2; make $@
	cd yp; make $@
	cd ypserv; make $@
