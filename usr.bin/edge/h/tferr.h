/*
 * Text frame error codes.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/h/RCS/tferr.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:07 $
 */

#define	TFERR_NOMEM	1	/* out memory */
#define	TFERR_BADROW	2	/* bad row # */
#define	TFERR_BADCOL	3	/* bad column # */
#define	TFERR_BADFONT	4	/* font file is not a font file */

extern	int	tferrno;
