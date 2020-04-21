
/*
 * Block types for SVS relocatable .j files
 */
#define HEAD1BLOCK	0x80 
#define END1BLOCK	0x81
#define ENTRY1BLOCK	0x82
#define EXTERN1BLOCKFF	0x83
#define START1BLOCK	0x84
#define CODE1BLOCK	0x85
#define RELOC1BLOCK	0x86
#define EXTERN1BLOCKFT	0x89
#define FDDEF1BLOCKF	0x8A
#define FDREF1BLOCKF	0x8C
#define ENTRY1BLOCKT	0xB0
#define EXTERN1BLOCKTF	0xB1
#define FDFREL1BLOCK	0xB3
#define FDDEF1BLOCKT	0xB4
#define FDINIT1BLOCK	0xB5
#define FDREF1BLOCKT	0xB6
#define FDREL1BLOCK	0xB7
#define VARTYP1BLOCK	0xA0
#define BPOINT1BLOCK	0xA1

typedef	struct dbxheader {
	long	nsyms;
	long	stringsize;
	long	nfiles;
} DBXHEADER;	

/*
 * The line number correlation table
 */
typedef struct	srcfile {
	long	filename;		/* Source file name */
	long	nfuncs;			/* number of FUNCHEADER's */
					/* in this file */
} SRCFILE;


typedef struct	lineheader {
	long	funcnum;
	long	nlines;
} LINEHEADER;

typedef	struct	lineaddr {
	short	addroff;
	short	lineno;
} LINEADDR;

typedef struct blockstrs {
	char	*blockname;
	int	blockid;
} BLOCKSTRS;

#define	NBLKNMS	19

extern	BLOCKSTRS	blockstrs[];

/*
 * Languages supported by SVS
 */

#define PASCAL	0
#define FORTRAN	1
#define BASIC	2
#define C	3

extern	char	*langstrs[];

/*
 * SVS variable and type descriptor
 * for procedures
 */

typedef	struct	procinfo {
	char	pr_lang;
	char	pr_version;
	char	pr_subvers;
	char	pr_level;
	char	pr_retlev;
	char	*pr_linknm;
	char	*pr_outlinknm;
	char	*pr_usernm;
} PROCINFO;

/*
 * pr_retlev possibilities:
 */

#define SVSMAIN 1
#define SVSPROC 2
#define SVSFUNC 3

/*
 * SVS basic types
 */

#define	SVSINT1	(-1)
#define	SVSINT2	(-2)
#define SVSINT4	(-3)
#define	SVSUINT1	(-4)
#define	SVSUINT2	(-5)
#define	SVSUINT4	(-6)
#define	SVSCHR1	(-7)
#define	SVSCHR2	(-8)
#define	SVSFLT	(-9)
#define	SVSDBL	(-10)
#define	SVSLOG1	(-11)
#define	SVSLOG2	(-12)
#define	SVSLOG4	(-13)
#define	SVSFILET	(-14)
#define	SVSCOMPX	(-15)

/*
 * type kinds
 */

#define SVSSCALER	0
#define SVSSUBRANGE	1
#define SVSPTR		2
#define SVSSET		3
#define SVSARRAY	4
#define SVSSTRNG	5
#define SVSFILE		6
#define SVSRECORD	7
#define SVSCHAR		9
#define SVSFORTARR	10

typedef	struct	svsfordim	SVSFORDIM;

typedef	struct	svsvarble {
	char	*name;
	short	vartype;
	int	varclass;
};

struct	svsfordim {
	char	flags;
	long	lobound;
	long	hibound;
	long	elemsize;
	SVSFORDIM	*nextdim;
};

typedef	struct	svsforarr {
	char	numdims;
	short	typeno;
	SVSFORDIM	*svsdimp;
}	SVSFORARR;

#define SVSFORARRSZ	7
#define	MAXTYPES	5000
#define	NUMSVSTYPS	16

#define getType(s)


extern	SRCFILE	srcf;
extern	Symbol	curmodule;
extern	Symbol	curparam;
extern	short	curlang;
extern	long	nesting;
extern	Symbol	typetable[];
extern	Symbol	makesymbol();
extern	long	cursymnum;
extern	DBXHEADER	dbxhead;
