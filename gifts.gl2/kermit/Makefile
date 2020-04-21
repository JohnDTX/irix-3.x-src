
include ${ROOT}/usr/include/make/commondefs

D = /usr/people/gifts/kermit
IDBTAG = -idb "root upd"
DIRS = doc src bin
M_GRP_O = -m 666 -g guest -u guest

install:
	make -f Makefile.usr kermit
	make -f Makefile.usr clean
	${INSTALL} -m 777 -g guest -u guest ${IDBTAG} -dir ${D}
	${INSTALL} ${M_GRP_O} ${IDBTAG} -F ${D} README
	for i in ${DIRS} ; do \
	    ${INSTALL} -m 777 -g guest -u guest ${IDBTAG} -dir ${D}/"$$i" ;\
	    find $$i ! -type d  -print | \
	    xargs | \
	    while read files_in; do \
		${INSTALL} ${M_GRP_O} ${IDBTAG} -F ${D} "$$files_in"; \
	    done;\
	done

all:
	make -f Makefile.usr kermit

clean clobber:
	cd src; make clean
	rm -f bin/kermit
