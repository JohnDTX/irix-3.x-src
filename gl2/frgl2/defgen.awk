#
#	This awk script generates #define type constucts for C, Pascal,
#	Fortran and PLI.  The format each line of the .prim input file is 
#		name value type /* comment
#	where "/* comment" is optional and only comment can contain white space.
#

BEGIN {
	fh = "tempdef.f"
}
/^#|^$/ { next }	# ignore comments and blank lines

# print out comment lines that begin with `/*'
/^\/\*/ {
	l = length($0)-2
	print "\nc" substr($0,3,l) "\n" >fh
	next
}
{
	# fortran  can't handle hex constants
	if ( index($2,"0x") == 1) {
		val = 0
		for (i=3; i<= length($2); i++)
			val = val*16 + substr($2,i,1) 
	} else {
		val = $2
	}
	if (length($1) < 4) {
		print "      PARAMETER ( " substr($1,1,6) " =\t\t" val " )" >fh
	} else {
		print "      PARAMETER ( " substr($1,1,6) " =\t" val " )" >fh
	}
}
