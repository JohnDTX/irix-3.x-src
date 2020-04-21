/* diag.mic   --- microcode diagnostics and miscellaneous	<< UC4 >>
 *			ADDED NEW FEEDBACK as FEEDBACK_NEXT
 *	contains:
 *		SAVE_REGS
 *		UNSAVE_REGS
 *		FEEDBACK_NEXT		(limit,[data]*,EOF1,EOF2,EOF3)
 *		    ENTER_FEEDBACK
 *		    OUTSEND
 *		LOADRAM			(addr, <n> data words)
 *		DUMPRAM			(addr)
 *		READ_CHARPOSN
 *		BPC_COMMAND
 *		BPC_READBUS
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

diag()
{
newfile("diag.c");

/*		SAVE and UNSAVE REGS
 *
 *	save state of 2903 registers in scratch ram
 */

label(SAVE_REGS)
	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE);_ES

	_NS RAM(RAMWR,0,INC); DOJSUB(STM1UP);_ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH);_ES

label(UNSAVE_REGS)
	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE);_ES

	_NS RAM(RAMRD,0,INC); DOJSUB(LM1UP);_ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH);_ES


label(FEEDBACK_NEXT)
/*	new feedback-everything mode
 *	input upper address limit of buffer
 *	saves GE outputs in ram until magic 3-word EOF sequence sent
 *	returns EOF sequence as well
 */
#define _WK	4
#define _ADR	5
#define _CT	6
#define _WK2	8
#define _EOF	9
#define _STARTMAR 10
#define _EOFSAVE  11
/*      _LIMIT    12	from consts.h
	_QUITFLAG 13
 */

	_NS SETROP(0,NONE); SETSOP(NONQOP,_QUITFLAG,RAMNOP);
	 ALUOP(ZERO); YQ(FF,LDQ,REGWRE); _ES
				/* say "don't quit after buffer overflow */
				/* clear Q reg for rightshift of EOF	*/

	_NS LOADMAR(_MASKEOL); CONST(_MASKEOL); _ES
						/* contains upper buf limit */

	_NS IMMRAM(SUBSRC,5,_LIMIT,HOLD); CONST(5); _ES	/* leave a wee space*/

label(ENTER_FEEDBACK)
	_NS LOADIMM(_STARTMAR,_STARTLIST); LDMAR; CONST(_STARTLIST); _ES
					/* start at bottom of useable space */

	_NS REGHOLD; LOADDI(UCOUNT); SEQ(LDCT); CONST(_EOFLENGTH); _ES
			 /* counter will be used to detect n-word EOF token */

	_NS LOADIMM(_EOF,_FBCEOF1); CONST(_FBCEOF1); _ES
				/* save special EOF token in a reg	*/

	_NS REGREG(MOVE,_EOF,_EOFSAVE); _ES	/* duplicate  */

	_NS REGREG(MOVE,_STARTMAR,_CT); DOJUMP(NEXT_FEEDWD); _ES
			/* _CT holds duplicate for inside loop; jump in */

label(FEEDWD_LOOP)
	_NS REGREG(INCR,_CT,_CT); INCMAR; COND(IFEQ); DOJUMP(EOFTEST); _ES
			/* increment MAR, duplicate;
			 * if wd matches EOF, advance pattern detector
			 */
	_NS REGREG(MOVE,_EOFSAVE,_EOF);
	 LOADDI(UCOUNT); SEQ(LDCT); CONST(_EOFLENGTH); _ES
			/* if no match, reload 1st EOF pattern */

	_NS REGCOMP(EQ,_EOF,_WK); _ES	/* retest in case pattern reset */

	_NS REGHOLD; COND(IFEQ); DOJUMP(EOFTEST); _ES

label(NEXT_FEEDWD)
	_NS REGCOMP(GT,_CT,_LIMIT); GEGET; _ES
				/* compare MAR with (lower) limit */

	_NS REGHOLD; COND(IFGT); DOJSUB(DUMP_FEEDDATA); _ES
				/* if full, dump the buffer to host	*/

	_NS LOADREG(_WK,ALL16,NOMORE); _ES
				/* move data to a register */

	_NS SETROP(_EOF,NONE); SETSOP(NONQOP,_WK,RAMWR); ALUOP(SUBRSC);
	 FTODO;  DOJUMP(FEEDWD_LOOP); _ES
				/* write to RAM; compare w/ EOF; loop */

label(EOFTEST)
	_NS SETROP(_EOF,NONE); SETSOP(NONQOP,_EOF,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE);
	 SEQ(RPCT); NEXT(NEXT_FEEDWD); _ES
			/* downshift the EOF pattern;
			 * until last pattern detected, return to loop */

	_NS REGHOLD; DOJSUB(DUMPFEED_DOIT); _ES
			/* go dump what's left in RAM */

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES	/* then exit */


label(DUMP_FEEDDATA)	/* subroutine to dump buffer to host */
	_NS REGCOMP(EQ,_EOFSAVE,_EOF); _ES
				/* if EOF is in mid-sequence... */

	_NS REGHOLD; COND(IFNQ); SEQ(RETN); _ES    /* keep stashing words */

label(DUMPFEED_DOIT)
	_NS LOADDI(UCONST); MICROCONST(_INTFEEDBACK); SETROP(0,ALL16);
	 ALUOP(RONLYOP,P0); FTODO;
	 LDOUT; INTERRUPTHOST; CONST(_INTFEEDBACK);_ES
			/* send interrupt code to host as signal	*/

	_NS SETROP(_STARTMAR,NONE); SETSOP(NONQOP,_CT,RAMNOP);
	 ALUOP(SUBSRC); FTODO; LDOUT; INTERRUPTHOST; _ES
						/* send wordcount to host */

	_NS REGREG(MOVE,_STARTMAR,_ADR); LDMAR;
	 LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP); _ES	
					/* set up MAR; show count to host */

	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES
						 /* load the counter */

	_NS REGHOLD; DOTOMAR(DEC); SEQ(RPCT); NEXT(DFDLP); _ES
		/* a HACK to decrement the counter by one */

label(DFDLP)
	_NS REGHOLD; INCMAR;	/* inc MAR (here because of INTERRPT) */
	 LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP); _ES   /* show data */

	_NS RAM(RAMRD,_WK2,HOLD); LDOUT; INTERRUPTHOST;
	 SEQ(RPCT); NEXT(DFDLP); _ES	/* give host a word from buffer; lp */

	_NS REGREG(MOVE,_EOFSAVE,_EOF);
	 LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP); _ES
			/* restore EOF sequence; show host last wd of data */

	_NS REGREG(MOVE,_QUITFLAG,_QUITFLAG);
	 LOADDI(UCOUNT); SEQ(LDCT); CONST(_EOFLENGTH); _ES
		/* look at quit flag */
		/* since we guarantee no dump in mid-EOF sequence,
		 * it's OK to reset the counter here  */

	_NS REGREG(MOVE,_STARTMAR,_CT); LDMAR; COND(IFZ); SEQ(RETN); _ES
		/* re-start buffer at bottom of scratch; return if flag 0 */

	_NS REGHOLD; SEQ(TEST); _ES	/* pop subroutine addr off stack */

	_NS REGHOLD; DOJUMP(DISPATCH); _ES	/* exit feedback mode */


label(LOADRAM)	/* stuff data into scratch */

	_NS LOADREG(_ADR,ALL16,NOMORE); DOTOMAR(LOAD); _ES	/* get adr */

	_NS REGREG(ZERO,0,_TEMP); GEGET; _ES	/* input no of wds to store */

	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(SUBRS); FTODO; LDOUT; _ES
				/* no. of words to write minus 1 to outreg */

	_NS LOADDI(OUTPUTCOUNT); SETROP(0,ALL16);
	 ALUOP(MOVE); FTODO; SEQ(LDCT); _ES	/* move to counter */

	_NS REGHOLD; GEGET; DOJUMP(INTO_RAM); _ES
						/* subr in attributes.c */


label(DUMPRAM)	/*feed back 16 words from specified scratch ram address */

	_NS LOADREG(_ADR,ALL16,NOMORE); DOTOMAR(LOAD);_ES
						/* get ram adr		*/

	_NS LOADDI(UCONST); MICROCONST(_INTDUMP); SETROP(0,ALL16);
	 ALUOP(RONLYOP,P0); FTODO;
	 LDOUT; INTERRUPTHOST; CONST(_INTDUMP);_ES
		/* send interrupt code to host  */

	_NS LOADDI(UCOUNT); REGHOLD; DOPUSH(15);_ES

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0); FTODO;
	 LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES
		/* send 16 items to host; pauses between		*/

	_NS REGHOLD; SEQ(LOUP); _ES

	_NS REGHOLD; DOJUMP(GETCMD);_ES

/*================================================================*/

/*			BPC DIAGNOSTICS
 *
 *	These commands require that the SUBSTCODE bit in the GE multibus
 *	control register be switched on, to allow the BPC command code
 *	to be derived from the DI bus.
 */

label(BPC_COMMAND)
/*
 * load a single specified input register on the Update Control or
 * issues a single command.
 * Input word of data in case it's needed; then the BPC code.
 * If sign bit of BPC code set, it's a command, otherwise strobe.
 */
	_NS LOADREG(_WK,ALL16,NOMORE);_ES
		/* this innocent command loads the data word and saves it   */

	_NS LOADIMM(_WK2,16); GEGET; CONST(16); _ES	/* REVX mask */
	    /* waits for command code to be placed in the input register.
	     * separating the above two instructions allows this command to
	     *  be properly used in SUBST mode.
	     */

	_NS LOADREG(_TEMP,ALL16,NOMORE); _ES	/* look at command code */

	_NS REGHOLD; COND(IFNEG); DOJUMP(BPC_CMD); _ES

	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_WK2,RAMNOP);
	 ALUOP(ANDRS); FTODO; _ES	/* test REVX bit */

	_NS REGHOLD; COND(IFNZ); DOJUMP(REV_STROBE); _ES

	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(LOADXS);_ES
	   /* this slightly less innocent command moves the data to the */
	   /* output register from the ALU and loads the command register */
	   /* from the lsb's of the input register.  The "LOADXS" is	*/
	   /* just a fake to get the strobe to assert ( next state )    */
	   /* assuming the aforementioned SUBSTCODE is set right.	*/
	   /* issue the strobe sans REVX bit	*/

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(REV_STROBE)
	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(LOADCONFIG);_ES	
					/* strobe w/ revx set */
	
	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(BPC_CMD)
	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_WK2,RAMNOP);
	 ALUOP(ANDRS); FTODO; _ES	/* test the bit in the input reg */

	_NS REGHOLD; COND(IFNZ); DOJUMP(REV_COMMAND);_ES

	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(OCT0VECT);_ES
				/* issue the command sans REVX bit	*/

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(REV_COMMAND)
	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(OCT0RVECT);_ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES


label(BPC_READBUS)
/* Similar to BPC_COMMAND, but feeds back what's on BPCBUS after operation
 * completed.
 */
	_NS LOADIMM(_TEMP,_INTBPCBUS); LDOUT;INTERRUPTHOST; CONST(_INTBPCBUS);
	 GEGET; _ES	/* warn user of impending feedback! oooh... */

	_NS LOADREG(_WK,ALL16,NOMORE);_ES	/* save data */

	_NS LOADIMM(_WK2,16); GEGET; CONST(16); _ES	/* REVX mask */

	_NS LOADREG(_TEMP,ALL16,NOMORE); _ES	/* look at command code */

	_NS REGHOLD; COND(IFNEG); DOJUMP(BPC_READ_CMD); _ES

	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_WK2,RAMNOP);
	 ALUOP(ANDRS); FTODO; _ES	/* test REVX bit */

	_NS REGHOLD; COND(IFNZ); DOJUMP(REV_READ_STROBE); _ES

	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(LOADXS);_ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); LDOUT;
	 READBPCBUS;				/* magic FBCCODE	*/
	 COND(IFFALSE); SEQ(CJPP);_ES
			/* load BPC bus into TEMP (host can spy now) */

	_NS REGREG(MOVE,_TEMP,_TEMP); LDOUT; INTERRUPTHOST; DOJUMP(GETCMD);_ES
			/* load data  into out reg, interrupt; exit	*/

label(REV_READ_STROBE)
	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(LOADCONFIG);_ES	
					/* strobe w/ revx set */
	
	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); LDOUT;
	 READBPCBUS;				/* magic FBCCODE	*/
	 COND(IFFALSE); SEQ(CJPP);_ES
			/* load BPC bus into TEMP (host can spy now) */

	_NS REGREG(MOVE,_TEMP,_TEMP); LDOUT; INTERRUPTHOST; DOJUMP(GETCMD);_ES
			/* load data  into out reg, interrupt; exit	*/

label(BPC_READ_CMD)
	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_WK2,RAMNOP);
	 ALUOP(ANDRS); FTODO; _ES	/* test the bit in the input reg */

	_NS REGHOLD; COND(IFNZ); DOJUMP(REV_READ_CMD);_ES

	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(OCT0VECT);_ES
				/* issue the command sans REVX bit	*/

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); LDOUT;
	 READBPCBUS;				/* magic FBCCODE	*/
	 COND(IFFALSE); SEQ(CJPP);_ES
			/* load BPC bus into TEMP (host can spy now) */

	_NS REGREG(MOVE,_TEMP,_TEMP); LDOUT; INTERRUPTHOST; DOJUMP(GETCMD);_ES
			/* load data  into out reg, interrupt; exit	*/

label(REV_READ_CMD)
	_NS LOADDI(INRJUST); REGREG(MOVE,_WK,_WK); BPC(OCT0RVECT);_ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); LDOUT;
	 READBPCBUS;				/* magic FBCCODE	*/
	 COND(IFFALSE); SEQ(CJPP);_ES
			/* load BPC bus into TEMP (host can spy now) */

	_NS REGREG(MOVE,_TEMP,_TEMP); LDOUT; INTERRUPTHOST; DOJUMP(GETCMD);_ES
			/* load data  into out reg, interrupt; exit	*/


label(READ_CHARPOSN)
	_NS LOADIMM(_WK2,_INTCHPOSN); LDOUT; INTERRUPTHOST;
	 CONST(_INTCHPOSN); _ES

	_NS LOADMAR(_CHARPOSN); CONST(_CHARPOSN); _ES
		/* finally,  send current char position back to host */

	_NS RAM(RAMRD,_WK2,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES

	_NS RAM(RAMRD,_WK2,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES

	_NS LOADMAR(_PASSCHARS); CONST(_PASSCHARS); _ES

	_NS RAM(RAMRD,_WK2,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES
}
