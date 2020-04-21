/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/gl2uctest.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:14 $
 */
/*  uctest.h
 *
 *	taken from uctest.h
 */

#ifdef INTER3
#define UC4setup	register unsigned short _ucbuffer
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
#define REQUEST(cmnd,x)	{UCWAIT; _ucbuffer=x;*UCCommandAddr(cmnd)=_ucbuffer;\
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
