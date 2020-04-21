#
# Makefile for libbsd
#
default install all boot:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make $@; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make $@; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make $@; \
	fi

clean clobber:
	cd ip2; make $@
	cd pm2; make $@
