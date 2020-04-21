# compiler switches
CC	=	cc
INCLUDE	=	-I. -I/usr/include
CFLAGS	=	-O -c $(ZF)
LIBS    = 	$(ZF) -Zg
ALL	=	clock cube showmap 
FALL	=	matrix ramp rollem poly3D template

install:	all

all:	$(ALL) $(FALL)

$(ALL):	$$@.c
	$(CC) $(INCLUDE) $? -o $@ $(LIBS) 

$(FALL):	$$@.f
	$(CC) $(INCLUDE) $? -o $@ $(LIBS) 

clean:
	rm -f .[BC]* .em* *.o $(ALL) $(FALL)
	@echo "make clean explorations completed"

neat:
	rm -f .[BC]* .em* *.o
	@echo "make neat explorations completed"

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $<

