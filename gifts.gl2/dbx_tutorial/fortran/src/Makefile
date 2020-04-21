#
#	Makefile for the FORTRAN version of the dbx tutorial.
#
#	    The tutorial is structured to sequentially take the
#	user through the repair of five bugs in a FORTRAN program 
#	which sorts alphanumeric records (named 'sort').  The
#	versions of sort which has these bugs fixed are named sort0,
#	sort1,... sort4, where sort0 has NO bugs repaired, sort1
#	has ONE bug repaired, etc.  In all cases, one version of
#	sort is considered 'current', and its source and executable
#	are named 'sort'.
#
#	    This Makefile has targets for forcing  the current version
#	('sort') to be the version corresponding to any of sort0 ... sort4.
#	These targets are named 'level0', 'level1', ... 'level4'.  
#	Level4, the level with ALL bugs repaired, has a synonym ('nobugs').
#
#	    Only one tutorial user is allowed to be running in this
#	directory at a time.  If there is a current user, the file 
#	'.LOCK' will appear, which will have the users id in it.
#	Only the owner of the lock file will be allowed to 'make' 
#	in the directory. When a user is done with 
#	a tutorial session, he/she should
#	do a make of one of 'save', or 'done'.  A 'make save' will
#	save the current source as the user's private copy in a 
#	file named USER.sort.f, where USER is the
#	user's id.  'Make save' will then issue a 'make done'.
#	A 'make done' will unlock the directory and clobber
#	it (put it in a state to begin a new tutorial session).  If
#	the user wants to restore a saved tutorial, a 'make restore' is
#	done.  This will restore the user's private copy, if one exists,
#	as the current copy.
#	
#	
#	    A simple 'make' will do one of the following:
#
#		If the directory is locked, and the lock
#		is owned by the current user, it will take the current 
#		source (sort.f) and make an executable out of it.  
#		Thus, as the tutorial progresses, after the user 
#		fixes a bug, a 'make' will recompile the current 
#		(just fixed) source and install it as the current 
#		executable.  If no current source exists, a level0 
#		make will be done.
#
#		If the directory is unlocked, a 'make restore' will
#		be attempted.  If no saved version exists, a 
#		'make level0' will be done.
#		
#	
#		

#F77FLAGS = -g -ZF+charequ -ZF-dc
F77FLAGS = -g 
CFLAGS = -g 
LDFLAGS = -g

default: accessible
	make sort

accessible:
	@if test -f .LOCK;\
	then \
		if test -w .LOCK;\
		then \
			exit 0;\
		else \
			echo 'tutorial currently in use:' ;\
			ls -l .LOCK ;\
			exit 1;\
		fi \
	else \
		touch .LOCK;\
		chmod og-w .LOCK;\
		if test -f $$LOGNAME.sort.f;\
		then\
			rm -f sort.f;\
			cp $$LOGNAME.sort.f sort.f;\
		else \
			make sort0.f;\
			ln sort0.f sort.f;\
		fi \
	fi

restore: accessible

save: accessible
	@if test -f sort.f ;\
	then \
		cp sort.f $$LOGNAME.sort.f ;\
	fi
	@make clobber
	@rm -f .LOCK 

done:
	@make clobber
	@rm -f .LOCK $$LOGNAME.sort.f

sort: sort.o 
	f77 -o sort $(F77FLAGS) sort.o 

sort.f:
	make sort0.f
	ln sort0.f sort.f

sort0.f:	sort.m
	@/lib/cpp -C -P -DBUGLEV=0 sort.m /tmp/sort0.f
	tr "%" "#" </tmp/sort0.f >sort0.f
	@rm /tmp/sort0.f
	@chmod uog+rw sort0.f

sort1.f:	sort.m
	@/lib/cpp -C -P -DBUGLEV=1 sort.m /tmp/sort1.f
	tr "%" "#" </tmp/sort1.f >sort1.f
	@rm /tmp/sort1.f
	@chmod uog+rw sort1.f

sort2.f:	sort.m
	@/lib/cpp -C -P -DBUGLEV=2 sort.m /tmp/sort2.f
	tr "%" "#" </tmp/sort2.f >sort2.f
	@rm /tmp/sort2.f
	@chmod uog+rw sort2.f

sort3.f:	sort.m
	@/lib/cpp -C -P -DBUGLEV=3 sort.m /tmp/sort3.f
	tr "%" "#" </tmp/sort3.f >sort3.f
	@rm /tmp/sort3.f
	@chmod uog+rw sort3.f

sort4.f:	sort.m
	@/lib/cpp -C -P -DBUGLEV=4 sort.m /tmp/sort4.f
	tr "%" "#" </tmp/sort4.f >sort4.f
	@rm /tmp/sort4.f
	@chmod uog+rw sort4.f

level0:	sort0.f
	@rm -f sort.f sort sort.o
	ln sort0.f sort.f
	make sort

level1:	sort1.f
	@rm -f sort.f sort sort.o
	ln sort1.f sort.f
	make sort

level2:	sort2.f
	@rm -f sort.f sort sort.o
	ln sort2.f sort.f
	make sort

level3:	sort3.f
	@rm -f sort.f sort sort.o
	ln sort3.f sort.f
	make sort

nobugs level4:	sort4.f
	@rm -f sort.f sort sort.o
	ln sort4.f sort.f
	make sort

clean:
	rm -f sort sort? *.o *.dbg

clobber: clean
	rm -f sort*.f 
	rm -f  core names.out \-o
