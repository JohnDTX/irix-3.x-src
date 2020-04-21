#       Makefile        4.6     83/08/13

include ${ROOT}/usr/include/make/commondefs

#
# Doing a make install builds /usr/include
#

STD=	mntent.h
SUNSTD=	netdb.h
IDBTAG = -idb "std.sw.unix"

all:

Include:
	${INSTALL} -u root -g sys -m 444 -F /usr/include \
	-idb "std.sw.unix mr" "${STD}"
	${INSTALL} -dir /usr/include/sun -idb "std.sw.unix mr" 
	${INSTALL} -u root -g sys -m 444 -F /usr/include/sun \
	-idb "std.sw.unix mr" "${SUNSTD}"

install: ${SHARED}
	-for i in ${STD}; do \
	        ${INSTALL} -m 444 ${IDBTAG} -F /usr/include   $$i; \
	done
	-for i in ${SUBDIRS}; do \
	        if [ ! -d ${ROOT}/usr/include/$$i ]; \
	        then \
	        	${INSTALL} -dir /usr/include/$$i ${IDBTAG} ;\
	        fi; \
	        (cd $$i; for j in *.[ih]; do  \
		    ${INSTALL} -m 444 ${IDBTAG} -F $$i  $$j;  done); \
	done

clean:

