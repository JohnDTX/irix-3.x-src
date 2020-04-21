# "$Header: /d2/3.7/src/bsd/etc/RCS/Makefile,v 1.1 89/03/27 14:58:21 root Exp $"
#
include ${ROOT}/usr/include/make/commondefs

IDBTAG	=-idb "std.sw.unix mr" 
INSCMD	=$(INSTALL) $(IDBTAG)

DIRS	=ftpd routed netstat tftpd timed

USRPROGS=inetd rlogind route rshd rwhod syslogd telnetd
IDPROGS=arp ping 
OTHER	=rexecd
ETCPROGS= havetcp ifconfig
SIMPLE	=${USRPROGS} ${IDPROGS} ${ETCPROGS}
PROGS	=${SIMPLE} ${OTHER}
ETCFILES=protocols services
USRFILES=inetd.conf

LCOPTS= -O
LCDEFS= -DUSG
LCINCS= -I${ROOT}/usr/include/bsd 
LLDLIBS= -lsun -lbsd -ldbm

all default clean clobber install: bsd_$$@
	for i in ${DIRS}; do cd $$i; make $@; cd ..; done

bsd_all bsd_default:	${PROGS} $(ETCFILES) $(USRFILES)

bsd_install:	bsd_default
	${INSCMD} -u root -g sys -F /etc "${ETCPROGS}"
	${INSCMD} -O -u root -g sys -F /usr/etc "${USRPROGS} ${OTHER}"
	${INSCMD} -m 4755 -F /usr/etc ping
	${INSCMD} -m 2755 -F /usr/etc arp
	${INSCMD} -u root -g sys -m 644 -F /etc "${ETCFILES}"
	${INSCMD} -u root -g sys -m 644 -F /usr/etc "${USRFILES}"

bsd_clean:
	rm -rf *.o core a.out

bsd_clobber:bsd_clean
	rm -rf ${PROGS}

${SIMPLE}: $$@.c
	${CCF} -o $@ $@.c ${LDFLAGS} 

# Special make lines for special programs
rexecd: $$@.c
	${CCF} -o $@ $@.c ${LDFLAGS} -lcrypt 

anything:
