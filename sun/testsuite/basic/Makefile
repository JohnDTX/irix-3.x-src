# to make tests, use 'make'
# to copy tests to another directory, use 'make copy DESTDIR=dir'
# to copy source to another directory, use 'make dist DESTDIR=dir'

CTESTS=	test1 test3 test4 test5 test6 test7 test8 test9
TESTS=	$(CTESTS) test2
DESTDIR=/no/such/path
CINC=	-I/usr/include/bsd
CFLAGS=	$(CDEF) $(CINC) $(COPT)
LDFLAGS=-lbsd

all: $(TESTS)

$(CTESTS): $$@.c subr.o
	$(CC) $(CFLAGS) $@.c subr.o $(LDFLAGS) -o $@

test2: test2.csh
	cp test2.csh test2
	chmod +x test2

clean:
	-rm -f *.o $(TESTS)

copy: $(TESTS)
	cp runtests defs.csh $(TESTS) $(DESTDIR)

dist:
	cp runtests Makefile *.c *.h *.csh $(DESTDIR)
