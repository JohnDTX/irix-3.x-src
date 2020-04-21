#include <stdio.h>
#include "globals.h"
#include "addrmodes.h"
#include "sym.h"
#include "inst.h"
int ea();
int r0_r9();
int ea_imm9();
int ea_imm();
int ea_r9();
int word_ext();
int long_ext();
int r9_imm0();
int r0_imm9();
int r0_imm();
int ea_r0();
int ext_r0();
int no_operands();
int r0();
int ea6_ea();
int spimm0();
int rext12_specialreg();
int r0_usp();
int r0disp_r9();
int ea_rext();
int cas_encode();
int cas2_encode();
int bfea();
int bfea_r();
int ea_rext2();

/*  the array base_inst.  This array should be initialized from the least
    to the most commonly used instruction, as the enter() routine links
    new entries to the FRONT of the hash chain. Additionally, within
    an instruction, the templates should be ordered with the most favorable
    (size, execution speed) FIRST.  Thus, if two instructions can move the
    immediate number 7 into d0, and inst A takes an immediate ea encoded in
    the following long and inst B encodes the small immediate operand in the
    instruction, instruction B should appear in the template first.

*/

struct template_s template[] = 
{
	{0xc100,A_DN,A_DN,r0_r9,A_SRC,0, },/* abcd */
	{0xc108,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0 },/* abcd */
	{0,	0,	0,	0,	0,},
	{0x5000,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* addb */
	{0x0600,A_CANBE_IMMB,(A_DN|A_ANYIND|A_ANYINDEX|A_ANYABS),ea_imm,A_DEST,0, },/* addb */
	{0xd100,A_DN,(A_ANYIND|A_ANYINDEX|A_ANYABS),ea_r9,A_DEST,0, },/* addb */
	{0xd000,A_ANYB,A_DN,ea_r9,A_SRC,0, },/* addb */
	{0,	0,	0,	0,	0,},
	{0x5080,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* addl */
	{0x0680,A_ANYIMM,(A_DN|A_ANYIND|A_ANYINDEX|A_ANYABS),ea_imm,A_DEST,0, },/* addl */
	{0xd1c0,A_ANY,A_AN,ea_r9,A_SRC,0, },/* addl */
	{0xd080,A_ANY,A_DN,ea_r9,A_SRC,0, },/* addl */
	{0xd180,A_DN,(A_ANYIND|A_ANYINDEX|A_ANYABS),ea_r9,A_DEST,0, },/* addl */
	{0,	0,	0,	0,	0,},
	{0x5000,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* addqb */
	{0,	0,	0,	0,	0,},
	{0x5080,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* addql */
	{0,	0,	0,	0,	0,},
	{0x5040,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* addqw */
	{0,	0,	0,	0,	0,},
	{0x5040,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* addw */
	{0x0640,A_CANBE_IMMW,(A_DN|A_ANYIND|A_ANYINDEX|A_ANYABS),ea_imm,A_DEST,0, },/* addw */
	{0xd040,A_ANYW,A_DN,ea_r9,A_SRC,0, },/* addw */
	{0xd140,A_DN,(A_ANYIND|A_ANYINDEX|A_ANYABS),ea_r9,A_DEST,0, },/* addw */
	{0xd0c0,A_ANYW,A_AN,ea_r9,A_SRC,0, },/* addw */
	{0,	0,	0,	0,	0,},
	{0xd100,A_DN,A_DN,r0_r9,A_SRC,0, },/* addxb */
	{0xd108,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0, },/* addxb */
	{0,	0,	0,	0,	0,},
	{0xd180,A_DN,A_DN,r0_r9,A_SRC,0, },/* addxl */
	{0xd188,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0, },/* addxl */
	{0,	0,	0,	0,	0,},
	{0xd140,A_DN,A_DN,r0_r9,A_SRC,0, },/* addxw */
	{0xd148,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0, },/* addxw */
	{0,	0,	0,	0,	0,},
	{0x0200,A_ANYIMM,(A_DN|A_ANYDATA),ea_imm,A_DEST,0, },/* andb */
	{0xc000,A_ANYBNOTAN,A_DN,ea_r9,A_SRC,0, },/* andb */
	{0xc100,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* andb */
	{0x023c,A_CANBE_IMMB,A_CCR,word_ext,A_SRC,0, },/* andb */
	{0,	0,	0,	0,	0,},
	{0x0280,A_ANYIMM,(A_DN|A_ANYDATA),ea_imm,A_DEST,0, },/* andl */
	{0xc080,A_ANYNOTAN,A_DN,ea_r9,A_SRC,0, },/* andl */
	{0xc180,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* andl */
	{0,	0,	0,	0,	0,},
	{0x0240,A_CANBE_IMMW,(A_DN|A_ANYDATA),ea_imm,A_DEST,0, },/* andw */
	{0xc040,A_ANYWNOTAN,A_DN,ea_r9,A_SRC,0, },/* andw */
	{0xc140,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* andw */
	{0x027c,A_CANBE_IMMW,A_SR,word_ext,A_SRC,0, },/* andw */
	{0,	0,	0,	0,	0,},
	{0xe100,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* aslb */
	{0xe120,A_DN,A_DN,r0_r9,A_DEST,0, },/* aslb */
	{0,	0,	0,	0,	0,},
	{0xe180,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* asll */
	{0xe1a0,A_DN,A_DN,r0_r9,A_DEST,0, },/* asll */
	{0,	0,	0,	0,	0,},
	{0xe140,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* aslw */
	{0xe160,A_DN,A_DN,r0_r9,A_DEST,0, },/* aslw */
	{0xe1c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* aslw */
	{0,	0,	0,	0,	0,},
	{0xe000,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* asrb */
	{0xe020,A_DN,A_DN,r0_r9,A_DEST,0, },/* asrb */
	{0,	0,	0,	0,	0,},
	{0xe080,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* asrl */
	{0xe0a0,A_DN,A_DN,r0_r9,A_DEST,0, },/* asrl */
	{0,	0,	0,	0,	0,},
	{0xe040,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* asrw */
	{0xe060,A_DN,A_DN,r0_r9,A_DEST,0, },/* asrw */
	{0xe0c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* asrw */
	{0,	0,	0,	0,	0,},
	{0x4848,A_NONE,A_CANBE_IMM0T7,spimm0,A_SINGLE,(X_IS68020|0x8), },/* bkpt */
	{0,	0,	0,	0,	0,},
	{0x64ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bcc */
	{0,	0,	0,	0,	0,},
	{0x65ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bcs */
	{0,	0,	0,	0,	0,},
	{0x67ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* beq */
	{0,	0,	0,	0,	0,},
	{0x6cff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bge */
	{0,	0,	0,	0,	0,},
	{0x6eff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bgt */
	{0,	0,	0,	0,	0,},
	{0x62ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bhi */
	{0,	0,	0,	0,	0,},
	{0x6fff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* ble */
	{0,	0,	0,	0,	0,},
	{0x63ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bls */
	{0,	0,	0,	0,	0,},
	{0x6dff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* blt */
	{0,	0,	0,	0,	0,},
	{0x6bff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bmi */
	{0,	0,	0,	0,	0,},
	{0x66ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bne */
	{0,	0,	0,	0,	0,},
	{0x6aff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bpl */
	{0,	0,	0,	0,	0,},
	{0x68ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bvc */
	{0,	0,	0,	0,	0,},
	{0x69ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bvs */
	{0,	0,	0,	0,	0,},
	{0x60ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bra */
	{0,	0,	0,	0,	0,},
	{0x64ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bccs */
	{0,	0,	0,	0,	0,},
	{0x65ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bcss */
	{0,	0,	0,	0,	0,},
	{0x67ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* beqs */
	{0,	0,	0,	0,	0,},
	{0x6cff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bges */
	{0,	0,	0,	0,	0,},
	{0x6eff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bgts */
	{0,	0,	0,	0,	0,},
	{0x62ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bhis */
	{0,	0,	0,	0,	0,},
	{0x6fff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bles */
	{0,	0,	0,	0,	0,},
	{0x63ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* blss */
	{0,	0,	0,	0,	0,},
	{0x6dff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* blts */
	{0,	0,	0,	0,	0,},
	{0x6bff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bmis */
	{0,	0,	0,	0,	0,},
	{0x66ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bnes */
	{0,	0,	0,	0,	0,},
	{0x6aff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bpls */
	{0,	0,	0,	0,	0,},
	{0x68ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bvcs */
	{0,	0,	0,	0,	0,},
	{0x69ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bvss */
	{0,	0,	0,	0,	0,},
	{0x60ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* bras */
	{0,	0,	0,	0,	0,},
	{0x6000,A_NONE,A_CANBE_DISPW,word_ext,A_SINGLE,(X_ISSDI|X_DONTCONDENSE), },/* braw */
	{0,	0,	0,	0,	0,},
	{0x0840,A_CANBE_IMM0T7,(A_ANYDATA),ea_imm,A_DEST,0, },/* bchg */
	{0x0840,A_CANBE_IMM0T31,A_DN,ea_imm,A_DEST,0, },/* bchg */
	{0x0140,A_DN,(A_ANYDATA|A_DN),ea_r9,A_DEST,0, },/* bchg */
	{0,	0,	0,	0,	0,},
	{0x0880,A_CANBE_IMM0T7,(A_ANYDATA),ea_imm,A_DEST,0, },/* bclr */
	{0x0880,A_CANBE_IMM0T31,A_DN,ea_imm,A_DEST,0, },/* bclr */
	{0x0180,A_DN,(A_ANYDATA|A_DN),ea_r9,A_DEST,0, },/* bclr */
	{0,	0,	0,	0,	0,},
	{0xeac0,A_NONE,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS),bfea,A_SINGLE,X_ISBITFIELD, },/* bfchg */
	{0,	0,	0,	0,	0,},
	{0xecc0,A_NONE,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS),bfea,A_SINGLE,X_ISBITFIELD, },/* bfclr */
	{0,	0,	0,	0,	0,},
	{0xebc0,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_DN,bfea_r,A_SRC,X_ISBITFIELD, },/* bfexts */
	{0,	0,	0,	0,	0,},
	{0xe9c0,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_DN,bfea_r,A_SRC,X_ISBITFIELD, },/* bfextu */
	{0,	0,	0,	0,	0,},
	{0xedc0,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_DN,bfea_r,A_SRC,X_ISBITFIELD, },/* bfffo */
	{0,	0,	0,	0,	0,},
	{0xefc0,A_DN,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS),bfea_r,A_SINGLE,X_ISBITFIELD, },/* bfins */
	{0,	0,	0,	0,	0,},
	{0xeec0,A_NONE,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS),bfea,A_SINGLE,X_ISBITFIELD, },/* bfset */
	{0,	0,	0,	0,	0,},
	{0xe8c0,A_NONE,(A_DN|A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),bfea,A_SINGLE,X_ISBITFIELD, },/* bftst */
	{0,	0,	0,	0,	0,},
	{0x08c0,A_CANBE_IMM0T7,(A_ANYDATA),ea_imm,A_DEST,0, },/* bset */
	{0x08c0,A_CANBE_IMM0T31,A_DN,ea_imm,A_DEST,0, },/* bset */
	{0x01c0,A_DN,(A_ANYDATA|A_DN),ea_r9,A_DEST,0, },/* bset */
	{0,	0,	0,	0,	0,},
	{0x0800,A_CANBE_IMM0T7,(A_ANYDATA),ea_imm,A_DEST,0, },/* btst */
	{0x0800,A_CANBE_IMM0T31,A_DN,ea_imm,A_DEST,0, },/* btst */
	{0x0100,A_DN,(A_ANYDATA|A_DN),ea_r9,A_DEST,0, },/* btst */
	{0,	0,	0,	0,	0,},
	{0x4eb9,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,(X_ISJSR|X_ISSDI), },/* bsr */
	{0x4e80,A_NONE,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYPC),ea,A_SINGLE,0, },/* bsr */
	{0,	0,	0,	0,	0,},
	{0x4eb9,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,(X_ISJSR|X_ISSDI), },/* bsrs */
	{0x4e80,A_NONE,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYPC),ea,A_SINGLE,0, },/* bsrs */
	{0,	0,	0,	0,	0,},
	{0x6100,A_NONE,A_CANBE_DISPW,word_ext,A_SINGLE,(X_DONTCONDENSE|X_ISSDI), },/* bsrw */
	{0x4e80,A_NONE,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYPC),ea,A_SINGLE,0, },/* bsrw */
	{0,	0,	0,	0,	0,},
	{0x06c0,A_CANBE_IMMB,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYPC),ea_imm,A_DEST,X_IS68020, },/* callm */
	{0,	0,	0,	0,	0,},
	{0x0ac0,A_DN,A_ANYDATA,cas_encode,A_THREEOPS,X_IS68020, },/* casb */
	{0,	0,	0,	0,	0,},
	{0x0ec0,A_DN,A_ANYDATA,cas_encode,A_THREEOPS,X_IS68020, },/* casl */
	{0,	0,	0,	0,	0,},
	{0x0cc0,A_DN,A_ANYDATA,cas_encode,A_THREEOPS,X_IS68020, },/* casw */
	{0,	0,	0,	0,	0,},
	{0x0efc,A_DOUBLEREG,A_DOUBLEIREG,cas2_encode,A_THREEOPS,X_IS68020, },/* cas2l */
	{0,	0,	0,	0,	0,},
	{0x0cfc,A_DOUBLEREG,A_DOUBLEIREG,cas2_encode,A_THREEOPS,X_IS68020, },/* cas2w */
	{0,	0,	0,	0,	0,},
	{0x4180,A_ANYWNOTAN,A_DN,ea_r9,A_SRC,0, },/* chk */
	{0,	0,	0,	0,	0,},
	{0x4100,A_ANYNOTAN,A_DN,ea_r9,A_SRC,X_IS68020, },/* chkl */
	{0,	0,	0,	0,	0,},
	{0x4180,A_ANYWNOTAN,A_DN,ea_r9,A_SRC,0, },/* chkw */
	{0,	0,	0,	0,	0,},
	{0x00c0,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_ANYREG,ea_rext,A_SRC,(X_FLAG0|X_IS68020), },/* chk2b */
	{0,	0,	0,	0,	0,},
	{0x04c0,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_ANYREG,ea_rext,A_SRC,(X_FLAG0|X_IS68020), },/* chk2l */
	{0,	0,	0,	0,	0,},
	{0x02c0,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_ANYREG,ea_rext,A_SRC,(X_FLAG0|X_IS68020), },/* chk2w */
	{0,	0,	0,	0,	0,},
	{0x4200,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* clrb */
	{0,	0,	0,	0,	0,},
	{0x4240,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* clrw */
	{0,	0,	0,	0,	0,},
	{0x4280,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* clrl */
	{0,	0,	0,	0,	0,},
	{0x0c00,A_CANBE_IMMB,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* cmpb */
	{0xb000,A_ANYBNOTAN,A_DN,ea_r9,A_SRC,0, },/* cmpb */
	{0xb108,A_ANINDPI,A_ANINDPI,r0_r9,A_SRC,0, },/* cmpb */
	{0,	0,	0,	0,	0,},
	{0x0c40,A_CANBE_IMMW,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* cmpw */
	{0xb040,A_ANYW,A_DN,ea_r9,A_SRC,0, },/* cmpw */
	{0xb0c0,A_ANYW,A_AN,ea_r9,A_SRC,0, },/* cmpw */
	{0xb148,A_ANINDPI,A_ANINDPI,r0_r9,A_SRC,0, },/* cmpw */
	{0,	0,	0,	0,	0,},
	{0x0c80,A_CANBE_IMML,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* cmpl */
	{0xb080,A_ANY,A_DN,ea_r9,A_SRC,0, },/* cmpl */
	{0xb1c0,A_ANY,A_AN,ea_r9,A_SRC,0, },/* cmpl */
	{0xb188,A_ANINDPI,A_ANINDPI,r0_r9,A_SRC,0, },/* cmpl */
	{0,	0,	0,	0,	0,},
	{0xb108,A_ANINDPI,A_ANINDPI,r0_r9,A_SRC,0, },/* cmpmb */
	{0,	0,	0,	0,	0,},
	{0xb188,A_ANINDPI,A_ANINDPI,r0_r9,A_SRC,0, },/* cmpml */
	{0,	0,	0,	0,	0,},
	{0xb148,A_ANINDPI,A_ANINDPI,r0_r9,A_SRC,0, },/* cmpmw */
	{0,	0,	0,	0,	0,},
	{0x00c0,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_ANYREG,ea_rext,A_SRC,(X_IS68020), },/* cmp2b */
	{0,	0,	0,	0,	0,},
	{0x04c0,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_ANYREG,ea_rext,A_SRC,(X_IS68020), },/* cmp2l */
	{0,	0,	0,	0,	0,},
	{0x02c0,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_ANYREG,ea_rext,A_SRC,(X_IS68020), },/* cmp2w */
	{0,	0,	0,	0,	0,},
	{0x54c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbcc */
	{0,	0,	0,	0,	0,},
	{0x55c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbcs */
	{0,	0,	0,	0,	0,},
	{0x57c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbeq */
	{0,	0,	0,	0,	0,},
	{0x51c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbf */
	{0,	0,	0,	0,	0,},
	{0x5cc8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbge */
	{0,	0,	0,	0,	0,},
	{0x5ec8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbgt */
	{0,	0,	0,	0,	0,},
	{0x52c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbhi */
	{0,	0,	0,	0,	0,},
	{0x5fc8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dble */
	{0,	0,	0,	0,	0,},
	{0x53c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbls */
	{0,	0,	0,	0,	0,},
	{0x5dc8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dblt */
	{0,	0,	0,	0,	0,},
	{0x5bc8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbmi */
	{0,	0,	0,	0,	0,},
	{0x56c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbne */
	{0,	0,	0,	0,	0,},
	{0x5ac8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbpl */
	{0,	0,	0,	0,	0,},
	{0x51c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbra */
	{0,	0,	0,	0,	0,},
	{0x50c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbt */
	{0,	0,	0,	0,	0,},
	{0x58c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbvc */
	{0,	0,	0,	0,	0,},
	{0x59c8,A_DN,A_CANBE_DISPW,ext_r0,A_DEST,0, },/* dbvs */
	{0,	0,	0,	0,	0,},
	{0x81c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* divs */
	{0,	0,	0,	0,	0,},
	{0x4c40,A_ANYNOTAN,A_DN,ea_rext2,A_SRC,(X_IS68020|X_FLAG0), },/* divsl */
	{0x4c40,A_ANYNOTAN,A_DOUBLEREG,ea_rext2,A_SRC,(X_IS68020|X_FLAG0|X_FLAG1), },/* divsl */
	{0,	0,	0,	0,	0,},
	{0x4c40,A_ANYNOTAN,A_DOUBLEREG,ea_rext2,A_SRC,(X_IS68020|X_FLAG0), },/* divsll */
	{0,	0,	0,	0,	0,},
	{0x81c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* divsw */
	{0,	0,	0,	0,	0,},
	{0x80c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* divu */
	{0,	0,	0,	0,	0,},
	{0x4c40,A_ANYNOTAN,A_DN,ea_rext2,A_SRC,X_IS68020, },/* divul */
	{0x4c40,A_ANYNOTAN,A_DOUBLEREG,ea_rext2,A_SRC,(X_IS68020|X_FLAG1), },/* divul */
	{0,	0,	0,	0,	0,},
	{0x4c40,A_ANYNOTAN,A_DOUBLEREG,ea_rext2,A_SRC,(X_IS68020), },/* divull */
	{0,	0,	0,	0,	0,},
	{0x80c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* divuw */
	{0,	0,	0,	0,	0,},
	{0x0a00,A_CANBE_IMMB,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* eorb */
	{0xb100,A_DN,(A_ANYDATA|A_DN),ea_r9,A_DEST,0, },/* eorb */
	{0x0a3c,A_CANBE_IMMB,A_CCR,word_ext,A_SRC,0, },/* eorb */
	{0,	0,	0,	0,	0,},
	{0x0a40,A_CANBE_IMMW,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* eorw */
	{0xb140,A_DN,(A_ANYDATA|A_DN),ea_r9,A_DEST,0, },/* eorw */
	{0x0a7c,A_CANBE_IMMW,A_SR,word_ext,A_SRC,0, },/* eorw */
	{0,	0,	0,	0,	0,},
	{0x0a80,A_CANBE_IMML,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* eorl */
	{0xb180,A_DN,(A_ANYDATA|A_DN),ea_r9,A_DEST,0, },/* eorl */
	{0,	0,	0,	0,	0,},
	{0xc140,A_DN,A_DN,r0_r9,A_DEST,0, },/* exg */
	{0xc148,A_AN,A_AN,r0_r9,A_DEST,0, },/* exg */
	{0xc188,A_DN,A_AN,r0_r9,A_DEST,0, },/* exg */
	{0,	0,	0,	0,	0,},
	{0xc140,A_DN,A_DN,r0_r9,A_DEST,0, },/* exgl */
	{0xc148,A_AN,A_AN,r0_r9,A_DEST,0, },/* exgl */
	{0xc188,A_DN,A_AN,r0_r9,A_DEST,0, },/* exgl */
	{0,	0,	0,	0,	0,},
	{0x4880,A_NONE,A_DN,r0,A_SINGLE,0, },/* extw */
	{0,	0,	0,	0,	0,},
	{0x4880,A_NONE,A_DN,r0,A_SINGLE,0, },/* extbw */
	{0,	0,	0,	0,	0,},
	{0x48c0,A_NONE,A_DN,r0,A_SINGLE,0, },/* extl */
	{0,	0,	0,	0,	0,},
	{0x48c0,A_NONE,A_DN,r0,A_SINGLE,0, },/* extwl */
	{0,	0,	0,	0,	0,},
	{0x49c0,A_NONE,A_DN,r0,A_SINGLE,X_IS68020, },/* extbl */
	{0,	0,	0,	0,	0,},
	{0x4a7c,A_NONE,A_NONE,no_operands,A_NONE,0 },/* illegal */
	{0,	0,	0,	0,	0,},
	{0x64ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jcc */
	{0,	0,	0,	0,	0,},
	{0x65ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jcs */
	{0,	0,	0,	0,	0,},
	{0x67ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jeq */
	{0,	0,	0,	0,	0,},
	{0x6cff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jge */
	{0,	0,	0,	0,	0,},
	{0x6eff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jgt */
	{0,	0,	0,	0,	0,},
	{0x62ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jhi */
	{0,	0,	0,	0,	0,},
	{0x6fff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jle */
	{0,	0,	0,	0,	0,},
	{0x63ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jls */
	{0,	0,	0,	0,	0,},
	{0x6dff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jlt */
	{0,	0,	0,	0,	0,},
	{0x6bff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jmi */
	{0,	0,	0,	0,	0,},
	{0x66ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jne */
	{0,	0,	0,	0,	0,},
	{0x6aff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jpl */
	{0,	0,	0,	0,	0,},
	{0x68ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jvc */
	{0,	0,	0,	0,	0,},
	{0x69ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jvs */
	{0,	0,	0,	0,	0,},
	{0x60ff,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,X_ISSDI, },/* jra */
	{0x4ec0,A_NONE,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYPC),ea,A_SINGLE,0, },/* jra */
	{0,	0,	0,	0,	0,},
	{0x4ec0,A_NONE,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),ea,A_SINGLE,0, },/* jmp */
	{0,	0,	0,	0,	0,},
	{0x4eb9,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,(X_ISJSR|X_ISSDI), },/* jbsr */
	{0x4e80,A_NONE,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYPC),ea,A_SINGLE,0, },/* jbsr */
	{0,	0,	0,	0,	0,},
	{0x4eb9,A_NONE,A_CANBE_DISPL,long_ext,A_SINGLE,(X_ISJSR|X_ISSDI), },/* jsr */
	{0x4e80,A_NONE,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYPC),ea,A_SINGLE,0, },/* jsr */
	{0,	0,	0,	0,	0,},
	{0x41c0,(A_ANIND|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_AN,ea_r9,A_SRC,0, },/* lea */
	{0,	0,	0,	0,	0,},
	{0x4e50,A_AN,A_CANBE_IMMW,r0_imm,A_SRC,0, },/* link */
	{0,	0,	0,	0,	0,},
	{0x4808,A_AN,A_CANBE_IMML,r0_imm,A_SRC,X_IS68020, },/* linkl */
	{0,	0,	0,	0,	0,},
	{0x4e50,A_AN,A_CANBE_IMMW,r0_imm,A_SRC,0, },/* linkw */
	{0,	0,	0,	0,	0,},
	{0xe108,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* lslb */
	{0xe128,A_DN,A_DN,r0_r9,A_DEST,0, },/* lslb */
	{0,	0,	0,	0,	0,},
	{0xe188,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* lsll */
	{0xe1a8,A_DN,A_DN,r0_r9,A_DEST,0, },/* lsll */
	{0,	0,	0,	0,	0,},
	{0xe148,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* lslw */
	{0xe168,A_DN,A_DN,r0_r9,A_DEST,0, },/* lslw */
	{0xe3c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* lslw */
	{0,	0,	0,	0,	0,},
	{0xe008,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* lsrb */
	{0xe028,A_DN,A_DN,r0_r9,A_DEST,0, },/* lsrb */
	{0,	0,	0,	0,	0,},
	{0xe088,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* lsrl */
	{0xe0a8,A_DN,A_DN,r0_r9,A_DEST,0, },/* lsrl */
	{0,	0,	0,	0,	0,},
	{0xe048,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* lsrw */
	{0xe068,A_DN,A_DN,r0_r9,A_DEST,0, },/* lsrw */
	{0xe2c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* lsrw */
	{0,	0,	0,	0,	0,},
	{0x1000,A_ANYBNOTAN,(A_ANYDATA|A_DN),ea6_ea,A_DEST,0, },/* movb */
	{0,	0,	0,	0,	0,},
	{0x4e7a,A_SPECIALREG,A_ANYREG,rext12_specialreg,A_DEST,0, },/* movec */
	{0x4e7b,A_ANYREG,A_SPECIALREG,rext12_specialreg,A_SRC,0, },/* movec */
	{0,	0,	0,	0,	0,},
	{0x0188,A_DN,A_ANINDDISP,r0disp_r9,A_DEST,0, },/* movepw */
	{0x0108,A_ANINDDISP,A_DN,r0disp_r9,A_SRC,0 },/* movepw */
	{0,	0,	0,	0,	0,},
	{0x01c8,A_DN,A_ANINDDISP,r0disp_r9,A_DEST,0, },/* movepl */
	{0x0148,A_ANINDDISP,A_DN,r0disp_r9,A_SRC,0 },/* movepl */
	{0,	0,	0,	0,	0,},
	{0x0e00,A_ANYREG,A_ANYDATA,ea_rext,A_DEST,(X_FLAG0|X_IS68020), },/* movesb */
	{0x0e00,A_ANYDATA,A_ANYREG,ea_rext,A_SRC,X_IS68020, },/* movesb */
	{0,	0,	0,	0,	0,},
	{0x0e80,A_ANYREG,A_ANYDATA,ea_rext,A_DEST,(X_FLAG0|X_IS68020), },/* movesl */
	{0x0e80,A_ANYDATA,A_ANYREG,ea_rext,A_SRC,X_IS68020, },/* movesl */
	{0,	0,	0,	0,	0,},
	{0x0e40,A_ANYREG,A_ANYDATA,ea_rext,A_DEST,(X_FLAG0|X_IS68020), },/* movesw */
	{0x0e40,A_ANYDATA,A_ANYREG,ea_rext,A_SRC,X_IS68020, },/* movesw */
	{0,	0,	0,	0,	0,},
	{0x3000,A_ANYW,(A_ANYDATA|A_DN),ea6_ea,A_DEST,0, },/* movw */
	{0x3040,A_ANYW,A_AN,ea_r9,A_SRC,0, },/* movw */
	{0x42c0,A_CCR,(A_ANYDATA|A_DN),ea,A_DEST,0, },/* movw */
	{0x44c0,A_ANYWNOTAN,A_CCR,ea,A_SRC,0, },/* movw */
	{0x40c0,A_SR,(A_ANYDATA|A_DN),ea,A_DEST,0, },/* movw */
	{0x46c0,A_ANYWNOTAN,A_SR,ea,A_SRC,0, },/* movw */
	{0,	0,	0,	0,	0,},
	{0x7000,A_CANBE_IMM0T31,A_DN,r9_imm0,A_DEST,0, },/* movl */
	{0x2000,A_ANY,(A_ANYDATA|A_DN),ea6_ea,A_DEST,0, },/* movl */
	{0x2040,A_ANY,A_AN,ea_r9,A_SRC,0, },/* movl */
	{0x4e60,A_AN,A_SPECIALREG,r0_usp,A_SRC,0, },/* movl */
	{0x4e68,A_SPECIALREG,A_AN,r0_usp,A_DEST,0, },/* movl */
	{0,	0,	0,	0,	0,},
	{0x48c0,A_CANBE_IMMW,(A_ANIND|A_ANINDPD|A_ANINDDISP|A_ANYINDEX|A_ANYABS),ea_imm,A_DEST,0, },/* moveml */
	{0x4cc0,(A_ANIND|A_ANINDPI|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_CANBE_IMMW,ea_imm,A_SRC,0, },/* moveml */
	{0,	0,	0,	0,	0,},
	{0x4880,A_CANBE_IMMW,(A_ANIND|A_ANINDPD|A_ANINDDISP|A_ANYINDEX|A_ANYABS),ea_imm,A_DEST,0, },/* movemw */
	{0x4c80,(A_ANIND|A_ANINDPI|A_ANINDDISP|A_ANYINDEX|A_ANYABS|A_ANYPC),A_CANBE_IMMW,ea_imm,A_SRC,0, },/* movemw */
	{0,	0,	0,	0,	0,},
	{0x7000,A_CANBE_IMMB,A_DN,r9_imm0,A_DEST,0, },/* moveq */
	{0,	0,	0,	0,	0,},
	{0xc1c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* muls */
	{0,	0,	0,	0,	0,},
	{0xc1c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* mulsw */
	{0,	0,	0,	0,	0,},
	{0x4c00,A_ANYNOTAN,A_DN,ea_rext2,A_SRC,(X_IS68020|X_FLAG0), },/* mulsl */
	{0x4c00,A_ANYNOTAN,A_DOUBLEREG,ea_rext2,A_SRC,(X_IS68020|X_FLAG0|X_FLAG1), },/* mulsl */
	{0,	0,	0,	0,	0,},
	{0xc0c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* mulu */
	{0,	0,	0,	0,	0,},
	{0x4c00,A_ANYNOTAN,A_DN,ea_rext2,A_SRC,(X_IS68020), },/* mulul */
	{0x4c00,A_ANYNOTAN,A_DOUBLEREG,ea_rext2,A_SRC,(X_IS68020|X_FLAG1), },/* mulul */
	{0,	0,	0,	0,	0,},
	{0xc0c0,(A_ANYDATA|A_DN|A_ANYPC|A_CANBE_IMMW),A_DN,ea_r9,A_SRC,0, },/* muluw */
	{0,	0,	0,	0,	0,},
	{0x4800,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* nbcd */
	{0,	0,	0,	0,	0,},
	{0x4400,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* negb */
	{0,	0,	0,	0,	0,},
	{0x4440,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* negw */
	{0,	0,	0,	0,	0,},
	{0x4480,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* negl */
	{0,	0,	0,	0,	0,},
	{0x4000,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* negxb */
	{0,	0,	0,	0,	0,},
	{0x4040,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* negxw */
	{0,	0,	0,	0,	0,},
	{0x4080,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* negxl */
	{0,	0,	0,	0,	0,},
	{0x4e71,A_NONE,A_NONE,no_operands,A_NONE,0, },/* nop */
	{0,	0,	0,	0,	0,},
	{0x4600,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* notb */
	{0,	0,	0,	0,	0,},
	{0x4640,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* notw */
	{0,	0,	0,	0,	0,},
	{0x4680,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* notl */
	{0,	0,	0,	0,	0,},
	{0x0000,A_CANBE_IMMB,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* orb */
	{0x8000,A_ANYBNOTAN,A_DN,ea_r9,A_SRC,0, },/* orb */
	{0x8100,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* orb */
	{0,	0,	0,	0,	0,},
	{0x0040,A_CANBE_IMMW,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* orw */
	{0x8040,A_ANYWNOTAN,A_DN,ea_r9,A_SRC,0, },/* orw */
	{0x8140,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* orw */
	{0x003c,A_CANBE_IMMW,A_CCR,word_ext,A_SRC,0, },/* orw */
	{0x007c,A_CANBE_IMMW,A_SR,word_ext,A_SRC },/* orw */
	{0,	0,	0,	0,	0,},
	{0x0080,A_CANBE_IMML,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* orl */
	{0x8080,A_ANYNOTAN,A_DN,ea_r9,A_SRC,0, },/* orl */
	{0x8180,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* orl */
	{0,	0,	0,	0,	0,},
	{0x4840,A_NONE,(A_ANYINDEX|A_ANYABS|A_ANYPC|A_ANIND|A_ANINDDISP),ea,A_SINGLE,0, },/* pea */
	{0,	0,	0,	0,	0,},
	{0x4e70,A_NONE,A_NONE,no_operands,A_NONE,0, },/* reset */
	{0,	0,	0,	0,	0,},
	{0xe118,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* rolb */
	{0xe138,A_DN,A_DN,r0_r9,A_DEST,0, },/* rolb */
	{0,	0,	0,	0,	0,},
	{0xe158,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* rolw */
	{0xe178,A_DN,A_DN,r0_r9,A_DEST,0, },/* rolw */
	{0,	0,	0,	0,	0,},
	{0xe198,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* roll */
	{0xe1b8,A_DN,A_DN,r0_r9,A_DEST,0, },/* roll */
	{0xe7c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* roll */
	{0,	0,	0,	0,	0,},
	{0xe018,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* rorb */
	{0xe038,A_DN,A_DN,r0_r9,A_DEST,0, },/* rorb */
	{0,	0,	0,	0,	0,},
	{0xe058,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* rorw */
	{0xe078,A_DN,A_DN,r0_r9,A_DEST,0, },/* rorw */
	{0,	0,	0,	0,	0,},
	{0xe098,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* rorl */
	{0xe0b8,A_DN,A_DN,r0_r9,A_DEST,0, },/* rorl */
	{0xe6c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* rorl */
	{0,	0,	0,	0,	0,},
	{0xe110,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* roxlb */
	{0xe130,A_DN,A_DN,r0_r9,A_DEST,0, },/* roxlb */
	{0,	0,	0,	0,	0,},
	{0xe150,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* roxlw */
	{0xe170,A_DN,A_DN,r0_r9,A_DEST,0, },/* roxlw */
	{0,	0,	0,	0,	0,},
	{0xe190,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* roxll */
	{0xe1b0,A_DN,A_DN,r0_r9,A_DEST,0, },/* roxll */
	{0xe5c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* roxll */
	{0,	0,	0,	0,	0,},
	{0xe010,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* roxrb */
	{0xe030,A_DN,A_DN,r0_r9,A_DEST,0, },/* roxrb */
	{0,	0,	0,	0,	0,},
	{0xe050,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* roxrw */
	{0xe070,A_DN,A_DN,r0_r9,A_DEST,0, },/* roxrw */
	{0,	0,	0,	0,	0,},
	{0xe090,A_CANBE_IMM1T8,A_DN,r0_imm9,A_DEST,0, },/* roxrl */
	{0xe0b0,A_DN,A_DN,r0_r9,A_DEST,0, },/* roxrl */
	{0xe4c0,A_NONE,A_ANYDATA,ea,A_SINGLE,0, },/* roxrl */
	{0,	0,	0,	0,	0,},
	{0x4e74,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,0, },/* rtd */
	{0,	0,	0,	0,	0,},
	{0x4e73,A_NONE,A_NONE,no_operands,A_NONE,0, },/* rte */
	{0,	0,	0,	0,	0,},
	{0x05c0,A_NONE,A_DN,r0,A_SINGLE,X_IS68020, },/* rtm */
	{0x05c8,A_NONE,A_AN,r0,A_SINGLE,X_IS68020, },/* rtm */
	{0,	0,	0,	0,	0,},
	{0x4e77,A_NONE,A_NONE,no_operands,A_NONE,0, },/* rtr */
	{0,	0,	0,	0,	0,},
	{0x4e75,A_NONE,A_NONE,no_operands,A_NONE,0, },/* rts */
	{0,	0,	0,	0,	0,},
	{0x8100,A_DN,A_DN,r0_r9,A_SRC,0, },/* sbcd */
	{0x8108,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0 },/* sbcd */
	{0,	0,	0,	0,	0,},
	{0x54c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* scc */
	{0,	0,	0,	0,	0,},
	{0x55c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* scs */
	{0,	0,	0,	0,	0,},
	{0x57c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* seq */
	{0,	0,	0,	0,	0,},
	{0x51c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* sf */
	{0,	0,	0,	0,	0,},
	{0x5cc0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* sge */
	{0,	0,	0,	0,	0,},
	{0x5ec0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* sgt */
	{0,	0,	0,	0,	0,},
	{0x52c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* shi */
	{0,	0,	0,	0,	0,},
	{0x5fc0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* sle */
	{0,	0,	0,	0,	0,},
	{0x53c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* sls */
	{0,	0,	0,	0,	0,},
	{0x5dc0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* slt */
	{0,	0,	0,	0,	0,},
	{0x5bc0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* smi */
	{0,	0,	0,	0,	0,},
	{0x56c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* sne */
	{0,	0,	0,	0,	0,},
	{0x5ac0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* spl */
	{0,	0,	0,	0,	0,},
	{0x50c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* st */
	{0,	0,	0,	0,	0,},
	{0x58c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* svc */
	{0,	0,	0,	0,	0,},
	{0x59c0,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* svs */
	{0,	0,	0,	0,	0,},
	{0x4e72,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,0, },/* stop */
	{0,	0,	0,	0,	0,},
	{0x5100,A_CANBE_IMM1T8,(A_ANYDATA|A_DN),ea_imm9,A_DEST,0, },/* subb */
	{0x0400,A_CANBE_IMMB,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* subb */
	{0x9000,A_ANYWNOTAN,A_DN,ea_r9,A_SRC,0, },/* subb */
	{0x9100,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* subb */
	{0,	0,	0,	0,	0,},
	{0x5180,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* subl */
	{0x0480,A_CANBE_IMML,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* subl */
	{0x9080,A_ANY,A_DN,ea_r9,A_SRC,0, },/* subl */
	{0x9180,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* subl */
	{0x91c0,A_ANY,A_AN,ea_r9,A_SRC,0, },/* subl */
	{0,	0,	0,	0,	0,},
	{0x5100,A_CANBE_IMM1T8,(A_ANYDATA|A_DN),ea_imm9,A_DEST,0, },/* subqb */
	{0,	0,	0,	0,	0,},
	{0x5180,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* subql */
	{0,	0,	0,	0,	0,},
	{0x5140,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* subqw */
	{0,	0,	0,	0,	0,},
	{0x5140,A_CANBE_IMM1T8,A_ANYALTERABLE,ea_imm9,A_DEST,0, },/* subw */
	{0x0440,A_CANBE_IMML,(A_ANYDATA|A_DN),ea_imm,A_DEST,0, },/* subw */
	{0x9040,A_ANYW,A_DN,ea_r9,A_SRC,0, },/* subw */
	{0x9140,A_DN,A_ANYDATA,ea_r9,A_DEST,0, },/* subw */
	{0x90c0,A_ANYW,A_AN,ea_r9,A_SRC,0, },/* subw */
	{0,	0,	0,	0,	0,},
	{0x9100,A_DN,A_DN,r0_r9,A_SRC,0, },/* subxb */
	{0x9108,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0, },/* subxb */
	{0,	0,	0,	0,	0,},
	{0x9180,A_DN,A_DN,r0_r9,A_SRC,0, },/* subxl */
	{0x9188,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0, },/* subxl */
	{0,	0,	0,	0,	0,},
	{0x9140,A_DN,A_DN,r0_r9,A_SRC,0, },/* subxw */
	{0x9148,A_ANINDPD,A_ANINDPD,r0_r9,A_SRC,0, },/* subxw */
	{0,	0,	0,	0,	0,},
	{0x4840,A_NONE,A_DN,r0,A_SINGLE,0, },/* swap */
	{0,	0,	0,	0,	0,},
	{0x4ac0,A_NONE,(A_DN|A_ANYIND|A_ANYINDEX|A_ANYABS),ea,A_SINGLE,0, },/* tasb */
	{0,	0,	0,	0,	0,},
	{0x4e40,A_NONE,A_CANBE_IMM0T31,spimm0,A_SINGLE,0x10, },/* trap */
	{0,	0,	0,	0,	0,},
	{0x54fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapcc */
	{0,	0,	0,	0,	0,},
	{0x54fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapccl */
	{0,	0,	0,	0,	0,},
	{0x54fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapccw */
	{0,	0,	0,	0,	0,},
	{0x55fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapcs */
	{0,	0,	0,	0,	0,},
	{0x55fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapcsl */
	{0,	0,	0,	0,	0,},
	{0x55fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapcsw */
	{0,	0,	0,	0,	0,},
	{0x57fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapeq */
	{0,	0,	0,	0,	0,},
	{0x57fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapeql */
	{0,	0,	0,	0,	0,},
	{0x57fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapeqw */
	{0,	0,	0,	0,	0,},
	{0x51fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapf */
	{0,	0,	0,	0,	0,},
	{0x51fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapfl */
	{0,	0,	0,	0,	0,},
	{0x51fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapfw */
	{0,	0,	0,	0,	0,},
	{0x5cfc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapge */
	{0,	0,	0,	0,	0,},
	{0x5cfb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapgel */
	{0,	0,	0,	0,	0,},
	{0x5cfa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapgew */
	{0,	0,	0,	0,	0,},
	{0x5efc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapgt */
	{0,	0,	0,	0,	0,},
	{0x5efb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapgtl */
	{0,	0,	0,	0,	0,},
	{0x5efa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapgtw */
	{0,	0,	0,	0,	0,},
	{0x52fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* traphi */
	{0,	0,	0,	0,	0,},
	{0x52fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* traphil */
	{0,	0,	0,	0,	0,},
	{0x52fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* traphiw */
	{0,	0,	0,	0,	0,},
	{0x5ffc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* traple */
	{0,	0,	0,	0,	0,},
	{0x5ffb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* traplel */
	{0,	0,	0,	0,	0,},
	{0x5ffa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* traplew */
	{0,	0,	0,	0,	0,},
	{0x53fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapls */
	{0,	0,	0,	0,	0,},
	{0x53fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* traplsl */
	{0,	0,	0,	0,	0,},
	{0x53fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* traplsw */
	{0,	0,	0,	0,	0,},
	{0x5dfc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* traplt */
	{0,	0,	0,	0,	0,},
	{0x5dfb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapltl */
	{0,	0,	0,	0,	0,},
	{0x5dfa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapltw */
	{0,	0,	0,	0,	0,},
	{0x5bfc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapmi */
	{0,	0,	0,	0,	0,},
	{0x5bfb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapmil */
	{0,	0,	0,	0,	0,},
	{0x5bfa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapmiw */
	{0,	0,	0,	0,	0,},
	{0x56fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapne */
	{0,	0,	0,	0,	0,},
	{0x56fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapnel */
	{0,	0,	0,	0,	0,},
	{0x56fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapnew */
	{0,	0,	0,	0,	0,},
	{0x5afc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trappl */
	{0,	0,	0,	0,	0,},
	{0x5afb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trappll */
	{0,	0,	0,	0,	0,},
	{0x5afa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapplw */
	{0,	0,	0,	0,	0,},
	{0x50fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapt */
	{0,	0,	0,	0,	0,},
	{0x50fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* traptl */
	{0,	0,	0,	0,	0,},
	{0x50fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* traptw */
	{0,	0,	0,	0,	0,},
	{0x58fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapvc */
	{0,	0,	0,	0,	0,},
	{0x58fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapvcl */
	{0,	0,	0,	0,	0,},
	{0x58fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapvcw */
	{0,	0,	0,	0,	0,},
	{0x59fc,A_NONE,A_NONE,no_operands,A_NONE,X_IS68020, },/* trapvs */
	{0,	0,	0,	0,	0,},
	{0x59fb,A_NONE,A_CANBE_IMML,long_ext,A_SINGLE,X_IS68020, },/* trapvsl */
	{0,	0,	0,	0,	0,},
	{0x59fa,A_NONE,A_CANBE_IMMW,word_ext,A_SINGLE,X_IS68020, },/* trapvsw */
	{0,	0,	0,	0,	0,},
	{0x4e76,A_NONE,A_NONE,no_operands,A_NONE,0, },/* trapv */
	{0,	0,	0,	0,	0,},
	{0x4a00,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* tstb */
	{0,	0,	0,	0,	0,},
	{0x4a80,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* tstl */
	{0,	0,	0,	0,	0,},
	{0x4a40,A_NONE,(A_ANYDATA|A_DN),ea,A_SINGLE,0, },/* tstw */
	{0,	0,	0,	0,	0,},
	{0x4e58,A_NONE,A_AN,r0,A_SINGLE,0, },/* unlk */
	{0,	0,	0,	0,	0,},
};
