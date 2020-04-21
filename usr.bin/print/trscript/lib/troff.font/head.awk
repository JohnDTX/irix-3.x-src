# transcript/lib/troff.font/head.awk
#
# Copyright (c) 1985 Adobe Systems, Inc.
#
# Gets used by the Makefile in this directory.
#
# This short "awk" program generates a ".head" file (a sequence of
# troff ".fp" commands) for a PostScript font family to be used with
# troff and pscat.  The ".map" file is the input, and the output is
# the ".head" data.  Note that position 4 is hard-wired to a font named
# "S" (not the fourth face in the @FACENAMES list).

/^@FACENAMES /{	print ".fp 1 " $2
		print ".fp 2 " $3
		print ".fp 3 " $4
		print ".fp 4 S"
		exit
		}
