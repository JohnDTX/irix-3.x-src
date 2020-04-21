
include ${ROOT}/usr/include/make/commondefs

D = /usr/people/tutorial/c.graphics
D1 = /usr/people/tutorial/c.graphics/src

default all clean clobber:
DIRS = include lib workshop

IDBTAGS	= -idb "gltut usr"
GRPOWN	= -g demos -u tutor
install:
	${INSTALL} -m 755 ${IDBTAGS} ${GRPOWN} -dir ${D} 
	${INSTALL} -m 755 ${IDBTAGS} ${GRPOWN} -dir ${D1} 
	${INSTALL} -m 644 ${IDBTAGS} ${GRPOWN} -F ${D} README
	${INSTALL} -m 644 ${IDBTAGS} ${GRPOWN} -F ${D} \
		-src Makefile.usr Makefile
	for i in ${DIRS} ; do \
	    ${INSTALL} -m 755 ${IDBTAGS} ${GRPOWN} -dir ${D}/"$$i" ;\
	    find $$i -name "*.[hc]" -print | \
	    xargs |  \
	    while read files_in; do \
		${INSTALL} -m 644 ${IDBTAGS} ${GRPOWN} -F ${D} "$$files_in" ;\
	    done ;\
	done
	cd online; make install
