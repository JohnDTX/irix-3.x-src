#! /bin/sh
# manegrep - egrep the man page source tree
#
# Usage: manegrep egrep_arguments
#
# egrep_arguments are of same form as arguments to egrep
#
# $Revision: 1.1 $
# $Date: 89/03/27 16:39:34 $
#
cd ..
find [gua]_man troff \( -name '*.[1-8]' -o -name '*.[1-8][a-z]' \) -print \
    | xargs egrep "$@"
