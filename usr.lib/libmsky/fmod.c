/*	@(#)fmod.c	1.5	*/
/*LINTLIBRARY*/

#include <fpregs.h>
#include <fpopcodes.h> 

float
fmod(x, y)
float x, y;
{

	if (y == 0.0)
		return (x);
	*SKYCOMREG = HW_MOD;
	*SKYFLREG = x;
	*SKYFLREG = y;
	while (*SKYSTATREG > 0) ;
	return(*SKYFLREG);

}


long float
_lfmod(x, y)
long float x, y;
{
	extern long float _lmodf();
	long float d;

	if (y == 0.0)
		return (x);
#ifndef	u3b
#ifndef	pdp11
	(void)
	/* This MUST be fixed for 5.0!  It is intended to be very temporary! */
#endif
#endif
	_lmodf(x/y, &d);
	return (x - d * y);

}
