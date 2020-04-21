#include "duart.h"

/*
	At init:
		set OPCR, ACR, IMR
 */

duart *dad[] = {
	(duart *)(D0A + 0*DINCR),
	(duart *)(D0A + 1*DINCR),
	(duart *)(D1A + 0*DINCR),
	(duart *)(D1A + 1*DINCR)
};

/* CSR baud rates -- high 4 bits, receive, low 4 bits, xmit */
#define	CSRDflt 0xBB		/* 9600 default */

/* OPCR bits, different between 2 duarts!!! */
char	opcr[4] = { 0xF4, 0xF4, 0x00, 0x00 };

/*
 * dinit	- initalize the duart (BOTH CHANNELS)
 */
dinit(n)
register n;
{
	register duart *dp;
	register i;

	n &= ~1;			/* Force even */
	dp = dad[n];
					/* Set common chip bits */
	dp->d_opcr = opcr[n];		/* Set Output Port bits */
	dp->d_acr  = ACR;		/* Set Aux Control bits */
	dp->d_imr  = IMR;		/* Set Interrupt mask */

	duart_port_dfl(dp);		/* Setup Port A */

	duart_port_dfl(dad[n+1]);	/* Setup Port B */
}

duart_port_dfl(dp)
	register duart *dp;
{
	dp->d_mr  = M1Dflt;		/* Always gets these bits */
	dp->d_mr  = M2Dflt;		/*  ditto */
	dp->d_cr  = CRRMR;		/* Reset MR to MR1 */
	dp->d_cr  = CRRR;		/* Reset Rcvr */
	dp->d_cr  = CRRT;		/* Reset Txmt */
	dp->d_cr  = CRRES;		/* Reset Error Status */
	dp->d_csr = CSRDflt;		/* Set speed to 9600 */
	dp->d_cr  = CRER|CRET;		/* Enable Receiver & Xmitter */
}



char du_speedbits[] =
{
    BAUD300,BAUD600,BAUD1200,BAUD2400,BAUD4800,BAUD9600,BAUD19200,BAUD300
};
short du_speeds[] =
{
        300,    600,    1200,    2400,    4800,    9600,    19200,0
};

setbaud(port,rate)
    int port;
    int rate;
{
    register int speedno;

    for( speedno = 0; du_speeds[speedno] != 0; speedno++ )
	if( du_speeds[speedno] == rate )
	    break;
    dad[port]->d_csr = du_speedbits[speedno];
}

modem(port,flag)
    int port;
    int flag;
{
    register duart *dp;

    dp = dad[port&~01];
    dp->d_opcr = opcr[port];
    if( flag )
	dp->d_sopbc = (SOPDS|SOPDT)<<(port&01);
    else
	dp->d_ropbc = (ROPDS|ROPDT)<<(port&01);
}
