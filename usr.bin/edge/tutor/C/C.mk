#
# This makefile installs the C version of the edge tutorial
#
include	${ROOT}/usr/include/make/commondefs

#
# Compile Time Options, e.g. LCDEFS, LLDLIBS.
#

#
# Local Definitions, e.g. source/object lists, installation information.
#
IDB_TAG = -idb "std.sw.cedgetut"

#
# Every makefile must have a rule named default
#
default:

install: default
	${INSTALL} -dir -m 755 -u tutor -g demos ${IDB_TAG} /usr/tutorial/edge/C
	${INSTALL} -f /usr/tutorial/edge/C ${IDB_TAG} names.in
	${INSTALL} -dir -m 755 -u tutor -g demos ${IDB_TAG} /usr/tutorial/edge/C/src
	${INSTALL} -f /usr/tutorial/edge/C/src ${IDB_TAG} Makefile
	${INSTALL} -f /usr/tutorial/edge/C/src ${IDB_TAG} names.in
	${INSTALL} -f /usr/tutorial/edge/C/src ${IDB_TAG} sort.m
	${INSTALL} -f /usr/tutorial/edge/C/src ${IDB_TAG} scrub
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
