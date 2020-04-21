#include <stdio.h>
#include "globals.h"
#include "sym.h"
#include "tokens.h"
#include "inst.h"
#include "addrmodes.h"

#define ISUNSIGNED TRUE
#define ISSIGNED FALSE
#define ENCODEATBIT6 TRUE
#define ENCODEATBIT0 FALSE

char *x_umsg[X_NUMSGS+1] = {
	"??? unimplemented user message??? - notify vendor.",
	"illegal length specification (long) for memory rotate -\n\t\t\tword length substituted.",
	"??? unimplemented user message??? - notify vendor.",
	"??? unimplemented user message??? - notify vendor."
	};


token_t 
*decode_op();

init_inst() 
{
	register inst_t *inst=base_inst;
	/* initialize the instruction hash table */
	while (inst->name)
	{
		enter(inst,NINSTBUCKETS,inst_bucket);
		inst++;
	}
}

inst(cur_token)
token_t *cur_token;
{
	/*  
	    The line is a statement.  It should be an instruction.
	    The current token is the instruction itself.  

	      *	The instruction string is looked up in the instruction
		hash table.  If it is not found, an error message is
		generated.

	      *	The rest of the tokens are decoded as operands.  The
		result is either zero, one, or two operands of some
		form.  The operand type is contained in the operand
		addressing mode.  In some cases, notably abs.[wl] and
		immediates, an operand can be one of several addressing
		modes, ie abs.w and abs.l.  The addressing mode is
		the most restrictive class that the tokens can satisfy.
		If a set of tokens is decoded as an addressing mode class
		and the tokens cannot be resolved to a number, which is
		necessary to decide which is the most restictive in the
		class they satisfy, the least restrictive of all addressing
		modes is chosen as the operand addressing mode.  Thus
		in the instruction

			movl	#CONST,d0

		where CONST is an absolute immediate, the source addressing
		mode would be immediate.long.  

	     *	The instruction hash table entry gives a pointer into the
		instruction template table at which the instruction templates
		begin.  Each of these template entries are compared with the
		current operands until a match is found.  The first match that
		is found is chosen.  If no match is found, two LAST CHANCE
		attempts are made:

			if the source operand is ABS/DISP/IMM of length long,
			and it relies on an undefined symbol (thus its size 
			was chosen as long to be conservative), the operand
			addressing mode is reduced to the corresponding word
			length, and the templates are perused again.

			if the destination operand is <ditto>, the operand
			is modified, and the templates are run again.

	      * If the instruction template flags field indicates that the
		instruction is an mc68020 instruction, and the is68020 flag
		is NOT set, the template is skipped.

	      *	If none of these attempts produces a match, an "illegal
		operand" error is returned.

	      *	When a match is found, 	the operands are passed to the routine
		indicated in the template in the prescribed order. (for 
		binary instructions, the arg word in the template says which
		operand is to be passed as the first operand (A_SRC says to 
		pass the source first) - the name of the routine also
		is indicative. ea_imm says that the first operand is an ea
		and the second is an immediate operand).  There are also
		indicators for single operand instructions (the destination is
		always the single operand) or no-operand instructions.

	*/
	char c;
	token_t *temp;
	register inst_t *instptr;
	register template_t *templateptr; 
	template_t *templateptrsv;
	register struct operand_s *srcop, *destop;
	struct operand_s *thirdop;
	int bfops = 0;

	/* look up the instruction string in the instruction hash tables */
	if ((instptr=(inst_t *)lookup((*cur_token)->u.cptr,
		NINSTBUCKETS,inst_bucket)) == (inst_t *)0)
	{
		/* no such instruction */
		error(*cur_token,"illegal instruction");
		errors_in_statement++;
		return;
	}

	/** HHHH AAA CCC KKK!!! - need the inherant size of the instruction
	    for immediate operands.  This is indicated by the last character
	    of the instruction string
	**/
	Inherantsize = LONG;
	c = *((*cur_token)->u.cptr+(*cur_token)->length - 1);
	if (c == 'w') Inherantsize = WORD;
	else if (c == 'b') Inherantsize = BYTE;

	cur_token++;
	templateptr = instptr->templateptr;

	/* okay, we have the template chain!  Decode the operands */
	if ((temp = decode_op(cur_token,&operand0)) == (token_t *)0)
	{
		/* failed on the first operand.  Bad.*/
		error(*cur_token,"illegal form for source operand");
		errors_in_statement++;
		return;
	}
	cur_token = temp;
	 
	if (operand0.info.isbitfield) bfops++;

	if (*cur_token == (token_t )0)
	{
		if (operand0.addrmode.l == 0)
		{
			destop = (operand_t *)0;
		}
		else destop = &operand0;
		srcop = (operand_t *)0;
	}
	else {
		if ((temp = decode_op(cur_token,&operand1)) == (token_t *)0)
		{
			/* failed on the second operand. */
			error(*cur_token,
				"illegal form for destination operand");
			errors_in_statement++;
			return;
		}
		cur_token = temp;
		if (operand1.info.isbitfield) bfops++;
		if (operand1.addrmode.l)
		{
			srcop = &operand0;
			destop = &operand1;
		}
		if (*cur_token != (token_t)0)
		{
			/* special addressing mode needing three operands! */
			if ((temp = decode_op(cur_token,&extra_operand)) == (token_t *)0)
			{
				/* failed on the third operand. */
				error(*cur_token,
					"illegal form for special operand");
				errors_in_statement++;
				return;
			}
			/* the third op is missing in the template */
			thirdop = &operand0;;
			srcop = &operand1;
			destop = &extra_operand;
		}
		else thirdop = (struct operand_s *)0;

		/* in the case of bitfields, just insure that ONE of the
		   operands is a bf operand.  The fact that the correct one
		   is a bf operand is enforced in the encoding routines.
		*/
		if ((templateptr->flags & X_ISBITFIELD)&&(!bfops)) 
		{
			error(0,"instruction must have bitfield operand.");
			errors_in_statement++;
			return;
		}
		if ((bfops) && (!(templateptr->flags & X_ISBITFIELD))) 
		{
			error(0,"instruction may not have bitfield operand.");
			errors_in_statement++;
			return;
		}



	}

	templateptrsv = templateptr;
tryagain:
	templateptr = templateptrsv;
	/*  okay -- now simply look for a template match */
	do 
	{
		/* if the instruction is a 68020 instruction and we are NOT
		   on a 68020, skip the instruction template
		*/
		if ((templateptr->flags & X_IS68020)&&(!is68020)) continue;

		if ((templateptr->src != A_NONE) && 
				(srcop != (struct operand_s *) 0))
			if (!(srcop->addrmode.l & templateptr->src)) continue;
		if ((templateptr->dest != A_NONE) && 
				(destop != (struct operand_s *) 0))
		if (!(destop->addrmode.l & templateptr->dest)) continue;

		/* ugly! one last test.  This is ONLY hit for the cas 
		   instruction, which has three ops.  The first two have
		   matched.  There are two possibilities for the
		   third: A_DOUBLEREG and A_DOUBLEIREG.
		*/
		if (((templateptr->arg == A_THREEOPS )) &&
			((thirdop == (struct operand_s *)0)))
			continue;


		/* match.  Restrict the operand to one of the permissible
		   types. 
		*/
		if (destop != (struct operand_s *)0)
			destop->addrmode.l &= templateptr->dest;
		if (srcop != (struct operand_s *)0)
			srcop->addrmode.l &= templateptr->src;

		if (templateptr->flags & X_USERMESSAGE) 
			warning(0,x_umsg[X_UMSG(templateptr->flags)]);

		switch (templateptr->arg)
		{
			case A_THREEOPS:
			if ((srcop == (struct operand_s *)0) ||
			    (destop == (struct operand_s *)0) ||
			    (thirdop == (struct operand_s *)0))
				break;
		return ((*templateptr->doinst)(templateptr->inst,thirdop,srcop,
				 destop,templateptr->flags));
			case A_SRC:
			if ((srcop == (struct operand_s *)0) ||
			    (destop == (struct operand_s *)0))
				break;
		return ((*templateptr->doinst)(templateptr->inst,srcop,destop,
				templateptr->flags));
			case A_DEST:
			if ((srcop == (struct operand_s *)0) ||
			    (destop == (struct operand_s *)0))
				break;
		return ((*templateptr->doinst)(templateptr->inst,destop,srcop,
				templateptr->flags));
			case A_SINGLE:
			if ((destop == (struct operand_s *)0)) 
				break;
			if (srcop != (struct operand_s *)0)
				warning(0,"additional operands ignored");
		return ((*templateptr->doinst)(templateptr->inst,destop,
				templateptr->flags));
			case A_NONE:
			if ((srcop != (struct operand_s *)0)||
			    (destop != (struct operand_s *)0))
				warning(0,"additional operands ignored");
		return ((*templateptr->doinst)(templateptr->inst,
				templateptr->flags));

			default:
				fatal(0,"error in template table");
		}
		
		/* insufficient number of arguments */
		error(0,"insufficient operands");
		errors_in_statement++;
		return(0);

	}
	while ((++templateptr)->dest != (addr_t )0);

	/* if the operand references an undefined symbol, the
	   default is the long version of the operand, if it is
	   immediate, absolute, or displacement.  In some cases,
	   notably moveml,link, etc, the long version may not exist.  
	   Substitute the short version for each such operand and try 
	   for another match.
	*/
	if ((srcop != (struct operand_s *)0) &&
	    (srcop->addrmode.l & (A_IS_ABSL|A_IS_DISPL|A_IS_IMML))&&
	    (srcop->info.immabs_label))
	{
		unsigned long bits;
		bits = srcop->addrmode.l & (A_IS_ABSL|A_IS_DISPL|A_IS_IMML);
		srcop->addrmode.l &= ~(A_IS_ABSL|A_IS_DISPL|A_IS_IMML);
		srcop->addrmode.l |= bits<<1 ;
		goto tryagain;
	}

	if ((destop != (struct operand_s *)0) &&
	    (destop->addrmode.l & (A_IS_ABSL|A_IS_DISPL|A_IS_IMML))&&
	    (destop->info.immabs_label))
	{
		unsigned long bits;
		bits = destop->addrmode.l & (A_IS_ABSL|A_IS_DISPL|A_IS_IMML);
		destop->addrmode.l &= ~(A_IS_ABSL|A_IS_DISPL|A_IS_IMML);
		destop->addrmode.l |= bits<<1 ;
		goto tryagain;
	}


	error(0,"illegal operand");
	errors_in_statement++;
	return;

}

/* a number is decoded and placed in a particular class, as indicated
   in dotokennum.  These types and arrays are used to map these number
   classes to operand classes.

   NOTE: These number and expression-handling routines were grown as the
   need arose.  The entire section of code needs to be redone, as it is
   messy and a time hog (GB)

*/
enum numsize_e { N_ZERO,N_1T7, N_8, N_LT32, N_BYTE, N_WORD, N_LONG, N_UNKN};

int numsize_to_dispsize[8] =  { 1, 2, 2, 2, 2, 2, 3, 3 };

int numsize_to_imm[8] = 
{
	(A_IS_IMM0),
	(A_IS_IMM1T7),
	(A_IS_IMM8),
	(A_IS_IMM9T31),
	(A_IS_IMMB),
	(A_IS_IMMW),
	(A_IS_IMML),
	(A_IS_IMML)
};

int numsize_to_abs[8] =
{
	(A_IS_DISPB|A_IS_ABSW),
	(A_IS_DISPB|A_IS_ABSW),
	(A_IS_DISPB|A_IS_ABSW),
	(A_IS_DISPB|A_IS_ABSW),
	(A_IS_DISPB|A_IS_ABSW),
	(A_IS_DISPW|A_IS_ABSW),
	(A_IS_DISPL|A_IS_ABSL),
	(A_IS_DISPL|A_IS_ABSL),
};

enum numsize_e 
dotokennum(tokenptrptr,immabs)
token_t **tokenptrptr;
register struct op_immabs *immabs;
{
	/* decode the number/abs/immediate in the token stream.
	   This routine and getimmabs() may call each other recursively.
	   Numbers found in the expression list are simply added
	   into the address of the op_immabs structure.  Symbols found
	   are entered as statusers via add_statuser.
	*/
	int isminus = 0;
	long num;
	int mustbeword = 0;
	token_t *tokenptr = *tokenptrptr;

addinnumber:
	if (((*tokenptr)->tokennum == T_MINUS)|| ((*tokenptr)->tokennum == T_PLUS))
	{
		if ((*tokenptr)->tokennum == T_MINUS) isminus++;
		else isminus=0;
		tokenptr++;
	}

	if ((*tokenptr)->tokennum != T_NUMBER)
	{
		if (((*tokenptr)->tokennum == T_DOT) ||
			((*tokenptr)->tokennum == T_ALPHA))
		{
			getimmabs(&tokenptr,immabs,isminus);
		}
		else {
			*tokenptrptr = ++tokenptr;
			error((*tokenptr),"number expected");
			errors_in_statement++;
			return(N_ZERO);
		}
		num = immabs->addr;
	}
	else {
		num = buildnum(((*tokenptr))->u.cptr);
		tokenptr++;
		if (isminus) num = (-num);
		num += immabs->addr;
		immabs->addr = num;
	}
	/*  check for :w extension for abs.w addressing mode */
	if (( (*tokenptr) != (token_t )0)&&
	    ((*tokenptr)->tokennum == T_COLON))
	{
		char c;
		tokenptr++;
		if ((*tokenptr == (token_t)0) || 
		    ((*tokenptr)->tokennum != T_ALPHA) ||
		    (((c = toupper(*((*tokenptr)->u.cptr))) != 'W') &&
		    (c != 'L')))
		{
			error((*tokenptr),"illegal operand length specifier");
			errors_in_statement++;
			return(N_LONG);
		}
		tokenptr++;
		if (c == 'W') { mustbeword++; immabs->isunsigned = 1; }
	}
	if (( (*tokenptr) != (token_t )0) &&
		(((*tokenptr)->tokennum == T_PLUS) || 
		((*tokenptr)->tokennum == T_MINUS)))
		goto addinnumber;

	*tokenptrptr = tokenptr;

	/* if there are symbols attached to this immediate datum, the
	   size is unknown 
	*/
	if (immabs->sym != (struct loc_s *)0) return((mustbeword)?N_WORD:N_UNKN);
	/* for instruction operands, there are six classes of
	   numbers:
		N_ZERO	- number is zero
		N_1T7	- number is 1 to 7, incl.
		N_8	- number is 8
		N_LT32  - number is 9 to 31, incl.
		N_BYTE  - number is -128..-1, 32..127, incl. if !isunsigned
		N_BYTE  - number is 32..255, incl. if isunsigned
		N_WORD  - number is -32768..32767, excpt -128..127. if !isunsigned.
		N_WORD  - number is 128..65535, if isunsigned.
		N_LONG  - number is not in above categories.
	*/

	if (num == 0) return(N_ZERO);
	if (num >0 )
	{
		if (num <8) return(N_1T7);
		if (num == 8) return(N_8);
		if (num < 32) return(N_LT32);
	}
	if ((immabs->isunsigned)&&(num > 0))
	{
		if ((num >= 0)&&(num <= 255)) return(N_BYTE);
		if ((num >= 0)&&(num <= 65535)) return(N_WORD);
		return(N_LONG);
#ifdef NOTDEF
		if ((Inherantsize == BYTE)&&
			(num > 127)&&(num < 256)) num -= 256;
		if ((Inherantsize == WORD)&&(num > 32767)&&
			(num < 65536)) num -= 65536;
#endif
	}
	if ((num >= (-128)) && (num <= 127)) return(N_BYTE);
	if ((num >= (-32768)) && (num <= 32767)) return(N_WORD);
	return(N_LONG);
}

static int zeroop[OPERAND_S_SIZE] = {0,};

getimmabs(tokenptrptr,immabs,isminus)
token_t **tokenptrptr;
register struct op_immabs *immabs;
int isminus;
{
	/* A label has been found when
	   a number is desired.  Fill in the 
	   op_immabs structure. Return 
		1 if byte,
		2 if word,
		3 if long.
	   zero otherwise.
	*/
	token_t *tokenptr = *tokenptrptr;
	int retval=3;
	int value;
	int newsymbol;
	symtabptr sym,getsym();

	if ((*tokenptr)->tokennum == T_DOT)
	{
		/*  define a temp label to attach to the current 
		    statement as a stat_user
		*/

		if (cur_stat->lab == (symtabptr)0)
		{
			cur_stat->lab = 
		    		define_label(allocate_templab(),
					cur_stat,curstat_labaddr);
			cur_stat->lab->syminfo.istemp = 1;
		}
		sym = cur_stat->lab;
	} 
	else 
		sym = getsym(*tokenptr,&newsymbol);

	tokenptr++;

	if (((*tokenptr) != (token_t)0) &&
		(((*tokenptr)->tokennum == T_PLUS)|| 
		((*tokenptr)->tokennum == T_MINUS)))
	{
		dotokennum(&tokenptr,immabs) ;
			
	}

	/* if the symbol is absolute and has previously been 
	   defined, simply add in the value rather than carrying
	   it around.
	*/
	if ((sym->syminfo.csect == ABS_CSECT) && (!sym->syminfo.isundefined))
	{
		value = sym->addr;
		if (isminus) value = -value;
		if (immabs->isunsigned)
		{
			if ((value >=0)&&(value <= 255)) retval = 1;
			else if ((value >= 0)&&(value <= 65535)) retval = 2;
		}
		else {
			if ((value >= (-128))&&(value <= 127)) retval = 1;
			else if ((value >= (-32768))&&(value <= 32767)) retval = 2;
		}
		value += immabs->addr;
		immabs->addr = value;
	}
	else {
		/* have to add a stat_user to the
		  	symbol chain .
		*/
		if (isminus) immabs->isnrexpr = 1;
		add_statuser(sym,immabs,isminus);
		/*
		*/
	}
	*tokenptrptr = tokenptr;
	return(retval);
}

add_statuser(symbol,opseg,isneg)
symtabptr symbol;
struct op_immabs *opseg;
int isneg;
{

	/*  the passed operand segment (actually an op_immabs) relies 
	    on the passed segment, and its value must be added into
	    the code stream later.  The initial addition of a stat user
	    for the current statement is made, and this is linked into
	    the list of symbols (loc_s chain) which affect the given
	    operand segment.  WHen the instruction has been encoded,
	    and the actual location of the code segment that relies
	    on this chain of symbols is known, this chain is reworked
	    and each element attached to the symbol it relies on.
	    For the time being, the symbol it relies on is placed in
	    the pointer which will later be used to point to the statement
	    the symbol affects.  In fix_statusers, this chain is detached
	    from the operand and reworked.
	*/
	    
	register struct loc_s *loc;

	loc = allocate_locs();

	loc->next = opseg->sym;
	opseg->sym = loc;
	loc->stat = (struct generic_s *)symbol;
	if (isneg) loc->s.symisneg = 1;
}



/* a special array mapping the control registers to the bits to encode
   them
*/
unsigned short special_reg_to_bits[8] = 
	{ 0x800,0,1,2,0x801,0x802,0x803,0x804 };

/* this saves lots of time over calling toupper() */
#define mapupper(c) if (c >= 'a') c -= 'a' - 'A'

isreg(cptr)
char *cptr;
{
	/* interpret the register name string, returning

		000 - 007 for D registers.
		010 - 017 for A registers.
		PCREG	  for pc
		SRREG	  for sr
		CCRREG	  for ccr
		USPREG	  for usp.
		SFCREG	  for sfc
		DFCREG    for dfc
		VBRREG    for vbr

		* on 68020 only, else error message *
		CACRREG   for cacr
		CAARREG   for caar
		MSPREG    for msp
		ISPREG    for isp
	*/
	register char c;
	register char num;
	int retval=0;

	if (cptr == NULL) return(NOREG);
	c = (*cptr++);
	mapupper(c);

	if (!c) return (NOREG);
	num = *cptr;
	mapupper(num);

	switch (c)

	{
	case 'A':
		retval = 010;
		break;
	case 'D':
		if (num == 'F') 
		{
			c = *++cptr;
			mapupper(c);
			if ((c == 'C')&&(*(cptr+1) == 0)) return(DFCREG);
		}
		break;
	case 'S':
		if (((num) == 'R')&&(*(cptr+1) == 0))
			return(SRREG);
		if (((num) == 'P')&&(*(cptr+1) == 0))
			return(010 | 007);
		if (num == 'F') 
		{
			c = *++cptr;
			mapupper(c);
			if ((c == 'C')&&(*(cptr+1) == 0)) return(SFCREG);
		}
		return(NOREG);
	case 'C':
		if ((num) == 'C')
		{
			c = *++cptr;
			if (c == 0) return(CCRREG);
			mapupper(c);
			if ((c == 'R')&&(*(cptr+1) == 0)) return(CCRREG);
		}
		else if (num == 'A')
		{
			c = *++cptr;
			mapupper(c);
			if (c  == 'A') retval = CAARREG;
			else if (c == 'C') retval = CACRREG;
			else return(NOREG);
			c = *++cptr;
			mapupper(c);
			if ((c == 'R')&&(*(cptr+1) == 0)) 
			{
				goto check20;
			}
		}
		return(NOREG);
	case 'U':
		if ((num) == 'S')
		{
			c = *++cptr;
			mapupper(c);
			if ((c == 'P')&&(*(cptr+1) == 0)) 
				return(USPREG);
		}
		return(NOREG);
	case 'P':
		if (((num) == 'C')&&(*(cptr+1)==0))
			return(PCREG);
		return(NOREG);
	case 'V':
		if (num == 'B') 
		{
			c = *++cptr;
			mapupper(c);
			if ((c == 'R')&&(*(cptr+1) == 0)) return(VBRREG);
		}
		return(NOREG);
	case 'M':
		if (num == 'S') 
		{
			c = *++cptr;
			mapupper(c);
			if ((c == 'P')&&(*(cptr+1) == 0)) 
			{
				retval = MSPREG;
				goto check20;
			}
		}
		return(NOREG);
	case 'I':
		if (num == 'S') 
		{
			c = *++cptr;
			mapupper(c);
			if ((c == 'P')&&(*(cptr+1) == 0)) 
			{
				retval = ISPREG;
				goto check20;
			}
		}
	default:
		return(NOREG);
	}

	/* do the number */
	if ((num < '0') || (num > '7')) return(NOREG);
	else return((num - '0')|retval);

check20:
	if (is68020) return(retval);
	else {
		/* the special register name will be treated as a global name */
		warning(0,"special register not available on mc68000/mc68020");
			return(NOREG);
	}
}


#define ISENDOP(t) (((*(t)) == (token_t)0) || \
		((*(t))->tokennum == T_CM))

#define ISENDOP_ORBF(t) (((*(t)) == (token_t)0) || \
		((*(t))->tokennum == T_CM) || \
		((*(t))->tokennum == T_LBRACE))


token_t *
getbf(tokenptr,op)
token_t *tokenptr;
struct operand_s *op;
{

	/* the first token in the passed list is a left brace.  Place
	   the bitfield specifier in the passed operand structure
	   and update the tokenptr to point to the next token past
	   the bitfield specifier.
	*/

	int isneg=0;
	int num,regno;
	short s=0;

	if ((*tokenptr)->tokennum != T_LBRACE) return(tokenptr);

	tokenptr++;

	/* 
	   the format for a bitfield indicator is

		'{' <offset> ':' <width> '}'
	   
	   where the offset and width may either be a data 
	   register or a number.  The offset and width may 
	   NOT be constant symbols.

	*/

	/* get the offset */
	if ISENDOP(tokenptr) goto bferror;
	if ((*tokenptr)->tokennum == T_MINUS)
	{
		isneg++;
		tokenptr++;
	}
	if ISENDOP(tokenptr) goto bferror;
	if ((*tokenptr)->tokennum == T_NUMBER)
	{
		/* get the number */
		num = buildnum(((*tokenptr))->u.cptr);
		if (isneg) num = (-num);
		if ((num > 31)||(num < 0))
		{
			error((*tokenptr),
			  "constant bit field offset must be in the range 0-31");
			errors_in_statement++;
			*tokenptr = 0;
			return(tokenptr);
		}
		op->bf.offset = num;
		op->bf.offset_in_reg = 0;
	}
	else if ((*tokenptr)->tokennum == T_ALPHA)
	{
		/* must be register */
		if ((isneg)||((regno = isreg((*tokenptr)->u.cptr)) == NOREG)||
		    ((regno & ADDRREG)))
		{
			error((*tokenptr),
			  "bit field offset must be constant or data register");
			errors_in_statement++;
			*tokenptr = 0;
			return(tokenptr);
		}
		op->bf.offset_in_reg = 1;
		op->bf.offset_reg = regno;
	}
	tokenptr++;
	/* get the colon */
	if ISENDOP(tokenptr) goto bferror;
	if (((*tokenptr)->tokennum != T_COLON)) goto bferror;

	tokenptr++;

	/* get the width */
	if ISENDOP(tokenptr) goto bferror;
	if ((*tokenptr)->tokennum == T_MINUS)
	{
		error((*tokenptr),
		 "bit field width cannot be negative");
		errors_in_statement++;
		*tokenptr = 0;
		return(tokenptr);
	}
	if ISENDOP(tokenptr) goto bferror;
	if ((*tokenptr)->tokennum == T_NUMBER)
	{
		/* get the number */
		s = 0;
		num = buildnum(((*tokenptr))->u.cptr);
		if ((num > 31)||(num < 0))
		{
			error((*tokenptr),
			  "bit field width must be in the range 0-31");
			errors_in_statement++;
			*tokenptr = 0;
			return(tokenptr);
		}
		op->bf.width = num;
		op->bf.width_in_reg = 0;
	}
	else if ((*tokenptr)->tokennum == T_ALPHA)
	{
		/* must be register */
		if ((isneg)||((regno = isreg((*tokenptr)->u.cptr)) == NOREG)||
		    ((regno & ADDRREG)))
		{
			error((*tokenptr),
			  "bit field width must be constant or data register");
			errors_in_statement++;
			*tokenptr = 0;
			return(tokenptr);
		}
		op->bf.width_in_reg = 1;
		op->bf.width_reg = regno;
	}
	tokenptr++;

	if ((*tokenptr)->tokennum != T_RBRACE) goto bferror;
	tokenptr++;

	op->info.isbitfield = 1;
	return(tokenptr);

bferror:
	error(0,
	 "illegal bit field specification.\n\tExpected '{ <offset>:<width> }'");
	errors_in_statement++;
	*tokenptr = 0;
	return(tokenptr);
}


getindx(tokenptrptr,op)
token_t **tokenptrptr;
struct operand_s *op;
{
	/* the current token may point to an index.  If so,
	   decode it and enter it into the indexreg section of the
	   operand structure.  The tokenptr should point to the
	   first token past the index upon return.

	   An index register specifier consists of

		Rn:L*Scale

	   where 
		Rn 	- is an address or data register.
		L	- is a length, W or L
		Scale	- is a scale - 1,2,4,8
	*/

	token_t *tokenptr = *tokenptrptr;
	int scale,regno;
	char c;

	/* the next token may be an LB */
	if ((*tokenptr != (token_t)0) && ((*tokenptr)->tokennum == T_LB))
	{
		tokenptr++;
	}

	op->info.isindexed = 0;

	if ((*tokenptr == (token_t )0) || ((*tokenptr)->tokennum != T_ALPHA))
		return(0);

	if ((regno = isreg((*tokenptr)->u.cptr)) & 020)
		/* special register or no register */
		return(0);

	op->info.isindexed = 1;
	op->indexreg.regno = regno;

	tokenptr++;
	if ((*tokenptr != (token_t)0) && 
	    ((*tokenptr)->tokennum == T_COLON))
	{
		tokenptr++;
		if ((*tokenptr == (token_t)0) || 
		    ((*tokenptr)->tokennum != T_ALPHA) ||
		    (((c = toupper(*((*tokenptr)->u.cptr))) != 'W') &&
		    (c != 'L')))
		{
			error(*tokenptr,"illegal index length specifier");
			errors_in_statement++;
			return(0);
		}
		if (c == 'L') op->indexreg.islongindex = 1;
		tokenptr++;
	}

	/* there may be a scale at this point */
	if (((*tokenptr != (token_t)0) && (*tokenptr)->tokennum == T_MUL))
	{
		tokenptr++;
		if (((*tokenptr == (token_t)0) || 
		    (*tokenptr)->tokennum != T_NUMBER))
		{
			error(*tokenptr,"scale must be integral");
			errors_in_statement++;
		}
		else {
			/* get the number - a single digit */
			c = *(*tokenptr)->u.cptr;
			scale = c - '0';
			switch (scale)
			{
			case 2:	scale = 01;
				break;
	
			case 4: scale = 02;
				break;

			case 8: scale = 03;
				break;

			default:
				if (scale != 1) 
				{
					error(*tokenptr,"scale must be 1,2,4,8");
					errors_in_statement++;
				}
				scale = 0;
			}
			op->indexreg.scale = scale;
			tokenptr++;
		}
	}
	/* and the next token may be an RB */
	if ((*tokenptr != (token_t)0) && ((*tokenptr)->tokennum == T_RB))
	{
		tokenptr++;
	}

	*tokenptrptr = tokenptr;
}


token_t *
decode_op(tokenptr,op)
token_t *tokenptr;
register struct operand_s *op;
{
	/*  decode an operand from the tokenlist beginning at tokenptr.
	    Return either a pointer to the first token of the next
	    operand, if there is one, NULL, if error, or a pointer to
	    a NULL token if none.
	*/
	

	long num;
	register int regno;
	int numsize;

	/* zero the operand */
	*op = * (struct operand_s *)zeroop;

	/* position the token stream.  This is somewhat murky, but it 
	   works ok.  Checks are done to ensure that we are not sitting
	   on a comma.
	*/
	if (*tokenptr == (token_t)0)
		return(tokenptr);

	if ((*tokenptr)->tokennum == T_CM)
		tokenptr++;

	if (*tokenptr == (token_t)0)
		return(tokenptr);

	
	switch ((*tokenptr)->tokennum)
	{
		case T_IMM:	/* i.e., '#' */
			/* past the immediate indicator */
			tokenptr++;
			/* get the number (or symbol) list */
#ifdef NOTDEF
			numsize = (int)dotokennum(&tokenptr,&op->imm,ISUNSIGNED);
#endif
			op->imm.isunsigned = 1;
			numsize = (int)dotokennum(&tokenptr,&op->imm);
			/* return the correct immediate addressing mode 
			   for the size of the number found
			*/
			op->addrmode.l = numsize_to_imm[(int)numsize];
			/* in the case of immediates, the immediate operand
			   size is ALWAYS the inherant size of the instruction 
			*/
			if (Inherantsize == LONG) 
				op->addrmode.l |= A_IS_IMML;
			else if (Inherantsize == WORD) 
				op->addrmode.l |= A_IS_IMMW;
			else if (numsize < (int)N_LONG) op->info.imm_word = 1; 
			/* if the immediate operand relies on label(s), set
			   the flag bit
			*/
			if (op->imm.sym != (struct loc_s *)0)
				op->info.immabs_label = 1;
			break;

		case T_CHAR:
		case T_STRING:
			/* only legal form is single character constant */
			{
				char *end,*cptr;
				char c;
				cptr = (*tokenptr)->u.cptr;
				end = cptr + (*tokenptr)->length;
				
				if ((c = *cptr) == '\\')
				{
					doesc(cptr,end);
					if (escval == (-1)) 
					{
						error(*tokenptr,
						  "illegal character constant");
						errors_in_statement++;
						return((token_t *)0);
					}
					else c = escval;
				}
				op->addrmode.l = A_IS_IMMB;
				op->imm.addr = c;
				op->info.imm_word = 1;
			}
			break;

		case T_MINUS:	
		case T_NUMBER:
			/* [signed] number.  do the same tricks */
			numsize = (int)dotokennum(&tokenptr,&op->imm);
			op->addrmode.l = numsize_to_abs[(int)numsize];
			if (numsize < (int)N_LONG) op->info.imm_word = 1; 
			if (op->imm.sym != (struct loc_s *)0)
				op->info.immabs_label = 1;
			break;

		case T_ALPHA:	
			/* if it is a register, do the more complex
			   addressing mode
			*/
			if ((regno = isreg((*tokenptr)->u.cptr)) != NOREG)
				goto docomplex;
		case T_DOT:
			{
				int immabssz = 
					getimmabs(&tokenptr,&op->imm,FALSE);
				op->info.imm_word = 
				     ((immabssz == 1)||(immabssz == 2));
				if (op->imm.sym != (struct loc_s *)0)
					op->info.immabs_label = 1;
				if (op->info.imm_word) 
					op->addrmode.l = (A_IS_ABSW|A_IS_DISPW);
				else 
					op->addrmode.l= (A_IS_ABSL|A_IS_DISPL);
				break;
			} 

		default:
			error((*tokenptr),"cannot parse operand");
			errors_in_statement++;
			return((token_t *)0);
	}

	goto finishdecode;
	

docomplex:

	tokenptr++;
	/* a register (regno) has been seen.  Do the rest of the address
	   token.  Choices are:

		aN		- addr reg direct
		dN		- data reg direct
		rN:rM		- double register (for mul,div,cas...)
		aN@		- addr reg indirect
		aN@:aM@		- double indirect register (for cas2)
		aN@(disp)	- addr reg with disp
		aN@(disp,[Xn])	- addr reg with disp, indexed.
		aN@(disp)@(disp,[Xn]) -addr reg with disp, indirect, post-indexed
		aN@(disp,[Xn])@(disp) -addr reg with disp, indirect, pre-indexed

	how about these?
		aN@(disp)@(disp) - addr reg with disp, indirect, post-indexed
		aN@(disp)@	- addr reg with disp, indirect
		aN@(disp)@(0,[Xn])
			and the corresponding pc register combinations

		additionally, a bitfield can be specified with the <ea>.  This
		is written as { offset:width } after the operand.  Thus,
		a bitfield op of a5@ with an offset of 2 and length of 8 is

		aN@{2:8}.

		Symbols cannot appear in bitfield selectors.
	*/

	if (regno == PCREG)
		op->info.pcmode = 1;
	if ISENDOP_ORBF(tokenptr)
	{
		/* simple register */
		if (regno <= 0x7)
			op->addrmode.l = A_DN;
		else if (regno <= 0017)
			op->addrmode.l = A_AN;
		else if (regno == SRREG)
			op->addrmode.l = A_SR;
		else if (regno == CCRREG)
			op->addrmode.l = A_CCR;
		else if (regno & 020)
			op->addrmode.l = A_SPECIALREG;
		op->reg0 = regno & 0x7;
	}
	else if ((*tokenptr)->tokennum == T_COLON)
	{
		/* has to be A_DOUBLEREG */
		if (regno & 0xf8)
		{
			error((*tokenptr),
			  "double register member must be data register");
			errors_in_statement++;
			return((token_t *)0);
		}
		op->reg0 = regno & 0x7;
		tokenptr++;
		if (((*tokenptr) != (token_t)0) && 
		    ((*tokenptr)->tokennum == T_ALPHA))
		{
			if ((regno = isreg((*tokenptr)->u.cptr)) != NOREG)
			{
				tokenptr++;
				if (!ISENDOP_ORBF(tokenptr))
				{
					error((*tokenptr),
				   "illegal format for double register spec");
					errors_in_statement++;
					return((token_t *)0);
				}
				if (regno & 0xf8)
				{
					error((*tokenptr),
			  	"double register member must be data register");
					errors_in_statement++;
					return((token_t *)0);
				}
				op->reg1 = regno;
				op->addrmode.l = A_DOUBLEREG;
				goto finishdecode;
			}
		}
		error((*tokenptr),
		   "second register must follow colon in double register spec");
		errors_in_statement++;
		return((token_t *)0);
	}
	else if ((*tokenptr)->tokennum != T_IND)
	{
		error((*tokenptr),"only @ may follow register spec");
		errors_in_statement++;
		return((token_t *)0);
	}
	else {
		/*  so far, it is indirect.... */
		/*  {aN,pc}@
			   ^
		*/
		tokenptr++;
		if ((!(regno & 0x8))&&(!op->info.pcmode))
		{
			/* data registers cant be indirect */
			/* oh, yes they can, if we are on a 20 and this
			   is a DOUBLEIREG address token */
			if ((!is68020)||((*tokenptr) == (token_t)0)||
			    ((*tokenptr)->tokennum != T_COLON))
			{
				error((*(tokenptr-1)),
				   "illegal indirect data register");
				errors_in_statement++;
				return((token_t *)0);
			}
		}
		op->reg0 = regno ;
basedisp_zero:
		if ISENDOP_ORBF(tokenptr)
			/* simple address register indirect */
		{
			if (op->info.pcmode)
			{
				/* pc indirect mode is undefined */
				error(*tokenptr,
				  "pc indirect mode is illegal");
				errors_in_statement++;
			}
			op->addrmode.l = A_ANIND;
			goto finishdecode;
		}
		/* more complex indirect addressing mode */
		switch ((*tokenptr)->tokennum)
		{
		case T_COLON:
			/* has to be A_DOUBLEIREG */
			tokenptr++;
			if (((*tokenptr) != (token_t)0) && 
	    			((*tokenptr)->tokennum == T_ALPHA))
			{
				/* ok, look for register */
				if ((regno = 
				  isreg((*tokenptr)->u.cptr)) != NOREG)
				{
					/* past the register */
					tokenptr++;

					if ((*tokenptr)->tokennum == 
						T_IND)
					{
						tokenptr++;
						if (ISENDOP_ORBF(tokenptr))
						{
							/* ok! */
							op->reg1= regno;
							op->addrmode.l =
							 A_DOUBLEIREG;
						   goto finishdecode;
						}
					}
				}
			}
			error((*tokenptr),
				  "illegal format for double indirect register spec");
			errors_in_statement++;
			return((token_t *)0);

		case T_MINUS:
			op->addrmode.l = A_ANINDPD;
			if (op->info.pcmode)
			{
				/* pc predecrement mode is undefined */
				error(*tokenptr,
				  "pc predecrement mode is illegal");
				errors_in_statement++;
			}
			goto finishdecode;
		case T_PLUS:
			op->addrmode.l = A_ANINDPI;
			if (op->info.pcmode)
			{
				/* pc postincrement mode is undefined */
				error(*tokenptr,
				  "pc postincrement mode is illegal");
				errors_in_statement++;
			}
			goto finishdecode;
		case T_LP:
			/* more complex addressing mode */
			tokenptr++;
			/* has to be an base displacement at this
				  point */
			op->info.basedisp_size = 
				numsize_to_dispsize[(int)dotokennum(&tokenptr,
						&op->basedisp)];
			if (op->info.basedisp_size == 1)
			{
				/* ak! the base displacement is zero.
				   If this is the end of the
				   addressing mode, delete it! In
				   this case, we are using the
				   address register indirect mode (really).
				*/
				if (((*tokenptr == (token_t )0) || 
					(((*tokenptr)->tokennum == T_RP))) &&
					(ISENDOP_ORBF(tokenptr+1)))
				{
					tokenptr++;
					goto basedisp_zero;
				}
			}
			if (op->basedisp.sym != (struct loc_s *)0)
				op->info.basedisp_label = 1;
			break;	

		default:
			error((*tokenptr),"illegal input following @");
			errors_in_statement++;
			return((token_t *)0);
		}
		
		/* ok - the base disp has been done.  A RP finishes it off */
		/*
			aN@(56)
			      ^
		*/

		if ((*tokenptr == (token_t )0) || 
			(((*tokenptr)->tokennum == T_RP)))
		{
			if ISENDOP_ORBF(tokenptr+1)
			{
				if (*tokenptr != (token_t)0) tokenptr++;
				op->addrmode.l = 
					(op->info.pcmode)?A_PCDISP:A_ANINDDISP; 
				if (op->info.basedisp_size == 3)
				{
					/* cannot have a long displacement 
					   in the simple indirect 
					   displacement mode
					*/
					if (!is68020) 
					{
						/* if its a label, trust it. */
						if (!op->info.basedisp_label)
							error(*tokenptr,
			"displacement in aN@(X) mode must be word. truncated.");
						op->info.basedisp_size = 2;
					}
					else
						op->addrmode.l = A_ANIXNI;
				}
				goto finishdecode;
			}
			/*
				aN@(56)...
			      	      ^
			*/
			tokenptr++;

			/* IND means indirect post-indexed */
			if ((*tokenptr)->tokennum == T_IND)
			{
				if (!is68020) goto m68020only;
				tokenptr++;
				/*
					aN@(56)@
					aN@(56)@(disp,Xn)
			      		        ^
				*/
				op->addrmode.l = 
				   (op->info.pcmode)?A_PCINDPOSTX:A_ANINDPOSTX;
				if ISENDOP_ORBF(tokenptr)
				{
					/* no outer displacement.  Mode
					   is A_{AN,PC}INDPOSTX;
					*/
					op->info.outerdisp_label = 0;
					op->info.outerdisp_size = 1;
					op->outerdisp.sym = (struct loc_s *)0;
					op->outerdisp.addr = 0;
					goto finishdecode;
				}
				if ((*tokenptr == (token_t)0) || 
				     ((*tokenptr)->tokennum != T_LP))
				{
					error(*(tokenptr-1),
		"only outer displacement can follow @ in memory indirect modes");
					errors_in_statement++;
					return((token_t *)0);
				}

				/* get the outer displacement */
				tokenptr++;
				if (*tokenptr != (token_t)0)
				{
					op->info.outerdisp_size = 
						numsize_to_dispsize[
						   (int)dotokennum(&tokenptr,
						   &op->outerdisp)];
					if (op->outerdisp.sym != 
						(struct loc_s *)0)
						op->info.outerdisp_label = 1;
				}
				if ((*tokenptr)->tokennum == T_CM)
				{
					tokenptr++;
					getindx(&tokenptr,op);
				}
				if (((*tokenptr) == (token_t)0) || 
				   ((*tokenptr)->tokennum != T_RP))
				{
					error(*tokenptr,
"right paren expected after indx/outer disp of memory indirect postindexed operand");
					errors_in_statement++;
				}
			}
			else {
				if (ISENDOP_ORBF(tokenptr)) goto finishdecode;
				error(*tokenptr, 
				    "only '@' may follow base displacement");
				errors_in_statement++;
				goto finishdecode;
			}	
		}
		else {
			/*
				aN@(disp,
					^
			*/
			/* only possibility is comma followed by index */
			if ((*tokenptr)->tokennum == T_CM)
			{
				tokenptr++;
				/*
					aN@(56,Xn)
			      		       ^
				*/
				getindx(&tokenptr,op);
				if ((op->info.basedisp_size <= 2)&&
				   (!op->info.basedisp_label)&&
				   ((op->basedisp.addr <= 127)&&
				    (op->basedisp.addr >= (-128))))
					op->addrmode.l = 
				  	    (op->info.pcmode)?A_PCIXBD:A_ANIXBD;
				else {
					if (!is68020) goto m68020only;
					op->addrmode.l =
					    (op->info.pcmode)?A_PCIXNI:A_ANIXNI;
				}
				if ((*tokenptr)->tokennum != T_RP)
				{
					errors_in_statement++;
					error((*tokenptr),
				    "right paren expected after simple index");
					return((token_t *)0);
				}
				tokenptr++;
				if ISENDOP_ORBF(tokenptr)
				    	goto finishdecode;
				if ((*tokenptr)->tokennum != T_IND)
				{
					errors_in_statement++;
					error((*tokenptr),
			"only '@' may follow indirect indexed operand");
					return((token_t *)0);
				}
				/* ok -- A_{AN,PC}INDPREX.  outer 
				   displacement may follow.
				*/

				if (!is68020) goto m68020only;
				/*
					aN@(56,[Xn])@(disp)
			      		            ^ 
				*/
				tokenptr++;
				op->addrmode.l = 
				   (op->info.pcmode)?A_PCINDPREX:A_ANINDPREX;

				if ISENDOP_ORBF(tokenptr)
				{
					op->info.outerdisp_label = 0;
					op->info.outerdisp_size = 1;
					op->outerdisp.sym = (struct loc_s *)0;
					op->outerdisp.addr = 0;
					goto finishdecode;
				}
				if ((*tokenptr)->tokennum != T_LP)
				{
					errors_in_statement++;
					error((*tokenptr),
    "only outer displacement may follow memory indirect post-indexed operand");
					return((token_t *)0);
				}
				/* get the outer displacement */
				tokenptr++;
				if (*tokenptr != (token_t)0)
				{
					op->info.outerdisp_size = 
						numsize_to_dispsize[
						   (int)dotokennum(&tokenptr,
						   &op->outerdisp)];
					if (op->outerdisp.sym != 
						(struct loc_s *)0)
						op->info.outerdisp_label = 1;
				}
			}
			else {
				error((*tokenptr),"comma or left paren expected");
				errors_in_statement++;
				return((token_t *)0);
			}
		}
	}

finishdecode:
	/* here, update the tokenptr to point to the first token
	   past the comma, or to a null  token.  */
	if (!ISENDOP(tokenptr))
	{
	    	if ((*tokenptr)->tokennum == T_LBRACE) 
			tokenptr = getbf(tokenptr,op);
		else {
		    	if ((*tokenptr)->tokennum != T_CM) 
				tokenptr++;

			if (((*tokenptr) != (tokentype *)0) && 
			    ((*tokenptr)->tokennum == T_LBRACE) )
				tokenptr = getbf(tokenptr,op);
		}
		if (!ISENDOP(tokenptr))
		{
			error((*tokenptr),"badly formed operands");
			errors_in_statement++;
			while (((*tokenptr) != (tokentype *)0) && 
				((*tokenptr)->tokennum != T_CM) )
				tokenptr++;
		}
	}
	return(tokenptr);
m68020only:
	error((*tokenptr),
	   "indicated addressing mode is illegal on mc68000/mc68010");
	errors_in_statement++;
	return(0);
}
