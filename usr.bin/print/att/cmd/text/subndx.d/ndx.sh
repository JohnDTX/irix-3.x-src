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
trap  'rm -f /tmp/slist.$$ /tmp/cpy*$$ /tmp/sj* /tmp/sb* /tmp/ndx*$$ /tmp/$$.*; exit' 1 2 3 15
UL=/usr/lib/dwb
#
#  test for the correct number of arguments and
#  that the subject file exists and is readable
#
if test $# -lt 2 -o $# -gt 2;
then
	echo 'Usage: ndx subject_file "formatter command line"' >&2
	exit 1
fi
if test "$1" = "-"; then
	echo 'Usage: ndx subject_file "formatter command line"' >&2
	exit 1
elif
	test ! -r "$1";
	then
		echo "ndx: ndx aborted, cannot read the file: $1" >&2
		echo "Please check to see that $1 exists and is readable" >&2
		exit 1
fi
echo "ndx running" >&2
TEXT=/tmp/cpy
SUBJS=/tmp/sb
SUJS=/tmp/sj

#  store name(s) of the textfile(s) in "textlist" and the options
#  to the formatter (nroff, troff, mmt, mm) in "optlist"
#
textlist=
optlist=
opt=n		#formatter option, default=nroff
pgopt=n			#section-page numbering, default = none
pagereg=n		#formatter page register, default=nroff
arg=$2
for i in $arg
        do
                case $i in
                        troff) opt=t;;
                        mmt) opt=z;;
			nroff) opt=n;;
			mm) pagereg=m; opt=m;;
			-rN3 | -rN5) pgopt=s;
				optlist="$optlist $i";;
			-z) ;;
			-*) optlist="$optlist $i";;
			*) textlist="$textlist $i";;
			default) echo $0: unknown formatter: $arg >&2;
				echo "formatters are: troff, nroff, mmt, mm">&2
                                exit;;
                esac
        done
#

	sed -e "/^$/d" -e "/^[ 	][ 	]*$/d" $1 > $SUBJS$$
	sort +0ub -o $SUBJS$$ $SUBJS$$
	pr -t -n1 -s $SUBJS$$ > $SUJS$$
	
	#
	#  begin inner loop: it repeats for each textfile
	#                    this is where the real business of this shell
	#                    program is begun.  The program ndexer, a
	#                    lex program, is run on each textfile in the
	#                    "textlist" seaching for the subjects listed in
	#                    the current subject list
	#                    save the marked up textfile in $TEXT$COUNT$$
	#
	
	COUNT=0
	for text in $textlist
	        do
		if
		      	test ! \( -r $text \)
		then
		      	echo "ndx: ndx aborted, cannot read file $text" >&2
		              echo "  please check to see that $text exists and is readable" >&2
		rm -f $TEXT*$$ $SUBJS$$.* $SUJS$$.*
		              exit 1
		fi
		$UL/ndexer $SUJS$$  $text $TEXT$COUNT$$ $pagereg > /dev/null
		if
	                test $? -ne 0
		then
	                echo "ndx: ndx aborted, files were $1 $text" >&2
	                rm -f $TEXT*$$ $SUBJS$$.* $SUJS$$.*
	                exit 2
		fi      
		COUNT=`expr "$COUNT" + 1`
		done
	
	FILE10=
	FILE100= 
	FINAL=/tmp/ndx0$$
	IFILE2=/tmp/ndx2$$
	SFILE=/tmp/ndx3$$
	
	#
	#  format the textfiles using the options saved in "optlist" and the user-
	#  named formatter.  Save only the subject numbers and page numbers in the
	#  file $FINAL
	#
	if test "$COUNT" -gt "9"; then FILE10="$TEXT??$$"; fi;
	if test "$COUNT" -gt "99"; then FILE100="$TEXT???$$"; fi;
	case $opt in
		t) troff -z $optlist  $TEXT?$$ $FILE10 $FILE100 1>/dev/null 2> $FINAL ;;
		n) nroff -z $optlist  $TEXT?$$ $FILE10 $FILE100 1>/dev/null 2> $FINAL ;;
		z) mmt   -z $optlist  $TEXT?$$ $FILE10 $FILE100 1>/dev/null 2> $FINAL ;;
		m) mm       $optlist  $TEXT?$$ $FILE10 $FILE100 1>/dev/null 2> $FINAL ;;
	esac
	
	if
		test $? -ne 0
	then
		case $opt in
			t) echo '  error message from troff' >&2;;
			n) echo '  error message from nroff' >&2;;
			z) echo '  error message from nroff' >&2;;
			m) echo '  error message from mm' >&2;;
		esac
	
	sed -n "/^[^0-9]/p" $FINAL >&2
	echo "  please check your formatter options"; echo
	exit 3
	fi
	
	#
	#  Process $FINAL replacing subject numbers
	#  with actual subjects and collating page numbers
	#  Append the index to INDEX$$
	#
	
	sed -n "/^[^0-9]/p" $FINAL >&2
	sed "/[^0-9] [^0-9] */d" $FINAL |
        sort  -u +0n +1n +2n | $UL/pages $pgopt >$IFILE2
	$UL/sbjprep $SUJS$$ >$SFILE
	paste -d] $SFILE $IFILE2 >> /tmp/INDEX$$ 
	rm -f $TEXT*$$
        rm -f $FINAL


#
#  if all the subject lists have been processed, sort the file
#  INDEX$$, make it look good, and output it to the user.
#  remove temporary files and exit.
#

        sed "s/,/ ,/" /tmp/INDEX$$ |
	sort -u +1fibt] -2 +1 -2  | 
	sed "s/ ,/,/" |
	$UL/ndxformat  
	rm -f /tmp/slist.$$ $SUBJS$$* $SUJS$$  $IFILE2 $SFILE  INDEX$$
echo "ndx has completed" >&2
