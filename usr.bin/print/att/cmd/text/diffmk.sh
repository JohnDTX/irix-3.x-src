#! /bin/sh
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

trap 'rm -f /tmp/d$$; exit' 1 2 3 15
if [ $? != 0 ] || test -z "$3" -o "$3" = "$1" -o "$3" = "$2"; then
	echo "usage: diffmk name1 name2 name3 -- name3 must be different" >&2
	exit 2
fi
if test "$3" = "-" -o "$1" = "-" -a "$2" = "-"; then
	echo "usage: diffmk name1 name2 name3 -- name3 must be different" >&2
	exit 2
fi
if test "$1" != "-" && test ! -r "$1"; then
	echo "diffmk: $1 can not be read" >&2
	exit 2
fi
if test "$2" != "-" && test ! -r "$2"; then
	echo "diffmk: $2 can not be read" >&2
	exit 2
fi
if test "$1" = "-"; then
	file1=/tmp/d$$
	cat - >$file1
else
	file1=$1
fi
diff -e $file1 $2 | (sed -n -e '
/[ac]$/{
	p
	a\
.mc \\(br
: loop
	n
	/^\.$/b done1
	p
	b loop
: done1
	a\
.mc\
.
	b
}

/d$/{
	s/d/c/p
	a\
.mc \\(**\
.mc\
.
	b
}'; echo '1,$p') | ed - $file1| sed -e '
/^\.TS/,/.*\. *$/b pos
/^\.T&/,/.*\. *$/b pos
p
d
:pos
/^\.mc/d
' > $3
if test "$1" = "-"; then
	rm -f /tmp/d$$
fi
