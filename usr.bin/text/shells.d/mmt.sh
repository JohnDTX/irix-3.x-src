#! /bin/sh
#	MMTSID (@(#)mmt.sh	1.12)
u=-cm;	if test `basename $0` = "mvt";	then u=-mv;	fi
if test $# = 0;	then help text1 >&2;	exit 1;	fi
O="-g";	o="|gcat -ph";	y=""
#			'If C/A/T connected to PDP-11, use O=""; o=""'
#			'If GCOS use O="-g"; o="|gcat -ph"'
while test -n "$1" -a ! -r "$1"
do case $1 in
	-a)		O="-a";		o="" ;;
	-Tst|-Ts)	O="-g";		o="|gcat -st" ;;
	-T4014|-Ttek)	O="-t";		o="|tc";	y="-rX1" ;;
	-Tvp)	O="-t";	o="|vpr -t";	y="-rX1" ;;
	-e)		e=eqn ;;
	-t)		f=tbl ;;
	-y)		if test "$u" = "-cm";	then u=-mm;	fi ;;
	-)		break ;;
	*)		a="$a $1" ;;
   esac
   shift
done
if test -z "$1";	then echo "$0: no input file" >&2;	exit 1;	fi
if test "$O" = "-g";	then x="-f$1";	fi
d="$*"
if test "$d" = "-";	then shift;	x="";	fi
if test -n "$f";	then f="tbl $*|";	d="-";	fi
if test -n "$e"
	then	if test -n "$f"
			then e="eqn /usr/pub/eqnchar -|"
			else	if test -z "$*"
					then e="eqn /usr/pub/eqnchar -|"
					else e="eqn /usr/pub/eqnchar $*|"
				fi;	d="-"
		fi
fi
if test "$u" = "-mm" -o "$u" = "-cm" ;	then y="";	fi
eval $f $e troff $y $O $u $a $d $o $x; exit 0
