#
# Makefile for libc
#
default boot install:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make $@; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make $@; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make $@; \
	fi

clean:
	cd ip2; make clean
	cd pm2; make clean

clobber:
	cd ip2; make clobber
	cd pm2; make clobber
