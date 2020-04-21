/*
 * Bitmap routines for the efs
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_bit.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:52 $
 */
#include "efs.h"
#if NEFS > 0

#include "../h/types.h"

#undef	SLOWBIT

#ifdef	SLOWBIT
/*
 * bset:
 *	- set bit b in bitmap bp
 */
bset(bp, b)
	bitmap *bp;
	long b;
{
	*(bp + (b >> 3)) |= 1 << (b & 7);
}

/*
 * bclr:
 *	- clear bit b in bitmap bp
 */
bclr(bp, b)
	bitmap *bp;
	long b;
{
	*(bp + (b >> 3)) &= ~(1 << (b & 7));
}

/*
 * btst:
 *	- test bit b in bitmap bp
 */
int
btst(bp, b)
	bitmap *bp;
	long b;
{
	return *(bp + (b >> 3)) & (1 << (b & 7));
}

/*
 * bfset:
 *	- set a bit field of length len in bitmap bp starting at b
 */
bfset(bp, b, len)
	register bitmap *bp;
	register int b;
	register int len;
{
	while (len--) {
		*(bp + (b >> 3)) |= 1 << (b & 7);
		b++;
	}
}

/*
 * bfclr:
 *	- clear a bit field of length len in bitmap bp starting at b
 */
bfclr(bp, b, len)
	register bitmap *bp;
	register long b, len;
{
	while (len--) {
		*(bp + (b >> 3)) &= ~(1 << (b & 7));
		b++;
	}
}

/*
 * bftstset:
 *	- test a bit field of length len in bitmap bp starting at b
 *	  to be all set
 *	- return a count of the number of set bits found
 */
bftstset(bp, b, len)
	register bitmap *bp;
	register long b, len;
{
	register long count;

	count = 0;
	while (len--) {
		if ( ! (*(bp + (b >> 3)) & (1 << (b & 7))) )
			break;
		b++;
		count++;
	}
	return (count);
}

/*
 * bftstclr:
 *	- test a bit field of length len in bitmap bp starting at b
 *	  to be all clear
 *	- return a count of the number of clear bits found
 */
bftstclr(bp, b, len)
	register bitmap *bp;
	register long b, len;
{
	register long count;

	count = 0;
	while (len--) {
		if ( (*(bp + (b >> 3)) & (1 << (b & 7))) )
			break;
		b++;
		count++;
	}
	return (count);
}

#endif	SLOWBIT

#endif
