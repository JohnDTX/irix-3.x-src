/*		All of the primatives for booting			*/

#include "pmII.h"
#include "Qglobals.h"
typedef unsigned char u_char;
#include "i488.h"

#define MAXBUZZ		0x80000
#define MIO_OFFSET	0x100

struct ni488device *Nip;
int i488present;
int Timeout;


# undef DEBUG do_debug
# include "dprintf.h"


init488(primaddr)
{
	register struct ni488device *ni;
	register int time;


	Nip = (struct ni488device *)mbiotov(MIO_OFFSET);
	ni = Nip;
	Timeout = MAXBUZZ;

	dprintf(("init488:ni is 0x%x, Timeout is 0x%x\n",ni,Timeout));

	/* poke the board	*/
	if ( probe(ni,1) )
		i488present = 1;
	else {
		i488present = 0;
		return( -1 );
	}

	/* Initialize the hardware */
	ni->ni_sr_cr0 = CR0_LMRESET;
	ni->ni_sr_cr0 = 0;
	ni->ni_cptr_auxmr = AC_CRESET;
	ni->ni_cptr_auxmr = AUXICR_MAGIC;
	ni->ni_cptr_auxmr = AUXPPR_UNCONFIG;
	ni->ni_cptr_auxmr = AUXB_INV | AUXB_TRI;

	/* now set up controller to be device number X and go into the 
		acceptor idle state */
	ni->ni_adr0_adr = primaddr;
	ni->ni_adr0_adr = ADR_SEL2 | ADR_DISTALK | ADR_DISLISTEN;
	ni->ni_adsr_admr = ADMR_NORMAL | ADMR_TRMMAGIC;
	ni->ni_isr1_imr1 = 0;
	ni->ni_isr2_imr2 = 0;
	ni->ni_cptr_auxmr = AC_PON;
	/* dont cares: eosr, auxre, auxa, spmr		*/
	return( 1 );
}

read488(buffer,count)
char *buffer;
int count;
{
	register struct ni488device *ni = Nip;
	register char *bp;
	register time;
	register cnt;
	register unsigned char isr1;

	dprintf(("read488\n"));

	bp = buffer;
	cnt = count;
	while ( cnt ) {
		time = Timeout;
		while ( time-- ) {
			if ( (isr1=ni->ni_isr1_imr1) & I1_DI ) {
				*bp++ = ni->ni_dir_cdor;
				cnt--;
				break;
			}
		}
		if ( time == -1 ) {
dprintf(("read488:timeout after %d chars, isr1 is 0x%x\n"
,count-cnt,isr1));
dprintf(("sr 0x%x, adsr 0x%x, isr2 0x%x, isr1 0x%x\n"
,ni->ni_sr_cr0,ni->ni_adsr_admr,ni->ni_isr2_imr2,ni->ni_isr1_imr1));
			return ( -1 );
		}
		if ( isr1 & I1_ENDIE )
			break;
	}
dprintf(("read488: returning %d, 1st long is 0x%x\n"
,count-cnt,*(long *)buffer));
	return ( count - cnt );
}

write488(buffer,count)
char *buffer;
int count;
{
	register struct ni488device *ni = Nip;
	register char *bp;
	register time;
	register cnt;

	dprintf(("write488\n"));

	bp = buffer;
	cnt = count;
	while ( cnt ) {
		time = Timeout;
		if ( cnt == 1 )
			ni->ni_cptr_auxmr = AC_SENDEOI;
		while ( time-- ) {
			if ( ni->ni_isr1_imr1 & I1_DO ) {
				ni->ni_dir_cdor = *bp++;
				cnt--;
				break;
			}
		}
		if ( time == -1 ) {
dprintf(("write488:timeout after %d chars\n",count-cnt));
dprintf(("sr 0x%x, adsr 0x%x, isr2 0x%x, isr1 0x%x\n"
,ni->ni_sr_cr0,ni->ni_adsr_admr,ni->ni_isr2_imr2,ni->ni_isr1_imr1));
			return ( -1 );
		}
	}
	return ( count - cnt );
}

srq488(pollchar)
unsigned char pollchar;
{
	register struct ni488device *ni = Nip;

	dprintf(("srq488\n"));
	ni->ni_spsr_spmr = pollchar | SP_REQ;
	dprintf(("srq488: spsr is 0x%x\n",ni->ni_spsr_spmr));
	return 0;
}

/* check if serial poll has responded.  0 - if timeout   1 - if poll occured */
polled488()
{
	register struct ni488device *ni = Nip;
	register time;

	dprintf(("polled488\n"));
	dprintf(("polled488: spsr is 0x%x\n",ni->ni_spsr_spmr));
	time = Timeout*2;
	while ( time-- ) {
		if ( !(ni->ni_spsr_spmr & SP_PEND) )
			return(1);
	}
	ni->ni_spsr_spmr = 0;
	return(0);
}

/* wait for device clear to come */
wait488dec()
{
	register struct ni488device *ni = Nip;

if( !i488present ) { printf("No GPIB!\n"); msdelay(30000); return; }
	printf("Waiting for device clear\n");
	while( !(ni->ni_isr1_imr1 & I1_DEC) )
		;
}
