#
#
include ${ROOT}/usr/include/make/commondefs
# Note: merge uses /usr/lib/rdiff3

EXTHDRS	= 
HDRS	= 
LIBS	= 
MAKEFILE= makefile
PR	= pr
BIN	= rdiff3
SRCS	= diff3.c

all:	$(BIN)

$(BIN):	$(SRCS) $(LIBS)
	$(CCF) $(LDFLAGS) $(SRCS) $(LIBS) -o $@

install:$(BIN)
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" "${BIN}"

clean:
	rm -f *.o

clobber:clean
	rm -f $(BIN)

depend:
	@mkmf -f $(MAKEFILE) BIN=$(BIN) DEST=$(DEST)

print:
	$(PR) $(HDRS) $(SRCS)

tags:	$(HDRS) $(SRCS)
	ctags $(HDRS) $(SRCS)
