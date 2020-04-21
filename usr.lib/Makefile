
#
#
include ${ROOT}/usr/include/make/commondefs
#
#

DIRS	= libPW libcrypt libcurses libl libplot libstand libtermcap liby \
	  libg libm libmsky libmfpa liboldmalloc
PROGS	= atrun clstat.o manprog unittab
FILES	= eign lib.b make.h
#makekey can't be shipped internationally.

BOOTDIRS = libPW libcrypt libl libm liby
BOOTPROGS = clstat.o
BOOTFILES = make.h

#	termcap done in bin/vi

all:	${PROGS} ${FILES} FRC
	for i in ${DIRS); do (cd $$i; make $@); done

install: ${PROGS} ${FILES} FRC
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix" "${PROGS}"
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix" "${FILES}"
	for i in ${DIRS}; do (cd $$i; echo 'Making' $$i 'Dir. '; make $@); done

clean:
	rm -f *.o a.out core y.tab.c y.tab.h
	for i in ${DIRS}; do (cd $$i; echo $$i; make $@); done

clobber:
	rm -f *.o a.out core y.tab.c y.tab.h
	rm -f ${PROGS}
	for i in ${DIRS}; do (cd $$i; echo $$i; make $@); done

${DIRS} DIRS: FRC
	echo ${cd $@; make all

FRC:
