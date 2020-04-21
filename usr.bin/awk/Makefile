#
#
include ${ROOT}/usr/include/make/commondefs

YFLAGS	= -d
LIB	= -lm

FILES	= awk.lx.o b.o main.o token.c tran.o lib.o run.o parse.o proctab.c

SOURCE	= awk.def awk.g.y awk.lx.l b.c lib.c main.c parse.c \
	  proc.c freeze.c run.c token.c tran.c

all:	awk

awk:	$(FILES) awk.g.o
	$(CC) $(LDFLAGS) awk.g.o $(FILES) $(LIB) -o $@

y.tab.h:awk.g.o

awk.h:	y.tab.h
	cp y.tab.h awk.h
	chmod u+w $@

$(FILES):awk.h awk.def

#	Possibly compile for source machine
token.c:awk.h token.c.proto
	rm -f temp
	cp token.c.proto token.c
	chmod u+w token.c
	ex - < tokenscript
	rm -f temp

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" awk

profile:awk.g.o $(FILES) mon.o
	cc -p -i awk.g.o $(FILES) mon.o $$ROOT/usr/lib/libm.a

find:
	egrep -n "$(PAT)" *.[ylhc] awk.def

list:
	-pr $(SOURCE) makefile tokenscript README EXPLAIN

lint:
	lint -spu b.c main.c token.c tran.c run.c lib.c parse.c $$ROOT/usr/lib/libm.a |\
		egrep -v '^(error|free|malloc)'

proctab.c:proc
	./proc >proctab.c

#	Compile for source machine
proc.o:	proc.c
	cc -I${ROOT}/usr/include -c proc.c
	
#	Compile for source machine
proc:	awk.h proc.o token.c
	cc -I${ROOT}/usr/include -o proc proc.o token.c

clean:
	rm -f *.o t.* *temp* *.out *junk* awk.h proc proctab.c \
	temp token.c 

clobber:clean
	rm -f awk

diffs:
	-for i in $(SOURCE); do echo $$i:; diff $$i /usr/src/cmd/awk | ind; done
