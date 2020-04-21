
# include "mfile2.h"	/* includes macdefs.h mac2defs.h manifest.h */

# define TSCALAR TCHAR|TUCHAR|TSHORT|TUSHORT|TINT|TUNSIGNED|TPOINT
/* GB - use TWORDORLONG for types when addressing B registers */
# define TWORDORLONG TSHORT|TUSHORT|TINT|TUNSIGNED|TPOINT
# define AWD SNAME|SOREG|SCON|STARNM|STARREG|SAREG
# define LWD SNAME|SOREG|SCON|SAREG
# define EA SNAME|SOREG|SCON|STARREG|SAREG|SBREG 
# define EAA SNAME|SOREG|SCON|STARREG|SAREG
# define EB SBREG

struct optab  table[] = {

ASSIGN,	INAREG|FOREFF|FORCC,
	EAA,	TSCALAR|TFLOAT,
	SZERO,	TANY,
		0,	RLEFT|RRIGHT|RESCC,
		"	clrZB	AL\n",

ASSIGN,	INAREG|FOREFF|FORCC,
	EAA,	TDOUBLE,
	SZERO,	TANY,
		0,	RLEFT|RRIGHT|RESCC,
		"	clrl	UL\n	clrl	AL\n",

ASSIGN,	INAREG|FOREFF|FORCC,
	SAREG|STAREG,	TSCALAR,
	SCCON,	TSCALAR,
		0,	RLEFT|RRIGHT|RESCC,
		"	moveq	AR,AL\n",

ASSIGN,	INAREG|FOREFF|FORCC,
	EAA,	TSCALAR|TFLOAT,
/* GB - bug86 - use following template for moving from B registers.
		changed following line from EA to EAA. */
	EAA,	TSCALAR|TFLOAT,
		0,	RLEFT|RRIGHT|RESCC,
		"	movZB	AR,AL\n",

ASSIGN,	INAREG|FOREFF|FORCC,
	EAA,	TSCALAR,
	EB,	TWORDORLONG,
		0,	RLEFT|RRIGHT|RESCC,
		"	movZB	AR,AL\n",

ASSIGN, INAREG|FOREFF|FORCC,
	EAA,	TSCALAR,
	EB,	TCHAR|TUCHAR,
		NASR|NAREG,	RLEFT|RRIGHT|RESCC,
		"\tmovw\tAR,A1\n\tmovb\tA1,AL\n",

ASSIGN,	INAREG|FOREFF,
	EAA,	TDOUBLE,
	EA,	TDOUBLE,
		0,	RLEFT|RRIGHT,
		"	movl	UR,UL\n	movl	AR,AL\n",

#ifndef DOUBLES32BITS


ASSIGN,	INAREG|FOREFF,
	EAA,	TFLOAT,
	EA,	TDOUBLE,
		0,	RLEFT|RRIGHT,
		"	movl	AR,AL\n",
#endif

ASSIGN,	INBREG|FOREFF,
	EB,	TSCALAR,
	EA,	TSCALAR,
		0,	RLEFT|RRIGHT|RESCC,
		"	movZB	AR,AL\n",

ASSIGN, INAREG|FOREFF,
	SFLD,	TANY,
	SZERO,	TANY,
		0,	RRIGHT,
		"	andY	#N,AL\n",

#ifdef SGI_FIELDS

ASSIGN, INAREG|FOREFF,
	SFLD,	TANY,
	EA,	TANY,
		2*NAREG,	RRIGHT,
		"ZZ", 	/* do the fields ourselves (local2) */

#else

ASSIGN, INTAREG|INAREG|FOREFF,
	SFLD,	TANY,
	STAREG,	TANY,
		NAREG,	RRIGHT,
		"F\tmovl\tAR,sp@-\n\tmovl\t#H,A1\n\tlslY\tA1,AR\n\tandY\t#M,AR\n\tandY\t#N,AL\n\torY\tAR,AL\nF\tmovl\tsp@+,AR\n",

ASSIGN, INAREG|FOREFF,
	SFLD,	TANY,
	EA,	TANY,
		2*NAREG,	RRIGHT,
		"\tmovZB\tAR,A1\n\tmovl\t#H,A2\n\tlslY\tA2,A1\n\tandY\t#M,A1\n\tandY\t#N,AL\n\torY\tA1,AL\n",

#endif

/* put this here so UNARY MUL nodes match OPLTYPE when appropriate */
UNARY MUL,	INTAREG|INAREG|FORCC,
	SBREG,	TSCALAR,
	SANY,	TANY,
		NAREG|NASR,	RESC1|RESCC,
		"	movZB	AL@,A1\n",

/* BUG FIX (GB) - free-standing OPLTYPE nodes which have side-effects shouldn't
   be thrown away 2-7-84. Changed left op in following from SANY to EA. */

OPLTYPE,	FOREFF,
	EA,	TANY,
	EA,	TANY,
		0,	RRIGHT,
		"",   /* this entry throws away computations which don't do anything */


/* YOU CANT USE A tst INSTRUCTION IF THE TARGET IS A CONSTANT!
   (scr0610 (GB) SGI 10/15/85).  This table entry often results in AWFUL
   code.  An attempt will be made to fix this SCR for the CBRANCH case
   in cbranch() (reader.c).  For now, the code we get is 
	moveq	#x,d0
	tstl	d0
	bne	.Ly
   when we should get bxx .Ly or nothing, depending on x.
*/
OPLTYPE,	FORCC,
	SANY,	TANY,
	SCON,	TSCALAR,
		NAREG,	RESCC,
		"	moveq\tAR,A1\n\ttstl\tA1\n",

OPLTYPE,	FORCC,
	SANY,	TANY,
	SNAME|SOREG|STARREG|SAREG,TSCALAR,
		0,	RESCC,
		"	tstZB	AR\n",		


OPLTYPE,	FORCC,
	SANY,	TANY,
	EB,	TSCALAR,
		0,	RESCC,
		"	cmpZB	#0,AR\n",

OPLTYPE,	INTAREG|INAREG|FORCC,
	SANY,	TANY,
	SZERO,	TSCALAR,
		NAREG|NASR,	RESC1|RESCC,
		"	clrZB	A1\n",

OPLTYPE,	INTAREG|INAREG|FORCC,
	SANY,	TANY,
	SCCON,	TSCALAR,
		NAREG|NASR,	RESC1|RESCC,
		"	moveq	AR,A1\n",

OPLTYPE,	INTAREG|INAREG|FORCC,
	SANY,	TANY,
	EA,	TSCALAR,
		NAREG|NASR,	RESC1|RESCC,
		"	movZB	AR,A1\n",

/* altered template below for double register (2* removed, A2 -> U1) GB */
OPLTYPE,	INTAREG|INAREG,
	SANY,	TANY,
	EA,	TDOUBLE,
		NAREG|NASR,	RESC1,
		"	movl	UR,U1\n	movl	AR,A1\n",

OPLTYPE,	INTAREG|INAREG,
	SANY,	TANY,
	EA,	TFLOAT,
		NAREG|NASR,	RESC1,
		"	movl	AR,A1\n",

OPLTYPE,	INTBREG|INBREG|FORCC,
	SANY,	TANY,
/*	GB - this should be a pointer, but has to be an INT too */
 	EA,	TSCALAR,
		NBREG|NBSR,	RESC1|RESCC,
		"	movZB	AR,A1\n",

OPLTYPE,	INTEMP|FORCC,
	SANY,	TANY,
	EA,	TSCALAR,
		NTEMP,	RESC1|RESCC,
		"	movZB	AR,A1\n",

/* added missing \n on template below to fix bugreport #16
   GB (SGI) 4/28/83 
*/
OPLTYPE,	INTEMP|FORCC,
	SANY,	TANY,
	EA,	TDOUBLE,
		NTEMP,	RESC1,
		"	movl	UR,U1\n	movl	AR,A1\n",

#ifdef DOUBLES32BITS
OPLTYPE,	INTEMP|FORCC,
	SANY,	TANY,
	EA,	TFLOAT,
		NTEMP,	RESC1,
		"	movl	AR,A1\n",
#endif

OPLTYPE,	FORARG,
	SANY,	TANY,
	SBREG,	TINT|TUNSIGNED|TPOINT,
		0,	RNULL,
#ifdef SGI_REGS
		"	Z-AR\n",
#else
		"	pea	AR@Z-\n",
#endif

OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TINT|TUNSIGNED|TPOINT,
		0,	RNULL,
#ifdef SGI_REGS
		"	Z-AR\n",
#else
		"	movl	AR,Z-\n",
#endif

/* 	Changed B-type of registers to A-type on template below
	in response to bugreport #25. (GB) 7/20/83. */


OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TSHORT,
		NAREG|NASR,	RNULL,
#ifdef SGI_REGS
		"	Z-XR1\n",
#else
		"	movw	AR,A1\n\textl	A1\n\tmovl	A1,Z-\n",
#endif

/*	Disallowed sharing of A reg on following template
	in response to bugreport #21. (GB) 7/17/83  */

OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TUSHORT,
		NAREG,	RNULL,
#ifdef SGI_REGS
		"	Z-MR1\n",
#else
		"	clrl	A1\n	movw	AR,A1\n	movl	A1,Z-\n",
#endif
OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TCHAR,
		NAREG|NASR,	RNULL,
#ifdef SGI_REGS
		"	Z-XR1\n",
#else
		"	movb	AR,A1\n	extw	A1\n	extl	A1\n	movl	A1,Z-\n",
#endif

/*		Disallowed sharing of A register on the below template in
		response to bugreport #21. (GB) 7/17/83.	*/

OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TUCHAR,
		NAREG,	RNULL,
#ifdef SGI_REGS
		"	Z-MR1\n",
#else
		"	clrl	A1\n	movb	AR,A1\n	movl	A1,Z-\n",
#endif

#ifdef DOUBLES32BITS
OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TFLOAT,
		0,	RNULL,
#ifdef SGI_REGS	
		"	Z-AR\n",
#else
		"	movl	AR,Z-\n",
#endif
#else

OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TFLOAT,
		0,	RNULL,
		"	clrl	Z-\n	movl	AR,Z-\n",
#endif

OPLTYPE,	FORARG,
	SANY,	TANY,
	SNAME|SOREG|SCON|SAREG|SBREG,	TDOUBLE,
		0,	RNULL,
#ifdef SGI_REGS
		"	Z-UR\n\tZ-AR\n",
#else
		"	movl	UR,Z-\n	movl	AR,Z-\n",
#endif

OPLTYPE,	FORARG,
	SANY,	TANY,
	STARREG,	TDOUBLE,
		0,	RNULL,
#ifdef SGI_REGS
		"	Z-UR\n\tZ-AR\n",
#else
		"	movl	UR,Z-\n	movl	AR,Z-\n",
#endif

/* The following template changed from version .v4 (GB) */
/* The following template added by GB to get rid of spurious
   cmpl #0,x instructions.  BUG106.
*/
OPLOG,	FORCC,
	SNAME|SOREG|STARREG|SAREG,TSCALAR,
	SZERO,	TSCALAR,
		0,	RESCC,
		"	tstZL	AL\nZI",		

OPLOG,	FORCC,
	SAREG|STAREG|SBREG|STBREG,	TSCALAR,
	EA,	TSCALAR,
		0,	RESCC,
		"	cmpZL	AR,AL\nZI",

/* The following template changed from .v4 to fix bug #26 */

OPLOG,	FORCC,
	SNAME|SOREG|STARREG|SAREG,	TSCALAR,
	SCON,	TSCALAR,
		0,	RESCC,
		"	cmpZL	AR,AL\nZI",
#ifdef DOUBLES32BITS

OPLOG,	FORCC,
	EA,	TFLOAT,
	EA,	TFLOAT,
		0,	RESCC,
#ifdef SGI_REGS
		"ZFZbZx\tZ-AR\n\tZ-AL\nZcZeZI",
#else
		"\tmovl\tAR,Z-\n\tmovl\tAL,Z-\n\tjbsr\t_f_cmp\n\taddql\t#8,sp\nZI",
#endif

OPLOG,	FORCC,
	EA,	TDOUBLE,
	EA,	TDOUBLE,
		0,	RESCC,
#ifdef SGI_REGS
		"ZFZbZx\tZ-UR\n\tZ-AR\n\tZ-UL\n\tZ-AL\nZcZeZI",
#else
		"Zb\tmovl\tUR,Z-\n\tmovl\tAR,Z-\n\tmovl\tUL,Z-\n\tmovl\tAL,Z-\n\tjbsr\t_d_cmp\nZeZI",
#endif

#else


OPLOG,	FORCC,
	EA,	TDOUBLE,
	EA,	TDOUBLE,
		NTEMP,	RESCC,
		"\tmovl\t#16,A1\n\tmovl\tUR,Z-\n\tmovl\tAR,Z-\n\tmovl\tUL,Z+\n\tmovl\tAL,Z+\n\tjbsr\tfcmp\n\taddl\tA1,sp\nZI",

#endif
/* scr1775 */
OPLOG,	FORCC,
	SANY,	TUNDEF,
	SANY,	TANY,
		0,	RESCC,
		"",

CCODES,	INTAREG|INAREG,
	SANY,	TANY,
	SANY,	TANY,
		NAREG,	RESC1,
		"	moveq	#1,A1\nZN",

#ifdef SGI_FIELDS
FLD,	INTAREG,
	SANY,	TANY,
	SFLD,	TANY,
		NAREG,	RESC1,
		"ZY",

FLD,	FORCC,
	SANY,	TANY,
	SFLD,	TANY,
		NAREG,	RESCC,
		"ZX",
#endif

UNARY MINUS,	INTAREG|INAREG,
	STAREG,	TSCALAR,
	SANY,	TANY,
		0,	RLEFT,
		"	negZB	AL\n",

#ifdef SGI_REGS
/* take float negate out of the hardop list (GB) */
UNARY MINUS,	INTAREG|INAREG,
	STAREG,	TFLOAT|TDOUBLE,
	SANY,	TANY,
		0,	RLEFT,
		"\tbchg\t#31,AL\n",

#endif

COMPL,	INTAREG|INAREG,
	STAREG,	TSCALAR,
	SANY,	TANY,
		0,	RLEFT,
		"	notZB	AL\n",

INCR,	INTAREG|INAREG|FOREFF,
	EAA,	TSCALAR,
	S8CON,	TSCALAR,
		NAREG,	RESC1,
		"F	movZB	AL,A1\n	addqZB	AR,AL\n",

DECR,	INTAREG|INAREG|FOREFF,
	EAA,	TSCALAR,
	S8CON,	TSCALAR,
		NAREG,	RESC1,
		"F	movZB	AL,A1\n	subqZB	AR,AL\n",

INCR,	INTAREG|INAREG|FOREFF,
	EAA,	TSCALAR,
	SCON,	TSCALAR,
		NAREG,	RESC1,
		"F	movZB	AL,A1\n	addZB	AR,AL\n",

DECR,	INTAREG|INAREG|FOREFF,
	EAA,	TSCALAR,
	SCON,	TSCALAR,
		NAREG,	RESC1,
		"F	movZB	AL,A1\n	subZB	AR,AL\n",

INCR,	INTBREG|INBREG|FOREFF,
	EB,	TSCALAR,
	S8CON,	TSCALAR,
		NBREG,	RESC1,
		"F	movZB	AL,A1\n	addqZB	AR,AL\n",

DECR,	INTBREG|INBREG|FOREFF,
	EB,	TSCALAR,
	S8CON,	TSCALAR,
		NBREG,	RESC1,
		"F	movZB	AL,A1\n	subqZB	AR,AL\n",

INCR,	INTBREG|INBREG|FOREFF,
	EB,	TSCALAR,
	SCON,	TSCALAR,
		NBREG,	RESC1,
		"F	movZB	AL,A1\n	addZB	AR,AL\n",

DECR,	INTBREG|INBREG|FOREFF,
	EB,	TSCALAR,
	SCON,	TSCALAR,
		NBREG,	RESC1,
		"F	movZB	AL,A1\n	subZB	AR,AL\n",

PLUS,		INBREG|INTBREG,
	SBREG,	TPOINT,
	SICON,	TANY,
		NBREG|NBSL,	RESC1,
		"	lea	AL@(ZO),A1\n",

PLUS,		FORARG,
	SBREG,	TPOINT,
	SICON,	TANY,
		0,	RNULL,
#ifdef SGI_REGS
		"\tZQ\tAL@(ZO)ZRZP\n",
#else
		"	pea	AL@(ZO)ZP\n",
#endif

MINUS,		INBREG|INTBREG,
	SBREG,	TPOINT,
	SICON,	TANY,
		NBREG|NBSL,	RESC1,
		"	lea	AL@(ZM),A1\n",

MINUS,		FORARG,
	SBREG,	TPOINT,
	SICON,	TANY,
		0,	RNULL,
#ifdef SGI_REGS
		"\tZQ\tAL@(ZM)ZRZP\n",
#else
		"	pea	AL@(ZM)ZP\n",
#endif
	/* GB - SCR1744 - generate hardware divide and multiply
	   instructions on the 68020.
	*/
MUL,	INTAREG,
	EAA,	TINT,
	SCON,	(TUNSIGNED|TINT),
		NAREG,	RESC1,
		"	movl\tAL,A1\n\tmulsl	AR,A1\n",

MUL,	INTAREG,
	EAA,	(TUNSIGNED|TPOINT),
	SCON,	(TUNSIGNED|TINT),
		NAREG,	RESC1,
		"	movl\tAL,A1\n\tmulul	AR,A1\n",


ASG PLUS,	INAREG|FORCC,
	EAA,	TSCALAR,
	S8CON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	addqZB	AR,AL\n",

ASG PLUS,	INBREG|FORCC,
	EB,	TSCALAR,
	S8CON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	addqZB	AR,AL\n",

ASG PLUS,	INAREG|FORCC,
	SAREG|STAREG,	TSCALAR,
	EA,	TSCALAR,
		0,	RLEFT|RESCC,
		"	addZB	AR,AL\n",

ASG PLUS,	INBREG,
	SBREG|STBREG,	TSCALAR,
	EA,	TSCALAR,
		0,	RLEFT,
		"	addZB	AR,AL\n",

ASG PLUS,	INAREG|FORCC,
	EAA,	TSCALAR,
	SAREG|STAREG,	TSCALAR,
		0,	RLEFT|RESCC,
		"	addZB	AR,AL\n",

ASG MINUS,	INAREG|FORCC,
	EAA,	TSCALAR,
	S8CON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	subqZB	AR,AL\n",

ASG MINUS,	INBREG|FORCC,
	EB,	TSCALAR,
	S8CON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	subqZB	AR,AL\n",

ASG MINUS,	INAREG|FORCC,
	SAREG|STAREG,	TSCALAR,
	EA,	TSCALAR,
		0,	RLEFT|RESCC,
		"	subZB	AR,AL\n",

ASG MINUS,	INBREG,
	SBREG|STBREG,	TSCALAR,
	EA,	TSCALAR,
		0,	RLEFT,
		"	subZB	AR,AL\n",

ASG MINUS,	INAREG|FORCC,
	EAA,	TSCALAR,
	SAREG|STAREG,	TSCALAR,
		0,	RLEFT|RESCC,
		"	subZB	AR,AL\n",

ASG ER, 	INAREG|FORCC,
	EAA,	TSCALAR,
	SCON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	eorZB	AR,AL\n",

ASG ER, 	INAREG|FORCC,
	EAA,	TSCALAR,
	SAREG|STAREG,	TSCALAR,
		0,	RLEFT|RESCC,
		"	eorZB	AR,AL\n",

ASG OPSIMP, 	INAREG|FORCC,
	SAREG|STAREG,	TSCALAR,
	EAA,	TSCALAR,
		0,	RLEFT|RESCC,
		"	OIZB	AR,AL\n",

ASG OPSIMP, 	INAREG|FORCC,
	EAA,	TSCALAR,
	SCON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	OIZB	AR,AL\n",

ASG OPSIMP, 	INAREG|FORCC,
	EAA,	TSCALAR,
	SAREG|STAREG,	TSCALAR,
		0,	RLEFT|RESCC,
		"	OIZB	AR,AL\n",

	/* GB - SCR1744 - generate hardware divide and multiply
	   instructions on the 68020.
	*/
ASG MUL,	INAREG|FORCC,
	SAREG|STAREG,	TINT,
	EAA,	TINT,
		0,	RLEFT|RESCC,
		"	mulsl	AR,AL\n",

ASG MUL,	INAREG|FORCC,
	SAREG|STAREG,	(TUNSIGNED|TPOINT),
	EAA,	(TINT|TUNSIGNED|TPOINT),
		0,	RLEFT|RESCC,
		"	mulul	AR,AL\n",

ASG MUL,	INAREG|FORCC,
	SAREG|STAREG,	(TINT|TUNSIGNED|TPOINT),
	EAA,	(TUNSIGNED|TPOINT),
		0,	RLEFT|RESCC,
		"	mulul	AR,AL\n",


ASG MUL,	INAREG|FORCC,
	SAREG|STAREG,	TSHORT,
	EAA,	TSHORT,
		0,	RLEFT|RESCC,
		"	muls	AR,AL\n",

ASG MUL,	INAREG|FORCC,
	SAREG|STAREG,	TUSHORT,
	EAA,	TUSHORT|TSHORT,
		0,	RLEFT|RESCC,
		"	mulu	AR,AL\n",

ASG MUL,	INAREG|FORCC,
	SAREG|STAREG,	TSHORT,
	EAA,	TUSHORT,
		0,	RLEFT|RESCC,
		"	mulu	AR,AL\n",

ASG MUL,	INAREG,
	SAREG|STAREG,	TCHAR,
	EAA,	TCHAR,
		NAREG,	RLEFT,
		"\textw	AL\n\tmovb	AR,A1\n\textw	A1\n\tmuls	A1,AL\n",

ASG MUL,	INAREG,
	SAREG|STAREG,	TUCHAR,
	EAA,	TUCHAR|TCHAR,
		NAREG,	RLEFT,
		"\tandw\t#255,AL\n\tclrw\tA1\n\tmovb\tAR,A1\n\tmuls\tA1,AL\n",

	/* GB - SCR1744 - generate hardware divide and multiply
	   instructions on the 68020.
	*/
ASG DIV,	INAREG|FORCC,
	SAREG|STAREG,	TINT,
	EAA,	TINT,
		0,	RLEFT|RESCC,
		"	divsl	AR,AL\n",

ASG DIV,	INAREG|FORCC,
	SAREG|STAREG,	(TUNSIGNED|TPOINT),
	EAA,	(TINT|TUNSIGNED|TPOINT),
		0,	RLEFT|RESCC,
		"	divul	AR,AL\n",

ASG DIV,	INAREG|FORCC,
	SAREG|STAREG,	(TINT|TUNSIGNED|TPOINT),
	EAA,	(TUNSIGNED|TPOINT),
		0,	RLEFT|RESCC,
		"	divul	AR,AL\n",

ASG DIV,	INAREG|FORCC,
	SAREG|STAREG,	TINT,
	EAA,	TINT,
		0,	RLEFT|RESCC,
		"	divs	AR,AL\n	extl	AL\n",

ASG DIV,	INAREG|FORCC,
	SAREG|STAREG,	TSHORT,
	EAA,	TSHORT,
		0,	RLEFT|RESCC,
		"	extl	AL\n	divs	AR,AL\n",

ASG DIV,	INAREG|FORCC,
	SAREG|STAREG,	TUSHORT,
	EAA,	TUSHORT|TSHORT,
		0,	RLEFT|RESCC,
		"	andl	#65535,AL\n	divu	AR,AL\n",

ASG DIV,	INAREG|FORCC,
	SAREG|STAREG,	TSHORT,
	EAA,	TUSHORT,
		0,	RLEFT|RESCC,
		"	andl	#65535,AL\n	divu	AR,AL\n",

/* needed an extl on AL (#53) (GB)*/
ASG DIV,	INAREG,
	SAREG|STAREG,	TCHAR,
	EAA,	TCHAR,
		NAREG,	RLEFT,
		"\textw	AL\n\textl AL\n\tmovb	AR,A1\n\textw	A1\n\tdivs	A1,AL\n",

/* destination operand is 32-bits needed andl (#53) */
ASG DIV,	INAREG,
	SAREG|STAREG,	TUCHAR,
	EAA,	TUCHAR|TCHAR,
		NAREG,	RLEFT,
		"\tandl	#255,AL\n\tclrw	A1\n\tmovb	AR,A1\n\tdivs	A1,AL\n",
	/* GB - SCR1744 - generate hardware divide and multiply
	   instructions on the 68020.
	*/
ASG MOD,	INAREG|FORCC,
	SAREG|STAREG,	TINT,
	EAA,	TINT,
		NAREG,	RLEFT|RESCC,
		"	divsll	AR,A1:AL\n\tmovl\tA1,AL\n",

ASG MOD,	INAREG|FORCC,
	SAREG|STAREG,	(TUNSIGNED|TPOINT),
	EAA,	(TINT|TUNSIGNED|TPOINT),
		NAREG,	RLEFT|RESCC,
		"	divull	AR,A1:AL\n\tmovl\tA1,AL\n",

ASG MOD,	INAREG|FORCC,
	SAREG|STAREG,	(TINT|TUNSIGNED|TPOINT),
	EAA,	(TUNSIGNED|TPOINT),
		NAREG,	RLEFT|RESCC,
		"	divull	AR,A1:AL\n\tmovl\tA1,AL\n",

ASG MOD,	INAREG,
	SAREG|STAREG,	TSHORT,
	EAA,	TSHORT,
		0,	RLEFT,
		"	extl	AL\n	divs	AR,AL\n	swap	AL\n",

ASG MOD,	INAREG,
	SAREG|STAREG,	TUSHORT,
	EAA,	TUSHORT|TSHORT,
		0,	RLEFT,
		"	andl	#65535,AL\n	divu	AR,AL\n	swap	AL\n",

ASG MOD,	INAREG,
	SAREG|STAREG,	TSHORT,
	EAA,	TUSHORT,
		0,	RLEFT,
		"	andl	#65535,AL\n	divu	AR,AL\n	swap	AL\n",
/** needed extl on AL below (GB) (#53) */
ASG MOD,	INAREG,
	SAREG|STAREG,	TCHAR,
	EAA,	TCHAR,
		NAREG,	RLEFT,
		"\textw	AL\n\textl\tAL\n\tmovb	AR,A1\n\textw	A1\n\tdivs\tA1,AL\n	swap	AL\n",

/* needed to andl on AL below (#53) (GB) */
ASG MOD,	INAREG,
	SAREG|STAREG,	TUCHAR,
	EAA,	TUCHAR|TCHAR,
		NAREG,	RLEFT,
		"\tandl	#255,AL\n\tclrw	A1\n\tmovb	AR,A1\n\tdivs	A1,AL\n	swap	AL\n",

ASG OPSHFT, 	INAREG|FORCC,
	SAREG,	TINT|TSHORT|TCHAR,
	S8CON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	aOIZB	AR,AL\n",

ASG OPSHFT, 	INAREG|FORCC,
	SNAME|SOREG,	TSHORT,
	SONE,	TSCALAR,
		0,	RLEFT|RESCC,
		"	aOIw	AL\n",

ASG OPSHFT, 	INAREG|FORCC,
	SAREG,	TINT|TSHORT|TCHAR,
	SAREG,	TSCALAR,
		0,	RLEFT|RESCC,
		"	aOIZB	AR,AL\n",

ASG OPSHFT, 	INAREG|FORCC,
	SAREG,	TUNSIGNED|TUSHORT|TUCHAR,
	S8CON,	TSCALAR,
		0,	RLEFT|RESCC,
		"	lOIZB	AR,AL\n",

ASG OPSHFT, 	INAREG|FORCC|RESCC,
	EA,	TUSHORT,
	SONE,	TSCALAR,
		0,	RLEFT,
		"	lOIw	AL\n",

ASG OPSHFT, 	INAREG|FORCC,
	SAREG,	TUNSIGNED|TUSHORT|TUCHAR,
	SAREG,	TSCALAR,
		0,	RLEFT|RESCC,
		"	lOIZB	AR,AL\n",

/* should be register 0 */

UNARY CALL,	INTAREG,
	SBREG|SNAME|SOREG|SCON,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1, 
		"ZC",

UNARY FORTCALL,	INTAREG,
	SBREG|SNAME|SOREG|SCON,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1, 
		"ZC",

SCONV,	INTAREG,
	STAREG,	TINT|TUNSIGNED|TPOINT,
	SANY,	TINT|TUNSIGNED|TPOINT,
		0,	RLEFT,
		"",

/* added from Unisoft */
SCONV,	INTAREG,
	STAREG,	TSHORT|TUSHORT,
	SANY,	TSHORT|TUSHORT,
		0,	RLEFT,
		"",

SCONV,	INTAREG,
	STAREG,	TSCALAR,
	SANY,	TUCHAR,
		0,	RLEFT,
		"	andl	#255,AL\n",

SCONV,	INTAREG,
	STAREG,	TINT|TUNSIGNED|TPOINT,
	SANY,	TUSHORT,
		0,	RLEFT,
		"	andl	#65535,AL\n",

SCONV,	INTAREG,
	STAREG,	TINT|TUNSIGNED|TPOINT,
	SANY,	TSHORT|TCHAR,
		0,	RLEFT,
		"",

/****?? should this be extended ???***/
SCONV,	INTAREG,
	STAREG,	TCHAR,
	SANY,	TSHORT|TUSHORT,
		0,	RLEFT,
		"	extw	AL\n",

SCONV,	INTAREG,
	STAREG,	TCHAR,
	SANY,	TINT|TUNSIGNED|TPOINT,
		0,	RLEFT,
		"	extw	AL\n	extl	AL\n",

SCONV,	INTAREG,
	STAREG,	TSHORT,
	SANY,	TINT|TUNSIGNED|TPOINT,
		0,	RLEFT,
		"	extl	AL\n",

/* added from Unisoft */
SCONV,	INTAREG,
	EA,	TSHORT,
	SANY,	TINT|TUNSIGNED|TPOINT,
		NAREG|NASL,	RESC1,
		"	movw	AL,A1\n	extl	A1\n",

/* added from Unisoft */
SCONV,	INTBREG,
	EA,	TSHORT,
	SANY,	TINT|TUNSIGNED|TPOINT,
		NAREG|NBREG|NASL|NBSL,	RESC2,
		"	movw	AL,A1\n	extl	A1\n	movl	A1,A2\n",


SCONV,	INTAREG,
	STAREG,	TUCHAR,
	SANY,	TSCALAR,
		0,	RLEFT,
		"	andl	#255,AL\n",

SCONV,	INTAREG,
	STAREG,	TUSHORT,
	SANY,	TINT|TUNSIGNED|TPOINT,
		0,	RLEFT,
		"	andl	#65535,AL\n",

/* the template below was replaced with the following 
     FIVE templates from Unisoft to fix bugreport #52.
SCONV,	INAREG|INTAREG,
	EA,	TINT|TUNSIGNED|TPOINT|TSHORT|TUSHORT,
	SANY,	TSHORT|TUSHORT|TCHAR|TUCHAR,
		0,	RLEFT,
		"ZT",
*/
/* from Unisoft */
SCONV,	INTAREG,
	EA,	TUSHORT,
	SANY,	TINT|TUNSIGNED|TPOINT,
		NAREG|NASL,	RESC1,
		"	movw	AL,A1\n	andl	#0xffff,A1\n",

/*from Unisoft */
SCONV,	INTBREG,
	STAREG,	TUSHORT,
	SANY,	TINT|TUNSIGNED|TPOINT,
		NBREG,	RESC1,
		"	andl	#0xffff,AL\n\tmovl\tAL,A1\n",

/* from Unisoft */
SCONV,	INAREG|INBREG,
	SNAME|SOREG|SCON|SAREG|SBREG,	TINT|TUNSIGNED|TPOINT|TSHORT|TUSHORT,
	SANY,	TSHORT|TUSHORT|TCHAR|TUCHAR,
		0,	RLEFT,
		"ZT",

/* and we had to add the following later to fix bugreport #54 */
SCONV,	INTAREG|INTBREG,
	SNAME|SOREG|SCON|STAREG|STBREG,	TINT|TUNSIGNED|TPOINT|TSHORT|TUSHORT,
	SANY,	TSHORT|TUSHORT|TCHAR|TUCHAR,
		0,	RLEFT,
		"ZT",

/* from Unisoft */
SCONV,	INTAREG|INTBREG,
	EA,	TCHAR,
	SANY,	TPOINT,
		NAREG|NASL,	RESC1,
		"	movb	AL,A1\n	extw	A1\n	extl	A1\n",

/* from Unisoft */
SCONV,	INTAREG|INTBREG,
	EA,	TUCHAR,
	SANY,	TPOINT,
		NAREG|NASL,	RESC1,
		"	movb	AL,A1\n	andl	#0xff,A1\n",

#ifdef notdef
SCONV, 	INBREG|INTBREG,
	SBREG|STBREG, TPOINT|TINT,
	SANY,	TPOINT|TINT,
		0,	RLEFT,
		"",
#endif

#ifndef DOUBLES32BITS

SCONV,	INAREG|INTAREG,
	EA,	TDOUBLE,
	SANY,	TFLOAT,
		0,	RLEFT,
		"",

/* altered template below for double register (2* removed, A2 -> U1) GB */
SCONV,	INAREG|INTAREG,
	EA,	TFLOAT,
	SANY,	TDOUBLE,
		NAREG|NASR,	RESC1,
		"	movl	AL,A1\n	clrl	U1\n",


/* added to fix bugreport #10.  L version suggestion.
   GB SGI (4/28/83).
*/
SCONV,  FORARG,
	EA,	TFLOAT,
	SANY,	TDOUBLE,
		0,	RNULL,
		"	clrl	Z-\n	movl	AL,Z-\n",

#endif

/* SOREG removed from right side of following template per Unisoft
   suggestion to fix bug #14. (GB) 8/29/83 */
STASG,	FOREFF,
	SNAME|SOREG,	TANY,
	SCON|SBREG,	TANY,
		0,	RNOP,
		"ZS",

STASG,	INTBREG|INBREG,
	SNAME|SOREG,	TANY,
	STBREG,	TANY,
		0,	RRIGHT,
		"ZS",

STASG, INBREG|INTBREG,
	SNAME|SOREG,	TANY,
	SCON|SBREG,	TANY,
		NBREG,	RESC1,
		"ZS	movl	AR,A1\n",

INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TINT|TUNSIGNED|TPOINT,
		0,	RNOP,
		"	.long	CL\n",

INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TSHORT|TUSHORT,
		0,	RNOP,
		"	.word	CL\n",

INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TCHAR|TUCHAR,
		0,	RNOP,
		"	.byte	CL\n",

	/* Default actions for hard trees ... */

# define DF(x) FORREW,SANY,TANY,SANY,TANY,REWRITE,x,""

UNARY MUL, DF( UNARY MUL ),

INCR, DF(INCR),

DECR, DF(INCR),

ASSIGN, DF(ASSIGN),

STASG, DF(STASG),

OPLEAF, DF(NAME),

OPLOG,	FORCC,
	SANY,	TANY,
	SANY,	TANY,
		REWRITE,	BITYPE,
		"",

OPLOG,	DF(NOT),

COMOP, DF(COMOP),

INIT, DF(INIT),

OPUNARY, DF(UNARY MINUS),


ASG OPANY, DF(ASG PLUS),

OPANY, DF(BITYPE),

FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	"help; I'm in trouble\n" };
