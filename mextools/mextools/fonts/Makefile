
# "$Header:"

include ${ROOT}/usr/include/make/commondefs

IDBTAG = -idb "std.sw.unix"
GRPOWN = -g sys -u root
MODS   = -m 755


all:

clean:

install:
	${INSTALL} ${GRPOWN} ${IDBTAG} ${MODS}  -dir /usr/lib/gl2 
	${INSTALL} ${GRPOWN} ${IDBTAG} ${MODS} -dir /usr/lib/gl2/fonts
	find . -name "*.fnt" ! -name "*,v" ! -type d -print | xargs | \
	while read files_in; do \
		${INSTALL} ${IDBTAG} ${GRPOWN} -F /usr/lib/gl2/fonts \
			"$$files_in" ;\
	done 
