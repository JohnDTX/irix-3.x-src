#	eqn make file (text subsystem)
#	SCCS:  @(#)eqn.mk	1.16

OL = /
SL = /usr/src/cmd
RDIR = ${SL}/text/eqn.d
INS = :
REL = current
CSID = -r`gsid eqn ${REL}`
MKSID = -r`gsid eqn.mk ${REL}`
LIST = lp
CINSDIR = ${OL}usr/bin
TINSDIR = ${OL}usr/pub
B20 =
CFLAGS = -O ${B20}
IFLAG = -n
YFLAGS = -d
SOURCE = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
	 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
	 pile.c shift.c size.c sqrt.c text.c
OFILES =  diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
	 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
	 pile.o shift.o size.o sqrt.o text.o
FILES =  ${OFILES} e.o
MAKE = make

compile all: eqn eqnch
	:

eqn:	$(FILES)
	$(CC) -s ${IFLAG} ${FFLAG} -o eqn $(FILES) $$DEST/usr/lib/liby.a
	$(INS) eqn $(CINSDIR)
	chmod 755 ${CINSDIR}/eqn
	@if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin eqn; chgrp bin eqn; \
	 fi

$(OFILES):: e.h e.def
	:

e.def:	  y.tab.h
	  -cmp -s y.tab.h e.def || cp y.tab.h e.def

y.tab.h:  e.o
	:

eqnch:	eqnchar
	${INS} eqnchar ${TINSDIR}
	chmod 664 ${TINSDIR}/eqnchar
	@if [ "${OL}" = "/" ] ; \
		then cd ${TINSDIR}; chown bin eqnchar; chgrp bin eqnchar; \
	 fi

install:
	${MAKE} -f eqn.mk INS=cp OL=${OL}
inseqnch:
	${MAKE} -f eqn.mk INS=cp OL=${OL} eqnch

build:	bldmk bldeqnch
	get -p ${CSID} s.eqn.src ${REWIRE} | ntar -d ${RDIR} -g
bldeqnch: ; get -p -r`gsid eqnchar ${REL}` s.eqnchar > ${RDIR}/eqnchar
bldmk:  ;  get -p ${MKSID} s.eqn.mk > ${RDIR}/eqn.mk

listing:
	pr eqn.mk eqnchar ${SOURCE} | ${LIST}
listeqnch: ; pr eqnchar | ${LIST}
listmk: ;  pr eqn.mk | ${LIST}

edit:
	get -e -p s.eqn.src | ntar -g

delta:
	ntar -p ${SOURCE} > eqn.src
	delta s.eqn.src
	rm -f ${SOURCE}

eqnchedit: ; get -e s.eqnchar
eqnchdelta: ; delta s.eqnchar

mkedit:  ;  get -e s.eqn.mk
mkdelta: ;  delta s.eqn.mk

clean:
	  rm -f *.o y.tab.h e.def

clobber:  clean
	  rm -f eqn

delete:	clobber
	rm -f ${SOURCE} eqnchar
