#
#   This awk script generates two pascal graphics libraries header
#   files for use on the workstation.  See the Makefile in this directory
#   for how it it used.
#   Note that its actions are bounded by search strings.  For instance,
#   only certain commands are executed on segments of the input file
#   (usually lib.prim) that are bounded by the strings #---------- ROUTINES
#   and #---------- END ROUTINES.  The number of dashes is important.
#	GEW 1/2/86
#

# startup
BEGIN {
    procf = "pglprocs.h"
    procd = "mandefs.p"
    macf = "pglmacs.h"

# print headings for files
    print "(* pascal function declarations *)" >procf
    print  "function G_P2CSTR(a: string128): pstring128; cexternal;" >procf

    print "			Pascal Declarations" > procd
    print "" > procd
    print "Note that arrays appear as their base type.  For example, what" > procd
    printf "used to be declared as an \"RGBarray\" now appears as \"RGBvalue\".\n" > procd
    print "The type RGBarray type, along with all the other fixed-length array" > procd
    print "types of length MAXARRAY no longer exist.  The programmer should declare " > procd
    print "an array of the proper base type in his program, then use it in the call" > procd
    printf "to a gl routine.  See \"Pascal Notes\" for more info.\n" > procd
    print "" > procd

    print  "function G_P2CSTR(a: string128): pstring128;" >procd
    print "" > procd

    argname = "abcdefghijklmnopqrstuvwxyz"
}


# ignore transfer types
/^#---------- TRANSFER TYPES/ , /^#---------- END TRANSFER TYPES$/ {

	    next
	}

# ignore triplets
/^#---------- TRIPLETS/ , /^#---------- END TRIPLETS$/ {

	    next
	}

# get type abbreviations, mapped to pascal
/^#---------- DEFTYPES/ , /^#---------- END DEFTYPES$/ {
	    if ( $0 ~ /^#|^$/ )
		next


	    basetype[$1] = $4
	    stacktype[$1] = $5
#end ws pascal
	}

# generate external definitions
/^#---------- ROUTINES/ , /^#---------- END ROUTINES$/ {
	    if ( $0 ~ /^#|^$|:bogus/ )
		next

# print raw form as a comment

	    print "(*", $0, "*)" >procf

# find true number of args; ignore trailing w, and comment

	    for ( i = 1; i < NF && substr($(i), 1, 1) != ")"; i++ )
		;
	    lastarg = i-1

	    firstarg = 2
	    na = lastarg-1
	    if ( substr($1, length($1), 1) != "(" )
	    {
		print "Missing '(' in "$1", "FILENAME" line "NR
		next
	    }
	    if ( substr($(i), 1, 1) != ")" )
	    {
		print "Missing ')' in "$1", "FILENAME" line "NR
		next
	    }

# split return triplet and function name

	    if ( split($1, fn, ":") != 2 )
	    {
		print "Colon miscount in "$1", "FILENAME" line "NR
		next
	    }

	    trip = fn[1]		# Return value triplet
	    funcname = fn[2]		# Function name
	    i = length(funcname) - 1
	    funcname = substr(funcname, 1, i)

	    if ( length(trip) != 3 )
	    {
		print "Illegal return triplet, "FILENAME" line "NR
		next
	    }

# save return type and mode

	    rettype = substr(trip, 1, 1)
	    retmode = substr(trip, 3, 1)
	    
# start building parts of the decl and doc

	    if ( rettype != "V" )
		decl = "function "
	    else
		decl = "procedure "

	    doc = decl funcname
	    decl = decl "_" funcname

# build formal arg list

	    oldtrip = $2

	    a = na
	    for ( i = lastarg; i >= firstarg; )
	    {
		if ( i == lastarg )
		    decl = decl "("

		trip = $(i)		# gather consecutive args of same type
		for ( j = i-1; j >= firstarg && $(j) == trip; j-- )
		    ;

		atype = substr(trip, 1, 1)
		amode = substr(trip, 3, 1)
		
		if ( amode == "r" || amode == "a" )
		    decl = decl "var "

		while ( i > j )		# kick out formal arg names
		{
		    decl = decl substr(argname, a, 1)
		    i--		# yes, we're really decrementing the loop var
		    a--
		    if ( i > j )
			decl = decl ", "
		    else
			decl = decl ": "  # terminate this set of args
		}

		if ( amode == "r" || amode == "a" )
		    decl = decl basetype[atype] # pick up the base type
		else
		    decl = decl stacktype[atype]


		if ( i >= firstarg )
		    decl = decl "; "
		else
		    decl = decl ")"
	    }

# BUILD DOCUMENTATION LINE FOR FORMAL ARGS
	    a = 1
	    for ( i = firstarg; i <= lastarg; )
	    {
		if ( i == firstarg )
		    doc = doc "("

		trip = $(i)		# gather consecutive args of same type

		for (j = i+1; j <= lastarg; j++) {
		    if ($j != trip)
			break
		}

		atype = substr(trip, 1, 1)
		amode = substr(trip, 3, 1)
		
		if ( amode == "r" || amode == "a" )
		    doc = doc "var "

		while ( i < j )		# kick out formal arg names
		{
		    doc = doc substr(argname, a, 1)
		    i++		# yes, we're really incrementing the loop var
		    a++
		    if ( i < j )
			doc = doc ", "
		    else
			doc = doc ": "  # terminate this set of args
		}

		doc = doc basetype[atype] # pick up the base type

		if ( i <= lastarg )
		    doc = doc "; "
		else
		    doc = doc ")"
	    }
# DONE W/ DOCUMENTATION LINE FOR FORMAL ARGS

	    if ( rettype != "V" )
	    {
# don't distinguish between returning an array and a value: we don't
# ever return arrays.
#		if ( retmode == "a" )
#		    dtype = basetype[rettype]
#		else
		    dtype = stacktype[rettype]

		decl = decl ": " dtype
		doc = doc ": " dtype
	    }

	    decl = decl "; cexternal;"
	    doc = doc ";"
	    printf("%s\n", decl) >procf
	    printf("%s\n\n", doc) >procd

# print macro

	    fmac = "_" funcname
	    imac = "# define " funcname

	    if ( na > 0 )
	    {
		imac = imac "("
		fmac = fmac "("
	    }
	    a = 1
	    for ( i = lastarg; i >= firstarg; i-- )
	    {
# the following goo is to find out if a string is around & to handle it
# Also, find out if a character is being passed.  If so, convert it to an
# ord4(x) so that it can be passed as a 32-bit guber on the stack.
		trip = $i
		atype = substr (trip, 1, 1)
		if ( atype == "E" )
			fmac = fmac "G_P2CSTR("
		if ( atype == "C" )
			fmac = fmac "ORD4("

		imac = imac substr(argname, a, 1)
		fmac = fmac substr(argname, i-1, 1)
		if ( atype == "E" || atype == "C") # more string, char detection
			fmac = fmac ")"

# now for craziness associated with arrays

		amode = substr(trip, 3, 1)
		if ( amode == "a" )
		{			# we're dissecting array info
		    if ( split(trip, tripvec, ":") > 1 )
			if ( split(tripvec[2], sizevec, "*") > 1 )
			{
			    j = sizevec[1]
			    if ( j == "2" || j == "3" || j == "4" )
				fmac = fmac "[0,0]"
			}
			else
			    fmac = fmac "[0]"
		}

		if ( i != firstarg )
		{
		    imac = imac ", "
		    fmac = fmac ", "
		}
		else
		{
		    imac = imac ")"
		    fmac = fmac ")"
		}
		a++
	    }
	    printf("%s %s\n", imac, fmac) >macf
	}

# cleanup
END	{
	    print "DONE"
	}
