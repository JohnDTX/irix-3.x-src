#	awk make file
#	SCCS:	@(#)awk.mk	1.4

OL = /
SL = /usr/src/cmd
INSDIR = $(OL)usr/bin
RDIR = $(SL)/awk
YACCRM=-rm
TESTDIR = .
FRC =
INS = /etc/install -n /usr/bin
B10 =
B11 =
YFLAGS = -d
IFLAG = -i
LDFLAGS = -s $(IFLAG) $(FFLAG)
CFLAGS = -O
REL = current
LIST = lp
SOURCE = EXPLAIN README awk.def awk.g.y awk.h awk.lx.l b.c lib.c main.c \
		parse.c proc.c run.c token.c tokenscript tran.c
FILES = awk.lx.o b.o main.o token.o tran.o lib.o run.o parse.o proctab.o 
MAKE = make

all:	awk
	:

awk:	$(FILES) awk.g.o
	$(CC) $(LDFLAGS) awk.g.o $(FILES) $$DEST/usr/lib/libm.a -o $(TESTDIR)/awk

install:	awk
	$(INS) $(TESTDIR)/awk

$(FILES):	awk.h awk.def $(FRC)
run.o:	awk.h awk.def
	cc $(CFLAGS) $(B11) -c run.c
token.c:	awk.h $(FRC)
	ed - <tokenscript
proctab.c:	awk.h proc.c token.c $(FRC)
	cc -o proc proc.c token.c
	-./proc > proctab.c
awk.g.o:	awk.def awk.g.c $(FRC)
	$(CC) $(CFLAGS) $(B10) -c awk.g.c

awk.g.h awk.g.c:	awk.g.y FRC
	-$(YACC) $(YFLAGS) awk.g.y && mv y.tab.h awk.g.h && mv y.tab.c awk.g.c

awk.h:	awk.g.h $(FRC)
	-cmp -s awk.g.h awk.h || cp awk.g.h awk.h

awk.lx.o:	awk.lx.c $(FRC)
	$(CC) $(CFLAGS) -c awk.lx.c

awk.lx.c:	awk.lx.l $(FRC)
	$(LEX) $(LFLAGS) awk.lx.l
	mv lex.yy.c awk.lx.c

FRC:

build:	bldmk
	get -p -r`gsid awk $(REL)` s.awk.src $(REWIRE) | ntar -d $(RDIR) -g
	cd $(RDIR); $(YACC) $(YFLAGS) awk.g.y
	cd $(RDIR); mv y.tab.c awk.g.c; rm y.tab.h

bldmk:;	get -p -r`gsid awk.mk $(REL)` s.awk.mk > $(RDIR)/awk.mk

listing:;	pr awk.mk $(SOURCE) | $(LIST)

listmk:;	pr awk.mk | $(LIST)

edit:;	get -e -p s.awk.src | ntar -g

delta:;	ntar -p $(SOURCE) > awk.src
	delta s.awk.src
	rm -f $(SOURCE)

mkedit:;	get -e s.awk.mk
mkdelta:;	delta s.awk.mk

clean:;	-rm -f *.o temp* core proc proctab.c
	$(YACCRM) -f awk.lx.c awk.g.c awk.g.h

clobber:	clean
	-rm -f $(TESTDIR)/awk

delete:	clobber
	rm -f $(SOURCE)
