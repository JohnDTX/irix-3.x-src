#
#
include ${ROOT}/usr/include/make/commondefs
.SUFFIXES: .o .p

SRC20 = ../../fortran20

SRCS = f.assign f.call f.dump.1 f.dump.2 f.exec.1 \
	   f.exec.2 f.exec.3 f.exec.4 f.expr f.init f.misc \
	   f.opt f.scan.1 f.scan.2 f.spec f.state f.symb fortran.p

OBJS = fortran.o ../svslib.o

all:	fortran10

install:	all
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" fortran10
	if test $$machine = iris;\
	then \
		${INSTALL} -ln /usr/lib/fortran10 -F /usr/lib \
		-idb "std.sw.unix mr" fortran ;\
	fi

fortran10:	$(OBJS)
	$(PC) $(PCFLAGS) -o fortran10 $(OBJS)

fortran.o: $(SRCS)

.p.o:
	$(PC) $(PCFLAGS) -c $<

f.assign:	$(SRC20)/fortran/f.assign
	rm -f f.assign 
	ln $(SRC20)/fortran/f.assign .

f.call:	$(SRC20)/fortran/f.call
	rm -f  f.call 
	ln $(SRC20)/fortran/f.call .

f.dump.1:	$(SRC20)/fortran/f.dump.1
	rm -f  f.dump.1 
	ln $(SRC20)/fortran/f.dump.1 .

f.dump.2:	$(SRC20)/fortran/f.dump.2
	rm -f  f.dump.2 
	ln $(SRC20)/fortran/f.dump.2 .

f.exec.1:	$(SRC20)/fortran/f.exec.1
	rm -f f.exec.1 
	ln $(SRC20)/fortran/f.exec.1 .

f.exec.2:	$(SRC20)/fortran/f.exec.2
	rm -f  f.exec.2 
	ln $(SRC20)/fortran/f.exec.2 .

f.exec.3:	$(SRC20)/fortran/f.exec.3
	rm -f f.exec.3 
	ln $(SRC20)/fortran/f.exec.3 .

f.exec.4:	$(SRC20)/fortran/f.exec.4
	rm -f  f.exec.4 
	ln $(SRC20)/fortran/f.exec.4 .

f.expr:	$(SRC20)/fortran/f.expr
	rm -f f.expr 
	ln $(SRC20)/fortran/f.expr .

f.init:	$(SRC20)/fortran/f.init
	rm -f f.init 
	ln $(SRC20)/fortran/f.init .

f.misc:	$(SRC20)/fortran/f.misc
	rm -f f.misc 
	ln $(SRC20)/fortran/f.misc .

f.opt:	$(SRC20)/fortran/f.opt
	rm -f  f.opt 
	ln $(SRC20)/fortran/f.opt .

f.scan.1:	$(SRC20)/fortran/f.scan.1
	rm -f  f.scan.1 
	ln $(SRC20)/fortran/f.scan.1 .

f.scan.2:	$(SRC20)/fortran/f.scan.2
	rm -f  f.scan.2 
	ln $(SRC20)/fortran/f.scan.2 .

f.spec:	$(SRC20)/fortran/f.spec
	rm -f  f.spec 
	ln $(SRC20)/fortran/f.spec .

f.state:	$(SRC20)/fortran/f.state
	rm -f  f.state 
	ln $(SRC20)/fortran/f.state .

f.symb:	$(SRC20)/fortran/f.symb
	rm -f  f.symb 
	ln $(SRC20)/fortran/f.symb .

fortran.p:	$(SRC20)/fortran/fortran.p
	rm -f  fortran.p 
	ln $(SRC20)/fortran/fortran.p .


../svslib.o:
	cd ..; $(MAKE) svslib

clean:
	rm -f fortran.o *.obj *.j

clobber:	clean
	rm -f fortran10
