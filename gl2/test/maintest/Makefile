#
# Makefile for rgltest - derived from VMS maintest program
#

# these are the VMS FORTRAN sources
SRCS = maintest.for subtest1.for subtest2.for subtest3.for subtest4.for \
       subtest5.for subtest6.for subtest7.for bogus.c bogus2.c isrem.f

OBJS = maintest.o subtest1.o subtest2.o subtest3.o subtest4.o subtest5.o \
       subtest6.o subtest7.o bogus.o 

.SUFFIXES: .for .f

.for.o:
	sed "s@^[ 	]*INCLUDE '\([^']*\)'@#include <\1>@" \
		$< > $*.f
	$(F77) $(F77FLAGS) -c -o $@ $(F77FLAGS) $*.f
	rm -f $*.f

ALL =  rgltest fgltest

all: $(ALL)

rgltest: $(OBJS) isrem.f
	$(F77) $(F77FLAGS) -c -DREMOTE isrem.f
	$(F77) $(LDFLAGS) -o $@ $(OBJS) isrem.o -lfgl2 -lrgl2

fgltest: $(OBJS) bogus2.o isrem.f
	$(F77) $(F77FLAGS) -c -DLOCAL isrem.f
	$(F77) $(LDFLAGS) -o $@ $(OBJS) isrem.o bogus2.o -lfgl2 -Zg 

clean:
	rm -f $(OBJS) $(ALL) *.f
