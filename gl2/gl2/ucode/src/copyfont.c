/* copyfont.c	--	UC Font RAM management		<< GF2/UC4 >>
 *
 *	COPY_FONT (FROM, TO, NWDS)
 *		copy a block of NWDS (bytes) from FROM to TO
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

#define _SRC	4
#define _DST	5
#define _NWDS	6
#define _DIR	8

copyfont()
{
newfile("copyfont.c");

label(COPY_FONT)
/*	_NS CLRZERO; GEGET; _ES  */

	_NS LOADREG(_SRC,ALL16,MORE); _ES

	_NS LOADREG(_DST,ALL16,MORE); _ES

	_NS LOADREG(_NWDS,ALL16,NOMORE); _ES

	_NS REGREG(SUBSR,_ZERO,_NWDS); LDOUT; _ES
				/* make NWDS-1; to outreg for counting */

	_NS REGCOMP(GT,_SRC,_DST); COND(IFNEG); DOJUMP(COP_EXIT); _ES	
				/* if (nwds-1) < 0 leave now */

	_NS REGREG(INCR,_ZERO,_DIR); COND(IFGT); DOJUMP(DO_FCOPY); _ES
				/* assume SRC above DST -- DIR is +1 */

	_NS REGREG(SUBRSC,_ZERO,_DIR); _ES
			/* SRC < DST -- copy backwards from end of block */

	_NS REGREG(ADD,_NWDS,_SRC); _ES

	_NS REGREG(ADD,_NWDS,_DST); _ES

label(DO_FCOPY)
	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES

	_NS REGREG(MOVE,_SRC,_SRC); BPC(LOADFA); _ES

	_NS REGREG(ADD,_DIR,_SRC); 	/* update source address */
	 BPC(SETADDRS); _ES	/* this noop causes FA to be loaded */

	_NS REGHOLD; BPC(READFONT); _ES		/* this performs the read */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); LDOUT;
	 READBPCBUS;				/* magic FBCCODE	*/
	 COND(IFFALSE); SEQ(CJPP); _ES
				/* read font data, stash in TEMP	*/

	_NS REGREG(MOVE,_DST,_DST); BPC(LOADFA); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGREG(MOVE,_TEMP,_TEMP); BPC(WRITEFONT); _ES
						/* write to dest adr */

	_NS REGREG(ADD,_DIR,_DST); SEQ(LOUP); _ES   /* update dest & loop */

label(COP_EXIT)
	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES
}
