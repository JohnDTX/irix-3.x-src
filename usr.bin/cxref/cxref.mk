#	@(#)cxref.mk	1.3
#	3.0 SID #	1.2
OWNER=/usr/lib
CC=cc
LINTF = -p
LINT = lint
OPRL = oprl
LFLAG1 = -i
LFLAG2 = -n
FFLAG =
CFLAGS = -O -c -DCXREF
CPASS1 =	cgram.c comm1.c optim.c pftn.c scan.c trees.c \
		xdefs.c xlocal.c lint.c
OPASS1 =	cgram.o comm1.o optim.o pftn.o scan.o trees.o \
		xdefs.o xlocal.o lint.o

XPASS =	cgram.y comm1.c common lint.c lmanifest macdefs manifest \
	mfile1 optim.c pftn.c scan.c trees.c xdefs.c xlocal.c

all :	cxref xpass xcpp

# CXREF

cxref :	cxr.c owner.h
	$(CC) $(FFLAG) -O cxr.c -o cxref

# XPASS

xpass:	$(OPASS1)
	$(CC) $(FFLAG) $(LFLAG1) $(OPASS1) -o xpass

$(OPASS1): manifest macdefs mfile1

cgram.c: cgram.y
	sed -e 's/\/\*CXREF\(.*\)\*\//\1/' cgram.y > gram.y
	yacc gram.y
	mv y.tab.c cgram.c
	-rm -f gram.y

cgram.o:	cgram.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) cgram.c

comm1.o: common
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) comm1.c

lint.o:	lmanifest lint.c
	$(CC) $(FFLAG) $(CFLAGS) lint.c
	
optim.o:	optim.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) optim.c
	
pftn.o:		pftn.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) pftn.c
	
scan.o: scan.c 
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) scan.c

trees.o:	trees.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) trees.c

xdefs.o: xdefs.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) xdefs.c
	
xlocal.o:	xlocal.c lmanifest
	$(CC) $(FFLAG) $(CFLAGS) xlocal.c
	
# XCPP

xcpp:	cpp.o cpy.o
	$(CC) $(FFLAG) $(LFLAG2) -o xcpp cpp.o cpy.o

cpp.o:	cpp.c
	$(CC) $(FFLAG) $(CFLAGS) -Dunix=1 cpp.c

cpy.o:	cpy.c yylex.c
	$(CC) $(FFLAG) $(CFLAGS) -Dunix=1 cpy.c

cpy.c:	cpy.y
	yacc cpy.y
	mv y.tab.c cpy.c

# UTILITIES

install :	all
	cp cxref /usr/bin
	cp xpass $(OWNER)
	cp xcpp $(OWNER)

clean:
	-rm -f *.o  

clobber:	clean
	-rm -f xpass cgram.c cxref xcpp cpy.c

lint:
	$(LINT) $(LINTF) cxr.c
	$(LINT) $(LINTF) -DBUG4 $(CPASS1)
	$(LINT) $(LINTF) -Dunix=1 -Dcpp.c cpy.c

cxref.list:	cxr.c owner.h
	@echo "Listing of cxref"
	$(OPRL) -x cxr.c owner.h
	$(OPRL) -x -C cxr.c
	touch cxref.list

xcpp.list:	cpp.c cpy.c yylex.c
	@echo "Listing of xcpp"
	$(OPRL) -x cpp.c cpy.y yylex.c
	$(OPRL) -x -C cpp.c cpy.c
	touch xcpp.list

xpass.list:	$(XPASS) cgram.c
	@echo "Listing of xpass"
	$(OPRL) -x $(XPASS)
	$(OPRL) -x -C $(CPASS1)
	touch xpass.list

list:	$(OWNER)/bin/xcpp $(OWNER)/bin/xpass $(OWNER)/bin/cxref \
	cxref.list xcpp.list xpass.list
	$(OPRL) -x Makefile
	touch list
