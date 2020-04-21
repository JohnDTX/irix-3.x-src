#! /bin/sh
#
# Copy the ../kernels tree into the destination tree
#

if test "$GLHDWR" = "gl1"
then
	cp ../kernels/1?00.* $1
else
	if test "$machine" = "iris"
	then
		cp ../kernels/2?00.* $1
	else
		cp ../kernels/3?00.* $1
	fi
fi
exit 0
