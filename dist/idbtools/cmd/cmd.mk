# ident "$Header: "
#
# Notes: Makefile for (unshipped) idb tools.
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
LCINCS	= -I../include
Progs	= idbedit idbinst idbrmat idbscan idbproto idbgen idbdelink idbdist
Links	= idbinq
CMDS	= $(Progs) $(Links)
LIB	= ../libidb/libidb.a

#
# Targets/Rules
#

default:	$(CMDS)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(CMDS)

FRC:

#
# Specific Target/Rules follow
#
install: default
	$(INSTALL) -F /usr/sbin -idb noship "$(Progs)"
	$(INSTALL) -lns idbscan -F /usr/sbin -idb noship idbinq

idbedit: idbedit.o $(LIB)
	$(CCF) -o idbedit idbedit.o $(LIB) $(LDFLAGS)
idbinst: idbinst.o $(LIB)
	$(CCF) -o idbinst idbinst.o $(LIB) $(LDFLAGS)
idbrmat: idbrmat.o $(LIB)
	$(CCF) -o idbrmat idbrmat.o $(LIB) $(LDFLAGS)
idbscan: idbscan.o $(LIB)
	$(CCF) -o idbscan idbscan.o $(LIB) $(LDFLAGS)
idbproto: idbproto.o $(LIB)
	$(CCF) -o idbproto idbproto.o $(LIB) $(LDFLAGS)
idbinq: idbscan
	rm -f idbinq ; ln idbscan idbinq
idbgen: idbgen.o $(LIB)
	$(CCF) -o idbgen idbgen.o $(LIB) -lbsd $(LDFLAGS)
idbdelink: idbdelink.o $(LIB)
	$(CCF) -o idbdelink idbdelink.o $(LIB) $(LDFLAGS)
idbmode: idbmode.o $(LIB)
	$(CCF) -o idbmode idbmode.o $(LIB) $(LDFLAGS)
idbdist: idbdist.o $(LIB)
	$(CCF) -o idbdist idbdist.o $(LIB) $(LDFLAGS)

idbedit.o: ../include/idb.h
idbinst.o: ../include/idb.h
idbrmat.o: ../include/idb.h
idbscan.o: ../include/idb.h
idbproto.o: ../include/idb.h
idbgen.o: ../include/idb.h
idbdelink.o: ../include/idb.h
idbmode.o: ../include/idb.h
idbdist.o: ../include/idb.h
