#
#
include ${ROOT}/usr/include/make/commondefs

files = ascii eqnchar greek tabs

all: $(files)

install:FRC $(ROOT)/usr/pub $(files)
	${INSTALL} -u bin -g bin -m 644 -F /usr/pub \
	-idb "std.sw.unix mr" "${files}"

clean clobber:

$(ROOT)/usr/pub:
	mkdir $@ ; chmod 755 $@ ; chgrp bin $@ ; chown bin $@

FRC:
