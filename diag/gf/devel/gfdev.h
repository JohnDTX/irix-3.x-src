/* gfdevel.h  -- definitions for Geometry System and Frame Buffer Control
 *		Multibus interfaces
 *	DEVELOPMENT SYSTEM VERSION
 *		GE registers are on GF board; FBC registers are on alpha FBC
 */



#ifndef GFMBM
#define GFMBMEMLOWADR	0x100000		/* was in pcmap.h */
#define GFMBM(x)	(GFMBMEMLOWADR + (x))
#define GFIOLOWADR	0x1f0000		/* was in pcmap.h */
#define GFMBI(x)	(GFIOLOWADR + (x))
#endif GFMBM

#ifndef GEPORT
#include "pm1.h"
#endif

/*================================================================*/
/* board addressing	  					  */
/*================================================================*/

#define FBCDEV		(0xC8000)		/* 0x1e8000 */

#define GEDEV		(0x1000)		/* 0x1f1000 */

/*================================================================*/
	/* on-board devices */
/*================================================================*/

#define READPIXEL	0	/* also FBCLRINT */

#define FBCTL		(2<<13)
#define FBDATA		(3<<13)
#define FBUCODE		(1<<13)
#define UCODE(x)	((x)<<3)

#define GECTL		(3<<10)
/* GEDATA				/*undefined*/

/*================================================================*/
/* addressing constants	*/
/*================================================================*/

#define GEdata		GEPORT
#define GEflags		*(unsigned short*)(GFMBI(GEDEV|GECTL))

#define FBCflags	*(unsigned short*)(GFMBM(FBCDEV|FBCTL))
#define FBCdata		*(unsigned short*)(GFMBM(FBCDEV|FBDATA))
#define FBCpixel	*(unsigned short*)(GFMBM(FBCDEV|READPIXEL))

#define FBCclrint	FBCpixel = 1	/* statement to clear FBC interrupt */

#define FBCmicro(x,y)	*(unsigned short*)(GFMBM(FBCDEV|FBUCODE|UCODE(x)|(y+y)))

/*================================================================*/
/* GEdata --  flags read	(GF1 board)
/*================================================================*/

#define LOWATER_BIT	0x1	/* input fifo low-water  */
#define TRAPINT_BIT	0x2	/* GE instruction trap interrupt  */
#define FIFOINT_BIT	0x4	/* input fifo interrupt (hi/lo water)  */
#define GETRAP_BIT(n)	(1<<(n+2))	/* GE instruction trap bits
					/* GE's are numbered 1-12;
					/* note inversion
					 */
#define HIWATER_BIT	0x8000	/* input fifo high-water  */

/*================================================================*/
/* GE flags written	(GF1 board)
/*================================================================*/

#define GERESET_BIT_		1
#define SUBSTBPCCODE_BIT_	0x2	/* BPC cmd code 0-3 <= DI bus   */
#define ENABFIFOINT_BIT_	0x4	/* enable input fifo interrupts */
#define ENABTRAPINT_BIT_	0x8	/* enable GE trap interrupts	*/
#define STATUSLED_BIT		0x10
/* note ... ENABVERTINT_BIT	0x20 ... no longer meaningful here.  */
	/* for upward compatibility:		*/
#define ENABINPINT_BIT		0x4
#define GERESET1		0x2f	/* standard reset sequence	*/
#define GERESET2		0x26
#define GERESET3		0x26	/* (trap interrupts enabled)	*/
#define GEDEBUG			0x2e	/* (		    disabled)	*/

/*================================================================*/
/* FBC flags read  (trailing _ indicates low-active)  */
/*================================================================*/

#define VERTINT_BIT_	0x80	/* vertical interval	*/
#define NEWVERT_BIT_	0x40	/* vertical interrupt	*/
#define ENABDOFBD_BIT	0x20	/* enable DO bus (not maint read)  */
#define INTERRUPT_BIT_	0x10
#define BPCACK_BIT_	8
#define FBCREQ_BIT_	4
#define FBCACK_BIT_	2
#define GEREQ_BIT_	1

/*================================================================*/
/* FBC flags written */
/*================================================================*/

#define FLOAT		0
#define RUNMODE		0x41
#define SUBSTIN		2
#define HOSTFLAG	4
#define MAINT_BIT	8
#define SUBST_BIT	0x10
#define FORCEREQ_BIT_	0x20
#define FORCEACK_BIT_	0x40	/* also substbpccode_bit_ */
#define ENABVERTINT_BIT_ 0x80
#define ENABVERT_SHIFTED_ 0x8000
#define SUBSTIN_BIT	SUBST_BIT

/*================================================================*/
	/* FBC flag combinations */
/*================================================================*/

#define FLOAT		0
#define READOUTRUN	0xC2	/* spy on output reg w/o disturbing runmode */
#define READOUT		0xC2
#define RUNDEBUG	0xF3	/* run substituted  - debug mode */
#define RUNDEBUGF	0xF7	/* debug mode w/ host flag  */
#define RUNSUBST	0xB3	/* RUNDEBUG with substbpccode */
#define READCODERUN	0xF6	/* spy on fbccode/BPC bus  */
#define READMICRO	0xFA
#define WRITEMICRO	0xFB
#define CYCINDEBUG	0xD3	/* use in debug mode */
#define CYCOUTDEBUG	0xB3	/* use in debug mode */
#define CYCFLOATBLOCKED	0x93
#define STARTDEV	0x90	/* start machine in debug mode */

/*================================================================*/
	/* version-independent enables & tests */
/*================================================================*/

	/* to these macros pass the rest of the flags desired */

#define MakeGFStatus(ge,fbc,new) \
		new = ((fbc)<<8) | (ge) ;

#define Enabvert(stat) \
		{ stat &= ~ENABVERT_SHIFTED_; FBCflags = (stat)>>8; }
#define Disabvert(stat) \
		{ stat |= ENABVERT_SHIFTED_; FBCflags = (stat)>>8; }
#define FBCenabvert(fl)		FBCflags = (fl) & ~ENABVERTINT_BIT_
#define FBCdisabvert(fl)	FBCflags = (fl) | ENABVERTINT_BIT_
	/* try not to use FBCxxxvert !	*/
#define GFenabvert(geflags,fbcflags) \
		FBCenabvert(fbcflags); GEflags = (geflags)
#define GFdisabvert(geflags,fbcflags) \
		FBCdisabvert(fbcflags); GEflags = (geflags)
