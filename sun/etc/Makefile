# makefile for making SUN etc files

#
include ${ROOT}/usr/include/make/commondefs
# $Source: /d2/3.7/src/sun/etc/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:21:12 $

UPDFILES= rpc
NOUPDFILES= ethers exports netgroup networks

all boot:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make $@; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make $@; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make $@; \
	fi

install:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make $@; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make $@; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make $@; \
	fi

	${INSTALL} -u root -g sys -m 644 -F /etc \
	    -idb "std.sw.unix mr config(update)" "${UPDFILES}"
	${INSTALL} -u root -g sys -m 644 -F /etc \
	    -idb "std.sw.unix mr config(noupdate)" "${NOUPDFILES}"

clean clobber:
	cd ip2; make $@
	cd pm2; make $@
