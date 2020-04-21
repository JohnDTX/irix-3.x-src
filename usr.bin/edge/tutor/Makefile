#
# This makefile creates the edge tutorial directory and it's subdirectories
#
include	${ROOT}/usr/include/make/commondefs

#
# Compile Time Options, e.g. LCDEFS, LLDLIBS.
#

#
# Local Definitions, e.g. source/object lists, installation information.
#
IDB_TAG = -idb "std.sw.unix"

#
# Every makefile must have a rule named default
#
default:

install: default
	${INSTALL} -dir -m 755 -u tutor -g demos ${IDB_TAG} /usr/tutorial
	${INSTALL} -dir -m 755 -u tutor -g demos ${IDB_TAG} /usr/tutorial/edge
	cd C ; make -f C.mk install
	cd fortran ; make -f fortran.mk install

depend:

incdepend:

fluff:

tags:

clean: force

clobber: force

#
# If your targets' names (e.g. clobber) overload filenames in the current
# directory, you must make these targets depend on force.
#
force:
