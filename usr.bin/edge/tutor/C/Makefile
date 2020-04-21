#
#	Makefile for the C version of the dbx tutorial.
#

default:
	make sort

restore:
	@if [ -f $$LOGNAME.sort.c ]; \
	then \
		make clean ;\
		cp $$LOGNAME.sort.c sort.c ;\
	fi

save:
	@if [ -f sort.c ]; \
	then \
		cp sort.c $$LOGNAME.sort.c ;\
		make clean;\
	fi

done: clean
	@rm -f sort*.c names.out $$LOGNAME.sort.c ../sort

sort: sort.c
#	cc -g -o sort sort.c

sort.c: sort.m
	@cp sort.m sort.c
	@chmod uog+rw sort.c

clean:
	@csh scrub
	@rm -f sort sort.c core sort*.o names.out scrub.out
