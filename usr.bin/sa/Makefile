# The sadc and sadp modules must be able to read /dev/kmem,

#
#
include ${ROOT}/usr/include/make/commondefs

# which standardly has restricted read permission.
# They must have set-group-ID mode
# and have the same group as /dev/kmem.
# Fix the vchk_tree appropriately.
#

G = $(DEST)/usr/lib/sa
L = $(DEST)/usr/lib/sa
B = $(DEST)/bin
U = $(DEST)/usr/bin

YPLIBS = -lyp -lrpc -lbsd -ldbm

all:	sadc sar sa1 sa2 timex sag # sadp

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib/sa \
	-idb "std.sw.unix mr" sadc
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" "timex sar sag "
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib/sa \
	-idb "std.sw.unix mr" "sa1 sa2 "

clean:
	rm -f *.o

clobber:clean
	rm -f sadc sar sa1 sa2 timex sag sadp

sadc:	$$@.c sa.h 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(YPLIBS)

sar:	$$@.c sa.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(YPLIBS)

timex:	$$@.c sa.h 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(YPLIBS)

sag:	saga.o sagb.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ saga.o sagb.o $(YPLIBS)

saga.o:	saghdr.h
	$(CC) $(CFLAGS) -c $*.c

sagb.o:	saghdr.h
	$(CC) $(CFLAGS) -c $*.c
