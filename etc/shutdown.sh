#! /bin/sh

#	@(#)shutdown.sh	1.5

case $1 in
-r)	RFLAG=1; shift ;;
esac
sync
a=`pwd`
if [ $a != / -a $a != /etc ]
then
	echo "shutdown: you must be in the root directory (/) to use shutdown"
	exit 2
fi
grace=${1-60}
T=/tmp
trap "rm -f $T/$$; exit 0" 0 1 2 15

echo "\nSHUTDOWN PROGRAM\n"
date
echo "\n"
cd /
a="`who ^wc -l`"
if [ $a -gt 1 ]
then
	echo "Do you want to send your own message? (y or n):   \c"
	read b
	if [ "$b" = "y" ]
	then
		echo "Type your message followed by ctrl d....\n"
		su adm -c /etc/wall
	else
		su adm -c /etc/wall <<!
PLEASE LOG OFF NOW ! ! !
System maintenance about to begin.
All processes will be killed in $grace seconds.
!
	fi
	sleep $grace
fi
/etc/wall <<!
SYSTEM BEING BROUGHT DOWN NOW ! ! !
!
if [ ${grace} -ne 0 ]
then
	sleep 60
fi
echo "\nBusy out (push down) the appropriate"
echo "phone lines for this system.\n"
echo "Do you want to continue? (y or n):   \c"
read b
if [ "$b" = "y" ]
then
	/usr/lib/acct/shutacct
	echo "Process accounting stopped."
	if [ -x /etc/errstop ]	# system V has this, we don't yet
	then
		/etc/errstop
		echo "Error logging stopped."
	fi
	mount |
	    sed -n -e '/^\/ /d' -e 's/^.* on\(.*\) read.*/umount \1/p' |
	    sh -
	sync;sync;sync
	if test $RFLAG
	then
		/etc/reboot
	else
		echo "Wait for \`INIT: SINGLE USER MODE' before halting."
		/etc/telinit k
	fi
	mount |
	    sed -n -e '/^\/ /d' -e 's/^.* on\(.*\) read.*/umount \1/p' |
	    sh -
	sync;sync;sync;
else
	echo "For help, call your system administrator."
fi
