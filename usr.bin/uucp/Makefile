# makefile for UUCP

include ${ROOT}/usr/include/make/commondefs

# @(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/Makefile,v 1.1 89/03/27 18:30:05 root Exp $

INSCMD	=$(INSTALL) -u uucpadm -g uucpadm -idb "usr upd"
INSNOUP	=$(INSTALL) -u uucpadm -g uucpadm -idb "usr upd config(noupdate)" 
INSSUG	=$(INSTALL) -u uucpadm -g uucpadm -idb "usr upd config(suggest)" 

LIB	= /usr/lib/uucp
LIBXNS	= -lxns
BIN	= /usr/bin
SPOOL	= /usr/spool/uucp
XQTDIR	= /usr/lib/uucp/.XQTDIR /usr/spool/uucp/.XQTDIR \
	  /usr/spool/uucppublic /usr/spool/uucppublic/receive
PKON	= pkon.o

DIRS	= $(LIB) $(SPOOL) $(XQTDIR)

COMMANDS= $(COMBIN) $(COMLIB)

COMBIN	= uucp uulog uuname uustat uusub uux uuencode uudecode

COMLIB	= uucico uuclean uuxqt

DATAFIL	= L.cmds L-devices L-dialcodes L.sys \
	USERFILE uudemon.day uudemon.hr uudemon.wk

ZLIB	=  L_stat L_sub R_stat R_sub SEQF

ZSPOOL	= AUDIT LOGFILE SYSLOG ERRLOG oAUDIT oLOGFILE oSYSLOG oERRLOG

HFILES	= uucp.h uucpdefs.h

OFILES	= assert.o cpmv.o expfile.o gename.o getpwinfo.o \
	prefix.o shio.o ulockf.o xqt.o

OUUCP	= uucpdefs.o uucp.o gwd.o chkpth.o getargs.o uucpname.o logent.o\
	versys.o us_crs.o

OUUX	= uucpdefs.o uux.o gwd.o anyread.o chkpth.o getargs.o getprm.o\
	uucpname.o logent.o versys.o

OUUXQT	= uucpdefs.o uuxqt.o mailst.o getprm.o gnamef.o uucpname.o \
	logent.o chkpth.o getargs.o anyread.o

OUUCICO	= uucpdefs.o cico.o conn.o cntrl.o pk0.o pk1.o gio.o anyread.o \
	anlwrk.o chkpth.o getargs.o gnamef.o gnsys.o gnxseq.o \
	imsg.o sysacct.o systat.o \
	mailst.o uucpname.o logent.o us_crs.o us_rrs.o us_sst.o ub_sst.o \
	line.o pkdefs.o gtcfile.o xio.o fwdchk.o versys.o

OUULOG	= uucpdefs.o uulog.o prefix.o xqt.o ulockf.o gnamef.o assert.o logent.o

OUUCLEAN= uucpdefs.o uuclean.o gnamef.o prefix.o mailst.o getpwinfo.o\
	 getargs.o assert.o

OUUNAME	= uuname.o uucpname.o logent.o uucpdefs.o

OUUSTAT	= uucpdefs.o uustat.o gnamef.o getpwinfo.o cpmv.o \
	assert.o ulockf.o logent.o

OUUSUB	= uucpdefs.o uusub.o getpwinfo.o xqt.o

LLDLIBS	= -lsun -lbsd -ldbm

all:	$(COMMANDS) ${DATAFIL}

install:all
	${INSCMD} -m 755 -dir "$(LIBS) $(SPOOL)"
	${INSCMD} -m 777 -dir "$(XQTDIR)"
	${INSCMD} -m 4111 -F ${LIB} "uucico uuxqt"
	${INSCMD} -m 111 -F ${LIB} "uuclean"
	${INSSUG} -m 400 -F ${LIB} "L.sys L.cmds USERFILE"
	${INSSUG} -m 444 -F ${LIB} "L-devices L-dialcodes"
	${INSNOUP} -m 755 -F ${LIB} "uudemon.day uudemon.hr uudemon.wk" 
	${INSNOUP} -m 644 -F ${LIB} "${ZLIB}"
	${INSNOUP} -m 600 -F ${LIB} "SEQF"
	${INSNOUP} -m 644 -F ${SPOOL} "${ZSPOOL}"
	${INSCMD} -m 4111 -F ${BIN} "${COMBIN}"


logfiles:
	touch SEQF ${ZSPOOL} ${ZLIB}

uucp:	$(OUUCP) $(OFILES) 
	$(CCF) $(OUUCP) $(OFILES) -o $@ $(LDFLAGS)

uux:	$(OUUX) $(OFILES) 
	$(CCF) $(OUUX) $(OFILES) -o $@ $(LDFLAGS)

uuxqt:	$(OUUXQT) $(OFILES) 
	$(CCF) $(OUUXQT) $(OFILES) -o $@ $(LDFLAGS)

uucico:	$(OUUCICO) $(OFILES) $(PKON)
	$(CCF) $(OUUCICO) $(OFILES) $(PKON) $(LIBXNS) -o $@ $(LDFLAGS)

uulog:	$(OUULOG)
	$(CCF) $(OUULOG) -o $@ $(LDFLAGS)

uuclean:$(OUUCLEAN)
	$(CCF) $(OUUCLEAN) -o $@ $(LDFLAGS)

uuname: $(OUUNAME) 
	$(CCF) $(OUUNAME) -o $@ $(LDFLAGS)

uustat:	$(OUUSTAT) 
	$(CCF) $(OUUSTAT) -o $@ $(LDFLAGS)

uusub:	$(OUUSUB) 
	$(CCF) $(OUUSUB) -o $@ $(LDFLAGS)


clean:
	rm -f *.o

clobber:clean
	rm -f $(COMMANDS)

