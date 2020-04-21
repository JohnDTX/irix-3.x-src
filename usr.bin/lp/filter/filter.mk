#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp:filter/filter.mk	1.7"
#	filter make file

include ${ROOT}/usr/include/make/commondefs

RDIR	= ${ROOT}/usr/src/cmd/lp/filter
INS	= :
REL	= current
OWN	= lp
GRP	= lp
LIST	= lp
BIN	= /usr/lib
IFLAG	= -n
LCOPTS	= -O
LDOPTS	= -s $(IFLAG)
SOURCE	= hp2631a.c prx.c pprx.c 5310.c
FILES	= hp2631a.o prx.o pprx.o 5310.o
TARGETS	= hp2631a prx pprx 5310
MAKE	= make
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -g ${GRP} -u ${OWN}

compile all: $(TARGETS)

5310:	5310.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o 5310 5310.o

hp2631a:	hp2631a.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o hp2631a hp2631a.o

prx:	prx.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o prx prx.o

pprx:	pprx.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o pprx pprx.o


install:	all
	${INSTALL} ${GRPOWN} ${IDBTAG} -F ${BIN} -m 755 "${TARGETS}"

#######################################################################
#################################DSL only section - for development use

build:	bldmk
	get -p -r`gsid filter $(REL)` s.filter.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:  ;  get -p -r`gsid filter.mk $(REL)` s.filter.mk > $(RDIR)/filter.mk

listing:
	pr filter.mk $(SOURCE) | $(LIST)
listmk: ;  pr filter.mk | $(LIST)

edit:
	get -e -p s.filter.src | ntar -g

delta:
	ntar -p $(SOURCE) > filter.src
	delta s.filter.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.filter.mk
mkdelta: ;  delta s.filter.mk
#######################################################################

clean:
	rm -f $(FILES) core a.out

clobber:  clean
	rm -f $(TARGETS)

delete:	clobber
	rm -f $(SOURCE)
