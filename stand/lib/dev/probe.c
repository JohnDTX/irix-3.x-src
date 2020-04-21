/*
* $Source: /d2/3.7/src/stand/lib/dev/RCS/probe.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:14:45 $
*/

#include	"sys/types.h"
#include	"cpureg.h"
#include	"common.h"
#include	"setjmp.h"
#include	"fpa.h"

/*
** probe
**   test and see if the 'addr' of size 'len' exists.
**   'len' of char, short only supported.
*/
probe( adr, len )
u_long	adr;
{
	jmp_buf	jb;
	int	*saved_jb;
	short	dummy;

	/* turn on quick timeouts	*/
	*STATUS_REG |= ST_EQKTIMO;

	saved_jb = _commdat->c_nofault;
	_commdat->c_nofault = jb;

	if ( setjmp( jb ) == 0 )
	{
	    /* go probe the address */
	    if ( len == sizeof (char) )
		    dummy = *(char *)adr;
	    else
		dummy = *(short *)adr;
	    dummy = 1;
	} else {
	    /* a mean and nasty bus error happened */
	    dummy = 0;
	}
	_commdat->c_nofault = saved_jb;

	/* quick timeouts off */
	*STATUS_REG &= ~ST_EQKTIMO;

	return ( (long)dummy );
}

/*
** probe by writing
**   test and see if the 'addr' of size 'len' exists.
**   'len' of char, short only supported.
*/
probewrite( adr, len, value)
u_long	adr;
u_long	value;
{
	jmp_buf	jb;
	int	*saved_jb;
	short	dummy;

	/* turn on quick timeouts	*/
	*STATUS_REG |= ST_EQKTIMO;

	saved_jb = _commdat->c_nofault;
	_commdat->c_nofault = jb;

	if ( setjmp( jb ) == 0 )
	{
	    /* go probe the address */
	    if ( len == sizeof (char) )
		    *(char *)adr = (char)value;
	    else
		*(short *)adr = (short)value;
	    dummy = 1;
	} else {
	    /* a mean and nasty bus error happened */
	    dummy = 0;
	}
	_commdat->c_nofault = saved_jb;

	/* quick timeouts off */
	*STATUS_REG &= ~ST_EQKTIMO;

	return ( (long)dummy );
}

/*
 * See if the FPA is present.  We do this in the ROMS because the FPA is
 * a memory device and if we timeout on it refresh can get hosed.  We will
 * report to unix if the FPA is there so it doesn't have to probe
 */
fpaprobe()
{

	jmp_buf	jb;
	int	*saved_jb;
	short	dummy;
	register u_short status;

	/* turn on quick timeouts and make sure we are not in system mode */
	status = *STATUS_REG;
	*STATUS_REG = status | ST_EQKTIMO | ST_SYSSEG_; 

	saved_jb = _commdat->c_nofault;
	_commdat->c_nofault = jb;

	if ( setjmp( jb ) == 0 )
	{
	    /* go probe the fpa */
	    fpaopcodeB( FPA_EREG, FPA_FILLER, FPA_FILLER ) = 0xFF;
	    dummy = 1;
	} else {
	    /* a mean and nasty bus error happened */
	    dummy = 0;
	}
	_commdat->c_nofault = saved_jb;

	/* quick timeouts off */
	*STATUS_REG = status;

	return ( (int)dummy );
}
