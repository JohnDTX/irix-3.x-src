/*
 *	Kurt Akeley			7/18/82
 *
 *	These macros are for use with the Electronics Solution interface
 *	  board.  They provide a complete interface to the Bit Plane
 *	  Controller, and therefore to the Bit Plane Memory boards.
 *	The complex addressing scheme is required due to faults in the
 *	  cc68 compiler, which generates 16 bit addresses if not
 *	  properly treated, and writes zeros with an assembler instruction
 *	  which results in a bus read as well as a write.
 *
 *	A second set of macros has been added to support testing through
 *	  the GF board.  If TB1 is defined, the test board macros are used.
 *	  If GF1 is defined, the new gf macros are used.  There are no
 *	  "elseif" includes, so GF2 (when it happens) can be added easily.
 *
 *	Updated 9/16/82 KBA	Config bit names included
 *	Updated 5/5/83  KBA	Vertint inverted
 *	Updated 8/28/84 MSG	Development system
 */

#ifdef INTER1
extern short _ucbuffer;	/* declared in interface.c */

#define UCTEST_INIT	/* do nothing here, GF requires init */

#define BASEADDR	((short*)0x112000)
#define LOAD(off,val)	{ _ucbuffer = val; *(BASEADDR + off) = (_ucbuffer); }
#define READ(off)	(*(BASEADDR + off))
#define UCADR(dev,adr)	(((dev)<<2)|(adr)&3)

#define LDCONFIG(x)	LOAD (UCADR (0, 0), x)
#define LDED(x)		LOAD (UCADR (0, 1), x)
#define LDEC(x)		LOAD (UCADR (0, 2), x)
#define LDXS(x)		LOAD (UCADR (0, 3), x)
#define LDXE(x)		LOAD (UCADR (1, 0), x)
#define LDYS(x)		LOAD (UCADR (1, 1), x)
#define LDYE(x)		LOAD (UCADR (1, 2), x)
#define LDFMADDR(x)	LOAD (UCADR (1, 3), x)

#define REQHIGH		(READ (UCADR (2, 3)) & 0x8000)
#define VERTINT		(!(READ (UCADR (2, 3)) & 0x2000))
#define RDPIXEL		(READ (UCADR (3, 2)) & 0xffff)
#define RDFONT		(READ (UCADR (2, 1)) & 0xff)

#define REQUEST(cmnd,x)	{ LOAD (UCADR (2, 0), x);	\
			  LOAD (UCADR (3, 0),cmnd);	\
			  while (REQHIGH);		\
			  }
#endif	/* INTER1 */

/*
 *	Following are GF related macros - to be used with UC4 and the
 *	  alphabeta GF hack
 */

#ifdef INTER4

#include "gfdev.h"
#include <betacodes.h>

#define intlevel7	asm("	orw	#/0700,sr"); \
			asm("	andw	#/F7FF,sr")
#ifdef HUH
#define FORCEREQ	((RUNMODE & ~FORCEACK_BIT_) | SUBSTDI | SUBSTIN_BIT)
				/* forceack = substbpccode */
#define FORCEWAIT	(FORCEREQ | FORCEREQ_BIT_)
#endif

#define FORCEWAIT	RUNSUBST
#define FORCEREQ	(RUNSUBST & ~FORCEREQ_BIT_)
#define FORCEREAD	(READOUTRUN | SUBSTIN_BIT | FORCEREQ_BIT_)

	/* GF1 reset procedure	*/

#define UCTEST_INIT     {  \
			short _ii; \
			intlevel7; \
			GFdisabvert(GEDEBUG,STARTDEV); \
			for (_ii=0; _ii<50; _ii++) ; \
			FBCflags = STARTDEV & ~FORCEREQ_BIT_; \
			FBCflags = STARTDEV & ~FORCEACK_BIT_; \
			FBCflags = STARTDEV; \
			FBCclrint; \
			FBCflags = FORCEWAIT; \
			FBCdata = 8; \
			FBCclrint; \
			FBCclrint; \
			FBCclrint; \
			for (_ii=0; _ii<50; _ii++) ; \
			FBCclrint; \
		     }

	/* mechanism for giving FBC a word of input	*/

#define SEND(x)	{ FBCdata = (x); \
		  FBCflags = FORCEREQ; \
		  FBCflags = FORCEWAIT; \
		}

	/* give the FBC a strobe command		*/

#define LOAD(which,data)	{ SEND(BPCcommand); \
				  SEND(data); \
				  SEND(which) \
				}

	/* BPC strobes	*/

#define LDCONFIG(x)	LOAD( 8,x);
#define LDED(x)		LOAD( 9,x);
#define LDEC(x)		LOAD(10,x);
#define LDXS(x)		LOAD(11,x);
#define LDXE(x)		LOAD(12,x);
#define LDYS(x)		LOAD(13,x);
#define LDYE(x)		LOAD(14,x);
#define LDFMADDR(x)	LOAD(15,x);

	/* BPC command request	*/

#define REQUEST(cmnd,x)	{ SEND(BPCcommand); \
			  SEND(x); \
			  SEND((cmnd)|0x8000); \
			  while (!(FBCflags&FBCREQ_BIT_)); \
			}

#define VERTINT		(FBCflags & VERTINT_BIT)
#undef  RDPIXEL
#define RDPIXEL		FBCpixel
#define RDFONT		(ReadFont () & 0xff)

#endif /* INTER4 */

#ifdef INTER3
/* UC4 has its own interface */
extern short _ucbuffer;	/* declared in interface.c */
#define LOAD(adr,val)	{ _ucbuffer = val; *UCBufferAddr(adr) = _ucbuffer; }
#define LDED(x)		LOAD (UC_EDB, x)
#define LDEC(x)		LOAD (UC_ECB, x)
#define LDXS(x)		LOAD (UC_XSB, x)
#define LDXE(x)		LOAD (UC_XEB, x)
#define LDYS(x)		LOAD (UC_YSB, x)
#define LDYE(x)		LOAD (UC_YEB, x)
#define LDFMADDR(x)	LOAD (UC_FMAB, x)
#define LDCONFIG(x)	LOAD (UC_CFB, x)
#define LDMODE(x)	LOAD (UC_MDB, x)
#define LDREPEAT(x)	LOAD (UC_RPB, x)
#define LDDDASAF(x)	LOAD (UC_DDASAF, x)
#define LDDDASAI(x)	LOAD (UC_DDASAI, x)
#define LDDDAEAF(x)	LOAD (UC_DDAEAF, x)
#define LDDDAEAI(x)	LOAD (UC_DDAEAI, x)
#define LDDDASDF(x)	LOAD (UC_DDASDF, x)
#define LDDDASDI(x)	LOAD (UC_DDASDI, x)
#define LDDDAEDF(x)	LOAD (UC_DDAEDF, x)
#define LDDDAEDI(x)	LOAD (UC_DDAEDI, x)

#define VERTINT		(*UCRAddr & UCR_VERTICAL)
#define RDPIXEL		(*UCCommandAddr(UC_READPIXELAB))
#define RDFONT		(*UCCommandAddr(UC_READFONT))

#define UCWAIT		while ((*UCRAddr) & UCR_BUSY)
#define REQUEST(cmnd,x)	{_ucbuffer=x;*UCCommandAddr(cmnd)=_ucbuffer; UCWAIT;\
			 UCWAIT;}
#define NWREQUEST(cmnd,x) {_ucbuffer=x;*UCCommandAddr(cmnd)=_ucbuffer;}

/* map new command names back into the old name (where applicable) */
#define WRITEFONT	UC_WRITEFONT
#define LOADXYADDR	UC_SETADDRS
#define READWORD	UC_SAVEWORD
#define WRITEWORD	UC_DRAWWORD
#define NOOP		UC_NOOP
#define DRAWCHAR	UC_DRAWCHAR
#define FILLRECT	UC_FILLRECT
#define CLEAR		UC_FILLRECT
#define DRAWLINE1	UC_DRAWLINE1
#define DRAWLINE2	UC_DRAWLINE2
#define DRAWLINE4	UC_DRAWLINE4
#define DRAWLINE5	UC_DRAWLINE5
#define DRAWLINE7	UC_DRAWLINE7
#define DRAWLINE8	UC_DRAWLINE8
#define DRAWLINE10	UC_DRAWLINE10
#define DRAWLINE11	UC_DRAWLINE11
#endif INTER3

#define FBC_INIT	UCTEST_INIT
