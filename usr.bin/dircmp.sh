#! /bin/sh
#	sccs: @(#)dircmp.sh	1.3
# @(#)$Header: /d2/3.7/src/usr.bin/RCS/dircmp.sh,v 1.1 89/03/27 17:39:59 root Exp $
PATH=/bin:/usr/bin
usage='usage: dircmp [-s] [-d] [-t] [-l<pagelen>] directory directory'
pagel=""
trap "rm -f /usr/tmp/dc$$*;exit" 1 2 3 15
while [ $# -ge 2 ]
do
	case $1 in
	-d)	Dflag="yes"
		shift
		;;
	-s)	Sflag="yes"
		shift
		;;
	-t)	Tflag="yes"
		shift
		;;
	-l*)	pagel="$1"
		shift
		;;
	-*)	echo dircmp: unknown option
		echo "$usage"
		exit 3
		;;
	 *)	D1=$1 D2=$2
		break
		;;
	esac
done
if [ $# -lt 2 ]
then echo dircmp: missing parameters
     echo "$usage"
     exit 1
elif [ ! -d "$D1" ]
then echo dircmp: $D1 not a directory !
     echo "$usage"
     exit 2
elif [ ! -d "$D2" ]
then echo dircmp: $D2 not a directory !
     echo "$usage"
     exit 2
fi
( cd $D1 ; find . -print | sort ) > /usr/tmp/dc$$a
( cd $D2 ; find . -print | sort ) > /usr/tmp/dc$$b
comm /usr/tmp/dc$$a /usr/tmp/dc$$b | sed -n \
	-e "/^		/w /usr/tmp/dc$$c" \
	-e "/^	[^	]/w /usr/tmp/dc$$d" \
	-e "/^[^	]/w /usr/tmp/dc$$e"
rm -f /usr/tmp/dc$$a /usr/tmp/dc$$b
pr $pagel -h "$D1 only and $D2 only" -m /usr/tmp/dc$$e /usr/tmp/dc$$d
rm -f /usr/tmp/dc$$e /usr/tmp/dc$$d
sed -e s/..// < /usr/tmp/dc$$c > /usr/tmp/dc$$f
rm -f /usr/tmp/dc$$c
umask 077
touch /usr/tmp/dc$$g
while read a
do
	if [ -d $D1/"$a" ]
	then if [ "$Sflag" != "yes" ]
	     then echo "directory	$a"
	     fi
	elif [ -f $D1/"$a" ]
	then cmp -s $D1/"$a" $D2/"$a"
	     x=$?
	     if [ $x = 0 ]
	     then if [ "$Sflag" != "yes" ]
		  then echo "same     	$a"
		  fi
	     elif [ $x = 1 ]
	     then echo "different	$a"
		  if [ "$Dflag" = "yes" ]
		  then if [ "$Tflag" != "yes" ] || \
			  file $D1/$a | awk '$NF == "text" { exit 0 } { exit 1 }'
		       then diff $D1/"$a" $D2/"$a" | pr $pagel -r -h "diff of $a in $D1 and $D2" >> /usr/tmp/dc$$g
		       fi
		  fi
	     else echo "unreadable	$a"
	     fi
	elif [ "$Sflag" != "yes" ]
	then echo "special  	$a"
	fi
done < /usr/tmp/dc$$f | pr $pagel -r -h "Comparison of $D1 and $D2"
if [ "$Dflag" = "yes" ]
then cat /usr/tmp/dc$$g
fi
rm -f /usr/tmp/dc$$*
