#include "pmII.h"
#include "duart.h"
#include "common.h"

#define MC68010 0x4	/* this bit is set in the type if 68010/20 */

char *typedesc[3] = {"trap","bus error","address error"};

grave_error(typecode,sr,aa,ir,pc)
long typecode,sr,aa,ir,pc;
{
	/* last chance error reporter.  At this point, there
	   is no guarantee that the duarts are running.  We will
	   (for now) take the simplistic approach that if the duart
	   is ready, it is okay. */
	register unsigned errindex;

	checkkbd();

	errindex = typecode&~MC68010;
	if( errindex >= 3 ) {
	    printf("illegal typecode 0x%x\n",typecode);
	    return(0);
	}
	printf("%s - FATAL ERROR: %s\n at PC = 0x%x, SR = 0x%x\n",
		(typecode&MC68010)?"mc68010/20":"mc68000",
		typedesc[errindex],pc,(unsigned short)sr);
	if (aa != (-1)) printf("access address = 0x%x ",aa);
	if (ir != (-1)) printf("IR = 0x%x",(unsigned short)ir);
	printf(
"\nPM2.1 registers:\nException register = %x, Status register = %x\n",
		(unsigned short)EXCEPTION_REG,(unsigned short)STATUS_REG);
	if (~(EXCEPTION_REG) & PARERR) {
		STATUS_REG &= ~PAR_EN;
		printf("PARITY ERROR : disabled\n");
	}
	delayed_reboot();
	return(0);
}

char	current_intlevel;		/* set by startup */
intr(ps, pc)
long ps, pc;
{
	register short junk;
	register unsigned char byte,isr;
	register int i;
	register short intreg;
	if ((current_intlevel & 0xf) != 4) checkkbd();
	printf("\nINTERRUPT level %d:\n",current_intlevel & 0xf);
	intreg = ~(EXCEPTION_REG);
	if (intreg & MBINT) printf("Mouse quad = %x, buttons = %x\n",
		(unsigned short)*MOUSE,(unsigned short)*MBUT);

	if (intreg & EXTINT) {
	    printf("Mailbox interrupt - disabled.\n");
	    STATUS_REG &= ~INT_EN;
	}

	if (intreg & P0INT) {
	    printf("Parallel port receive interrupt\n");
	    junk = *PPport;
	}

	if (intreg & P1INT) {
	    printf("Parallel port transmit interrupt\n");
	    *PPport = junk;
	}

	/* ok - now the messy part - the duarts */
#ifdef NOTDEF
	for (i=0;i<3;i+=2) {
	    if (isr = dad[i]->d_isr) {
	    
	    if (isr & ISIPCS) {
		byte = dad[i]->d_ipcr;
		dad[i]->d_imr &= ~IMRII;
		printf("I/P port change duart %d - disabled\n",i);
	    }
	    if (isr & ISCBB) {
		dad[i]->d_imr &= ~ISCBB;
		printf("DELTA BREAK: port B, duart %d - disabled\n",i);
	    }
	    if (isr & ISCB) {
		dad[i]->d_imr &= ~ISCB;
		printf("DELTA BREAK: port A, duart %d - disabled\n",i);
	    }
	    if (isr & ISRRB) {
		dad[i]->d_imr &= ~ISRRB;
		printf("RxRDY: port B duart %d - disabled\n",i);
	    }
	    if (isr & ISTRB) {
		dad[i]->d_imr &= ~ISTRB;
		printf("TxRDY: port B duart %d - disabled\n",i);
	    }
	    if (isr & ISTR) {
		dad[i]->d_imr &= ~ISTR;
		printf("TxRDY: port A duart %d - disabled\n",i);
	    }
	    if (isr & ISCR) {
		dad[i]->d_imr &= ~ISCR;
		printf("Timer: duart %d - disabled\n",i);
	    }
	    if (isr & ISRR) {
		dad[i]->d_imr &= ~ISRR;
		printf("RxRDY: port A duart %d - disabled\n",i);
	    }
	    
   	    }
	}
#endif
	delayed_reboot();
	return(0);
}
		    

		
checkkbd() {
	msdelay(3);	/*1000*/

	if (!ISCOMMONOK) setkbd();

	if ISMICROSW {
		InScreen();
		/* _commdat->savenblines = _commdat->nblines; */
		/* _commdat->nblines = 5; */
	} else  {
	    if (!(dad[LOCAL]->d_sr & SRTR))  {
	    	dinit(0);
	    	dinit(2);
		msdelay(3);	/*1000*/
    	    	if (!(dad[LOCAL]->d_sr & SRTR))  return(0); 
	    }
	} 
	return 1;
}
