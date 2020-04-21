# Make the timed demon
# "$Header: /d2/3.7/src/bsd/etc/timed/RCS/Makefile,v 1.1 89/03/27 14:59:22 root Exp $"

include	$(ROOT)/usr/include/make/commondefs


# optional flags are: MEASURE TESTING DEBUG

SRCDS=	acksend.c candidate.c correct.c \
	master.c netdelta.c readmsg.c slave.c timed.c
OBJDS=	$(SRCDS:.c=.o)

SRCCS=	cmds.c cmdtab.c timedc.c
OBJCS=	$(SRCCS:.c=.o)

COMMS=	byteorder.c measure.c cksum.c
COMMO=	$(COMMS:.c=.o)

TARGETS=timed timedc

VCFLAGS	=-I$(ROOT)/usr/include/bsd -DTESTING
LLDLIBS=-lsun -lbsd
I_FLAGS=-idb "std.sw.tcp"


default all:${TARGETS}

install:default
	$(INSTALL) $(I_FLAGS) -u root -g sys -m 4755 -F /usr/etc timedc
	$(INSTALL) $(I_FLAGS) -u root -g sys -m 755 -F /usr/etc timed

MAKEFILE=Makefile
include $(ROOT)/usr/include/make/commonrules

timed: 	${OBJDS} ${COMMO}
	${CCF} ${OBJDS} ${COMMO} ${LDFLAGS} -o timed 

timedc: ${OBJCS} ${COMMO}
	${CCF} ${OBJCS} ${COMMO} ${LDFLAGS} -o timedc


OBJECTS=$(OBJDS) $(OBJCS) $(COMMO)
DEPFILES=$(SRCDS) $(SRCCS) $(COMMS)

# DO NOT DELETE THIS LINE -- make depend uses it
acksend.o: acksend.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h 
candidate.o: candidate.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h 
correct.o: correct.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h 
master.o: master.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h $(INCLDIR)/bsd/sys/file.h \
	$(INCLDIR)/sys/types.h $(INCLDIR)/sys/file.h \
	$(INCLDIR)/sys/fcntl.h $(INCLDIR)/setjmp.h \
	$(INCLDIR)/sys/setjmp.h $(INCLDIR)/utmp.h 
netdelta.o: netdelta.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h 
readmsg.o: readmsg.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h 
slave.o: slave.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h $(INCLDIR)/setjmp.h \
	$(INCLDIR)/sys/setjmp.h 
timed.o: timed.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h $(INCLDIR)/net/if.h \
	$(INCLDIR)/net/if_arp.h $(INCLDIR)/bsd/sys/file.h \
	$(INCLDIR)/sys/types.h $(INCLDIR)/sys/file.h \
	$(INCLDIR)/sys/fcntl.h $(INCLDIR)/bsd/sys/ioctl.h \
	$(INCLDIR)/bsd/sys/ttychars.h $(INCLDIR)/termio.h \
	$(INCLDIR)/sys/termio.h $(INCLDIR)/net/soioctl.h \
	$(INCLDIR)/bsd/sys/types.h $(INCLDIR)/setjmp.h \
	$(INCLDIR)/sys/setjmp.h 
cmds.o: cmds.c timedc.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/bsd/netdb.h \
	$(INCLDIR)/bsd/arpa/inet.h $(INCLDIR)/netinet/in_systm.h \
	$(INCLDIR)/netinet/ip.h $(INCLDIR)/netinet/ip_icmp.h \
	$(INCLDIR)/bsd/protocols/timed.h $(INCLDIR)/bsd/sys/file.h \
	$(INCLDIR)/sys/types.h $(INCLDIR)/sys/file.h \
	$(INCLDIR)/sys/fcntl.h 
cmdtab.o: cmdtab.c timedc.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/bsd/netdb.h \
	$(INCLDIR)/bsd/arpa/inet.h 
timedc.o: timedc.c timedc.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/bsd/netdb.h \
	$(INCLDIR)/bsd/arpa/inet.h $(INCLDIR)/signal.h \
	$(INCLDIR)/sys/signal.h $(INCLDIR)/ctype.h $(INCLDIR)/setjmp.h \
	$(INCLDIR)/sys/setjmp.h $(INCLDIR)/bsd/syslog.h 
byteorder.o: byteorder.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h 
measure.o: measure.c globals.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/bsd/syslog.h \
	$(INCLDIR)/sys/socket.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/netdb.h $(INCLDIR)/bsd/arpa/inet.h \
	$(INCLDIR)/bsd/protocols/timed.h $(INCLDIR)/netinet/in_systm.h \
	$(INCLDIR)/netinet/ip.h $(INCLDIR)/netinet/ip_icmp.h 
cksum.o: cksum.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h 
# DO NOT DELETE THIS 2nd LINE -- make depend uses it
