#! /bin/sh
#
# rootcopy - root partition copying program for the 2300.  With no
#	     argument and running with root as partition c, it dd's
#	     partition c to a and reboots if successful.  If given
#            the argument `ok', it will copy partition a to partition c.
#

PATH=:/etc:/bin
export PATH
case `/bin/uname -t` in
2300|2300T|3010) 
	if [ "$1" = ok ]
	then
		ok=ok
	fi
	rpar=a
	bupar=c
	set `devnm /`
	partition=$1
	drive=/dev/r`echo $partition | dd bs=1 count=3 2> /dev/null`
	set `sgilabel $partition`
	cylsz=$2
	case $partition in 
	    *$bupar) 
		#
		# dd whole thing over.  If successful, then reboot.
		#
		if dd if=${drive}${bupar} of=${drive}${rpar} bs=${cylsz}b
		then
			reboot -q
		else
			echo Error while restoring root.
			exit 1
		fi
		;;
	esac
	if [ "$ok" ]
	then
		dd if=${drive}${rpar} of=${drive}${bupar} bs=${cylsz}b
		exit
	fi
		
	;;
esac
exit 1
