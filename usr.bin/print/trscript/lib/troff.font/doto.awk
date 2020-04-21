# transcript/lib/troff.font/doto.awk
#
# Copyright (c) 1985 Adobe Systems, Inc.
#
# Gets used by the Makefile in this directory.
#
# This short "awk" program generates a list of ".o" files by
# searching its input (a ".map" file) for the @FACENAMES line.
# This list is used as the argument string to a "make" so that
# the troff width tables can be built automatically from the ".map"
# file.

/^@FACENAMES /	{print "ft" $2 ".o", "ft" $3 ".o", "ft" $4 ".o", "ft" $5 ".o"
		exit}
