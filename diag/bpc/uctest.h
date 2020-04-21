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

extern short _ucbuffer;
extern short uc_ucr;
extern short uc_cfb;
extern short uc_edb;
extern short uc_ecb;
extern short uc_xsb;
extern short uc_xeb;
extern short uc_ysb;
extern short uc_yeb;
extern short uc_fmab;
extern short uc_rpb;
extern short uc_mdb;
extern short uc_ddasaf;
extern short uc_ddasai;
extern short uc_ddaeaf;
extern short uc_ddaeai;
extern short uc_ddasdf;
extern short uc_ddasdi;
extern short uc_ddaedf;
extern short uc_ddaedi;

/*
 *	Following are GF related macros
 */

#ifdef INTER2

#define GFBETA
#include <gfdev.h>
#include <betacodes.h>

#define intlevel7	asm("	orw	#/0700,sr"); \
			asm("	andw	#/F7FF,sr")


#define FORCEREQ	(RUNMODE | SUBSTDI | SUBSTIN_BIT)
#define FORCEWAIT	(FORCEREQ | FORCEREQ_BIT_)
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

#define SEND(x) { FBCdata = (x); \
		  FBCflags = FORCEREQ; \
		  FBCflags = FORCEWAIT; \
		}

	/* give the FBC a strobe command		*/

#define LOAD(which,data,reg)	{ reg = (data); \
				  SEND(BPCloadreg); \
				  SEND(data); \
				  SEND(which) \
				}

	/* BPC strobes	*/

#define LDCONFIG(x)	LOAD( 8,x,uc_cfb);
#define LDED(x)		LOAD( 9,x,uc_edb);
#define LDEC(x)		LOAD(10,x,uc_ecb);
#define LDXS(x)		LOAD(11,x,uc_xsb);
#define LDXE(x)		LOAD(12,x,uc_xeb);
#define LDYS(x)		LOAD(13,x,uc_ysb);
#define LDYE(x)		LOAD(14,x,uc_yeb);
#define LDFMADDR(x)	LOAD(15,x,uc_fmab);

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

#ifdef INTER3
/* UC4 has its own interface */

#define LOAD(adr,val,reg) { reg = (val); *UCBufferAddr(adr) = reg; }
#define LDED(x)		LOAD (UC_EDB, x, uc_edb)
#define LDEC(x)		LOAD (UC_ECB, x, uc_ecb)
#define LDXS(x)		LOAD (UC_XSB, x, uc_xsb)
#define LDXE(x)		LOAD (UC_XEB, x, uc_xeb)
#define LDYS(x)		LOAD (UC_YSB, x, uc_ysb)
#define LDYE(x)		LOAD (UC_YEB, x, uc_yeb)
#define LDFMADDR(x)	LOAD (UC_FMAB, x, uc_fmab)
#define LDCONFIG(x)	LOAD (UC_CFB, x, uc_cfb)
#define LDMODE(x)	LOAD (UC_MDB, x, uc_mdb)
#define LDREPEAT(x)	LOAD (UC_RPB, x, uc_rpb)
#define LDDDASAF(x)	LOAD (UC_DDASAF, x, uc_ddasaf)
#define LDDDASAI(x)	LOAD (UC_DDASAI, x, uc_ddasai)
#define LDDDAEAF(x)	LOAD (UC_DDAEAF, x, uc_ddaeaf)
#define LDDDAEAI(x)	LOAD (UC_DDAEAI, x, uc_ddaeai)
#define LDDDASDF(x)	LOAD (UC_DDASDF, x, uc_ddasdf)
#define LDDDASDI(x)	LOAD (UC_DDASDI, x, uc_ddasdi)
#define LDDDAEDF(x)	LOAD (UC_DDAEDF, x, uc_ddaedf)
#define LDDDAEDI(x)	LOAD (UC_DDAEDI, x, uc_ddaedi)

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
