
include ${ROOT}/usr/include/make/commondefs

IDBTAG = -idb "std.sw.unix"

all:

clean:

Include install: 
	${INSTALL} ${IDBTAG} -F /usr/include/gl2 "port.h image.h"
	${INSTALL} ${IDBTAG} -ln /usr/include/gl2/port.h \
		-F /usr/include  port.h
	${INSTALL} ${IDBTAG} -ln /usr/include/gl2/image.h \
		-F /usr/include  image.h
