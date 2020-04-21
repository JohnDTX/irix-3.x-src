#
include ${ROOT}/usr/include/make/commondefs

# Build all the various SGI kernels, as well as /usr/include
#
# $Source: /d2/3.7/src/sys/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 17:26:10 $
#

# Macros for the reconfigurable kernel tree
HDIRS	 = bfs debug efs gl1 gpib h ipII multibus multibusif net netinet \
	   nfs pmII rpc streams vm xns com bsd 
SYSDIRS = h bfs efs streams vm com
STDDIRS = pmII ipII multibus multibusif gpib gl1 xns net netinet nfs xns
IDBTAG	= -idb "sys"
GRPOWN  = -g sys -u root
INSCMD	=${INSTALL} ${IDBTAG}
INSHDR	=${INSCMD} ${GRPOWN} -m 444

default:
	cd conf; make

#
# Generate subdirectories of /usr/include
#
Include:
	${INSCMD} ${GRPOWN} -dir /usr/include/sys
	for i in ${STDDIRS}; do \
		${INSCMD} -dir /usr/include/"$$i" ;\
		cd $$i ; ${INSHDR} -F /usr/include/"$$i" "`echo *.h`";\
		cd .. ;\
	done
	for i in ${SYSDIRS}; do \
		cd $$i; ${INSHDR} -F /usr/include/sys "`echo *.h`";\
		cd .. ;\
	done
	if test "${machine}" = juniper; then \
		${INSCMD} -lns /usr/include/ipII -F /usr/include machine; \
	else \
		${INSCMD} -lns /usr/include/pmII -F /usr/include machine; \
	fi

install:
	cd conf; make $@
	cd config; make $@
	${INSCMD} -u root -g sys -dir /usr/sys
	for i in ${HDIRS}; do \
	    ${INSCMD} -u root -g sys -dir /usr/sys/"$$i" ;\
	    cd $$i; ${INSCMD} -u root -g sys -idb "sys" \
		-F /usr/sys/$$i  "`echo *.h`" ;\
	    cd ..; \
	done

clean clobber:
	cd config; make $@
	cd conf; make $@
	cd kernels; rm -f *


anything:
