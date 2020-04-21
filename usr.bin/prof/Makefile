#	@(#)Makefile	4.1 (Berkeley) 6/2/83
#
include ${ROOT}/usr/include/make/commondefs
#
# If you don't want to plot, take out the -Dplot and the ref. to plot.a

PLOT = -lplot -Dplot

all:	prof

prof:	$$@.c
	$(CCF) $(LDFLAGS) -o $@ $? $(PLOT)

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" prof

clean:
	rm -f *.o

clobber:clean
	rm -f prof
