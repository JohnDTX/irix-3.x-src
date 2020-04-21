#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#		Copyright (c) 1984 AT&T
#		  All Rights Reserved
#     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#   The copyright notice above does not evidence any actual
#   or intended publication of such source code.
if test $# = 0;
then
cat >&2 <<!
Usage: "mm [options] files" where "options" are:
 -12	=> 12-pitch (effective only with DASI terminals)
 -e	=> neqn
 -t	=> tbl
 -c	=> col
 -E	=> "-e" option of NROFF (not effective with "-c")
 -Tterm	=> where term is the desired terminal type, e.g., -Thp.  (Cat
	   /usr/pub/terminals for a list of available terminal types.)
	   If no -T option is given then \$TERM is used if it has a value
	   otherwise the default is DASI 450.
 -	=> instead of "files" inside a pipeline.
Other options as required by NROFF and/or MM.
!
exit 1
fi
trap 'trap 0; if test "$m" = 0; then mesg -y; fi; exit $z' 0
trap 'trap 0; if test "$m" = 0; then mesg -y; fi; exit 1' 1 2 3 15
if test -t;	then mesg >/dev/null;	m=$?;	mesg -n; else m=1; fi
PATH=$PATH:/bin:/usr/bin
TERM=${TERM-450};  t=0;  v=0;  y=0;  g="";  h="-h";  w="";  u="-mm"
a= c= e= f=
while test -n "$1" -a ! -f "$1"
do case $1 in
	-T*)	TERM=`echo $1 | sed "s/-T//"` ;;
	-12)		y=1 ;;
	-e)		e=neqn ;;
	-t)		f=tbl ;;
	-c)		c=c ;;
	-E)		a="$a -e" ;;
	-y)		u=-mm ;;
	-)		break ;;
	*)		a="$a $1" ;;
   esac
   shift
done
if test -z "$1";	then echo "mm: no input file" >&2;		z=1;	exit;	fi
case "$TERM" in
	300|300s|450|37|300-12|300s-12|450-12|4000a|382)	;;
	X)		v=4;;
	4014)	g="|4014" ;;
	1620)		TERM=450 ;;
	1620-12)	TERM=450-12 ;;
	hp|2621|2640|2645)	v=2;	c=c;	a="-u1 $a";	g="|hp";	TERM=hp ;;
	735|745|43|40/4|40/2)	v=2;	c=c ;;
	2631|2631-c|2631-e)	v=3;	c=c ;;
	8510)	v=3;	c=c;	TERM=8510 ;;
	*)	TERM=lp;	v=2;	c=c ;;
esac
if test \( "$y" = 1 \) -a \( "$TERM" = 300 -o "$TERM" = 300s -o "$TERM" = 450 \)
	then TERM="$TERM"-12
fi
if test "$c" = c
then
	case "$TERM" in
		300|300s|450|4014)	g="|col -f|greek -T$TERM" ;;
		300-12|300s-12|450-12)	g="|col -f|greek -T$TERM";	w="-rW72" ;;
		37|4000a|382)	g="|col -f" ;;
		X)		g="|col -f"; v=5;;
		hp)		g="|col|hp" ;;
		2631-c|2631-e)	g="|col -p" ;;
		735|745)	g="|col -x" ;;
		43)		g="|col -x";	w="-rW75" ;;
		40/4|40/2)	g="|col -b" ;;
		8510)	g="|col -xp" ;;
		lp|2631)	g="|col" ;;
	esac
	h=""
	if test "$v" = 0;	then v=1;	fi
fi
d="$*"
if test "$d" = "-";	then shift;		d="";	fi
if test -n "$f"
	then	if test "$v" -gt 1
			then f="tbl -TX $*|"
			else f="tbl $*|"
		fi
		d=""
fi
if test -n "$e"
	then	if test -n "$f"
			then e="neqn /usr/pub/eqnchar -|"
			else	if test -z "$*"
					then e="neqn /usr/pub/eqnchar -|"
					else e="neqn /usr/pub/eqnchar $*|"
				fi;	d=""
		fi
fi
if test "$TERM" = 4014 -o "$TERM" = hp;		then v=1;	fi
case "$v" in
	1|5)	TERM=37 ;;
	2)	TERM=lp ;;
esac
eval "$f $e nroff $u $w -T$TERM $h $a $d $g";	z=0;	exit
