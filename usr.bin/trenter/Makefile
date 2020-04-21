
#include /usr/include/make/commondefs

INS	= /etc/install -n /usr/bin
B	= usr/bin
S	= usr/spool/trenter

TRESRC	= trenter.c obj.c util.c
TREOBJ	= trenter.o obj.o util.o

TRE	= trenter

all:$(TRE)

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" "${TRE}"
	${INSTALL} -d "${S}" -u bin -g bin -m 755 \
	-idb "std.sw.unix mr" 
	${INSTALL} -d "${S}/send" -u bin -g bin -m 777 \
	-idb "std.sw.unix mr" 
	${INSTALL} -src dottrdef -u bin -g bin -m 644 -F "${S}" \
	-idb "std.sw.unix mr" .trdef

$(TRE):	$(TREOBJ)
		$(CC) $(LDFLAGS) $(TREOBJ) $(NFLAG) -o $(TRE)

.s.o:
	$(CC) $(CFLAGS) -c $<

.c.o:
	$(CC) $(CFLAGS) $(NFLAG) -c $<

clean:
	rm -f $(TREOBJ)

clobber:clean
	rm -f $(TRE)
