#include <stdio.h>
#include "globals.h"
#include "sym.h"
#include "tokens.h"
#include "inst.h"
#include "addrmodes.h"
extern unsigned short special_reg_to_bits[];

union longinst
{
	struct {
		unsigned
		:16,
		regno:4,
		controlreg:12;
		}r_cntl;
	struct {
		unsigned 
		:16,
		regno:4,
		:4,
		immb0:8;
		}r_imm;
	struct {
		unsigned short word0,word1;
		}words;
}; 

rext12_specialreg(inst,rext12,specialreg,flags)
union instword_u inst;
struct operand_s *rext12,*specialreg;
{

	union longinst *mytemp;

	mytemp = (union longinst *)&binary.shorts[0];
	mytemp->words.word1 = 0;
	mytemp->r_cntl.regno =  rext12->reg0;
	if (rext12->addrmode.l & A_AN)
		mytemp->r_cntl.regno |= 0x8;
	mytemp->r_cntl.controlreg = 
		special_reg_to_bits[specialreg->reg0 & 0x7];
	update_dot(4);
	mytemp->words.word0 = inst.words.instword;

}

#ifdef NOTDEF
ea_ximm0(inst,ea,imm,flags)
union instword_u inst;
struct operand_s *ea,*imm;
{

	union longinst *mytemp;


	update_dot(4);

	/* okay, encode the ea. */

	doea(&inst,ea,ENCODEATBIT0);

		flags--; 
		inst.immb0.immb0 = ((inst.immb0.immb0 & (~flags))|
			(temp->u.addrl & flags));

	mytemp = (union longinst *)&binary.shorts[0];
	mytemp->words.word1 = 0;
	mytemp->r_cntl.regno =  rext12->reg0;
	update_dot(4);
	mytemp->words.word0 = inst.words.instword;

	binary.shorts[0] = inst.words.instword;

}

#endif

spimm0(inst,imm,flags)
union instword_u inst;
struct operand_s *imm;
int flags;
{
	int nbits,firstbit;
	register struct op_immabs_wa *temp = (struct op_immabs_wa *)&imm->imm;

	nbits = addrmodebit(flags) - 1;
	if (imm->addrmode.l & (A_ANYIMM))
	{
		firstbit = addrmodebit(imm->addrmode.l,1);
		/* we HAVE TO TURN OFF THE OTHER BITS! */
		if (firstbit) imm->addrmode.l &= ( 1 << (firstbit-1));
	}
	if ((imm->addrmode.l & A_CANBE_IMMB) && 
		(temp->u.addrl < (flags & 0xff)))
	{
		flags--; 
		inst.immb0.immb0 = ((inst.immb0.immb0 & (~flags))|
			(temp->u.addrl & flags));
	}
	else if (!(imm->info.immabs_label))
	{	
		error(0,"number too large to be encoded in %d bits",nbits);
		errors_in_statement++;
	}
	if ((imm->info.immabs_label))
	{
		fix_statusers(imm->imm.sym,cur_stat,0,nbits,
			( ISBITSZ|ISNREXPR));
	}
	update_dot(2);
	binary.shorts[0] = inst.words.instword;
}



r0_usp(inst,r0,specialreg,flags)
union instword_u inst;
struct operand_s *r0,*specialreg;
{
	/* 
	*/
	inst.r9_r0.r0 = r0->reg0;
	if ((specialreg->reg0 & 0x7) != (USPREG & 0x7))
		error(0,"illegal special register operand");
	else
	{
		update_dot(2);
		binary.shorts[0] = inst.words.instword;
	}

}

struct dudc_s {
	unsigned 
		junk:16,rn:4,:3,du:3,:3,dc:3;
} ;

static struct dudc_s *du_dc;

cas_encode(inst,dc,du,ea,flags)
union instword_u inst;
struct operand_s *dc,*du,*ea;
int flags;
{

	/* encode the cas instruction.

	   We also must check the consistency of the dc operand addressing
	   mode.  This was NOT checked in the instruction parser.

	   The encoding is as follows:

		the ea is encoded at bit 0 of the instword.

		the du register is encoded at bit 6 of the extension word.
		the dc register is encoded at bit 0 of the extension word.

	*/


	/* use the instword_u structure, as it already has the 
	   appropriate bits defined (in the r0_r9 part)
	*/

	if ((!(dc->addrmode.l & A_DN)))
	{
		error("Dc register must be data register");
		errors_in_statement++;
		return(0);
	}

	binary.shorts[1] = 0;
	du_dc = (struct dudc_s *) &binary.shorts[0];

	du_dc->du = du->reg0;
	du_dc->dc = dc->reg0;

	update_dot(4);
	bins = &binary.shorts[2];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;
	

}



cas2_encode(inst,dc,du,rn,flags)
union instword_u inst;
struct operand_s *dc,*du,*rn;
int flags;
{
	/* encode the cas2 instruction.  Sigh.

	   This is more confusing.  

	   1.  The unchecked first operand has to be checked.

	   2.  Rn1, Du1 and Dc1 are encoded in the first extension word.

	   3.  Rn2, Du2 and Dc2 are encoded in the second extension word.

	   The encoding is similar to that used for cas, thus the same
	   overlay is used.

	*/

	if ((!(dc->addrmode.l & A_DOUBLEREG)))
	{
		error(0,"first operand of cas2 must be double register");
		errors_in_statement++;
		return(0);
	}

	binary.shorts[1] = binary.shorts[2] = 0;
	du_dc = (struct dudc_s *) &binary.shorts[0];

	du_dc->du = du->reg0;
	du_dc->dc = dc->reg0;
	du_dc->rn = rn->reg0;

	du_dc = (struct dudc_s *) &binary.shorts[1];

	du_dc->du = du->reg1;
	du_dc->dc = dc->reg1;
	du_dc->rn = rn->reg1;

	update_dot(6);

	binary.shorts[0] = inst.words.instword;
	



}


ea_rext(inst,ea,rext,flags)
union instword_u inst;
struct operand_s *ea,*rext;
int flags;
{
	/* encode an ea at bit zero, and the passed register
	   (data or address) in an extension word at bit 12.
	   Additionally (flags & X_FLAG0 ) is used to indicate
	   whether the value of bit 11 in the extension word
	   is to be one (set) or zero (!(flags & X_FLAG0)).
	*/


	union extword {
		unsigned short s;
		struct {
			unsigned da:1,reg:3,
				flag:1;
		} bits;
	} *extword ;

	/* the instruction word itself is two words (the
	   instruction and two bytes of extension */
	
	extword = (union extword *)&binary.shorts[1];

	/* encode the register in the extension word, along
	   with the value for bit 11.*/
	extword->s = 0;
	extword->bits.reg = rext->reg0;
	if (rext->addrmode.l == A_AN) extword->bits.da = 1;
	if (flags & X_FLAG0) extword->bits.flag = 1;

	update_dot(4);
	bins = &binary.shorts[2];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;

}

union bf_u {
	struct {
		unsigned 
		   :16,
		   mustbezero:1,
		   reg:3,
		   offset_in_reg:1,
		   offset:5,
		   width_in_reg:1,
		   width:5;
		} bf;
	struct 	{
		unsigned short junk,bfpart;
		} s;
} ;

bfea(inst,ea,flags)
union instword_u inst;
struct operand_s *ea;
int flags;
{
	/* encode a single-operand bitfield instruction */
	union bf_u *bfu;
	bfu = (union bf_u *)&binary.shorts[0];

	/* encode the width and offset.  */

	bfu->s.bfpart = 0;

	if (ea->bf.width_in_reg)
	{
		bfu->bf.width_in_reg = 1;
		bfu->bf.width = ea->bf.width_reg;
	}
	else {
		bfu->bf.width = ea->bf.width;
	}
	if (ea->bf.offset_in_reg)
	{
		bfu->bf.offset_in_reg = 1;
		bfu->bf.offset = ea->bf.offset_reg;
	} 
	else {
		bfu->bf.offset = ea->bf.offset;
	}

	update_dot(4);
	bins = &binary.shorts[2];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;



}

bfea_r(inst,ea,rext,flags)
union instword_u inst;
struct operand_s *ea,*rext;
int flags;
{
	/* encode a bitfield with a second register operand. */
	union bf_u *bfu;
	bfu = (union bf_u *)&binary.shorts[0];

	/* encode the width and offset.  */

	bfu->s.bfpart = 0;

	if (ea->bf.width_in_reg)
	{
		bfu->bf.width_in_reg = 1;
		bfu->bf.width = ea->bf.width_reg;
	}
	else {
		bfu->bf.width = ea->bf.width;
	}
	if (ea->bf.offset_in_reg)
	{
		bfu->bf.offset_in_reg = 1;
		bfu->bf.offset = ea->bf.offset_reg;
	} 
	else {
		bfu->bf.offset = ea->bf.offset;
	}

	bfu->bf.reg = rext->reg0;

	update_dot(4);
	bins = &binary.shorts[2];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;

}


ea_rext2(inst,ea,rext,flags)
union instword_u inst;
struct operand_s *ea,*rext;
int flags;
{
	/* encode an ea at bit zero, and the passed register
	   or register pair in the extension word.
	   (flags & X_FLAG0) indicates if the instruction is signed.
	   (flags & X_FLAG1 ) is used as the 
	   value of bit 10 of the extension word.  

	   This encoding is used for the mul and div instrs.
	*/


	union ext2word_u {
		struct 	{
			unsigned short junk,signif;
			} s;
		struct {
			unsigned :16,
				:1,
				dq:3,
				issigned:1,
				sz:1,
				:7,
				dr:3;
			} bits;
	} *extword ;

	/* the instruction word itself is two words (the
	   instruction and two bytes of extension */
	
	extword = (union ext2word_u *)&binary.shorts[0];

	/* encode the register in the extension word, along
	   with the value for bit 11.*/
	extword->s.signif = 0;
	if (flags & X_FLAG0) extword->bits.issigned = 1;

	if (rext->addrmode.l == A_DOUBLEREG)
	{
		/* ok, interchange dr and dq.  dq is the SECOND register now. */
		extword->bits.dr = rext->reg0;
		extword->bits.dq = rext->reg1;
		if (flags & X_FLAG1) extword->bits.sz = 1;
	}
	else {
		/* GB SCR1818.  Be sure to encode BOTH registers as reg0! */
		extword->bits.dq = rext->reg0;
		extword->bits.dr = rext->reg0;
	}

	update_dot(4);
	bins = &binary.shorts[2];

	doea(&inst,ea,ENCODEATBIT0);

	binary.shorts[0] = inst.words.instword;

}
