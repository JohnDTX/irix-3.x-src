# include "setjmp.h"

jmp_buf bejmp;

memread(addr,data,len)
register unsigned long addr,*data;
register int len;
{
	/* read the data at address of length len.  return
	   true if no bus error, false if bus error */

	    if (len != 1)
	        /* make sure of an even address */
		addr = (addr & ~1);
	    
	    if (setjmp(bejmp)) {
	  	/* bus error at read....inform user */
		return(0);
	    } else {
	    /* retrieve the data at addr */
		if (len == 1) *data = *(unsigned char *)addr;
		else if (len == 2) *data = *(unsigned short *)addr;
		else *data = *(unsigned long *)addr;
		*bejmp = 0;
		return(1);
	    }
}
