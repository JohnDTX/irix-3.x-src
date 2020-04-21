# makefile for lp line printer spooler system
#
include ${ROOT}/usr/include/make/commondefs
# $Source: /d2/3.7/src/usr.bin/lp/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:04:16 $

LL	= /usr/lib
BB	= /usr/bin
SS	= /usr/spool/lp
L	= $(LL)
B	= $(BB)
S	= $(SS)
M	= $S/model
DIRS	= $S $M $S/class $S/member $S/interface $S/request
# name of "lp" owner in /etc/passwd
LPUSER	= lp
PROGS	= accept cancel disable enable lp lpadmin lpmove lpsched lpshut lpstat \
	  reject
LIB	= lib.a
LCDEFS	= -DSPOOL='"$(SS)"' -DADMIN='"$(LPUSER)"' \
	  -DADMDIR='"$(LL)"' -DUSRDIR='"$(BB)"'
YPLIBS	= -lsun -lbsd -ldbm
LFILES	= dest.c destlist.c fullpath.c gwd.c enter.c fifo.c getname.c \
	  isclass.c isprinter.c isrequest.c outlist.c outputq.c pstatus.c \
	  date.c isdest.c qstatus.c fatal.c lock.c request.c \
	  sendmail.c trim.c wrtmsg.c findtty.c
SFILES	= accept.c cancel.c disable.c enable.c lp.c lpadmin.c lpmove.c \
	  lpsched.c lpshut.c lpstatus.c reject.c
HFILES	= lp.h lpsched.h
MEMBERS	= \
	  $(LIB)(dest.o)	\
	  $(LIB)(destlist.o)	\
	  $(LIB)(fullpath.o)	\
	  $(LIB)(gwd.o)		\
	  $(LIB)(enter.o)	\
	  $(LIB)(fifo.o)	\
	  $(LIB)(getname.o)	\
	  $(LIB)(isclass.o)	\
	  $(LIB)(isprinter.o)	\
	  $(LIB)(isrequest.o)	\
	  $(LIB)(outlist.o)	\
	  $(LIB)(outputq.o)	\
	  $(LIB)(pstatus.o)	\
	  $(LIB)(date.o)	\
	  $(LIB)(isdest.o)	\
	  $(LIB)(eaccess.o)	\
	  $(LIB)(qstatus.o)	\
	  $(LIB)(fatal.o)	\
	  $(LIB)(lock.o)	\
	  $(LIB)(request.o)	\
	  $(LIB)(sendmail.o)	\
	  $(LIB)(trim.o)	\
	  $(LIB)(wrtmsg.o)	\
	  $(LIB)(findtty.o)	\
	  $(NULL)

MODELS	= 1640 dumb f450 hp pprx prx

all:	$(PROGS) ${LFILES}
	cd filter ; make -f filter.mk install
install:all dirs
	cd model ; ${INSTALL} -u "$(LPUSER)" -g "$(LPUSER)" -m 644 -F "$M" \
			-idb "std.sw.unix mr" "${MODELS}"
	${INSTALL} -u "$(LPUSER)" -g "$(LPUSER)" -m 6755 -F "$B" \
		-idb "std.sw.unix mr" "cancel disable enable lp lpstat"
	${INSTALL} -u "$(LPUSER)" -g "$(LPUSER)" -m 6755 -F "$L" \
		-idb "std.sw.unix mr" "accept lpmove lpshut reject "
	${INSTALL} -u "$(LPUSER)" -g "$(LPUSER)" -m 4755 -F "$L" \
		-idb "std.sw.unix mr" "lpadmin lpsched"
	touch pstatus qstatus
	${INSTALL} -u "$(LPUSER)" -g "$(LPUSER)" \
		-m 644 -F "$S" -idb "std.sw.unix mr" "pstatus "
	${INSTALL} -u "$(LPUSER)" -g "$(LPUSER)" \
		-m 644 -F "$S" -idb "std.sw.unix mr" "qstatus "
	cd filter ; make -f filter.mk install

dirs:	
	${INSTALL} -dir -u "${LPUSER}" -g "${LPUSER}"  -m 755   \
	-idb "std.sw.unix mr" "${DIRS}"

clean:
	rm -f *.o $(LIB)

clobber:	clean
	rm -f $(PROGS)
	cd filter; make -f filter.mk clobber

.c:
	$(CCF) -o $@ $< ${LDFLAGS} $(LIB) $(YPLIBS)

$(PROGS):$$@.c lp.h $(LIB)
	$(CCF) -o $@ $@.c ${LDFLAGS} $(LIB) $(YPLIBS)

$(LIB):	$(MEMBERS)
	ranlib $(LIB)

date.o:		lp.h
dest.o:		lpsched.h lp.h
destlist.o:	lpsched.h
eaccess.o:	lp.h
enter.o:	lp.h
fatal.o:	lp.h
fifo.o:		lp.h
findtty.o:	lp.h lpsched.h
fullpath.o:	lp.h
getname.o:	lp.h
gwd.o:		lp.h
isclass.o:	lp.h
isdest.o:	lp.h
isprinter.o:	lp.h
isrequest.o:	lp.h
lock.o:		lp.h
outlist.o:	lp.h lpsched.h
outputq.o:	lp.h
pstatus.o:	lp.h
qstatus.o:	lp.h
request.o:	lp.h
sendmail.o:	lp.h
trim.o:		lp.h
wrtmsg.o:	lp.h lpsched.h

FRC:
