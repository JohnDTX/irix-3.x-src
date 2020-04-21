/*	@(#)ldnlseek.c	2.1	*/
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

int
ldnlseek(ldptr, sectname)

LDFILE	*ldptr;
char 	*sectname; 

{
	extern	int	ldnshread( );
	extern	int	fseek( );

	SCNHDR	shdr;

	if (ldnshread(ldptr, sectname, &shdr) == SUCCESS) {
		if (shdr.s_nlnno != 0) {
		    if (FSEEK(ldptr, shdr.s_lnnoptr, BEGINNING) == OKFSEEK) {
			    return(SUCCESS);
		    }
		}
	}

	return(FAILURE);
}
