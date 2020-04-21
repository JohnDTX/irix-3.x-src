#	checkmm make file
#	SCCS:	@(#)checkmm.mk	1.7

OL = /
SL = /usr/src/cmd
RDIR = ${SL}/text/checkmm
INS = :
REL = current
CSID = -r`gsid checkmm ${REL}`
MKSID = -r`gsid checkmm.mk ${REL}`
LIST = lp
INSDIR = ${OL}usr/bin
B10 =
CFLAGS = -O ${FFLAG} ${B10}
IFLAG = -n
SOURCE = chekl.l chekmain.c
FILES = chekl.o chekmain.o
MAKE = make

compile all: checkmm
	:

checkmm:	$(FILES)
	$(CC) -s ${B10} ${IFLAG} -o checkmm $(FILES) $$DEST/usr/lib/libl.a $$DEST/usr/lib/libPW.a
	$(INS) checkmm $(INSDIR)
	chmod 755 ${INSDIR}/checkmm
	@if [ "${OL}" = "/" ]; \
		then cd ${INSDIR}; chown bin checkmm; chgrp bin checkmm; \
	 fi

$(FILES)::
	:

install:
	${MAKE} -f checkmm.mk INS=cp OL=${OL}

build:	bldmk
	get -p ${CSID} s.checkmm.src ${REWIRE} | ntar -d ${RDIR} -g
bldmk:  ;  get -p ${MKSID} s.checkmm.mk > ${RDIR}/checkmm.mk

listing:
	pr checkmm.mk ${SOURCE} | ${LIST}
listmk: ;  pr checkmm.mk | ${LIST}

edit:
	get -e -p s.checkmm.src | ntar -g

delta:
	ntar -p ${SOURCE} > checkmm.src
	delta s.checkmm.src
	rm -f ${SOURCE}

mkedit:  ;  get -e s.checkmm.mk
mkdelta: ;  delta s.checkmm.mk

clean:
	  rm -f ${FILES}

clobber:  clean
	  rm -f checkmm

delete:	clobber
	rm -f ${SOURCE}
