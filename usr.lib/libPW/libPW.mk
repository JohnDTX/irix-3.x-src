LNAME = libPW.a

OBJ1 = abspath.o anystr.o bal.o curdir.o fdfopen.o giveup.o
OBJ2 = imatch.o index.o lockit.o logname.o move.o patoi.o
OBJ3 = patol.o regcmp.o regex.o rename.o repeat.o repl.o satoi.o
OBJ4 = setsig.o sname.o strend.o substr.o trnslat.o userdir.o
OBJ5 = username.o verify.o any.o xalloc.o xcreat.o xlink.o
OBJ6 = xopen.o xpipe.o xunlink.o xwrite.o xmsg.o alloca.o
OBJ7 = cat.o dname.o fatal.o clean.o userexit.o zero.o zeropad.o

init:
	@-if /bin/vax;\
	then\
		cp vax/move.s move.s;\
		cp vax/alloca.s alloca.s;\
	elif /bin/u370;\
	then\
		cp u370/move.c move.c;\
		cp u370/alloca.c alloca.c;\
	elif /bin/u3b;\
	then\
		cp u3b/move.c move.c;\
		cp u3b/alloca.s alloca.s;\
	else\
		cp pdp11/move.c move.c;\
		cp pdp11/alloca.s alloca.s;\
	fi

$(LNAME):	init $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7)
	-rm -f $(LNAME)
	ar r $(LNAME) $(OBJ1)
	ar r $(LNAME) $(OBJ2)
	ar r $(LNAME) $(OBJ3)
	ar r $(LNAME) $(OBJ4)
	ar r $(LNAME) $(OBJ5)
	ar r $(LNAME) $(OBJ6)
	ar r $(LNAME) $(OBJ7)
	-if /bin/pdp11 ; \
		then strip $(LNAME) ; \
		else strip -r $(LNAME) ; \
		     ar ts $(LNAME) ; \
	fi
install:	$(LNAME)
	/etc/install -n /usr/lib $(LNAME)

clean:
	-rm -f $(OBJ1)
	-rm -f $(OBJ2)
	-rm -f $(OBJ3)
	-rm -f $(OBJ4)
	-rm -f $(OBJ5)
	-rm -f $(OBJ6)
	-rm -f $(OBJ7)
	-rm -f move.c move.s alloca.c alloca.s $(LNAME)

clobber:	clean
