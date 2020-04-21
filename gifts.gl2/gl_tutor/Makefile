
include ${ROOT}/usr/include/make/commondefs

TUTORDIR= /usr/people/tutorial
GLTUTDIR= ${TUTORDIR}/c.graphics

IDBTAG  = -idb "gltut usr"
GRPOWN  = -g demos -u tutor
IDBINST = ${INSTALL} ${GRPOWN} ${IDBTAG} -F ${GLTUTDIR}

MEXFILES= ismex whichmex
LCINCS  = -I${ROOT}/usr/include -I${ROOT}/usr/include/gl2
LLDOPTS = -Zg

default: all 

install: ${MEXFILES}
	${INSTALL} -g demos -u tutor -idb "upd usr" -dir ${TUTORDIR}
	cd src; make $@
	find explore ! -type d  ! -name "*,v" -print | \
	xargs |  \
	while read files_in; do \
	    ${IDBINST} -m 644 "$$files_in" ;\
	done
	${IDBINST} -m 755 "ismex whichmex"
	${IDBINST} ".mexrc .login .cshrc README ismex.c whichmex.c"
	${IDBINST} -src Makefile.usr Makefile

all:	${MEXFILES}
	cd src ; make $@
	cd explore ; make $@

clean:
	rm -f .[BC]* .em* *.o core
	cd src ; make $@
	cd explore ; make $@

clobber: clean
	rm -f whichmex ismex
	cd src ; make $@
	cd explore ; make $@
