#include "mem.h"

probe(adr,len)
	unsigned long adr;
{	short dummy;
	if (setjmp(bejmp)==0) {
	    /* go probe the address */
	    dummy = (len==1)?*(char *)adr:*(short *)adr;
	    /* no problem */
	    *bejmp = 0;
	    return(1);
	} else {
	    /* bus error happened */
	    return(0);
	}
}

