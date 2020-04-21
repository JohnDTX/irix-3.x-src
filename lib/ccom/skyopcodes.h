
/*	
 *
 *		fpopcodes.h - definitions for the sky h/w registers
 *					and opcodes
 *
 *
 */


/* single precision compiler-generated functions */
#define	SKY_SPADD		0x1001
#define SKY_SPSUB		0x1007
#define	SKY_SPMUL		0x100b
#define SKY_SPDIV		0x1013
#define	SKY_SPMOD		0x1030
#define SKY_SPCMP		0x105d

/* double precision compiler-generated functions */
#define	SKY_DPADD		0x1002
#define SKY_DPSUB		0x1008
#define	SKY_DPMUL		0x100c
#define SKY_DPDIV		0x1014
#define SKY_DPCMP		0x105e

/* conversion routines */
#define	SKY_SPDP			0x1042	/* float -> long float */
#define SKY_DPSP			0x1043	/* long float -> float */
#define SKY_SPFLOAT		0x1024  /* integer -> float */
#define SKY_DPFLOAT		0x1044  /* integer -> long float */
#define SKY_DPFIX		0x1045  /* long float -> integer */
#define SKY_SPFIX		0x1027  /* float -> integer */


/* transcendental functions for the math library */
#define SKY_COS			0x1028	/* single precision cosine */
#define SKY_SIN			0x1029	/* single precision sine */
#define SKY_TAN			0x102A	/* single precision tangent */
#define SKY_ATAN			0x102B	/* single precision arctan */
#define SKY_EXP			0x102C	/* e ^ X */
#define SKY_LN			0x102D	/* ln (x) */
#define SKY_POW			0x102E	/* a1^a2 */
#define SKY_SQRT			0x102F	/* sqrt(x) */
#define SKY_MOD			0x1030	/* b1 = a1 - n*a2 */


/* other functions */
#define SKY_IMUL			0x1046	/* 32-bit integer multiply */
#define SKY_CTXSV		0x1040	/* context save function - read 8 longwords*/
#define SKY_CTXRSTR		0x1041	/* context restore function - write 8 */

