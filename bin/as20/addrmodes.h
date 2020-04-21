/*
 *
 *    addrmodes.h	
 *
 *	addressing mode constants for the SGI 68020 virtual
 *	memory assembler.
 *
 *	The usable addressing modes for a particular operand are
 *	encoded in a longword, with a single bit for each mode.  The
 *	longword containing the bits for the addressing modes legal
 *	for a particular operand in an instruction format are then
 *	AND'd with the longword describing the addressing mode of the
 *	current operand.	
 *	    
 */

/* class one - simple register addressing modes. */
#define A_DN 0x80000000
#define A_AN 0x40000000
#define A_CCR 0x20000000
#define A_SR 0x10000000

/* class two - simple address register indirect, predecr, postincr and
   with disp.
*/
#define A_ANIND 0x08000000
#define A_ANINDPI 0x04000000
#define A_ANINDPD 0x02000000
#define A_ANINDDISP 0x01000000

/* class three - address register with index, singly and doubly indirect */
/* 	with byte displacement */
#define A_ANIXBD 0x00800000
#define A_ANIXBD_BIT 24
/* 	with base displacement, NO indirection */
#define A_ANIXNI 0x00400000
#define A_ANIXNI_BIT 23
/* 	pre-indexed with base displacement, indirect after adding outer disp  */
#define A_ANINDPREX 0x00200000
#define A_ANINDPREX_BIT 22
/* 	with base displacement, indirect before adding outer disp */
#define A_ANINDPOSTX 0x00100000
#define A_ANINDPOSTX_BIT 21
/* class four - absolute A_*/
#define A_IS_ABSW	0x00080000
#define A_CANBE_ABSW	A_IS_ABSW

#define A_IS_ABSL	0x00040000
#define A_CANBE_ABSL (A_IS_ABSL|A_IS_ABSW)

#define A_IS_DISPB 0x00020000
#define A_CANBE_DISPB A_IS_DISPB

#define A_IS_DISPW 0x00010000
#define A_CANBE_DISPW (A_IS_DISPB|A_IS_DISPW)

#define A_IS_DISPL 0x00008000
#define A_CANBE_DISPL (A_IS_DISPB|A_IS_DISPW|A_IS_DISPL)

/* class five A_- immediate modes */
#define A_IS_IMM0	0x00004000
#define A_CANBE_IMM0	A_IS_IMM0
#define A_IS_IMM1T7	0x00002000
#define A_CANBE_IMM0T7	(A_IS_IMM0|A_IS_IMM1T7)
#define A_IS_IMM8  0x00001000
#define A_CANBE_IMM1T8	(A_IS_IMM1T7|A_IS_IMM8)
#define A_IS_IMM9T31 0x00000800
#define A_CANBE_IMM0T31 (A_IS_IMM0|A_IS_IMM1T7|A_IS_IMM8|A_IS_IMM9T31)
#define A_IS_IMMB	0x00000400
#define A_CANBE_IMMB	(A_IS_IMM0|A_IS_IMM1T7|A_IS_IMM8|A_IS_IMM9T31|A_IS_IMMB)
#define A_IS_IMMW	0x00000200
#define A_CANBE_IMMW	(A_IS_IMM0|A_IS_IMM1T7|A_IS_IMM8|A_IS_IMM9T31|A_IS_IMMB|A_IS_IMMW)
#define A_IS_IMML	0x00000100
#define A_CANBE_IMML	(A_IS_IMM0|A_IS_IMM1T7|A_IS_IMM8|A_IS_IMM9T31|A_IS_IMMB|A_IS_IMMW|A_IS_IMML)

#define A_SPECIALREG	0x00000080
#define A_DOUBLEREG	0x00000040
#define A_DOUBLEIREG	0x00000020
/* class six -  pc indirect */
#define A_PCDISP  0x00000010

/* class seven - pc indexed */
#define A_PCIXBD	0x00000008
#define A_PCIXBD_BIT 4
#define A_PCIXNI	0x00000004
#define A_PCIXNI_BIT 3
#define A_PCINDPREX 0x00000002
#define A_PCINDPREX_BIT 2
#define A_PCINDPOSTX 0x00000001
#define A_PCINDPOSTX_BIT 1

#define A_NONE 0
#define A_ANYREG (A_AN|A_DN)
#define A_ANYIND (A_ANIND|A_ANINDPI|A_ANINDPD|A_ANINDDISP)
#define A_ANYINDEX (A_ANIXBD|A_ANIXNI|A_ANINDPREX|A_ANINDPOSTX)
#define A_ANYABS (A_CANBE_ABSL)
#define A_ANYDISP (A_CANBE_DISPL)
#define A_ANYIMM (A_CANBE_IMML)
#define A_ANYPC  (A_PCDISP|A_PCIXBD|A_PCIXNI|A_PCINDPREX|A_PCINDPOSTX)

#define A_ANY	 (A_ANYREG|A_ANYIND|A_ANYINDEX|A_ANYABS|A_ANYIMM|A_ANYPC)
#define A_ANYNOTAN (A_ANYIND|A_ANYINDEX|A_ANYABS|A_ANYIMM|A_ANYPC|A_DN)
#define A_ANYDATA (A_ANYIND|A_ANYINDEX|A_ANYABS)
#define A_ANYALTERABLE (A_ANYREG|A_ANYDATA)

#define A_ANYB	 (A_ANYREG|A_ANYIND|A_ANYINDEX|A_CANBE_IMMB|A_ANYABS|A_ANYPC)
#define A_ANYBNOTAN (A_ANYIND|A_ANYINDEX|A_CANBE_IMMB|A_ANYABS|A_ANYPC|A_DN)

#define A_ANYW	 (A_ANYREG|A_ANYIND|A_ANYINDEX|A_ANYABS|A_CANBE_IMMW|A_ANYPC)
#define A_ANYWNOTAN (A_ANYIND|A_ANYINDEX|A_ANYABS|A_CANBE_IMMW|A_ANYPC|A_DN)

#define A_THREEOPS 4
#define A_SINGLE 3
#define A_SRC 2
#define A_DEST 1
#define A_NONE 0

#define X_ISSDI 0x100
#define X_IS68020 0x200
#define X_ISBITFIELD 0x400
#define X_FLAG0 0x800
#define X_FLAG1 0x1000
#define X_ISJSR 0x2000
#define X_DONTCONDENSE 0x4000

/* in the 3.6 version, the flag below forced the printing of a user-warning
   located in a 16-entry string array if the instruction was selected.
*/
#define X_USERMESSAGE 0x8000
#define X_NUMSGS 0x3
#define X_UMSG(flags) (flags & X_NUMSGS)

/* x_umsg is the user-warning string array as indicated above.  It is
   initialized in instparse.c 
*/
char *x_umsg[];

#define NOREG (-1)
#define ADDRREG 010
#define PCREG 020
#define SRREG 021
#define CCRREG 022
#define USPREG 060
#define SFCREG 061
#define DFCREG 062
#define CACRREG 063
#define VBRREG 064
#define CAARREG 065
#define MSPREG 066
#define ISPREG 067


#define ISUNSIGNED TRUE
#define ISSIGNED FALSE
#define ENCODEATBIT6 TRUE
#define ENCODEATBIT0 FALSE
