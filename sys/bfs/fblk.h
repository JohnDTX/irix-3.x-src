#ifndef	__FBLK__
#define	__FBLK__

/* @(#)fblk.h	1.1 */
struct	fblk
{
	int	df_nfree;
	/* short	df_nfree; */
	daddr_t	df_free[NICFREE];
};

#endif	__FBLK__
