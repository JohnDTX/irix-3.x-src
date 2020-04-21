#
include ${ROOT}/usr/include/make/commondefs


install: 
	${INSTALL} -u root -g sys -m 755 -F /dev -idb "std.sw.unix mr" MAKEDEV
	cd ${ROOT}/dev ; ./MAKEDEV

clean clobber:
