#include "mical.h"
#include "a.out.h"

End()
  {	register int i;
	register struct csect *p;

/* Pass 2 */
	if (Pass > 1) {				/* On the second pass, */
		Fix_Rel();			/* patch up object file */
		return; }

/* Pass 1 */
	fseek(stdin,0L,0);

	sdi_resolve();	/* resolve span dependent instructions */

	for (p = &Csects[0], i=0; i < Csect_load; i++, p++)
		p->len_cs += sdi_inc(p, p->len_cs);
#ifdef	notdef
	tsize = (Text_csect->len_cs + 3) & ~3;	/* make long aligned */
	dsize = (Data_csect->len_cs + 3) & ~3;
	bsize = (Bss_csect->len_cs + 3) & ~3;
#else
	tsize = (Text_csect->len_cs + 1) & ~1;	/* make short aligned */
	dsize = (Data_csect->len_cs + 1) & ~1;
	bsize = (Bss_csect->len_cs + 1) & ~1;
#endif

	Sym_Fix();	/* relocate and globalize */
	sdi_free();	/* release sdi blocks */
	Rel_Header();	/* Initialize output stuff */
	Start_Pass();	/* Init per-pass variables */
	return;
}

/* Initialize per-pass variables */

Start_Pass()
 {	register int i;

	Last_symbol = Dot_bkt;	/* last defined symbol at start of pass */
	Line_no = 0;
	Errors = 0;
	Pass++;
	if (Pass != 2) for (i=0; i<Csect_load; i++) Csects[i].dot_cs = 0;
	else {
	  Text_csect->dot_cs = 0;
	  Data_csect->dot_cs = tsize;
	  Bss_csect->dot_cs = tsize + dsize;
	}
	New_Csect(&Csects[0]);	/* start in text segment */
}

/*
 * .even handler
 */
Even()
{
	if (Dot&1) {
		Dot++;
		Code[0] = 0;
		Put_Text(Code,1);
	}
}

ByteWord(Which)
  register int Which;
  {	register int i;
	register struct oper *p;
	char temp;

	for (i=0, p=operands; i < numops; i++, p++) {
	  if ((p->type_o != t_normal)&&(p->type_o != t_abss)) {
	    p->sym_o = 0;
	    p->type_o = t_normal;
	    p->value_o = 0;
	    Prog_Error(E_OPERAND);
	  } else if (p->sym_o) Put_Rel(p,Which,Dot+BC);

	  switch (Which) {
	    case L:	WCode[0] = p->value_o >> 16;
			WCode[1] = p->value_o;
			Put_Words(WCode,4);
			BC += 4;
			break;

	    case W:	WCode[0] = p->value_o;
	  		Put_Words(WCode,2);
			BC += 2;
			break;

	    case B:	temp = p->value_o;
			Put_Text(&temp,1);
			BC++;
			break;
	  }
	}
}

/* handle .ascii and .asciz pseudo ops -- zero<>0 indicates that
 * user wants zero byte at end of string.
 */
Ascii(zero)
  {	register char *p;

	if (numops!=1 || operands[0].type_o!=t_string)
	  { Prog_Error(E_OPERAND); return; }
	p = (char *)operands[0].value_o;
	nolist = 1;
	listascii(p, zero);
	while (*p) { Put_Text(p++,1); BC++; }
	if (zero) { Put_Text(p,1); BC++; }
}	

#ifndef	SYS_3_AOUTH

struct csect Csects[CSECT_MAX] = {
  { ".text", 0,0,N_TEXT,R_ISPC|R_PURE, },	/* text csect */
  { ".data", 0,0,N_DATA,0, },			/* data csect */
#ifdef	franz
  { ".data1",0,0,N_DATA,0, },			/* data csect */
  { ".data2",0,0,N_DATA,0, },			/* data csect */
#endif
  { ".bss",  0,0,N_BSS, 0 },			/* uninitialized csect */
};

#else

struct csect Csects[CSECT_MAX] = {
  ".text",0,0,0,R_ISPC|R_PURE,	/* text csect */
  ".data",0,0,0,0,		/* data csect */
  ".bss",0,0,0,0		/* uninitialized csect */
};

#endif

int Csect_load = 4;			/* Next available csect in Csects[] */
struct csect *Cur_csect = &(Csects[0]);	/* ptr to current csect */
struct csect *Text_csect = &(Csects[0]);
struct csect *Data_csect = &(Csects[1]);
struct csect *Bss_csect = &(Csects[2]);

New_Csect(csp)
  register struct csect *csp;
  {	register struct sym_bkt *sbp;	/* for defining new symbol */
	extern struct csect *Cur_csect;	/* ptr to current csect */
	extern struct sym_bkt	*Last_symbol;	/* used for local symbols */
	extern struct sym_bkt	*Dot_bkt;	/* sym_bkt for location counter */
	
	Cur_csect = csp;
	Dot = csp->dot_cs;
	Dot_bkt->csect_s = Cur_csect;	/* update dot's csect */
	Dot_bkt->value_s = csp->dot_cs;

	sbp = Lookup(csp->name_cs);
	sbp->attr_s |= S_DEC | S_DEF | S_LOCAL;
	sbp->csect_s = Cur_csect;
	sbp->value_s = 0;
}

Globl()
  {	register int i;
	register struct sym_bkt *sbp;

	if (Pass == 1) for (i=0; i<numops; i++) {
	  sbp = operands[i].sym_o;
	  if (sbp == NULL) Prog_Error(E_SYMBOL);
	  else {
	    sbp->csect_s = 0;		/* don't know which */
	    sbp->attr_s |= S_DEC | S_EXT;	/* declared and external */
	  }
	}
	return;
}

Comm()
  {	register struct sym_bkt *sbp;

	if (Pass == 1) {
	  sbp = operands[0].sym_o;
	  if (sbp == NULL) Prog_Error(E_OPERAND);
	  else {
	    sbp->csect_s = 0;	/* make it undefined */
	    sbp->attr_s |= S_DEC | S_EXT | S_COMM;
	    sbp->value_s = operands[1].value_o;
	  }
	}
	return;
}
