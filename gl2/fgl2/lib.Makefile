.SUFFIXES:
.SUFFIXES: .o .fc

OBJS	=$(SRCS:.fc=.o)

.fc.o: 
	mkf2c $*.fc $*.s
	as -o $*.o $*.s
	rm -f $*.s

all:	$(OBJS)
	touch lastdone
