SHELL=/bin/sh

# stty erase '^H' kill '^U' intr '^C' echoe 
root=md0a

# df /dev/${disk}a | if test `uname -h` = GL2 && grep -s ' 1k '
if /isbell /dev/$root
then
	echo "Convert filesystem to EFS (y/n) ? \\c" ; read x
	case $x in
	y*|Y*)
		echo "Insert the EFS floppy, then press RETURN.\\c"
		read x
		tar xvf /dev/rmf0a
		./upgrade $root
		makeroot=":"		# filesystems already made
		;;
	*)
		set `sgilabel $root`	# make bell file systems
		makeroot="mkfs.bell /dev/r$root $1 10 $2"
		;;
	esac
else
	makeroot="mkfs /dev/r$root"	# make efs file systems
fi

$makeroot
mkdir /root
mount /dev/$root /root
tar cfb - 80 / | (cd /root; tar xvfpb - 80)
