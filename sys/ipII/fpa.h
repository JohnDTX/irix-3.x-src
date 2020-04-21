/*
** $Source: /d2/3.7/src/sys/ipII/RCS/fpa.h,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:30:45 $
**
** include file for the FPA board.
** NOTE: this is not a complete list of opcodes or other values for the
**       FPA.  It is only the information needed to initialize/save/restore
**       and handle bus errors
*/
#define fpaopcodeL(opc,reg)	    *((u_long *)(0x8000|SEG_FPA|((opc)<<8)|((reg)<<4)))
#define fpaopcodeB(opc,dst,src) *((u_char *)(0x8000|SEG_FPA|((opc)<<8)|((dst)<<4)|(src)))

#define FPA_FILLER	0	/* dummy filler used with some opcode	*/

/*
** Command opcodes
*/
#define	FPA_DBLEHI	3
#define FPA_DBLELO	4
#define FPA_OREG	6
#define FPA_CREG	7
#define FPA_EREG	8
#define FPA_MREG	9

/*
** defines for the Error Register (as well as the Error Mask register)
*/
#define FPA_EREG_ZDIV		0
#define FPA_EREG_OVFL		1
#define FPA_EREG_UNDRFL		2
#define FPA_EREG_DENORM		3
#define FPA_EREG_NAN		4
#define FPA_EREG_ILLOP		5
#define FPA_EREG_INEXACT	6
