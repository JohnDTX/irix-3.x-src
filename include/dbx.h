/*
 * $Source: /d2/3.7/src/include/RCS/dbx.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:17 $
 */

/*
 *	This file describes the fortran dbx symbol and line information
 */

typedef	struct dbxheader {
	long	nsyms;				/* number of symbol entries */
	long	stringsize;			/* size of string table */
	long	nfiles;				/* Number of source files */
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
	long	funcnum;			/* The symbol number of the function these
								 * lines are from */
	long	nlines;				/* The number of lines in this function */
} LINEHEADER;

typedef	struct	lineaddr {
	short	addroff;			/* The offset of this line from the beggining
								 * of the function or the last line */
	short	lineno;				/* The line number */
} LINEADDR;

typedef struct blockstrs {
	char	*blockname;
	int	blockid;
} BLOCKSTRS;

#define	NBLKNMS	19

extern	BLOCKSTRS	blockstrs[];
