#include "duart.h"

/*
 * get from microsw keyboard.
 */

int
getkbd() 
{
	register int c;

	while( (c = nwgetkbd()) == NOCHAR )
	    ;
	return(c);
}

int
nwgetkbd() 
{
	register int c;

	if ((dad[SCREEN]->d_sr & SRRR) == 0) return NOCHAR;

	c = (unsigned char)dad[SCREEN]->d_rhr;

	return dutranslate(c);
}
