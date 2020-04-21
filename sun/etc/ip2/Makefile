# ip2/pm2 makefile for nfs/yp etc utilities

#
#
include ${ROOT}/usr/include/make/commondefs

LCINCS=	-I${ROOT}/usr/include/sun -I${ROOT}/usr/include/bsd
LCOPTS=	-O 
LLDLIBS= -lsun -lbsd -lrpcsvc
ETCTARGS=mount umount havenfs
USRTARGS=portmap
NFSTARGS=biod nfsd ypbind
TARGETS=$(ETCTARGS) $(USRTARGS) $(NFSTARGS)

MAKEPATH=.:..

.c:
	$(CCF) -o $@ $? $(LDFLAGS) 

all:	$(TARGETS)

install: all
	${INSTALL} -F /etc -idb "upd" "${ETCTARGS}"
	${INSTALL} -O -F /usr/etc -idb "upd" "${USRTARGS}"
	${INSTALL} -O -F /usr/etc -idb "nfs" "${NFSTARGS}"

clean:
	rm -f *.o a.out core

clobber: clean
	rm -f $(TARGETS)
