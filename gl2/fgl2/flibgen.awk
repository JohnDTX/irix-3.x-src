#
#   This awk script does most of the work to generate the remote graphics
#   libraries (stubs).  See the Makefile in this directory
#   for how it it used and what is required to complete the stubs and
#   header file generation.
#
#   The creation of tempgl.h has been commented out.  We don't want to have
#   to change include file names when moving from workstation to remote 
#   host.  CSK 6/5/85
#
#   Note that its actions are bounded by search strings.  For instance,
#   only certain commands are executed on segments of the input file
#   (usually lib.prim) that are bounded by the strings #---------- ROUTINES
#   and #---------- END ROUTINES.  The number of dashes is important.
#	GEW 1/2/86
#
BEGIN {
    cl = "lib.i"
#   ch = "tempgl.h"

# print headings for files

}

# DO FOR EVERY LINE BRACKETED BY "---------- TRANSFER TYPES"...."---------- END TRIPLETS"
/^#---------- TRANSFER TYPES/ , /^#---------- END TRIPLETS$/  {
    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }    # ignore comments and blank lines

# Read type definitions into an array
	    ctyp[$1] = $2
	    ftyp[$1] = $3
}

# DO FOR EVERY LINE BRACKETED BY "---------- ROUTINES"...."---------- END ROUTINES"
/^#---------- ROUTINES/ , /^#---------- END ROUTINES$/ {
    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }    # ignore comments and blank lines

# functions that are special cases

#	if ( $0 ~ /:gflush/ ) { next }
#	if ( $0 ~ /:ginit/ ) { $1 = "VVv:xginit(" }
#	if ( $0 ~ /:greset/ ) { $1 = "VVv:xgreset(" }
#	if ( $0 ~ /:gexit/ ) { $1 = "VVv:xgexit(" }
#	if ( $0 ~ /:setslowcom/ ) { $1 = "VVv:xsetslowcom(" }
#	if ( $0 ~ /:setfastcom/ ) { $1 = "VVv:xsetfastcom(" }
	if ( $0 ~ /:bogus/ ) { cmd++; next }

# skip functions that do not need wrappers, as indicated by no 'w' appended
#   to the function in lib.prim

	if ( $0 !~ / \)w/ ) { next }

# FOR EACH FUNCTION, check for number of fields and proper parens

	for ( nf=0 ; nf<NF && substr($(nf+1),1,1)!=")" ; nf++ ) { }
		# this line allows comments after the right paren in lib.prim
		# nf = number of parameter fields
	printf "\n" >cl
	if ( substr($1,length($1))!="(" ) {
	    print "Missing \"(\" in "$1", "FILENAME" line",NR
	    next
	}
	if ( substr($(nf+1),1,1)!=")" ) {
	    print "Missing \")\" in "$1", "FILENAME" line",NR
	    next
	}

# find function name, return value, return value type, etc.

	h = split($1,fn,":")	# Split return type from function name
	if ( h != 2 ) {
	    print "\""$1"\" should have exactly one colon, "FILENAME" line",NR
	    next
	}
	retval = fn[1]		# Return value

	if ( length( ftyp[retval] ) != 1 ) {
	    print "Return value should not be an array in \""$1"\", "FILENAME" line",NR
	    next
	}
	deftype[0] = ctyp[ftyp[retval]]		# type declaration
	if ( length( deftype[0] ) == 0 ) {
	    print "No type for \"" retval "\" found in \""$1"\", "FILENAME" line",NR
	    next
	}
	k = length(fn[2]) - 1
	fnname = substr(fn[2],1,k) 

# print comment, then function or subroutine declaration line

	print "/*\n** prim " $0 "\n*/" >cl
	printf deftype[0] " " >cl
#	printf "extern " deftype[0] "\t" >ch
	recsval[0] = 0				# assume void (int) returned
	recsvals = 0				# assume no values received
		# recsval[n] & recsvals are used as Booleans
	if ( deftype[0] != "int" ) {
	    recsval[0] = 1		# nope, its a function
	    recsvals = 1		# flag echo sw, at least 1 val received
	}

# print function or subroutine name

	printf fnname "(" >cl
#	print fnname "();" >ch

# print parens and args on function or subroutine declaration line

	for ( i=1 ; i<nf ; i++ ) {
	    printf " a" i >cl
	    if ( i<nf-1 ) {
		printf "," >cl
	    }
	}
	print " )" >cl

# FOR EACH PARAMETER i, print declarations

	for ( i=1 ; i<nf ; i++ ) {
	    hasszfld[i] = split( $(i+1),p,":" ) - 1
		# Boolean hasszfld[i] is true if arg i has an array size field
	    deffld[i] = p[1]
	    if ( hasszfld[i] > 1 ) {
		printf "Too many colons in \"" $(i+1) "\", function \""
		print fnname "\", "FILENAME" line",NR
		next
	    }

	    hasss = length( ftyp[deffld[i]] ) - 1
		# Boolean hasss is true if defined type is 2 chars (i.e. array)
	    phystype = substr( ftyp[deffld[i]],1,1 )
	    if ( mode = substr( deffld[i],3,1 ) >="A" && mode<="Z" ) {
		if ( ctyp[phystype] != "long" ) {
		    deftype[i] = "unsigned " ctyp[phystype]
		} else {
		    deftype[i] = ctyp[phystype]
		}
	    } else {
		deftype[i] = ctyp[phystype]
	    }
	    if ( mode == "V" ) { mode = "v" }
	    if ( mode == "R" ) { mode = "r" }
	    if ( mode == "A" ) { mode = "a" }
	    if ( length( deftype[i] ) == 0 ) {
		print "No type for \"" deffld[i] "\" found in \""$1"\", "FILENAME" line",NR
		next
	    }

	    if ( hasszfld[i] && ! hasss ) {
		printf "Size field unused in \"" $(i+1) "\", function \""
		print fnname "\", "FILENAME" line",NR
		hasszfld[i] = 0
	    }
	    if ( ! hasszfld[i] && hasss ) {
		printf "Size field missing in \"" $(i+1) "\", function \""
		print fnname "\", "FILENAME" line",NR
		next
	    }
	    recsval[i] = 0	# assume this param sent, not received
	    if ( (xfrtyp[i] = substr(deffld[i],2,1)) >="A" && xfrtyp[i]<="Z" ) {
		recsval[i] = 1	# nope, this param is received
		recsvals = 1	# If param is received, flag echo sw needed
	    }

# print the type of the parameter

	    printf deftype[i] " " >cl
	    if ( mode == "r" && ! hasszfld[i] ) {
				# if passed by ref & has no array size field
		printf "*" >cl
	    }
	    printf "a" i >cl
		# declaration is complete unless the arg's xfr type is array

	    if ( hasszfld[i] ) {
		    printf "[]" >cl
	    }
	    print ";" >cl
	}

	print "{\n}" >cl
}
