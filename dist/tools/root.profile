exec < /dev/tty > /dev/tty
SHELL=/bin/sh

stty erase '^H' kill '^U' intr '^C' echoe 

if test -f /yes
then	ask=: getanswer=: ans=y
	echo
	echo "Autoloading..."
	echo
else	ask=echo getanswer="read"
fi

set `devnm /` ; dev=$1
case "$dev" in
md0b)
	disk=md0
	root=md0a
	usr=md0c
	;;
ip0b)
	disk=ip0
	root=ip0a
	usr=ip0c
	;;
si0b)
	root=si0a
	usr=si0f
	rm -f /dev/rmt1 /dev/rmt2
	ln /dev/sq0 /dev/rmt1
	ln /dev/nrsq0 /dev/rmt2
	;;
esac

if ( < /dev/rmt2 ) 2>/dev/null
then
	whence=
	reader="< /dev/rmt2"
	mt=mt
else
	echo "Can't access tape drive; assuming network update."
	while true
	do
		echo
		echo "Enter hostname of machine with tape drive : \c"
		read machine
		whence="xx $machine "
		if $whence true ; then break ; fi
		echo
		echo "Cannot access '$machine'."
	done
	reader="xx $machine dd if=/dev/rmt2 bs=250k count=\$count |"
	case `xx $machine ls /bin/smt` in
	/bin/smt) mt=smt ;;
	*) mt=mt ;;
	esac
fi

makeroot="mkfs /dev/r$root"		# make efs file systems by default
makeusr="mkfs /dev/r$usr"

if test ! -x /isefs || { /isefs /dev/$root && /isefs /dev/$usr ; }
then	:
else
	$ask "Convert filesystems to EFS (y/n) ? \\c" ; $getanswer ans
	case $ans in
	y*|Y*)
		/upgrade $root $usr
		;;
	*)
		set `sgilabel $root`	# make bell file systems
		makeroot="mkfs.bell /dev/rmd0a $1 10 $2"
		set `sgilabel $usr`
		makeusr="mkfs.bell /dev/rmd0c $1 10 $2"
		;;
	esac
fi

if test ! -d /root ; then mkdir /root ; fi

$ask "Load new system (y/n) ? \\c" ; $getanswer ans
case $ans in
y*|Y*)	
	$ask "Include /usr with new system ? \\c" ; $getanswer ans
	$whence $mt -t /dev/rmt2 rewind
	count=`grep . /distsize`
	eval $reader cpio -ihmud dist/toc
	if test ! -f dist/toc
	then
		echo "Tape error: Can't read dist/toc."
	else
		$whence $mt -t /dev/rmt2 rewind 
		$whence $mt -t /dev/rmt2 fsf 2 
		case $ans in
		y*|Y*) set -- `grep . dist/toc` ;;
		*) set -- `grep -v usr dist/toc` ;;
		esac
		while test $# -gt 1
		do
			name=$1 size=$2 ; shift ; shift
			case $name in
			root)	$makeroot
				labelit /dev/$root root sgi
				mount /dev/$root /root
				mkdir /root/usr
				cd /root
				;;
			usr)	
				$makeusr
				labelit /dev/$usr usr sgi
				mount /dev/$usr /root/usr
				;;
			esac
			case $name in
			root*|usr*)
				count=`expr $size / 250`
				eval $reader cpio -ihumd
			esac
		done
		cd /
		umount /dev/$usr 2> /dev/null
		umount /dev/$root 2> /dev/null
		$whence $mt -t /dev/rmt2 rewind
	fi
esac

$ask "Reboot system ? \c"; $getanswer ans
case $ans in
y*|Y*) /etc/reboot
esac
