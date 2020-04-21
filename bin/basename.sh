#! /bin/sh
#	@(#)basename.sh	1.3
a=`expr //${1-.} : '.*/\(.*\)'`
expr $a : "\(.*\)$2$" \| $a
