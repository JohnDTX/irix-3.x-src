#

.SUFFIXES: .b .c

OBJS = headin0.b headin1.b headin2.b headin3.b headin4.b headin5.b \
	headin6.b headin7.b headin8.b tailin0.b tailin1.b tailin2.b \
	tailin3.b tailin4.b headin9.b headout9.b \
	headout0.b headout1.b headout2.b headout3.b headout4.b headout5.b \
	headout6.b headout7.b headout8.b tailout0.b tailout1.b tailout2.b \
	tailout3.b tailout4.b

all: $(OBJS)


.c.b:
	cc68 -c -I.. $<
