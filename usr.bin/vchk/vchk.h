/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/* @(#)vchk.h	1.4 */

#ifdef PWB
#undef UniSoft
#define mc68000
#include "v3.h"
#endif

#ifndef mc68000
#define UniSoft
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LINESIZ	1500		/* Length of longest legal input line */
#define PATHSIZ	800		/* Length of longest legal pathname */
#define TAB_WIDTH 8		/* tab width length for P optn */
#define LINE_LEN 80		/* line length for P optn */
#define NBRKT	40		/* max depth of () and {} structures */

int Pdfcol, Psfcol;		/* tabstop of data and spec field (bflag) */

#define SE sys_errlist[errno]
extern char *sys_errlist[];
extern errno;
#define when break; case
#define otherwise break; default

	/* following initialized in vchk.c*/
int	Defmode;		/* default mode */
int	Lno;			/* Current line number in spec file */

int Curtype;			/* type of thing being defined */

/* type of line currently processing,
 * DIRECTORY for directories, ITEM all other kinds of files,
 * CONTENTS for contents of a directory, and ERROR if unknown or syntax error.
 */
#define DIRECTORY	1
#define ITEM		2
#define CONTENTS	3
#define IGNORE		4
#define ERROR		5

char Err[LINESIZ];
int Errflag;			/* exit when convenient if set */

	/* command line options */
#ifndef NODEBUG
int Dflag;			/* debugging level */
#endif

int Bflag;			/* rebuild description file */
int Iflag;			/* ignore lines of source */
int Sflag;			/* do it, but don't say a word */

int aflag;			/* do everything (even < lines) */
int bflag;			/* build new tree file on standard output */
int cflag;			/* output commands to fix things */
int dflag;			/* don't try to remake anything */
int eflag;			/* check for existance only */
int iflag;			/* interactive query about certain things */
int kflag;			/* perform chksums on indicated lines */
int lflag;			/* don't load contents of directories */
int mflag;			/* don't complain about duplicate files */
int nflag;			/* use the ethernet */
int rflag;			/* don't filter repeated uids from passwd file*/
int sflag;			/* be silent about password errors */
int vflag;			/* be silent about mode and owner problems */
int xflag;			/* execute commands */
int yflag;			/* sync after remake commands */

char *Pflag;			/* preprocess file only (for oem specified) */
char *fflag;			/* use different file */
char *pflag;			/* re-evaluate or use diff password file */

int lastnl;			/* flag to suppress redundant blank lines */

struct plist			/* abbreviated password entry list */
{	char *name;
	short uid;
	short gid;
};
#define ANYOWNER -2		/* kludgy way to signify any owner */
struct plist Defownr;		/* default owner */

/* One node per directory referenced explicitly or implicitly.
 * Used for keeping default mode, owner for that directory, list
 * of file in each directory (unless no memory or lflag)
 */
struct tree_node {
	struct tree_node *t_parent;	/* null if top node */
	struct tree_node *t_next;	/* next at this level */
	struct tree_node *t_list;	/* first subdirectory entry */
	char	       ***t_ls;		/* list of all filenames in dir */
	short		  t_flags;	/* special flags (see TF_ below) */
	short		  t_dmode;	/* default mode for contents */
	short		  t_duid;	/* def uid for contents */
	short		  t_dgid;	/* def gid for contents */
	short		  t_knt;	/* number of entries in this dir */
	short		  t_gid;	/* gid for this directory */
	short		  t_uid;	/* uid for this directory */
	short		  t_mode;	/* mode for directory */
	short		  t_size;	/* number of subdirectories */
	short		  t_idnum;	/* internal number (unique per dir) */
	char		  t_name[16];	/* name of this entry */
};
	/* values of t_flags above */
#define TF_ANYOWND 1			/* may have any owner */
#define TF_ANYOWNC 2			/* files may have any onwer */
#define TF_HADERR  4			/* listed directory in errlist file */
#define TF_SETUP   8			/* this dir has its owner and mode */

struct tree_node *first_node;		/* first tree in list */
struct tree_node *Curdir;		/* Directory containing items to check*/
int cur_idnum;				/* last idnum used */

struct find_next {			/* for resolving links -- see below */
	struct find_next *fn_next;
	char		  fn_name[2];	/* first byte has flags */
};

struct find_list {			/* link header */
	short		  fl_dev;	/* device */
	short		  fl_ino;	/* inode number */
	short		  fl_knt;	/* to find before resolved */
	short		  fl_id;	/* dir containing resolving link */
	struct find_list *fl_nxt;	/* next in list of link headers */
	struct find_next  fl_ent;	/* dont put anything after this */
};

struct find_list *lhdr;

#define X(y) { if (!Sflag) lerrout y ; }
#define M(y) { if (!Sflag) errout y ; }
#define N(y) { if (!Sflag && !sflag) lerrout y ; }
#define C(y) { if (!Sflag && !sflag) errout y ; }

		/* enables limit checking on some datastructs */
#define BUGCHECK

#ifndef NODEBUG
#define D(level,y) { if (Dflag >= level) errout y ; }
#define DEBUG
#else
#define D(level,y)
#endif
#define E(y) exit((lerrout y,1))
#define F(y) exit((errout y,1))

char	LPRENS[];		/* set of meaningful prens */
char	RPRENS[];

#define LPREN LPRENS[0]		/* Parenthesis () */
#define RPREN RPRENS[0]

#define LSQIG LPRENS[1]		/* Squiggly braces {} */
#define RSQIG RPRENS[1]

#define LBRKT LPRENS[2]		/* Square brackets [] */
#define RBRKT RPRENS[2]

#define LANGL LPRENS[3]		/* Angle brackets <> */
#define RANGL RPRENS[3]

#define F_COMMAND	1
#define F_FILEPTR	2
#define F_FILENAME	3
#define F_STRING	4

char *pindex();			/* libucsc -- like index(libc) but does \ esc */
char *cindex();			/* libucsc -- like pindex but arg2 is str */
char *pmatch();			/* libucsc -- pmatch(cp) = ptr matching pren*/

int skipping;
struct stat sibuf;

struct key {			/* one for each {} alternative on a line */
	short	k_mis;		/* list of entries that were missing */
	short	k_chk;		/* list of entries that we checked */
	short	k_dup;		/* list of ents that are duplicates or links */
	short	k_alt;		/* list that are named the same but diff */
};

#define NKEYS 50
struct key keytab[NKEYS];	/* max number of {} alternative expansions */
short reptab[NKEYS];		/* ptr to representative string for each */

struct ent {			/* continuation pointers (short k_...) */
	short e_nxt;		/* Index of next entry in entab */
	short e_str;		/* Index of string start in chrbuf */
};

struct macro {			/* malloc'd structure used to hold macros */
	struct macro *m_next;
	struct macro *m_prev;
	char	     *m_val;
	char	      m_name[1];
};
struct macro *first_mac;	/* pointer to linked list of defined macros*/

#define ENTSIZ	300
struct ent entab [ENTSIZ];	/* max number of strings in chrbuf */
#define MAXCHRS (LINESIZ << 2)
char chrbuf[MAXCHRS];		/* list of different files */
char *chrbp;			/* next avail character (chrbuf entry)*/
struct ent *nxtent;		/* next avail entab entry */
struct key *nxtkey;		/* next avail keytab entry */
struct key *extkey;		/* current entry working on */
char brktbuf[NBRKT];		/* max depth of () and {} structures */
char *brktlev;			/* map of ( {( {}) } )'s ... */
char blk_buf[BUFSIZ];		/* for various setbufs */
