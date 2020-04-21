include $(ROOT)/usr/include/make/commondefs

LLDLIBS= -ll -lbsd -ldbm
LCINCS=
LCDEFS=  -DSYSTEM5
ALL=	 hyroute

default: $(ALL)

hyroute: hyr_main.o hyr_lex.o
	$(CCF) -o $@ hyr_main.o hyr_lex.o $(LDFLAGS)

install: default
	$(INSTALL) -F /usr/etc -idb "upd usr hyper" hyroute 

hyr_main.o:	hyr_main.c hyr_sym.h
hyr_lex.o:	hyr_lex.l hyr_sym.h

clean:
	rm -f *.o 

clobber: clean
	rm -f $(ALL)
