# make bootp (RCF 951) server
#
include ${ROOT}/usr/include/make/commondefs

LCINCS=	-I$(ROOT)/usr/include/bsd
LCOPTS=	-O 
LLDLIBS= -lsun -lbsd
TARGETS=bootp

all default:$(TARGETS)

bootp:	bootp.c
	$(CCF) -o bootp bootp.c $(LDFLAGS)

install: all
	${INSTALL} -F /usr/etc -idb "std.sw.unix" "bootp bootptab"

clean clobber:
	rm -f *.BAK *.CKP .emacs* *.o $(TARGETS)
