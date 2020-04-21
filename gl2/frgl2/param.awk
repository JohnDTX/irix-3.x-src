#
#	This awk script generates types for PARAMETERs in Fortran.
#	The format each line of the .prim input file is 
#		name value type /* comment
#	where "/* comment" is optional and only comment can contain white space.
#

BEGIN {
	fh = "temptype.f"
}
/^#|^$/ { next }	# ignore comments and blank lines

# ignore comment lines that begin with `/*'
/^\/\*/ { next }

{
	if ( $3 == "s" ) {
		print "      integer*2 " substr($1,1,6) >fh
	} else if ( $3 == "l" ) {
		print "      integer*4 " substr($1,1,6) > fh
	}
}
