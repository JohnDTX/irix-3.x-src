#ifndef FALSE
#define FALSE	0
#endif  FALSE

#ifndef TRUE
#define TRUE	1
#endif TRUE

#define	SINGLESIZE	0
#define DOUBLESIZE	1
#define	CONDREG		2

typedef unsigned long *fplong;
typedef unsigned char *fpchar;

#define FPADDR 0xFFFF8000
#define Instr(opc,dst,src)            (FPADDR|((opc)<<8)|((dst)<<4)|(src))
#define Instrl(opc,reg)	    *((fplong)(FPADDR|((opc)<<8)|((reg)<<4)      ))
#define Instrb(opc,dst,src) *((fpchar)(FPADDR|((opc)<<8)|((dst)<<4)|(src)))

#define RESULT	15
#define DUMMY	0

/* command opcodes */
#define SINGLE		1
#define INTEGER		2
#define	DOUBLEHI	3
#define DOUBLELO	4
#define COPY		5
#define OR		6
#define CR		7
#define ER		8
#define MR		9
#define ADDSGL		10
#define ADDSGLUPD	11
#define ADDDBL		12
#define ADDDBLUPD	13
#define SUBSGL		14
#define SUBSGLUPD	15
#define SUBDBL		16
#define SUBDBLUPD	17
#define REVSUBSGL	18
#define REVSUBSGLUPD	19
#define REVSUBDBL	20
#define REVSUBDBLUPD	21
#define MULSGL		22
#define MULSGLUPD	23
#define MULDBL		24
#define MULDBLUPD	25
#define DIVSGL		26
#define DIVSGLUPD	27
#define DIVDBL		28
#define DIVDBLUPD	29
#define REVDIVSGL	30
#define REVDIVSGLUPD	31
#define REVDIVDBL	32
#define REVDIVDBLUPD	33
#define SGLDBL		34
#define SGLDBLUPD	35
#define DBLSGL		36
#define DBLSGLUPD	37
#define INTSGL		38
#define INTSGLUPD	39
#define SGLINT		40
#define SGLINTUPD	41
#define INTDBL		42
#define INTDBLUPD	43
#define DBLINT		44
#define DBLINTUPD	45
#define NEGSGL		46
#define NEGSGLUPD	47
#define NEGDBL		48
#define NEGDBLUPD	49
#define CMPSGL		50
#define CMPDBL		51
#define TSTSGL		52
#define TSTDBL		53
#define DIVSGLLOOKUP	0x50
#define DIVDBLLOOKUP	0x51

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
