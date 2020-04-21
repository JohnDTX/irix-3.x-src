# include "setjmp.h"

jmp_buf bejmp;

memwrite(addr,data,len)
register long addr,data;
register int len;
{
	/* write the data at address.  catch bus errors */
	if (setjmp(bejmp)) {
	    /* bus error on write....inform user */
	    printf("BUS ERROR writing %d bytes of %8x at %8x\n ",
		    len,data,addr);
	} else {
	    if (len == 1) *(unsigned char *)addr = (unsigned char)data;
	    else if (len == 2) *(short *)addr = (short)data;
	    else  *(long *)addr = data;
	    /* clear out the jump buffer */
	    *bejmp = 0;
	}
}
