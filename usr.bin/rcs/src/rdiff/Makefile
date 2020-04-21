#
#
include ${ROOT}/usr/include/make/commondefs
# Note:	RCS uses /usr/lib/rdiff and /usr/lib/diffh

CFLAGS2	= -DDIFF='"$(DIFF)"' -DDIFFH='"$(DIFFH)"' -DPR='"$(PR)"' -DSYS5
DIFF	= /bin/diff
DIFFH	= /usr/lib/diffh
EXTHDRS	= 
HDRS	= diff.h
LIBS	= # $$DEST/usr/lib/libndir.a
MAKEFILE= makefile
OBJS	= diff.o diffdir.o diffreg.o
PR	= pr
BIN	= rdiff
SRCS	= diff.c diffdir.c diffreg.c

all:	$(BIN)

$(BIN):	$(OBJS)
	$(CCF) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

install:$(BIN)
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" "${BIN}"

clean:
	rm -f $(OBJS)

clobber:clean
	rm -f $(BIN)

print:
	$(PR) $(HDRS) $(SRCS)

tags:	$(HDRS) $(SRCS)
	ctags $(HDRS) $(SRCS)

.c.o:
	$(CCF) $(CFLAGS2) -c $<
