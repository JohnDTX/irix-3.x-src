#! /bin/sh
#
# This command file's function is to set up model-specific
# configuration miscellany.
#

trap "" 2
MODEL=/etc/model
FSTAB=/etc/fstab

if test ! -s $MODEL; then
	# Prompt user for model number and get the reply
	while :
	do
		echo "\n/etc/model does not exist."
		echo "Please enter the model number of your machine: \c"
		read reply
		case "$reply" in

		2300|2300T )
			model=$reply
			root=md0a
			usr=
			break
			;;

		1400|2400|2400T )
			model=$reply
			root=md0a
			usr=md0c
			break
			;;

		1500|2500|2500T )
			model=$reply
			root=ip0a
			usr=ip0c
			break
			;;

		3010|3110 ) 
			model=$reply
			root=md0a
			usr=
			break
			;;

		3020|3115 )
			model=$reply
			root=md0a
			usr=md0c
			break
			;;

		3030|3120B|3130 )
			model=$reply
			root=si0a
			usr=si0f
			break
			;;

		* )
			echo "Invalid model number. Please try again."
			echo "Currently the only valid model numbers are:"
			echo "\t1400, 1500\n"
			echo "\t2300, 2400, 2500, 2300T, 2400T, 2500T\n"
			echo "\t3010, 3020, 3030\n"
			echo "\t3110, 3115, 3120B, 3130\n"
			continue
			;;
		esac
	done

	# If $FSTAB exists, leave it alone, otherwise build default list.
	if test ! -f $FSTAB; then
		(echo "$root /";
		 if test -n "$usr" ; then echo "$usr /usr"; fi) |
			setmnt -f $FSTAB
		chmod 644 $FSTAB
	fi

	# Now create $MODEL
	echo $model > $MODEL
	chmod 644 $MODEL

	#
	# Depending upon the model, we make generic device links.
	#
	case "$model" in

	1400|2300|2300T|2400|2400T|2500|2500T|3010|3020|3110|3115|3120B )
		rm -f /dev/floppy /dev/rfloppy
		ln /dev/mf0a /dev/floppy
		ln /dev/rmf0a /dev/rfloppy
		;;

	3030|3130 )
		rm -f /dev/rmt1 /dev/rmt2
		ln /dev/sq0 /dev/rmt1
		ln /dev/nrsq0 /dev/rmt2
		rm -f /dev/floppy /dev/rfloppy
		ln /dev/sf0a /dev/floppy
		ln /dev/rsf0a /dev/rfloppy
		;;
	esac
fi

#
# Depending on the model we decide if the sky floating point board
# should be downline loaded
#
model=`uname -t`
case "$model" in

1400 | 1500 | 2300 | 2400 | 2500 )
	/etc/fload
	;;
esac
