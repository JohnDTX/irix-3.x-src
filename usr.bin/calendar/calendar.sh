#! /bin/sh
# Must be run as root if processing everyone's calendar files
# $Header: /d2/3.7/src/usr.bin/calendar/RCS/calendar.sh,v 1.1 89/03/27 17:43:35 root Exp $

PATH=/bin:/usr/bin:
pattern=/tmp/cala$$
data=/tmp/calb$$
results=/tmp/calc$$

trap "rm -f $pattern $data $results"
trap exit 0 1 2 13 15
umask 022
/usr/lib/calprog >$pattern
umask 077

case $# in
0)
	trap "rm -f $pattern $data $results; exit" 0 1 2 13 15
	/usr/lib/calscript $pattern $data $results .
	cat $results;;

*)
	trap "rm -f $pattern $data $results; exit" 0 1 2 13 15

	/usr/lib/calnames | while read logname logdir 
	do
		if test -r $logdir/calendar
		then
			su $logname << security
/usr/lib/calscript $pattern $data $results $logdir
security
			if test -s $results
			then
				HOME=/dev/null
				LOGNAME=calendar
				export HOME LOGNAME
				Mail -s "`head -1 $results`" $logname < $results
			fi
		fi
	done
esac
