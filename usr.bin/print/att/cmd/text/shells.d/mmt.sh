#! /bin/sh
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

u=-mm;  if echo `basename $0` | grep mv >/dev/null;  then u=-mv;  fi
if test $# = 0;
then 
cat >&2 <<!
Usage: "mmt [options] files" (or "mvt [options] files") where "options" are:
 -a	=> output to terminal
 -e	=> eqn
 -p	=> pic
 -g	=> grap
 -t	=> tbl
 -Taps	=> APS-5
 -Tptty	=> format for troff device ptty
 -D4014	=> Tektronix 4014
 -Di10	=> send to Imagen Imprint-10
 -z	=> use no postprocessor for troff output
 -	=> instead of "files" inside a pipeline.
Other options as required by TROFF and the macros.
!
exit 1
fi
ptty=  x=  Teqn=  ec=/usr/pub/eqnchar
PATH=$PATH:/bin:/usr/bin  O=  o="|daps"  y=
#	'If phototypesetter connected to unix, use o="|daps"'
#	'If sending to MHCC use o="|apsend"'
a= e= f= p= z= g=
while test -n "$1" -a ! -f "$1"
do case $1 in
	-a)	O="-a"  o=  x=  ;;
	-Taps)	ptty=aps  Teqn=-Taps  ec=/usr/pub/apseqnchar  ;;
	-Ti10)	ptty=i10  Teqn=-Ti10  o="|di10"  x=  ;;
	-Di10)	o="|di10"  x=  ;;
	-D4014|-Dtek|-T4014)	o="|tc"  x=  y="-rX1" ;;
	-T*)	ptty=`echo $1 | sed "s/-T//"`
		o=  x=  ;;
	-e)	e=eqn ;;
	-t)	f=tbl ;;
	-p)	p=pic ;;
	-g)	g=grap  p=pic ;;
	-y)	;;
	-)	break ;;
	-z)	z=z  ;;
	*)	a="$a $1" ;;
   esac
   shift
done
if test -z "$1";  then echo "$0: no input file" >&2;  exit 1;  fi
if test -n "$ptty"
then troff="troff -T$ptty"
else troff=troff
fi
if test -n "$z";  then o=  x=;  fi
if test "|apsend" = "$o";  then x="$x c=$1";  fi
d="$*"
if test -n "$g";  then g="grap $d|";  d="-";  fi
if test -n "$p";  then p="pic $Teqn $d|";  d="-";  fi
if test -n "$f";  then f="tbl $d|";  d="-";  fi
if test -n "$e";  then e="eqn $Teqn $ec $d|";  d="-";  fi
if test "$u" != "-mv";  then y=;  fi
eval "$g $p $f $e $troff $y $O $u $a $d $o $x"; exit 0
