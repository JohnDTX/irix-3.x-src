#! /bin/sh

if test ! -w /bin/su ; then echo 'kernel: not superuser' ; exit 1 ; fi

if test $# -lt 1 
then
	set `ls -i /vmunix`
	ls -i /kernels | awk $1' == $1 { print $2; }'
	exit
fi
if test $# -ne 1 ; then echo 'usage: kernel [type]' ; exit 1 ; fi
type=$1

cd /

set -- /kernels/*.$1

if test $1 = '/kernels/*.'$type
then
	echo "kernel: can't find $type in /kernels" ; exit 1
fi

case $# in
1)	if test ! -r $1 ; then echo kernel: $type not readable ; exit 1 ; fi
	rm -f vmunix defaultboot
	ln $1 vmunix
	ln $1 defaultboot
	;;
*)	echo "kernel: more than one $type in /kernels" ; exit 1
	;;
esac
