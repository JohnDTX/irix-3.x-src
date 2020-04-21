/*
 * Find the first set bit (least signifigant 1 bit):
 *	mask	lsb #
 *	----	-----
 *	  0	 -1
 *	  1	  1
 *	  2	  2
 *	  3	  1
 *	  4	  3
 * etc.
 *
 * $Source: /d2/3.7/src/bsd/usr.lib/libbsd/common/gen/RCS/ffs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:00:59 $
 */

int
ffs(mask)
	register unsigned int mask;
{
	register int i;

	if ( ! mask )
		return -1;
	i = 1;
	while (! (mask & 1)) {
		i++;
		mask = mask >> 1;
	}
	return i;
}
