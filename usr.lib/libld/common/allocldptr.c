/*	@(#)allocldptr.c	2.1	*/
#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"lddef.h"

LDLIST	*_ldhead = NULL;


LDFILE *
allocldptr( )

{
    extern char		*calloc( );

    extern LDLIST	*_ldhead;

    LDLIST		*ldptr,
			*ldindx;

    if ((ldptr = (LDLIST *) calloc(1, LDLSZ)) == NULL) {
	return(NULL);
    }

    ldptr->ld_next = NULL;

    if (_ldhead == NULL) {
	_ldhead = ldptr;
    } else {
	for (ldindx=_ldhead; ldindx->ld_next != NULL; ldindx=ldindx->ld_next);
	ldindx->ld_next = ldptr;
    }

    return((LDFILE *) ldptr);
}
