# generate sendmail object
#
# "$Header"

include ${ROOT}/usr/include/make/commondefs
IDBTAG=-idb "std.sw.unix"

OBJS	=conf.o main.o collect.o parseaddr.o alias.o deliver.o \
	savemail.o err.o readcf.o stab.o headers.o recipient.o \
	stats.o daemon.o usersmtp.o srvrsmtp.o queue.o \
	macro.o util.o clock.o trace.o envelope.o \
	sysexits.o arpadate.o convtime.o version.o \
	aliasfetch.o

LCINCS	=-I../include -I${ROOT}/usr/include/bsd 
LCOPTS	=-DVMUNIX -DUSG -DSCANF -Dsgi_vax
LLDLIBS = -lsun -lbsd -ldbm

default all:sendmail

sendmail:$(OBJS)
	$(CCF) $(OBJS) $(LDFLAGS) -o sendmail

install:default
	$(INSTALL) $(IDBTAG) -F /usr/lib -u root -m 4755 sendmail

$(OBJS): sendmail.h conf.h ../include/useful.h

clobber clean:
	rm -f core sendmail a.out *.o

# to make IRIS work:
.c.o:
	$(CCF) -c $<
