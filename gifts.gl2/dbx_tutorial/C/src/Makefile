#
#	Makefile for the C version of the dbx tutorial.
#
#	    The tutorial is structured to sequentially take the
#	user through the repair of five bugs in a C program 
#	which sorts alphanumeric records (named 'sort').  The
#	versions of sort which has these bugs fixed are named sort0,
#	sort1,... sort5, where sort0 has NO bugs repaired, sort1
#	has ONE bug repaired, etc.  In all cases, one version of
#	sort is considered 'current', and its source and executable
#	are named 'sort'.
#
#	    This Makefile has targets for forcing  the current version
#	('sort') to be the version corresponding to any of sort0 ... sort5.
#	These targets are named 'level0', 'level1', ... 'level5'.  
#	Level5, the level with ALL bugs repaired, has a synonym ('nobugs').
#
#	    Only one tutorial user is allowed to be running in this
#	directory at a time.  If there is a current user, the file 
#	'.LOCK' will appear, which will have the users id in it.
#	Only the owner of the lock file will be allowed to 'make' 
#	in the directory. When a user is done with 
#	a tutorial session, he/she should
#	do a make of one of 'save', or 'done'.  A 'make save' will
#	save the current source as the user's private copy in a 
#	file named USER.sort.c, where USER is the
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
#		source (sort.c) and make an executable out of it.  
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

CFLAGS = -g

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
		if test -f $$LOGNAME.sort.c;\
		then\
			rm -f sort.c;\
			cp $$LOGNAME.sort.c sort.c;\
		else \
			make sort0.c;\
			ln sort0.c sort.c;\
		fi \
	fi

restore: accessible

save: accessible
	@if test -f sort.c ;\
	then \
		cp sort.c $$LOGNAME.sort.c ;\
	fi
	@make clobber
	@rm -f .LOCK 

done:
	@make clobber
	@rm -f .LOCK $$LOGNAME.sort.c

sort.c:
	make sort0.c
	ln sort0.c sort.c

sort0.c:	sort.m
	@/lib/cpp -C -P -DBUGLEV=0 sort.m /tmp/sort0.c
	@tr "~@" "*/" </tmp/sort0.c >sort0.c
	@chmod uog+rw sort0.c

sort1.c:	sort.m
	@/lib/cpp -C -P -DBUGLEV=1 sort.m /tmp/sort1.c
	@tr "~@" "*/" </tmp/sort1.c >sort1.c
	@chmod uog+rw sort1.c

sort2.c:	sort.m
	@/lib/cpp -C -P -DBUGLEV=2 sort.m /tmp/sort2.c
	@tr "~@" "*/" </tmp/sort2.c >sort2.c
	@chmod uog+rw sort2c

sort3.c:	sort.m
	@/lib/cpp -C -P -DBUGLEV=3 sort.m /tmp/sort3.c
	@tr "~@" "*/" </tmp/sort3.c >sort3.c
	@chmod uog+rw sort3.c

sort4.c:	sort.m
	@/lib/cpp -C -P -DBUGLEV=4 sort.m /tmp/sort4.c
	@tr "~@" "*/" </tmp/sort4.c >sort4.c
	@chmod uog+rw sort4.c

sort5.c:	sort.m
	@/lib/cpp -C -P -DBUGLEV=5 sort.m /tmp/sort5.c
	@tr "~@" "*/" </tmp/sort5.c >sort5.c
	@chmod uog+rw sort5.c

level0:	sort0.c
	@rm -f sort.c sort
	ln sort0.c sort.c
	make sort

level1:	sort1.c
	@rm -f sort.c sort
	ln sort1.c sort.c
	make sort

level2:	sort2.c
	@rm -f sort.c sort
	ln sort2.c sort.c
	make sort

level3:	sort3.c
	@rm -f sort.c sort
	ln sort3.c sort.c
	make sort

level4:	sort4.c
	@rm -f sort.c sort
	ln sort4.c sort.c
	make sort

nobugs level5:	sort5.c
	@rm -f sort.c sort
	ln sort5.c sort.c
	make sort

clean:
	rm -f sort sort? sort*.o

clobber: clean
	rm -f sort*.c 
	rm -f  core names.out \-o
