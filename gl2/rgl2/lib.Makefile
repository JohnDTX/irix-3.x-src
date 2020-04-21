.SUFFIXES:
.SUFFIXES: .o .c

OBJS	=$(SRCS:.c=.o)

all:	$(OBJS)
	touch lastdone

clean:
	rm -f *.c
	rm -f *.o

.c.o: 
	cc -O -c -I.. *.c
