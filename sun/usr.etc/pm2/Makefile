# ip2/pm2 makefile for nfs/yp/rpc usr/etc utilities

include ${ROOT}/usr/include/make/commondefs

LCINCS=	-I${ROOT}/usr/include/sun -I${ROOT}/usr/include/bsd
LCOPTS=	-O 
LLDLIBS=  -lrpcsvc -lsun -lbsd -ldbm
ETCTARGS=showmount
NFSTARGS=rpc.mountd rpc.rstatd rpc.rwalld rpc.ruserd \
	rpc.sprayd rpc.passwd rpcinfo rwall spray
TARGETS=$(ETCTARGS) $(NFSTARGS)

MAKEPATH=..


all:	$(TARGETS) nfsstat

.c:
	$(CCF) -o $@ $? $(LDFLAGS) 

rpc.passwd:	$$@.c
	$(CCF) -o $@ $? $(LDFLAGS) -lcrypt 

#
# rpc.mountd shouldn't be stripped, so the user can patch
# some security related switches using adb.
#
MOUNTLDFLAGS = `echo $(LDFLAGS) | sed -e "s/ -s / /g"`
rpc.mountd:	$$@.c
	$(CCF) -o $@ $? $(MOUNTLDFLAGS) 

install: all
	${INSTALL} -idb "nfs" -F /etc  "${ETCTARGS}"
	${INSTALL} -idb "nfs" -F /usr/etc  "${NFSTARGS}"
	${INSTALL} -idb "nfs" -u root -g sys -m 2755 -F /usr/etc nfsstat

clean clobber:
	rm -f $(TARGETS) *.o a.out core
