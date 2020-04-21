#
/*
 * mbmalloc() --
 * allocate multibus-visible memory.
 */
# include "pmIImacros.h"


extern long 	MBMemSize;
extern char	*MBMemArea;	/* lowest VA to allocate */
extern long	LocalMemSize;

char *mbm_ptr;
long mbm_count;

char *
mbmalloc(nbytes)
    register unsigned int nbytes;
{
    /* first time through, initialize */
    if( mbm_ptr == 0 )
    {
	if( LocalMemSize == 0 )	/* fail until vars have been set up! */
	    return 0;

	mbm_count = MBMemSize;
	mbm_ptr = MBMemArea+MBMemSize;
    }

    nbytes += sizeof (long)-1;
    nbytes -= nbytes % sizeof (long);

    if( nbytes > mbm_count )
	return 0;

    mbm_count -= nbytes;
    mbm_ptr -= nbytes;
    bzero(mbm_ptr, nbytes);

    return mbm_ptr;
}

# ifdef notdef
mbmfree(ptr, nbytes)
    char *ptr;
    int nbytes;
{
    nbytes += sizeof (long)-1;
    nbytes -= nbytes % sizeof (long);
    if( mbm_ptr + nbytes == ptr )
	mbm_ptr += nbytes , mbm_count += nbytes;
}
# endif notdef
