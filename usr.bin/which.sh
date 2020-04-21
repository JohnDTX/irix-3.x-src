#! /bin/csh
#
# Derived from which 4.2bsd.  It differs from #	in 2 respects:
#	    1.	"-a" tells it to keep looking, even after it has found a match
#	    2.	It searches among your current aliases.
#
set fstop = ""
set noglob
foreach arg ( $argv )
    switch ("$arg")
	case "-a" :
	    set fstop = "-a"
	    continue
    endsw
    set alius = `alias $arg`
    switch ( $#alius )
	case 0 :
	    breaksw
        default :
	    echo ${arg} aliased to \"$alius\"
	    set arg = $alius[1]
	    breaksw
    endsw
    unset found
    if ( "$arg:h" != "$arg:t" ) then
	if ( -e "$arg" ) then
	    echo $arg
	else
	    echo $arg not found
	endif
	continue
    else
	foreach i ( $path )
	    if ( -x $i/$arg && ! -d $i/$arg ) then
		echo $i/$arg
		set found
		if ("$fstop" =~ "") break
	    endif
	end
    endif
    if ( ! $?found ) then
	echo $arg not in $path
    endif
end
