LCDEFS  = -O -V -N
LCINCS  = -I../include

# next line must have nothing after the = sign.
NULL=

.SUFFIXES : .ls .s .c .E

.c.o :
	$(NOGL2UNIX) $(CC) $(CFLAGS) $< -DUNIX -DPM2 -c ; \
		rm -f ../uobj/$@; cp $@ ../uobj/$@
	$(NOGL3UNIX) $(CC) $(CFLAGS) $< -DUNIX -DIP2 -c ; \
		rm -f ../u3obj/$@; cp $@ ../u3obj/$@

.c.E :
	$(CC) $(CFLAGS) -E $< > $*.E

.c.s :
	$(CC) -S $(CFLAGS) $<

.c.ls :
	$(CC) -L $(CFLAGS) $<

SRCS =	\
	capture.c \
	$(NULL)

OBJS =	\
	capture.o \
	$(NULL)

all: $(OBJS)

objlist:
	@echo $(OBJS)

install: all

clean:
	rm -f $(OBJS) *.out *.CKP *.BAK .emacs* *.s *.ls

clobber: clean

release:
	release $(SRCS) Makefile Files
