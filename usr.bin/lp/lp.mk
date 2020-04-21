# Makefile for lp line printer spooler system
#
# base level delta lp.mk  2.1
#
# for the DSL @(#)lp.mk	1.3
#


OL	= /
SL	= /usr/src/cmd
RDIR	= $(SL)/lp
REL	= current
LIST	= lp
ALL	= accept cancel disable enable lp lpadmin lpmove lpsched lpshut lpstat reject
SPOOL	= $(OL)usr/spool/lp
ADMIN	= lp
ADMDIR	= $(OL)usr/lib
USRDIR	= $(OL)usr/bin
GROUP	= bin
LIB	= lib.a
CFLAGS	= -O
DEFS	= -DSPOOL='"$(SPOOL)"' -DADMIN='"$(ADMIN)"'\
	  -DADMDIR='"$(ADMDIR)"' -DUSRDIR='"$(USRDIR)"'
LDFLAGS	= -s -n
COMPILE	= $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIB) $(DEFS)
INSUSR	= $(INS) cp $@ $(USRDIR);\
	  $(INS) if [ "$(OL)" = "/" ]; \
	      $(INS) then \
	      $(INS) chown $(ADMIN) $(USRDIR)/$@;\
	      $(INS) chgrp $(GROUP) $(USRDIR)/$@;\
	      $(INS) chmod 6775 $(USRDIR)/$@; \
	  $(INS) fi
INSADM	= $(INS) cp $@ $(ADMDIR);\
	  $(INS) if [ "$(OL)" = "/" ]; \
	      $(INS) then \
	      $(INS) chown $(ADMIN) $(ADMDIR)/$@;\
	      $(INS) chgrp $(GROUP) $(ADMDIR)/$@;\
	      $(INS) chmod 6775 $(ADMDIR)/$@; \
	  $(INS) fi
INS	= :
LFILES	= dest.c destlist.c fullpath.c gwd.c enter.c fifo.c getname.c \
	  isclass.c isprinter.c isrequest.c outlist.c outputq.c pstatus.c \
	  date.c isdest.c eacross.c qstatus.c fatal.c lock.c request.c \
	  sendmail.c trim.c wrtmsg.c findtty.c
SFILES	= accept.c cancel.c disable.c enable.c lp.c lpadmin.c lpmove.c \
	  lpsched.c lpshut.c lpstatus.c reject.c
HFILES	= lp.h lpsched.h

all:	new $(ALL)

.c.a:

.PRECIOUS: $(LIB)

#install:
#	$(MAKE) -f lp.mk $(ARGS) INS= \
#		SPOOL='$(SPOOL)' ADMDIR='$(ADMDIR)' GROUP='$(GROUP)' \
#		USRDIR='$(USRDIR)' ADMIN='$(ADMIN)' LIB='$(LIB)' \
#		CFLAGS='$(CFLAGS)' LDFLAGS='$(LDFLAGS)'

new:
	if [ ! -d $(SPOOL) ] ; \
	then \
		set -e ; \
		mkdir $(SPOOL) $(SPOOL)/model ; \
		cp model/* $(SPOOL)/model ; \
		cd $(SPOOL) ; \
		mkdir class interface member request ; \
		chmod 755 . class interface member model request ; \
		chmod 644 model/* ; \
		>pstatus >qstatus ; \
		chmod 644 pstatus qstatus ; \
		if [ "$(OL)" = "/" ]; \
		  then chown $(ADMIN) . * model/* ; \
		       chgrp $(GROUP) . * model/* ; \
		fi;  \
	fi

accept:	accept.c lp.h $(LIB)
	$(COMPILE)
	$(INSADM)
cancel:	cancel.c lp.h $(LIB)
	$(COMPILE)
	$(INSUSR)
disable:	disable.c lp.h $(LIB)
	$(COMPILE)
	$(INSUSR)
enable:	enable.c lp.h $(LIB)
	$(COMPILE)
	$(INSUSR)
lp:	lp.c lp.h $(LIB)
	$(COMPILE)
	$(INSUSR)
lpadmin:	lpadmin.c lp.h $(LIB)
	$(COMPILE)
	$(INSADM)
	$(INS) chown root $(ADMDIR)/$@
lpmove:	lpmove.c lp.h $(LIB)
	$(COMPILE)
	$(INSADM)
lpsched:	lpsched.c lpsched.h lp.h $(LIB)
	$(COMPILE)
	$(INSADM)
	$(INS) chown root $(ADMDIR)/$@
lpshut:	lpshut.c lp.h $(LIB)
	$(COMPILE)
	$(INSADM)
lpstat:	lpstat.c lpsched.h lp.h $(LIB)
	$(COMPILE)
	$(INSUSR)
reject:	reject.c lp.h $(LIB)
	$(COMPILE)
	$(INSADM)

$(LIB): \
	$(LIB)(dest.o) \
	$(LIB)(destlist.o) \
	$(LIB)(fullpath.o) \
	$(LIB)(gwd.o) \
	$(LIB)(enter.o) \
	$(LIB)(fifo.o) \
	$(LIB)(getname.o) \
	$(LIB)(isclass.o) \
	$(LIB)(isprinter.o) \
	$(LIB)(isrequest.o) \
	$(LIB)(outlist.o) \
	$(LIB)(outputq.o) \
	$(LIB)(pstatus.o) \
	$(LIB)(date.o) \
	$(LIB)(isdest.o) \
	$(LIB)(eaccess.o) \
	$(LIB)(qstatus.o) \
	$(LIB)(fatal.o) \
	$(LIB)(lock.o) \
	$(LIB)(request.o) \
	$(LIB)(sendmail.o) \
	$(LIB)(trim.o) \
	$(LIB)(wrtmsg.o) \
	$(LIB)(findtty.o)
	$(CC) -c $(CFLAGS) $(DEFS) $(?:.o=.c)
	ar r $(LIB) $?
	rm -f $?
	chmod 664 $@

$(LIB)(date.o): date.c lp.h
$(LIB)(dest.o): dest.c lpsched.h lp.h
$(LIB)(destlist.o): destlist.c lpsched.h
$(LIB)(eaccess.o): eaccess.c lp.h
$(LIB)(enter.o): enter.c lp.h
$(LIB)(fatal.o): fatal.c lp.h
$(LIB)(fifo.o): fifo.c lp.h
$(LIB)(findtty.o): findtty.c lp.h lpsched.h
$(LIB)(fullpath.o): fullpath.c lp.h
$(LIB)(getname.o): getname.c lp.h
$(LIB)(gwd.o): gwd.c lp.h
$(LIB)(isclass.o): isclass.c lp.h
$(LIB)(isdest.o): isdest.c lp.h
$(LIB)(isprinter.o): isprinter.c lp.h
$(LIB)(isrequest.o): isrequest.c lp.h
$(LIB)(lock.o): lock.c lp.h
$(LIB)(outlist.o): outlist.c lp.h lpsched.h
$(LIB)(outputq.o): outputq.c lp.h
$(LIB)(pstatus.o): pstatus.c lp.h
$(LIB)(qstatus.o): qstatus.c lp.h
$(LIB)(request.o): request.c lp.h
$(LIB)(sendmail.o): sendmail.c lp.h
$(LIB)(trim.o): trim.c lp.h
$(LIB)(wrtmsg.o): wrtmsg.c lp.h lpsched.h

install:
	$(MAKE) -f lp.mk OL=$(OL) INS= $(ARGS)
	cd filter;  $(MAKE) -f filter.mk OL=$(OL) install
	cd model;   $(MAKE) -f model.mk  OL=$(OL) install
insaccept:	;  $(MAKE) -f lp.mk OL=$(OL) INS= accept
inscancel:	;  $(MAKE) -f lp.mk OL=$(OL) INS= cancel
insdisable:	;  $(MAKE) -f lp.mk OL=$(OL) INS= disable
insenable:	;  $(MAKE) -f lp.mk OL=$(OL) INS= enable
inslp:		;  $(MAKE) -f lp.mk OL=$(OL) INS= lp
inslpadmin:	;  $(MAKE) -f lp.mk OL=$(OL) INS= lpadmin
inslpmove:	;  $(MAKE) -f lp.mk OL=$(OL) INS= lpmove
inslpsched:	;  $(MAKE) -f lp.mk OL=$(OL) INS= lpsched
inslpshut:	;  $(MAKE) -f lp.mk OL=$(OL) INS= lpshut
inslpstat:	;  $(MAKE) -f lp.mk OL=$(OL) INS= lpstat
insreject:	;  $(MAKE) -f lp.mk OL=$(OL) INS= reject
inslib:	$(LIB)
	:
#######################################################################
#################################DSL only section - for development use

build:	bldacc bldcan blddis bldena bldlp bldlpa bldlpm bldlpsc bldlpsh \
	bldlpst bldrej bldlph bldlpschedh bldlib buildmk
	cd filter; $(MAKE) -f filter.mk SL=$(SL) REL=$(REL) REWIRE="$(REWIRE)" build
	cd model;  $(MAKE) -f model.mk  SL=$(SL) REL=$(REL) REWIRE="$(REWIRE)" build
	:
bldacc:		;  get -p -r`gsid accept $(REL)` s.accept.c > $(RDIR)/accept.c
bldcan:		;  get -p -r`gsid cancel $(REL)` s.cancel.c > $(RDIR)/cancel.c
blddis:		;  get -p -r`gsid disable $(REL)` s.disable.c > $(RDIR)/disable.c
bldena:		;  get -p -r`gsid enable $(REL)` s.enable.c > $(RDIR)/enable.c
bldlp:		;  get -p -r`gsid lp $(REL)` s.lp.c > $(RDIR)/lp.c
bldlpa:		;  get -p -r`gsid lpadmin $(REL)` s.lpadmin.c > $(RDIR)/lpadmin.c
bldlpm:		;  get -p -r`gsid lpmove $(REL)` s.lpmove.c > $(RDIR)/lpmove.c
bldlpsc:	;  get -p -r`gsid lpsched $(REL)` s.lpsched.c > $(RDIR)/lpsched.c
bldlpsh:	;  get -p -r`gsid lpshut $(REL)` s.lpshut.c > $(RDIR)/lpshut.c
bldlpst:	;  get -p -r`gsid lpstat $(REL)` s.lpstat.c > $(RDIR)/lpstat.c
bldrej:		;  get -p -r`gsid reject $(REL)` s.reject.c > $(RDIR)/reject.c
buildmk:	;  get -p -r`gsid lp.mk $(REL)` s.lp.mk > $(RDIR)/lp.mk
bldlph:		;  get -p -r`gsid lp.h $(REL)` s.lp.h $(REWIRE) > $(RDIR)/lp.h
bldlpschedh:	; get -p -r`gsid lpsched.h $(REL)` s.lpsched.h $(REWIRE) > $(RDIR)/lpsched.h
bldlib:	blddest blddestlist bldfullpath bldgwd bldenter bldfifo bldgetname \
	bldisclass bldisprinter bldisrequest bldoutlist bldoutputq \
	bldpstatus blddate bldisdest bldeaccess bldqstatus bldfatal \
	bldlock bldrequest bldsendmail bldtrim bldwrtmsg bldfindtty
	:
blddest:	;  get -p -r`gsid lpl-dest $(REL)` s.dest.c > $(RDIR)/dest.c
blddestlist:	;  get -p -r`gsid lpl-destlist $(REL)` s.destlist.c > $(RDIR)/destlist.c
bldfullpath:	;  get -p -r`gsid lpl-fullpath $(REL)` s.fullpath.c > $(RDIR)/fullpath.c
bldgwd:		;  get -p -r`gsid lpl-gwd $(REL)` s.gwd.c > $(RDIR)/gwd.c
bldenter:	;  get -p -r`gsid lpl-enter $(REL)` s.enter.c > $(RDIR)/enter.c
bldfifo:	;  get -p -r`gsid lpl-fifo $(REL)` s.fifo.c > $(RDIR)/fifo.c
bldgetname:	;  get -p -r`gsid lpl-getname $(REL)` s.getname.c > $(RDIR)/getname.c
bldisclass:	;  get -p -r`gsid lpl-isclass $(REL)` s.isclass.c > $(RDIR)/isclass.c
bldisprinter:	;  get -p -r`gsid lpl-isprinter $(REL)` s.isprinter.c > $(RDIR)/isprinter.c
bldisrequest:	;  get -p -r`gsid lpl-isrequest $(REL)` s.isrequest.c > $(RDIR)/isrequest.c
bldoutlist:	;  get -p -r`gsid lpl-outlist $(REL)` s.outlist.c > $(RDIR)/outlist.c
bldoutputq:	;  get -p -r`gsid lpl-outputq $(REL)` s.outputq.c > $(RDIR)/outputq.c
bldpstatus:	;  get -p -r`gsid lpl-pstatus $(REL)` s.pstatus.c > $(RDIR)/pstatus.c
blddate:	;  get -p -r`gsid lpl-date $(REL)` s.date.c > $(RDIR)/date.c
bldisdest:	;  get -p -r`gsid lpl-isdest $(REL)` s.isdest.c > $(RDIR)/isdest.c
bldeaccess:	;  get -p -r`gsid lpl-eaccess $(REL)` s.eaccess.c > $(RDIR)/eaccess.c
bldqstatus:	;  get -p -r`gsid lpl-qstatus $(REL)` s.qstatus.c > $(RDIR)/qstatus.c
bldfatal:	;  get -p -r`gsid lpl-fatal $(REL)` s.fatal.c > $(RDIR)/fatal.c
bldlock:	;  get -p -r`gsid lpl-lock $(REL)` s.lock.c > $(RDIR)/lock.c
bldrequest:	;  get -p -r`gsid lpl-request $(REL)` s.request.c > $(RDIR)/request.c
bldsendmail:	;  get -p -r`gsid lpl-sendmail $(REL)` s.sendmail.c > $(RDIR)/sendmail.c
bldtrim:	;  get -p -r`gsid lpl-trim $(REL)` s.trim.c > $(RDIR)/trim.c
bldwrtmsg:	;  get -p -r`gsid lpl-wrtmsg $(REL)` s.wrtmsg.c > $(RDIR)/wrtmsg.c
bldfindtty:	;  get -p -r`gsid lpl-findtty $(REL)` s.findtty.c > $(RDIR)/findtty.c

edit:	;  :
accedit:	;  get -e s.accept.c
canedit:	;  get -e s.cancel.c
disedit:	;  get -e s.disable.c
enaedit:	;  get -e s.enable.c
lpedit:		;  get -e s.lp.c
lpaedit:	;  get -e s.lpadmin.c
lpmedit:	;  get -e s.lpmove.c
lpscedit:	;  get -e s.lpsched.c
lpshedit:	;  get -e s.lpshut.c
lpstedit:	;  get -e s.lpstat.c
rejedit:	;  get -e s.reject.c
mkedit:		;  get -e s.lp.mk
lphedit:	;  get -e s.lp.h
lpschedhedit:	;  get -e s.lpsched.h
libedit:
	:
destedit:	;  get -e s.dest.c
destlistedit:	;  get -e s.destlist.c
fullpathedit:	;  get -e s.fullpath.c
gwdedit:	;  get -e s.gwd.c
enteredit:	;  get -e s.enter.c
fifoedit:	;  get -e s.fifo.c
getnameedit:	;  get -e s.getname.c
isclassedit:	;  get -e s.isclass.c
isprinteredit:	;  get -e s.isprinter.c
isrequestedit:	;  get -e s.isrequest.c
outlistedit:	;  get -e s.outlist.c
outputqedit:	;  get -e s.outputq.c
pstatusedit:	;  get -e s.pstatus.c
dateedit:	;  get -e s.date.c
isdestedit:	;  get -e s.isdest.c
eaccessedit:	;  get -e s.eaccess.c
qstatusedit:	;  get -e s.qstatus.c
fataledit:	;  get -e s.fatal.c
lockedit:	;  get -e s.lock.c
requestedit:	;  get -e s.request.c
sendmailedit:	;  get -e s.sendmail.c
trimedit:	;  get -e s.trim.c
wrtmsgedit:	;  get -e s.wrtmsg.c
findttyedit:	;  get -e s.findtty.c

delta:	;  :
accdelta:	;  delta s.accept.c
candelta:	;  delta s.cancel.c
disdelta:	;  delta s.disable.c
enadelta:	;  delta s.enable.c
lpdelta:	;  delta s.lp.c
lpadelta:	;  delta s.lpadmin.c
lpmdelta:	;  delta s.lpmove.c
lpscdelta:	;  delta s.lpsched.c
lpshdelta:	;  delta s.lpshut.c
lpstdelta:	;  delta s.lpstat.c
rejdelta:	;  delta s.reject.c
mkdelta:	;  delta s.lp.mk
lphdelta:	;  delta s.lp.h
lpschedhdelta:	;  delta s.lpsched.h
libdelta:
	:
destdelta:	;  delta s.dest.c
destlistdelta:	;  delta s.destlist.c
fullpathdelta:	;  delta s.fullpath.c
gwddelta:	;  delta s.gwd.c
enterdelta:	;  delta s.enter.c
fifodelta:	;  delta s.fifo.c
getnamedelta:	;  delta s.getname.c
isclassdelta:	;  delta s.isclass.c
isprinterdelta:	;  delta s.isprinter.c
isrequestdelta:	;  delta s.isrequest.c
outlistdelta:	;  delta s.outlist.c
outputqdelta:	;  delta s.outputq.c
pstatusdelta:	;  delta s.pstatus.c
datedelta:	;  delta s.date.c
isdestdelta:	;  delta s.isdest.c
eaccessdelta:	;  delta s.eaccess.c
qstatusdelta:	;  delta s.qstatus.c
fataldelta:	;  delta s.fatal.c
lockdelta:	;  delta s.lock.c
requestdelta:	;  delta s.request.c
sendmaildelta:	;  delta s.sendmail.c
trimdelta:	;  delta s.trim.c
wrtmsgdelta:	;  delta s.wrtmsg.c
findttydelta:	;  delta s.findtty.c

listing:
	pr lp.mk $(LFILES) $(SFILES) | $(LIST)
	cd filter; $(MAKE) -f filter.mk LIST="$(LIST)" listing
	cd model;  $(MAKE) -f model.mk  LIST="$(LIST)" listing
listmk:		;  pr lp.mk | $(LIST)
listacc:	;  pr accept.c | $(LIST)
listcan:	;  pr cancel.c | $(LIST)
listdis:	;  pr disable.c | $(LIST)
listena:	;  pr enable.c | $(LIST)
listlp:		;  pr lp.c | $(LIST)
listlpa:	;  pr lpadmin.c | $(LIST)
listlpm:	;  pr lpmove.c | $(LIST)
listlpsc:	;  pr lpsched.c | $(LIST)
listlpsh:	;  pr lpshut.c | $(LIST)
listlpst:	;  pr lpstat.c | $(LIST)
listrej:	;  pr reject.c | $(LIST)
listmk:		;  pr lp.mk | $(LIST)
listlph:	;  pr lp.h | $(LIST)
listlpschedh:	;  pr lpsched.h | $(LIST)
listlib:	;  pr $(LFILES) | $(LIST)
listdest:	;  pr dest.c | $(LIST)
listdestlist:	;  pr destlist.c | $(LIST)
listfullpath:	;  pr fullpath.c | $(LIST)
listgwd:	;  pr gwd.c | $(LIST)
listenter:	;  pr enter.c | $(LIST)
listfifo:	;  pr fifo.c | $(LIST)
listgetname:	;  pr getname.c | $(LIST)
listisclass:	;  pr isclass.c | $(LIST)
listisprinter:	;  pr isprinter.c | $(LIST)
listisrequest:	;  pr isrequest.c | $(LIST)
listoutlist:	;  pr outlist.c | $(LIST)
listoutputq:	;  pr outputq.c | $(LIST)
listpstatus:	;  pr pstatus.c | $(LIST)
listdate:	;  pr date.c | $(LIST)
listisdest:	;  pr isdest.c | $(LIST)
listeaccess:	;  pr eaccess.c | $(LIST)
listqstatus:	;  pr qstatus.c | $(LIST)
listfatal:	;  pr fatal.c | $(LIST)
listlock:	;  pr lock.c | $(LIST)
listrequest:	;  pr request.c | $(LIST)
listsendmail:	;  pr sendmail.c | $(LIST)
listtrim:	;  pr trim.c | $(LIST)
listwrtmsg:	;  pr wrtmsg.c | $(LIST)
listfindtty:	;  pr findtty.c | $(LIST)
##########################################################################

clean:	topclean
	cd filter; $(MAKE) -f filter.mk clean
	cd model;  $(MAKE) -f model.mk  clean
topclean:	;  rm -f $(LIB) *.o
accclean:	;  rm -f accept.o
canclean:	;  rm -f cancel.o
disclean:	;  rm -f disable.o
enaclean:	;  rm -f enable.o
lpclean:	;  rm -f lp.o
lpaclean:	;  rm -f lpadmin.o
lpmclean:	;  rm -f lpmove.o
lpscclean:	;  rm -f lpsched.o
lpshclean:	;  rm -f lpshut.o
lpstclean:	;  rm -f lpstat.o
rejclean:	;  rm -f reject.o
libclean:	destclean destlistclean fullpathclean gwdclean enterclean fifoclean getnameclean \
	isclassclean isprinterclean isrequestclean outlistclean outputqclean \
	pstatusclean dateclean isdestclean eaccessclean qstatusclean fatalclean \
	lockclean requestclean sendmailclean trimclean wrtmsgclean findttyclean
	:
destclean:	;  rm -f dest.o
destlistclean:	;  rm -f destlist.o
fullpathclean:	;  rm -f fullpath.o
gwdclean:	;  rm -f gwd.o
enterclean:	;  rm -f enter.o
fifoclean:	;  rm -f fifo.o
getnameclean:	;  rm -f getname.o
isclassclean:	;  rm -f isclass.o
isprinterclean:	;  rm -f isprinter.o
isrequestclean:	;  rm -f isrequest.o
outlistclean:	;  rm -f outlist.o
outputqclean:	;  rm -f outputq.o
pstatusclean:	;  rm -f pstatus.o
dateclean:	;  rm -f date.o
isdestclean:	;  rm -f isdest.o
eaccessclean:	;  rm -f eaccess.o
qstatusclean:	;  rm -f qstatus.o
fatalclean:	;  rm -f fatal.o
lockclean:	;  rm -f lock.o
requestclean:	;  rm -f request.o
sendmailclean:	;  rm -f sendmail.o
trimclean:	;  rm -f trim.o
wrtmsgclean:	;  rm -f wrtmsg.o
findttyclean:	;  rm -f findtty.o

clobber:	topclobber
	cd filter; $(MAKE) -f filter.mk clobber
	cd model;  $(MAKE) -f model.mk  clobber
topclobber: topclean
	rm -f $(ALL)
accclean:	;  rm -f accept
canclean:	;  rm -f cancel
disclean:	;  rm -f disable
enaclean:	;  rm -f enable
lpclean:	;  rm -f lp
lpaclean:	;  rm -f lpadmin
lpmclean:	;  rm -f lpmove
lpscclean:	;  rm -f lpsched
lpshclean:	;  rm -f lpshut
lpstclean:	;  rm -f lpstat
rejclean:	;  rm -f reject
libclobber:
	rm -f $(LIB)
destclobber:	;  :
destlistclobber:	;  :
fullpathclobber:	;  :
gwdclobber:	;  :
enterclobber:	;  :
fifoclobber:	;  :
getnameclobber:	;  :
isclassclobber:	;  :
isprinterclobber:	;  :
isrequestclobber:	;  :
outlistclobber:	;  :
outputqclobber:	;  :
pstatusclobber:	;  :
dateclobber:	;  :
isdestclobber:	;  :
eaccessclobber:	;  :
qstatusclobber:	;  :
fatalclobber:	;  :
lockclobber:	;  :
requestclobber:	;  :
sendmailclobber:	;  :
trimclobber:	;  :
wrtmsgclobber:	;  :
findttyclobber:	;  :

delete:	topclobber
	cd filter; $(MAKE) -f filter.mk delete
	cd model;  $(MAKE) -f model.mk  delete
topdelete:	topclobber
	rm -f $(SFILES)
accdelete:	;  rm -f accept.c
candelete:	;  rm -f cancel.c
disdelete:	;  rm -f disable.c
enadelete:	;  rm -f enable.c
lpdelete:	;  rm -f lp.c
lpadelete:	;  rm -f lpadmin.c
lpmdelete:	;  rm -f lpmove.c
lpscdelete:	;  rm -f lpsched.c
lpshdelete:	;  rm -f lpshut.c
lpstdelete:	;  rm -f lpstat.c
rejdelete:	;  rm -f reject.c
libdelete:	;  rm -f $(LFILES)
lphdelete:	;  rm -f lp.h
lpschedhdelete:	;  rm -f lpsched.h
destdelete:	;  rm -f dest.c
destlistdelete:	;  rm -f destlist.c
fullpathdelete:	;  rm -f fullpath.c
gwddelete:	;  rm -f gwd.c
enterdelete:	;  rm -f enter.c
fifodelete:	;  rm -f fifo.c
getnamedelete:	;  rm -f getname.c
isclassdelete:	;  rm -f isclass.c
isprinterdelete:	;  rm -f isprinter.c
isrequestdelete:	;  rm -f isrequest.c
outlistdelete:	;  rm -f outlist.c
outputqdelete:	;  rm -f outputq.c
pstatusdelete:	;  rm -f pstatus.c
datedelete:	;  rm -f date.c
isdestdelete:	;  rm -f isdest.c
eaccessdelete:	;  rm -f eaccess.c
qstatusdelete:	;  rm -f qstatus.c
fataldelete:	;  rm -f fatal.c
lockdelete:	;  rm -f lock.c
requestdelete:	;  rm -f request.c
sendmaildelete:	;  rm -f sendmail.c
trimdelete:	;  rm -f trim.c
wrtmsgdelete:	;  rm -f wrtmsg.c
findttydelete:	;  rm -f findtty.c
