/* @(#)$Header: /d2/3.7/src/usr.bin/print/lp.spool/drivers/RCS/vcmd.h,v 1.1 89/03/27 18:16:34 root Exp $ */
/*
 * $Log:	vcmd.h,v $
 * Revision 1.1  89/03/27  18:16:34  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  86/11/17  19:54:15  root
 * Initial revision
 * 
 */

#define	VLF		0001	/* not in 4.2bsd VAX */
#define	VFF		0002	/* not in 4.2bsd VAX */
#define	VEOT		0004	/* not in 4.2bsd VAX */

#define	VPRINT		0100
#define	VPLOT		0200
#define	VPRINTPLOT	0400

#define	VGETSTATE	0
#define	VSETSTATE	1
