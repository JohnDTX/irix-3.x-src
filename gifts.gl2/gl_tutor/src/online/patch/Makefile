# compiler switches
CC	=	cc
INCLUDE	=	-I. -I/usr/include -I$(TUTOR)/src/include
CFLAGS	=	-O -c -D$(LANGUAGE) $(ZF)
LIBS    = 	$(TUTOR)/src/lib/libtutor.a $(TUTOR)/src/lib/mymath.a \
	$(TUTOR)/src/lib/gltools.a $$TUTOR/src/lib/libtutor.a $(ZF) -Zg
OBJS	=	patch.o help.o front.o sliders.o state.o matrix.o
ALL	=	patch

install:
	ln $(ALL) $(TUTOR)/online

all:	$(ALL)

$(ALL): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(ALL) 

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $<

clean:
	rm -f .[BC]* .em* *.o $(ALL)

neat:
	rm -f .[BC]* .em* *.o
