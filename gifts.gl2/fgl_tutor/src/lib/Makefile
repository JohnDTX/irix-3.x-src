# compiler switches
CC	=	cc
INCLUDE =	-I. -I/usr/include -I../include
CFLAGS	=	-O -c $(ZF)
ALL	=	gltools.a mymath.a digits.a libtutor.a

install:	all

all:	$(ALL)

gltools.a:	gltools.o
	ar cr gltools.a gltools.o
	ranlib gltools.a

mymath.a:	mymath.o
	ar cr mymath.a mymath.o
	ranlib mymath.a

digits.a:	digits.o
	ar cr digits.a digits.o
	ranlib digits.a

libtutor.a: libtutor.o help.o tsliders.o background.o
	ar cr libtutor.a libtutor.o help.o tsliders.o background.o
	ranlib libtutor.a

clean:	
	rm -f .[BC]* .em* *.o $(ALL) core

neat:	
	rm -f .[BC]* .em* core

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $<

