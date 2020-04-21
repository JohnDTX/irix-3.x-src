#! /bin/sh
#
# usage: mkboot [ -s standdir ] [ -r rootdev ] [-t host ] \
#		[ -u username ] [ filesystem ... ]
#
# The options are:
#
#	-s	specifies the directory containing the standalone
#		programs to be written to the tape.  The default
#		is /stand.
#	-r	specifies an alternate root device to be copied to tape.
#	-t	specifies a TCP host where the tape drive resides.
#	-u	specifies a userid other than 'guest' to be used when
#		doing remote TCP commands to the host specified in -t
#		option.  The userid executing mkboot must be able
#		to 'rsh host -l username' in order for mkboot -t to
#		work.
#
# Each optional "filesystem" will be written to the tape in cpio format
# after the root system in dd format.
#
# Thus the tape contains a file containing the standalone utilities in cpio
# format, a dd of the "g" partition (2400/2500, only), a "dd" image of
# the root file system, and then zero or more user file systems in cpio format.
#
echo "Make sure the system is idle: no other user activity."
echo

stand=/stand
gpart=
dev=
user=
host=
rsh=
acct=guest

while test $# -gt 0
do
	case $1 in
	-s)	shift; stand=$1 ;;
	-r)	shift; dev=$1 ;;
	-t)	shift; host=$1; rsh=rsh ;;
	-u)	shift; acct=$1 ;;
	-*)	echo "mkboot: unrecognized option $1." ; exit 1 ;;
	*)	user="$user $1" ;;
	esac
	shift
done

if test -z "$dev"
then
	set `devnm /`
	dev=$1
fi

if test ! -b /dev/$dev
then
	echo "mkboot: /dev/$dev: not a block device."
	exit 1
fi

for d in $stand $user
do
	if test ! -d $d
	then
		echo "mkboot: $d: not a directory." ; exit 1
	fi

done

set `sgilabel $dev`

size=`expr $2 '*' 512`
count=`expr $1 / $2`

fsck /dev/$dev

if test -z "$host"
then
	mt rewind
else
	$rsh $host -l $acct mt rewind
fi

fcount=1

echo mkboot: file $fcount = cpio of $stand 
fcount=`expr $fcount + 1`
cd $stand
if test -z "$host"
then
	echo * | cpio -oh2
else
	echo * | cpio -oh | $rsh $host -l $acct dd of=/dev/rmt2 obs=250k
fi

# g partition hack

if [ ! -s /etc/model ]
then
# Prompt user for model number and get the reply,
# Nothing is safe, be redundant.
	while :
	do
		echo "\n/etc/model does not exist."
		echo "Please enter the model number of your machine: \c"
		read reply
		case "$reply" in
		2400 )
			g=md0g
			set `sgilabel $g`
			gsize=`expr $2 '*' 512`
			gcount=`expr $1 / $2`
		
			echo mkboot: file $fcount = dd of $g
			fcount=`expr $fcount + 1`
			if test -z "$host"
			then
 				dd if=/dev/r$g of=/dev/rmt2 bs=$gsize count=$gcount
			else
 				dd if=/dev/r$g bs=$gsize count=$gcount | \
				$rsh $host -l $acct dd of=/dev/rmt2 obs=250k
			fi
			break
			;;
		2500 )
			g=ip0g
			set `sgilabel $g`
			gsize=`expr $2 '*' 512`
			gcount=`expr $1 / $2`
		
			echo mkboot: file $count = dd of $g
			fcount=`expr $fcount + 1`
			if test -z "$host"
			then
 				dd if=/dev/r$g of=/dev/rmt2 bs=$gsize count=$gcount
			else
 				dd if=/dev/r$g bs=$gsize count=$gcount | \
				$rsh $host -l $acct dd of=/dev/rmt2 obs=250k
			fi
			break
			;;
		1400|1500|2400T|2500T|3020|3030|3115|3120B|3130 )
			break
			;;
		* )
			echo "Invalid model number. Please try again."
			echo "Currently the only valid model numbers are:"
			echo "\t1400, 1500\n"
			echo "\t2400, 2500, 2400T, 2500T\n"
			echo "\t3020, 3030\n"
			echo "\t3115, 3120B, 3130\n"
			continue
			;;
		esac
	done
else	
	set `cat /etc/model`
	if  test $1 = 2500 -o $1 = 2400
	then
	    case "$1" in
	      2400 )
		g=md0g
		break
		;;
	      2500 )
		g=ip0g
		break
		;;
	    esac
		set `sgilabel $g`
		gsize=`expr $2 '*' 512`
		gcount=`expr $1 / $2`

		echo mkboot: file $fcount = dd of $g
		fcount=`expr $fcount + 1`
		if test -z "$host"
		then
	 		dd if=/dev/r$g of=/dev/rmt2 bs=$gsize count=$gcount
		else
	 		dd if=/dev/r$g bs=$gsize count=$gcount | \
			$rsh $host -l $acct dd of=/dev/rmt2 obs=250k
		fi
	fi
fi

# end hack

echo mkboot: file $fcount = dd of $dev
fcount=`expr $fcount + 1`
if test -z "$host"
then
	dd if=/dev/r$dev of=/dev/rmt2 bs=$size count=$count
else
	dd if=/dev/r$dev bs=$size count=$count | \
	$rsh $host -l $acct dd of=/dev/rmt2 obs=250k
fi

for d in $user
do (
	echo mkboot: file $fcount = cpio of $d 
	fcount=`expr $fcount + 1`
	cd $d
	if test -z "$host"
	then
		cpio -oah2 .
	else
		cpio -oah . | $rsh $host -l $acct dd of=/dev/rmt2 obs=250k
	fi
) done

if test -z "$host"
then
	mt rewind
else
	$rsh $host -l $acct mt rewind
fi

echo "mkboot complete."

