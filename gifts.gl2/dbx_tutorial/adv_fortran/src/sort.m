%define foo 0
c
c
c     sort.f -	This program does a quick sort of its input, 
c		writing the sorted list to the named output
c		file.  Each sort element comprises an entire line
c		in both the input and the output.  This program
c		uses the qsort() package of the C library.
c
c	usage:
c		sort [-i] -o ofile ifile
c
c	  where
c		ofile	is the name of the output file.
c		ifile	is the name of the input file.
c		-i	is used if the case of characters
c			  is to be ignored (e.g. 'A' == 'a').
c
c
c

#define MAXRECLEN 80
#define FILENAMELEN 40
#if BUGLEV<4
#define islower(c) ( ( c .ge. 'A') .and. ( c .le. 'z') ) 
#else
#define islower(c) ( ( c .ge. 'A') .and. ( c .le. 'Z') ) 
#endif
#define tolower(c) char( ichar(c) + ichar('a') - ichar('A') )

$include sort.h

c	the input and output file names 
	character * FILENAMELEN outputfile, inputfile

c	the status of the ignore-case option. (nonzero -> ignore case.)  
	integer*4 ignorecase

c	an array of input records 
	character * MAXRECLEN rec(MAXRECS)
	common ignorecase, rec

c	and an array of indices
	integer index(MAXRECS)

c	a dummy counter.  Used as index into argv[] 
	integer*4 i

c	curarg is the current command-line argument 
	character * 100 curarg

c	unit numbers for the input and output files 
	integer ip,op
	parameter (ip=3,op=4)

c	the number of records being processed 
	integer nrecs

c	the number of arguments to the program
	integer argc

c	our routine to compare two records 
	external cmprec,iargc

c	our routine to read the file of input records
	integer readrecs

	data ignorecase / 0 /
	data i /1/

c	process the arguments.  legal switches are
c
c		-o <filename> 	output filename.
c		-i		ignore case
c
c	  the only other legal argument is the input filename.
c
c	  Example:
c		sort -i input -o output
c
c	  would sort input onto output and ignore case during
c	  the sort.
c
c	
c 	curarg = '   '
 	outputfile = '    '
 	inputfile = '    '
	argc = iargc()

	do 100 i=2,argc
#if BUGLEV>0
c		get the current arg 
		call getarg(i,curarg)
#endif

c		is it a switch? 
		if (curarg(1:1) .eq. '-') then

c			yes.  process the switch 

			if (curarg(2:2) .eq. 'i') then
				ignorecase = 1
#if BUGLEV<3
c				bump the counter 
				i = i + 1
#endif
			elseif (curarg(2:2) .eq. 'o') then
c				the output file name follows 
#if BUGLEV>1
c				increment past the switch.  The
c				next argument is the name of the output
c				file
				i = i + 1
#endif
c				get the output filename 
				if (i .le. argc) then
					call getarg(i,outputfile)
				else 
c					ran out of arguments 
					call fatal(
     1					   'output filename must follow -o')
				endif
			else
c				no other switches are recognized 
				call fatal2c('unrecognized switch - ',curarg)
			endif
		else
c			the argument isn't a switch.  It must 
c			  the name of the input file 
			inputfile = curarg
		endif

 100	continue

c	open the files 
	open(ip,status='old',file=inputfile,form='formatted',err=90001)
c	if ((ip = fopen(inputfile,"r")) == NULL) 
	open(op,status='new',file=outputfile,form='formatted',err=90002)
c	if ((op = fopen(outputfile,"w")) == NULL) 

c	read the input file. 
	nrecs = readrecs(ip,rec)

c	generate the array of indices.  To start with, index(1) = 1,
c	index(2) = 2, etc.  After the sort, index(1) = N, where N is
c	the index of the record found to be lexographically smallest. 
	do 110 i = 1,nrecs
 110	index(i) = i

c	do the sort 
	write(*,1000)
 1000	format(/,'sorting...')

c	Qsort is a C library routine which takes four arguments.  
c	These are
c		the array of record indices (index)
c		the number of records (nrecs)
c		the size of an element (the size of an integer)
c		the address of a routine to pass pointers to two
c			elements to compare them (cmprec)
c	
c	Since qsort must call our routine (cmprec) and 
c	it expects cmprec to be a C routine, we
c	cannot call qsort directly.  Instead, we call a C routine fsort
c	to do the interfacing.
c
d	write(*,*) 'indices:',(index(i),i=1,nrecs)
	call fsort(index,nrecs,4)
d	write(*,*) 'sorted indices:',(index(i),i=1,nrecs)

c	write out the result 
	do 120, i=1,nrecs
		write(op,1010) rec(index(i))
 1010		format(a)
  120	continue

	write(*,1011) nrecs, inputfile, outputfile
 1011	format(i5,' records sorted from input file ',a,
     1  	/,8x,'onto output file ',a)

	go to 99999
	
90001	call fatal2c('cant open input file ',inputfile)
	go to 99999

90002	call fatal2c('cant open output file ',outputfile)

99999	close(ip)
	close(op)
	end


	integer function readrecs(inputunit,rec)
	integer inputunit
	character * MAXRECLEN rec(*)

c	read the input file and fill in the array of
c	records to be sorted.  

$include sort.h

	integer len
	integer nextrecno

	do 200 i=1,MAXRECS+1

		nextrecno=i

c		have we read too many records? 
		if (nextrecno .gt. MAXRECS) go to 200

c		get the next record 
		read(unit=inputunit,fmt=2000,end=91000,err=91001) 
     1			rec(nextrecno)
 2000		format(a)

  200	continue

c	the standard loop exit is only taken if an attempt is made
c	to read too many records.
     	call fatal2i('too many records in input. Max ',MAXRECS)

c	the end-of-file exit is the normal exit.
91000	continue
	readrecs = nextrecno - 1
	return

c	the read error exit.
91001	call fatal2i('error while reading record #',nextrecno)
	end

	integer function cmprec(index0,index1)

	integer index0,index1

$include sort.h

c	cmprec() is effectively called by qsort.  qsort passes
c	it the indices of two records and expects it to 
c	return an integer value 0 if the associated records are
c	equal, > 0 if the first record is 'greater' than the second, 
c	< 0 if the first record is 'less' than the second.

c	buffers for lowercasing records.  Used if ignoring case. 
	character * MAXRECLEN tempbuf0,tempbuf1,lower
	external lower
	character * MAXRECLEN rec(MAXRECS)
	common ignorecase, rec
	integer retval
d
d	write(*,*) 'index0 = ',index0, 'index1 = ', index1
d	write(*,*) 'rec0 = ',rec(index0)
d	write(*,*) 'rec1 = ',rec(index1)
d
	if (ignorecase .ne. 0) then

c		lowercase the records, and place the result in
c		the temporary buffers.
		tempbuf0 = lower(rec(index0))
		tempbuf1 = lower(rec(index1))
		if (tempbuf0 .lt. tempbuf1) then
			retval = -1
		elseif (tempbuf0 .eq. tempbuf1) then
			retval = 0
		else
			retval = 1
		endif
	else
		if (rec(index0) .lt. rec(index1)) then
			retval = -1
		elseif (rec(index0) .eq. rec(index1)) then
			retval = 0
		else
			retval = 1
		endif
	endif
d
d	write(*,*) 'returning ',retval
d
	cmprec = retval
	return
	end


	character * MAXRECLEN function lower(input)
	character * MAXRECLEN input, result

c	lowercase the character string input, returning the result
$include sort.h
	character c

	result = input

	do 300 i = 1, MAXRECLEN

c		get the character
		c = input(i:i)
c		is the next character lowercase?
		if (islower(c)) then
			result(i:i) = tolower(c)
		endif

  300	continue
	lower = result
	return
	end

c
c	our fatal error message reporters 
c
	subroutine fatal(msg)

$include sort.h
	character * (*) msg

 	write(errunit,1010) msg
 1010	format('sort: ',a)
	stop
	end


	subroutine fatal2c(msg0,msg1)

$include sort.h
	character * (*) msg0, msg1

 	write(errunit,1010) msg0, msg1
 1010	format('sort: ',2a)
	stop
	end


	subroutine fatal2i(msg,arg)

$include sort.h
	character * (*) msg
	integer arg

 	write(errunit,1010) msg,arg
 1010	format('sort: ',a,i8)
	stop
	end

