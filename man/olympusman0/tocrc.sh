#! /bin/sh
# tocrc - create permuted index and table of contents for manual
#
# Must be run from this subdirectory (but is independent of its name).
# Includes gl2 demo man pages.
#
# $Revision: 1.1 $
# $Date: 89/03/27 16:39:51 $
#
tmp=/tmp/toc$$
tmp2=/tmp/cattoc$$
tmp3=/tmp/notcap$$

troff=xroff
trap "rm -f $tmp $tmp2 $tmp3; exit" 2 3 15
if [ $# -eq 0 ]
then
	echo "Usage: tocrc section | \"all\" | \"tr\""
	exit 1
fi
if [ $# -eq 2 ]
then
	: create toc input file for one section only

	dirs="../[au]_man/man$1"
	if [ $1 = 1 ]
	then
    		dirs="$dirs ../u_man/man1/gl2"
	fi
	for dir in $dirs
	do
	    	( cd $dir ; csh -cf "/usr/lib/getNAME -t *.$1 *.$1[a-z]" )
	done | sort | sed > tocx$1 \
			-e 's/\\s[-+][0-9]//g' \
			-e 's/\\s0//g' \
			-e 's/\\f[1234RIBP]//g' \
			-e 's/  *\\-  */: /' \
			-e 's/ *$/./' \
			-e 's/.TH.*	//'
			# remove font and size changes as they alphabetize
			# wrong.

else 
	case $1 in
	all )
		:   tocx files for all sections and everything else

		for x in 1 2 3 4 5 6 7 8
		do
			./$0 $x $x
		done
		./$0 t
		;;
	t )
		:   permuted index and toc files

		if [ ! -f tocx1 ]
		then
			echo "tocx* files missing; must run 'tocrc all' first"
			exit
		fi
		sed \
			-e 's/(1c)/(1C)/' \
			-e 's/(1d)/(1D)/' \
			-e 's/(1g)/(1G)/' \
			-e 's/(1m)/(1M)/' \
			-e 's/(1w)/(1W)/' \
			-e 's/(3c)/(3C)/' \
			-e 's/(3m)/(3M)/' \
			-e 's/(3n)/(3N)/' \
			-e 's/(3s)/(3S)/' \
			-e 's/(3x)/(3X)/' \
			-e 's/(7p)/(7P)/' \
			-e '/"\."/d' \
			tocx* cshcmd shcmd \
		> $tmp2

		: check for uncapitalized subsections
		awk '{print $1}' $tmp2 | sed -n 's/.*(\(.[a-z]\))$/\1/p' | \
			sort -u > $tmp3
		if [ -s $tmp3 ]
		then
			echo tocrc needs changing to capitalize these subsections:
			cat $tmp3
		fi
		rm -f $tmp3

		/usr/local/bin/ptx -r -f -x MR 10 5.4 -T '\(br' \
			-b break -i ignore $tmp2  ptxx

		for f in tocx*
		do
			< $f grep '^intro' >$tmp
				sed \
				-e '2,${' \
				-e '/^intro/d' \
				-e '}' \
				-e 's/ .*://' \
				-e 's/.$//' \
				-e 's/([^)]*) /" "/' \
				-e 's/.*/.xx "&"/' \
				-e '/""/d' \
				$tmp $f \
			> `echo $f | sed 's/tocx/toc/'`
		done
		;;
	tr )
		$troff ptx.in
		$troff -ra0 toc.in
		$troff -ra1 toc.in
		;;
	* )
		./$0 $1 $1
		;;
	esac
fi
rm -f $tmp $tmp2
exit
