#!/bin/sh -
#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)newvers.sh	5.1 (Berkeley) 6/6/85
#
awk 'END { printf "char version[] = \"Version SGI 4.%d ", version > "vers.c"; }' \
	version
echo `date`'";' >> vers.c
