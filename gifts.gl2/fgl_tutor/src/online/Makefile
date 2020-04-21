
include ${ROOT}/usr/include/make/commondefs

D = /usr/people/tutorial/f.graphics/src/online

default all clean clobber:
DIRS = backface curve gouraud patch
IDBTAG = -idb "ftn"
GRPOWN = -g demos -u tutorf

install:
	${INSTALL} ${IDBTAG} ${GRPOWN} -m 644 -F ${D} -src Makefile.usr Makefile
	for i in ${DIRS} ; do \
	    ${INSTALL} ${IDBTAG} ${GRPOWN} -m 755 -dir ${D}/"$$i" ;\
	done
	find . -name "*.[hc]" -print | \
	xargs |  \
	while read files_in; do \
	    ${INSTALL} ${IDBTAG} ${GRPOWN} -m 644 -F ${D} "$$files_in" ;\
	done 
