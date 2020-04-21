#! /bin/csh -fe
#   Compute header file dependencies
#	This follows the recursive tree of header file inclusions.
#
#   An invocation of this script in a make file should be of the
#   following form:
#	depend:source-files	# the dependencies are optional
#
#	c-depend "${CC} -I." makefile makefile.bak ${SRC}
#	modified for cpp -M filtration  -be

#   1 April 86

# notice 'incremental' mode
if ("$1" == "-i") then
    set incr = 1
    shift
else
    set incr = 0
endif

if ($#argv < 4) then
    echo usage: '[-i] "${CC} -I. -E" makefile makefile.bak ${SRC}'
    exit 1
endif

# define flag lines
set fline = '# DO NOT DELETE THIS LINE -- make depend uses it'
set lline = '# DO NOT DELETE THIS 2nd LINE -- make depend uses it'

# get awk script to use
set awk = $0
set awk = ${awk:r}.awk

# get cpp dependency generator command
set cpp = "$1 -M"
shift

# get make file names
set nf = "$1"
shift
set of = "$1"
shift

# save the old file in case of problems
if (-f $nf) mv -f $nf $of

# remove old dependencies from the makefile
sed -e "/^${fline}/,/^${lline}/d" -e "/^${fline}/"',$d' $of > $nf

# delimit the dependencies
if ($incr) then
    # save old dependences in 'incremental' mode
    sed -n -e "/^${fline}/,/^${lline}/p" -e "/^${fline}/"',$d' $of >> $nf
else
    echo $fline >> $nf
endif

# process each source file
# XXX when cc recognizes -M, and *provided* argv doesn't exceed NCARGS
# XXX characters, can do this all at once.
#	$cpp $argv[1-] >> $nf
while ($#argv > 0)
    $cpp $1 | \
    sed -e 's:\.\./[^ ]*/\.\.:..:g' \
	-e 's: ./: :' | \
    awk -f $awk - >> $nf
    shift
end

echo $lline >> $nf
