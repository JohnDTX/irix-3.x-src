#
# Not-shipped software development tools.
#

include ${ROOT}/usr/include/make/commondefs

LLDLIBS= -lbsd -lPW
IDBTAG= -idb "std.sw.unix noship"

TARGETS= tlink idbcanon.sh Mk.sh idb_scn

default: ${TARGETS}

install: default
	cp Mk.sh ../

tlink:
	${CCF} -o tlink tlink.c ${LDFLAGS} 

idb_scn:
	${CCF} -O -o idb_scn idb_scn.c
