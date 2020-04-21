# make Cypress routing code
# $Header: /d2/3.7/src/cypress/RCS/Makefile,v 1.1 89/03/27 15:03:28 root Exp $

include $(ROOT)/usr/include/make/commondefs
COMMONPREF=cyetc
MAKEFILE=Makefile

DIRS=route

LCOPTS=-O
LCINCS=-I$(ROOT)/usr/include/sun -I$(ROOT)/usr/include/bsd
# until we run bind(1), use BSD42
LCDEFS=-DBSD42
LLDLIBS=-ll -lsun -lbsd

BINFILES=cyconfig cyerrd cyioctl cymonitord cyinstall cy_routed cymon 
LIBFILES=cypstat getroute
IDBTAG=cypress
TARGETS=$(BINFILES) $(LIBFILES)
BINDIR=/usr/etc/cypress/bin
LIBDIR=/usr/etc/cypress/bin

CFILES=	cy_routed.c cyconfig.c cyinstall.c cyioctl.c \
	cymon.c cymonitord.c cypstat.c getroute.c \
	utils.c cyerrd.c sgi_utils.c


all $(COMMONTARGS):$(COMMONPREF)$$@
	@for nm in $(DIRS); do \
		echo "==== cypress/etc/$$nm"; cd $$nm; $(MAKE) $@; cd ..; \
	done

include $(ROOT)/usr/include/make/commonrules

$(COMMONPREF)default $(COMMONPREF)all:$(TARGETS)

$(COMMONPREF)install: all
	$(INSTALL) $(IDBTAG) -dir "$(BINDIR) $(LIBDIR)"
	$(INSTALL) $(IDBTAG) -F $(BINDIR) $(BINFILES)
	$(INSTALL) $(IDBTAG) -F $(LIBDIR) $(LIBFILES)

cyconfig cyerrd cyioctl: $$@.c utils.o sgi_utils.o
	$(CCF) -o $@ $@.c utils.o sgi_utils.o $(LDFLAGS)

cymonitord cyinstall cy_routed cypstat getroute: $$@.c sgi_utils.o
	$(CCF) -o $@ $@.c sgi_utils.o $(LDFLAGS)

cymon: $$@.c utils.o
	$(CCF) -o $@ $@.c utils.o -lcurses -ltermlib  $(LDFLAGS)



# DO NOT DELETE THIS LINE -- make depend uses it
cy_routed.o: cy_routed.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/wait.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/ctype.h $(INCLDIR)/signal.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/sun/netdb.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h 
cyconfig.o: cyconfig.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/net/if.h \
	$(INCLDIR)/net/if_arp.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/file.h /usr/include/sys/types.h \
	/usr/include/sys/file.h $(INCLDIR)/sys/fcntl.h \
	$(INCLDIR)/bsd/sys/ioctl.h $(INCLDIR)/bsd/sys/ttychars.h \
	$(INCLDIR)/termio.h $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/net/soioctl.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/sun/netdb.h $(INCLDIR)/bsd/sys/time.h if_cy.h \
	$(INCLDIR)/ctype.h $(INCLDIR)/strings.h \
	$(INCLDIR)/bsd/arpa/inet.h 
cyinstall.o: cyinstall.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/ioctl.h $(INCLDIR)/bsd/sys/ttychars.h \
	$(INCLDIR)/termio.h $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/net/soioctl.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/bsd/sys/time.h \
	$(INCLDIR)/bsd/sys/wait.h $(INCLDIR)/net/if.h \
	$(INCLDIR)/net/if_arp.h $(INCLDIR)/stdio.h $(INCLDIR)/errno.h \
	$(INCLDIR)/sys/errno.h $(INCLDIR)/ctype.h \
	$(INCLDIR)/sun/netdb.h $(INCLDIR)/bsd/arpa/inet.h if_cy.h \
	defs.h 
cyioctl.o: cyioctl.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/ioctl.h $(INCLDIR)/bsd/sys/ttychars.h \
	$(INCLDIR)/termio.h $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/net/soioctl.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/nlist.h \
	$(INCLDIR)/strings.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/net/if.h $(INCLDIR)/net/if_arp.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/errno.h $(INCLDIR)/sys/errno.h $(INCLDIR)/ctype.h \
	$(INCLDIR)/sun/netdb.h if_cy.h 
cymon.o: cymon.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/file.h /usr/include/sys/types.h \
	/usr/include/sys/file.h $(INCLDIR)/sys/fcntl.h \
	$(INCLDIR)/bsd/sys/ioctl.h $(INCLDIR)/bsd/sys/ttychars.h \
	$(INCLDIR)/termio.h $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/net/soioctl.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/net/if.h $(INCLDIR)/net/if_arp.h \
	$(INCLDIR)/sys/dk.h $(INCLDIR)/sun/netdb.h \
	$(INCLDIR)/bsd/arpa/inet.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/ctype.h $(INCLDIR)/errno.h $(INCLDIR)/sys/errno.h \
	$(INCLDIR)/curses.h $(INCLDIR)/stdio.h $(INCLDIR)/termio.h \
	$(INCLDIR)/sys/termio.h $(INCLDIR)/signal.h \
	$(INCLDIR)/sys/signal.h $(INCLDIR)/strings.h if_cy.h cypstat.h \
	cymon.h 
cymonitord.o: cymonitord.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/ioctl.h $(INCLDIR)/bsd/sys/ttychars.h \
	$(INCLDIR)/termio.h $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/net/soioctl.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/wait.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/stdio.h $(INCLDIR)/ctype.h $(INCLDIR)/signal.h \
	$(INCLDIR)/sys/signal.h $(INCLDIR)/sun/netdb.h \
	$(INCLDIR)/errno.h $(INCLDIR)/sys/errno.h 
cypstat.o: cypstat.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/ioctl.h $(INCLDIR)/bsd/sys/ttychars.h \
	$(INCLDIR)/termio.h $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/net/soioctl.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/bsd/sys/param.h \
	$(INCLDIR)/bsd/sys/../../sys/param.h \
	$(INCLDIR)/sys/sysmacros.h $(INCLDIR)/machine/sysmacros.h \
	$(INCLDIR)/bsd/sys/types.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/machine/param.h $(INCLDIR)/nlist.h \
	$(INCLDIR)/strings.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/net/if.h $(INCLDIR)/net/if_arp.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/bsd/sys/file.h /usr/include/sys/types.h \
	/usr/include/sys/file.h $(INCLDIR)/sys/fcntl.h \
	$(INCLDIR)/errno.h $(INCLDIR)/sys/errno.h $(INCLDIR)/ctype.h \
	$(INCLDIR)/signal.h $(INCLDIR)/sys/signal.h \
	$(INCLDIR)/sun/netdb.h $(INCLDIR)/sys/dk.h if_cy.h cypstat.h 
getroute.o: getroute.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/file.h /usr/include/sys/types.h \
	/usr/include/sys/file.h $(INCLDIR)/sys/fcntl.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/sun/netdb.h \
	$(INCLDIR)/bsd/arpa/inet.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/ctype.h $(INCLDIR)/errno.h $(INCLDIR)/sys/errno.h \
	defs.h 
utils.o: utils.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/net/if.h $(INCLDIR)/net/if_arp.h if_cy.h \
	$(INCLDIR)/errno.h $(INCLDIR)/sys/errno.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/sun/netdb.h $(INCLDIR)/ctype.h 
cyerrd.o: cyerrd.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/sys/ioctl.h $(INCLDIR)/bsd/sys/ttychars.h \
	$(INCLDIR)/termio.h $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/net/soioctl.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/time.h $(INCLDIR)/nlist.h \
	$(INCLDIR)/strings.h $(INCLDIR)/netinet/in.h \
	$(INCLDIR)/net/if.h $(INCLDIR)/net/if_arp.h $(INCLDIR)/stdio.h \
	$(INCLDIR)/errno.h $(INCLDIR)/sys/errno.h $(INCLDIR)/ctype.h \
	$(INCLDIR)/sun/netdb.h if_cy.h 
sgi_utils.o: sgi_utils.c $(INCLDIR)/sys/termio.h \
	$(INCLDIR)/sys/stropts.h 
# DO NOT DELETE THIS 2nd LINE -- make depend uses it
