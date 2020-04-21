#!/bin/csh
#
# usage: test2 [-thf]
#
source defs.csh
set Myname=$0
set timeit="no"

if ($#argv > 0) then
	switch ($argv[1])

	case -t:
		set timeit="yes"
		breaksw

	case -f:
	case -ft:
	case -tf:
		breaksw

	default:
		echo "usage: $Myname [-thf]"
		exit(1);
	
	endsw
endif

if ( ! -d $TESTDIR ) then
	setenv NFSTESTDIR $TESTDIR
	test1 -s
endif

echo "${Myname}: File and directory removal test"

if ($timeit == "yes") then
	/bin/time rm -r $TESTDIR
else
	rm -r $TESTDIR
endif

if ( -d $TESTDIR ) then
	echo ${Myname}: $TESTDIR not removed, test failed
	exit(1);
endif

echo "        ${Myname} ok."
exit(0)
