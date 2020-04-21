
#
#
include ${ROOT}/usr/include/make/commondefs

OFILES	= bs.o atof.o string.o

all:	bs

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" bs

bs:	$(OFILES)
	$(CCF) $(NFLAG) $(LDFLAGS) -o $@ $(OFILES) $$ROOT/usr/lib/libm.a 

clean:	
	rm -f *.o

clobber:clean
	rm -f bs
