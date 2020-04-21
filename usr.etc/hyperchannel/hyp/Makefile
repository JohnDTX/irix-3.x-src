include $(ROOT)/usr/include/make/commondefs

LDLIBS= -lbsd -ldbm
#LCOPTS= -g 
LCINCS= -I$(ROOT)/usr/include/bsd
YFLAGS= -v
ALL= hyp


default: $(ALL)

hyp: hyp.o hyp_main.o
	$(CCF) -o hyp hyp.o hyp_main.o $(LDFLAGS)

install: default
	$(INSTALL) -F /usr/etc -idb "upd usr hyper" hyp

hyp.o:		hyp.y defs.h
hyp_main.o:	hyp_main.c defs.h

clean:
	rm -f hyp_main.o hyp.o y.output y.tab.o y.tab.c

clobber: clean 
	rm -f $(ALL)
