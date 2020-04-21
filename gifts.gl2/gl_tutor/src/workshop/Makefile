# compiler switches
CC	=	cc
INCLUDE	=	-I. -I/usr/include
CFLAGS	=	-O -c $(ZF)
LIBS    = 	$(ZF) -Zg
ALL	=	diamond1 colored2 double3 overlay4 poll5 aim6 queue7 \
	menu8 threed9 coord10 translate11 composite12 view13 parabola14

install:	all
	-ln $(ALL) $$TUTOR/workshop
	@echo "make install workshops completed"

all:	$(ALL)
	@echo "make all workshops completed"

$(ALL):	$$@.c
	$(CC) $(INCLUDE) $? -o $@ $(LIBS) 

clean:
	rm -f .[BC]* .em* *.o $(ALL) core
	@echo "make clean workshops completed"

neat:
	rm -f .[BC]* .em* *.o core
	@echo "make neat workshops completed"

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $<

