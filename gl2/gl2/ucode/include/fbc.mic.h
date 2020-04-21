/* fbc.mic.h */

#define LDOUT   DOTOOUTREG
#define LDMAR   DOTOMAR(LOAD)
#define INCMAR  DOTOMAR(INC)
#define DECMAR  DOTOMAR(DEC)
#define HIBYTE   YTODO(HI8)
#define SWAPBYTE YTODO(HI8)
#define GEGET	GEOMENGDATA

#define YQ(a,b,c)   FTOYANDQ(a,b,c)
#define	FTODO 	FTOYANDQ(FF,OLDQ,REGWRD);YTODO(ALL16)
						/* don't write reg file	*/

#define REGHOLD	REGREG(RONLYOP,P0,0,0)

#define BPC(cmd) DOTOOUTREG;BPCCMD(cmd)

/* redefine 2903 opcodes to be like PP definitions, subsuming "P0" and "P1"
/*	Can then use REGREG(ADDC,2,3) instead of REGREG(ADDOP,P0,2,3)
*/
#define ADD	ADDOP,P0
#define ADDC	ADDOP,P1
#define SUBRS	SUBRSOP,P0
#define SUBRSC	SUBRSOP,P1
#define SUBSR	SUBSROP,P0
#define SUBSRC	SUBSROP,P1
#define NAND	NANDOP,P0
#define ANDRS	ANDOP,P0
#define XOR	XOROP,P0
#define ORRS	IOROP,P0
/*	extras -- instead of procedures ZEROS,WRTZEROS, etc.	*/
#define ZERO	FLOWOP,P0
#define ONES	FHIGHOP,P0
#define MOVE	RONLYOP,P0
#define INCR	RONLYOP,P1
#define COM2	COMPROP,P1

/* pseudo-procedures for frequent sequencing ops */
#define DOJSUB(a)	SEQ(JSUB);NEXT(a)
#define DOJUMP(a)	SEQ(JUMP);NEXT(a)
#define DOPUSH(a)	SEQ(PUSH);CONST(a)
