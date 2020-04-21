#
#
include ${ROOT}/usr/include/make/commondefs


#	@(#)cxref.mk	1.3
#	3.0 SID #	1.2

LINTF	= -p
LINT	= lint
OPRL	= oprl

CPASS1	= cgram.c comm1.c optim.c pftn.c scan.c trees.c \
	  xdefs.c xlocal.c lint.c
OPASS1	= cgram.o comm1.o optim.o pftn.o scan.o trees.o \
	  xdefs.o xlocal.o lint.o

XPASS	= cgram.y comm1.c common lint.c lmanifest macdefs manifest \
	  mfile1 optim.c pftn.c scan.c trees.c xdefs.c xlocal.c

all:	cxref xpass xcpp

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" cxref
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" "xcpp xpass"

clean:
	rm -f *.o y.tab.c cpy.c cgram.c

clobber:clean
	rm -f xpass cxref xcpp

# CXREF
cxref:	cxr.c owner.h
	$(CCF) cxr.c -o $@ $(LDFLAGS) 

# XPASS
xpass:	$(OPASS1)
	$(CCF) $(OPASS1) -o $@ $(LDFLAGS) 

$(OPASS1):manifest macdefs mfile1

cgram.o: cgram.y
	sed -e 's/\/\*CXREF\(.*\)\*\//\1/' cgram.y > gram.y
	yacc gram.y
	mv y.tab.c cgram.c
	$(CCF) -DBUG4 $(FFLAG) -c cgram.c
	rm -f gram.y cgrap.c

comm1.o: common
	$(CCF) -DBUG4 -c comm1.c

lint.o:	lmanifest lint.c
	$(CCF) -c lint.c
	
optim.o:optim.c
	$(CCF) -DBUG4 -c optim.c
	
pftn.o:	pftn.c
	$(CCF) -DBUG4 -c pftn.c
	
scan.o: scan.c 
	$(CCF) -DBUG4 -c scan.c

trees.o:trees.c
	$(CCF) -DBUG4 -c trees.c

xdefs.o: xdefs.c
	$(CCF) -DBUG4 -c xdefs.c
	
xlocal.o:xlocal.c lmanifest
	$(CCF) -c xlocal.c
	
# XCPP
xcpp:	cpp.o cpy.o
	$(CCF) -o $@ cpp.o cpy.o $(LDFLAGS) 

cpp.o:	cpp.c
	$(CCF) -c -Dunix=1 cpp.c

cpy.o:	cpy.y yylex.c
	yacc cpy.y
	mv y.tab.c cpy.c
	$(CCF) -c -Dunix=1 cpy.c
	rm -f cpy.c

# UTILITIES
lint:
	$(LINT) $(LINTF) cxr.c
	$(LINT) $(LINTF) -DBUG4 $(CPASS1)
	$(LINT) $(LINTF) -Dunix=1 -Dcpp.c cpy.c

cxref.list:cxr.c owner.h
	@echo "Listing of cxref"
	$(OPRL) -x cxr.c owner.h
	$(OPRL) -x -C cxr.c
	touch cxref.list

xcpp.list:cpp.c cpy.c yylex.c
	@echo "Listing of xcpp"
	$(OPRL) -x cpp.c cpy.y yylex.c
	$(OPRL) -x -C cpp.c cpy.c
	touch xcpp.list

xpass.list:$(XPASS) cgram.c
	@echo "Listing of xpass"
	$(OPRL) -x $(XPASS)
	$(OPRL) -x -C $(CPASS1)
	touch xpass.list

list:	xcpp xpass cxref cxref.list xcpp.list xpass.list
	$(OPRL) -x Makefile
	touch list
