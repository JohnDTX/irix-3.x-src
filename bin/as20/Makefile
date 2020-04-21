
include ${ROOT}/usr/include/make/commondefs

LCDEFS = -DNEWJXX -DSDIPASS2 
LCINCS = -I. -I${ROOT}/usr/include 
LCOPTS = -O
BINARIES = cmap.o scan.o statement.o error.o \
	   ps.o instparse.o instencode.o code.o stab.o date.o \
	   sym.o alloc.o instea.o template.o insthash.o update.o spinst.o

STRIP	= strip

all: as20 as10

date:
	rm -f date.o
	make date.o

system:  all

as20: date template date.o main20.o $(BINARIES)
	make quick20	
 
as10: date template date.o main10.o $(BINARIES)
	make quick10
 
quick20:
	$(CCF) -o as20 $(BINARIES) main20.o $(LDFLAGS) 

quick10:
	$(CCF) -o as10 $(BINARIES) main10.o $(LDFLAGS) 

date.o:  mkdate
	rm -f date.c
	./mkdate > date.c
	$(CCF) -c date.c 

mkdate: mkdate.c
	${CCF} -DVERSION="\"2.6/3.6\"" $(DEBUG_FLAG) mkdate.c -o mkdate

main.o: scan.h globals.h
scan.o: scan.c scan.h tokens.h globals.h
error.o: error.c globals.h
ps.o: globals.h ps.h tokens.h sym.h
sym.o: sym.h globals.h sym.c tokens.h
alloc.o: sym.h globals.h alloc.c
statement.o : statement.c sym.h globals.h scan.h tokens.h ps.h
template.o : inst.h template.c
instparse.o : sym.h globals.h inst.h tokens.h addrmodes.h
instencode.o : sym.h globals.h inst.h tokens.h addrmodes.h
spinst.o : sym.h globals.h inst.h tokens.h addrmodes.h
instea.o : inst.h
insthash.o : inst.h
code.o : sym.h globals.h 
update.o : sym.h globals.h 
cmap.o : tokens.h

template: mktemplate template.c insthash.c

template.c: template.tab globals.h addrmodes.h sym.h inst.h
	rm -f template.c insthash.c
	./mktemplate template.tab

insthash.c: template.tab
	rm -f template.c insthash.c
	./mktemplate template.tab

main20.o: main.c
	$(CCF) -c main.c -o main20.o
	
main10.o: main.c
	$(CCF) -DM68010_ONLY -c main.c -o main10.o
	

mktemplate: mktemplate.c
	$(CCF) -o mktemplate mktemplate.c

.c.o:
	$(CCF) -c $<

clean:
	rm -f *.[osjai] template.c insthash.c date.c

clobber: clean
	rm -f mktemplate as10 as20 mkdate

install: all
	$(STRIP) as20
	${INSTALL} -m 755 -g bin -u bin -idb "root upd" -F /bin as20
	$(STRIP) as10
	${INSTALL} -m 755 -g bin -u bin -idb "root upd" -F /bin as10
	if test $$machine = juniper;\
	then \
		${INSTALL} -ln /bin/as20 -idb "root upd" -F /bin as;\
	else \
		${INSTALL} -ln /bin/as10 -idb "root upd" -F /bin as;\
	fi


