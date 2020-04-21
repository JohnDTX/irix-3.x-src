#	diff3 make file
#	SCCS:	@(#)diff3.mk	1.4

OL = /
SL = /usr/src/cmd
RDIR = $(SL)/diff3
INS = :
REL = current
SSID = -r`gsid diff3 $(REL)`
CSID = -r`gsid diff3prog $(REL)`
MKSID = -r`gsid diff3.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SHSOURCE = diff3.sh
CSOURCE = diff3prog.c
MAKE = make

compile all: diff3 diff3prog
	:

diff3:
	cp diff3.sh diff3
	$(INS) diff3 $(INSDIR)
	chmod 775 $(INSDIR)/diff3
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin diff3; chgrp bin diff3; \
	 fi

diff3prog:
	$(CC) $(CFLAGS) $(LDFLAGS) -o diff3prog diff3prog.c
	$(INS) diff3prog $(INSLIB)
	chmod 775 $(INSLIB)/diff3prog
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSLIB); chown bin diff3prog; chgrp bin diff3prog; \
	 fi

install:
	$(MAKE) -f diff3.mk INS=cp OL=$(OL)
insdif3p:
	$(MAKE) -f diff3.mk INS=cp OL=$(OL) diff3prog

build:	bldmk blddif3p
	get -p $(SSID) s.diff3.sh $(REWIRE) > $(RDIR)/diff3.sh
blddif3p:
	get -p $(CSID) s.diff3prog.c $(REWIRE) > $(RDIR)/diff3prog.c
bldmk:
	get -p $(MKSID) s.diff3.mk > $(RDIR)/diff3.mk

listing:  ;	pr diff3.mk $(SHSOURCE) $(CSOURCE) | $(LIST)
listdif3: ;	pr $(SHSOURCE) | $(LIST)
lsitdif3p: ;	pr $(CSOURCE) | $(LIST)
listmk: ;	pr diff3.mk | $(LIST)

edit:
	get -e s.diff3.sh
dif3pedit:
	get -e s.diff3prog.c

delta:
	delta s.diff3.sh
dif3pdelta:
	delta s.diff3prog.c

mkedit:  ;  get -e s.diff3.mk
mkdelta: ;  delta s.diff3.mk

clean:
	:

clobber:
	rm -f diff3 diff3prog

delete:	clobber
	rm -f $(SHSOURCE) $(CSOURCE)
