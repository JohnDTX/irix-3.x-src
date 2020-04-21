# Construct /usr/include/make
#
# $Source: /d2/3.7/src/include/make/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 16:12:32 $

include ${ROOT}/usr/include/make/commondefs
INSTCMD	=${INSTALL} -u root -g sys -idb "upd usr"

all default clean clobber:

install: commonrules commondefs mkdepend
	${INSTCMD} -dir /usr/include/make
	${INSTCMD} -m 444 -idb ${PRODUCT} -F /usr/include/make ${PRODUCT}defs
	${INSTCMD} -m 444 -idb ${PRODUCT} -lns ${PRODUCT}defs \
		-F /usr/include/make defs
	${INSTCMD} -m 444 -F /usr/include/make "commonrules commondefs"
	${INSTCMD} -F /usr/bin "mkdepend"
