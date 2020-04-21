# make Cypress routing code
# $Header: /d2/3.7/src/cypress/route/RCS/Makefile,v 1.1 89/03/27 15:03:48 root Exp $

include $(ROOT)/usr/include/make/commondefs
MAKEFILE=Makefile

BINDIR=/usr/etc/cypress/bin
IDBTAG="-idb cypress"

TARGETS= cypnet
LCOPTS=-O
LCINCS=-I$(ROOT)/usr/include/sun -I$(ROOT)/usr/include/bsd
LCDEFS=-DBSD43
LLDLIBS=-ll -lsun -lbsd

CFILES	=main.c buildtbl.c misc.c hosts.c paths.c route.c AdjList.c \
	components.c bicomp.c cache.c getcynam.c strcmpfold.c
YFILES	=parser.y
LFILES	=scanner.l

all default: $(TARGETS)

install: all
	$(INSTALL) -F $(BINDIR) "$(TARGETS)"

include $(COMMONRULES)

cypnet: ${OBJECTS}
	$(CCF) ${OBJECTS} -o cypnet $(LDFLAGS) 

scanner.c: scanner.l
	lex scanner.l
	mv -f lex.yy.c $@

parser.c tn.h: parser.y
	yacc -dv parser.y
	mv -f y.tab.h tn.h
	mv -f y.tab.c parser.c
depend:tn.h


# DO NOT DELETE THIS LINE -- make depend uses it
main.o: main.c $(INCLDIR)/stdio.h $(INCLDIR)/sun/netdb.h defs.h 
buildtbl.o: buildtbl.c $(INCLDIR)/stdio.h $(INCLDIR)/sun/netdb.h defs.h 
misc.o: misc.c $(INCLDIR)/stdio.h $(INCLDIR)/strings.h defs.h 
hosts.o: hosts.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/sun/netdb.h \
	$(INCLDIR)/stdio.h $(INCLDIR)/ctype.h defs.h 
paths.o: paths.c $(INCLDIR)/stdio.h $(INCLDIR)/strings.h defs.h 
route.o: route.c defs.h 
AdjList.o: AdjList.c $(INCLDIR)/stdio.h $(INCLDIR)/strings.h defs.h 
components.o: components.c $(INCLDIR)/stdio.h $(INCLDIR)/strings.h \
	defs.h 
bicomp.o: bicomp.c $(INCLDIR)/stdio.h defs.h 
cache.o: cache.c $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sys/stat.h \
	defs.h $(INCLDIR)/stdio.h 
getcynam.o: getcynam.c defs.h $(INCLDIR)/bsd/sys/types.h \
	$(INCLDIR)/bsd/sys/../../sys/types.h $(INCLDIR)/sun/netdb.h \
	$(INCLDIR)/netinet/in.h $(INCLDIR)/sys/socket.h \
	$(INCLDIR)/bsd/arpa/inet.h $(INCLDIR)/stdio.h 
strcmpfold.o: strcmpfold.c $(INCLDIR)/ctype.h 
scanner.l:  
scanner.o: scanner.l tn.h 
parser.y:  
parser.o: parser.y defs.h $(INCLDIR)/strings.h 
# DO NOT DELETE THIS 2nd LINE -- make depend uses it
