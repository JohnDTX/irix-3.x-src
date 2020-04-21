#
#ident	"\%W\%"
#
# Notes:
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#

#
# Local Definitions
#

IDB_TAG = -idb "std.sw.unix"
#
# Targets/Rules
#
default:
	if test "${machine}" = iris; then \
		echo no edge for 2400s; \
		exit 1; \
	else \
		cd libwin/ip2; \
		$(MAKE) -$(MAKEFLAGS) libwin.a;\
		cd ../../ip2; \
		$(MAKE) -$(MAKEFLAGS) edge; \
		cd ../gsh; \
		$(MAKE) -$(MAKEFLAGS); \
	fi

clean:
	cd libwin/ip2;\
	$(MAKE) -$(MAKEFLAGS) clean
	cd ip2;\
	$(MAKE) -$(MAKEFLAGS) clean
	cd gsh;\
	$(MAKE) -$(MAKEFLAGS) clean

clobber:	clean
	cd libwin/ip2;\
	$(MAKE) -$(MAKEFLAGS) clobber
	cd ip2;\
	$(MAKE) -$(MAKEFLAGS) clobber
	cd gsh;\
	$(MAKE) -$(MAKEFLAGS) clobber

FRC:

#
# Specific Target/Rules follow
#
install: default
	cd ip2;\
	$(INSTALL) -F /usr/bin $(IDB_TAG) -m 4755 -u root -g sys edge
	cd gsh;\
	$(INSTALL) -F /usr/lib $(IDB_TAG) -m 4755 -u root -g sys gsh
	cd tutor;\
	make install

depend: FRC
	$(MKDEPEND)
