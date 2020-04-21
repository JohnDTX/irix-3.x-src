
#
#
include ${ROOT}/usr/include/make/commondefs


PROGS	= ctags fold mkstr xstr
IDBTAG = -idb "root upd"

all:  $(PROGS)

install:all
	${INSTALL} -m 755 -g bin -u bin -F /usr/bin ${IDBTAG} "${PROGS}"

clean:

clobber:clean
	rm -f $(PROGS)
