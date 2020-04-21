/*
* $Source: /d2/3.7/src/stand/lib/dev/RCS/subr.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:14:54 $
*/

#include "ctype.h"

/*
** aton
**  convert ascii to a number using supplied base
*/
long
aton( cptr, base )
register char	*cptr;
int		base;
{
	register long	v;
	register char	c;

	v = 0L;

	while ( c = *cptr++ )
	{
		if ( base == 16 && ( c >= 'A' && c <= 'F' ) )
			c = 10 + ( c - 'A' );
		else
		if ( base == 16 && ( c >= 'a' && c <= 'f' ) )
			c = 10 + ( c - 'a' );
		else
		if ( isdigit( c ) )
			c -= '0';
		else
			return -1L;

		if ( base == 16 )
			v = ( v << 4 ) + c;
		else
			v = ( v * 10L) + c;

	}
	return v;
}
