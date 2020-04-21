# include "macdefs.h"
# include "mac2defs.h"
# include "manifest.h"

#ifndef FPA_TYPES
#define FPA_TYPES
#define NO_FPA 0
#define SKY_FPA 1
#define JUNIPER_FPA 2
int fpatype;
#endif

# ifdef ONEPASS

/*	bunch of stuff for putting the passes together... */
# define crslab crs2lab
# define where where2
# define xdebug x2debug
# define tdebug t2debug
# define deflab def2lab
# define edebug e2debug
# define eprint e2print
# define getlab get2lab
# define filename ftitle
# endif

/*	cookies, used as arguments to codgen */

# define FOREFF 01 /* compute for effects only */
# define INAREG 02 /* compute into a register */
# define INTAREG 04 /* compute into a scratch register */
# define INBREG 010 /* compute into a lvalue register */
# define INTBREG 020 /* compute into a scratch lvalue register */
# define FORCC 040 /* compute for condition codes only */
# define INTEMP 010000 /* compute into a temporary location */
# define FORARG 020000 /* compute for an argument of a function */
# define FORREW 040000 /* search the table, looking for a rewrite rule */

	/* OP descriptors */
	/* the ASG operator may be used on some of these */

# define OPSIMP 010000    /* +, -, &, |, ^ */
# define OPCOMM 010002  /* +, &, |, ^ */
# define OPMUL 010004  /* *, / */
# define OPDIV 010006 /* /, % */
# define OPUNARY 010010  /* unary ops */
# define OPLEAF 010012  /* leaves */
# define OPANY 010014  /* any op... */
# define OPLOG 010016 /* logical ops */
# define OPFLOAT 010020 /* +, -, *, or / (for floats) */
# define OPSHFT 010022  /* <<, >> */
# define OPLTYPE 010024  /* leaf type nodes (e.g, NAME, ICON, etc. ) */

	/* match returns */

# define MNOPE 010000
# define MDONE 010001

	/* shapes */

# define SANY 01	/* same as FOREFF */
# define SAREG 02	/* same as INAREG */
# define STAREG 04	/* same as INTAREG */
# define SBREG 010	/* same as INBREG */
# define STBREG 020	/* same as INTBREG */
# define SCC 040	/* same as FORCC */
# define SNAME 0100
# define SCON 0200
# define SFLD 0400
# define SOREG 01000
# define STARNM 02000
# define STARREG 04000
# define SWADD 040000
# define SPECIAL 0100000
# define SZERO SPECIAL
# define SONE (SPECIAL|1)
# define SMONE (SPECIAL|2)

	/* FORARG and INTEMP are carefully not conflicting with shapes */

	/* types */

# define TUNDEF 0
# define TCHAR 01
# define TSHORT 02
# define TINT 04
# define TLONG 010
# define TFLOAT 020
# define TDOUBLE 040
# define TPOINT 0100
# define TUCHAR 0200
# define TUSHORT 0400
# define TUNSIGNED 01000
# define TULONG 02000
# define TPTRTO 04000  /* pointer to one of the above */
# define TANY 010000  /* matches anything within reason */
# define TSTRUCT 020000   /* structure or union */

	/* reclamation cookies */

# define RNULL 0    /* clobber result */
# define RLEFT 01
# define RRIGHT 02
# define RESC1 04
# define RESC2 010
# define RESC3 020
# define RESCC 04000
# define RNOP 010000   /* DANGER: can cause loops.. */

	/* needs */

# define NAREG 01
# define NACOUNT 03
# define NAMASK 017
# define NASL 04  /* share left register */
# define NASR 010 /* share right register */
# define NBREG 020
# define NBCOUNT 060
# define NBMASK 0360
# define NBSL 0100
# define NBSR 0200
# define NTEMP 0400
# define NTMASK 07400
# define REWRITE 010000


# define MUSTDO 010000   /* force register requirements */
# define NOPREF 020000  /* no preference for register assignment */


	/* register allocation */

extern int rstatus[];
extern int busy[];

extern struct respref { int cform; int mform; } respref[];

# define isbreg(r) (rstatus[r]&SBREG)
# define istreg(r) (rstatus[r]&(STBREG|STAREG))
# define istnode(p) (p->in.op==REG && istreg(p->tn.rval))

# define TBUSY 01000
# define REGLOOP(i) for(i=0;i<REGSZ;++i)

# define SETSTO(x,y) (stotree=(x),stocook=(y))
extern int stocook;
# define DELAYS 20
extern NODE *deltrees[DELAYS];
extern int deli;   /* mmmmm */

extern NODE *stotree;
extern int callflag;

extern int fregs;

# ifndef ONEPASS
union ndu {

	struct {
		int op;
		int rall;
		TWORD type;
		int su;
		char name[NCHNAM];
		NODE *left;
		NODE *right;
		}in; /* interior node */
	
	struct {
		int op;
		int rall;
		TWORD type;
		int su;
		char name[NCHNAM];
		CONSZ lval;
		int rval;
		}tn; /* terminal node */
	
	struct {
		int op, rall;
		TWORD type;
		int su;
		int label;  /* for use with branching */
		}bn; /* branch node */

	struct {
		int op, rall;
		TWORD type;
		int su;
		int stsize;  /* sizes of structure objects */
		int stalign;  /* alignment of structure objects */
		}stn; /* structure node */

	};
#endif

extern NODE node[];

extern struct optab {
	int op;
	int visit;
	int lshape;
	int ltype;
	int rshape;
	int rtype;
	int needs;
	int rewrite;
	char * cstring;
	}
	table[];

extern NODE resc[];

extern OFFSZ tmpoff;
extern OFFSZ maxoff;
extern OFFSZ baseoff;
extern OFFSZ maxtemp;
extern int maxtreg;
extern int ftnno;
extern int rtyflg;

extern int nrecur;  /* flag to keep track of recursions */

# define NRECUR (10*TREESZ)	/* decreased 7/13/87 from 100*TREESZ (gb) */

extern NODE
	*talloc(),
	*eread(),
	*tcopy(),
	*getlr();

extern CONSZ rdin();

extern int eprint();

extern char *rnames[];

extern int lineno;
extern char filename[];
extern int fldshf, fldsz;
extern int lflag, xdebug, udebug, edebug, odebug, rdebug, radebug, tdebug, sdebug;

#ifndef callchk
#define callchk(x) allchk()
#endif

#ifndef PUTCHAR
# define PUTCHAR(x) putchar(x)
#endif

	/* macros for doing double indexing */
# define R2PACK(x,y) (0200*((x)+1)+y)
# define R2UPK1(x) ((((x)>>7)&0177)-1)
# define R2UPK2(x) ((x)&0177)
# define R2TEST(x) ((x)>=0200)
