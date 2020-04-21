#
#	Makefile for the FORTRAN version of the dbx tutorial.
#

default:
	make sort

restore:
	@if [ -f $$LOGNAME.sort.f ];\
	then \
		make clean ;\
		cp $$LOGNAME.sort.f sort.f ;\
	fi

save:
	@if [ -f sort.f ];\
	then \
		cp sort.f $$LOGNAME.sort.f ;\
		make clean;\
	fi

done: clean
	@rm -f sort*.f names.out $$LOGNAME.sort.f

sort: sort.f
#	f77 -g -o sort sort.f 

sort.f: sort.m
	@cp sort.m sort.f
	@chmod uog+rw sort.f

clean:
	@csh scrub
	@rm -f sort sort.f core sort*.o *.dgb names.out scrub.out
