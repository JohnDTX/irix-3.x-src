#include "mical.h"
#include "inst.h"

char Title[STR_MAX];
char O_outfile = 0;		/* 1 if .rel file name is specified by uder */
int Pass = 0;			/* which pass we're on */
int listf = 0;			/* List flag */
int quickf = 0;			/* quick/short instruction flag */
#ifdef	m68010
int f68010 = 1;			/* 68010 processor flag */
#else
int f68010 = 0;			/* same flag, but off */
#endif
char Rel_name[STR_MAX];		/* Name of .rel file */
FILE *Rel_file;			/* and ptr to it */
char List_name[STR_MAX];	/* Name of .lst file */
extern FILE *List_file;		/* and ptr to it */
struct sym_bkt *Dot_bkt ;	/* Ptr to location counter's symbol bucket */
long tsize = 0;			/* sizes of three main csects */
long dsize = 0;
long bsize = 0;
struct ins_bkt *ins_hash_tab[HASH_MAX];

/* List of 68000 op codes */
struct ins_init { char *opstr; short opnum; } op_codes[] = {
	"abcd", i_abcd,
	"addb",	i_addb,
	"addw",	i_addw,
	"addl",	i_addl,
	"addqb",	i_addqb,
	"addqw",	i_addqw,
	"addql",	i_addql,
	"addxb",	i_addxb,
	"addxw",	i_addxw,
	"addxl",	i_addxl,
	"andb",	i_andb,
	"andw",	i_andw,
	"andl",	i_andl,
	"aslb",	i_aslb,
	"aslw",	i_aslw,
	"asll",	i_asll,
	"asrb",	i_asrb,
	"asrw",	i_asrw,
	"asrl",	i_asrl,
	"bcc",	i_bcc,
	"bccs",	i_bccs,
	"bchg",	i_bchg,
	"bclr",	i_bclr,
	"bcs",	i_bcs,
	"bcss",	i_bcss,
	"beq",	i_beq,
	"beqs",	i_beqs,
	"bge",	i_bge,
	"bges",	i_bges,
	"bgt",	i_bgt,
	"bgts",	i_bgts,
	"bhi",	i_bhi,
	"bhis",	i_bhis,
	"ble",	i_ble,
	"bles",	i_bles,
	"bls",	i_bls,
	"blss",	i_blss,
	"blt",	i_blt,
	"blts",	i_blts,
	"bmi",	i_bmi,
	"bmis",	i_bmis,
	"bne",	i_bne,
	"bnes",	i_bnes,
	"bpl",	i_bpl,
	"bpls",	i_bpls,
	"bra",	i_bra,
	"bras",	i_bras,
	"bset",	i_bset,
	"bsr",	i_bsr,
	"bsrs",	i_bsrs,
	"btst",	i_btst,
	"bvc",	i_bvc,
	"bvcs",	i_bvcs,
	"bvs",	i_bvs,
	"bvss",	i_bvss,
	"chk",	i_chk,
	"clrb",	i_clrb,
	"clrw",	i_clrw,
	"clrl",	i_clrl,
	"cmpb",	i_cmpb,
	"cmpw",	i_cmpw,
	"cmpl",	i_cmpl,
	"cmpmb",	i_cmpmb,
	"cmpmw",	i_cmpmw,
	"cmpml",	i_cmpml,
	"dbcc",	i_dbcc,
	"dbcs",	i_dbcs,
	"dbeq",	i_dbeq,
	"dbf",	i_dbf,
	"dbra",	i_dbra,
	"dbge",	i_dbge,
	"dbgt",	i_dbgt,
	"dbhi",	i_dbhi,
	"dble",	i_dble,
	"dbls",	i_dbls,
	"dblt",	i_dblt,
	"dbmi",	i_dbmi,
	"dbne",	i_dbne,
	"dbpl",	i_dbpl,
	"dbt",	i_dbt,
	"dbvc",	i_dbvc,
	"dbvs",	i_dbvs,
	"divs",	i_divs,
	"divu",	i_divu,
	"eorb",	i_eorb,
	"eorw",	i_eorw,
	"eorl",	i_eorl,
	"exg",	i_exg,
	"extw",	i_extw,
	"extl",	i_extl,
	"illegal",	i_illegal,
	"jbsr", i_jbsr,
	"jcc",	i_jcc,
	"jcs",	i_jcs,
	"jeq",	i_jeq,
	"jge",	i_jge,
	"jgt",	i_jgt,
	"jhi",	i_jhi,
	"jle",	i_jle,
	"jls",	i_jls,
	"jlt",	i_jlt,
	"jmi",	i_jmi,
	"jmp",	i_jmp,
	"jne",	i_jne,
	"jpl",	i_jpl,
	"jra",	i_jra,
	"jsr",	i_jsr,
	"jvc",	i_jvc,
	"jvs",	i_jvs,
	"lea",	i_lea,
	"link",	i_link,
	"lslb",	i_lslb,
	"lslw",	i_lslw,
	"lsll",	i_lsll,
	"lsrb",	i_lsrb,
	"lsrw",	i_lsrw,
	"lsrl",	i_lsrl,
	"movb",	i_movb,
	"movw",	i_movw,
	"movl",	i_movl,
	"movec",	i_movec,
	"movemw",	i_movemw,
	"moveml",	i_moveml,
	"movepw",	i_movepw,
	"movepl",	i_movepl,
	"moveq",	i_moveq,
	"movesb",	i_movesb,
	"movesw",	i_movesw,
	"movesl",	i_movesl,
	"muls",	i_muls,
	"mulu",	i_mulu,
	"nbcd",	i_nbcd,
	"negb",	i_negb,
	"negw",	i_negw,
	"negl",	i_negl,
	"negxb",	i_negxb,
	"negxw",	i_negxw,
	"negxl",	i_negxl,
	"nop",	i_nop,
	"notb",	i_notb,
	"notw",	i_notw,
	"notl",	i_notl,
	"orb",	i_orb,
	"orw",	i_orw,
	"orl",	i_orl,
	"pea",	i_pea,
	"reset",	i_reset,
	"rolb",	i_rolb,
	"rolw",	i_rolw,
	"roll",	i_roll,
	"rorb",	i_rorb,
	"rorw",	i_rorw,
	"rorl",	i_rorl,
	"roxlb",	i_roxlb,
	"roxlw",	i_roxlw,
	"roxll",	i_roxll,
	"roxrb",	i_roxrb,
	"roxrw",	i_roxrw,
	"roxrl",	i_roxrl,
	"rtd",	i_rtd,
	"rte",	i_rte,
	"rtr",	i_rtr,
	"rts",	i_rts,
	"sbcd",	i_sbcd,
	"scc",	i_scc,
	"scs",	i_scs,
	"seq",	i_seq,
	"sf",	i_sf,
	"sge",	i_sge,
	"sgt",	i_sgt,
	"shi",	i_shi,
	"sle",	i_sle,
	"sls",	i_sls,
	"slt",	i_slt,
	"smi",	i_smi,
	"sne",	i_sne,
	"spl",	i_spl,
	"st",	i_st,
	"stop",	i_stop,
	"subb",	i_subb,
	"subw",	i_subw,
	"subl",	i_subl,
	"subqb",	i_subqb,
	"subqw",	i_subqw,
	"subql",	i_subql,
	"subxb",	i_subxb,
	"subxw",	i_subxw,
	"subxl",	i_subxl,
	"svc",	i_svc,
	"svs",	i_svs,
	"swap",	i_swap,
	"tas",	i_tas,
	"trap",	i_trap,
	"trapv",	i_trapv,
	"tstb",	i_tstb,
	"tstw",	i_tstw,
	"tstl",	i_tstl,
	"unlk",	i_unlk,
	".long", i_long,
	".word", i_word,
	".byte", i_byte,
	".text", i_text,
	".data", i_data,
	".bss", i_bss,
	".globl", i_globl,
	".comm", i_comm,
	".even", i_even,
	".asciz", i_asciz,
	".ascii", i_ascii,
/** GB stabs */
        ".stabs", i_stabs,  /* YIP */
        ".stabd", i_stabd,  /* YIP */
        ".stabn", i_stabn,  /* YIP */
/**/
	".space", i_space,
	0 };

char *Source_name = NULL;
char **File_names;

Init(argc,argv)
char *argv[];
{	register int i,j;
	char *strncpy();
	char *cp1, *cp2, *end, *rindex();
	char **last;

	if ((last = File_names = (char **)calloc(argc,sizeof(char *))) == NULL)
	  Sys_Error("%s: File name storage execeeded\n","Init");
	argv++;
	while (--argc) {
	  if (argv[0][0] == '-') switch (argv[0][1]) {
	    case 'l':	listf++;
			break;
	    case 'o':	O_outfile++;
			Concat(Rel_name,argv[1],"");
			argv++;			
			argc--;
			break;
	    case 'q':	quickf++;
			break;
#ifdef	m68000
	    case 'v':	f68010++;
			break;
#endif
		/* GB (SGI) 2/23/86 - ignore -i switch for IRIS systems. 
							  print error for illegal -j switch.
		*/
		case 'i':	break;
		case 'j':	fprintf(stderr,
				"/bin/as on this machine is 68010 only: '-j' disallowed.\n");
	    default:	fprintf(stderr,
			    "usage:as [-l] [-q] [-v] [-o ofile] [ifile]\n");
			exit(-1);
	  } else if (Source_name == NULL) {
	    Source_name = argv[0];
	    *last++ = *argv;
	  } else {
	    *last++ = *argv;
	  }
	  argv++;
	}
	*last = 0;


/* Check to see if we can open output file */
	if(!O_outfile)
		strcpy(Rel_name, "a.out");	/* put output in a.out */
	if ((Rel_file = fopen(Rel_name,"w")) == NULL) {
		fprintf(stderr,"Can't create output file: %s\n",Rel_name);
		exit(1);
	}
	if (listf) {
	      strncpy(List_name, Rel_name, STR_MAX);
	      if (end = rindex(List_name, '.'))
		Concat(end, ".lst", "");
	      if ((List_file = fopen(List_name,"w")) == NULL) {
	        fprintf(stderr,"Can't create listing file: %s\n",List_name);
		listf = 0;
	      }
	}
	fclose(Rel_file);	/* Rel_Header will open properly */

/* Initialize symbols */
	Sym_Init();
	Dot_bkt = Lookup(".");		/* make bucket for location counter */
	Dot_bkt->csect_s = Cur_csect;
	Dot_bkt->attr_s = S_DEC | S_DEF | S_LABEL; 	/* "S_LABEL" so it cant be redefined as a label */
	init_regs();			/* define register names */
	d_ins();			/* set up opcode hash table */
	Perm();
	Start_Pass();
}

d_ins()
{	register struct ins_init *p;
	register struct ins_bkt *insp;
	register int save;

	for (p = op_codes; p->opstr != 0; p++) {
          if ((insp=(struct ins_bkt *)calloc(1,sizeof(struct ins_bkt))) == NULL)
            Sys_Error("%s: Symbol bucket storage exceeded\n","d_ins");
	  insp->text_i = p->opstr;
	  insp->code_i = p->opnum;
	  insp->next_i = ins_hash_tab[save = Hash(insp->text_i)];
	  ins_hash_tab[save] = insp;
	}
}

struct def { char *rname; int rnum; } defregs[] = {
  "d0", 0, "d1", 1, "d2", 2, "d3", 3,
  "d4", 4, "d5", 5, "d6", 6, "d7", 7,
  "a0", 8, "a1", 9, "a2", 10, "a3", 11,
  "a4", 12, "a5", 13, "a6", 14, "a7", 15, "sp", 15,
  "pc", 16, "cc", 17, "sr", 18, "usp", 19,
  "vbr", 20, "sfc", 21, "dfc", 22,
  0, 0
};

init_regs()
  {	register struct sym_bkt *sbp;
	register struct def *p = defregs;
	struct sym_bkt *Lookup();

	while (p->rname) {
	  sbp = Lookup(p->rname);	/* Make a sym_bkt for it */
	  sbp->value_s = p->rnum;	/* Load the sym_bkt */
	  sbp->csect_s = 0;
	  sbp->attr_s = S_DEC | S_DEF | S_REG;
	  p++;
	}
}

Concat(s1,s2,s3)
  register char *s1,*s2,*s3;
  {	while (*s1++ = *s2++);
	s1--;
	while (*s1++ = *s3++);
}


/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
*/
char *
rindex(sp, c)
register char *sp, c;
{
	register char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}


/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

char *
strncpy(s1, s2, n)
register char *s1, *s2;
{
	register i;
	register char *os1;

	os1 = s1;
	for (i = 0; i < n; i++)
		if ((*s1++ = *s2++) == '\0') {
			while (++i < n)
				*s1++ = '\0';
			return(os1);
		}
	return(os1);
}

strcmp(s1, s2)
register char *s1, *s2;
{
	while (*s1 == *s2++)
		if (*s1++=='\0')
			return(0);
	return(1);
}
