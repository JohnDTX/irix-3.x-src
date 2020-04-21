RUNTESTS= open-unlk open-chmod dupreq excltest statfs negseek rename holey
TESTS= $(RUNTESTS) nstat stat stat2 touchn
DESTDIR=/no/such/path
LIBES=
CC=cc
CINC=	-I/usr/include/bsd
COPT=	-lbsd
CFLAGS=	$(CDEF) $(CINC) $(COPT)

rall: $(RUNTESTS)
all: $(TESTS)

$(TESTS):
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBES)

open-unlk: open-unlk.c
open-chmod: open-chmod.c
dupreq: dupreq.c
excltest: excltest.c
statfs: statfs.c
negseek: negseek.c
nstat: nstat.c
rename: rename.c
stat: stat.c
stat2: stat2.c
touchn: touchn.c
holey: holey.c

clean:
	-rm -f *.o $(TESTS) testfile exctest.file holeyfile

copy: $(RUNTESTS) mkdestdir
	cp runtests $(RUNTESTS) $(DESTDIR)

copyall: $(TESTS) mkdestdir
	cp runtests $(TESTS) $(DESTDIR)

dist: mkdestdir
	cp runtests Makefile *.c $(DESTDIR)

mkdestdir:
	if test ! -d $(DESTDIR); then \
		rm -rf $(DESTDIR); mkdir $(DESTDIR); \
	fi
