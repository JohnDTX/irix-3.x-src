# compiler switches
CC	=	cc
INCLUDE	=	-I. -I/usr/include -I$$TUTOR/src/include
CFLAGS	=	-O -c $(ZF)
LIBS    = 	$$TUTOR/src/lib/libtutor.a $$TUTOR/src/lib/digits.a \
	$$TUTOR/src/lib/mymath.a $$TUTOR/src/lib/gltools.a $(ZF) -Zg
OBJS	=	backface.o front.o example.o 
ALL	=	backface

install:
	ln $(ALL) $$TUTOR/online

all:	$(ALL)

$(ALL): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(ALL) 

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $<

clean:
	rm -f .[BC]* .em* *.o $(ALL)

neat:
	rm -f .[BC]* .em* *.o
