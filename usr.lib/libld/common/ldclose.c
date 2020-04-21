/*	@(#)ldclose.c	2.1	*/
#include    <stdio.h>
#include    <ar.h>
#include    "filehdr.h"
#include    "ldfcn.h"

int
ldclose(ldptr)

LDFILE    *ldptr; 

{
    extern int		fseek( );
    extern int		fread( );

    extern long		sgetl( );

    extern 		fclose( );

    extern int		vldldptr( );
    extern int		freeldptr( );

    if (vldldptr(ldptr) == SUCCESS) {
	if (TYPE(ldptr) == ARTYPE) {
	    {
		struct arf_hdr 	arhdr;
		long ar_size;

	       if (FSEEK(ldptr, -((long) sizeof(struct arf_hdr)), BEGINNING)
					== OKFSEEK) {
		    if (FREAD(&arhdr, sizeof(struct arf_hdr), 1, ldptr) == 1) {
			ar_size = sgetl(arhdr.arf_size);
			OFFSET(ldptr) = OFFSET(ldptr) + ar_size
				      + sizeof(struct arf_hdr) + (ar_size & 01);
					/* (to make OFFSET even) */
			if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK) {
			    if (FREAD(&(HEADER(ldptr)), FILHSZ, 1,
				ldptr) == 1) {
				return(FAILURE);
			    }
			}
		    }
		}

	    }
    	}

	fclose(IOPTR(ldptr));
	freeldptr(ldptr);
    }

    return(SUCCESS);

}
