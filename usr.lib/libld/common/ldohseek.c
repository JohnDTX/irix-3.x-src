/*	@(#)ldohseek.c	2.1	*/
#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"

int
ldohseek(ldptr)

LDFILE		*ldptr;

{
    extern int		fseek( );

    extern int		vldldptr( );

    if (vldldptr(ldptr) == SUCCESS) {
	if (HEADER(ldptr).f_opthdr != 0) {
	    if (FSEEK(ldptr, (long) FILHSZ, BEGINNING) == OKFSEEK) {
		return(SUCCESS);
	    }
	}
    }

    return(FAILURE);
}
