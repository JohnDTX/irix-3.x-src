.SUFFIXES: .b .c

OBJS = testmm1.b testmm2.b testmm3.b testmm4.b testcl1.b testcl2.b \
	testcl3.b testcl4.b testcl5.b testcl6.b testsc1.b testgso.b \
	testgsi.b test10sc1.b test10gso.b

CC = cc68 -c -I../include

all: $(OBJS)
	@echo 'tests made.'

.c.b:
	$(CC) $<
