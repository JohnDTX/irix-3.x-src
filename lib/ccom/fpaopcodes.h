
/*	
 *
 *		fpaopcodes.h - 
 *
 *		     	definitions for the opcodes for the juniper fpa.
 *			These opcodes are for moving the
 *			data on the bus as the last operand.  If
 *			the operation has two operands, the second
 *			operand has been moved to register one on
 *			the board, and this register should be 
 *			updated with the result.  
 *
 *
 */


/* single precision compiler-generated functions */
#define	FPA_SPADD		0x8000+0xb00+0x10
#define FPA_SPSUB		0x8000+0xf00+0x10
#define	FPA_SPMUL		0x8000+0x1700+0x10
#define FPA_SPDIV		0x8000+0x1b00+0x10
#define FPA_SPCMP		0x8000+0x3200+0x10

/* double precision compiler-generated functions */
#define	FPA_DPADD		0x8000+0xd00+0x10
#define FPA_DPSUB		0x8000+0x1100+0x10
#define	FPA_DPMUL		0x8000+0x1900+0x10
#define FPA_DPDIV		0x8000+0x1d00+0x10
#define FPA_DPCMP		0x8000+0x3300+0x10

/* conversion routines */
#define	FPA_SPDP		0x8000+0x2200 	/* float -> long float */
#define FPA_DPSP		0x8000+0x2400	/* long float -> float */
#define FPA_SPFLOAT		0x8000+0x2600  	/* integer -> float */
#define FPA_DPFLOAT		0x8000+0x2a00 	/* integer -> long float */

#ifdef NOTDEF  /* these operations take special handling of the mode reg */
#define FPA_DPFIX		0x8000+0x2c00 	/* long float -> integer */
#define FPA_SPFIX		0x8000+0x2800 	/* float -> integer */
#else
#define FPA_DPFIX		0
#define FPA_SPFIX		0
#endif

/* bit masks for getting at the various fields */
#define FPA_OPCODE_MASK	0x7f00
#define FPA_OPERAND0_MASK	0x00f0
#define FPA_OPERAND1_MASK	0x000f
#define FPA_OPERANDS_MASK	0x00ff

/* operation codes for simple write and read. */
#define FPA_DMOVEMSL	0x8300
#define FPA_DMOVELSL	0x8400
#define FPA_SMOVE	0x8100
