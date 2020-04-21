/*
 * 68010 stack frame as setup by locore.  The actual frame may be
 * shorter than shown if the fault is not a bus or address error
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/frame.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:36 $
 */

#ifndef	LOCORE
/* SHORT ALIGNED STRUCT */
struct	frame {
	/*
	 * This section is organized to make access into u.u_ar0[reg] easy
	 * for all registers, including the PC and SR
	 */
	long	regs[16];	/* all user registers... */
	long	introutine;	/* longword filler for interrupts */
	long	sr;		/* (really a short, zero padded by locore) */
	long	pc;		/* pc at time of fault */

	/*
	 * The vector offset is always pushed onto the stack and is used
	 * by trap() to figure out which kind of trap occured
	 */
	ushort	vecoffset;	/* vector offset */

	/*
	 * For bus errors and address errors, you get this stuff.
	 */
	ushort	faultstatus;	/* fault status (includes function code) */
	long	aaddr;		/* fault address */
	short	dummy1;		/* UNUSED, RESERVED */
	ushort	dataobuf;	/* data output buffer */
	short	dummy2;		/* UNUSED, RESERVED */
	ushort	dataibuf;	/* data intput buffer */
	short	dummy3;		/* UNUSED, RESERVED */
	ushort	insibuf;	/* instruction input buffer */
	short	internal[16];	/* internal infomration; 16 words */
};
#endif

#define	VECOFF_BIGFRAME		0x8000
#define	VECOFF_MUNGE		0x7fff
