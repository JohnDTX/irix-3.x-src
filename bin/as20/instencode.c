#include <stdio.h>
#include "globals.h"
#include "sym.h"
#include "tokens.h"
#include "inst.h"
#include "addrmodes.h"

/*
	INSTRUCTION ENCODING ROUTINES.

	These routines are used to encode the operands into
	the binary instructions themselves.  The path followed
	is as follows:

	     *	a pointer to a routine is obtained when the 
		instruction is decoded and found in the templates.

	     *	This routine (called the "instruction encoding routine")
		controls the combination of operands
		to be encoded in the instruction.  In most cases,
		this is broken down into a routine for each operand
		(called the "operand encoding routine(s)").

	     *	These operand encoding routines, when called, 
		expect to encode their
		operand at the current position in a global binary
		code array, and, possibly, to alter the instruction
		word itself, which is passed to them.  
		The responsibility for positioning the
		pointer into the binary code array rests with the 
		instruction encoding routine, as does the final
		placing of the instruction word itself into the
		code array.

		The positioning of the pointer into the binary code
		array is done with the macro update_dot() which also updates
		the current csect.

	As an example, one of the templates for the addb instruction is

	0x0600,A_CANBE_IMMB,(A_DN|A_ANYIND|A_ANYINDEX|A_ANYABS),ea_imm,A_DEST,0,

	This template tells how to encode an addb instruction whose first
	operand is an immediate of byte length and whose second operand is
	one of: a data register, an indirect type (indirect, indirect pre/post
	incr/decr, or indirect with displacement), an indexed type, or an 
	absolute.  The instruction encoding, as might be guessed, yields 
	an add immediate byte instruction.  The routine specified for the
	encoding is ea_imm, and the flag A_DEST says that the destination 
	operand (the ea) is to be passed as the first operand.

	Looking at ea_imm, the first thing that happens is that the binary
	code pointer is positioned past the instruction word (update_dot(2)).
	Then doimm() is called to encode the immediate datum at the current
	position in the binary code array.  Doimm() will increment the
	pointer into the binary code array as it needs to generate code.
	Next doea() is called to encode the first argument as 
	an effective address, again using the current position of the
	binary code array.  Note that both doimm and doea are passed the
	address of the instruction word itself, so that they can encode
	certain bits in the instruction word as necessary.  Last, the 
	instruction word itself is placed in the binary code array.  When
	the routine exits, both dot[cur_csect] and bins have been incremented
	correctly for the current instruction.  It is then the job of 
	do_statement() to copy the current instruction's binary from
	the global binary array into the current statement's structure.
		
*/



union hack
{
	struct {
		unsigned 
		    nib7:4,
		    nib6:4,
		    nib5:4,
		    nib4:4,
		    nib3:4,
		    nib2:4,
		    nib1:4,
		    nib0:4;
		} nibble;
	struct {
		unsigned 
		    byte3:8,
		    byte2:8,
		    byte1:8,
		    byte0:8;
		} byte;
	struct {
		unsigned short w1,
		w0;
		} shorts;
	unsigned long l;
	struct {
		unsigned :31,bit0:1;
		} testbit;
};

addrmodebit(l,multiple)
union hack l;
int multiple;
{
	/*
		addrmodebit is a very important routine.  It
		is used to find the least restrictive addressing
		mode when multiple addressing modes are allowed.
		It has a tendancy to be VERY slow - hence the
		hack incorporated above to avoid single bit shifts
	*/
	int retval = 0;
	register count=1;

	if (!l.shorts.w0) { count+=16; l.l >>= 16;}
	if (!l.byte.byte0) { count +=8; l.l >>=8;}
	if (!l.nibble.nib0) { count += 4; l.l >>=4;}
	while ((l.l)&&(!retval))
	{
		if (l.testbit.bit0)
			retval = count;
		count++;
		l.l >>= 1;
	}
	if (l.l && !multiple) return(0); else return(retval);
}
		

doimm0(inst,imm)
register union instword_u *inst;
register struct operand_s *imm;
{
	/* encode an imm0t7 to an immb in the byte field 
	   beginning at bit 0 in the instruction word
	*/
	register struct op_immabs_wa *temp = (struct op_immabs_wa *)&imm->imm;
	int firstbit;

	if (imm->addrmode.l & (A_ANYIMM))
	{
		firstbit = addrmodebit(imm->addrmode.l,1);
		/* we HAVE TO TURN OFF THE OTHER BITS! */
		if (firstbit) imm->addrmode.l &= ( 1 << (firstbit-1));
	}
	if (imm->addrmode.l & (A_CANBE_IMMB|A_CANBE_DISPB))
		inst->immb0.immb0 = temp->u.addrl;
	else if (!imm->info.immabs_label)
	{
		/* immediate datum will not fit into the instruction */
		error(0,"illegal small immediate data encoding");
		errors_in_statement++;
	}
	if (imm->info.immabs_label)
	{
		/* the small constant can be a forward definition symbol
		   since it is a full byte
		*/
		fix_statusers(imm->imm.sym,cur_stat,nbinary,1,
			(((imm->addrmode.l & A_CANBE_DISPB)?ISDISP:0))|
			 ((imm->imm.isnrexpr)?ISNREXPR:0));
	}
	if ((imm->addrmode.l & A_CANBE_DISPB) && 
		(cur_stat->lab == (symtabptr)0))
	{	
		/*  if this is a displacement and there is no
		    label attached to the statement, we must generate
		    a dummy label to make sure the statement address
		    gets updated!
		*/
		cur_stat->lab = 
		    define_label(allocate_templab(),cur_stat,curstat_labaddr);
		cur_stat->lab->syminfo.istemp = 1;
	}

}

doimm9(inst,imm)
register union instword_u *inst;
register struct operand_s *imm;
{
	/* encode an imm0t7 or an imm1t8 in the three bit field 
	   beginning at bit 9 in the instruction word
	*/

	register struct op_immabs_wa *temp = (struct op_immabs_wa *)&imm->imm;
	int firstbit;

	if (imm->addrmode.l & (A_ANYIMM))
	{
		firstbit = addrmodebit(imm->addrmode.l,1);
		/* we HAVE TO TURN OFF THE OTHER BITS! */
		if (firstbit) imm->addrmode.l &= ( 1 << (firstbit-1));
	}
	if (imm->addrmode.l & A_CANBE_IMM1T8)
		inst->ea_imm.imm1t8 = temp->u.addrl;
	else if (!(imm->info.immabs_label))
	{	
		error(0,"number too large to be encoded in three bits");
		errors_in_statement++;
	}
	if ((imm->info.immabs_label))
	{
		/* GB FIX - dont allow non-constants to be encoded in bitfields
		   now */
		error(0,
    "forward referenced constants cannot be encoded in instruction bitfields");
#ifdef NOTDEF
			fix_statusers(imm->imm.sym,cur_stat,nbinary*8+9,3,
				( ISBITSZ|ISNREXPR));
#endif
	}
	if ((imm->addrmode.l & A_CANBE_DISPB) && 
		(cur_stat->lab == (symtabptr)0))
	{	
		/*  if this is a displacement and there is no
		    label attached to the statement, we must generate
		    a dummy label to make sure the statement address
		    gets updated!
		*/
		cur_stat->lab = 
		    define_label(allocate_templab(),cur_stat,curstat_labaddr);
		cur_stat->lab->syminfo.istemp = 1;
	}

}
doimm(inst,imm,is6)
register union instword_u *inst;
register struct operand_s *imm;
int is6;
{

	/* encode an immediate datum (word or long) in the code
	   stream following the instruction itself.
	*/
	int nb=0;
	register struct op_immabs_wa *temp = (struct op_immabs_wa *)&imm->imm;
	int firstbit;

	if (imm->addrmode.l & (A_ANYIMM))
	{
		firstbit = addrmodebit(imm->addrmode.l,1);
		/* we HAVE TO TURN OFF THE OTHER BITS! */
		if (firstbit) imm->addrmode.l &= ( 1 << (firstbit-1));
	}
#ifdef NOTDEF
	if (imm->addrmode.l & A_CANBE_IMM1T8)
		inst->ea_imm.imm1t8 = temp->u.addrl;
	else 
#endif
	if (imm->addrmode.l & (A_CANBE_IMMW|A_CANBE_ABSW))
	{
		/* datum is word */
		nb = 2;
		*bins++ = temp->u.addrw.w1;
	}
	else {
		/* datum is long */
		nb = 4;
		*bins++ = temp->u.addrw.w0;
		*bins++ = temp->u.addrw.w1;
	}
	if (imm->info.immabs_label)
	{
		/* datum is either displacement or unknown.
		   Set up to fix it later and mark it as
		   condensable if it is a long.
		*/
		if ((imm->addrmode.l & A_CANBE_ABSL))
			fix_statusers(imm->imm.sym,cur_stat,nbinary,nb,
				(CANBECONDENSED|
				 ((imm->addrmode.l & A_ANYDISP)?ISDISP:0)|
				 ((imm->imm.isnrexpr)?ISNREXPR:0)),
				   (is6)?9:0,0);
		else
			fix_statusers(imm->imm.sym,cur_stat,nbinary,nb,
				( ((imm->addrmode.l & A_ANYDISP)?ISDISP:0))|
				 ((imm->imm.isnrexpr)?ISNREXPR:0));
	}
	if ((imm->addrmode.l & A_CANBE_DISPB) && 
		(cur_stat->lab == (symtabptr)0))
	{	
		/*  if this is a displacement and there is no
		    label attached to the statement, we must generate
		    a dummy label to make sure the statement address
		    gets updated!
		*/
		cur_stat->lab = 
		    define_label(allocate_templab(),cur_stat,curstat_labaddr);
		cur_stat->lab->syminfo.istemp = 1;
	}
	update_dot(nb);
}

doea(inst,ea,is6)
register union instword_u *inst;
register struct operand_s *ea;
register int is6;
{
	register int firstbit;
	int r;
	int fmtwd = nbinary>>1;

	register union format_word *ext_fmt; 
	struct op_immabs_wa *temp ;

	/* okay, encode the ea. */
	/* the only case in which the ea addrmode is not specific
	   at this point would be if it is immediate.  Since there
	   are no immediate to immediate operations, this is not
	   allowed.  Thus, there should only be a single bit set
	   in the ea addrmode at the current time, if it is not 
	   immediate.
	*/

	if (ea->addrmode.l & ~(A_ANYIMM))
		firstbit = addrmodebit(ea->addrmode.l,0);
	else
	{
		firstbit = addrmodebit(ea->addrmode.l,1);
		/* we HAVE TO TURN OFF THE OTHER BITS! */
		if (firstbit) ea->addrmode.l &= ( 1 << (firstbit-1));
	}
	if (!firstbit ) 
		fatal(0,"ea addressing mode is not determined in doea");

	/*  ext_fmt is a pointer to the ea extension words (brief or long
	    format).  
	*/
	ext_fmt = (union format_word *)bins;
	ext_fmt->all = 0;

	/*  encode the ea mode in the correct bitfield in the instruction.
	    This is a six-bit field, beginning at bit 0 unless is6 is TRUE,
	    in which case it begins at bit six
	*/
	/* encode the mode */
	if (instea[firstbit].mode != I_NOMODE)
		if (is6)
			inst->ea6.mode = instea[firstbit].mode;
		else 
			inst->ea_imm.mode = instea[firstbit].mode;

	/* encode the register */
	if (instea[firstbit].reg != I_NOMODE)
	{
		if (instea[firstbit].reg == I_NEEDSREG)
			r = ea->reg0;
		else 
			r = instea[firstbit].reg;

		if (is6)
			inst->ea6.reg = r;
		else
			inst->ea_imm.reg = r;
	}

	/* if the ea is immediate, encode it */
	if ((ea->addrmode.l & A_ANYIMM) || (ea->addrmode.l & A_ANYABS))
	{
		doimm(inst,ea,is6);
		return;
	}

	/* encode indexed ea's */
	if (ea->addrmode.basemode.AnIndexed || ea->addrmode.basemode.PcIndexed)
	{

		/* encode the low 9 bits of the format word */
		switch (firstbit)
		{
		case A_PCIXBD_BIT :
		case A_ANIXBD_BIT :
				if (ea->info.immabs_label)
				{
					error(0,
	"label for byte base displacement in indexed operand is illegal");
					errors_in_statement++;
				}
				ext_fmt->brief_format.disp = ea->basedisp.addr;
				ea->info.basedisp_size = 1;
				break;

		case A_ANINDPOSTX_BIT:
		case A_PCINDPOSTX_BIT:
				ext_fmt->full_format.postindexed = 1;

		case A_ANINDPREX_BIT:
		case A_PCINDPREX_BIT:
				/* is=0, od_size=0,  */
				ext_fmt->full_format.od_size = ea->info.outerdisp_size;

		case A_ANIXNI_BIT:
		case A_PCIXNI_BIT:
				/* pc is encoded, so the same */
				/* is=0, od_size=0, post_indexed=0 */
#ifdef NOTDEF
/* this is not true upon testing.  The bdsize = null
   is all we need.
*/
				/* if the displacement is zero, bs=1 */
				if ((!ea->info.basedisp_label)&&(ea->basedisp.addr == 0))
					ext_fmt->full_format.bs = 1;
#endif
				/* null displacement */

		commonlongfmt:
				ext_fmt->full_format.mustbeone = 1;
				ext_fmt->full_format.bd_size = 	ea->info.basedisp_size;
		}

		/* all indexed modes */
		/* at this point, the pc is encoded */
		/* use brief form */
		ext_fmt->brief_format.regno = ea->indexreg.regno;
		if (ea->indexreg.islongindex) 
			ext_fmt->brief_format.islong = 1;
		ext_fmt->brief_format.scale = ea->indexreg.scale;
		if (!(ea->info.isindexed))
		{
			ext_fmt->full_format.is = 1;
			ext_fmt->full_format.postindexed = 0;
		}
		update_dot(2);
		bins++ ;
	}

	/* basedisp_size (and outerdisp_size) are 1 (null), 2 (word), or
	   3 (long), corresponding to the values the bdsize and i/is fields
	   need to be in the extended format words
	*/
	if (ea->info.basedisp_size > 1)
	{
		int nb;
		/* encode the base displacement */
		temp = (struct op_immabs_wa *)&ea->basedisp;

		if (ea->info.basedisp_label)
		{
			/* The base displacement is a label.
			   enter a stat user for it. 
			*/
			if (ea->addrmode.l != A_ANINDDISP)
			{
				/* assume long */
				nb = 4;
				*bins++ = temp->u.addrw.w0;
			}
			else nb=2;
			fix_statusers(ea->basedisp.sym,cur_stat,nbinary,nb,
				( ((nb==4)?CANBECONDENSED:0)|ISNREXPR),4,fmtwd);
			*bins++ = temp->u.addrw.w1;
			update_dot(nb);
		}
		else {
			if (ea->info.basedisp_size == 3)
			{
				*bins++ = temp->u.addrw.w0;
				update_dot(2);
			}
			*bins++ = temp->u.addrw.w1;
			update_dot(2);
		}
			
	}
	if (ea->info.outerdisp_size > 1)
	{
		/* encode the outer displacement */
		temp = (struct op_immabs_wa *)&ea->outerdisp;
		if (ea->info.outerdisp_label)
		{
			/* enter a stat user for the outer disp. */

			fix_statusers(ea->outerdisp.sym,cur_stat,nbinary,4,
				( CANBECONDENSED|ISNREXPR),0,fmtwd);
			*bins++ = temp->u.addrw.w0;
			*bins++ = temp->u.addrw.w1;
			update_dot(4);
		}
		else {
			if (ea->info.outerdisp_size == 3)
			{
				*bins++ = temp->u.addrw.w0;
				update_dot(2);
			}
			*bins++ = temp->u.addrw.w1;
			update_dot(2);
		}
	}
}

ea_imm(inst,ea,imm,flags)
union instword_u inst;
struct operand_s *ea,*imm;
{
/* 
	ea_imm - encode an ea and an immediate datum.  At this point,
	we dont know which was the source, and which was the destination.
	We only know which was the ea and which is the immediate data.
	This is okay because the order of extension words is
	imm, src ea, dest ea.
*/

	/*  encode the immediate operand. There may still be several
	    choices for this case.
	*/

	/* minimum of two bytes */


	bins = &binary.shorts[1];

	update_dot(2);

	doimm(&inst,imm,0);

	/* okay, encode the ea. */
	/* the only case in which the ea addrmode is not specific
	   at this point would be if it is immediate.  Since there
	   are no immediate to immediate operations, this is not
	   allowed.  Thus, there should only be a single bit set
	   in the ea addrmode at the current time.
	*/

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;

}

ea_imm9(inst,ea,imm,flags)
union instword_u inst;
struct operand_s *ea,*imm;
{
/* 
	ea_imm9 - encode an ea and an immediate datum.  The immediate
	datum is to be encoded in the instruction word at bit 9. 
*/

	/* minimum of two bytes */


	bins = &binary.shorts[1];

	doimm9(&inst,imm);

	update_dot(2);

	/* okay, encode the ea. */
	/* the only case in which the ea addrmode is not specific
	   at this point would be if it is immediate.  Since there
	   are no immediate to immediate operations, this is not
	   allowed.  Thus, there should only be a single bit set
	   in the ea addrmode at the current time.
	*/

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;

}

ea(inst,ea,flags)
union instword_u inst;
struct operand_s *ea;
{
/*
	ea - encode the ea in the instruction word.  The source or destination
	is indicated.
*/
	/* minimum of two bytes */
	update_dot(2);

	bins = &binary.shorts[1];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;
	
}

ext_r0(inst,ea,r0,flags)
union instword_u inst;
struct operand_s *ea,*r0;
{
/*
	ext_r0 - encode the indicated displacement in the
	instruction word.  The other operand is encoded as a register in
	the three-bit field beginning with zero.
*/

	inst.r9_r0.r0 = r0->reg0;

	if (ea->addrmode.l & A_IS_DISPW)
		word_ext(inst,ea,flags);
	else 
		long_ext(inst,ea,flags);

}

ea_r0(inst,ea,r0,flags)
union instword_u inst;
struct operand_s *ea,*r0;
{
/*
	ea_r0 - encode the indicated source or destination ea in the 
	instruction word.  The other operand is encoded as a register in
	the three-bit field beginning with zero.
*/

	inst.r9_r0.r0 = r0->reg0;
	update_dot(2);

	bins = &binary.shorts[1];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;
	
}

r0(inst,r0,flags)
union instword_u inst;
struct operand_s *r0;
{
/*
	r0 - encode the single indicated operand in the three bit field 
	beginning at bit zero of the instruction word.  
*/

	inst.r9_r0.r0 = r0->reg0;
	update_dot(2);

	bins = &binary.shorts[1];

	binary.shorts[0] = inst.words.instword;
}

ea_r9(inst,ea,r9,flags)
union instword_u inst;
struct operand_s *ea,*r9;
{
/*
	ea_r9 - encode an ea and a register.  

*/

	inst.r9_r0.r9 = r9->reg0;
	update_dot(2);

	bins = &binary.shorts[1];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;
	
	

}

r0_r9(inst,r0,r9,flags)
union instword_u inst;
struct operand_s *r9,*r0;
{
/*
	r0_r9 - encode the indicated register operand in the three bits 
	beginning at bit 0, the other register in the three bits beginning
	at r9.
	
*/

	inst.r9_r0.r9 = r9->reg0;
	inst.r9_r0.r0 = r0->reg0;
	update_dot(2);

	bins = &binary.shorts[1];

	binary.shorts[0] = inst.words.instword;
	
	

}


r0disp_r9(inst,r0disp,r9,flags)
union instword_u inst;
struct operand_s *r9,*r0disp;
{
/*
	r0disp_r9 - encode the indicated register operand in the three bits 
	beginning at bit 0, the other register in the three bits beginning
	at r9.
	
*/

	register struct op_immabs_wa *temp = 
	      (struct op_immabs_wa *)&r0disp->basedisp;
	inst.r9_r0.r9 = r9->reg0;
	inst.r9_r0.r0 = r0disp->reg0;
	update_dot(2);

	bins = &binary.shorts[1];

	if (r0disp->info.basedisp_label)
	{
		fix_statusers(r0disp->basedisp.sym,cur_stat,nbinary,2,
			0);
	}
	else if ((temp->u.addrw.w0)&&(temp->u.addrw.w0 != 0xffff))
	{
		error(0,"aN displacement is too large to fit in word");
		errors_in_statement++;
	}
	*bins++ = temp->u.addrw.w1;

	update_dot(2);

	binary.shorts[0] = inst.words.instword;
	
	

}


r9_imm0(inst,r9,imm,flags)
union instword_u inst;
struct operand_s *r9,*imm;
{
	/*
	r9_imm0 - encode the indicated register operand in the three bits
	beginning at bit 9, the immediate datum in the instruction
	word beginning at bit 0.
	*/

	inst.r9_r0.r9 = r9->reg0;

	bins = &binary.shorts[1];

	doimm0(&inst,imm);

	update_dot(2);

	binary.shorts[0] = inst.words.instword;

}


r0_imm(inst,r0,imm,flags)
union instword_u inst;
struct operand_s *r0,*imm;
{
	/*
	r0_imm - encode the immediate operand following the inst word, 
	and the register in the three bits beginning
	at d0.  The location of the r0 operand is indicated.
	*/

	inst.r9_r0.r0 = r0->reg0;
	bins = &binary.shorts[1];

	update_dot(2);
	doimm(&inst,imm,0);



	binary.shorts[0] = inst.words.instword;
}

r0_imm9(inst,r0,imm,flags)
union instword_u inst;
struct operand_s *r0,*imm;
{
	/*
	r0_imm9 - encode the immediate operand in the three bits 
	beginning at bit 9, the other register in the three bits beginning
	at d0.  The location of the r0 operand is indicated.
	*/

	inst.r9_r0.r0 = r0->reg0;
	bins = &binary.shorts[1];

	doimm9(&inst,imm);

	update_dot(2);
	binary.shorts[0] = inst.words.instword;
}

word_ext(inst,ea,flags)
union instword_u inst;
register struct operand_s *ea;
{
/*
	word_ext - encode the single operand (immediate or address ) as
	word data following the static instruction word given.  The word
	data is the given operand.
*/
	register struct op_immabs_wa *temp = (struct op_immabs_wa *)&ea->imm;
	long addr;
	int suflags = 0;

	bins = &binary.shorts[1];
	update_dot(2);
	if (ea->addrmode.l & (A_ANYIMM|A_ANYABS|A_ANYDISP))
	{
		if (ea->info.immabs_label)
		{
			if (ea->addrmode.l & A_ANYDISP) suflags = ISDISP;
			if (flags & X_ISSDI) {
				suflags = ISDISP;
				if (!(flags & X_DONTCONDENSE))
					suflags |= 
						(CANBECONDENSED|CANBEBYTEDISP);	
			}
			if (ea->imm.isnrexpr) suflags |= ISNREXPR;
			fix_statusers(ea->imm.sym,cur_stat,nbinary,2,
				suflags);
		}
		else if ((temp->u.addrw.w0)&&(temp->u.addrw.w0 != 0xffff))
		{
			error(0,"immediate/absolute is too large to fit in word");
			errors_in_statement++;
		}
		*bins++ = temp->u.addrw.w1;
		update_dot(2);
	}
	else 
	{
		error(0,"word_ext called when ea is not abs.w");
		errors_in_statement++;
		return(0);
	}
	if ((ea->addrmode.l & A_ANYDISP) && 
		(cur_stat->lab == (symtabptr)0))
	{	
		/*  if this is a displacement and there is no
		    label attached to the statement, we must generate
		    a dummy label to make sure the statement address
		    gets updated!
		*/
		cur_stat->lab = 
		    define_label(allocate_templab(),cur_stat,curstat_labaddr);
		cur_stat->lab->syminfo.istemp = 1;
	}
	cur_stat->sinfo.issdi = (flags & X_ISSDI)?1:0;
	binary.shorts[0] = inst.words.instword;
}


long_ext(inst,ea,flags)
union instword_u inst;
register struct operand_s *ea;
{
/*
	long_ext - encode the single operand (immediate or address ) as
	long data following the static instruction word given.  The long
	data is the given operand.
*/
	register struct op_immabs_wa *temp = (struct op_immabs_wa *)&ea->imm;
	long addr;
	int suflags;

	bins = &binary.shorts[1];
	update_dot(2);
	if (ea->addrmode.l & (A_ANYIMM|A_ANYABS|A_ANYDISP))
	{
		if (ea->info.immabs_label)
		{
			if (ea->addrmode.l & A_ANYDISP) suflags = ISDISP;
			if (flags & X_ISSDI) {
				suflags = ISDISP;
				if (!(flags & X_DONTCONDENSE))
					suflags |= 
						(CANBECONDENSED|CANBEBYTEDISP);	
			}
			if (ea->imm.isnrexpr) suflags |= ISNREXPR;
			fix_statusers(ea->imm.sym,cur_stat,nbinary,4,
				suflags);
#ifdef NEWJXX
			if (flags & X_ISJSR) cur_stat->sinfo.isjsr = 1;
#endif
		}
		*bins++ = temp->u.addrw.w0;
		*bins++ = temp->u.addrw.w1;
		update_dot(4);
	}
	else 
	{
		error(0,"long_ext called when ea is not abs/imm.l");
		errors_in_statement++;
		return(0);
	}
	if ((ea->addrmode.l & A_ANYDISP) && 
		(cur_stat->lab == (symtabptr)0))
	{	
		/*  if this is a displacement and there is no
		    label attached to the statement, we must generate
		    a dummy label to make sure the statement address
		    gets updated!
		*/
		cur_stat->lab = 
		    define_label(allocate_templab(),cur_stat,curstat_labaddr);
		cur_stat->lab->syminfo.istemp = 1;
	}
	cur_stat->sinfo.issdi = (flags & X_ISSDI)?1:0;

	binary.shorts[0] = inst.words.instword;
}


ea6_ea(inst,ea6,ea,flags)
union instword_u inst;
struct operand_s *ea6,*ea;
{
	/*
	ea6_ea - encode two eas, one at bits 0-5 and the indicated one
	at bits 6-11.
	*/

	bins = &binary.shorts[1];

	update_dot(2);

	doea(&inst,ea,ENCODEATBIT0);

	doea(&inst,ea6 ,ENCODEATBIT6);

	binary.shorts[0] = inst.words.instword;

}

no_operands(inst,flags)
union instword_u inst;
{

	/* no operands.  just add the inst word */

	binary.shorts[0] = inst.words.instword;
	update_dot(2);

}


