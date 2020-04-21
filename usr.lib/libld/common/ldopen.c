/*	@(#)ldopen.c	2.1	*/
#include    <stdio.h>
#include    <ar.h>
#include    "filehdr.h"
#include    "ldfcn.h"

LDFILE *
ldopen(filename, ldptr)

char	*filename;
LDFILE	*ldptr; 

{
    /* functions called */
    extern FILE		*fopen( );
    extern		fclose( );
    extern long		sgetl( );
    extern int		fseek( );
    extern int		fread( );

    extern int		vldldptr( );
    extern LDFILE	*allocldptr( );
    extern int		freeldptr( );

    FILE	 	*ioptr;
    unsigned short	 	type;
    struct ar_hdr		arbuf;
    long			nsyms;

    if (vldldptr(ldptr) == FAILURE) {

	if ((ioptr=fopen(filename, "r")) == NULL) {
	    return(NULL);
	}

	if ( fread(&arbuf, sizeof(struct ar_hdr), 1, ioptr) != 1)
			arbuf.ar_magic[0] = '\0';
	fseek(ioptr,0L,0);

	if (fread(&type, sizeof(type), 1, ioptr) != 1) {
	    fclose(ioptr);
	    return(NULL);
	}

	if ((ldptr = allocldptr( )) == NULL) {
	    fclose(ioptr);
	    return(NULL);
	}

	if ( strncmp(arbuf.ar_magic,ARMAG,SARMAG) == 0 ) {
		nsyms = sgetl(arbuf.ar_syms);
		TYPE(ldptr) = (unsigned short) ARTYPE;
		OFFSET(ldptr) = sizeof(struct arf_hdr) + sizeof(struct ar_hdr) +
				(nsyms * sizeof(struct ar_sym));
	} else {
		TYPE(ldptr) = type;
		OFFSET(ldptr) = 0L;
	}
	IOPTR(ldptr) = ioptr;

	if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK) {
	    if (FREAD(&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1) {
		return(ldptr);
	    }
	}
    } else {
	if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK) {
	    return(ldptr);
	}
    }
    fclose(IOPTR(ldptr));
    freeldptr(ldptr);
    return(NULL);
}
