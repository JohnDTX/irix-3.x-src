
typedef	struct dbxheadr {
	long	nsyms;
	long	stringsize;
	long	nfiles;
} DBXHEADER;	

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

#define getType(s)


extern	Symbol	curmodule;
extern	Symbol	curparam;
extern	Language	curlang;
extern	long	nesting;
extern	Symbol	typetable[];
extern	Symbol	makesymbol();
extern	long	cursymnum;
extern	DBXHEADER	dbxhead;
extern int curlevel;
extern	Symbol	mainfunc;
extern	Symbol	*symrefs;
extern char *newstringtab;
