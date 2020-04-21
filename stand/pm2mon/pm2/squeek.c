# include "pmII.h"
# include "Qglobals.h"
# include "IrisConf.h"

# define DEBUG do_debug

squeek() 
{
	register short s;
	
	/* satisfy a mouse interrupt */
	s = (unsigned short)*MOUSE;
	s = (unsigned short)*MBUT;

	/* and turn off mailbox interrupts, just in case */
	STATUS_REG &= ~INT_EN;

# ifdef DEBUG
	if( !(do_debug&0x80) )
# endif DEBUG
	if (VERBOSE(switches))
		printf("\7\n<squ%c%c%ck!>\n"
			,s&04?'E':'e',s&02?'E':'e',s&01?'E':'e');
}
