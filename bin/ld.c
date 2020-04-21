#ifndef lint
static	char sccsid[] = "@(#)ld.c 4.9 7/1/83";
#endif

/*
 * ld - string table version for VAX
 */

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <ar.h>
#include <a.out.h>
#include <ranlib.h>
#include <sys/stat.h>

/*
 * Basic strategy:
 *
 * The loader takes a number of files and libraries as arguments.
 * A first pass examines each file in turn.  Normal files are
 * unconditionally loaded, and the (external) symbols they define and require
 * are noted in the symbol table.   Libraries are searched, and the
 * library members which define needed symbols are remembered
 * in a special data structure so they can be selected on the second
 * pass.  Symbols defined and required by library members are also
 * recorded.
 *
 * After the first pass, the loader knows the size of the basic text
 * data, and bss segments from the sum of the sizes of the modules which
 * were required.  It has computed, for each ``common'' symbol, the
 * maximum size of any reference to it, and these symbols are then assigned
 * storage locations after their sizes are appropriately rounded.
 * The loader now knows all sizes for the eventual output file, and
 * can determine the final locations of external symbols before it
 * begins a second pass.
 *
 * On the second pass each normal file and required library member
 * is processed again.  The symbol table for each such file is
 * reread and relevant parts of it are placed in the output.  The offsets
 * in the local symbol table for externally defined symbols are recorded
 * since relocation information refers to symbols in this way.
 * Armed with all necessary information, the text and data segments
 * are relocated and the result is placed in the output file, which
 * is pasted together, ``in place'', by writing to it in several
 * different places concurrently.
 */

/*
 * Internal data structures
 *
 * All internal data structures are segmented and dynamically extended.
 * The basic structures hold 1103 (NSYM) symbols, ~~200 (NROUT)
 * referenced library members, and 100 (NSYMPR) private (local) symbols
 * per object module.  For large programs and/or modules, these structures
 * expand to be up to 40 (NSEG) times as large as this as necessary.
 */
#define	NSEG	40		/* Number of segments, each data structure */
#define	NSYM	1103		/* Number of symbols per segment */
#define	NROUT	250		/* Number of library references per segment */
#define	NSYMPR	100		/* Number of private symbols per segment */

/*
 * Structure describing each symbol table segment.
 * Each segment has its own hash table.  We record the first
 * address in and first address beyond both the symbol and hash
 * tables, for use in the routine symx and the lookup routine respectively.
 * The symfree routine also understands this structure well as it used
 * to back out symbols from modules we decide that we don't need in pass 1.
 *
 * Csymseg points to the current symbol table segment;
 * csymseg->sy_first[csymseg->sy_used] is the next symbol slot to be allocated,
 * (unless csymseg->sy_used == NSYM in which case we will allocate another
 * symbol table segment first.)
 */
struct	symseg {
	struct	nlist *sy_first;	/* base of this alloc'ed segment */
	struct	nlist *sy_last;		/* end of this segment, for n_strx */
	int	sy_used;		/* symbols used in this seg */
	struct	nlist **sy_hfirst;	/* base of hash table, this seg */
	struct	nlist **sy_hlast;	/* end of hash table, this seg */
} symseg[NSEG], *csymseg;

/*
 * The lookup routine uses quadratic rehash.  Since a quadratic rehash
 * only probes 1/2 of the buckets in the table, and since the hash
 * table is segmented the same way the symbol table is, we make the
 * hash table have twice as many buckets as there are symbol table slots
 * in the segment.  This guarantees that the quadratic rehash will never
 * fail to find an empty bucket if the segment is not full and the
 * symbol is not there.
 */
#define	HSIZE	(NSYM*2)

/*
 * Xsym converts symbol table indices (ala x) into symbol table pointers.
 * Symx (harder, but never used in loops) inverts pointers into the symbol
 * table into indices using the symseg[] structure.
 */
#define	xsym(x)	(symseg[(x)/NSYM].sy_first+((x)%NSYM))
/* symx() is a function, defined below */

struct	nlist cursym;		/* current symbol */
struct	nlist *lastsym;		/* last symbol entered */
struct	nlist *nextsym;		/* next available symbol table entry */
struct	nlist *addsym;		/* first sym defined during incr load */
int	nsym;			/* pass2: number of local symbols in a.out */
/* nsym + symx(nextsym) is the symbol table size during pass2 */

struct	nlist **lookup(), **slookup();
struct	nlist *p_etext, *p_edata, *p_end, *p_bstart, *entrypt;

/*
 * Definitions of segmentation for library member table.
 * For each library we encounter on pass 1 we record pointers to all
 * members which we will load on pass 2.  These are recorded as offsets
 * into the archive in the library member table.  Libraries are
 * separated in the table by the special offset value -1.
 */
off_t	li_init[NROUT];
struct	libseg {
	off_t	*li_first;
	int	li_used;
	int	li_used2;
} libseg[NSEG] = {
	li_init, 0, 0,
}, *clibseg = libseg;

/*
 * In processing each module on pass 2 we must relocate references
 * relative to external symbols.  These references are recorded
 * in the relocation information as relative to local symbol numbers
 * assigned to the external symbols when the module was created.
 * Thus before relocating the module in pass 2 we create a table
 * which maps these internal numbers to symbol table entries.
 * A hash table is constructed, based on the local symbol table indices,
 * for quick lookup of these symbols.
 */
#define	LHSIZ	31
struct	local {
	int	l_index;		/* index to symbol in file */
	struct	nlist *l_symbol;	/* ptr to symbol table */
	struct	local *l_link;		/* hash link */
} *lochash[LHSIZ], lhinit[NSYMPR];
struct	locseg {
	struct	local *lo_first;
	int	lo_used;
} locseg[NSEG] = {
	lhinit, 0
}, *clocseg;

/*
 * Libraries are typically built with a table of contents,
 * which is the first member of a library with special file
 * name __.SYMDEF and contains a list of symbol names
 * and with each symbol the offset of the library member which defines
 * it.  The loader uses this table to quickly tell which library members
 * are (potentially) useful.  The alternative, examining the symbol
 * table of each library member, is painfully slow for large archives.
 *
 * See <ranlib.h> for the definition of the ranlib structure and an
 * explanation of the __.SYMDEF file format.
 */
int	tnum;		/* number of symbols in table of contents */
int	ssiz;		/* size of string table for table of contents */
struct	ranlib *tab;	/* the table of contents (dynamically allocated) */
char	*tabstr;	/* string table for table of contents */

/*
 * We open each input file or library only once, but in pass2 we
 * (historically) read from such a file at 2 different places at the
 * same time.  These structures are remnants from those days,
 * and now serve only to catch ``Premature EOF''.
 * In order to make I/O more efficient, we provide routines which
 * work in hardware page sizes. The associated constants are defined 
 * as BLKSIZE, BLKSHIFT, and BLKMASK.
 */
#define BLKSIZE 1024
#define BLKSHIFT 10
#define BLKMASK (BLKSIZE - 1)
typedef struct {
	short	*fakeptr;
	int	bno;
	int	nibuf;
	int	nuser;
	char	buff[BLKSIZE];
} PAGE;

PAGE	page[2];

struct {
	short	*fakeptr;
	int	bno;
	int	nibuf;
	int	nuser;
} fpage;

typedef struct {
	char	*ptr;
	int	bno;
	int	nibuf;
	long	size;
	long	pos;
	PAGE	*pno;
} STREAM;

STREAM	text;
STREAM	reloc;

/*
 * Header from the a.out and the archive it is from (if any).
 */
struct	exec filhdr;
struct	ar_hdr archdr;
#define	OARMAG 0177545

/*
 * Options.
 */
int	trace;
int	xflag;		/* discard local symbols */
int	Xflag;		/* discard locals starting with '.' */ /* changed (GB) */
int	Sflag;		/* discard all except locals and globals*/
int	rflag;		/* preserve relocation bits, don't define common */
int	arflag;		/* original copy of rflag */
int	sflag;		/* discard all symbols */
int	Mflag;		/* print rudimentary load map */
int	nflag;		/* pure procedure */
int	zflag;		/* demand paged  */
int	dflag;		/* define common even with rflag */
long	hsize;		/* size of hole at beginning of data to be squashed */
int	Aflag;		/* doing incremental load */
int	Nflag;		/* want impure a.out */
int	funding;	/* reading fundamental file for incremental load */
int	yflag;		/* number of symbols to be traced */
int	seenlibs;	/* set when the first library is seen in the args (GB)*/
char	**ytab;		/* the symbols */
char	*libroot = "";  /* root of library tree (GB) */

#define LIBPATHMAX 0x80
#define MINLIBEXT 9
#define LIBTREELEN 8
#define USRLIBTREELEN 12
#define LOCALLIBTREELEN 18
#define LIBEXTMAX LIBPATHMAX - LOCALLIBTREELEN - 3
int libextmax = MINLIBEXT;
char *def_libtree = "/lib/lib";
char *def_usrlibtree = "/usr/lib/lib";
char *def_locallibtree = "/usr/local/lib/lib";
char usrlibtree[LIBPATHMAX+1] = "/usr/lib/lib";
char libtree[LIBPATHMAX+1] = "/lib/lib";
char locallibtree[LIBPATHMAX+1] = "/usr/local/lib/lib";
char *libtreehackbase = &libtree[LIBTREELEN];
char *usrlibtreehackbase = &usrlibtree[USRLIBTREELEN];
char *locallibtreehackbase = &locallibtree[LOCALLIBTREELEN];

long	stringsize;

/*
 * These are the cumulative sizes, set in pass 1, which
 * appear in the a.out header when the loader is finished.
 */
off_t	tsize, dsize, bsize, trsize, drsize, ssize;

/*
 * Symbol relocation: c?rel is a scale factor which is
 * added to an old relocation to convert it to new units;
 * i.e. it is the difference between segment origins.
 * (Thus if we are loading from a data segment which began at location
 * 4 in a .o file into an a.out where it will be loaded starting at
 * 1024, cdrel will be 1020.)
 */
long	ctrel, cdrel, cbrel;

/*
 * Textbase is the start address of all text, DEFAULTORIGIN unless given by -T.
 * Database is the base of all data, computed before and used during pass2.
 */
#define DEFAULTORIGIN 0x1000
long	textbase = DEFAULTORIGIN, database;

/*
 * The base addresses for the loaded text, data and bss from the
 * current module during pass2 are given by torigin, dorigin and borigin.
 */
long	torigin, dorigin, borigin;

/*
 * Errlev is nonzero when errors have occured.
 * Delarg is an implicit argument to the routine delexit
 * which is called on error.  We do ``delarg = errlev'' before normal
 * exits, and only if delarg is 0 (i.e. errlev was 0) do we make the
 * result file executable.
 */
int	errlev;
int	delarg	= 4;

/*
 * The biobuf structure and associated routines are used to write
 * into one file at several places concurrently.  Calling bopen
 * with a biobuf structure sets it up to write ``biofd'' starting
 * at the specified offset.  You can then use ``bwrite'' and/or ``bputc''
 * to stuff characters in the stream, much like ``fwrite'' and ``fputc''.
 * Calling bflush drains all the buffers and MUST be done before exit.
 */
struct	biobuf {
	short	b_nleft;		/* Number free spaces left in b_buf */
/* Initialize to be less than BUFSIZ initially, to boundary align in file */
	char	*b_ptr;			/* Next place to stuff characters */
	char	b_buf[BUFSIZ];		/* The buffer itself */
	off_t	b_off;			/* Current file offset */
	struct	biobuf *b_link;		/* Link in chain for bflush() */
} *biobufs;
#define	bputc(c,b) ((b)->b_nleft ? (--(b)->b_nleft, *(b)->b_ptr++ = (c)) \
		       : bflushc(b, c))
int	biofd;
off_t	boffset;
struct	biobuf *tout, *dout, *trout, *drout, *sout, *strout;

/*
 * Offset is the current offset in the string file.
 * Its initial value reflects the fact that we will
 * eventually stuff the size of the string table at the
 * beginning of the string table (i.e. offset itself!).
 */
off_t	offset = sizeof (off_t);

int	ofilfnd;		/* -o given; otherwise move l.out to a.out */
char	*ofilename = "l.out";
int	ofilemode;		/* respect umask even for unsucessful ld's */
int	infil;			/* current input file descriptor */
char	*filname;		/* and its name */
#ifdef PROMSTUFF
int 	LDorigin;
int 	LCorigin;
#endif

/*
 * Base of the string table of the current module (pass1 and pass2).
 */
char	*curstr;

/*
 * System software page size, as returned by getpagesize.
 */
int	pagesize;

char 	get();
int	delexit();
char	*savestr();

main(argc, argv)
char **argv;
{
	register int c, i; 
	int num;
	register char *ap, **p;
	char save;
	int newrootlen;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
		signal(SIGINT, delexit);
		signal(SIGTERM, delexit);
	}
	if (argc == 1)
		exit(4);
	p = argv+1;
#ifdef	vax
	pagesize = getpagesize();
#else
	pagesize = 4096;
#endif

	/*
	 * Scan files once to find where symbols are defined.
	 */
	for (c=1; c<argc; c++) {
		if (trace)
			fprintf(stderr,"%s:\n", *p);
		filname = 0;
		ap = *p++;
		if (*ap != '-') {
			load1arg(ap);
			continue;
		}
		for (i=1; ap[i]; i++) switch (ap[i]) {

		case 'o':
			if (++c >= argc)
				error(1, "-o: no output file given.");
			ofilename = *p++;
			ofilfnd++;
			continue;
		case 'u':
		case 'e':
			if (++c >= argc)
				error(1, "-u or -c: arg missing");
			enter(slookup(*p++));
			if (ap[i]=='e')
				entrypt = lastsym;
			continue;
		case 'H':
			if (++c >= argc)
				error(1, "-H: arg missing");
			if (tsize!=0)
				error(1, "-H: too late, some text already loaded");
			hsize = atoi(*p++);
			continue;
		case 'A':
			if (++c >= argc)
				error(1, "-A: arg missing");
			if (Aflag) 
				error(1, "-A: only one base file allowed");
			Aflag = 1;
			nflag = 0;
			funding = 1;
			load1arg(*p++);
			trsize = drsize = tsize = dsize = bsize = 0;
			ctrel = cdrel = cbrel = 0;
			funding = 0;
			addsym = nextsym;
			continue;
		case 'D':
			if (++c >= argc)
				error(1, "-D: arg missing");
			num = htoi(*p++);
			if (dsize > num)
				error(1, "-D: too small");
			dsize = num;
			continue;
#ifdef PROMSTUFF
		case 'L':
			if ((ap[i+1]=='T') || (ap[i+1]=='D') ||
		     		(ap[i+1]=='C' ))
			{
				char ch;
				int *p1;
				switch (ch = ap[++i]) {
					case 'T':	p1 = &textbase;	break;
					case 'D':	p1 = &LDorigin;	break;
					case 'C':	p1 = &LCorigin;	break;
				}
				if (tsize!=0)
				{
					error(1, 
				"-Lx: too late, some text already loaded");
				}
				else if ((c+1) >= argc)
				{
					error(1,
			"missing argument following command option -Lx");
				}
				else 
				{
					c++;
					*p1 = htoi(*p++);
				}
			}
			else error(1,"-Lx: unknown segment");
			continue;
#endif
		/* GB - added -R <path> for alternate library path */
		case 'R':
			if (seenlibs)
				error(1, "-R: too late, some libraries loaded.");
			if (++c >= argc)
				error(1, "-R: arg missing");
			if ((*libroot) != 0)
				error(1, "-R: already specified (%s)",libroot);
			libroot = *p++;
#ifdef LIBPATH_DEBUG
			fprintf(stderr,"libroot = '%s'\n",libroot);
#endif
			/* ok, add the new root to the 
			   libpath and the local lib path */
			newrootlen = strlen(libroot);
			if (newrootlen > (LIBEXTMAX - MINLIBEXT))
				error(1, "-R: path too long.  Max %d chars.",
					(LIBEXTMAX - MINLIBEXT));

			/* remake the possible library paths */
			/* and adjust the pointers to the modifiable part. */

			/* make the new library root */
			strncpy(libtree,libroot,newrootlen);
			strcpy(&libtree[newrootlen],def_libtree);
			libtreehackbase += newrootlen;
			*libtreehackbase = 0;

			/* make the new usr library root */
			strncpy(usrlibtree,libroot,newrootlen);
			strcpy(&usrlibtree[newrootlen],def_usrlibtree);
			usrlibtreehackbase += newrootlen;
			*usrlibtreehackbase = 0;

			/* make the new local library root */
			strncpy(locallibtree,libroot,newrootlen);
			strcpy(&locallibtree[newrootlen],def_locallibtree);
			locallibtreehackbase += newrootlen;
			*locallibtreehackbase = 0;

			/* and adjust the maximum length 
			   of a library extension 
			*/
			libextmax = LIBEXTMAX - newrootlen;
#ifdef LIBPATH_DEBUG
			fprintf(stderr,"libtree = %s\nusrlibtree = %s\nlocallibtree = %s, maxlibext = %d\n",
				libtree,usrlibtree,locallibtree,libextmax);
#endif
			continue;
		case 'T':
			if (++c >= argc)
				error(1, "-T: arg missing");
			if (tsize!=0)
				error(1, "-T: too late, some text already loaded");
			textbase = htoi(*p++);
			continue;
		case 'l':
			seenlibs++;
			if (strlen(&ap[i+1]) > libextmax)
				error(1, 
			    "-l: library name (%s) too long. Max %d chars.",
					&ap[i],libextmax);
			save = ap[--i]; 
			ap[i]='-';
			load1arg(&ap[i]); 
			ap[i]=save;
			goto next;
		case 'M':
			Mflag++;
			continue;
		case 'x':
			xflag++;
			continue;
		case 'X':
			Xflag++;
			continue;
		case 'S':
			Sflag++; 
			continue;
		case 'r':
			rflag++;
			arflag++;
			continue;
		case 's':
			sflag++;
			xflag++;
			continue;
		case 'n':
			nflag++;
			Nflag = zflag = 0;
			continue;
		case 'N':
			Nflag++;
			nflag = zflag = 0;
			continue;
		case 'd':
			dflag++;
			continue;
		case 'i':
			fprintf(stderr,"ld: -i ignored\n");
			continue;
		case 't':
			trace++;
			continue;
		case 'y':
			if (ap[i+1] == 0)
				error(1, "-y: symbol name missing");
			if (yflag == 0) {
				ytab = (char **)calloc(argc, sizeof (char **));
				if (ytab == 0)
					error(1, "ran out of memory (-y)");
			}
			ytab[yflag++] = &ap[i+1];
			goto next;
		case 'z':
			zflag++;
			Nflag = nflag = 0;
			continue;
		default:
			filname = savestr("-x");	/* kludge */
			filname[1] = ap[i];		/* kludge */
			archdr.ar_name[0] = 0;		/* kludge */
			error(1, "bad flag");
		}
next:
		;
	}
	/* dont use demand paged files yet GB
	if (rflag == 0 && Nflag == 0 && nflag == 0)
		zflag++;
	*/
	if (rflag) textbase = 0; /* GB */
	endload(argc, argv);
	exit(0);
}

/*
 * Convert a ascii string which is a hex number.
 * Used by -T and -D options.
 */
htoi(p)
	register char *p;
{
	register int c, n;

	n = 0;
	while (c = *p++) {
		n <<= 4;
		if (isdigit(c))
			n += c - '0';
		else if (c >= 'a' && c <= 'f')
			n += 10 + (c - 'a');
		else if (c >= 'A' && c <= 'F')
			n += 10 + (c - 'A');
		else
			error(1, "badly formed hex number");
	}
	return (n);
}

delexit()
{
	struct stat stbuf;
	long size;
	char c = 0;

	bflush();
	unlink("l.out");
	if (delarg==0 && Aflag==0)
		chmod(ofilename, ofilemode);
	/*
	 * We have to insure that the last block of the data segment
	 * is allocated a full BLKSIZE block. If the underlying
	 * file system allocates frags that are smaller than BLKSIZE,
	 * a full zero filled BLKSIZE block needs to be allocated so 
	 * that when it is demand paged, the paged in block will be 
	 * appropriately filled with zeros.
	 */
	fstat(biofd, &stbuf);
	size = round(stbuf.st_size, BLKSIZE);
	if (!rflag && size > stbuf.st_size) {
		lseek(biofd, size - 1, 0);
		write(biofd, &c, 1);
	} else {
		lseek(biofd, 0, 2);
	}
#ifdef DEBUG1
	asm("	jmp 0");
#endif
	catdbxinfo(biofd);
	exit (delarg);
}

endload(argc, argv)
	int argc; 
	char **argv;
{
	register int c, i; 
	long dnum;
	register char *ap, **p;

	clibseg = libseg;
	filname = 0;
	middle();
	setupout();
	p = argv+1;
	for (c=1; c<argc; c++) {
		ap = *p++;
		if (trace)
			fprintf(stderr,"%s:\n", ap);
		if (*ap != '-') {
			load2arg(ap);
			continue;
		}
		for (i=1; ap[i]; i++) switch (ap[i]) {

		case 'D':
			dnum = htoi(*p);
			if (dorigin < dnum)
				while (dorigin < dnum)
					bputc(0, dout), dorigin++;
			/* fall into ... */
		case 'T':
		case 'R': /* GB */
		case 'u':
		case 'e':
		case 'o':
		case 'H':
			++c; 
			++p;
			/* fall into ... */
		default:
			continue;
		case 'A':
			funding = 1;
			load2arg(*p++);
			funding = 0;
			c++;
			continue;
		case 'y':
			goto next;
		case 'l':
			ap[--i]='-'; 
			load2arg(&ap[i]);
			goto next;
#ifdef PROMSTUFF
		case 'L':
			++i;
			++c; 
			++p;
#endif
		}
next:
		;
	}
	finishout();
}

/*
 * Scan file to find defined symbols.
 */
load1arg(cp)
	register char *cp;
{
	register struct ranlib *tp;
	off_t nloc;
	int kind;

#ifdef LIBPATH_DEBUG
	fprintf(stderr,"load1: getfile of %s\n",cp);
#endif
	kind = getfile(cp);
#ifdef LIBPATH_DEBUG
	fprintf(stderr,"load1: getfile returned %s as kind %d\n",cp,kind);
#endif
	if (Mflag)
		fprintf(stderr,"%s\n", filname);
	switch (kind) {

	/*
	 * Plain file.
	 */
	case 0:
		load1(0, 0L);
		break;

	/*
	 * Archive without table of contents.
	 * (Slowly) process each member.
	 */
	case 1:
		/* GB - make no TOC legal - dont complain */
#ifdef NOTDEF
		error(-1,
"warning: archive has no table of contents; add one using ranlib(1)");
#endif
		nloc = SARMAG;
		while (step(nloc))
			nloc += sizeof(archdr) +
			    round(atol(archdr.ar_size), sizeof (short));
		break;

	/*
	 * Archive with table of contents.
	 * Read the table of contents and its associated string table.
	 * Pass through the library resolving symbols until nothing changes
	 * for an entire pass (i.e. you can get away with backward references
	 * when there is a table of contents!)
	 */
	case 2:
		nloc = SARMAG + sizeof (archdr);
		dseek(&text, nloc, sizeof (tnum));
		mget((char *)&tnum, sizeof (tnum), &text);
		nloc += sizeof (tnum);
		tab = (struct ranlib *)malloc(tnum);
		if (tab == 0)
			error(1, "ran out of memory (toc)");
		dseek(&text, nloc, tnum);
		mget((char *)tab, tnum, &text);
		nloc += tnum;
		tnum /= sizeof (struct ranlib);
		dseek(&text, nloc, sizeof (ssiz));
		mget((char *)&ssiz, sizeof (ssiz), &text);
		nloc += sizeof (ssiz);
		tabstr = (char *)malloc(ssiz);
		if (tabstr == 0)
			error(1, "ran out of memory (tocstr)");
		dseek(&text, nloc, ssiz);
		mget((char *)tabstr, ssiz, &text);
		for (tp = &tab[tnum]; --tp >= tab;) {
			if (tp->ran_un.ran_strx < 0 ||
			    tp->ran_un.ran_strx >= ssiz)
				error(1, "mangled archive table of contents");
			tp->ran_un.ran_name = tabstr + tp->ran_un.ran_strx;
		}
		while (ldrand())
			continue;
		cfree((char *)tab);
		cfree(tabstr);
		nextlibp(-1);
		break;

	/*
	 * Table of contents is out of date, so search
	 * as a normal library (but skip the __.SYMDEF file).
	 */
	case 3:
		error(-1,
"warning: table of contents for archive is out of date; rerun ranlib(1)");
		nloc = SARMAG;
		do
			nloc += sizeof(archdr) +
			    round(atol(archdr.ar_size), sizeof(short));
		while (step(nloc));
		break;
	}
	close(infil);
}

/*
 * Advance to the next archive member, which
 * is at offset nloc in the archive.  If the member
 * is useful, record its location in the liblist structure
 * for use in pass2.  Mark the end of the archive in libilst with a -1.
 */
step(nloc)
	off_t nloc;
{

	dseek(&text, nloc, (long) sizeof archdr);
	if (text.size <= 0) {
		nextlibp(-1);
		return (0);
	}
	getarhdr();
	if (load1(1, nloc + (sizeof archdr)))
		nextlibp(nloc);
	return (1);
}

/*
 * Record the location of a useful archive member.
 * Recording -1 marks the end of files from an archive.
 * The liblist data structure is dynamically extended here.
 */
nextlibp(val)
	off_t val;
{

	if (clibseg->li_used == NROUT) {
		if (++clibseg == &libseg[NSEG])
			error(1, "too many files loaded from libraries");
		clibseg->li_first = (off_t *)malloc(NROUT * sizeof (off_t));
		if (clibseg->li_first == 0)
			error(1, "ran out of memory (nextlibp)");
	}
	clibseg->li_first[clibseg->li_used++] = val;
	if (val != -1 && Mflag)
		fprintf(stderr,"\t%s\n", archdr.ar_name);
}

/*
 * One pass over an archive with a table of contents.
 * Remember the number of symbols currently defined,
 * then call step on members which look promising (i.e.
 * that define a symbol which is currently externally undefined).
 * Indicate to our caller whether this process netted any more symbols.
 */
ldrand()
{
	register struct nlist *sp, **hp;
	register struct ranlib *tp, *tplast;
	off_t loc;
	int nsymt = symx(nextsym);

	tplast = &tab[tnum-1];
	for (tp = tab; tp <= tplast; tp++) {
		/* GB - missing indirection below */
		if (*(hp = slookup(tp->ran_un.ran_name)) == 0)
			continue;
		sp = *hp;
		if (sp->n_type != N_EXT+N_UNDF)
			continue;
		step(tp->ran_off);
		loc = tp->ran_off;
		while (tp < tplast && (tp+1)->ran_off == loc)
			tp++;
	}
	return (symx(nextsym) != nsymt);
}

/*
 * Examine a single file or archive member on pass 1.
 */
load1(libflg, loc)
	off_t loc;
{
	register struct nlist *sp;
	struct nlist *savnext;
	int ndef, nlocal, type, size, nsymt;
	register int i;
	off_t maxoff;
	struct stat stb;

	readhdr(loc);
	if (filhdr.a_syms == 0) {
		if (filhdr.a_text+filhdr.a_data == 0)
			return (0);
		error(1, "no namelist");
	}
	if (libflg)
		maxoff = atol(archdr.ar_size);
	else {
		fstat(infil, &stb);
		maxoff = stb.st_size;
	}
	if (N_STROFF(filhdr) + sizeof (off_t) >= maxoff)
		error(1, "too small (old format .o?)");
	ctrel = tsize; cdrel += dsize; cbrel += bsize;
	ndef = 0;
	nlocal = sizeof(cursym);
	savnext = nextsym;
	loc += N_SYMOFF(filhdr);
	dseek(&text, loc, filhdr.a_syms);
	dseek(&reloc, loc + filhdr.a_syms, sizeof(off_t));
	mget(&size, sizeof (size), &reloc);
	stringsize = size;
	dseek(&reloc, loc + filhdr.a_syms+sizeof (off_t), size-sizeof (off_t));
	curstr = (char *)malloc(size);
	if (curstr == NULL)
		error(1, "no space for string table");
	mget(curstr+sizeof(off_t), size-sizeof(off_t), &reloc);
	while (text.size > 0) {
		mget((char *)&cursym, sizeof(struct nlist), &text);
		if (cursym.n_un.n_strx) {
			if (cursym.n_un.n_strx<sizeof(size) ||
			    cursym.n_un.n_strx>=size)
				error(1, "bad string table index (pass 1)");
			cursym.n_un.n_name = curstr + cursym.n_un.n_strx;
		}
		type = cursym.n_type;
		if ((type&N_EXT)==0) {
			/* GB */
			if ((Xflag==0)  ||
			    (type & N_STAB) ||
				(cursym.n_un.n_name[0]!='.')) 
				nlocal += sizeof cursym;
			continue;
		}
		symreloc();
		if (enter(lookup()))
			continue;
		if ((sp = lastsym)->n_type != N_EXT+N_UNDF)
			continue;
		if (cursym.n_type == N_EXT+N_UNDF) {
			if (cursym.n_value > sp->n_value)
				sp->n_value = cursym.n_value;
			continue;
		}
		if (sp->n_value != 0 && cursym.n_type == N_EXT+N_TEXT)
			continue;
		ndef++;
		sp->n_type = cursym.n_type;
		sp->n_value = cursym.n_value;
	}
	if (libflg==0 || ndef) {
		tsize += filhdr.a_text;
		dsize += round(filhdr.a_data, sizeof (long));
		bsize += round(filhdr.a_bss, sizeof (long));
		ssize += nlocal;
		trsize += filhdr.a_trsize;
		drsize += filhdr.a_drsize;
		if (funding)
			textbase = (*slookup("_end"))->n_value;
		nsymt = symx(nextsym);
		for (i = symx(savnext); i < nsymt; i++) {
			sp = xsym(i);
			sp->n_un.n_name = savestr(sp->n_un.n_name);
		}
		free(curstr);
		return (1);
	}
#ifdef DEBUG
	else fprintf(stderr,"no symbols defined for this member\n");
#endif
	/*
	 * No symbols defined by this library member.
	 * Rip out the hash table entries and reset the symbol table.
	 */
	symfree(savnext);
	free(curstr);
	return(0);
}

middle()
{
	register struct nlist *sp;
	long csize, t, corigin, ocsize;
	int nund, rnd;
	char s;
	register int i;
	int nsymt;

	torigin = 0; 
	dorigin = 0; 
	borigin = 0;

	p_etext = *slookup("_etext");
	p_edata = *slookup("_edata");
	p_end = *slookup("_end");
	p_bstart = *slookup("_bstart");
	/*
	 * If there are any undefined symbols, save the relocation bits.
	 */
	nsymt = symx(nextsym);
	if (rflag==0) {
		for (i = 0; i < nsymt; i++) {
			sp = xsym(i);
			if (sp->n_type==N_EXT+N_UNDF && sp->n_value==0 &&
			    sp!=p_end && sp!=p_edata && 
			    sp != p_bstart && sp!=p_etext) {
				rflag++;
				dflag = 0;
				break;
			}
		}
	}
	if (rflag) 
		sflag = zflag = 0;
	/*
	 * Assign common locations.
	 */
	csize = 0;
	if (!Aflag)
		addsym = symseg[0].sy_first;
	database = round(tsize+textbase,
	    (nflag||zflag? pagesize : sizeof (long)));
	database += hsize;
	if (dflag || rflag==0) {
		ldrsym(p_etext, tsize, N_EXT+N_TEXT);
		ldrsym(p_edata, dsize, N_EXT+N_DATA);
		ldrsym(p_end, bsize, N_EXT+N_BSS);
		ldrsym(p_bstart, 0, N_EXT+N_COMM);
		for (i = symx(addsym); i < nsymt; i++) {
			sp = xsym(i);
			if ((s=sp->n_type)==N_EXT+N_UNDF &&
			    (t = sp->n_value)!=0) {
				if (t >= sizeof (double))
					rnd = sizeof (double);
				else if (t >= sizeof (long))
					rnd = sizeof (long);
				else
					rnd = sizeof (short);
				csize = round(csize, rnd);
				sp->n_value = csize;
				sp->n_type = N_EXT+N_COMM;
				ocsize = csize;	
				csize += t;
			}
			if (s&N_EXT && (s&N_TYPE)==N_UNDF && s&N_STAB) {
				sp->n_value = ocsize;
				sp->n_type = (s&N_STAB) | (N_EXT+N_COMM);
			}
		}
	}
	/*
	 * Now set symbols to their final value
	 */
	csize = round(csize, sizeof (long));
	torigin = textbase;
#ifdef PROMSTUFF
	if (LDorigin > dorigin) dorigin = LDorigin;
	else if (LDorigin) error((-1),"user-specified data origin may overlap text");
	else dorigin = database;
	if ( (LCorigin & 0xf0000000) == (dorigin & 0xf0000000 ) )
	{
		if (LCorigin > (dorigin + dsize)) corigin = LCorigin;
		else if (LCorigin){
			error((-1),"user-specified common origin may overlap data");
		}
		else corigin = dorigin + dsize;
	}
	else corigin = LCorigin;
#else
 	dorigin = database;
 	corigin = dorigin + dsize;
#endif
	borigin = corigin + csize;
	nund = 0;
	nsymt = symx(nextsym);
	for (i = symx(addsym); i<nsymt; i++) {
		sp = xsym(i);
		switch (sp->n_type & (N_TYPE+N_EXT)) {

		case N_EXT+N_UNDF:
			if (arflag == 0)
				errlev |= 01;
			if ((arflag==0 || dflag) && sp->n_value==0) {
				if (sp==p_end || sp==p_etext || 
				    sp == p_bstart || sp==p_edata)
					continue;
				if (nund==0)
					fprintf(stderr,"Undefined:\n");
				nund++;
				fprintf(stderr,"%s\n", sp->n_un.n_name);
			}
			continue;
		case N_EXT+N_ABS:
		default:
			continue;
		case N_EXT+N_TEXT:
			sp->n_value += torigin;
			continue;
		case N_EXT+N_DATA:
			sp->n_value += dorigin;
			continue;
		case N_EXT+N_BSS:
			sp->n_value += borigin;
			continue;
		case N_EXT+N_COMM:
			sp->n_type = (sp->n_type & N_STAB) | (N_EXT+N_BSS);
			sp->n_value += corigin;
			continue;
		}
	}
	if (sflag || xflag)
		ssize = 0;
	bsize += csize;
	nsym = ssize / (sizeof cursym);
	if (Aflag) {
		fixspec(p_etext,torigin);
		fixspec(p_edata,dorigin);
		fixspec(p_end,borigin);
		fixspec(p_bstart,borigin);
	}
}

fixspec(sym,offset)
	struct nlist *sym;
	long offset;
{

	if(symx(sym) < symx(addsym) && sym!=0)
		sym->n_value += offset;
}

ldrsym(sp, val, type)
	register struct nlist *sp;
	long val;
{

	if (sp == 0)
		return;
	if ((sp->n_type != N_EXT+N_UNDF || sp->n_value) && !Aflag) {
		fprintf(stderr,"%s: ", sp->n_un.n_name);
		error(0, "user attempt to redfine loader-defined symbol");
		return;
	}
	sp->n_type = type;
	sp->n_value = val;
}

off_t	wroff;
struct	biobuf toutb;

setupout()
{
	int bss;
	extern char *sys_errlist[];
	extern int errno;

	ofilemode = 0777 & ~umask(0);
	biofd = creat(ofilename, 0666 & ofilemode);
	if (biofd < 0) {
		filname = ofilename;		/* kludge */
		archdr.ar_name[0] = 0;		/* kludge */
		error(1, sys_errlist[errno]);	/* kludge */
	} else {
		struct stat mybuf;		/* kls kludge */
		fstat(biofd, &mybuf);		/* suppose file exists, wrong*/
		if(mybuf.st_mode & 0111) {	/* mode, ld fails? */
			chmod(ofilename, mybuf.st_mode & 0666);
			ofilemode = mybuf.st_mode;
		}
	}
	tout = &toutb;
	bopen(tout, 0);
	filhdr.a_magic = nflag ? NMAGIC : (zflag ? ZMAGIC : OMAGIC);
	filhdr.a_text = nflag ? tsize :
	    round(tsize, zflag ? pagesize : sizeof (long));
	filhdr.a_data = zflag ? round(dsize, pagesize) : dsize;
	bss = bsize - (filhdr.a_data - dsize);
	if (bss < 0)
		bss = 0;
	filhdr.a_bss = bss;
	filhdr.a_trsize = trsize;
	filhdr.a_drsize = drsize;
	filhdr.a_syms = sflag? 0: (ssize + (sizeof cursym)*symx(nextsym));
	if (entrypt) {
		if (entrypt->n_type!=N_EXT+N_TEXT)
			error(0, "entry point not in text");
		else
			filhdr.a_entry = entrypt->n_value;
	} else
		/* GB - not zero, textbase instead */
		filhdr.a_entry = textbase;
	filhdr.a_trsize = (rflag ? trsize:0);
	filhdr.a_drsize = (rflag ? drsize:0);
#ifdef DEBUG
fprintf(stderr,"bwriting filhdr\n");
#endif
	bwrite((char *)&filhdr, sizeof (filhdr), tout);
	if (zflag) {
		bflush1(tout);
		biobufs = 0;
		bopen(tout, pagesize);
	}
	wroff = N_TXTOFF(filhdr) + filhdr.a_text;
	outb(&dout, filhdr.a_data);
	if (rflag) {
		outb(&trout, filhdr.a_trsize);
		outb(&drout, filhdr.a_drsize);
	}
	if (sflag==0 || xflag==0) {
		outb(&sout, filhdr.a_syms);
		wroff += sizeof (offset);
		outb(&strout, 0);
	}
}

outb(bp, inc)
	register struct biobuf **bp;
{

	*bp = (struct biobuf *)malloc(sizeof (struct biobuf));
	if (*bp == 0)
		error(1, "ran out of memory (outb)");
	bopen(*bp, wroff);
	wroff += inc;
}

load2arg(acp)
char *acp;
{
	register char *cp;
	int kind;
	off_t loc;

	cp = acp;
#ifdef LIBPATH_DEBUG
	fprintf(stderr,"load1: getfile of %s\n",cp);
	kind = getfile(cp);
	fprintf(stderr,"load1: getfile returned %s as kind %d\n",cp,kind);
	if (kind == 0) {
#else
	if (getfile(cp) == 0) {
#endif
		while (*cp)
			cp++;
		while (cp >= acp && *--cp != '/');
		mkfsym(++cp);
		load2(0L);
	} else {	/* scan archive members referenced */
		for (;;) {
			if (clibseg->li_used2 == clibseg->li_used) {
				if (clibseg->li_used < NROUT)
					error(1, "libseg botch");
				clibseg++;
			}
			loc = clibseg->li_first[clibseg->li_used2++];
			if (loc == -1)
				break;
			dseek(&text, loc, (long)sizeof(archdr));
			getarhdr();
			mkfsym(archdr.ar_name);
			load2(loc + (long)sizeof(archdr));
		}
	}
	close(infil);
}

load2(loc)
long loc;
{
	int size;
	register struct nlist *sp;
	register struct local *lp;
	register int symno, i;
	int type;

	readhdr(loc);
	cpdbxinfo(loc);
	if (!funding) {
		ctrel = torigin;
		cdrel += dorigin;
		cbrel += borigin;
	}
	/*
	 * Reread the symbol table, recording the numbering
	 * of symbols for fixing external references.
	 */
	for (i = 0; i < LHSIZ; i++)
		lochash[i] = 0;
	clocseg = locseg;
	clocseg->lo_used = 0;
	symno = -1;
	loc += N_TXTOFF(filhdr);
	dseek(&text, loc+filhdr.a_text+filhdr.a_data+
		filhdr.a_trsize+filhdr.a_drsize+filhdr.a_syms, sizeof(off_t));
	mget(&size, sizeof(size), &text);
	dseek(&text, loc+filhdr.a_text+filhdr.a_data+
		filhdr.a_trsize+filhdr.a_drsize+filhdr.a_syms+sizeof(off_t),
		size - sizeof(off_t));
	curstr = (char *)malloc(size);
	if (curstr == NULL)
		error(1, "out of space reading string table (pass 2)");
	mget(curstr+sizeof(off_t), size-sizeof(off_t), &text);
	dseek(&text, loc+filhdr.a_text+filhdr.a_data+
		filhdr.a_trsize+filhdr.a_drsize, filhdr.a_syms);
	while (text.size > 0) {
		symno++;
		mget((char *)&cursym, sizeof(struct nlist), &text);
		if (cursym.n_un.n_strx) {
			if (cursym.n_un.n_strx<sizeof(size) ||
			    cursym.n_un.n_strx>=size)
				error(1, "bad string table index (pass 2)");
			cursym.n_un.n_name = curstr + cursym.n_un.n_strx;
		}
/* inline expansion of symreloc() */
		switch (cursym.n_type & 017) {

		case N_TEXT:
		case N_EXT+N_TEXT:
			cursym.n_value += ctrel;
			break;
		case N_DATA:
		case N_EXT+N_DATA:
			cursym.n_value += cdrel;
			break;
		case N_BSS:
		case N_EXT+N_BSS:
			cursym.n_value += cbrel;
			break;
		case N_EXT+N_UNDF:
			break;
		default:
			if (cursym.n_type&N_EXT)
				cursym.n_type = N_EXT+N_ABS;
		}
/* end inline expansion of symreloc() */
		type = cursym.n_type;
		if (yflag && cursym.n_un.n_name)
			for (i = 0; i < yflag; i++)
				/* fast check for 2d character! */
				if (ytab[i][1] == cursym.n_un.n_name[1] &&
				    !strcmp(ytab[i], cursym.n_un.n_name)) {
					tracesym();
					break;
				}
		if ((type&N_EXT) == 0) {
			/* GB */
			if (!sflag&&!xflag&&
			    (!Xflag||type&N_STAB||cursym.n_un.n_name[0]!='.'))
				symwrite(&cursym, sout);
			continue;
		}
		if (funding)
			continue;
		if ((sp = *lookup()) == 0)
			error(1, "internal error: symbol not found");
		if (cursym.n_type == N_EXT+N_UNDF) {
			if (clocseg->lo_used == NSYMPR) {
				if (++clocseg == &locseg[NSEG])
					error(1, "local symbol overflow");
				clocseg->lo_used = 0;
			}
			if (clocseg->lo_first == 0) {
				clocseg->lo_first = (struct local *)
				    malloc(NSYMPR * sizeof (struct local));
				if (clocseg->lo_first == 0)
					error(1, "out of memory (clocseg)");
			}
			lp = &clocseg->lo_first[clocseg->lo_used++];
			lp->l_index = symno;
			lp->l_symbol = sp;
			lp->l_link = lochash[symno % LHSIZ];
			lochash[symno % LHSIZ] = lp;
			continue;
		}
		if (cursym.n_type & N_STAB)
			continue;
		if (cursym.n_type!=sp->n_type || cursym.n_value!=sp->n_value) {
			fprintf(stderr,"%s: ", cursym.n_un.n_name);
			error(0, "multiply defined");
		}
	}
	if (funding)
		return;
	dseek(&text, loc, filhdr.a_text);
	dseek(&reloc, loc+filhdr.a_text+filhdr.a_data, filhdr.a_trsize);
	load2td(ctrel, torigin - textbase, tout, trout);
	dseek(&text, loc+filhdr.a_text, filhdr.a_data);
	dseek(&reloc, loc+filhdr.a_text+filhdr.a_data+filhdr.a_trsize,
	    filhdr.a_drsize);
	load2td(cdrel, dorigin - database, dout, drout);
	while (filhdr.a_data & (sizeof(long)-1)) {
		bputc(0, dout);
		filhdr.a_data++;
	}
	torigin += filhdr.a_text;
	dorigin += round(filhdr.a_data, sizeof (long));
	borigin += round(filhdr.a_bss, sizeof (long));
	free(curstr);
}

struct tynames {
	int	ty_value;
	char	*ty_name;
} tynames[] = {
	N_UNDF,	"undefined",
	N_ABS,	"absolute",
	N_TEXT,	"text",
	N_DATA,	"data",
	N_BSS,	"bss",
	N_COMM,	"common",
	0,	0,
};

tracesym()
{
	register struct tynames *tp;

	if (cursym.n_type & N_STAB)
		return;
	fprintf(stderr, "%s", filname);
	if (archdr.ar_name[0])
		fprintf(stderr,"(%s)", archdr.ar_name);
	fprintf(stderr,": ");
	if ((cursym.n_type&N_TYPE) == N_UNDF && cursym.n_value) {
		fprintf(stderr,"definition of common %s size %d\n",
		    cursym.n_un.n_name, cursym.n_value);
		return;
	}
	for (tp = tynames; tp->ty_name; tp++)
		if (tp->ty_value == (cursym.n_type&N_TYPE))
			break;
	fprintf(stderr,(cursym.n_type&N_TYPE) ? "definition of" : "reference to");
	if (cursym.n_type&N_EXT)
		fprintf(stderr," external");
	if (tp->ty_name)
		fprintf(stderr," %s", tp->ty_name);
	fprintf(stderr," %s\n", cursym.n_un.n_name);
}

/*
 * This routine relocates the single text or data segment argument.
 * Offsets from external symbols are resolved by adding the value
 * of the external symbols.  Non-external reference are updated to account
 * for the relative motion of the segments (ctrel, cdrel, ...).  If
 * a relocation was pc-relative, then we update it to reflect the
 * change in the positioning of the segments by adding the displacement
 * of the referenced segment and subtracting the displacement of the
 * current segment (creloc).
 *
 * If we are saving the relocation information, then we increase
 * each relocation datum address by our base position in the new segment.
 */
load2td(creloc, position, b1, b2)
	long creloc, offset;
	struct biobuf *b1, *b2;
{
	register struct nlist *sp;
	register struct local *lp;
	long tw;
	register struct relocation_info *rp, *rpend;
	struct relocation_info *relp;
	char *codep;
	register char *cp;
	int relsz, codesz;

	relsz = reloc.size;
	relp = (struct relocation_info *)malloc(relsz);
	codesz = text.size;
	codep = (char *)malloc(codesz);
	if (relp == 0 || codep == 0)
		error(1, "out of memory (load2td)");
	mget((char *)relp, relsz, &reloc);
	rpend = &relp[relsz / sizeof (struct relocation_info)];
	mget(codep, codesz, &text);
	for (rp = relp; rp < rpend; rp++) {
		cp = codep + rp->r_address;
		/*
		 * Pick up previous value at location to be relocated.
		 */
		switch (rp->r_length) {

		case 0:		/* byte */
			tw = *cp;
			break;

		case 1:		/* word */
			tw = *(short *)cp;
			break;

		case 2:		/* long */
			tw = *(long *)cp;
			break;

		default:
			error(1, "load2td botch: bad length");
		}
		/*
		 * If relative to an external which is defined,
		 * resolve to a simpler kind of reference in the
		 * result file.  If the external is undefined, just
		 * convert the symbol number to the number of the
		 * symbol in the result file and leave it undefined.
		 */
		if (rp->r_extern) {
			/*
			 * Search the hash table which maps local
			 * symbol numbers to symbol tables entries
			 * in the new a.out file.
			 */
			lp = lochash[rp->r_symbolnum % LHSIZ];
			if (lp == 0) {
				fprintf(stderr,"symbol #%d, chain %d\n",rp->r_symbolnum,
					(rp->r_symbolnum % LHSIZ));
				error(1, "local symbol botch");
			}
			while (lp->l_index != rp->r_symbolnum) {
				lp = lp->l_link;
				if (lp == 0) {
					fprintf(stderr,"symbol #%d, chain %d\n",rp->r_symbolnum,
						(rp->r_symbolnum % LHSIZ));
					error(1, "local symbol botch");
				}
			}
			sp = lp->l_symbol;
			if (sp->n_type == N_EXT+N_UNDF)
				rp->r_symbolnum = nsym+symx(sp);
			else {
				rp->r_symbolnum = sp->n_type & N_TYPE;
				tw += sp->n_value;
				rp->r_extern = 0;
			}
		} else switch (rp->r_symbolnum & N_TYPE) {
		/*
		 * Relocation is relative to the loaded position
		 * of another segment.  Update by the change in position
		 * of that segment.
		 */
		case N_TEXT:
			tw += ctrel;
			break;
		case N_DATA:
			tw += cdrel;
			break;
		case N_BSS:
			tw += cbrel;
			break;
		case N_ABS:
			break;
		default:
			error(1, "relocation format botch (symbol type))");
		}
		/*
		 * Relocation is pc relative, so decrease the relocation
		 * by the amount the current segment is displaced.
		 * (E.g if we are a relative reference to a text location
		 * from data space, we added the increase in the text address
		 * above, and subtract the increase in our (data) address
		 * here, leaving the net change the relative change in the
		 * positioning of our text and data segments.)
		 */
		if (rp->r_pcrel)
			tw -= creloc;
		/*
		 * Put the value back in the segment,
		 * while checking for overflow.
		 */
		switch (rp->r_length) {

		case 0:		/* byte */
			if (tw < -128 || tw > 127) 
				error(0, "byte displacement overflow");
			*cp = tw;
			break;
		case 1:		/* word */
			if (tw < -32768 || tw > 32767) {
				fprintf(stderr,"at 0x%x\n",rp->r_address);
				error(0, "word displacement overflow");
			}
			*(short *)cp = tw;
			break;
		case 2:		/* long */
			*(long *)cp = tw;
			break;
		}
		/*
		 * If we are saving relocation information,
		 * we must convert the address in the segment from
		 * the old .o file into an address in the segment in
		 * the new a.out, by adding the position of our
		 * segment in the new larger segment.
		 */
		if (rflag)
			rp->r_address += position;
	}
#ifdef DEBUG
fprintf(stderr,"bwriting code, 0x%x bytes\n",codesz);
#endif
	bwrite(codep, codesz, b1);
	if (rflag) {
#ifdef DEBUG
fprintf(stderr,"bwriting relocation info of 0x%x bytes\n",relsz);
#endif
		bwrite(relp, relsz, b2);
	}
	cfree((char *)relp);
	cfree(codep);
}

finishout()
{
	register int i;
	int nsymt;

#ifdef DEBUG
fprintf(stderr,"bwriting symbols\n");
#endif
	if (sflag==0) {
		nsymt = symx(nextsym);
		for (i = 0; i < nsymt; i++)
			symwrite(xsym(i), sout);
#ifdef DEBUG
fprintf(stderr,"bwriting symtab size of 0x%x bytes\n",offset);
#endif
		bwrite(&offset, sizeof offset, sout);
	}
	if (!ofilfnd) {
		unlink("a.out");
		if (link("l.out", "a.out") < 0)
			error(1, "cannot move l.out to a.out");
		ofilename = "a.out";
	}
	delarg = errlev;
	delexit();
}

mkfsym(s)
char *s;
{

	if (sflag || xflag)
		return;
	cursym.n_un.n_name = s;
	cursym.n_type = N_TEXT;
	cursym.n_value = torigin;
	symwrite(&cursym, sout);
}

getarhdr()
{
	register char *cp;

	mget((char *)&archdr, sizeof archdr, &text);
	for (cp=archdr.ar_name; cp<&archdr.ar_name[sizeof(archdr.ar_name)];)
		if (*cp++ == ' ') {
			cp[-1] = 0;
			return;
		}
}

mget(loc, n, sp)
register STREAM *sp;
register char *loc;
{
	register char *p;
	register int take;

top:
	if (n == 0)
		return;
	if (sp->size && sp->nibuf) {
		p = sp->ptr;
		take = sp->size;
		if (take > sp->nibuf)
			take = sp->nibuf;
		if (take > n)
			take = n;
		n -= take;
		sp->size -= take;
		sp->nibuf -= take;
		sp->pos += take;
		do
			*loc++ = *p++;
		while (--take > 0);
		sp->ptr = p;
		goto top;
	}
	if (n > BUFSIZ) {
		take = n - n % BLKSIZE;
		lseek(infil, (sp->bno+1)*BLKSIZE, 0);
		if (take > sp->size || read(infil, loc, take) != take)
			error(1, "premature EOF");
		loc += take;
		n -= take;
		sp->size -= take;
		sp->pos += take;
		dseek(sp, (sp->bno+1+take/BLKSIZE)*BLKSIZE, -1);
		goto top;
	}
	*loc++ = get(sp);
	--n;
	goto top;
}

symwrite(sp, bp)
	struct nlist *sp;
	struct biobuf *bp;
{
	register int len;
	register char *str;

	str = sp->n_un.n_name;
	if (str) {
		sp->n_un.n_strx = offset;
		len = strlen(str) + 1;
#ifdef DEBUG
fprintf(stderr,"bwriting symbol %s of size 0x%x bytes\n",str,len);
#endif
		bwrite(str, len, strout);
		offset += len;
	}
	bwrite(sp, sizeof (*sp), bp);
	sp->n_un.n_name = str;
}

dseek(sp, loc, s)
register STREAM *sp;
long loc, s;
{
	register PAGE *p;
	register b, o;
	int n;

	b = loc>>BLKSHIFT;
	o = loc&BLKMASK;
	if (o&01)
		error(1, "loader error; odd offset");
	--sp->pno->nuser;
	if ((p = &page[0])->bno!=b && (p = &page[1])->bno!=b)
		if (p->nuser==0 || (p = &page[0])->nuser==0) {
			if (page[0].nuser==0 && page[1].nuser==0)
				if (page[0].bno < page[1].bno)
					p = &page[0];
			p->bno = b;
			lseek(infil, loc & ~(long)BLKMASK, 0);
			if ((n = read(infil, p->buff, sizeof(p->buff))) < 0)
				n = 0;
			p->nibuf = n;
	} else
		error(1, "botch: no pages");
	++p->nuser;
	sp->bno = b;
	sp->pno = p;
	if (s != -1) {sp->size = s; sp->pos = 0;}
	sp->ptr = (char *)(p->buff + o);
	if ((sp->nibuf = p->nibuf-o) <= 0)
		sp->size = 0;
}

char
get(asp)
STREAM *asp;
{
	register STREAM *sp;

	sp = asp;
	if ((sp->nibuf -= sizeof(char)) < 0) {
		dseek(sp, ((long)(sp->bno+1)<<BLKSHIFT), (long)-1);
		sp->nibuf -= sizeof(char);
	}
	if ((sp->size -= sizeof(char)) <= 0) {
		if (sp->size < 0)
			error(1, "premature EOF");
		++fpage.nuser;
		--sp->pno->nuser;
		sp->pno = (PAGE *) &fpage;
	}
	sp->pos += sizeof(char);
	return(*sp->ptr++);
}

getfile(acp)
char *acp;
{
	register char *cp;
	char arcmag[SARMAG+1];
	struct stat stb;

	cp = acp; 
	infil = -1;
	archdr.ar_name[0] = '\0';
	filname = cp;
	if ((*cp++ =='-') && (*cp++ =='l')) {
		char *locfilename = locallibtreehackbase;
		char *usrfilename = usrlibtreehackbase;
		int len = 0;
		filname = libtreehackbase;
		if (*cp == '\0')
			cp = "a";
		while (*cp && len<libextmax ) {
			len++;
			*filname++ = *usrfilename++ = *locfilename++ = *cp++;
		}
		if (len >= libextmax)
			error(1,"library %s name too long.  Max %d chars.",
				acp,libextmax);
		*filname++ = *locfilename++ = *usrfilename++ = '.';
		*filname++ = *locfilename++ = *usrfilename++ = 'a';
		*filname++ = *locfilename++ = *usrfilename++ = '\0';
#ifdef LIBPATH_DEBUG
		fprintf(stderr,"getfile: arg = %s\nlooking in %s, %s, and %s\n",
			acp,libtree,usrlibtree,locallibtree);
		if ((filname > &libtree[LIBPATHMAX])||
				(filname < libtree))
			error(1,"libtree substitution overwrote memory");
		if ((usrfilename > &usrlibtree[LIBPATHMAX])||
				(usrfilename < usrlibtree))
			error(1,"usr libtree substitution overwrote memory");
		if ((locfilename > &locallibtree[LIBPATHMAX])||
				(locfilename < locallibtree))
			error(1,"local libtree substitution overwrote memory");
#endif
		filname = libtree;
		if ((infil = open(filname, 0)) < 0) {
			filname = usrlibtree;
			if ((infil = open(filname, 0)) < 0) 
				filname = locallibtree;
		}
	}
	if (infil == -1 && (infil = open(filname, 0)) < 0)
		error(1, "cannot open");
#ifdef LIBPATH_DEBUG
	fprintf(stderr,"getfile: found %s as %s\n",acp,filname);
#endif
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = (PAGE *) &fpage;
	fpage.nuser = 2;
	dseek(&text, 0L, SARMAG);
	if (text.size <= 0)
		error(1, "premature EOF");
	mget((char *)arcmag, SARMAG, &text);
	arcmag[SARMAG] = 0;
	if (strcmp(arcmag, ARMAG))
		return (0);
	dseek(&text, SARMAG, sizeof archdr);
	if(text.size <= 0)
		return (1);
	getarhdr();
	if (strncmp(archdr.ar_name, "__.SYMDEF", sizeof(archdr.ar_name)) != 0)
		return (1);
	fstat(infil, &stb);
	return (stb.st_mtime > atol(archdr.ar_date) ? 3 : 2);
}

struct nlist **
lookup()
{
	register int sh; 
	register struct nlist **hp;
	register char *cp, *cp1;
	register struct symseg *gp;
	register int i;

	sh = 0;
	for (cp = cursym.n_un.n_name; *cp;)
		sh = (sh<<1) + *cp++;
	sh = (sh & 0x7fffffff) % HSIZE;
	for (gp = symseg; gp < &symseg[NSEG]; gp++) {
		if (gp->sy_first == 0) {
			gp->sy_first = (struct nlist *)
			    calloc(NSYM, sizeof (struct nlist));
			gp->sy_hfirst = (struct nlist **)
			    calloc(HSIZE, sizeof (struct nlist *));
			if (gp->sy_first == 0 || gp->sy_hfirst == 0)
				error(1, "ran out of space for symbol table");
			gp->sy_last = gp->sy_first + NSYM;
			gp->sy_hlast = gp->sy_hfirst + HSIZE;
		}
		if (gp > csymseg)
			csymseg = gp;
		hp = gp->sy_hfirst + sh;
		i = 1;
		do {
			if (*hp == 0) {
				if (gp->sy_used == NSYM)
					break;
				return (hp);
			}
			cp1 = (*hp)->n_un.n_name; 
			for (cp = cursym.n_un.n_name; *cp == *cp1++;)
				if (*cp++ == 0)
					return (hp);
			hp += i;
			i += 2;
			if (hp >= gp->sy_hlast)
				hp -= HSIZE;
		} while (i < HSIZE);
		if (i > HSIZE)
			error(1, "hash table botch");
	}
	error(1, "symbol table overflow");
	/*NOTREACHED*/
}

symfree(saved)
	struct nlist *saved;
{
	register struct symseg *gp;
	register struct nlist *sp;

	for (gp = csymseg; gp >= symseg; gp--, csymseg--) {
		sp = gp->sy_first + gp->sy_used;
		if (sp == saved) {
			nextsym = sp;
			return;
		}
		for (sp--; sp >= gp->sy_first; sp--) {
			gp->sy_hfirst[sp->n_hash] = 0;
			gp->sy_used--;
			if (sp == saved) {
				nextsym = sp;
				return;
			}
		}
	}
	if (saved == 0)
		return;
	error(1, "symfree botch");
}

struct nlist **
slookup(s)
	char *s;
{

	cursym.n_un.n_name = s;
	cursym.n_type = N_EXT+N_UNDF;
	cursym.n_value = 0;
	return (lookup());
}

enter(hp)
register struct nlist **hp;
{
	register struct nlist *sp;

	if (*hp==0) {
		if (hp < csymseg->sy_hfirst || hp >= csymseg->sy_hlast)
			error(1, "enter botch");
		*hp = lastsym = sp = csymseg->sy_first + csymseg->sy_used;
		csymseg->sy_used++;
		sp->n_un.n_name = cursym.n_un.n_name;
		sp->n_type = cursym.n_type;
		sp->n_hash = hp - csymseg->sy_hfirst;
		sp->n_value = cursym.n_value;
		nextsym = lastsym + 1;
		return(1);
	} else {
		lastsym = *hp;
		return(0);
	}
}

symx(sp)
	struct nlist *sp;
{
	register struct symseg *gp;

	if (sp == 0)
		return (0);
	for (gp = csymseg; gp >= symseg; gp--)
		/* <= is sloppy so nextsym will always work */
		if (sp >= gp->sy_first && sp <= gp->sy_last)
			return ((gp - symseg) * NSYM + sp - gp->sy_first);
	error(1, "symx botch");
	/*NOTREACHED*/
}

symreloc()
{
	if(funding) return;
	switch (cursym.n_type & 017) {

	case N_TEXT:
	case N_EXT+N_TEXT:
		cursym.n_value += ctrel;
		return;

	case N_DATA:
	case N_EXT+N_DATA:
		cursym.n_value += cdrel;
		return;

	case N_BSS:
	case N_EXT+N_BSS:
		cursym.n_value += cbrel;
		return;

	case N_EXT+N_UNDF:
		return;

	default:
		if (cursym.n_type&N_EXT)
			cursym.n_type = N_EXT+N_ABS;
		return;
	}
}

error(n, s,arg1,arg2)
char *s;
{

	if (errlev==0)
		fprintf(stderr,"ld:");
	if (filname) {
		fprintf(stderr,"%s", filname);
		if (n != -1 && archdr.ar_name[0])
			fprintf(stderr,"(%s)", archdr.ar_name);
		fprintf(stderr,": ");
	}
	fprintf(stderr,s,arg1,arg2);
	putchar('\n');
	if (n == -1)
		return;
	if (n)
		delexit();
	errlev = 2;
}

readhdr(loc)
off_t loc;
{

	dseek(&text, loc, (long)sizeof(filhdr));
	mget((short *)&filhdr, sizeof(filhdr), &text);
	if (N_BADMAG(filhdr)) {
		if (filhdr.a_magic == OARMAG)
			error(1, "old archive");
		error(1, "bad magic number");
	}
	if (filhdr.a_text&01 || filhdr.a_data&01)
		error(1, "text/data size odd");
 	if (filhdr.a_magic == NMAGIC || filhdr.a_magic == ZMAGIC) {
		cdrel = -round(filhdr.a_text, pagesize);
		cbrel = cdrel - filhdr.a_data;
	} else if (filhdr.a_magic == OMAGIC) {
		cdrel = -filhdr.a_text;
		cbrel = cdrel - filhdr.a_data;
	} else
		error(1, "bad format");
}

round(v, r)
	int v;
	u_long r;
{

	r--;
	v += r;
	v &= ~(long)r;
	return(v);
}

#define	NSAVETAB	8192
char	*savetab;
int	saveleft;

char *
savestr(cp)
	register char *cp;
{
	register int len;

	len = strlen(cp) + 1;
	if (len > saveleft) {
		saveleft = NSAVETAB;
		if (len > saveleft)
			saveleft = len;
		savetab = (char *)malloc(saveleft);
		if (savetab == 0)
			error(1, "ran out of memory (savestr)");
	}
	strncpy(savetab, cp, len);
	cp = savetab;
	savetab += len;
	saveleft -= len;
	return (cp);
}

bopen(bp, off)
	struct biobuf *bp;
{

	bp->b_ptr = bp->b_buf;
	bp->b_nleft = BUFSIZ - off % BUFSIZ;
	bp->b_off = off;
	bp->b_link = biobufs;
	biobufs = bp;
}

int	bwrerror;

bwrite(p, cnt, bp)
	register char *p;
	register int cnt;
	register struct biobuf *bp;
{
	register int put;
	register char *to;

#ifdef DEBUG
fprintf(stderr,"bwrite called to write 0x%x bytes\n",cnt);
#endif

top:
	if (cnt == 0)
		return;
	if (bp->b_nleft) {
		put = bp->b_nleft;
		if (put > cnt)
			put = cnt;
		bp->b_nleft -= put;
		to = bp->b_ptr;
#ifdef	vax
		asm("movc3 r8,(r11),(r7)");
#else
		blt(to, p, put);
#endif
		bp->b_ptr += put;
		p += put;
		cnt -= put;
		goto top;
	}
	if (cnt >= BUFSIZ) {
		if (bp->b_ptr != bp->b_buf)
			bflush1(bp);
		put = cnt - cnt % BUFSIZ;
		if (boffset != bp->b_off)
			lseek(biofd, bp->b_off, 0);
#ifdef DEBUG
fprintf(stderr,"writing 0x%x bytes to biofd at byte 0x%x\n",put,bp->b_off);
#endif
		if (write(biofd, p, put) != put) {
			bwrerror = 1;
			error(1, "output write error");
		}
		bp->b_off += put;
		boffset = bp->b_off;
		p += put;
		cnt -= put;
		goto top;
	}
	bflush1(bp);
	goto top;
}

bflush()
{
	register struct biobuf *bp;

	if (bwrerror)
		return;
	for (bp = biobufs; bp; bp = bp->b_link)
		bflush1(bp);
}

bflush1(bp)
	register struct biobuf *bp;
{
	register int cnt = bp->b_ptr - bp->b_buf;

	if (cnt == 0)
		return;
	if (boffset != bp->b_off)
		lseek(biofd, bp->b_off, 0);
#ifdef DEBUG
fprintf(stderr,"bflush1 writing 0x%x bytes to biofd at byte 0x%x\n",cnt,bp->b_off);
#endif
	if (write(biofd, bp->b_buf, cnt) != cnt) {
		bwrerror = 1;
		error(1, "output write error");
	}
	bp->b_off += cnt;
	boffset = bp->b_off;
	bp->b_ptr = bp->b_buf;
	bp->b_nleft = BUFSIZ;
}

bflushc(bp, c)
	register struct biobuf *bp;
{

	bflush1(bp);
	bputc(c, bp);
}

/*
 * Symbol classes
 */

typedef enum {
    BADUSE, CONST, TYPE, VAR, ARRAY, DYNARRAY, SUBARRAY, PTRFILE, RECORD, FIELD,
    PROC, FUNC, FVAR, REF, PTR, FILET, SET, RANGE, 
    LABEL, WITHPTR, SCAL, STR, PROG, IMPROPER, VARNT,
    FPROC, FFUNC, MODULE, TAG, COMMON, EXTREF, TYPEREF
} Symclass;

typedef enum { R_CONST, R_TEMP, R_ARG, R_ADJUST } Rangetype; 

#define INREG 0
#define STK 1
#define EXT 2

typedef unsigned int Storage;
typedef unsigned long Name;
typedef unsigned long Symbol;
typedef unsigned long Node;
typedef short Boolean;
typedef unsigned long Address;

struct Symbol {
    Name name;
    long language;
    Symclass class : 8;
    Storage storage : 2;
    unsigned int level : 6;	/* for variables stored on stack only */
    Symbol type;
    Symbol chain;
	long	symnum;
    union {
	Node constval;		/* value of constant symbol */
	int offset;		/* variable address */
	long iconval;		/* integer constant value */
	double fconval;		/* floating constant value */
	int ndims;		/* no. of dimensions for dynamic/sub-arrays */
	struct {		/* field offset and size (both in bits) */
	    int offset;
	    int length;
	} field;
	struct {		/* common offset and chain; used to relocate */
	    int offset;         /* vars in global BSS */
	    Symbol chain;
	} common;
	struct {		/* range bounds */
            Rangetype lowertype : 16; 
            Rangetype uppertype : 16;  
	    long lower;
	    long upper;
	} rangev;
	struct {
	    int offset : 16;	/* offset for of function value */
	    Boolean src : 1;	/* true if there is source line info */
	    Boolean inline : 1;	/* true if no separate act. rec. */
	    Boolean intern : 1; /* internal calling sequence */
	    Boolean ismain : 1; 
	    int unused : 12;
	    Address beginaddr;	/* address of function code */
	} funcv;
	struct {		/* variant record info */
	    int size;
	    Symbol vtorec;
	    Symbol vtag;
	} varnt;
	char	* typeref;		/* type defined by "<module>:<type>" */
	Symbol extref;		/* indirect symbol for external reference */
    } symvalue;
    Symbol block;		/* symbol containing this symbol */
    Symbol next_sym;		/* hash chain */
};


#include <fcntl.h>
#include "dbx.h"
DBXHEADER	dbxhead;
DBXHEADER	outdbxhead;


int	symtf;
int	strtf;
int	lintf;
char	*symtfn = "/tmp/ldsyXXXXXX";
char	*strtfn = "/tmp/ldstXXXXXX";
char	*lintfn = "/tmp/ldlnXXXXXX";
cpdbxinfo(topoffile) 
long	topoffile;
{

	int	fd;
	int	pad;
	SRCFILE		srcfile;
	LINEHEADER		linehdr;
	char	*linetab;
	struct	exec	hdr;
	struct	stat	statbuf;
	long	stringsize;
	long	filesize;
	long	stringoff;
	char	*stringtab;
	char	*mktemp();
	long	saveoff;
	struct	Symbol	sym;
	int	j;
	int	i;

	saveoff = lseek(infil, 0, 1);
	lseek(infil, topoffile, 0);
	lseek(infil, N_STROFF(filhdr), 1);
	if (read(infil, &stringsize, sizeof(stringsize)) == -1) {
		error(1,"cannot read stringsize\n");
	}
	hdr = filhdr;
	if (topoffile == 0) {
		fstat(infil, &statbuf);
		filesize = statbuf.st_size;
	} else {
		filesize = atol(archdr.ar_size);
	}
	if ((N_STROFF(hdr) + stringsize) < filesize) {
		lseek(infil, stringsize - sizeof(stringsize), 1);
		if (!symtf) {
			symtfn = mktemp(symtfn);
			strtfn = mktemp(strtfn);
			lintfn = mktemp(lintfn);
			if ((symtf = open(symtfn, O_CREAT|O_WRONLY, 0666)) == -1)  {
				error(1, "output dbx open error on symtf\n");
			}
			if ((strtf = open(strtfn, O_CREAT|O_WRONLY, 0666)) == -1)  {
				error(1, "output dbx open erroron \n");
			}
			if ((lintf = open(lintfn, O_CREAT|O_WRONLY, 0666)) == -1)  {
				error(1, "output dbx open error\n");
			}
		}
	} else {
		return;
	}

	if (read(infil, &dbxhead, sizeof(DBXHEADER)) == -1) {
		error(1,"cannot read dbxheader\n");
	}
	for (i = 0; i < dbxhead.nsyms; i++) {
		if (read(infil, &sym, sizeof(struct Symbol)) == -1) {
			error(1,"cannot read dbx symbols\n");
		}
		sym.symnum += outdbxhead.nsyms;
		if (sym.type) {
			sym.type += outdbxhead.nsyms;
		}
		if (sym.chain) {
			sym.chain += outdbxhead.nsyms;
		}
		if (sym.block) {
			sym.block += outdbxhead.nsyms;
		}
		if (sym.name != -1) {
			sym.name += outdbxhead.stringsize;
		}
		if (sym.class == COMMON ||
			(sym.storage == EXT && sym.class == VAR)) {
			sym.symvalue.common.chain += outdbxhead.nsyms;
		}
		if (write(symtf, &sym, sizeof(struct Symbol)) == -1) {
			error(1,"cannot write dbx symbols\n");
		}
	}
	stringtab = (char *) malloc(dbxhead.stringsize);
	if (read(infil, stringtab, dbxhead.stringsize) == -1) {
		error(1,"cannot read dbx strings\n");
	}
	if (write(strtf, stringtab, dbxhead.stringsize) == -1) {
		error(1,"cannot write dbx strings\n");
	}
	free(stringtab);
	for (i = 0; i < dbxhead.nfiles; i++) {
		if (read(infil, &srcfile, sizeof(SRCFILE) ) == -1) {
			error(1,"cannot read dbx SRCFILE\n");
		}
		srcfile.filename += outdbxhead.stringsize;
		if (write(lintf, &srcfile, sizeof(SRCFILE) ) == -1) {
			error(1,"cannot write dbx SRCFILE\n");
		}
		for (j = 0; j < srcfile.nfuncs; j++) {
			if (read(infil, &linehdr, sizeof(LINEHEADER) ) == -1) {
				error(1,"cannot read dbx LINEHEADER \n");
			}
			linehdr.funcnum += outdbxhead.nsyms;

			if (write(lintf, &linehdr, sizeof(LINEHEADER) ) == -1) {
				error(1,"cannot write dbx LINEHEADER\n");
			}
			linetab = (char *) 
				malloc(linehdr.nlines * sizeof(LINEADDR));
			if (read(infil, linetab, 
				linehdr.nlines * sizeof(LINEADDR)) == -1) {
				error(1,"cannot read dbx line numbers\n");
			}
			if (write(lintf, linetab, 
				linehdr.nlines * sizeof(LINEADDR)) == -1) {
				error(1,"cannot write dbx line numbers\n");
			}
		}

	}
	saveoff = lseek(infil, saveoff, 0);
	outdbxhead.nsyms += dbxhead.nsyms;
	outdbxhead.stringsize += dbxhead.stringsize;
	outdbxhead.nfiles += dbxhead.nfiles;
}


catdbxinfo(fd)
int	fd;
{
	if (!symtf) {
		return;
	}
	close(strtf);
	close(lintf);
	close(symtf);
	if ((symtf = open(symtfn, O_RDONLY)) == -1)  {
		error(1, "output dbx open error on symtf\n");
	}
	if ((strtf = open(strtfn, O_RDONLY)) == -1)  {
		error(1, "output dbx open erroron \n");
	}
	if ((lintf = open(lintfn, O_RDONLY)) == -1)  {
		error(1, "output dbx open error\n");
	}

	if (write(fd, &outdbxhead, sizeof(DBXHEADER) ) == -1) {
		error(1,"cannot write dbx DBXHEADER\n");
	}
	cattmp(symtf, fd);
	cattmp(strtf, fd);
	cattmp(lintf, fd);
	close(strtf);
	close(lintf);
	close(symtf);
	unlink(strtfn);
	unlink(lintfn);
	unlink(symtfn);
}

cattmp(infd, outfd)
int	infd;
int	outfd;
{

	long	bytes_read;
	char	buffer[BUFSIZ];

	while ((bytes_read = read(infd, buffer, BUFSIZ)) == BUFSIZ) {
		write(outfd, buffer, BUFSIZ);
	}
	if (bytes_read) {
		write(outfd, buffer, bytes_read);
	}
}


