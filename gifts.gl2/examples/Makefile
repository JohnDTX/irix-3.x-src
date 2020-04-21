
include ${ROOT}/usr/include/make/commondefs

D = /usr/people/gifts/examples
IDBTAG = -idb "root upd"
M_GRP_O = -m 644 -g guest -u guest
DIRS = Fortran Pascal xns tcp misc
GUIDES = 1.ProgGuide 2.Append 4.WindMgr 5.ProgExamples

install:
	${INSTALL} -m 755 -g guest -u guest ${IDBTAG} -dir ${D}
	${INSTALL} ${M_GRP_O} ${IDBTAG} -src Make.examp \
		-F ${D} Makefile
	for i in ${GUIDES} ; do \
	    cd $$i;  find . ! -type d ! -name "*,v" -print | xargs | \
	    while read files_in; do \
		${INSTALL} ${M_GRP_O} ${IDBTAG} -F ${D} "$$files_in"; \
	    done; cd ..; \
	done
	for i in ${DIRS} ; do \
	    ${INSTALL} -m 755 -g guest -u guest ${IDBTAG} -dir ${D}/$$i ;\
	    find $$i ! -type d ! -name "*,v" -print | xargs | \
	    while read files_in; do \
		${INSTALL} ${M_GRP_O} ${IDBTAG} -F ${D} "$$files_in"; \
	    done;\
	done

all:

clean clobber:
	cd 2.Append; make clean
