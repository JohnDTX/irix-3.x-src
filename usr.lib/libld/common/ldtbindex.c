/*	@(#)ldtbindex.c	2.1	*/
#include	<stdio.h>
#include	"filehdr.h"
#include	"syms.h"
#include	"ldfcn.h"

long
ldtbindex(ldptr)

LDFILE	*ldptr;

{
    extern long		ftell( );

    extern int		vldldptr( );

    long		position;


    if (vldldptr(ldptr) == SUCCESS) {
	if ((position = FTELL(ldptr) - OFFSET(ldptr) - HEADER(ldptr).f_symptr) 
	    >= 0) {

	    if ((position % SYMESZ) == 0) {
		return(position / SYMESZ);
	    }
	}
    }

    return(BADINDEX);
}
