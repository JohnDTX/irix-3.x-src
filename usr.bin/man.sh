#! /bin/sh
#	MANSID (@(#)man.sh	1.39)
z=1
if test $# = 0;		then help text2 >&2;	exit 1;	fi
umask 0
ec=/usr/pub/eqnchar
PATH=/bin:/usr/bin:/usr/lbin: y=0; tbl="tbl"; u="-man";
troff=0;	cf=0;	sec=\?;	mdir=0
cmd= fil= opt= i= all= eopt= j=
fast=y
pause=""
		# in case they are defined in env
a= c= cat= catlist= ck= d= fil1= fil2= found= g= h= in= o= pack= prep= q=
r= rfill= v=
for i
do case $i in
	[1-8])	sec=$i ;;
	-s)	if test "$cmd" = "";	then cmd=t;	fi
		troff=1;	opt="$opt -rs1";	eopt=-s9;fast=n ;;
	-t)	troff=1;	cmd=t;fast=n ;;
	-Tst)	troff=1;	cmd=s;fast=n ;;
	-T4014|-Ttek)	troff=1;	cmd=k;fast=n ;;
	-Tvp)	troff=1;	cmd=v;fast=n ;;
	-c)	c=c;fast=n ;;
	-12)	y=1;fast=n;;
	-d)	mdir=1;fast=n ;;
	-w)	cmd=w;fast=n ;;
	-y)	u="-man" ;;
	-T*)	TERM=`echo $i | sed "s/-T//"`;export TERM;fast=n ;;
	-*)	opt="$opt $i";fast=n ;;
	*)	if test "$mdir" = 0
		then
			cd /usr/man
			j=$i
			if [ $fast = "y" -a -t 1 ]
			then
				found=no
				case $i in
				    *[*?[]*) # user knows what he's doing
					;;
				    *)	# if $i > 9 chars, truncate
					#    to 9 chars and append *
					# translate to lowercase as all
					# man titles are case insensitive
					j=\
`echo $i | sed 's/^\(.........\)..*$/\1*/;p;d' | tr '[A-Z]' '[a-z]'`
					;;
				esac
				q=`echo local/cat$sec/$j.* [ua]_man/cat$sec/$j.*`
				for r in $q
				do
					case "$r" in
					  *\*) ;;
					  *.z)
						if [ ! -s $r ]
						then rm -f $r;continue;fi
						found=yes
						if [ "$pause" != "" ]
						then
							echo "Hit _R_E_T_U_R_N to continue:\c" | more
							read junk
						fi
						pause=please
						pcat $r | ul | page -f -s
						continue;;
					  *)
						if [ ! -s $r ]
						then rm -f $r;continue;fi
						if [ -r $r.z ]
						then
							rm -f $r
							continue
						fi
						found=yes
						if [ "$pause" != "" ]
						then
							echo "Hit _R_E_T_U_R_N to continue:\c" | more
							read junk
						fi
						pause=please
						ul $r | page -f -s
						pack $r 2>&1 > /dev/null
						continue;;
					esac
				done
				if [ $found = yes ]
				then
					continue
				fi
			fi
			fil1=`echo local/man$sec/$j.*`
			fil2=`echo [ua]_man/man$sec/$j.*`
			case $fil1 in
				*\*)	case $fil2 in
						*\*)	echo man: "$j" not found >&2 ;;
						*)	all="$all $fil2" ;;
					esac ;;
				*)	case $fil2 in
						*\*)	all="$all $fil1" ;;
						*)	all="$all $fil1 $fil2" ;;
					esac ;;
			esac
		else
			if test ! -r "$i"
			then
				echo man: "$i" not found >&2
			else
				all="$all $i"
			fi
		fi
   esac
done
if test "$cmd" = "w";	then echo $all;	z=0;	exit;	fi
if test $troff -eq 0
then
	v=0;	h="-h";	g=""
	if test "$TERM" = "";	then TERM=lp; fi
	case "$TERM" in
		300|300s|450|37|300-12|300s-12|450-12|4000a|382|X)	;;
		4014|tek)	g="|4014" ;;
		1620)	TERM=450 ;;
		1620-12)	TERM=450-12 ;;
		hp|2621|2640|2645)	v=1;	c=c;	a="-u1 $a";	g="|hp -m";	TERM=hp ;;
		735|745|40/4|40/2)	v=1;	c=c ;;
		43)	v=1;	c=c;	opt="$opt -rT1" ;;
		2631|2631-c|2631-e)	v=3;	c=c ;;
		*)	TERM=lp;	v=1;	c=c ;;
	esac
	if test \( "$y" = 1 \) -a \( "$TERM" = 300 -o "$TERM" = 300s -o "$TERM" = 450 \)
		then TERM="$TERM"-12
	fi
	if test "$c" = c
	then
		case "$TERM" in
			300|300s|450|300-12|300s-12|450-12|4014|tek)	g="|col -f|greek -T$TERM" ;;
			37|4000a|382|X)	g="|col -f" ;;
			hp)	g="|col|hp -m" ;;
			2631-c|2631-e)	g="|col -p" ;;
			735|745|43)	g="|col -x" ;;
			40/4|40/2)	g="|col -b" ;;
			lp|2631)	g="|col" ;;
		esac
		h=""
		if test "$v" = 0;	then v=2;	fi
	fi
	if test "$v" = 1 -o "$v" = 3;	then tbl="tbl -TX";	fi
	if test "$TERM" = 4014 -o "$TERM" = hp;	then v=2;	fi
	if test "$v" = 1;	then TERM=lp;	fi
	if test "$v" = 2;	then TERM=37;	fi
fi
if [ "$all" != "" ]
then
	echo Reformatting $all 1>&2
fi
for fil in $all
do
	ln=`head -1 < $fil`
	ck=`echo $ln | sed "s/ .*//"`
	if test "$ck" = ".so"
	then
		rfil=`echo $ln | sed "s/.so //"`
	else
		rfil=$fil
	fi
	if test "$cmd" = s -o "$cmd" = t;	then cf=1;	fi
	prep="cat $fil"
	ln=`head -1 < $rfil`
	ck=`echo $ln | sed "s/ .*//"`
	if test "$ck" = "'\\\""
	then
		case `echo $ln | sed "s/....//"` in
		c)	if test "$cf" = 1;	then prep="cw $fil";	fi ;;
		e)	if test $troff -eq 1
			then
				prep="eqn $eopt $ec $fil"
			else
				prep="neqn $ec $fil"
			fi ;;
		t)	prep="$tbl $fil" ;;
		ce | ec)
			if test "$cf" = 1
			then
				prep="cw $fil | eqn $eopt $ec -"
			elif test $troff -eq 1
			then
				prep="eqn $eopt $ec $fil"
			else
				prep="neqn $ec $fil"
			fi ;;
		ct | tc)
			if test "$cf" = 1
			then
				prep="cw $fil | $tbl"
			else
				prep="$tbl $fil"
			fi ;;
		et | te)
			if test $troff -eq 1
			then
				prep="$tbl $fil | eqn $eopt $ec -"
			else
				prep="$tbl $fil | neqn $ec -"
			fi ;;
		cet | cte | ect | etc | tce | tec)
			if test "$cf" = 1
			then
				prep="cw $fil | $tbl | eqn $eopt $ec -"
			elif test $troff -eq 1
			then
				prep="$tbl $fil | eqn $eopt $ec -"
			else
				prep="$tbl $fil | neqn $ec -"
			fi ;;
		esac
	fi
	d=`/usr/lib/manprog $rfil`
	O="-g";	o="|gcat -ph -f$fil"
			# If GCOS, set O="-g"; o="|gcat -ph -f$fil"
			# If on-line typesetter, set O=""; o=""
	case $cmd in
	  "")
		if [ $fast = n -o ! -t 1 ]
		then
			eval "$prep" | eval "nroff -T$TERM $d$opt $h $a $u $g"
		else
			catfil=`echo $fil | sed 's|/man|/cat|'`
			#
			# keep track of all catfiles generated.
			#
			catlist="$catlist $catfil $catfil.z"
			cat=`echo "| bcat -s | tee " $catfil "2>/dev/null"`
			umask 022
			if [ "$pause" != "" ]
			then
				echo "Hit _R_E_T_U_R_N to continue:" | more
				read junk
			fi
			pause=please
			#
			# set signal trap to remove the cat files if the
			# user kills the nroff.  note that we can't disable
			# this trap, we just keep adding names to the list.
			# the reason is that the man script expects all cats
			# or all nroffs, no mixing allowed.
			#
			trap 'rm -f $catlist; exit 1' 1 2 3 15
			eval "$prep" | eval "nroff -T$TERM $d$opt $h $a $u $g $cat" | ul | page -s -f
			pack $catfil 2>&1 > /dev/null
		fi;;
	  t)	eval "$prep | psroff $d$opt $u " ;;
	  s)	eval "$prep | troff -g $d$opt $u | gcat -st -f$fil" ;;
	  k)	eval "$prep | troff $d -t$opt $u | tc" ;;
	  v)	eval "$prep | troff $d -t$opt $u | vpr -t" ;;
	esac
done
exit
