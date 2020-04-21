
include ${ROOT}/usr/include/make/commondefs

IDBTAG = -idb "root upd"
GRPOWN = -u guest -g guest

install:
	${INSTALL} ${GRPOWN} ${IDBTAG} \
		-dir /usr/people/gifts/dbx_tutorial/C
	${INSTALL} ${GRPOWN} ${IDBTAG} \
		-dir /usr/people/gifts/dbx_tutorial/C/src
	${INSTALL} ${GRPOWN} ${IDBTAG} \
		-F /usr/people/gifts/dbx_tutorial/C names.in
	find src ! -type d  -print | \
	xargs | \
	while read files_in; do \
		${INSTALL} ${GRPOWN} ${IDBTAG} \
			-F /usr/people/gifts/dbx_tutorial/C "$$files_in"; \
	done

