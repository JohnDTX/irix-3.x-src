
all:		accept connect select

accept:		accept.c
		cc -O accept.c -o accept -lbsd

connect:	connect.c
		cc -O connect.c -o connect -lbsd -ldbm

select:		select.c
		cc -O select.c -o select -lbsd

clean:
		rm -f accept connect select
