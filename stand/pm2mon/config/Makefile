CFLAGS = -I../sys/gl2 -I../include

all: gl2config

clean:
	rm -f gl2config *.prom *.o

install: all
	gl2config -o il_nih.s -l i -h n
	as -o il_nih.o il_nih.s
	ld -o il_nih.prom -T fb0000 il_nih.o
	gl2config -o il_rh.s -l i -h r
	as -o il_rh.o il_rh.s
	ld -o il_rh.prom -T fb0000 il_rh.o
	gl2config -o nil_rh.s -l n -h r
	as -o nil_rh.o nil_rh.s
	ld -o nil_rh.prom -T fb0000 nil_rh.o

gl2config: gl2config.o
	cc -o gl2config gl2config.o


