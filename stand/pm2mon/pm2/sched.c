# include "Qdevices.h"
# include "Qglobals.h"



/*
**	ready - see what resources are available (not busy)
**
*/
ready( waitvector )
int waitvector;
{
    /* Check all Uart Tx Regs and set status bits in the qreadyvector */
    CheckTxRdy();
    return (Qreadyvector & waitvector);
}

/*
**	wait - wait for resources to become available
**
*/
wait( waitvector )
int waitvector;
{
    register int n;

    MAKENOTREADY(CLOCKBITS);
    while( !(n=ready(waitvector)) )
	Qupcount++;
    return n;
}
