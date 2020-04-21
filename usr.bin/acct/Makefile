# makefile for usr.bin/acct
#
include ${ROOT}/usr/include/make/commondefs

# $Source: /d2/3.7/src/usr.bin/acct/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:42:07 $

LIB	= lib/a.a
LLDLIBS	= -lsun -lbsd -ldbm 
LCINCS  = -I${ROOT}/usr/include
IDBTAG  = -idb "usr upd"
GRPOWN  = -g adm -u adm
ACCTDIR  = /usr/adm/acct

PROGS	= acctcms acctcom acctcon1 acctcon2 acctdisk acctdusg acctmerg \
	  accton acctprc1 acctprc2 acctwtmp fwtmp wtmpfix

SCRIPTS	= chargefee ckpacct dodisk lastlogin monacct nulladm \
	 prctmp prdaily prtacct remove runacct shutacct startup turnacct

DIRS	= $(ACCTDIR)/fiscal $(ACCTDIR)/nite $(ACCTDIR)/sum

all:	library $(PROGS) $(SCRIPTS)

$(PROGS):$$@.c $(LIB)
	$(CCF)  -o $@ $@.c $(LDFLAGS) ./lib/a.a

$(LIB) library:
	cd lib; make all

install:all
	${INSTALL} ${IDBTAG} ${GRPOWN} -m 775 -dir "/usr/adm ${ACCTDIR} ${DIRS}"
	${INSTALL} ${IDBTAG}  ${GRPOWN} -m 755 -F /usr/lib/acct \
		"acctcms acctcon1 acctcon2 acctdisk acctdusg \
		 acctmerg acctprc1 acctprc2 acctwtmp fwtmp wtmpfix "
	${INSTALL} -u root -g sys -m 4755 -F /usr/lib/acct accton
	${INSTALL}  ${IDBTAG} ${GRPOWN} -m 755 -F /usr/bin acctcom
	${INSTALL} ${IDBTAG} ${GRPOWN} -m 644 -idb "config(suggest)" \
		-src holidays.sh -F /usr/lib/acct holidays
	${INSTALL} ${IDBTAG} ${GRPOWN} -m 664 -idb "config(noupdate)" \
		-F /usr/adm pacct

clean:
	rm -f *.o
	cd lib; $(MAKE) clean

clobber:
	rm -f *.o
	rm -f $(PROGS) $(SCRIPTS)
	cd lib; $(MAKE) clobber


.sh:
	${INSTALL} ${IDBTAG} ${GRPOWN} -src $< -m 755 -F /usr/lib/acct $@

.c:
	$(CC) ${CFLAGS} -o $@ $< $(LDFLAGS) $(LIB) 
