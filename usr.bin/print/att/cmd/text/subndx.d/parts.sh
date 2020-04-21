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
PATH='/bin:/usr/bin'
L=/usr/lib/dwb
wflag=
LISTmflg="-m m"
mflag=-mm
iflag=-li
LISTiflg="-i g"

case "$WWBFMT" in
	""|m) LISTmflg="-m m";mflag=-mm;;
	   s) LISTmflg="-m s";mflag=-ms;;
	   n) LISTmflg="-m n";mflag=n;;
esac

#	GET OPTIONS

while test -n "$1"
do
	case $1 in
	-mm) mflag=-mm;shift;continue;;
	-ms) mflag=-ms;LISTmflg="-m s";shift;continue;;
	-ml|-li) iflag=-li;shift;continue;;
	+li) iflag=+li; LISTiflg="-i n"; shift;continue;;
	-i) 	shift;

#		GET i ARGUMENTS
 
		case $1 in
          	n) iflag=+li;LISTiflg="-i n";;
		g) iflag=-li;;
		*) echo argument for -i must be n or g;exit;
		esac
		shift;continue;;
	-m) 	shift;

#		GET m ARGUMENTS
 
		case $1 in
          	m) mflag=-mm;;
		n) mflag=n;LISTmflg="-m n";;
		s) mflag=-ms;LISTmflg="-m s";;
		*) echo argument for -m must be m,n  or s;exit;
		esac
		shift;continue;;
	-w | -o) wflag=-w;shift;continue;;
	-h) 	reropts="$reropts "-h
		shift;continue;;
	-B|-I|-P) reropts="$reropts $1"; shift;
		  reropts="$reropts $1";
		  if	[ "$1" = "" ]
		  then  echo reroff option argument missing. >&2;exit;
	          fi
	          if	test $1 -gt 0 -a $1 -lt 99  
		  then 	shift;continue
	   	  else 	if    [ "$1" = "0" ]
			then  shift; continue;
			else  echo unknown reroff option argument $1 >&2;exit;
		        fi
	          fi;;
	-V |-ver) echo $0 version 3.1: 3.0, 3.0, 3.0, 3.0: : ;exit;;
	-O |-flags|-0)

	echo "\nparts [-wOV] [-i g|n] [-m m|n|s] [reroff-options] [--] [-|file ...]\n\n"\
	"   -w ........Print one word per line with the word's part of speech.\n"\
	"   -O ........Print command option synopsis, this list, then exit.\n"\
	"   -V ........Print the version number of this program, then exit.\n"\
	"   -i g ......Ignore lists in the analysis.\n"\
	"   -i n ......Include lists in the analysis.\n"\
	"   -m m ......File contains mm macros.\n"\
	"   -m n ......File contains no macros.\n"\
	"   -m s ......File contains ms macros.\n\n"\
	" reroff options:  (Use only with -m n option.)\n\n"\
	"   -h ........Don't remove hyphens.\n"\
	"   -B num ....The input text has between-line spacing equal to \"num.\"\n"\
	"   -I num ....The standard indent (margin) is \"num\" spaces.\n"\
	"   -P num ....The standard indent for paragraphs is \"num\" spaces.";
	exit;;
	--) shift;break;;
	-) 
		shift; break;;
	    
	-*) echo unknown $0 option $1 >&2;exit;;
	*)  break;;

	esac	
done

for i in $*
do
	if	test ! -r $i 
	then	echo "\n$0 can't access the text file $i;"\
		"\ntry specifying a more complete pathname.">&2; 
		exit; 
	fi
done

#  Give error message and exit if reroff opts used without -m n.


if [ "$reropts" != "" -a "$mflag" != "n" ]
then
	echo $0 $reropts can only be used with the "-m n" option ;exit;
fi

#	Initial echo line

echo $0 $LISTmflg  $LISTiflg $wflag $reropts $*

if      [ "$iflag" = "-li" ]
then	iflag=-ml
fi
if 	test $mflag = n
then    if	test -n "$wflag" 
	then	cat -- $* | col -x | $L/reroff/reroff -w $reropts |
		$L/deroff -mm $iflag |
		$L/style1 |
		$L/style2 |
		$L/style3 -P
	else	cat -- $* | col -x | $L/reroff/reroff -w $reropts |
		$L/deroff -mm $iflag  |
		$L/style1 |
		$L/style2 |
		$L/style3 -P |
		$L/partsline
	fi
else    if	test -n "$wflag" 
	then	$L/deroff $mflag $iflag -- $* |
		$L/style1 |
		$L/style2 |
		$L/style3 -P
	else	$L/deroff $mflag $iflag -- $* |
		$L/style1 |
		$L/style2 |
		$L/style3 -P |
		$L/partsline
	fi
fi
