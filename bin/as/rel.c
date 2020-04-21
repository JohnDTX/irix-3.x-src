#include "mical.h"
#include "a.out.h"

/*  Handle output file processing for a.out files */

FILE *tout;		/* text portion of output file */
FILE *dout;		/* data portion of output file */
FILE *rtout;		/* text relocation commands */
FILE *rdout;		/* data relocation commands */

long rtsize;		/* size of text relocation area */
long rdsize;		/* size of data relocation area */

char *rtname;		/* name of file for text relocation commands */
char *rdname;		/* name of file for data relocation commands */

struct exec filhdr;	/* header for b.out files, contains sizes */

/* Initialize files for output and write out the header */
Rel_Header()
{ long Sym_Write();
  char *mktemp();

#ifdef	franz
  if ((tout = fopen(Rel_name, "w")) == NULL ||
	(dout = fopen(Rel_name, "a")) == NULL ||
	(d1out = fopen(Rel_name, "a")) == NULL ||
	(d2out = fopen(Rel_name, "a")) == NULL)
#else
  if ((tout = fopen(Rel_name, "w")) == NULL ||
	(dout = fopen(Rel_name, "r+")) == NULL)
#endif
     Sys_Error("open on output file %s failed", Rel_name);

	rtname = mktemp("/tmp/astXXXXXX");
	rdname = mktemp("/tmp/asdXXXXXX");
	if ((rtout = fopen(rtname, "w")) == NULL)
		Sys_Error("open on output file %s failed", rtname);
	if ((rdout = fopen(rdname, "w")) == NULL)
		Sys_Error("open on output file %s failed", rdname);

  filhdr.a_magic = OMAGIC;
  filhdr.a_text = tsize;
#ifdef	franz
  filhdr.a_data = dsize+d1size+d2size;
#else
  filhdr.a_data = dsize;
#endif
  filhdr.a_bss = bsize;
  filhdr.a_entry = 0;
  filhdr.a_trsize = rtsize;
  filhdr.a_drsize = rdsize;

  fseek(tout, 0L, 0);
  fwrite(&filhdr, sizeof(filhdr), 1, tout);

  fseek(tout, (long)(N_TXTOFF(filhdr)), 0);   /* seek to start of text */
  fseek(dout, (long)(N_TXTOFF(filhdr)+tsize), 0);	
#ifdef	franz
  fseek(d1out, (long)(N_TXTOFF(filhdr)+tsize+dsize), 0);
  fseek(d2out, (long)(N_TXTOFF(filhdr)+tsize+dsize+d1size), 0);
#endif
  rtsize = 0;
  rdsize = 0;

} /* end Rel_Header * /

/*
 * Fix_Rel -	Fix up the object file
 *	1)	append the relocation segments
 *	2)	fix up the rtsize and rdsize in the header
 *	3)	delete the temporary file for relocation commands
 */
Fix_Rel()
{
	long i;
	register FILE *fin, *fout;

	fclose(rtout);
	fclose(rdout);
	fout = tout;

    /* first write text relocation commands */
	if ((fin = fopen(rtname, "r")) == NULL)
		Sys_Error("cannot reopen relocation file %s", rtname);
	fseek(fout, (long)(N_TXTOFF(filhdr)+filhdr.a_text+filhdr.a_data), 0);
	redosyms();
	filhdr.a_trsize = dorseg(fin,fout,rtsize);

    /* seek to start of data segment relocation commands */
	fclose(fin);
	if ((fin = fopen(rdname, "r")) == NULL)
		Sys_Error("cannot reopen relocation file %s", rdname);
	filhdr.a_drsize = dorseg(fin,fout,rdsize);
	fclose(fin);

    /* Now put the full symbol table out there */
	filhdr.a_syms = Sym_Write(fout);

    /* Now for the string table */
	i = Str_Write(fout);

    /* After the table is written, rewrite the length */
	fseek(fout,N_STROFF(filhdr),0);
	fwrite(&i,sizeof(long),1,fout);

    /* now re-write header */
	fseek(fout, 0, 0);
	fwrite(&filhdr, sizeof(filhdr), 1, fout);
	fclose(fin);
	unlink(rtname);
	unlink(rdname);
}

/* rel_val -	Puts value of operand into next bytes of Code
 * updating Code_length. Put_Rel is called to handle possible relocation.
 * If size=L a longword is stored, otherwise a word is stored 
 */
rel_val(opnd,size)
register struct oper *opnd;
{	register int i;
	register struct sym_bkt *sp;
	long val;
	char *CCode;			/* char version of this */

	i = Code_length>>1;	/* get index into WCode */
	if (sp = opnd->sym_o)
		Put_Rel(opnd, size, Dot + Code_length);
	val = opnd->value_o;
	switch(size)
	{
	case L:
		WCode[i++] = val>>16;
		Code_length += 2;
	case W:
		WCode[i] = val;
		Code_length += 2;
		break;
	case B:
		CCode = (char *)WCode;
		CCode[Code_length++] = val;
	}
}

/* Version of Put_Text which puts whole words, thus enforcing the mapping
 * of bytes to words.
 */

#ifndef	XAS

Put_Words(code,nbytes)
  char *code;
  {	if (nbytes & 1) Sys_Error("Put_Words given odd nbytes=%d",nbytes);
	Put_Text(code,nbytes);
}

#else

Put_Words(code,nbytes)
register char *code;
{	register char *cc, ch;
	register int i;
	char tcode[100];

	cc = tcode;
	for (i=0; i<nbytes; i++) tcode[i] = code[i];
	i = nbytes>>1;
	if (nbytes & 1) Sys_Error("Put_Words given odd nbytes=%d\n",nbytes);
	while (i--) { ch = *cc; *cc = cc[1]; *++cc = ch; cc++; }
	Put_Text(tcode,nbytes);
}
#endif

/*
 * Put_Text
 *	- Write out text to proper portion of file
 */
Put_Text(code,length)
	register char *code;
	int length;
{	
	if (Pass != 2)
		return;
	if (Cur_csect == Text_csect) {
		if (nolist == 0)
			listinst(length, code);
		if (fwrite(code, length, 1, tout) != 1)
			Sys_Error("Put_Text:  fwrite failed\n", (char *) NULL);
	} else
	if (Cur_csect == Data_csect) {
		if (fwrite(code, length, 1, dout) != 1)
			Sys_Error("Put_Text:  fwrite failed\n", (char *) NULL);
	} else
		return;	/* ignore if bss segment */
}

/*
 * set up relocation word for operand:
 *  opnd	pointer to operand structure
 *  size	0 = byte, 1 = word, 2 = long/address
 *  offset	offset into WCode & WReloc array
 */
Put_Rel(opnd, size, offset)
	struct oper *opnd;
	int size;
	long offset;
{
	if (opnd->sym_o == 0)
		return;	/* no relocation */
	if (Cur_csect == Text_csect)
		rtsize += rel_cmd(opnd, size, offset, rtout);
	else
	if (Cur_csect == Data_csect)
		rdsize += rel_cmd(opnd, size, offset - tsize, rdout);
	else
		return;	/* just ignore if bss segment */
}

/*
 * This structure is used to save the relocation info and the symbol table
 * pointer (so we can find sp->final later on)
 */
struct	xrelocation_info {
	struct	relocation_info relo;
	struct	sym_bkt *sp;
	short	undef;
} relo;

/* rel_cmd -	Generate a relocation command and output */
rel_cmd(opnd, size, offset, file)
	struct oper *opnd;
	int size;
	long offset;
	FILE *file;
{
	int csid;			/* reloc csect identifier */
	register struct csect *csp;	/* pointer to csect of sym */
	register struct sym_bkt *sp;	/* pointer to symbol */

	sp = opnd->sym_o;
	csp = sp->csect_s;
	if (Pass == 2) {
		relo.relo.r_extern = 0;
		relo.sp = sp;
		relo.undef = 0;
		if (!(sp->attr_s & S_DEF) && (sp->attr_s & S_EXT)) {
			relo.relo.r_extern = 1;
			relo.undef = 1;
		} else
		if (csp == Text_csect)
			relo.relo.r_symbolnum = N_TEXT;
		else
		if (csp == Data_csect)
			relo.relo.r_symbolnum = N_DATA;
		else
		if (csp == Bss_csect)
			relo.relo.r_symbolnum = N_BSS;
		else
			Prog_Error(E_RELOCATE);
		if (relo.relo.r_symbolnum == -1)
			Prog_Error(E_RELOCATE);
		relo.relo.r_address = offset;
		relo.relo.r_pcrel = 0;
		relo.relo.r_length = size;
		if (fwrite(&relo, sizeof relo, 1, file) != 1)
			Sys_Error("rel_cmd:  fwrite failed\n", (char *) NULL);
	}
	return(sizeof relo);
}

/*
 * dorseg:
 *	- copy from the relocation file to the a.out file
 *	- add in local symbol ordinal info, now that its available
 *	- this is garbage
 */
dorseg(fin, fout, oldsize)
	register FILE *fin, *fout;
	register int oldsize;
{
	struct xrelocation_info old;
	struct relocation_info new;
	register long newsize;

	newsize = 0;
	while (oldsize > 0) {
		oldsize -= sizeof(old);
		newsize += sizeof(new);
		fread(&old, sizeof(old), 1, fin);
		new = old.relo;
		if (old.undef) {
			new.r_symbolnum = old.sp->final;
		}
		fwrite(&new, sizeof(new), 1, fout);
	}
	if (oldsize != 0)
		Sys_Error("dorseg: botched relocation file\n", (char *) NULL);
	return newsize;
}
