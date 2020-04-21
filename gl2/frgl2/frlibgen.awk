#
#   This awk script generates lib.f and fgl.h, both of which are used to
#   make the remote graphics library (stubs), flibgl2.a.  See the Makefile
#   in this directory for how it it used and what is required to complete
#   the stubs and header file generation.
#
#   Also added now is generation of a set of function and subroutine
#   definitions (in mandefs.fr) to check the graphics library manual
#   against.  RDG, Feb 85
#
#   The creation of frgl.h has been commented out from here and the makefile.
#   We don't want to have to change include file names when moving from 
#   workstation to remote host.  CSK 6/5/85
#
#   Note that its actions are bounded by search strings.  For instance,
#   only certain commands are executed on segments of the input file
#   (usually lib.prim) that are bounded by the strings #---------- ROUTINES
#   and #---------- END ROUTINES.  The number of dashes is important.
#	GEW 1/2/86
#

BEGIN {
    cmd = 0		# graphics function call number
    fl = "lib.f"
#   fh = "tempfgl.h"
    fd = "mandefs.fr"

# print headings for files

    print "C\nC      FORTRAN graphics library\nC" >fl
#   print "\nC      function declarations\n" >fh

    printf "\nDefinitions of Fortran graphics functions & subroutines " >fd
    print "to check against manual\n" >fd
    print "Notes:\n" >fd
    print "	(1) Arrays abc(x,y) in the manual may be presented here as" >fd
    print "	    abc(x*y) or abc(y*x).  In lib.f, they are merely abc(*).\n" >fd
    print "	(2) gflush() is missing, & an x prefix is added as follows:" >fd
    print "	    the following:  gexit, ginit, greset, setfas, setslo" >fd
    print "	    become:         xgexi, xgini, xgrese, xsetfa, xsetsl\n" >fd
    print "	(3) The notation <received> is for reference only\n" >fd
}

# DO FOR EVERY LINE BRACKETED BY "---------- TRANSFER TYPES"...."---------- END TRIPLETS"
/^#---------- TRANSFER TYPES/ , /^#---------- END TRIPLETS$/ {
    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }   # ignore comments and blank lines
    typ[$1] = $3	# READ TYPE DEFINITIONS INTO AN ARRAY
}

# GENERATE LIB.F, ETC.
# Do for every line bracketed by "---------- ROUTINES"...."---------- END ROUTINES"
/^#---------- ROUTINES/ , /^#---------- END ROUTINES$/ {
    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }    # ignore comments and blank lines

# functions that are special cases

	if ( $0 ~ /:gflush/ ) { next }
	if ( $0 ~ /:ginit/ ) { $1 = "VVv:xginit(" }
	if ( $0 ~ /:greset/ ) { $1 = "VVv:xgreset(" }
	if ( $0 ~ /:gexit/ ) { $1 = "VVv:xgexit(" }
	if ( $0 ~ /:setslowcom/ ) { $1 = "UOv:xsetslowcom(" }
	if ( $0 ~ /:setfastcom/ ) { $1 = "UOv:xsetfastcom(" }
	if ( $0 ~ /:bogus/ ) { cmd++; next }
	if ( $0 ~ /:callfunc/ ) { cmd++; next }	    # works only in 'c'
	if ( $0 ~ /:charstr/ ) { cmd++; next }
				# 'charst', used for Fortran, is different
	if ( $0 ~ /:strwidth/ ) { cmd++; next }
				# 'strwid', used for Fortran, is different
	if ( $0 ~ /:getport/ ) { cmd++; next }
				# 'getpor', used for Fortran, is different
	if ( $0 ~ /:winopen/ ) { cmd++; next }
				# 'winope', used for Fortran, is different
	if ( $0 ~ /:wintitle/ ) { cmd++; next }
				# 'wintit', used for Fortran, is different
	if ( $0 ~ /:iftpsetup/ ) { cmd++; next }
				# 'iftpse', used for Fortran, is different
	if ( $0 ~ /:capture/ ) { cmd++; next }
				# 'captur', used for Fortran, is different
	if ( $0 ~ /:rcapture/ ) { cmd++; next }
				# 'rcaptu', used for Fortran, is different
	if ( $0 ~ /:addtopup/ ) { cmd++; next }
				# 'addtop', used for Fortran, is different

# FOR EACH FUNCTION, check for number of fields and proper parens

	for ( nf=0 ; nf<NF && substr($(nf+1),1,1)!=")" ; nf++ ) { }
		# this line allows comments after the right paren in lib.prim
		# nf = number of parameter fields
	printf "\n" >fl
	printf "\n" >fd
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

	if ( length( typ[retval] ) != 1 ) {
	    print "Return value should not be an array in \""$1"\", "FILENAME" line",NR
	    next
	}
	deftype[0] = typ[typ[retval]]		# type declaration
	if ( length( deftype[0] ) == 0 ) {
	    print "No type for \"" retval "\" found in \""$1"\", "FILENAME" line",NR
	    next
	}
	k = length(fn[2]) - 1
	if ( k>6 ) { k=6 }
	fnname = substr(fn[2],1,k) 

# print comment, then function or subroutine declaration line

	print "C\nC     prim " $0 "\nC" >fl
	print $0 "\n" >fd
	printf "      " deftype[0] >fl
	printf "\t\t" deftype[0] >fd
	recsval[0] = 0			# assume a subroutine
	recsvals = 0			# assume no values received
		# recsval[n] & recsvals are used as Booleans
	if ( deftype[0] != "subroutine" ) {
	    recsval[0] = 1		# nope, its a function
	    recsvals = 1		# at least 1 value received
#	    printf "      " deftype[0] >fh
	    printf " function" >fl
	    printf " function" >fd
	}

# print function or subroutine name (fgl.h (>fh) receives only function decl's)

	printf " " fnname "(" >fl
	printf " " fnname "(" >fd
#	if ( recsval[0] ) { print " " fnname >fh }

# print parens and as on function or subroutine declaration line

	for ( i=1 ; i<nf ; i++ ) {
	    printf "a" i >fl
	    printf " a" i >fd
	    if ( i<nf-1 ) {
		printf "," >fl
		printf "," >fd
	    }
	}
	print ")" >fl
	print " )" >fd

# print declaration of value returned, if a function

	dcl["F"] = 0		# These are Boolean values, true once the
	dcl["L"] = 0		#   recX function for the transfer type
	dcl["O"] = 0		#   represented by the subscript has
	dcl["B"] = 0		#   been declared.
	dcl["S"] = 0

	if ( ! recsval[0] ) {	    # if a subroutine, do nothing
	} else if ( (xfrtyp[0] = substr(retval,2,1)) ~ /[FLO]/ ) {
				# if a float, long, or Boolean is returned
	    if ( typ[retval] != xfrtyp[0] ) {
		printf "Physical and defined return types do not match in \""
		print $1"\", "FILENAME" line",NR
	    }
	    print "      " deftype[0] " rec" xfrtyp[0] >fl
	    dcl[xfrtyp[0]] = 1
	} else {
	    print "Invalid return value type for \""$1"\", "FILENAME" line",NR
	    next
	}

# FOR EACH PARAMETER i, print declarations

	for ( i=1 ; i<nf ; i++ ) {
	    hasszfld[i] = split( $(i+1),p,":" ) - 1
		# Boolean hasszfld[i] is true if a i has an array size field
	    deffld[i] = p[1]			       # def'n field of param i
	    if ( hasszfld[i] ) { szfield[i] = p[2] }   # if is array, size field
	    if ( hasszfld[i] > 1 ) {
		printf "Too many colons in \"" $(i+1) "\", function \""
		print fnname "\", "FILENAME" line",NR
		next
	    }

	    hasss = length( typ[deffld[i]] ) - 1
		# Boolean hasss is true if defined type is 2 chars (i.e. array)
	    phystype[i] = substr( typ[deffld[i]],1,1 )
	    deftype[i] = typ[phystype[i]]
	    if ( length( deftype[i] ) == 0) {
		if ( typ[deffld[i]] != "<not_avail>" )
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

	    if ( deftype[i] == "character" && hasss ) {
		deftype[i] = "character*(*)"
	    }
	    printf "      " deftype[i] " a" i >fl
	    printf "\t\t" deftype[i] " a" i >fd
		# declaration is complete if the a has no size field spec

	    if ( hasszfld[i] ) {
		  # must add size to declaration if array
		if ( deftype[i] == "character*(*)" ) {
		    printf "*" szfield[i] >fd
		} else {			# if array is not a byte array
		    printf "(*)" >fl
		    if ( phystype[i] ~ /[qQ]/ ) {
			printf "(4*" szfield[i] ")" >fd
		    } else {
			printf "(" szfield[i] ")" >fd
		    }
		}
	    }
	    if ( recsval[i] ) {
		printf "  <received>" >fd
	    }
	    print "" >fl
	    print "" >fd

# declare return value, if needed and if not an array

	    if ( ! hasszfld[i] && recsval[i] && ! dcl[xfrtyp[i]] ) {
		if ( phystype[i] == xfrtyp[i] ) {
		    print "      " deftype[i] " rec" xfrtyp[i] >fl
		    dcl[xfrtyp[i]] = 1
		} else {
		    printf "No receive type conversions available for "
		    print fnname ", " FILENAME " line " NR " and not reinitialized"
		}
	    }
	}	# end of aument declarations

# CALL GRAPHICS PRIMITIVE

	if ( recsvals ) {
	    print "      call echoff" >fl
	}
	print "      call gcmd(" cmd ")" >fl
	cmd++

# FOR EACH PARAMETER TO BE SENT

	for ( i=1 ; i<nf ; i++ ) {
	    if ( ! recsval[i] ) {		# if this parameter is sent
		if ( phystype[i] == xfrtyp[i] ) {
		    printf "      call send" typ[deffld[i]] >fl
		} else {
		    if ( hasszfld[i] ) {
			printf "There are no array type conversion routines for "
			print fnname ", " FILENAME " line " NR
		    } else {
			printf "      call senf" phystype[i] xfrtyp[i] >fl
		    }
		}

		if ( hasszfld[i] ) {		# if an array
	# Note: this also works if a single typedef array is sent by reference
		    print "(a" i "," szfield[i] ")" >fl
		} else {
		    print "(a" i ")" >fl
		}
	    }
	}

# FOR EACH VALUE TO BE RECEIVED

	if ( recsvals ) {			# if any vals received
	    print "      call flushg" >fl
	    if ( recsval[0] ) {			# if a function, not subroutine
		print "      " fnname " = rec" xfrtyp[0] "()" >fl
	    }

	    for ( i=1 ; i<nf ; i++ ) {		# for each parameter
		if ( recsval[i] ) {		# if this param is received
		    if ( hasszfld[i] ) {		# if an array
	# Note: this also works if a single typedef array is sent by reference
			print "      call rec" xfrtyp[i] "s(a" i ")" >fl
		    } else {			# if not an array
			print "      a" i " = rec" xfrtyp[i] "()" >fl
		    }
		}
	    }
	    print "      call reccr\n      call echoon" >fl
	}

# Endpri is not yet used, even though it appears in the c version
#   print "      call endpri" >fl

	print "      return\n      end" >fl
}
