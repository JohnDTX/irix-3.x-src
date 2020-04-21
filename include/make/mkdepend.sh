#! /bin/sh
# NAME
#	mkdepend - compute header file dependencies
# SYNOPSIS
#	mkdepend [-c compilehow] [-e sedprog] [-f force] [-i] depfile file ...
# DESCRIPTION
#	Mkdepend infers make dependencies from source containing C #include
#	directives.  Given a shell command compilehow which consists of cc
#	followed by options, mkdepend processes its file arguments and edits
#	the generated dependency information into depfile, which may be a
#	makefile or a make include file.
#
#	The -e flag passes an immediate program to sed, which is applied to
#	raw dependency information of the following form:
#	
#	target: dependent
#
#	Thus one may substitute pathname prefixes with envariable parameters,
#	for example.
#
#	The -f flag causes mkdepend to add a dependent named force to each
#	target file's dependency list.  Using -f '$(FRC)' and setting FRC=FRC
#	in a make's environment, one may rebuild certain objects without first
#	removing them.
#
#	Normally, old dependencies are deleted from the depfile.  The -i
#	option causes mkdepend to preserve old dependencies.  When invoked
#	from a makefile, the following rule enables incremental updates to
#	the ".mkdepend" dependency database:
#
#	.mkdepend: $(CFILES)
#		mkdepend -c "$(CC) $(CFLAGS)" -i $@ $?
#
depgen="cc -M"
incr=no

#
# Process options and arguments.  We put quotes around the edit (-e) arguments
# and use eval "sed -e ..." later, to preserve spaces in the edit command.
#
while test $# -gt 0; do
	case $1 in
	  -c)	depgen="$2 -M" shift; shift;;
	  -e)	sedprog="$sedprog -e '$2'" shift; shift;;
	  -f)	force="$force $2" shift; shift;;
	  -i)	incr=yes shift;;
	  *)	break
	esac
done
case $# in
  0)
	echo \
    "usage: $0 [-c compilehow] [-e sedprog] [-f force] [-i] depfile [file ...]"
	;;
  1)
	exit 0		# no source file arguments, so nothing to do
esac

newmakefile="$1"
oldmakefile="#$1"
shift

#
# An awk script to compress dependencies.
#
awkprog='
BEGIN {
	INDENT = "\t"
	INDENTSIZE = 8
	MAXLINE = 72
	MAXINDENTEDLINE = MAXLINE - INDENTSIZE - 1
}

#
# For each line of the form "target1 ... targetN: dependent", where the
# spaces are literal blanks, do the following:
#
NF > 0 {
	if ($1 != target) {
		target = $1
		if (depline) print depline 
		colon = index($0, ":")
		depline = substr($0, 1, colon)
		if ("'$force'" != "") {
			depline = depline " '"$force"' "
		} else {
			depline = depline " "
		}
		lim = MAXLINE
	}
	rhs = substr($0, colon + 2)
	if (length(depline) + length(rhs) > lim) {
		print depline "\\"
		depline = INDENT
		lim = MAXINDENTEDLINE
	}
	depline = depline rhs " "
}

END {
	print depline 
}'

#
# Save the old file in case of problems.  Use a temporary file for incremental
# make depend.  Clean up if interrupted.
#
deptmp=/tmp/incdep.$$
if test $incr = yes; then
	trapcmds="rm -f $deptmp;"
fi
if test -f $newmakefile; then
	trapcmds="$trapcmds mv -f $oldmakefile $newmakefile;"
	mv -f $newmakefile $oldmakefile
fi
trap "$trapcmds exit 0" 1 2 15
if test ! -f $oldmakefile; then
	echo "Neither $oldmakefile nor $newmakefile exists."
	exit 1
fi

#
# Remove any old dependencies from the makefile.
#
firstline='# DO NOT DELETE THIS LINE -- make depend uses it'
lastline='# DO NOT DELETE THIS 2nd LINE -- make depend uses it'

sed -e "/^$firstline/,/^$lastline/d" -e "/^$firstline/"',$d' $oldmakefile \
    > $newmakefile

#
# Delimit the beginning of the dependencies.  Save old dependences in
# incremental mode.
#
if test $incr = no; then
	echo $firstline >> $newmakefile
else
	fgrep -s "$firstline" $oldmakefile
	if test $? -ne 0; then
		echo $firstline >> $newmakefile
	fi
	sed -n -e "/^$firstline/,/^$lastline/p" $oldmakefile |
	    grep -v "^$lastline" > $deptmp
	awkinc='/^[^ 	].*$/ { deleting = 0; }'
	for f in $*
	do
		basename=`expr $f : '\(.*\)\..*'`
		basename=`basename $basename`
		awkinc="$awkinc /^$basename/ { deleting = 1; }"
	done
	awkinc="$awkinc { if (deleting == 0) print \$0; }"
	awk "$awkinc" $deptmp >> $newmakefile
	rm -f $deptmp
fi

#
# Process source files - the third sed command is needed because of bogus IRIS
# cc blather on stdout instead of on stderr.
#
$depgen $* |
    eval "sed -e 's:\.\./[^\./][^\./]*/\.\.:..:g' \
	-e 's: \./: :' \
	-e '/^[^ 	]*\..*:$/d' \
	$sedprog" |
    sort -u |
    awk "$awkprog" >> $newmakefile
echo $lastline >> $newmakefile
rm -f $oldmakefile
