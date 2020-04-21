#
# Makefile for GF2 microassembler
#

LINCS  = ../include/mas.h \
	   ../include/fbcparams.h \
	   ../include/fbc.h \
	   ../include/fbc.mic.h \
	   ../include/libh.h
LCDEFS    = -DUC4 -DUNIX

SRCS     = main.c mas.c endstate.c fbc.c \
	defines.c symbols.c libh.c

WSOBJS = main.o libh.o mas.o fbc.o endstate.o defines.o symbols.o

uc4ws:	$(WSOBJS) 

.c.o: 
	$(CC) $(CFLAGS) -c $< 

# remove files that can be re-created

clean :
	rm -f *.out *.CKP *.BAK .emacs* *.o $(UC3OBJS) $(UC4OBJS)

