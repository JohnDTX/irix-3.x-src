#	bc make file
#	SCCS:	@(#)bc.mk	1.2

OL = /
SL = /usr/src/cmd
RDIR = $(SL)/bc
INS = :
REL = current
CSID = -r`gsid bc $(REL)`
MKSID = -r`gsid bc.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
B20 =
CFLAGS = -O $(B20)
FFLAG =
IFLAG = -n
LDFLAG = -s $(IFLAG) $(FFLAG)
SOURCE = bc.y lib.b.sh
FILES = bc.c
MAKE = make
YACC = yacc

compile all: bc lib.b
	:

bc:	$(FILES)
	$(CC) $(CFLAGS) $(LDFLAG) -o bc $(FILES)
	$(INS) bc $(INSDIR)
	chmod 775 $(INSDIR)/bc
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin bc; chgrp bin bc; \
	 fi

$(FILES):
	-$(YACC) bc.y && mv y.tab.c bc.x
	cp bc.x bc.c

lib.b:
	cp lib.b.sh lib.b
	$(INS) lib.b $(INSLIB)
	chmod 775 $(INSLIB)/lib.b
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSLIB); chown bin lib.b; chgrp bin lib.b; \
	 fi
install:
	$(MAKE) -f bc.mk INS=cp IFLAG=$(IFLAG) FFLAG=$(FFLAG) OL=$(OL)

build:	bldmk
	get -p $(CSID) s.bc.src $(REWIRE) | ntar -d $(RDIR) -g
	cd $(RDIR); $(YACC) bc.y; mv y.tab.c bc.x

bldmk:  ;  get -p $(MKSID) s.bc.mk > $(RDIR)/bc.mk

listing:
	pr bc.mk $(SOURCE) | $(LIST)
listmk: ;  pr bc.mk | $(LIST)

edit:
	get -e -p s.bc.src | ntar -g

delta:
	ntar -p $(SOURCE) > bc.src
	delta s.bc.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.bc.mk
mkdelta: ;  delta s.bc.mk

clean:
	:

clobber:	clean
	rm -f bc bc.c lib.b bc.x

delete:	clobber
	rm -f $(SOURCE) bc.x
