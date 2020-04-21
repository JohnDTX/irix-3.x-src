#	/* @(#)uucp.mk	1.7 */
#	/*  11/45, 11/70, and VAX version ('-i' has no effect on VAX)	*/
#	/* for 11/23, 11/34 (without separate I/D), IFLAG= */
CC=cc
CFLAGS=-O 
LDFLAGS=
IFLAG = -i
LIBS=
FRC=
OWNER=uucp
INSDIR=/usr/lib/uucp
BIN=/usr/bin
UPATH=.
PUBDIR=/usr/spool/uucppublic
SPOOL=/usr/spool/uucp
XQTDIR=/usr/lib/uucp/.XQTDIR
OLDLOG=/usr/lib/uucp/.OLD
PKON=pkon.o
PKONSRC=pkon.c
LINE=line.o
LINESRC=line.c
IOCTL=
GETOPT=
CLEAN=
LINTOP=
COMMANDS=uucp uux uuxqt uucico uulog uuclean uuname uustat uusub
OFILES=assert.o cpmv.o expfile.o gename.o getpwinfo.o \
	prefix.o shio.o ulockf.o xqt.o logent.o versys.o gnamef.o rkill.o
LFILES=assert.c cpmv.c expfile.c gename.c getpwinfo.c \
	prefix.c shio.c ulockf.c xqt.c logent.o
OUUCP=uucpdefs.o uucp.o gwd.o chkpth.o getargs.o logent.o uucpname.o\
	versys.o us_crs.o
LUUCP=uucpdefs.c uucp.c gwd.c chkpth.c getargs.c logent.c uucpname.c\
	versys.c us_crs.c
OUUX=uucpdefs.o uux.o gwd.o anyread.o chkpth.o getargs.o getprm.o\
	logent.o uucpname.o versys.o
LUUX=uucpdefs.c uux.c gwd.c anyread.c chkpth.c getargs.c getprm.c\
	logent.c uucpname.c versys.c
OUUXQT=uucpdefs.o uuxqt.o mailst.o getprm.o gnamef.o logent.o uucpname.o \
	chkpth.o getargs.o anyread.o
LUUXQT=uucpdefs.c uuxqt.c mailst.c getprm.c gnamef.c logent.c uucpname.c \
	chkpth.c getargs.c anyread.c
OUUCICO=uucpdefs.o cico.o conn.o cntrl.o pk0.o pk1.o gio.o xio.o anyread.o \
	anlwrk.o chkpth.o getargs.o gnamef.o gnsys.o gnxseq.o \
	pkdefs.o imsg.o fwdchk.o  logent.o sysacct.o systat.o \
	gtcfile.o us_crs.o mailst.o uucpname.o us_rrs.o us_sst.o ub_sst.o $(LINE)
LUUCICO=uucpdefs.c cico.c conn.c cntrl.c pk0.c pk1.c gio.c xio.c anyread.c \
	anlwrk.c chkpth.c getargs.c gnamef.c gnsys.c gnxseq.c \
	$(PKONSRC) pkdefs.c imsg.c fwdchk.c logent.c sysacct.c systat.c \
	gtcfile.o us_crs.o mailst.c uucpname.c us_rrs.c us_sst.c ub_sst.c $(LINESRC)
OUULOG=uucpdefs.o uulog.o prefix.o xqt.o ulockf.o gnamef.o assert.o logent.o
LUULOG=uucpdefs.c uulog.c prefix.c xqt.c ulockf.c gnamef.c assert.c logent.o
OUUCLEAN=uucpdefs.o uuclean.o gnamef.o prefix.o mailst.o getpwinfo.o\
	 getargs.o assert.o
LUUCLEAN=uucpdefs.c uuclean.c gnamef.c prefix.c mailst.c getpwinfo.c\
	 getargs.c
OUUNAME=uuname.o uucpname.o uucpdefs.o getpwinfo.o
LUUNAME=uuname.c uucpname.c uucpdefs.c getpwinfo.c
OUUSTAT=uucpdefs.o uustat.o gnamef.o getpwinfo.o $(GETOPT) \
	cpmv.o ulockf.o assert.o logent.o
LUUSTAT=uucpdefs.c uustat.c gnamef.c getpwinfo.c \
	cpmv.c ulockf.c assert.c logent.o
OUUSUB=uucpdefs.o uusub.o getpwinfo.o xqt.o $(GETOPT)
LUUSUB=uucpdefs.c uusub.c getpwinfo.c xqt.c 
INIT=init

all:	$(INIT) $(COMMANDS) 

install:	all cp $(CLEAN)

new:		mkdirs cpfiles

cp:	all
	chown $(OWNER) $(INSDIR)
	chmod 755 $(INSDIR)
	-chmod u+w $(BIN)/uucp
	cp uucp $(BIN)
	chown $(OWNER) $(BIN)/uucp
	chmod 4111 $(BIN)/uucp
	-chmod u+w $(BIN)/uux
	cp uux $(BIN)
	chown $(OWNER) $(BIN)/uux
	chmod 4111 $(BIN)/uux
	-chmod u+w $(INSDIR)/uuxqt
	cp uuxqt $(INSDIR)
	chown $(OWNER) $(INSDIR)/uuxqt
	chmod 4111 $(INSDIR)/uuxqt
	-chmod u+w $(INSDIR)/uucico
	cp uucico $(INSDIR)
	chown $(OWNER) $(INSDIR)/uucico
	chmod 4111 $(INSDIR)/uucico
	-chmod u+w $(BIN)/uulog
	cp uulog $(BIN)
	chown $(OWNER) $(BIN)/uulog
	chmod 4111 $(BIN)/uulog
	-chmod u+w $(INSDIR)/uuclean
	cp uuclean $(INSDIR)
	chown $(OWNER) $(INSDIR)/uuclean
	chmod 4111 $(INSDIR)/uuclean
	-chmod u+w $(BIN)/uuname
	cp uuname $(BIN)
	chown $(OWNER) $(BIN)/uuname
	chmod 4111 $(BIN)/uuname
	-chmod u+w $(BIN)/uustat
	cp uustat $(BIN)
	chown $(OWNER) $(BIN)/uustat
	chmod 4111 $(BIN)/uustat
	-chmod u+w $(INSDIR)/uusub
	cp uusub $(INSDIR)
	chmod 100 $(INSDIR)/uusub
	chown $(OWNER) $(INSDIR)/uusub

save:	all
	chown $(OWNER) $(INSDIR)
	chmod 755 $(INSDIR)
	-mv $(BIN)/uucp $(BIN)/OLDuucp
	cp uucp $(BIN)
	chown $(OWNER) $(BIN)/uucp
	chmod 4111 $(BIN)/uucp
	-mv $(BIN)/uux $(BIN)/OLDuux
	cp uux $(BIN)
	chown $(OWNER) $(BIN)/uux
	chmod 4111 $(BIN)/uux
	-mv $(INSDIR)/uuxqt $(INSDIR)/OLDuuxqt
	cp uuxqt $(INSDIR)
	chown $(OWNER) $(INSDIR)/uuxqt
	chmod 4111 $(INSDIR)/uuxqt
	-mv $(INSDIR)/uucico $(INSDIR)/OLDuucico
	cp uucico $(INSDIR)
	chown $(OWNER) $(INSDIR)/uucico
	chmod 4111 $(INSDIR)/uucico
	-mv $(BIN)/uulog $(BIN)/OLDuulog
	cp uulog $(BIN)
	chown $(OWNER) $(BIN)/uulog
	chmod 4111 $(BIN)/uulog
	-mv $(INSDIR)/uuclean $(INSDIR)/OLDuuclean
	cp uuclean $(INSDIR)
	chown $(OWNER) $(INSDIR)/uuclean
	chmod 4111 $(INSDIR)/uuclean
	-mv $(BIN)/uuname $(BIN)/OLDuuname
	cp uuname $(BIN)
	chown $(OWNER) $(BIN)/uuname
	chmod 4111 $(BIN)/uuname
	-mv $(BIN)/uustat $(BIN)/OLDuustat
	cp uustat $(BIN)
	chown $(OWNER) $(BIN)/uustat
	chmod 4111 $(BIN)/uustat
	-mv $(INSDIR)/uusub $(INSDIR)/OLDuusub
	cp uusub $(INSDIR)
	chmod 100 $(INSDIR)/uusub
	chown $(OWNER) $(INSDIR)/uusub

restore:
	-chmod u+w $(BIN)/uucp
	-mv $(BIN)/OLDuucp $(BIN)/uucp
	chown $(OWNER) $(BIN)/uucp
	chmod 4111 $(BIN)/uucp
	-chmod u+w $(BIN)/uux
	-mv $(BIN)/OLDuux $(BIN)/uux
	chown $(OWNER) $(BIN)/uux
	chmod 4111 $(BIN)/uux
	-chmod u+w $(INSDIR)/uuxqt
	-mv $(INSDIR)/OLDuuxqt $(INSDIR)/uuxqt
	chown $(OWNER) $(INSDIR)/uuxqt
	chmod 4111 $(INSDIR)/uuxqt
	-chmod u+w $(INSDIR)/uucico
	-mv $(INSDIR)/OLDuucico $(INSDIR)/uucico
	chown $(OWNER) $(INSDIR)/uucico
	chmod 4111 $(INSDIR)/uucico
	-chmod u+w $(BIN)/uulog
	-mv $(BIN)/OLDuulog $(BIN)/uulog
	chown $(OWNER) $(BIN)/uulog
	chmod 4111 $(BIN)/uulog
	-chmod u+w $(INSDIR)/uuclean
	-mv $(INSDIR)/OLDuuclean $(INSDIR)/uuclean
	chown $(OWNER) $(INSDIR)/uuclean
	chmod 4111 $(INSDIR)/uuclean
	-chmod u+w $(BIN)/uuname
	-mv $(BIN)/OLDuuname $(BIN)/uuname
	chown $(OWNER) $(BIN)/uuname
	chmod 4111 $(BIN)/uuname
	-chmod u+w $(BIN)/uustat
	-mv $(BIN)/OLDuustat $(BIN)/uustat
	chown $(OWNER) $(BIN)/uustat
	chmod 4111 $(BIN)/uustat
	-chmod u+w $(INSDIR)/uusub
	-mv $(INSDIR)/OLDuusub $(INSDIR)/uusub
	chmod 100 $(INSDIR)/uusub
	chown $(OWNER) $(INSDIR)/uusub

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(COMMANDS)

burn:
	-rm -f $(BIN)/OLDuucp
	-rm -f $(BIN)/OLDuux
	-rm -f $(INSDIR)/OLDuuxqt
	-rm -f $(INSDIR)/OLDuucico
	-rm -f $(BIN)/OLDuulog
	-rm -f $(INSDIR)/OLDuuclean
	-rm -f $(BIN)/OLDuuname
	-rm -f $(BIN)/OLDuustat
	-rm -f $(INSDIR)/OLDuusub

get:
	cp $(UPATH)/* .

cmp:	all
	cmp uucp $(BIN)
	rm uucp
	cmp uux $(BIN)
	rm uux
	cmp uuxqt $(INSDIR)/uuxqt
	rm uuxqt
	cmp uucico $(INSDIR)/uucico
	rm uucico
	cmp uulog $(BIN)
	rm uulog
	cmp uuclean $(INSDIR)/uuclean
	rm uuclean
	cmp uuname $(BIN)
	rm uuname
	cmp uustat $(BIN)
	rm uustat
	cmp uusub $(INSDIR)
	rm uusub
	rm *.o


init:	anlwrk.o anyread.o chkpth.o cpmv.o expfile.o gename.o \
	getargs.o getprm.o getpwinfo.o gnamef.o gnsys.o \
	gnxseq.o gwd.o imsg.o logent.o \
	prefix.o mailst.o shio.o sysacct.o \
	$(GETOPT) systat.o ulockf.o uucpname.o versys.o xqt.o

uucp:	$(OUUCP) $(OFILES)
	$(CC) $(LDFLAGS) $(OUUCP) $(OFILES) $(LIBS) -o uucp

uux:	$(OUUX) $(OFILES)
	$(CC) $(LDFLAGS) $(OUUX) $(OFILES) $(LIBS) -o uux

uuxqt:	$(OUUXQT) $(OFILES)
	$(CC) $(LDFLAGS) $(OUUXQT) $(OFILES) $(LIBS) -o uuxqt

uucico:	$(OUUCICO) $(OFILES) $(IOCTL) $(PKON)
	$(CC) $(IFLAG) $(LDFLAGS) $(OUUCICO) $(OFILES) $(IOCTL) $(PKON) $(LIBS)\
	-o uucico

uulog:	$(OUULOG)
	$(CC) $(LDFLAGS) $(OUULOG) $(LIBS) -o uulog

uuclean:  $(OUUCLEAN) $(OFILES)
	$(CC) $(LDFLAGS) $(OUUCLEAN) $(OFILES) $(LIBS) -o uuclean

uuname:	$(OUUNAME)
	$(CC) $(LDFLAGS) $(OUUNAME) $(LIBS) -o uuname
 
uustat:	$(OUUSTAT) $(OFILES)
	$(CC) $(LDFLAGS) $(OUUSTAT) $(OFILES) $(LIBS) -o uustat

uusub:	$(OUUSUB)
	$(CC) $(LDFLAGS) $(OUUSUB) $(LIBS) -o uusub


ub_sst.o uusub.o:	uusub.h

cico.o:	uusub.h uust.h uucp.h

anlwrk.o cntrl.o us_crs.o us_rrs.o\
	uuclean.o rkill.o us_sst.o uucp.o uustat.o:	uust.h uucp.h

anyread.o assert.o chkpth.o cico.o conn.o cpmv.o expfile.o gename.o\
	getpwinfo.o gio.o fwdch.o xio.o gnamef.o gnsys.o gnxseq.o gwd.o imsg.o ioctl.o\
	logent.o mailst.o sdmail.o $(LINE) shio.o\
	systat.o ulockf.o uuclean.o uucpdefs.o uucpname.o uulog.o uuname.o\
	uux.o uuxqt.o versys.o xqt.o:	uucp.h

FRC:

mkdirs:
	-mkdir $(INSDIR)
	chmod 755 $(INSDIR)
	-mkdir $(SPOOL)
	chmod 777 $(SPOOL)
	chown $(OWNER) $(SPOOL)
	-mkdir $(PUBDIR)
	chmod 777 $(PUBDIR)
	chown $(OWNER) $(PUBDIR)
	-mkdir $(XQTDIR)
	chmod 777 $(XQTDIR)
	chown $(OWNER) $(XQTDIR)
	-mkdir $(OLDLOG)
	chmod 777 $(OLDLOG)
	chown $(OWNER) $(OLDLOG)

cpfiles:
	cp $(UPATH)/L* $(UPATH)/USERFILE $(INSDIR)
	cp $(UPATH)/uudemon* $(INSDIR)
	chmod 755 $(INSDIR)/uudemon*
	chmod 400 $(INSDIR)/L.sys $(INSDIR)/USERFILE
	chmod 444 $(INSDIR)/L-*
	chown $(OWNER) $(INSDIR)/*

#  lint procedures

lint:	lintuucp lintuucico lintuux lintuuxqt lintuulog lintuuclean\
	lintuuname lintuustat lintuusub
lintuucp:
	lint $(LINTOP) $(LUUCP) $(LFILES)

lintuucico:
	lint $(LINTOP) $(LUUCICO) $(LFILES)

lintuux:
	lint $(LINTOP) $(LUUX) $(LFILES)

lintuuxqt:
	lint $(LINTOP) $(LUUXQT) $(LFILES)

lintuulog:
	lint $(LINTOP) $(LUULOG)

lintuuclean:
	lint $(LINTOP) $(LUUCLEAN)

lintuuname:
	lint $(LINTOP) $(LUUNAME)

lintuustat:
	lint $(LINTOP) $(LUUSTAT)

lintuusub:
	lint $(LINTOP) $(LUUSUB)

