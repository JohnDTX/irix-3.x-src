NAME =		bpcd
DIRNAME =	makedir
HDWR =		../..
mdirs = makedir3321 makedir3322 makedir3421 makedir3422\
 makedir4331 makedir4332 makedir4431 makedir4432 makedir4433


all:		4431.r 4331.r 3421.r 3321.r 4432.r 4332.r 3422.r 3322.r \
		4433.r

4431.r:
	- make -f Makefile2 $(NAME)4431.r UC=4 DC=4 INTER=3 PROCESSOR=1 \
	  HDWR=$(HDWR)

4331.r:
	- make -f Makefile2 $(NAME)4331.r UC=4 DC=3 INTER=3 PROCESSOR=1 \
	  HDWR=$(HDWR)

3421.r:
	- make -f Makefile2 $(NAME)3421.r UC=3 DC=4 INTER=2 PROCESSOR=1 \
	  HDWR=$(HDWR)

3321.r:
	- make -f Makefile2 $(NAME)3321.r UC=3 DC=3 INTER=2 PROCESSOR=1 \
	  HDWR=$(HDWR)

4432.r:
	- make -f Makefile2 $(NAME)4432.r UC=4 DC=4 INTER=3 PROCESSOR=2 \
	  HDWR=$(HDWR)

4332.r:
	- make -f Makefile2 $(NAME)4332.r UC=4 DC=3 INTER=3 PROCESSOR=2 \
	  HDWR=$(HDWR)

3422.r:
	- make -f Makefile2 $(NAME)3422.r UC=3 DC=4 INTER=2 PROCESSOR=2 \
	  HDWR=$(HDWR)

3322.r:
	- make -f Makefile2 $(NAME)3322.r UC=3 DC=3 INTER=2 PROCESSOR=2 \
	  HDWR=$(HDWR)

4433.r:
	- make -f Makefile2 $(NAME)4433.r UC=4 DC=4 INTER=3 PROCESSOR=3 \
	  HDWR=$(HDWR)

#
#	The following entries support compilation of an individual file,
#	  rather than of the entire set.  You type (for example)
#
#		make 4431 TARGET=console.c
#
#	  to compile console.c to makedir4431/console.b using
#	  UC4, DC4, INTER3, PM1
#

4431:
	make -f Makefile2 makedir4431/$(TARGET) UC=4 DC=4 INTER=3 \
	  PROCESSOR=1 HDWR=$(HDWR)

4331:
	make -f Makefile2 makedir4331/$(TARGET) UC=4 DC=3 INTER=3 \
	  PROCESSOR=1 HDWR=$(HDWR)

3421:
	make -f Makefile2 makedir3421/$(TARGET) UC=3 DC=4 INTER=2 \
	  PROCESSOR=1 HDWR=$(HDWR)

3321:
	make -f Makefile2 makedir3321/$(TARGET) UC=3 DC=3 INTER=2 \
	  PROCESSOR=1 HDWR=$(HDWR)

4432:
	make -f Makefile2 makedir4432/$(TARGET) UC=4 DC=4 INTER=3 \
	  PROCESSOR=2 HDWR=$(HDWR)

4332:
	make -f Makefile2 makedir4332/$(TARGET) UC=4 DC=3 INTER=3 \
	  PROCESSOR=2 HDWR=$(HDWR)

3422:
	make -f Makefile2 makedir3422/$(TARGET) UC=3 DC=4 INTER=2 \
	  PROCESSOR=2 HDWR=$(HDWR)

3322:
	make -f Makefile2 makedir3322/$(TARGET) UC=3 DC=3 INTER=2 \
	  PROCESSOR=2 HDWR=$(HDWR)

4433:
	make -f Makefile2 makedir4433/$(TARGET) UC=4 DC=4 INTER=3 \
	  PROCESSOR=3 HDWR=$(HDWR)

install:	install4331 install4431 install3321 install3421 \
		install4332 install4432 install3322 install3422 \
		install4433

install4431:
	- cp $(NAME)4431.r $$BOOT

install4331:
	- cp $(NAME)4331.r $$BOOT

install3421:
	- cp $(NAME)3421.r $$BOOT

install3321:
	- cp $(NAME)3321.r $$BOOT

install4432:
	- cp $(NAME)4432.r $$BOOT

install4332:
	- cp $(NAME)4332.r $$BOOT

install3422:
	- cp $(NAME)3422.r $$BOOT

install3322:
	- cp $(NAME)3322.r $$BOOT

install4433:
	- cp $(NAME)4433.r $$BOOT

neat:
	- rm -f *BAK *CKP .emacs* *.*e *.*o *.xsm

clean:		neat
	- rm -f *.b *.r
	for d in $(mdirs) ; do (rm -rf $$d) done
	(cd iolib ; make clean)
