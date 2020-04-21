#ifndef UCTESTDEF
#define UCTESTDEF
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
 */

#ifdef INTER1
extern short _ucbuffer;	/* declared in interface.c */

#define UCTEST_INIT	/* do nothing here, GF requires init */

#define BASEADDR	((short*)0x112000)
#define LOAD(off,val)	{ _ucbuffer = val; *(BASEADDR + off) = (_ucbuffer); }
#define READ(off)	(*(BASEADDR + off))
#define UCADR(dev,adr)	(((dev)<<2)|(adr)&3)

#ifdef UC2
#define LDED(x)		LOAD (UCADR (0, 0), x)
#define LDEC(x)		LOAD (UCADR (0, 1), x)
#define LDXS(x)		LOAD (UCADR (0, 2), x)
#define LDXE(x)		LOAD (UCADR (0, 3), x)
#define LDYS(x)		LOAD (UCADR (1, 0), x)
#define LDYE(x)		LOAD (UCADR (1, 1), x)
#define LDFMADDR(x)	LOAD (UCADR (1, 2), x)
#define LDCONFIG(x)	LOAD (UCADR (1, 3), x)
#endif /* UC2 */

#ifdef UC3
#define LDCONFIG(x)	LOAD (UCADR (0, 0), x)
#define LDED(x)		LOAD (UCADR (0, 1), x)
#define LDEC(x)		LOAD (UCADR (0, 2), x)
#define LDXS(x)		LOAD (UCADR (0, 3), x)
#define LDXE(x)		LOAD (UCADR (1, 0), x)
#define LDYS(x)		LOAD (UCADR (1, 1), x)
#define LDYE(x)		LOAD (UCADR (1, 2), x)
#define LDFMADDR(x)	LOAD (UCADR (1, 3), x)
#endif /* UC3 */

#define REQHIGH		(READ (UCADR (2, 3)) & 0x8000)
#ifdef DC2
#define VERTINT		(READ (UCADR (2, 3)) & 0x2000)
#endif /* DC2 */
#ifdef DC3
#define VERTINT		(!(READ (UCADR (2, 3)) & 0x2000))
#endif /* DC3 */
#define RDPIXEL		(READ (UCADR (3, 2)) & 0xffff)
#ifdef UC2
#define RDMASK		(READ (UCADR (2, 0)) & 0xff)
#endif /* UC2 */
#define RDFONT		(READ (UCADR (2, 1)) & 0xff)

#define REQUEST(cmnd,x)	{ LOAD (UCADR (2, 0), x);	\
			  LOAD (UCADR (3, 0),cmnd);	\
			  while (REQHIGH);		\
			  }
#endif	/* INTER1 */

/*
 *	Following are GF related macros
 */

#ifdef INTER2

#define GFBETA
#include <gfdev.h>
#include <betacodes.h>

#define intlevel7	asm("	orw	#/0700,sr"); \
			asm("	andw	#/F7FF,sr")


#define FORCEWAIT	(RUNMODE | SUBSTDI | SUBSTIN_BIT | FORCEREQ_BIT_)
#define FORCEREQ	(FORCEWAIT &  ~FORCEREQ_BIT_)
#define FORCEREAD	(READOUTRUN | SUBSTIN_BIT | FORCEREQ_BIT_)

	/* GF1 reset procedure	*/

#define UCTEST_INIT     {  \
			short _ii; \
			intlevel7; \
			GFdisabvert(GEDEBUG & ~SUBSTBPCCODE_BIT_,STARTDEV); \
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

#define LOAD(which,data)	{ SEND(BPCloadreg); \
				  SEND(data); \
				  SEND(which) \
				}

	/* BPC strobes	*/

#ifdef UC3
#define LDCONFIG(x)	LOAD( 8,x);
#define LDED(x)		LOAD( 9,x);
#define LDEC(x)		LOAD(10,x);
#define LDXS(x)		LOAD(11,x);
#define LDXE(x)		LOAD(12,x);
#define LDYS(x)		LOAD(13,x);
#define LDYE(x)		LOAD(14,x);
#define LDFMADDR(x)	LOAD(15,x);
#endif /* UC3 */

	/* BPC command request	*/

#define REQUEST(cmnd,x)	{ SEND(BPCcommand); \
			  SEND(x); \
			  SEND(cmnd); \
/*			  while (FBCdata != 0x40) ;  */ \
/*			  while (FBCdata != 0x40) ;  */ \
			  while (!(FBCflags&FBCREQ_BIT_)); \
			}

#define VERTINT		(FBCflags & VERTINT_BIT)
#undef  RDPIXEL
#define RDPIXEL		FBCpixel
#define RDFONT		(ReadFont () & 0xff)

#endif /* INTER2 */

#ifdef INTER4	/* gf2 */

#include "gf2.h"
#include "gl2cmds.h"
#ifndef FBCsend
#include "gf2init.c"
#endif FBCsend

#define intlevel7	asm("	orw	#/0700,sr"); \
			asm("	andw	#/F7FF,sr")


#define FORCEWAIT	(RUNMODE | SUBSTDI | SUBSTIN_BIT | FORCEREQ_BIT_)
#define FORCEREQ	(FORCEWAIT &  ~FORCEREQ_BIT_)
#define FORCEREAD	(READOUTRUN | SUBSTIN_BIT | FORCEREQ_BIT_)

#define UCTEST_INIT	FBC_Reset()

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

#ifdef UC3
#define LDCONFIG(x)	LOAD( 8,x);
#define LDED(x)		LOAD( 9,x);
#define LDEC(x)		LOAD(10,x);
#define LDXS(x)		LOAD(11,x);
#define LDXE(x)		LOAD(12,x);
#define LDYS(x)		LOAD(13,x);
#define LDYE(x)		LOAD(14,x);
#define LDFMADDR(x)	LOAD(15,x);
#endif /* UC3 */

	/* BPC command request	*/

#define REQUEST(cmnd,x)	{ SEND(BPCcommand); \
			  SEND(x); \
			  SEND(cmnd|0x8000); \
			  while (!(FBCflags&FBCREQ_BIT_)); \
			}

#define VERTINT		(FBCflags & VERTINT_BIT)
#undef  RDPIXEL
#define RDPIXEL		FBCpixel
#define RDFONT		(ReadFont () & 0xff)

#endif INTER4

#endif UCTESTDEF
