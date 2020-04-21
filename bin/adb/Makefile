#
# $Source: /d2/3.7/src/bin/adb/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 14:51:48 $
#
install all:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make $@; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make $@; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make $@; \
	fi

boot:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make install; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make install; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make install; \
	fi

clean clobber:
	cd ip2; make $@
	cd pm2; make $@
