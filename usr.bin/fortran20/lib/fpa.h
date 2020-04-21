#define FPADDR 0x8000
#ifdef NOTDEF
#define fpaop(opc,dst,src)	FPADDR+opc+dst+dst+dst+dst\
+dst+dst+dst+dst+dst+dst+dst+dst+dst+dst+dst+dst+src:w
#endif
#define fpaop(opc,dst,src)	FPADDR+opc+0x/**/dst/**/0+0x/**/src:w

#define RESULT	f
#define DUMMY	0

/* command opcodes */
#define SINGLE		0x100
#define INTEGER		0x200
#define	DOUBLEHI	0x300
#define DOUBLELO	0x400
#define COPY		0x500
#define OR		0x600
#define CR		0x700
#define ER		0x800
#define MR		0x900
#define ADDSGL		0xa00
#define ADDSGLUPD	0xb00
#define ADDDBL		0xc00
#define ADDDBLUPD	0xd00
#define SUBSGL		0xe00
#define SUBSGLUPD	0xf00
#define SUBDBL		0x1000
#define SUBDBLUPD	0x1100
#define REVSUBSGL	0x1200
#define REVSUBSGLUPD	0x1300
#define REVSUBDBL	0x1400
#define REVSUBDBLUPD	0x1500
#define MULSGL		0x1600
#define MULSGLUPD	0x1700
#define MULDBL		0x1800
#define MULDBLUPD	0x1900
#define DIVSGL		0x1a00
#define DIVSGLUPD	0x1b00
#define DIVDBL		0x1c00
#define DIVDBLUPD	0x1d00
#define REVDIVSGL	0x1e00
#define REVDIVSGLUPD	0x1f00
#define REVDIVDBL	0x2000
#define REVDIVDBLUPD	0x2100
#define SGLDBL		0x2200
#define SGLDBLUPD	0x2300
#define DBLSGL		0x2400
#define DBLSGLUPD	0x2500
#define INTSGL		0x2600
#define INTSGLUPD	0x2700
#define SGLINT		0x2800
#define SGLINTUPD	0x2900
#define INTDBL		0x2a00
#define INTDBLUPD	0x2b00
#define DBLINT		0x2c00
#define DBLINTUPD	0x2d00
#define NEGSGL		0x2e00
#define NEGSGLUPD	0x2f00
#define NEGDBL		0x3000
#define NEGDBLUPD	0x3100
#define CMPSGL		0x3200
#define CMPDBL		0x3300
#define TSTSGL		0x3400
#define TSTDBL		0x3500
#define	ADDDBLC1	ADDDBL
#define	ADDDBLUPDC1	ADDDBLUPD
#define	MULDBLC1	MULDBL
#define	MULDBLUPDC1	MULDBLUPD
#define CMPDBLC1	CMPDBL
#define COPYC1		COPY
#define REVDIVDBLUPDC1	REVDIVDBLUPD
#define SUBDBLC1	SUBDBL
#define SUBDBLUPDC1	SUBDBLUPD

#define SIGNBIT	0x80000000
#define EXPMASK 0x7f800000
#define EXPSHIFT 23
#define EXPONE	0x00800000
#define SINGLEONE	(0x7f)<<EXPSHIFT
#define DOUBLEONEHI	0x3ff00000
#define INFINITY	(255)<<EXPSHIFT
#define SINGLENAN	0x7F800000

/* bits in the error register (ER) - also in mask register (MR) */
/* error asserted low, mask enables bus error when low */
#define ER_DIVBYZERO	0x01
#define ER_OVERFLOW	0x02
#define ER_UNDERFLOW	0x04
#define ER_DENORMALIZE	0x08
#define ER_NAN		0x10
#define ER_ILLEGALOP	0x20
#define ER_INEXACT	0x40

/* bits in the option register (OR) */
/* asserted high */
#define OR_FASTMODE	0x01
#define OR_FIXTOZERO	0x02
#define OR_ROUNDMODE0	0x04
#define OR_ROUNDMODE1	0x08
#define OR_OVERLAP	0x10
