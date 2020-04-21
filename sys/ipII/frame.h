/*
** 68020 stack frame as setup by locore.
** The are 7 different exeception frames each with a different amount of
** information on the stack. The frame structure is organized to make
** accessing the short and long bus and address error frames easier.
** All exception frames are the same upto and including the fr_vecoff
** field.
**
** $Source: /d2/3.7/src/sys/ipII/RCS/frame.h,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:30:46 $
*/

/*
** information particular to the short exception frame format
*/
struct sbeframe
{
	ushort	sfr_intern3[ 2 ];/* internal register		*/
};

/*
** information particular to the long exception frame format
*/
struct lbeframe
{
	ushort	lfr_intern3[ 4 ];	/* internal registers	*/
	ulong	lfr_sBaddr;		/* stage B address	*/
	ushort	lfr_intern4[ 2 ];	/* internal register	*/
	ulong	lfr_dib;		/* data input buffer	*/
	ushort	lfr_intern5[ 22 ];	/* internal registers	*/
};

/* SHORT ALIGNED STRUCT */
struct	frame
{
	/*
	** This section is organized to make access into u.u_ar0[reg] easy
	** for all registers, including the PC and SR
	*/
	long	fr_regs[ 16 ];	/* all user registers...		*/
	long	fr_intrhandler;	/* holds interrupt handler address	*/
	long	fr_sr;		/* status register. Padded to a long by	*/
				/*  locore assist.			*/
	long	fr_pc;		/* pc					*/
	ushort	fr_vecoff;	/* vector offset			*/
	ushort	fr_intern1;	/* internal register		*/
	ushort	fr_ssw;		/* special status word		*/
	ushort	fr_ipsC;	/* instruction pipe stage C	*/
	ushort	fr_ipsB;	/* instruction pipe stage B	*/
	long	fr_dcfa;	/* data cycle fault address	*/
	ushort	fr_intern2[ 2 ];
	ulong	fr_dob;
	/*
	** The remaining info is used to map the short and long bus
	** error frames in a convient fashion.  Some info from other
	** exception formats may overlap.
	*/
	union
	{
		struct sbeframe	sbefr;
		struct lbeframe	lbefr;
	} fr_beframe;
};

/* stack frame types (high nibble) */
#define	VECOFF_SHORT		0x0000
#define	VECOFF_THROWAWAY	0x1000
#define	VECOFF_EXCEPTION	0x2000
#define	VECOFF_BUSFAULT		0x8000
#define	VECOFF_COPROCCESOR	0x9000
#define	VECOFF_SHORTBUSFAULT	0xA000
#define	VECOFF_LONGBUSFAULT	0xB000

/* mask for detecting all bus error frames */
#define	VECOFF_BIGFRAME		0x8000

/* special vecoffset used by trap code (also see locore.c/backtouser) */
#define	VECOFF_MUNGE		0x4000
