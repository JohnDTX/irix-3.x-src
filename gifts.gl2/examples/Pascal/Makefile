.SUFFIXES:
.SUFFIXES: .c .o .p .s
.p.o:
	pc -c $*.p

all:	ptest1 ptest2

ptest1:	ptest1.o
	pc -Zg -o ptest1 ptest1.o
	echo done with ptest1

ptest2:	ptest2.o
	pc -Zg -o ptest2 ptest2.o
	echo done with ptest2

clean:
	rm -f *.o

