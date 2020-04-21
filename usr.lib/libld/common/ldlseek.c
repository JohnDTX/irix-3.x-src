/*	@(#)ldlseek.c	2.1	*/
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

int
ldlseek(ldptr, sectnum)

LDFILE		*ldptr;
unsigned short	sectnum; 

{
	extern	int	ldshread( );
	extern	int	fseek( );

	SCNHDR	shdr;

	if (ldshread(ldptr, sectnum, &shdr) == SUCCESS) {
		if (shdr.s_nlnno != 0) {
		    if (FSEEK(ldptr, shdr.s_lnnoptr, BEGINNING) == OKFSEEK) {
			    return(SUCCESS);
		    }
		}
	}

	return(FAILURE);
}
