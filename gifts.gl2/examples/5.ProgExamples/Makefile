INCLUDES =  	-I. -I/usr/include
LIBS	 =	-lgl -lm
EXEC 	 =	first tpbig doily bounce iobounce draw popup   \
		trans vlsi colored 

all: $(EXEC)

first: first.o
	cc $(INCLUDES) first.o $(LIBS) -o first

tpbig: tpbig.o
	cc $(INCLUDES) tpbig.o $(LIBS) -o tpbig

doily: doily.o
	cc $(INCLUDES) doily.o $(LIBS) -o doily

bounce: bounce.o
	cc $(INCLUDES) bounce.o $(LIBS) -o bounce

iobounce: iobounce.o
	cc $(INCLUDES) iobounce.o $(LIBS) -o iobounce

draw: draw.o
	cc $(INCLUDES) draw.o $(LIBS) -o draw

popup: popup.o
	cc $(INCLUDES) popup.o $(LIBS) -o popup

trans: trans.o popuputils.o
	cc $(INCLUDES) trans.o popuputils.o $(LIBS) -o trans

vlsi: vlsi.o
	cc $(INCLUDES) vlsi.o $(LIBS) -o vlsi

colored: colored.o
	cc $(INCLUDES) colored.o $(LIBS) -o colored

.o.o:
	cc -c $(INCLUDES) $<

clean:
	rm -f *.o $(EXEC) core .[BC]*

neat:
	rm -f *.o core .[BC]*


