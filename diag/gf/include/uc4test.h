/* modified from uctest.h
 */

#ifdef INTER3
/* UC4 has its own interface */

#define UC4setup	register short _rtmp

#define LOAD(adr,val,reg) { reg = (val); *UCBufferAddr(adr) = reg; }
#define LDED(x)		LOAD (UC_EDB, x, _rtmp)
#define LDEC(x)		LOAD (UC_ECB, x, _rtmp)
#define LDXS(x)		LOAD (UC_XSB, x, _rtmp)
#define LDXE(x)		LOAD (UC_XEB, x, _rtmp)
#define LDYS(x)		LOAD (UC_YSB, x, _rtmp)
#define LDYE(x)		LOAD (UC_YEB, x, _rtmp)
#define LDFMADDR(x)	LOAD (UC_FMAB, x, _rtmp)
#define LDCONFIG(x)	LOAD (UC_CFB, x, _rtmp)
#define LDMODE(x)	LOAD (UC_MDB, x, _rtmp)
#define LDREPEAT(x)	LOAD (UC_RPB, x, _rtmp)
#define LDDDASAF(x)	LOAD (UC_DDASAF, x, _rtmp)
#define LDDDASAI(x)	LOAD (UC_DDASAI, x, _rtmp)
#define LDDDAEAF(x)	LOAD (UC_DDAEAF, x, _rtmp)
#define LDDDAEAI(x)	LOAD (UC_DDAEAI, x, _rtmp)
#define LDDDASDF(x)	LOAD (UC_DDASDF, x, _rtmp)
#define LDDDASDI(x)	LOAD (UC_DDASDI, x, _rtmp)
#define LDDDAEDF(x)	LOAD (UC_DDAEDF, x, _rtmp)
#define LDDDAEDI(x)	LOAD (UC_DDAEDI, x, _rtmp)

#define VERTINT		(*UCRAddr & UCR_VERTICAL)
#define RDPIXEL		(*UCCommandAddr(UC_READPIXELAB))
#define RDFONT		(*UCCommandAddr(UC_READFONT))

#define UCWAIT		while ((*UCRAddr) & UCR_BUSY)
#define REQUEST(cmnd,x)	{_rtmp=x;*UCCommandAddr(cmnd)=_rtmp; UCWAIT;\
			 UCWAIT;}
#define NWREQUEST(cmnd,x) {_rtmp=x;*UCCommandAddr(cmnd)=_rtmp;}

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
