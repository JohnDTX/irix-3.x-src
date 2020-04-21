#	text subsystem shells make file
#	SCCS:  @(#)shells.mk	1.18

OL = /
SL = /usr/src/cmd
RDIR = ${SL}/text/shells.d
INS = :
REL = current
MKSID = -r`gsid shells.mk ${REL}`
MMSID = -r`gsid mm.sh ${REL}`
MMTSID = -r`gsid mmt.sh ${REL}`
MANSID = -r`gsid man.sh ${REL}`
MVTSID = -r`gsid mvt.sh ${REL}`
ORGSID = -r`gsid org.sh ${REL}`
OSDDSID = -r`gsid osdd.sh ${REL}`
GSID = -r`gsid greek.sh ${REL}`
TERMHSID = -r`gsid termhelp ${REL}`
TEXTHSID = -r`gsid texthelp ${REL}`
LIST = lp
CINSDIR = ${OL}usr/bin
HINSDIR = ${OL}usr/lib/help
FILES = mm.sh mmt.sh man.sh mvt.sh org.sh osdd.sh greek.sh term text
MAKE = make

compile all:  man mm mmt mvt osdd greek termh texth
	:

man:	;  cp man.sh man
	${INS} man ${CINSDIR}
	chmod 755 ${CINSDIR}/man
	@if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin man; chgrp bin man; \
	 fi

mm:	;  cp mm.sh mm
	${INS} mm ${CINSDIR}
	chmod 755 ${CINSDIR}/mm
	@if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin mm; chgrp bin mm; \
	 fi

mmt:	;  cp mmt.sh mmt
	${INS} mmt ${CINSDIR}
	chmod 755 ${CINSDIR}/mmt
	@if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin mmt; chgrp bin mmt; \
	 fi

mvt:	mmt
	rm -f ${CINSDIR}/mvt
	ln ${CINSDIR}/mmt ${CINSDIR}/mvt
	chmod 755 ${CINSDIR}/mvt
	@if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin mvt; chgrp bin mvt; \
	 fi

org:	;  cp org.sh org
	   ${INS} org ${CINSDIR}
	   chmod 755 ${CINSDIR}/org
	   @if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin org; chgrp bin org; \
	    fi
osdd:	;  cp osdd.sh osdd
	   ${INS} osdd ${CINSDIR}
	   chmod 755 ${CINSDIR}/osdd
	   @if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin osdd; chgrp bin osdd; \
	    fi
greek:	;  cp greek.sh greek
	${INS} greek ${CINSDIR}
	chmod 755 ${CINSDIR}/greek
	@if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin greek; chgrp bin greek; \
	 fi

helpdir:
	-mkdir ${OL}usr/lib/help

termh:	helpdir
	${INS} term ${HINSDIR}
	chmod 644 ${HINSDIR}/term
	@if [ "${OL}" = "/" ]; \
	    then cd ${HINSDIR}; chown bin term; chgrp bin term; \
	 fi

texth:	helpdir
	${INS} text ${HINSDIR}
	chmod 644 ${HINSDIR}/text
	@if [ "${OL}" = "/" ]; \
	    then cd ${HINSDIR}; chown bin text; chgrp bin text; \
	 fi

install:
	${MAKE} -f shells.mk INS=cp OL=${OL}
insmm:	;  ${MAKE} -f shells.mk INS=cp OL=${OL} mm
insmmt:	;  ${MAKE} -f shells.mk INS=cp OL=${OL} mmt
insman:	;  ${MAKE} -f shells.mk INS=cp OL=${OL} man
insmvt:	;  ${MAKE} -f shells.mk INS=cp OL=${OL} mvt
insorg:	;  ${MAKE} -f shells.mk INS=cp OL=${OL} org
insosdd: ; ${MAKE} -f shells.mk INS=cp OL=${OL} osdd
insgreek: ; ${MAKE} -f shells.mk INS=cp OL=${OL} greek
instermh: ; ${MAKE} -f shells.mk INS=cp OL=${OL} termh
instexth: ; ${MAKE} -f shells.mk INS=cp OL=${OL} texth

listing:  ;  pr shells.mk ${FILES}  |  ${LIST}
listmk:    ;  pr shells.mk | ${LIST}
listmm:    ;  pr mm.sh | ${LIST}
listmmt:   ;  pr mmt.sh | ${LIST}
listman:   ;  pr man.sh | ${LIST}
listmvt:   ;  pr -h "linked copy of mmt - mvt" mvt.sh | ${LIST}
listorg:   ;  pr org.sh | ${LIST}
listosdd:  ;  pr osdd.sh | ${LIST}
listgreek: ;  pr greek.sh | ${LIST}
listtermh: ; pr term | ${LIST}
listtexth: ; pr text | ${LIST}

build:  bldmk bldmm bldmmt bldman bldosdd bldgreek bldtermh bldtexth
	:
bldmk:  ;  get -p ${MKSID} s.shells.mk > ${RDIR}/shells.mk
bldmm:	;  get -p ${MMSID} s.mm.sh ${REWIRE} > ${RDIR}/mm.sh
bldmmt bldmvt:	;  get -p ${MMTSID} s.mmt.sh ${REWIRE} > ${RDIR}/mmt.sh
bldman:	;  get -p ${MANSID} s.man.sh ${REWIRE} > ${RDIR}/man.sh
bldorg:	;  get -p ${ORGSID} s.org.sh ${REWIRE} > ${RDIR}/org.sh
bldosdd: ; get -p ${OSDDSID} s.osdd.sh ${REWIRE} > ${RDIR}/osdd.sh
bldgreek: ; get -p ${GSID} s.greek.sh ${REWIRE} > ${RDIR}/greek.sh
bldtermh: ; get -p ${TERMHSID} s.term > ${RDIR}/term
bldtexth: ; get -p ${TEXTHSID} s.text > ${RDIR}/text

edit:	manedit mmedit mmtedit orgedit osddedit greekedit \
		termhedit texthedit mkedit
	:
manedit:  ;  get -e s.man.sh
mmedit:   ;  get -e s.mm.sh
mmtedit mvtedit:  ;  get -e s.mmt.sh
orgedit:  ;  get -e s.org.sh
osddedit: ;  get -e s.osdd.sh
greekedit: ; get -e s.greek.sh
termhedit: ; get -e s.term
texthedit: ; get -e s.text
mkedit:   ;  get -e s.shells.mk

delta:	mandelta mmdelta mmtdelta orgdelta osdddelta greekdelta \
		termhdelta texthdelta  mkdelta
	:
mandelta:  ;  delta s.man.sh
mmdelta:   ;  delta s.mm.sh
mmtdelta mvtdelta:  ;  delta s.mmt.sh
orgdelta:  ;  delta s.org.sh
osdddelta: ;  delta s.osdd.sh
greekdelta: ; delta s.greek.sh
termhdelta: ; delta s.term
texthdelta: ; delta s.text
mkdelta:   ;  delta s.shells.mk

clean:
	:
clobber:  clean manclobber mmclobber mmtclobber orgclobber \
		osddclobber greekclobber
	:
manclobber:  ;  rm -f man
mmclobber:   ;  rm -f mm
mmtclobber mvtclobber:  ;  rm -f mmt
orgclobber:  ;  rm -f org
osddclobber: ;  rm -f osdd
greekclobber: ; rm -f greek
delete:  clobber mandelete mmdelete mmtdelete orgdelete osdddelete \
		greekdelete termhdelete texthdelete
	:
mandelete:	manclobber
	rm -f man.sh
mmdelete:	mmclobber
	rm -f mm.sh
mmtdelete mvtdelete:	mmtclobber mvtclobber
	rm -f mmt.sh
orgdelete:	orgclobber
	rm -f org.sh
osdddelete:	osddclobber
	rm -f osdd.sh
greekdelete:	greekclobber
	rm -f greek.sh
termhdelete: ; rm -f term
texthdelete: ; rm -f text
