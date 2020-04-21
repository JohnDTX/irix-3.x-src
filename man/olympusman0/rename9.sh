#! /bin/sh
# rename9 - create a list of commands to rename man page files and their 
#	    RCS files to have basenames of 9 characters or less
#
# Usage: rename9 > rename9.out
#	 (examine rename9.out)
#	 mv rename9.out ..
#	 cd ..
#	 sh -x rename9.out
#
cwd=`pwd`
cd .. 
find [gau]_man troff \( -name '*.[1-8]' -o -name '*.[1-8][a-z]' \) -print \
	| xargs $cwd/rename9x
