/*	@(#)ldahread.c	2.1	*/
#include    <stdio.h>
#include    <ar.h>
#include    "filehdr.h"
#include    "ldfcn.h"

int
ldahread(ldptr, arhead)

LDFILE    *ldptr;
ARCHDR    *arhead; 

{
    extern int		fseek( );
    extern int		fread( );

    extern long 	sgetl( );

    extern int		vldldptr( );

    struct   arf_hdr arfbuf;

    if ((vldldptr(ldptr) == SUCCESS) && (TYPE(ldptr) == ARTYPE)) { 
	if (FSEEK(ldptr, -((long) sizeof(struct arf_hdr)), BEGINNING)
					== OKFSEEK) {
	    if (FREAD(&arfbuf, sizeof(struct arf_hdr), 1, ldptr) == 1) {
		strncpy(arhead->ar_name, arfbuf.arf_name, sizeof(arhead->ar_name));
		arhead->ar_date = sgetl(arfbuf.arf_date);
		arhead->ar_uid = sgetl(arfbuf.arf_uid);
		arhead->ar_gid = sgetl(arfbuf.arf_gid);
		arhead->ar_mode = sgetl(arfbuf.arf_mode);
		arhead->ar_size = sgetl(arfbuf.arf_size);
	    	return(SUCCESS);
	    }
	}
    }

    return(FAILURE);
}
