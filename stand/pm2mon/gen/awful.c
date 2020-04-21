# include "pmIImacros.h"

# define PROMSTATIC

# define AWFUL_SLOP	512
PROMSTATIC	char *awful_ptr;

/*
 * awful_alloc() --
 * "allocate" an area from the bottom of the
 * current stack page.  don't YOU try it.
 */
char *
awful_alloc(size)
    unsigned size;
{
    auto char crud;
    register char *cp,*op;

    crud = 1;

    if( (op = awful_ptr) == 0 )
    {
	op = &crud;
	op = (char *)ptoa(atop((long)op));
    }

    cp = op + size;
    if( cp + AWFUL_SLOP >= &crud )
	return 0;

    awful_ptr = cp;
    return op;
}
