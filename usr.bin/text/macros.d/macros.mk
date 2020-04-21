#	text Development Support Library (DSL) macros make file
#	SCCS: @(#)macros.mk	1.31

OL = /
SL = /usr/src/cmd
RDIR = ${SL}/text/macros.d
REL = current
NTARSID = -r`gsid ntar ${REL}`
MMNSID = -r`gsid mmn ${REL}`
MMTSID = -r`gsid mmt ${REL}`
VMSID = -r`gsid vmca ${REL}`
ORGSID = -r`gsid org ${REL}`
OSDDSID = -r`gsid osdd ${REL}`
MANSID = -r`gsid manmacs ${REL}`
TMSID = -r`gsid tmacs ${REL}`
MACRUNCHSID = -r`gsid macrunch ${REL}`
MKSID = -r`gsid macros.mk ${REL}`
LIST = lp
INSMAC = ${OL}usr/lib/macros
INSTMAC = ${OL}usr/lib/tmac
TMACFILES = tmac.an tmac.m tmac.org tmac.ptx tmac.osd tmac.v
MAKE = make
IFLAG = -n

compile all:	ntar mmn mmt vmca osdd man tmac ptx
	:
ntar:	ntar.c
	${CC} ${IFLAG} -O -s -o ntar ntar.c
mmn:	ntar
	sh ./macrunch -n m ${MINS} -d ${OL}usr mmn

mmt:	ntar
	sh ./macrunch -t m ${MINS} -d ${OL}usr mmt

vmca:	ntar
	sh ./macrunch ${MINS} -d ${OL}usr vmca

org:	ntar
	sh ./macrunch ${MINS} -d ${OL}usr org

osdd:	ntar
	sh ./macrunch ${MINS} -d ${OL}usr osdd

man:	ntar
	sh ./macrunch -n an ${MINS} -d ${OL}usr an
	sh ./macrunch -t an ${MINS} -d ${OL}usr an

ptx:	ntar
	sh ./macrunch ${MINS} -d ${OL}usr ptx

tmac:
	cp ${TMACFILES} ${INSTMAC}
	cd ${INSTMAC}; chmod 644 ${TMACFILES}
	@if [ "${OL}" = "/" ]; \
		then cd ${INSTMAC}; chown bin ${TMACFILES}; chgrp bin ${TMACFILES}; \
	 fi

build:  bldmk bldntar bldmmn bldmmt bldvmca bldosdd bldman bldtmac \
		bldmacr bldptx
	:
bldntar: ;  get -p ${NTARSID} s.ntar.c > ${RDIR}/ntar.c
bldmk:   ;  get -p ${MKSID} s.macros.mk > ${RDIR}/macros.mk
bldmmn:  ;  get -p ${MMNSID} s.mmn.src ${REWIRE} > ${RDIR}/mmn.src
bldmmt:  ;  get -p ${MMTSID} s.mmt.src ${REWIRE} > ${RDIR}/mmt.src
bldvmca: ;  get -p ${VMSID} s.vmca.src ${REWIRE} > ${RDIR}/vmca.src
bldorg:  ;  get -p ${ORGSID} s.org.src ${REWIRE} > ${RDIR}/org.src
bldosdd: ;  get -p ${OSDDSID} s.osdd.src ${REWIRE} > ${RDIR}/osdd.src
bldman:  ;  get -p ${MANSID} s.an.src ${REWIRE} > ${RDIR}/an.src
bldptx:  ;  get -p -r`gsid ptxmacs ${REL}` s.ptx.src ${REWIRE} > ${RDIR}/ptx.src
bldtmac: ;  get -p ${TMSID} s.tmacs.src ${REWIRE} | ntar -d ${RDIR} -g
bldmacr: ;  get -p ${MACRUNCHSID} s.macrunch > ${RDIR}/macrunch
	    chmod 755 ${RDIR}/macrunch

listing:  listmk listntar listmmn listmmt listvmca listorg listosdd \
		listman listtmac listmacr listptx
	:
listntar: ; pr ntar.c | ${LIST}
listmk:  ;  pr macros.mk | ${LIST}
listmmn: ;  nl -ba mmn.src | pr -h "mmn.src" | ${LIST}
	    macref -s -t mmn.src | pr -h "macref of mmn.src" | ${LIST}
listmmt: ;  nl -ba mmt.src | pr -h "mmt.src" | ${LIST}
	    macref -s -t mmt.src | pr -h "macref of mmt.src" | ${LIST}
listvmca: ; nl -ba vmca.src | pr -h "vmca.src" | ${LIST}
	    macref -s -t vmca.src | pr -h "macref of vmca.src" | ${LIST}
listorg: ;  nl -ba org.src | pr -h "org.src" | ${LIST}
	    macref -s -t org.src | pr -h "macref of org.src" | ${LIST}
listosdd: ; nl -ba osdd.src | pr -h "osdd.src" | ${LIST}
	    macref -s -t osdd.src | pr -h "macref of osdd.src" | ${LIST}
listman: ;  nl -ba an.src | pr -h "an.src" | ${LIST}
	    macref -s -t an.src | pr -h "macref of an.src" | ${LIST}
listptx: ;  nl -ba ptx.src | pr -h "ptx.src" | ${LIST}
	    macref -s -t ptx.src | pr -h "macref of ptx.src" | ${LIST}
listtmac: ; pr ${TMACFILES} | ${LIST}
listmacr: ; pr macrunch | ${LIST}

install:
	${MAKE} -f macros.mk MINS=-m OL=${OL}

insmmn:  ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} mmn
insmmt:  ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} mmt
insvmca: ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} vmca
insorg:  ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} org
insosdd: ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} osdd
insman:  ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} man
insptx:  ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} ptx
instmac: ;  ${MAKE} -f macros.mk MINS=-m OL=${OL} tmac

edit:	ntaredit mmnedit mmtedit vmcaedit orgedit osddedit manedit \
		tmacsedit macredit mkedit ptxedit
	:
ntaredit: ;  get -e s.ntar.c
mmnedit:  ;  get -e s.mmn.src
mmtedit:  ;  get -e s.mmt.src
vmcaedit: ;  get -e s.vmca.src
orgedit:  ;  get -e s.org.src
osddedit: ;  get -e s.osdd.src
manedit:  ;  get -e s.an.src

ptxedit:  ;  get -e s.ptx.src
tmacsedit: ; get -e -p s.tmacs.src | ntar -g
macredit: ;  get -e s.macrunch
mkedit:   ;  get -e s.macros.mk

delta:	ntardelta mmndelta mmtdelta vmcadelta orgdelta osdddelta \
		mandelta tmacsdelta macrdelta mkdelta ptxdelta
	:
ntardelta: ;  delta s.ntar.c
mmndelta:  ;  delta s.mmn.src
mmtdelta:  ;  delta s.mmt.src
vmcadelta: ;  delta s.vmca.src
orgdelta:  ;  delta s.org.src
osdddelta: ;  delta s.osdd.src
mandelta:  ;  delta s.an.src
ptxdelta:  ;  delta s.ptx.src
tmacsdelta: ; ntar -p ${TMACFILES} > tmacs.src
	      delta s.tmacs.src
	      rm -f ${TMACFILES}
macrdelta: ;  delta s.macrunch
mkdelta:   ;  delta s.macros.mk

clean:
	:
clobber:  clean ntarclobber mmnclobber mmtclobber vmcaclobber \
		orgclobber osddclobber manclobber ptxclobber
	:
ntarclobber: ;  rm -f ntar
mmnclobber:  ;  rm -f mmn
mmtclobber:  ;  rm -f mmt
vmcaclobber: ;  rm -f vmca
orgclobber:  ;  rm -f org
osddclobber: ;  rm -f osdd
manclobber:  ;  rm -f an
ptxclobber:  ;  rm -f ptx

delete:	clobber ntarclobber mmndelete mmtdelete vmcadelete orgdelete \
		osdddelete mandelete tmacsdelete macrdelete ptxdelete
	:
ntardelete: ntarclobber
	    rm -f ntar.c
mmndelete:  mmnclobber
	    rm -f mmn.src
mmtdelete:  mmtclobber
	    rm -f mmt.src
vmcadelete: vmcaclobber
	    rm -f vmca.src
orgdelete:  orgclobber
	    rm -f org.src
osdddelete: osddclobber
	    rm -f osdd.src
mandelete:  manclobber
	    rm -f an.src
ptxdelete:  ptxclobber
	    rm -f ptx.src
tmacsdelete: ;  rm -f ${TMACFILES}
macrdelete: ;  	rm -f macrunch
