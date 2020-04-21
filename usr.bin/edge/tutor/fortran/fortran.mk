#
# This makefile installs the fortran version of the edge tutorial
#
include	${ROOT}/usr/include/make/commondefs

#
# Compile Time Options, e.g. LCDEFS, LLDLIBS.
#

#
# Local Definitions, e.g. source/object lists, installation information.
#
IDB_TAG = -idb "ftn.sw.fedgetut"

#
# Every makefile must have a rule named default
#
default:

install: default
	${INSTALL} -dir -m 755 -u tutor -g demos ${IDB_TAG} /usr/tutorial/edge/fortran
	${INSTALL} -f /usr/tutorial/edge/fortran ${IDB_TAG} names.in
	${INSTALL} -dir -m 755 -u tutor -g demos ${IDB_TAG} /usr/tutorial/edge/fortran/src
	${INSTALL} -f /usr/tutorial/edge/fortran/src ${IDB_TAG} Makefile
	${INSTALL} -f /usr/tutorial/edge/fortran/src ${IDB_TAG} names.in
	${INSTALL} -f /usr/tutorial/edge/fortran/src ${IDB_TAG} sort.m
	${INSTALL} -f /usr/tutorial/edge/fortran/src ${IDB_TAG} sort.h
	${INSTALL} -f /usr/tutorial/edge/fortran/src ${IDB_TAG} scrub
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
