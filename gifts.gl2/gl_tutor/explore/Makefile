# compiler switches
CC	=	cc
INCLUDE	=	-I. -I/usr/include
CFLAGS	=	-O -c $(ZF)
LIBS    = 	$(ZF) -Zg
ALL	=	clock cube matrix poly3D ramp rollem showmap \
	template 

install:	all

all:	$(ALL)

$(ALL):	$$@.c
	$(CC) $(INCLUDE) $? -o $@ $(LIBS) 

clean:
	rm -f .[BC]* .em* *.o $(ALL)
	@echo "make clean explorations completed"

neat:
	rm -f .[BC]* .em* *.o
	@echo "make neat explorations completed"

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $<

