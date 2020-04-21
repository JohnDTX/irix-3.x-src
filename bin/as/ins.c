#include "mical.h"
#include "inst.h"

#define AR 0	/* must be an areg */
#define DR 1	/* must be a dreg */

char	Code_length;		/* Number of bytes in the current instruction*/
short	*WCode = (short *)Code;
struct oper operands[OPERANDS_MAX];	/* where all the operands go */
int	numops;			/* # of operands to the current instruction */
int	nolist;			/* flag for no listing in text section */
extern int quickf;		/* quick/short instruction flag */
extern int f68010;		/* 68010 processor flag */
extern int labsw;		/* flag for labels on line */

/* Instruction  -- 68000 assembler
 * This program is called from the main loop. It checks to see if the
 * operator field is a valid instruction mnemonic. If so, it calls the
 * operand evaluator and generates the machine code for the instrucion.
 * On pass 2 it prints the listing line for the current statement 
 */
Instruction(opindex)
{	register int i;		/* gen purpose index */
	long j, a[2];		/* temp list variables */
	char temp;

	if (Cur_csect==Text_csect && (Dot&1)) {
		switch (opindex) {
		case i_byte: case i_text: case i_data:
		case i_bss: case i_globl: case i_comm:
		case i_even: case i_ascii: case i_asciz:
		case i_space:
			break;
		default:
			Prog_Error(E_ODDADDR);
		}
	}
	Code_length = 2;		/* always at least 2 bytes of code */
	nolist = 0;			/* indicate text listing */
	for(i=0;i < CODE_MAX; i++) { Code[i] = 0; }
					/* Clear buffer for generated code */
	switch (opindex) {		/* dispatch to handle each inst */

/* instruction classes */
	case i_reset:	no_op(0x4E70); break;
	case i_nop:	no_op(0x4E71); break;
	case i_rte:	no_op(0x4E73); break;
	case i_rts:	no_op(0x4E75); break;
	case i_trapv:	no_op(0x4E76); break;
	case i_rtr:	no_op(0x4E77); break;
	case i_illegal:	no_op(0x4AFC); break;

	case i_stop:	stop_op(0x4E72); break;
	case i_negxb:	one_op(0x4000, B); break;
	case i_negxw:	one_op(0x4040, W); break;
	case i_negxl:	one_op(0x4080, L); break;
	case i_clrb:	one_op(0x4200, B); break;
	case i_clrw:	one_op(0x4240, W); break;
	case i_clrl:	one_op(0x4280, L); break;
	case i_negb:	one_op(0x4400, B); break;
	case i_negw:	one_op(0x4440, W); break;
	case i_negl:	one_op(0x4480, L); break;
	case i_notb:	one_op(0x4600, B); break;
	case i_notw:	one_op(0x4640, W); break;
	case i_notl:	one_op(0x4680, L); break;
	case i_tstb:	one_op(0x4A00, B); break;
	case i_tstw:	one_op(0x4A40, W); break;
	case i_tstl:	one_op(0x4A80, L); break;
	case i_tas:	one_op(0x4AC0, B); break;
	case i_nbcd:	one_op(0x4800, B); break;

	case i_st:	one_op(0x50C0, B); break;
	case i_sf:	one_op(0x51C0, B); break;
	case i_shi:	one_op(0x52C0, B); break;
	case i_sls:	one_op(0x53C0, B); break;
	case i_scc:	one_op(0x54C0, B); break;
	case i_scs:	one_op(0x55C0, B); break;
	case i_sne:	one_op(0x56C0, B); break;
	case i_seq:	one_op(0x57C0, B); break;
	case i_svc:	one_op(0x58C0, B); break;
	case i_svs:	one_op(0x59C0, B); break;
	case i_spl:	one_op(0x5AC0, B); break;
	case i_smi:	one_op(0x5BC0, B); break;
	case i_sge:	one_op(0x5CC0, B); break;
	case i_slt:	one_op(0x5DC0, B); break;
	case i_sgt:	one_op(0x5EC0, B); break;
	case i_sle:	one_op(0x5FC0, B); break;

	case i_pea:	pea_op(); break;
	case i_jmp:	ctrl_op(0x4EC0); break;
	case i_jsr:	ctrl_op(0x4E80); break;

	case i_movb:	move_op(0x1000, B); break;
	case i_movw:	move_op(0x3000, W); break;
	case i_movl:	move_op(0x2000, L); break;

	case i_orb:	two_op(0x8000, 0x0000, B, 0); break;
	case i_orw:	two_op(0x8000, 0x0000, W, 0); break;
	case i_orl:	two_op(0x8000, 0x0000, L, 0); break;
	case i_subb:	two_op(0x9000, 0x0400, B, 0); break;
	case i_subw:	two_op(0x9000, 0x0400, W, 0); break;
	case i_subl:	two_op(0x9000, 0x0400, L, 0); break;
	case i_cmpb:	two_op(0xB000, 0x0C00, B, 0); break;
	case i_cmpw:	two_op(0xB000, 0x0C00, W, 0); break;
	case i_cmpl:	two_op(0xB000, 0x0C00, L, 0); break;
	case i_eorb:	two_op(0xB100, 0x0A00, B, 0); break;
	case i_eorw:	two_op(0xB100, 0x0A00, W, 0); break;
	case i_eorl:	two_op(0xB100, 0x0A00, L, 0); break;
	case i_andb:	two_op(0xC000, 0x0200, B, 0); break;
	case i_andw:	two_op(0xC000, 0x0200, W, 0); break;
	case i_andl:	two_op(0xC000, 0x0200, L, 0); break;
	case i_addb:	two_op(0xD000, 0x0600, B, 0); break;
	case i_addw:	two_op(0xD000, 0x0600, W, 0); break;
	case i_addl:	two_op(0xD000, 0x0600, L, 0); break;

	case i_addqb:	two_op(0x5000, 0x5000, B, 1); break;
	case i_addqw:	two_op(0x5000, 0x5000, W, 1); break;
	case i_addql:	two_op(0x5000, 0x5000, L, 1); break;
	case i_subqb:	two_op(0x5100, 0x5100, B, 1); break;
	case i_subqw:	two_op(0x5100, 0x5100, W, 1); break;
	case i_subql:	two_op(0x5100, 0x5100, L, 1); break;

	case i_jra:	jbrnch(0x6000,0x4EC0); break;
	case i_jhi:	cbrnch(0x6200); break;
	case i_jcc:	cbrnch(0x6400); break;
	case i_jne:	cbrnch(0x6600); break;
	case i_jvc:	cbrnch(0x6800); break;
	case i_jpl:	cbrnch(0x6A00); break;
	case i_jge:	cbrnch(0x6C00); break;
	case i_jgt:	cbrnch(0x6E00); break;
	case i_jbsr:	jbrnch(0x6100,0x4E80); break;
	case i_jls:	cbrnch(0x6300); break;
	case i_jcs:	cbrnch(0x6500); break;
	case i_jeq:	cbrnch(0x6700); break;
	case i_jvs:	cbrnch(0x6900); break;
	case i_jmi:	cbrnch(0x6B00); break;
	case i_jlt:	cbrnch(0x6D00); break;
	case i_jle:	cbrnch(0x6F00); break;

	case i_bra:	branch(0x6000); break;
	case i_bhi:	branch(0x6200); break;
	case i_bcc:	branch(0x6400); break;
	case i_bne:	branch(0x6600); break;
	case i_bvc:	branch(0x6800); break;
	case i_bpl:	branch(0x6A00); break;
	case i_bge:	branch(0x6C00); break;
	case i_bgt:	branch(0x6E00); break;
	case i_bsr:	branch(0x6100); break;
	case i_bls:	branch(0x6300); break;
	case i_bcs:	branch(0x6500); break;
	case i_beq:	branch(0x6700); break;
	case i_bvs:	branch(0x6900); break;
	case i_bmi:	branch(0x6B00); break;
	case i_blt:	branch(0x6D00); break;
	case i_ble:	branch(0x6F00); break;

	case i_bras:	brnchs(0x6000); break;
	case i_bhis:	brnchs(0x6200); break;
	case i_bccs:	brnchs(0x6400); break;
	case i_bnes:	brnchs(0x6600); break;
	case i_bvcs:	brnchs(0x6800); break;
	case i_bpls:	brnchs(0x6A00); break;
	case i_bges:	brnchs(0x6C00); break;
	case i_bgts:	brnchs(0x6E00); break;
	case i_bsrs:	brnchs(0x6100); break;
	case i_blss:	brnchs(0x6300); break;
	case i_bcss:	brnchs(0x6500); break;
	case i_beqs:	brnchs(0x6700); break;
	case i_bvss:	brnchs(0x6900); break;
	case i_bmis:	brnchs(0x6B00); break;
	case i_blts:	brnchs(0x6D00); break;
	case i_bles:	brnchs(0x6F00); break;

	case i_abcd:	regmem(0xC100); break;
	case i_sbcd:	regmem(0x8100); break;
	case i_addxb:	regmem(0xD100); break;
	case i_addxw:	regmem(0xD140);	break;
	case i_addxl:	regmem(0xD180); break;
	case i_subxb:	regmem(0x9100); break;
	case i_subxw:	regmem(0x9140); break;
	case i_subxl:	regmem(0x9180); break;

	case i_asrb:	shift_op(0xE000); break;
	case i_asrw:	shift_op(0xE040); break;
	case i_asrl:	shift_op(0xE080); break;
	case i_aslb:	shift_op(0xE100); break;
	case i_aslw:	shift_op(0xE140); break;
	case i_asll:	shift_op(0xE180); break;
	case i_lsrb:	shift_op(0xE008); break;
	case i_lsrw:	shift_op(0xE048); break;
	case i_lsrl:	shift_op(0xE088); break;
	case i_lslb:	shift_op(0xE108); break;
	case i_lslw:	shift_op(0xE148); break;
	case i_lsll:	shift_op(0xE188); break;
	case i_rorb:	shift_op(0xE018); break;
	case i_rorw:	shift_op(0xE058); break;
	case i_rorl:	shift_op(0xE098); break;
	case i_rolb:	shift_op(0xE118); break;
	case i_rolw:	shift_op(0xE158); break;
	case i_roll:	shift_op(0xE198); break;
	case i_roxrb:	shift_op(0xE010); break;
	case i_roxrw:	shift_op(0xE050); break;
	case i_roxrl:	shift_op(0xE090); break;
	case i_roxlb:	shift_op(0xE110); break;
	case i_roxlw:	shift_op(0xE150); break;
	case i_roxll:	shift_op(0xE190); break;

	case i_swap:	reg_op(0x4840, DR); break;
	case i_extw:	reg_op(0x4880, DR); break;
	case i_extl:	reg_op(0x48C0, DR); break;
	case i_unlk:	reg_op(0x4E58, AR); break;

	case i_cmpmb:	postinc(0xB108); break;
	case i_cmpmw:	postinc(0xB148); break;
	case i_cmpml:	postinc(0xB188); break;

	case i_bchg:	bit_op(0x0040); break;
	case i_bclr:	bit_op(0x0080); break;
	case i_bset:	bit_op(0x00C0); break;
	case i_btst:	bit_op(0x0000); break;

	case i_chk:	memreg(0x4180); break;
	case i_divs:	memreg(0x81C0); break;
	case i_divu:	memreg(0x80C0); break;
	case i_muls:	memreg(0xC1C0); break;
	case i_mulu:	memreg(0xC0C0); break;
/* miscellaneous classes */

	case i_dbcc:	regbrnch(0x54C8); break;
	case i_dbcs:	regbrnch(0x55C8); break;
	case i_dbeq:	regbrnch(0x57C8); break;
	case i_dbra:
	case i_dbf:	regbrnch(0x51C8); break;
	case i_dbge:	regbrnch(0x5CC8); break;
	case i_dbgt:	regbrnch(0x5EC8); break;
	case i_dbhi:	regbrnch(0x52C8); break;
	case i_dble:	regbrnch(0x5FC8); break;
	case i_dbls:	regbrnch(0x53C8); break;
	case i_dblt:	regbrnch(0x5DC8); break;
	case i_dbmi:	regbrnch(0x5BC8); break;
	case i_dbne:	regbrnch(0x56C8); break;
	case i_dbpl:	regbrnch(0x5AC8); break;
	case i_dbt:	regbrnch(0x50C8); break;
	case i_dbvc:	regbrnch(0x58C8); break;
	case i_dbvs:	regbrnch(0x59C8); break;

	case i_exg:	exg_op(0xC100); break;

	case i_lea:	addr_op(0x41C0, L); break;

	case i_link:	link_op(0x4E50); break;

	case i_movemw:	movem_op(0x4880); break;
	case i_moveml:	movem_op(0x48C0); break;

	case i_movepw:	movep_op(0x0108); break;
	case i_movepl:	movep_op(0x0148); break;

	case i_moveq:	moveq(0x7000); break;

	case i_trap:	trap(0x4E40); break;

	case i_rtd:	if (!f68010) goto error;
			rtd_op(0x4E74); break;

	case i_movec:	if (!f68010) goto error;
			movec_op(0x4E7A); break;

	case i_movesb:	i = B; goto movesx;
	case i_movesw:	i = W; goto movesx;
	case i_movesl:	i = L;
	movesx:		if (!f68010) goto error;
			moves_op(0x0E00, i); break;

	/* pseudo ops */
	case i_space:	if (numops != 1) Prog_Error(E_OPERAND);
			nolist = 1;
			j = operands[0].value_o;
			a[0] = j >> 16;
			a[1] = j;
			listdata(2, W, a);
			temp = 0;
			while (j-- > 0)
				Put_Text(&temp, 1);
			BC += operands[0].value_o;
			goto pseudo;
	case i_long:	if (Dot & 1) Prog_Error(E_ODDADDR);
			nolist = 1;
			ByteWord(L);
			listwords(L);
			goto pseudo;
	case i_word:	if (Dot & 1) Prog_Error(E_ODDADDR);
			nolist = 1;
			ByteWord(W);
			listwords(W);
			goto pseudo;
	case i_byte:	nolist = 1;
			ByteWord(B);
			listwords(B);
			goto pseudo;
	case i_text:	if (labsw) Prog_Error(E_LABEL);
			New_Csect(Text_csect);
			listline();
			goto pseudo;
	case i_data:	if (labsw) Prog_Error(E_LABEL);
			New_Csect(Data_csect);
			listline();
			goto pseudo;
	case i_bss:	if (labsw) Prog_Error(E_LABEL);
			New_Csect(Bss_csect);
			j = operands[0].value_o;
			a[0] = j >> 16;
			a[1] = j;
			listdata(-2, W, a);
			goto pseudo;
	case i_globl:	if (labsw) Prog_Error(E_LABEL);
			Globl();
			listline();
			goto pseudo;
	case i_comm:	if (labsw) Prog_Error(E_LABEL);
			Comm();
			j = operands[1].value_o;
			a[0] = j >> 16;
			a[1] = j;
			listdata(-2, W, a);
			goto pseudo;
	case i_even:	if (labsw) Prog_Error(E_LABEL);
			nolist = 1;
			Even();
			listline();
			goto pseudo;
	case i_ascii:	Ascii(0);
			goto pseudo;
	case i_asciz:	Ascii(1);
/* GB stabs */
			goto pseudo;
        case i_stabs:
        case i_stabd:
        case i_stabn:   Stab( opindex ); goto pseudo;
/**/
	pseudo:		Code_length = 0;
			break;

	default:	listline();
	error:		Prog_Error(E_OPCODE);
	};

	if (Code_length) {
	  Put_Words(Code,Code_length);	/* output text */
	  BC = Code_length;		/* increment LC */
	}
}
/* move_op - uses two effective addresses */

move_op(opr, size)
{
	register struct oper *op1, *op2;
	register int r1, r2;

	op1 = operands;
	op2 = &operands[1];
	r1 = (int)op1->value_o;
	r2 = (int)op2->value_o;

	if (numops != 2) Prog_Error(E_NUMOPS);
	else if (f68010 && op1->type_o == t_reg && ccreg(r1) &&
			data_addr(op2) && alt_addr(op2) && size == W)
	{
		WCode[0] = 0x42C0;
		eaddr(op2, W);
	}
	else if (op1->type_o == t_reg && srreg(r1) &&
			data_addr(op2) && alt_addr(op2) && size == W)
	{
		WCode[0] = 0x40C0;
		eaddr(op2, W);
	}
	else if (op2->type_o == t_reg && ccreg(r2) && data_addr(op1)
			&& size == W)
	{
		WCode[0] = 0x44C0;
		eaddr(op1, W);
	}
	else if (op2->type_o == t_reg && srreg(r2) && data_addr(op1)
			&& size == W)
	{
		WCode[0] = 0x46C0;
		eaddr(op1, W);
	}
	else if (op1->type_o == t_reg && uspreg(r1) && size == L)
	{
		if (op2->type_o != t_reg || !areg(r2)) Prog_Error(E_REG);
		WCode[0] = 0x4E68 | (r2 & 07);
	}
	else if (op2->type_o == t_reg && uspreg(r2) && size == L)
	{
		if (op1->type_o != t_reg || !areg(r1)) Prog_Error(E_REG);
		WCode[0] = 0x4E60 | (r1 & 07);
	}
	else if (adrbyte(op1, size) && !sr_addr(op1) && !cr_addr(op1)
			&& data_addr(op2) && alt_addr(op2))
	{
		char reg, mode;
		int source;
		WCode[0] = 0;
		eaddr(op1, size);		/* get source address */
		source = WCode[0];		/* save it */
		WCode[0] = 0;
		eaddr(op2, size);		/* get destination address */
		reg = WCode[0] & 07;
		mode = (WCode[0] >> 3) & 07;
		WCode[0] = opr | (reg << 9) | (mode << 6) | source;
	}
	else if (op2->type_o == t_reg && areg(r2) && size != B
			&& !sr_addr(op1) && !cr_addr(op1))
	{
		WCode[0] = 0x0040|((size==W)?0x3000:0x2000)|((r2 & 07)<<9);
		eaddr(op1, size);
	}
	else
	{
		WCode[0] = 0;
		Prog_Error(E_OPERAND);
	}
}
/* two_ops - these are of the forms:
	xxx Dn,<eaddr>
	xxx <eaddr>,Dn
	xxx #yyy,<eaddr>
	xxx <eaddr>,An
 */

two_op(opr, iopr, size, quick)
int opr;	/* normal operand field */
int iopr;	/* operand if immediate */
int size;	/* B, W, or L for the size of the operation */
int quick;	/* 1 => quick immediate instruction */
{
	register struct oper *op1, *op2;
	int r1, r2;
	int flag;	/* use to analyze addressing modes */
	op1 = operands;
	op2 = &operands[1];
	r1 = (int)op1->value_o;
	r2 = (int)op2->value_o;

	WCode[0] = opr | (size << 6);
	if (numops != 2) Prog_Error(E_NUMOPS);
	if (op1->type_o == t_immed)
	{
		if (op2->type_o == t_immed)
			Prog_Error(E_BADIMMED);
		WCode[0] = iopr | (size << 6);
		if (quick)
		{	
			if (op1->value_o > 8 || op1->value_o < 0)
				Prog_Error(E_CONSTANT);
			if (op1->value_o == 8) r1 = 0;	/* 0 represents 8 */
			WCode[0] |= (r1 & 07) << 9;
			if (alt_addr(op2)&&adrbyte(op2, size))eaddr(op2,size);
			else Prog_Error(E_OPERAND);
			return;
		}
		else
		{
			switch(opr)
			{
			case 0x0600:	/* add */
			case 0x0400:	/* sub */
			case 0x0C00:	/* cmp */
				flag = data_addr(op2) & alt_addr(op2);
				break;
			default:
				flag = (data_addr(op2) & alt_addr(op2)) |
					sr_addr(op2);
				break;
			}
			if (flag)
			{
				rel_val(op1, (size == L)? L:W); /* bytes to W*/
				eaddr(op2, size);
				return;
			}
		}
	}
	WCode[0] = opr | (size << 6);
	if (op1->type_o == t_reg && dreg((int)op1->value_o))
	{
		switch(opr)
		{
			case 0xB000:	/* cmp */
				flag = 0;	/* not allowed */
				break;
			case 0xB100:	/* eor */
				flag = data_addr(op2) & alt_addr(op2);
				break;
			default:
				flag = mem_addr(op2) & alt_addr(op2);
				break;
		}
		if (flag)
		{
			WCode[0] |= 0400|(((int)op1->value_o&07)<<9);
			eaddr(op2, size);
			return;
		}
	}
	if (op2->type_o == t_reg && dreg((int)op2->value_o))
	{
		switch(opr)
		{
			case 0xB000:	/* cmp */
			case 0xD000:	/* add */
			case 0x9000:	/* sub */
				flag = adrbyte(op1, size);
				break;
			case 0xB100:	/* eor */
				flag = 0;
				break;
			default:
				flag = data_addr(op1);
				break;
		}
		if (flag)
		{
			WCode[0] |= (((int)op2->value_o & 07) << 9);
			eaddr(op1, size);
			return;
		}
	}
	if (op2->type_o == t_reg && areg(r2) && size != B)
	{
		int op_mode;
		op_mode = (size == W)? 0300: 0700;
		switch(opr)
		{
		case 0xD000:	/* add */
		case 0x9000:	/* sub */
		case 0xB000:	/* cmp */
			WCode[0] = opr | ((r2 & 07) << 9) | op_mode;
			eaddr(op1, size);
			return;
		default:	;
		}
	}
	Prog_Error(E_OPERAND);
}
/* one_ops - install opr, check for exactly one operand and compute eaddr */

one_op(opr, size)
{
	register struct oper *op = operands;
	WCode[0] = opr;
	if (numops != 1) Prog_Error(E_NUMOPS);
	if (data_addr(op) && alt_addr(op)) eaddr(op, size);
	else Prog_Error(E_OPERAND);
}

stop_op(opr)
{
	register struct oper *op = operands;
	WCode[0] = opr;
	if (numops != 1) Prog_Error(E_NUMOPS);
	if (op->type_o != t_immed) Prog_Error(E_OPERAND);
	rel_val(op, W);
}


/* pea_op - like ctrl op, in that it requires control addressing modes;
   if operand is a constant of some sort (t_normal && no symbol) then
   see if we can fit it in 16 bits. Check for sign extension of the
   value (must be in range of [-32767 <= value <= 32767] */

pea_op()
{
	register struct oper *op = operands;

	WCode[0] = 0x4840;
	if (numops != 1)
		Prog_Error(E_NUMOPS);
	if (ctrl_addr(op)) {
		if ((op->type_o == t_normal) && (op->sym_o == NULL)) {
		    /* see if constant will fit in 16 bits */
			if ((op->value_o <= 32767) &&
			    (op->value_o >= -32767)) {
				WCode[0] |= 070;
				rel_val(op, W);
			} else {
				WCode[0] |= 071;
				rel_val(op, L);
			}
		} else
			eaddr(op, W);
	} else
		Prog_Error(E_OPERAND);
}

/* ctrl_op - like one op but requires control addressing modes */

ctrl_op(opr)
{
	register struct oper *op = operands;
	WCode[0] = opr;
	if (numops != 1) Prog_Error(E_NUMOPS);
	if (ctrl_addr(op)) eaddr(op, W);
	else Prog_Error(E_OPERAND);
}


/* no_op(opr) -- places opr in WCode[0].  Ensures there are no operands. */

no_op(opr)
{
	WCode[0] = opr;
	if (numops != 0) Prog_Error(E_NUMOPS);
};

/* branch - process branch offsets */
branch(opr)
{
	long offs = 0;
	register struct oper *opp = operands;
	extern struct csect *Cur_csect;

	WCode[0] = opr;
	if (numops != 1) Prog_Error(E_NUMOPS);
	else if (opp->type_o == t_reg) Prog_Error(E_REG);
	else if (opp->sym_o == 0 || opp->sym_o->csect_s != Cur_csect)
		Prog_Error(E_RELOCATE);
	else
	{
		offs = opp->value_o - (Dot + 2);
		if (offs > 32767L || offs < -32768L)
			Prog_Error(E_OFFSET);
	}
	opp->value_o = (int)offs;
	opp->sym_o = 0;	/* mark as none relocateable expression */
	rel_val(opp, W);
}


brnchs(opr)
{
	long offs = 0;
	register struct oper *opp = operands;
	extern struct csect *Cur_csect;

	WCode[0] = opr;
	if (numops != 1) Prog_Error(E_NUMOPS);
	else if (opp->type_o == t_reg) Prog_Error(E_REG);
	else if (opp->sym_o == 0 || opp->sym_o->csect_s != Cur_csect)
		Prog_Error(E_RELOCATE);
	else
	{
		offs = opp->value_o - (Dot + 2);
		if (offs > 127 || offs < -128)
			Prog_Error(E_OFFSET);
	}
	if (offs != 0)
		WCode[0] |= offs&0377;
	else if (opr != 0x6100)	/* not a bsr, offset 0 => long address, so do a nop instead */
		WCode[0] = 0x4e71;
	else
		Prog_Error(E_OFFSET);
}
/*
 * generate a short or a long branch instruction as appropriate
 * note that:
 *		jra	foo
 *	foo:
 * is translated (by brnchs) to a nop.  If we attempt to
 * optimize this to generate no code, then after resolving
 * the span-dependent instructions, the value of
 * foo is the same as the address of the jra instruction.
 * Therefore, on pass 2, the jra looks like a jra .
 * Presumably, this could be fixed by keeping more information
 * after the sdi's are resolved
 */
jbrnch(opr,lopr)
{
	long offs = 0;
	register struct oper *opp = operands;
	extern struct csect *Cur_csect;

	if (numops != 1) Prog_Error(E_NUMOPS);
	else if ((opp->type_o != t_normal)&&(opp->type_o != t_abss))
	/* not a direct address */
		ctrl_op(lopr);	/* jmp or jsr as appropriate */
	else {
		offs = opp->value_o - (Dot + 2);
		if (opp->flags_o & O_COMPLEX)	/* not a simple address */
			ctrl_op(lopr);		/* use the long form */
		else if (Pass == 1)
			Code_length = makesdi(opp, 6, Dot + 2,
			 sdi_bound(2, -128L, 127L,
			  sdi_bound(4, -32768L, 32767L, 0)));
		else if (opp->sym_o == 0		/* absolute address */
		 || opp->sym_o->csect_s != Cur_csect	/* not in the same csect */
		 || offs < -32768L || offs > 32767L)	/* offset too large */
			ctrl_op(lopr);
		else if (offs > 127 || offs < -128)
			branch(opr);
		else
			brnchs(opr);
	}
}

/*
 * generate a short or a long conditional branch instruction as appropriate
 * see the comments preceding jbrnch
 */
cbrnch(opr)
{
	long offs = 0;
	register struct oper *opp = operands;
	extern struct csect *Cur_csect;

	if (numops != 1) Prog_Error(E_NUMOPS);
	else if ((opp->type_o != t_normal)&&(opp->type_o != t_abss))
	/* not a direct address */
		baround(opr);
	else {
		offs = opp->value_o - (Dot + 2);
		if (opp->flags_o & O_COMPLEX)	/* not a simple address */
			baround(opr);
		else if (Pass == 1)
			Code_length = makesdi(opp, 6, Dot + 2,
			 sdi_bound(2, -128L, 127L,
			  sdi_bound(4, -32768L, 32767L, 0)));
		else if (opp->sym_o == 0	/* absolute address */
		 || opp->sym_o->csect_s != Cur_csect /* not in the same csect */
		 || offs < -32768L || offs > 32767L)	/* offset too large */
			baround(opr);
		else if (offs > 127 || offs < -128)
			branch(opr);
		else
			brnchs(opr);
	}
}

/*
 * generate a conditional branch around a jmp
 */
baround(opr)
{
	WCode[0] = opr ^ 0X106;	/* reverse the sense of the condition */
	WCode[1] = 0x4EC0 | 072;	/* jmp xxxxxx */
	Code_length = 4;		/* so far */
	rel_val(&operands[0], L);	/* the address */
}

/* regmem - either register register or memory memory instructions */

regmem(opr)
{
	register struct oper *op1, *op2;	/* operands */
	int sr, dr;				/* registers */

	op1 = operands;
	op2 = &operands[1];
	sr = (int)op1->value_o;
	dr = (int)op2->value_o;
	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	else switch(op1->type_o)
	{
	case t_reg:
		if (op2->type_o == t_reg && dreg(sr) && dreg(dr))
			WCode[0] |= sr | (dr << 9);		
		else Prog_Error(E_OPERAND);
		break;
	case t_predec:
		if (op2->type_o == t_predec && areg(sr) && areg(dr))
			WCode[0] |= (sr & 07) | ((dr & 07) << 9) | 010;
		else Prog_Error(E_OPERAND);
		break;
	default:
		Prog_Error(E_OPERAND);
	}
}


/* addr_op - things of the form lea <eaddr>,ax */

addr_op(opr, size)
{
	register struct oper *op1, *op2;
	int ar;	/* address register */

	op1 = operands;
	op2 = &operands[1];

	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	eaddr(op1, size);		/* get source */
	ar = (int)op2->value_o;
	if (op2->type_o == t_reg && areg(ar))
		WCode[0] |= (ar & 07) << 9;
	else Prog_Error(E_OPERAND);
}

/* shift op -	shift either a register or an effective address */

shift_op(opr)
{
	register struct oper *op1, *op2;
	short shift_type;
	op1 = &operands[0];
	op2 = &operands[1];

	WCode[0] = opr;
	if (numops == 1)
	{
		shift_type = opr & 030;	/* get type of shift */
		WCode[0] &= ~030;		/* mask it out */
		WCode[0] |= shift_type << 6;	/* and move it left */
		WCode[0] |= 0300;	/* size field 3 for eaddr */
		if (mem_addr(op1) && alt_addr(op1)) eaddr(op1, W);
		else Prog_Error(E_OPERAND);
		return;
	}
	if (numops == 2)
	{
		int val1, val2;
		val1 = (int)op1->value_o;
		val2 = (int)op2->value_o;
		if (op1->type_o==t_immed
			&& op2->type_o == t_reg && dreg(val2))
		{
			if ((op1->value_o <= 0) || (op1->value_o > 8))
				Prog_Error(E_CONSTANT);
			if (val1 == 8) val1 = 0;
			WCode[0] |= ((val1 & 07) << 9) | (val2 & 07);
			return;
		}
		else if (op1->type_o==t_reg && dreg(val1)
			&& op2->type_o == t_reg && dreg(val2))
		{
			WCode[0] |= ((val1 & 07) << 9) | (val2 & 07) | 040;
			return;
		}
	}
	Prog_Error(E_OPERAND);
}
/* bit_op - of the form xxx Dn,<eaddr> or xxx #nnn,<eaddr> */

bit_op(opr)
{
	register struct oper *op1, *op2;
	op1 = operands;
	op2 = &operands[1];

	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	else if (op1->type_o == t_reg && dreg((int)op1->value_o) &&
		data_addr(op2) && (alt_addr(op2) || opr == 0))
	{
		/* <eaddr> is destination */
		WCode[0] |= 0400 | ((int)op1->value_o << 9);
		eaddr(op2, W);
	}
	else if (op1->type_o == t_immed && data_addr(op2) &&
		(alt_addr(op2) || (opr == 0 && op2->type_o != t_immed)))
	{
		WCode[0] |= 04000;
		rel_val(op1, W);
		eaddr(op2, W);
	}
	else Prog_Error(E_OPERAND);
}
/* memreg - instructions of the form xxx <eaddr>,Dn  eg. divu */

memreg(opr)
{
	register struct oper *op1, *op2;
	int dr;	/* data register */

	op1 = operands;
	op2 = &operands[1];

	WCode[0] = opr;
	dr = (int)op2->value_o;
	if (numops != 2) Prog_Error(E_NUMOPS);
	if (data_addr(op1) && op2->type_o == t_reg && dreg(dr))
	{
		WCode[0] |= (dr & 07) << 9;
		eaddr(op1, W);		/* get source */
	}
	else Prog_Error(E_OPERAND);
}


/* postinc - instructions of the form xxx Ax@+,Ay@+ */

postinc(opr)
{
	register struct oper *op1, *op2;
	int ar1, ar2;

	op1 = operands;
	op2 = &operands[1];
	ar1 = (int)op1->value_o;
	ar2 = (int)op2->value_o;

	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	else if (op1->type_o == t_postinc && areg(ar1) &&
		op2->type_o == t_postinc && areg(ar2))
		WCode[0] |= ((ar2 & 07) << 9) | (ar1 & 07);
	else Prog_Error(E_OPERAND);
}

/* regbrnch - instructions like DBcc Dn,<label> */

regbrnch(opr)
{
	long offs = 0;
	int dr;
	register struct oper *op1, *op2;
	op1 = operands;
	op2 = &operands[1];
	dr = (int)op1->value_o;
	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	/* we should check here for relocatable expression */
	else if (op1->type_o == t_reg && dreg(dr))
	{
		WCode[0] |= dr;
		offs = op2->value_o - (Dot + 2);
		if (((offs<<16)>>16) != offs) Prog_Error(E_OFFSET);
		WCode[1] = offs;
		Code_length = 4;
	} else Prog_Error(E_DBRA);
}

/* exg_op - instructions like exg rx,ry */

exg_op(opr)
{
	int r1, r2;
	register struct oper *op1, *op2;
	op1 = operands;
	op2 = &operands[1];
	r1 = (int)op1->value_o;
	r2 = (int)op2->value_o;

	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	else if (op1->type_o == t_reg && op2->type_o == t_reg)
	{
		if (dreg(r1) && dreg(r2))
			WCode[0] |= 0100 | (r1 << 9) | r2;
		else if (areg(r1) && areg(r2))
			WCode[0] |= 0110 | ((r1 & 07) << 9) | (r2 & 07);
		else if (areg(r1) && dreg(r2))
			WCode[0] |= 0210 | (r2 << 9) | (r1 & 07);
		else if (dreg(r1) && areg(r2))
			WCode[0] |= 0210 | (r1 << 9) | (r2 & 07);
		else Prog_Error(E_REG);
	}
	else Prog_Error(E_OPERAND);
}		

/* movec_op - form: movec rc,rx  or  movec rx,rc */

movec_op(opr)
{
	int r1, r2;
	register struct oper *op1, *op2;
	op1 = operands;
	op2 = &operands[1];
	r1 = (int)op1->value_o;
	r2 = (int)op2->value_o;

	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	else if (op1->type_o == t_reg && op2->type_o == t_reg)
	{
		if (cr_addr(op1) && (dreg(r2) || areg(r2)))
		{
			/* (reg - 21) & 0x 801 is a kludge to
			 * get the correct control register code
			 */
			op2->disp_o = (r1 - 21) & 0x801;
			qindex(op2);		/* compute quasi-index */
			rel_val(op2, W);	/* install quasi-index */
		}
		else if (cr_addr(op2) && (dreg(r1) || areg(r1)))
		{
			WCode[0] |= 01;
			op1->disp_o = (r2 - 21) & 0x801;
			qindex(op1);		/* compute quasi-index */
			rel_val(op1, W);	/* install quasi-index */
		}
		else Prog_Error(E_REG);
	}
	else Prog_Error(E_OPERAND);
}		
/* moves_op - form: movesx <eaddr>,rx  or  movesx rx,<eaddr> */

moves_op(opr, size)
int opr;	/* normal operand field */
int size;	/* B, W, or L for the size of the operation */
{
	register struct oper *op1, *op2;
	int r1, r2;
	int flag;	/* use to analyze addressing modes */
	op1 = operands;
	op2 = &operands[1];
	r1 = (int)op1->value_o;
	r2 = (int)op2->value_o;

	WCode[0] = opr | (size << 6);
	if (numops != 2) Prog_Error(E_NUMOPS);
	else if (mem_addr(op1) && alt_addr(op1) && op2->type_o == t_reg
	    && (dreg(r2) || areg(r2)))
	{
		op2->disp_o = 0;
		qindex(op2);		/* compute quasi-index */
		rel_val(op2, W);	/* install quasi-index */
		eaddr(op1, W);		/* get source */
	}
	else if (mem_addr(op2) && alt_addr(op2) && op1->type_o == t_reg
	    && (dreg(r1) || areg(r1)))
	{
		op1->disp_o = 0x800;
		qindex(op1);		/* compute quasi-index */
		rel_val(op1, W);	/* install quasi-index */
		eaddr(op2, W);		/* get destination */
	}
	else Prog_Error(E_OPERAND);
}
/* reg_op - instructions of the form xxx dn, if flag == D else xxx an */

reg_op(opr, flag)
{
	register struct oper *opp = operands;
	int reg = (int)opp->value_o;
	if (numops != 1) Prog_Error(E_NUMOPS);
	if (opp->type_o != t_reg) Prog_Error(E_OPERAND);
	if ((flag == DR && !dreg(reg)) || (flag == AR && !areg(reg)))
		Prog_Error(E_REG);
	WCode[0] = opr | (reg & 07);
}


/* link_op - form: link An,#<disp> */

link_op(opr)
{
	int ar;
	register struct oper *op1, *op2;
	op1 = operands;
	op2 = &operands[1];
	if (numops != 2) Prog_Error(E_NUMOPS);
	if (op1->type_o != t_reg || op2->type_o != t_immed)
		Prog_Error(E_OPERAND);
	ar = (int)op1->value_o;
	if (!areg(ar)) Prog_Error(E_REG);
	WCode[0] = opr | (ar & 07);
	rel_val(op2, W);
}


/* rtd_op - form: rtd #<disp> */

rtd_op(opr)
{
	int ar;
	register struct oper *opp = operands;
	if (numops != 1) Prog_Error(E_NUMOPS);
	if (opp->type_o != t_immed)
		Prog_Error(E_OPERAND);
	WCode[0] = opr;
	rel_val(opp, W);
}


/* movem_op -	of the form: movem #xxx,<eaddr> or movem <eaddr>,#xxx */

movem_op(opr)
{
	register struct oper *op1, *op2;
	op1 = operands;
	op2 = &operands[1];
	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	if (op1->type_o == t_immed && (ctrl_addr(op2)||op2->type_o==t_predec))
	{
		rel_val(op1, W);
		eaddr(op2, W);
	}
	else if(op2->type_o == t_immed &&
		(ctrl_addr(op1) || op1->type_o == t_postinc))
	{
		WCode[0] |= 02000;	/* memory to register flag */
		rel_val(op2, W);
		eaddr(op1, W);
	}
	else Prog_Error(E_OPERAND);
}
/* movep_op - of the form:  movep Dx,Ay@(d) or movep Ay@(d),Dx */

movep_op(opr)
{
	register struct oper *op1, *op2;
	int r1, r2;
	op1 = operands;
	op2 = &operands[1];
	r1 = (int)op1->value_o;
	r2 = (int)op2->value_o;
	WCode[0] = opr;
	if (numops != 2) Prog_Error(E_NUMOPS);
	if (op1->type_o == t_reg && dreg(r1))
	{
		if (op2->type_o != t_displ || !areg(op2->reg_o))
			Prog_Error(E_OPERAND);
		WCode[0] |= (r1 << 9) | 0600 | (op2->reg_o & 07);
		rel_val(op2, W);		
	}
	else if (op2->type_o == t_reg && dreg(r2))
	{
		if (op1->type_o != t_displ || !areg(op1->reg_o))
			Prog_Error(E_OPERAND);
		WCode[0] |= (r2 << 9) | 0400 | (op1->reg_o & 07);
		rel_val(op1, W);
	}
	else Prog_Error(E_OPERAND);
}	


/* moveq -  form: moveq #<data>,Dn */

moveq(opr)
{
	register struct oper *op1, *op2;
	int r2;
	op1 = operands;
	op2 = &operands[1];
	r2 = (int)op2->value_o;
	if (op1->value_o > 0177 || op1->value_o < -0200)
		Prog_Error(E_CONSTANT);
	WCode[0] = opr | ((r2 & 07) << 9) | ((short)op1->value_o & 0377);
	if (numops != 2) Prog_Error(E_NUMOPS);
	if (op1->type_o != t_immed || op2->type_o != t_reg || !dreg(r2))
		Prog_Error(E_OPERAND);
}


/* trap - form: trap #xxx */

trap(opr)
{
	register struct oper *opp = operands;
	if (numops != 1) Prog_Error(E_NUMOPS);
	if (opp->type_o != t_immed) Prog_Error(E_OPERAND);
	WCode[0] = opr | (((char)opp->value_o) & 017);
}


/* eaddr - put in stuff for an effective address */
eaddr(opptr, size)
register struct oper *opptr;
{
	int reg = (int)opptr->value_o;
	switch(opptr->type_o)
	{
	case t_reg:
		if (areg(reg) || dreg(reg))
			WCode[0] |= (int)opptr->value_o;
		else if (srreg(reg) || ccreg(reg))
			WCode[0] |= 074;
		else Prog_Error(E_REG);
		break;
	case t_defer:
		if (areg(reg))
			WCode[0] |= (((int)opptr->value_o) & 07) | 020;
		else Prog_Error(E_REG);
		break;
	case t_postinc:
		if (areg(reg))
			WCode[0] |= (((int)opptr->value_o) & 07) | 030;
		else Prog_Error(E_REG);
		break;
	case t_predec:
		if (areg(reg))
			WCode[0] |= (reg & 07) | 040;
		else Prog_Error(E_REG);
		break;
	case t_displ:
		if (areg(opptr->reg_o))
		{
			WCode[0] |= (opptr->reg_o & 07) | 050;
			rel_val(opptr, W);	/* install displacement */
		}
		else if (pcreg(opptr->reg_o))
		{
			WCode[0] |= 072;
			rel_val(opptr, W);	/* install displacement */
		}
		else Prog_Error(E_REG);
		break;
	case t_index:
		if (areg(opptr->reg_o))
		{
			WCode[0] |= (opptr->reg_o & 07) | 060;
			index(opptr);		/* compute index word */
			rel_val(opptr, W);	/* install index word */
		}
		else if (pcreg(opptr->reg_o))
		{
			WCode[0] |= 073;
			index(opptr);		/* compute index word */
			rel_val(opptr, W);	/* install index word */
		}
		else Prog_Error(E_REG);
		break;
	case t_abss:
		WCode[0] |= 070;
		rel_val(opptr, W);	/* install short address */
		break;
	case t_normal:
	case t_absl:
		if (quickf) {
			WCode[0] |= 070;
			rel_val(opptr, W);	/* install short address */
		} else {
			WCode[0] |= 071;
			rel_val(opptr, L);
		}
		break;
	case t_immed:
		WCode[0] |= 074;
		rel_val(opptr, (size == L)? L:W);	/* change bytes to W */
		break;
	default:
		Prog_Error(E_OPERAND);
		/* Sys_Error("Unrecognized address mode in line %s", iline); */
	}
}
/* index -	Use data in operand structure to compute an index word
		and leave the word in the operand value
 */

index(opptr)
register struct oper *opptr;
{
	int indexval = 0;

	if (areg((int)opptr->value_o) || dreg((int)opptr->value_o))
		indexval = (int)opptr->value_o << 12;
	else Prog_Error(E_REG);
	if (opptr->flags_o == O_LINDEX) indexval |= 0x0800;	/* else word */
	if (opptr->disp_o < 128 && opptr->disp_o >= -128)
		indexval |= (opptr->disp_o & 0xFF);
	else Prog_Error(E_OFFSET);
	opptr->value_o = indexval;
}

/* qindex -	Use data in operand structure to compute an index like
		word and leave the word in the operand value
 */

qindex(opptr)
register struct oper *opptr;
{
	int indexval = 0;

	if (areg((int)opptr->value_o) || dreg((int)opptr->value_o))
		indexval = (int)opptr->value_o << 12;
	else Prog_Error(E_REG);
	indexval |= (opptr->disp_o & 0xFFF);
	opptr->value_o = indexval;
}
/* areg -	Returns 1 if reg is an address register */

areg(reg) {	return(reg >= 8 && reg < 16); }


/* dreg -	Returns 1 if reg is a data register */

dreg(reg) {	return(reg >= 0 && reg < 8); }


/* uspreg -	Returns 1 if reg is usp */

uspreg(reg) {	return(reg == 19); }


/* pcreg -	Returns 1 if reg is pc */

pcreg(reg) {	return(reg == 16); }


/* srreg -	Returns 1 if reg is sr */

srreg(reg) {	return(reg == 18); }


/* ccreg -	Returns 1 if reg is cc */

ccreg(reg) {	return(reg == 17); }


/* vbrreg -	Returns 1 if reg is vbr */

vbrreg(reg) {	return(reg == 20); }


/* sfcreg -	Returns 1 if reg is sfc */

sfcreg(reg) {	return(reg == 21); }


/* dfcreg -	Returns 1 if reg is dfc */

dfcreg(reg) {	return(reg == 22); }


/* check for a data addressing mode */

data_addr(op)
register struct oper *op;
{
	switch(op->type_o)
	{
	case t_reg:
		if (areg((int)op->value_o)) return(0);
	default:
		return(1);
	}
}


/* check for an alterable data addressing mode */

alt_addr(op)
register struct oper *op;
{
	switch(op->type_o)
	{
	case t_immed:
		return(0);
	case t_displ:
	case t_index:
		if (pcreg(op->reg_o)) return(0);
	default:
		return(1);
	}
}


/* check for a memory addressing mode */

mem_addr(op)
register struct oper *op;
{
	switch(op->type_o)
	{
	case t_reg:
		return(0);
	default:
		return(1);
	}
}


/* check for a control addressing mode */

ctrl_addr(op)
register struct oper *op;
{
	switch(op->type_o)
	{
	case t_reg:
	case t_postinc:
	case t_predec:
	case t_immed:
		return(0);
	default:
		return(1);
	}
}


/* sr_addr - check to see if addressing mode is sr */

sr_addr(op)
register struct oper *op;
{
	int reg = (int)op->value_o;
	if (op->type_o == t_reg && (srreg(reg) || ccreg(reg)))
		return(1);
	return(0);
}


/* cr_addr - check to see if addressing mode is cr */

cr_addr(op)
register struct oper *op;
{
	int reg = (int)op->value_o;
	if (op->type_o == t_reg &&
	    (sfcreg(reg) || dfcreg(reg) || vbrreg(reg) || uspreg(reg)))
		return(1);
	return(0);
}


/* check to make sure that addr mode is not addr register direct if size = B */

adrbyte(op, size)
register struct oper *op;
register int size;	/* B,W or L */
{
	if (op->type_o == t_reg && size == B && areg((int)op->value_o))
		return(0);
	return(1);
}
