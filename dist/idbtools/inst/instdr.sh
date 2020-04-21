#! /bin/sh

export HOME ; HOME=/
. /etc/TIMEZONE

trap '' 2

stty line 1 erase '^H' kill '^U' intr '^C' echoe 
umask 022

m=`uname -m`

if test ! -f /unix -a -f /unix.$m
then
	if mv /unix.$m /unix
	then
		rm /unix.*
	fi
fi

echo
echo "Type 'quit' to exit the software installation tool."
echo

if /usr/sbin/inst
then
	echo "System configuration: please wait."
	if test `/bin/ls /root/dev|wc -l` -lt 10
	then
		( cd /root/dev ; ./MAKEDEV )
	fi
	if test -x /root/etc/init.d/autoconfig
	then
		rm -rf /root/tmp ; mkdir /root/tmp ; chmod +w /root/tmp
		if echo y | /root/etc/chroot /root /etc/init.d/autoconfig > /dev/null
		then
			if test -x /root/unix.install
			then
				mv /root/unix.install /root/unix
			fi
		fi
	fi
	rm -rf /tmp ; ln -s /root/tmp /tmp
	if test -f /root/stand/sash
	then
		/etc/dvhtool -v del sash -v add /root/stand/sash sash
	fi
	if test -f /root/stand/fx
	then
		/etc/dvhtool -v del fx -v add /root/stand/fx fx
	fi
	rm -rf /tmp ; mkdir /tmp
	cd /
	umount /root/usr
	umount /root
	echo
	echo "Reboot ? \c" ; x= ; read x
	case $x in
	y*|Y*) /etc/telinit 0 ;;
	esac
fi
trap 2 ; /bin/csh -i
