#define UCBIT(x)	(1<<(x))

#define UC_DISPLAYA	UCBIT(0)
#define UC_DISPLAYB	UCBIT(1)
#define UC_UPDATEA	UCBIT(2)
#define UC_UPDATEB	UCBIT(3)
#define UCR_MBENAB	UCBIT(9)	/* read/write */
#define UC_PFICD	UCBIT(8)
#define UC_PFIREAD	UCBIT(9)
#define UC_PFICOLUMN	UCBIT(10)
#define UC_PFIXDOWN	UCBIT(11)
#define UC_PFIYDOWN	UCBIT(12)

#define UCCommandAddr(command)	(short*)(UCMBM(0x200|((command)<<1)))
#define UCRAddr			(short*)(UCMBM(0x180))
#define UCBufferAddr(buffer)	(short*)(UCMBM(0x80|((buffer)<<1)))

#define UC_XSB		0x03
#define UC_XEB		0x04
#define UC_YSB		0x05
#define UC_YEB		0x06

#define UC_SETADDRS	0x03
#define UC_FILLRECT	0x0a
#define UC_SETCOLORCD	0x14
#define UC_SETCOLORAB	0x15
#define UC_SETWECD	0x16
#define UC_SETWEAB	0x17
#define UC_READPIXELCD	0x18
#define UC_READPIXELAB	0x19
#define UC_DRAWPIXELCD	0x1a
#define UC_DRAWPIXELAB	0x1b
#define UC_CFB		0x12
#define UC_MDB		0x10
#define UC_FMADDR	0x07
#define UCR_BUSY	UCBIT(15)	/* read only */

#define LOAD(adr,val) { *UCBufferAddr(adr) = (val); }

#define LDXS(x)		LOAD (UC_XSB, x)
#define LDXE(x)		LOAD (UC_XEB, x)
#define LDYS(x)		LOAD (UC_YSB, x)
#define LDYE(x)		LOAD (UC_YEB, x)

#define LDFMADDR(x)	LOAD (UC_FMADDR, x)

#define RANDOMSETUP	register short *UCX = \
			    (short*)(UCMBM(0x80|((UC_XSB)<<1))); \
			register short *UCY = \
			    (short*)(UCMBM(0x80|((UC_YSB)<<1)));

#define RANDOMLDX(x)	{*UCX = x;}
#define RANDOMLDY(y)	{*UCY = y;}

#define UCWAIT		while ((*UCRAddr) & UCR_BUSY)
#define REQUEST(cmnd,x)	{*UCCommandAddr(cmnd)=(x);}
#define LDCONFIG(x)	LOAD (UC_CFB, x)
#define LDMODE(x)	LOAD (UC_MDB, x)

#define XBEFOREY 0
#define YBEFOREX 1

#define UC_SWIZZLE	UCBIT(0)
#define UC_DOUBLE	UCBIT(1)
#define UC_DEPTHCUE	UCBIT(2)
#define UC_MBSETADR	UCBIT(5)

#define UC_READPIXELAB_SETADR	(UC_MBSETADR | UC_READPIXELAB)
#define UC_READPIXELCD_SETADR	(UC_MBSETADR | UC_READPIXELCD)
#define UC_DRAWPIXELAB_SETADR	(UC_MBSETADR | UC_DRAWPIXELAB)
#define UC_DRAWPIXELCD_SETADR	(UC_MBSETADR | UC_DRAWPIXELCD)

#define RGB		0
#define SINGLE		1
#define DOUBLE_DISPA	2
#define DOUBLE_DISPB	3
