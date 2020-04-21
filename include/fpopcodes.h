
/*	
 *
 *		fpopcodes.h - definitions for the sky h/w registers
 *					and opcodes
 *
 *
 */


/* single precision compiler-generated functions */
#define	HW_SPADD		0x1001
#define HW_SPSUB		0x1007
#define	HW_SPMUL		0x100b
#define HW_SPDIV		0x1013
#define	HW_SPMOD		0x1030
#define HW_SPCMP		0x105d

/* double precision compiler-generated functions */
#define	HW_DPADD		0x1002
#define HW_DPSUB		0x1008
#define	HW_DPMUL		0x100c
#define HW_DPDIV		0x1014
#define HW_DPCMP		0x105e

/* conversion routines */
#define	HW_SPDP			0x1042	/* float -> long float */
#define HW_DPSP			0x1043	/* long float -> float */
#define HW_SPFLOAT		0x1024  /* integer -> float */
#define HW_DPFLOAT		0x1044  /* integer -> long float */
#define HW_DPFIX		0x1045  /* long float -> integer */
#define HW_SPFIX		0x1027  /* float -> integer */


/* transcendental functions for the math library */
#define HW_COS			0x1028	/* single precision cosine */
#define HW_SIN			0x1029	/* single precision sine */
#define HW_TAN			0x102A	/* single precision tangent */
#define HW_ATAN			0x102B	/* single precision arctan */
#define HW_EXP			0x102C	/* e ^ X */
#define HW_LN			0x102D	/* ln (x) */
#define HW_POW			0x102E	/* a1^a2 */
#define HW_SQRT			0x102F	/* sqrt(x) */
#define HW_MOD			0x1030	/* b1 = a1 - n*a2 */


/* other functions */
#define HW_IMUL			0x1046	/* 32-bit integer multiply */
#define HW_CTXSV		0x1040	/* context save function - read 8 longwords*/
#define HW_CTXRSTR		0x1041	/* context restore function - write 8 */

