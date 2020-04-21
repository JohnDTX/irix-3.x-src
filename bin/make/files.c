/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	sgi
#ident	"@(#)make:files.c	1.5.1.1"
#endif

#include "defs"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <pwd.h>
#include <ar.h>
/* UNIX DEPENDENT PROCEDURES */

#ifdef	sgi
#define	sgiAR
#endif	/* sgi */
#define equaln		!strncmp

/*
* For 6.0, create a make which can understand all three archive
* formats.  This is kind of tricky, and <ar.h> isn't any help.
* Note that there is no sgetl() and sputl() on the pdp11, so
* make cannot handle anything but the one format there.
*/
char	archmem[64];		/* archive file member name to search for */
char	archname[64];		/* archive file to be opened */

int	ar_type;	/* to distiguish which archive format we have */
#define ARpdp	1
#define AR5	2
#define ARport	3

long	first_ar_mem;	/* where first archive member header is at */
long	sym_begin;	/* where the symbol lookup starts */
long	num_symbols;	/* the number of symbols available */
long	sym_size;	/* length of symbol directory file */

/*
* Defines for all the different archive formats.  See next comment
* block for justification for not using <ar.h>'s versions.
*/
#define ARpdpMAG	0177545		/* old format (short) magic number */

#define AR5MAG		"<ar>"		/* 5.0 format magic string */
#define SAR5MAG		4		/* 5.0 format magic string length */

#define ARportMAG	"!<arch>\n"	/* Port. (6.0) magic string */
#define SARportMAG	8		/* Port. (6.0) magic string length */
#define ARFportMAG	"`\n"		/* Port. (6.0) end of header string */

/*
* These are the archive file headers for the three formats.  Note
* that it really doesn't matter if these structures are defined
* here.  They are correct as of the respective archive format
* releases.  If the archive format is changed, then since backwards
* compatability is the desired behavior, a new structure is added
* to the list.
*/
struct		/* pdp11 -- old archive format */
{
	char	ar_name[14];	/* '\0' terminated */
	long	ar_date;	/* native machine bit representation */
	char	ar_uid;		/* 	"	*/
	char	ar_gid;		/* 	"	*/
	int	ar_mode;	/* 	"	*/
	long	ar_size;	/* 	"	*/
} ar_pdp;

struct		/* pdp11 a.out header */
{
	short		a_magic;
	unsigned	a_text;
	unsigned	a_data;
	unsigned	a_bss;
	unsigned	a_syms;		/* length of symbol table */
	unsigned	a_entry;
	char		a_unused;
	char		a_hitext;
	char		a_flag;
	char		a_stamp;
} arobj_pdp;

struct		/* pdp11 a.out symbol table entry */
{
	char		n_name[8];	/* null-terminated name */
	int		n_type;
	unsigned	n_value;
} ars_pdp;

struct		/* UNIX 5.0 archive header format: vax family; 3b family */
{
	char	ar_magic[SAR5MAG];	/* AR5MAG */
	char	ar_name[16];		/* ' ' terminated */
	char	ar_date[4];		/* sgetl() accessed */
	char	ar_syms[4];		/* sgetl() accessed */
} arh_5;

struct		/* UNIX 5.0 archive symbol format: vax family; 3b family */
{
	char	sym_name[8];	/* ' ' terminated */
	char	sym_ptr[4];	/* sgetl() accessed */
} ars_5;

struct		/* UNIX 5.0 archive member format: vax family; 3b family */
{
	char	arf_name[16];	/* ' ' terminated */
	char	arf_date[4];	/* sgetl() accessed */
	char	arf_uid[4];	/*	"	*/
	char	arf_gid[4];	/*	"	*/
	char	arf_mode[4];	/*	"	*/
	char	arf_size[4];	/*	"	*/
} arf_5;

struct		/* Portable (6.0) archive format: vax family; 3b family */
{
	char	ar_name[16];	/* '/' terminated */
	char	ar_date[12];	/* left-adjusted; decimal ascii; blank filled */
	char	ar_uid[6];	/*	"	*/
	char	ar_gid[6];	/*	"	*/
	char	ar_mode[8];	/* left-adjusted; octal ascii; blank filled */
	char	ar_size[10];	/* left-adjusted; decimal ascii; blank filled */
	char	ar_fmag[2];	/* special end-of-header string (ARFportMAG) */
} ar_port;


#if 0		/* no longer used! */
	/*
	 *	New common object version of files.c
	 */
	char		archmem[64];
	char		archname[64];		/* name of archive library */
	struct ar_hdr	head;		/* archive file header */
	struct ar_sym	symbol;		/* archive file symbol table entry */
	struct arf_hdr	fhead;		/* archive file object file header */
#endif

TIMETYPE	afilescan();
TIMETYPE	entryscan();
TIMETYPE	pdpentrys();
FILE		*arfd;
char		BADAR[] = "BAD ARCHIVE";


TIMETYPE
exists(pname)
NAMEBLOCK pname;
{
	register CHARSTAR s;
	struct stat buf;
	TIMETYPE lookarch();
	CHARSTAR filename;

	filename = pname->namep;

	if(any(filename, LPAREN))
		return(lookarch(filename));

	if(stat(filename,&buf) < 0) 
	{
		s = findfl(filename);
		if(s != (CHARSTAR )-1)
		{
			pname->alias = copys(s);
			if(stat(pname->alias, &buf) == 0)
				return(buf.st_mtime);
		}
		return(0);
	}
	else
		return(buf.st_mtime);
}


TIMETYPE
prestime()
{
	TIMETYPE t;
	time(&t);
	return(t);
}



#ifdef	sgi
FSTATIC char n15[MAXNAMLEN+1];
FSTATIC CHARSTAR n15end = &n15[MAXNAMLEN];
#else	/* sgi */
FSTATIC char n15[15];
FSTATIC CHARSTAR n15end = &n15[14];
#endif	/* sgi */



DEPBLOCK
srchdir(pat, mkchain, nextdbl)
register CHARSTAR pat;		/* pattern to be matched in directory */
int mkchain;			/* nonzero if results to be remembered */
DEPBLOCK nextdbl;		/* final value for chain */
{
#ifdef	sgi
	DIR *dirf;
#else	/* sgi */
	FILE * dirf;
#endif	/* sgi */
	int i, nread;
	CHARSTAR dirname, dirpref, endir, filepat, p;
#ifdef	sgi
	char temp[MAXNAMLEN];
	char fullname[MAXNAMLEN];
#else	/* sgi */
	char temp[100];
	char fullname[100];
	CHARSTAR p1, p2;
#endif	/* sgi */
	NAMEBLOCK q;
	DEPBLOCK thisdbl;
	OPENDIR od;
	int dirofl = 0;
	static opendirs = 0;
	PATTERN patp;

#ifdef	sgi
	register struct dirent *dp;
	char *dirpath;
	extern char *makepath;
	extern CHARSTAR execat();
	char newname[MAXNAMLEN];
	int canfail;
#else	/* sgi */
	struct direct entry[32];
#endif	/* sgi */


	thisdbl = 0;

	if(mkchain == NO)
		for(patp=firstpat ; patp!=0 ; patp = patp->nextpattern)
			if(equal(pat,patp->patval))
				return(0);

	patp = ALLOC(pattern);
	patp->nextpattern = firstpat;
	firstpat = patp;
	patp->patval = copys(pat);

	endir = 0;

	for(p=pat; *p!=CNULL; ++p)
		if(*p==SLASH)
			endir = p;

	if(endir==0)
	{
#ifdef	sgi
		if (makepath)
			dirpath = makepath;
		else {
			dirpath = NULL;
			dirname = ".";
			dirpref = "";
		}
#else	/* sgi */
		dirname = ".";
		dirpref = "";
#endif	/* sgi */
		filepat = pat;
	}
	else
	{
		*endir = CNULL;
		dirpref = concat(pat, "/", temp);
		filepat = endir+1;
		dirname = temp;
#ifdef	sgi
		dirpath = NULL;
#endif	/* sgi */
	}

#ifdef	sgi
next_path:
	/*
	 * Construct directory name based on head component of dirpath
	 */
	canfail = 0;
	if (dirpath) {
		dirpath = execat(dirpath, "", newname);
		dirname = &newname[0];
		/*
		 * Get rid of "." or "./" as the directory name.
		 */
		if ((dirname[0] == '.') &&
		    ((dirname[1] == CNULL) ||
		     (dirname[1] == '/' && dirname[2] == CNULL))) {
			dirname[0] = '.';
			dirname[1] = 0;
			dirpref = "";
		} else
			dirpref = dirname;
if (IS_ON(DBUG))
	printf("srchdir: dirpref=\"%s\"\n", dirpref);
		canfail = 1;
	}
#endif	/* sgi */
	dirf = NULL;

	for(od=firstod ; od!=0; od = od->nextopendir)
		if(equal(dirname, od->dirn))
		{
			dirf = od->dirfc;
#ifdef	sgi
			rewinddir(dirf); /* start over at the beginning  */
#else	/* sgi */
			fseek(dirf,0L,0); /* start over at the beginning  */
#endif	/* sgi */
			break;
		}

	if(dirf == NULL)
	{
#ifdef	sgi
		dirf = opendir(dirname);
		if (dirf == NULL) {
			/*
			 * Make directories which don't exist in the MAKEPATH
			 * silently be ignored.  Also make sure that we don't
			 * stuff the opendir list full of non-usable
			 * directories by NOT saving it.
			 */
if (IS_ON(DBUG))
	printf("srchdir: couldn't open directory \"%s\"\n", dirname);
			if (canfail && dirpath)
				goto next_path;
			/* if path is empty, stop. */
			return (0);
		}
#else
		dirf = fopen(dirname, "r");
#endif
		if(++opendirs < MAXODIR)
		{
			od = ALLOC(opendir);
			od->nextopendir = firstod;
			firstod = od;
			od->dirfc = dirf;
			od->dirn = copys(dirname);
		}
		else
			dirofl = 1;
	}

	if(dirf == NULL)
	{
		fprintf(stderr, "Directory %s: ", dirname);
		fatal("Cannot open");
	}

#ifdef	sgi
	else
	while ((dp = readdir(dirf)) != NULL) {
		if (dp->d_ino!= 0) {
			if (amatch(dp->d_name,filepat)) {
if (IS_ON(DBUG))
	printf("srchdir: found \"%s\" using pattern \"%s\"\n",
			 dp->d_name, pat);
				concat(dirpref,dp->d_name,fullname);
				if (canfail) {
					/*
					 * If we are searching for a relative
					 * filename, then cache the relative
					 * name, and set the alias to the
					 * place on MAKEPATH that we found it.
					 * Cache name without path prefix.
					 * Later searches of the hash table
					 * will find the un-prefixed name,
					 * but use the alias.
					 */
					if ((q=srchname(dp->d_name)) == 0)
						q = makename(copys(dp->d_name));
					q->alias = copys(fullname);
				} else {
					/*
					 * If file name is positionally fixed,
					 * then look for it directly.
					 */
					if ((q=srchname(fullname)) == 0)
						q = makename(copys(fullname));
				}
				if(mkchain) {
					thisdbl = ALLOC(depblock);
					thisdbl->nextdep = nextdbl;
					thisdbl->depname = q;
					nextdbl = thisdbl;
				}
			}
		}
	}
	/*
	 * Try looking in next component of MAKEPATH.
	 */
	if (thisdbl == NULL && dirpath)
		goto next_path;
#else	/* sgi */
	else	do
		{
			nread = fread(entry,sizeof(entry[0]),32,dirf) ;
			for(i=0; i<nread; ++i)
				if(entry[i].d_ino!= 0)
				{
					p1 = entry[i].d_name;
					p2 = n15;
					while( (p2<n15end) &&
					  (*p2++ = *p1++)!=CNULL );
					if( amatch(n15,filepat) )
					{
						concat(dirpref,n15,fullname);
						if( (q=srchname(fullname)) ==0)
							q = makename(copys(fullname));
						if(mkchain)
						{
							thisdbl = ALLOC(depblock);
							thisdbl->nextdep = nextdbl;
							thisdbl->depname = q;
							nextdbl = thisdbl;
						}
					}
				}
		} while(nread==32);
#endif	/* sgi */

	if(endir != 0)
		*endir = SLASH;
	if(dirofl)
#ifdef	sgi
		fclose(dirf);
#else	/* sgi */
		closedir(dirf);
#endif	/* sgi */

	return(thisdbl);
}

/* stolen from glob through find */

amatch(s, p)
CHARSTAR s, p;
{
	register int cc, scc, k;
	int c, lc;

	scc = *s;
	lc = 077777;
	switch (c = *p)
	{

	case LSQUAR:
		k = 0;
		while (cc = *++p)
		{
			switch (cc)
			{

			case RSQUAR:
				if (k)
					return(amatch(++s, ++p));
				else
					return(0);

			case MINUS:
				k |= lc <= scc & scc <= (cc=p[1]);
			}
			if(scc==(lc=cc))
				k++;
		}
		return(0);

	case QUESTN:
	caseq:
		if(scc)
			return(amatch(++s, ++p));
		return(0);
	case STAR:
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if(c==scc)
		goto caseq;
	return(0);
}

umatch(s, p)
register CHARSTAR s, p;
{
	if(*p==0)
		return(1);
	while(*s)
		if(amatch(s++,p))
			return(1);
	return(0);
}


#ifndef	sgiAR
/* look inside archives for notations a(b) and a((b))
	a(b)	is file member   b   in archive a
	a((b))	is entry point   b  in object archive a
*/


TIMETYPE
lookarch(filename)
register CHARSTAR filename;
{
	register int i;
	CHARSTAR p, q;
	char s[15];
	int nc, objarch;
	TIMETYPE la();

	for(p = filename; *p!= LPAREN ; ++p);
	i = p - filename;
	strncpy(archname, filename, i);
	archname[i] = CNULL;
	if(archname[0] == CNULL)
		fatal1("Null archive name `%s'", filename);
	p++;
	if(*p == LPAREN)
	{
		objarch = YES;
		++p;
		if((q = strchr(p, RPAREN)) == NULL)
			q = p + strlen(p);
		strncpy(s,p,q-p);
		s[q-p] = CNULL;
	}
	else
	{
		objarch = NO;
		if((q = strchr(p, RPAREN)) == NULL)
			q = p + strlen(p);
		i = q - p;
		strncpy(archmem, p, i);
		archmem[i] = CNULL;
		nc = 14;
		if(archmem[0] == CNULL)
			fatal1("Null archive member name `%s'", filename);
		if(q = strrchr(archmem, SLASH))
			++q;
		else
			q = archmem;
		strncpy(s, q, nc);
		s[nc] = CNULL;
	}
	return(la(s, objarch));
}
TIMETYPE
la(am,flag)
register char *am;
register int flag;
{
	TIMETYPE date = 0L;

	if(openarch(archname) == -1)
		return(0L);
	if(flag)
		date = entryscan(am);	/* fatals if cannot find entry */
	else
		date = afilescan(am);
	clarch();
	return(date);
}

TIMETYPE
afilescan(name)		/* return date for named archive member file */
char *name;
{
	int	len = strlen(name);
	long	ptr;

	if (fseek(arfd, first_ar_mem, 0) != 0)
	{
	seek_error:;
		fatal1("Seek error on archive file %s", archname);
	}
	/*
	* Hunt for the named file in each different type of
	* archive format.
	*/
	switch (ar_type)
	{
	case ARpdp:
		for (;;)
		{
			if (fread((char *)&ar_pdp,
				sizeof(ar_pdp), 1, arfd) != 1)
			{
				if (feof(arfd))
					return (0L);
				break;
			}
			if (equaln(ar_pdp.ar_name, name, len))
				return (ar_pdp.ar_date);
			ptr = ar_pdp.ar_size;
			ptr += (ptr & 01);
			if (fseek(arfd, ptr, 1) != 0)
				goto seek_error;
		}
		break;
#ifndef pdp11
	case AR5:
		for (;;)
		{
			if (fread((char *)&arf_5, sizeof(arf_5), 1, arfd) != 1)
			{
				if (feof(arfd))
					return (0L);
				break;
			}
			if (equaln(arf_5.arf_name, name, len))
				return (sgetl(arf_5.arf_date));
			ptr = sgetl(arf_5.arf_size);
			ptr += (ptr & 01);
			if (fseek(arfd, ptr, 1) != 0)
				goto seek_error;
		}
		break;
	case ARport:
		for (;;)
		{
			if (fread((char *)&ar_port, sizeof(ar_port), 1,
				arfd) != 1 || !equaln(ar_port.ar_fmag,
				ARFportMAG, sizeof(ar_port.ar_fmag)))
			{
				if (feof(arfd))
					return (0L);
				break;
			}
			if (   equaln(ar_port.ar_name, name, len)
			    && (   len == sizeof(ar_port.ar_name)
				|| ar_port.ar_name[len] == '/'
				|| ar_port.ar_name[len] == ' '
				|| ar_port.ar_name[len] == '\0'
			       )
			   )
			{
				long date;

				if (sscanf(ar_port.ar_date, "%ld", &date) != 1)
				{
					fatal1("Bad date field for %.14s in %s",
						name, archname);
				}
				return (date);
			}
			if (sscanf(ar_port.ar_size, "%ld", &ptr) != 1)
			{
				fatal1("Bad size field for %.14s in archive %s",
					name, archname);
			}
			ptr += (ptr & 01);
			if (fseek(arfd, ptr, 1) != 0)
				goto seek_error;
		}
		break;
#endif
	}
	/*
	* Only here if fread() [or equaln()] failed and not at EOF
	*/
	fatal1("Read error on archive %s", archname);
	/*NOTREACHED*/
#if 0
	long date;
	long nsyms;
	long ptr;

	nsyms = sgetl(head.ar_syms);
	if(fseek(arfd, (long)( nsyms*sizeof(symbol)+sizeof(head) ), 0) == -1)
		fatal("Seek error on archive file %s", archname);
	for(;;)
	{
		if(fread(&fhead, sizeof(fhead), 1, arfd) != 1)
			if(feof(arfd))
				break;
			else
				fatal("Read error on archive %s", archname);
		if(equaln(fhead.arf_name, name, 14))
		{
			date = sgetl(fhead.arf_date);
			return(date);
		}
		ptr = sgetl(fhead.arf_size);
		ptr = (ptr+1)&(~1);
		fseek(arfd, ptr, 1);
	}
	return( 0L );
#endif
}

TIMETYPE
entryscan(name)		/* return date of member containing global var named */
char *name;
{
	/*
	* Hunt through each different archive format for the named
	* symbol.  Note that the old archive format does not support
	* this convention since there is no symbol directory to
	* scan through for all defined global variables.
	*/
	if (ar_type == ARpdp)
		return (pdpentrys(name));
	if (sym_begin == 0L || num_symbols == 0L)
	{
	no_such_sym:;
		fatal1("Cannot find symbol %s in archive %s", name, archname);
	}
	if (fseek(arfd, sym_begin, 0) != 0)
	{
	seek_error:;
		fatal1("Seek error on archive file %s", archname);
	}
#ifndef pdp11
	if (ar_type == AR5)
	{
		register int i;
		int len = strlen(name);

		if (len > 8)
			len = 8;
		for (i = 0; i < num_symbols; i++)
		{
			if (fread((char *)&ars_5, sizeof(ars_5), 1, arfd) != 1)
			{
			read_error:;
				fatal1("Read error on archive %s", archname);
			}
			if (equaln(ars_5.sym_name, name, len))
			{
				if (fseek(arfd, sgetl(ars_5.sym_ptr), 0) != 0)
					goto seek_error;
				if (fread((char *)&arf_5,
					sizeof(arf_5), 1, arfd) != 1)
				{
					goto read_error;
				}
				
				/* replace symbol name w/ member name */
				strncpy(archmem, arf_5.arf_name,
					sizeof(arf_5.arf_name));

				return (sgetl(arf_5.arf_date));
			}
		}
	}
	else	/* ar_type == ARport */
	{
		extern char *malloc();
		int strtablen;
		register char *offs;	/* offsets table */
		register char *syms;	/* string table */
		register char *strend;	/* end of string table */
		char *strbeg;

		/*
		* Format of the symbol directory for this format is
		* as follows:	[sputl()d number_of_symbols]
		*		[sputl()d first_symbol_offset]
		*			...
		*		[sputl()d number_of_symbols'_offset]
		*		[null_terminated_string_table_of_symbols]
		*/
		if ((offs = malloc(num_symbols * sizeof(long))) == NULL)
		{
			fatal1("Cannot alloc offsets table for archive %s",
				archname);
		}
		if (fread(offs, sizeof(long), num_symbols, arfd) != num_symbols)
			goto read_error;
		strtablen = sym_size - ((num_symbols + 1) * sizeof(long));
		if ((syms = malloc(strtablen)) == NULL)
		{
			fatal1("Cannot alloc string table for archive %s",
				archname);
		}
		if (fread(syms, sizeof(char), strtablen, arfd) != strtablen)
			goto read_error;
		strend = &syms[strtablen];
		strbeg = syms;
		while (syms < strend)
		{
			if (equal(syms, name))
			{
				long ptr, date;
				register char *ap, *hp;

				ptr = sgetl(offs);
				if (fseek(arfd, ptr, 0) != 0)
					goto seek_error;
				if (fread((char *)&ar_port, sizeof(ar_port), 1,
					arfd) != 1 || !equaln(ar_port.ar_fmag,
					ARFportMAG, sizeof(ar_port.ar_fmag)))
				{
					goto read_error;
				}
				if (sscanf(ar_port.ar_date, "%ld", &date) != 1)
				{
					fatal1("Bad date for %.14s, archive %s",
						ar_port.ar_name, archname);
				}

				/* replace symbol name w/ member name */
				ap = archmem;
				hp = ar_port.ar_name;
				while ( *hp && *hp != '/'
					&& ap < archmem + sizeof(archmem))
					*ap++ = *hp++;

				free(strbeg);
				return (date);
			}
			syms += strlen(syms) + 1;
			offs += sizeof(long);
		}
		free(strbeg);
	}
#endif
	goto no_such_sym;
#if 0
	register int i;
	long date;
	long ptr;
	long nsyms;

	nsyms = sgetl(head.ar_syms);
	for(i = 0; i < nsyms; i++)
	{
		if(fread(&symbol, sizeof(symbol), 1, arfd) != 1)
badread:
			fatal("Read error on archive %s", archname);
		if(equaln(symbol.sym_name, name, 8))
		{
			ptr = sgetl(symbol.sym_ptr);
			if(fseek(arfd, ptr, 0) == -1)
				fatal("Seek error on archive file %s",archname);
			if(fread(fhead, sizeof(fhead), 1, arfd) != 1)
				goto badread;
			date = sgetl(fhead.arf_date);
			return(date);
		}
	}
	fatal("Cannot find symbol %s in archive %s", name, archname);
#endif
}

TIMETYPE
pdpentrys(name)
	char *name;
{
	long	skip;
	long	last;
	int	len;
	register int i;

#ifndef pdp11
	fatal("Cannot do global variable search in pdp11 or old object file.");
#endif
	len = strlen(name);
	if (len > 8)
		len = 8;
	/*
	* Look through archive, an object file entry at a time.  For each
	* object file, jump to its symbol table and check each external
	* symbol for a match.  If found, return the date of the module
	* containing the symbol.
	*/
	if (fseek(arfd, sizeof(short), 0) != 0)
	{
	seek_error:;
		fatal1("Cannot seek on archive file %s", archname);
	}
	while (fread((char *)&ar_pdp, sizeof(ar_pdp), 1, arfd) == 1)
	{
		last = ftell(arfd);
		if (ar_pdp.ar_size < sizeof(arobj_pdp) ||
			fread((char *)&arobj_pdp, sizeof(arobj_pdp), 1, arfd)
			!= 1 ||
			(arobj_pdp.a_magic != 0401 &&	/* A_MAGIC0 */
			arobj_pdp.a_magic != 0407 &&	/* A_MAGIC1 */
			arobj_pdp.a_magic != 0410 &&	/* A_MAGIC2 */
			arobj_pdp.a_magic != 0411 &&	/* A_MAGIC3 */
			arobj_pdp.a_magic != 0405 &&	/* A_MAGIC4 */
			arobj_pdp.a_magic != 0437)) 	/* A_MAGIC5 */
		{
			fatal1("%s is not an object module", ar_pdp.ar_name);
		}
		skip = arobj_pdp.a_text + arobj_pdp.a_data;
		if (!arobj_pdp.a_flag)
			skip *= 2;
		if (skip >= ar_pdp.ar_size || fseek(arfd, skip, 1) != 0)
			goto seek_error;
		skip = ar_pdp.ar_size;
		skip += (skip & 01) + last;
		i = (arobj_pdp.a_syms / sizeof(ars_pdp)) + 1;
		while (--i > 0)		/* look through symbol table */
		{
			if (fread((char *)&ars_pdp, sizeof(ars_pdp), 1, arfd)
				!= 1)
			{
				fatal1("Read error on archive %s", archname);
			}
			if ((ars_pdp.n_type & 040)	/* N_EXT for pdp11 */
				&& equaln(ars_pdp.n_name, name, len))
			{
				(void)strncpy(archmem, ar_pdp.ar_name, 14);
				archmem[14] = '\0';
				return (ar_pdp.ar_date);
			}
		}
		if (fseek(arfd, skip, 0) != 0)
			goto seek_error;
	}
	return (0L);
}


openarch(f)
register CHARSTAR f;
{
	unsigned short	mag_pdp;		/* old archive format */
	char		mag_5[SAR5MAG];		/* 5.0 archive format */
	char		mag_port[SARportMAG];	/* port (6.0) archive format */

	arfd = fopen(f, "r");
	if(arfd == NULL)
		return(-1);
	/*
	* More code for three archive formats.  Read in just enough to
	* distinguish the three types and set ar_type.  Then if it is
	* one of the newer archive formats, gather more info.
	*/
	if (fread((char *)&mag_pdp, sizeof(mag_pdp), 1, arfd) != 1)
		return (-1);
	if (mag_pdp == (unsigned short)ARpdpMAG)
	{
		ar_type = ARpdp;
		first_ar_mem = ftell(arfd);
		sym_begin = num_symbols = sym_size = 0L;
		return (0);
	}
	if (fseek(arfd, 0L, 0) != 0 || fread(mag_5, SAR5MAG, 1, arfd) != 1)
		return (-1);
	if (equaln(mag_5, AR5MAG, SAR5MAG))
	{
		ar_type = AR5;
		/*
		* Must read in header to set necessary info
		*/
		if (fseek(arfd, 0L, 0) != 0 ||
			fread((char *)&arh_5, sizeof(arh_5), 1, arfd) != 1)
		{
			return (-1);
		}
#ifdef pdp11
		fatal1("Cannot handle 5.0 archive format for %s", archname);
		/*NOTREACHED*/
#else
		sym_begin = ftell(arfd);
		num_symbols = sgetl(arh_5.ar_syms);
		first_ar_mem = sym_begin + sizeof(ars_5) * num_symbols;
		sym_size = 0L;
		return (0);
#endif
	}
	if (fseek(arfd, 0L, 0) != 0 ||
		fread(mag_port, SARportMAG, 1, arfd) != 1)
	{
		return (-1);
	}
	if (equaln(mag_port, ARportMAG, SARportMAG))
	{
		ar_type = ARport;
		/*
		* Must read in first member header to find out
		* if there is a symbol directory
		*/
		if (fread((char *)&ar_port, sizeof(ar_port), 1, arfd) != 1 ||
			!equaln(ARFportMAG, ar_port.ar_fmag,
			sizeof(ar_port.ar_fmag)))
		{
			return (-1);
		}
#ifdef pdp11
		fatal1("Cannot handle portable archive format for %s",
			archname);
		/*NOTREACHED*/
#else
		if (ar_port.ar_name[0] == '/')
		{
			char s[4];

			if (sscanf(ar_port.ar_size, "%ld", &sym_size) != 1)
				return (-1);
			sym_size += (sym_size & 01);	/* round up */
			if (fread(s, sizeof(s), 1, arfd) != 1)
				return (-1);
			num_symbols = sgetl(s);
			sym_begin = ftell(arfd);
			first_ar_mem = sym_begin + sym_size - sizeof(s);
		}
		else	/* there is no symbol directory */
		{
			sym_size = num_symbols = sym_begin = 0L;
			first_ar_mem = ftell(arfd) - sizeof(ar_port);
		}
		return (0);
#endif
	}
	fatal1("%s is not an archive", f);
	/*NOTREACHED*/
#if 0
	if(fread(&head, sizeof(head), 1, arfd) != 1)
		return(-1);
	if(!equaln(head.ar_magic, ARMAG, 4))
		fatal1("%s is not an archive", f);
	return(0);
#endif
}
#endif	/* sgiAR */

clarch()
{
	if(arfd != NULL)
		fclose(arfd);
}

/*
 *	Used when unlinking files. If file cannot be stat'ed or it is
 *	a directory, then do not remove it.
 */
isdir(p)
char *p;
{
	struct stat statbuf;

	if(stat(p, &statbuf) == -1)
		return(1);		/* no stat, no remove */
	if((statbuf.st_mode&S_IFMT) == S_IFDIR)
		return(1);
	return(0);
}
#ifdef	sgiAR

/* look inside archives for notations a(b) and a((b))
	a(b)	is file member   b   in archive a
	a((b))	is entry point  _b  in object archive a
*/

#include <a.out.h>

static struct ar_hdr arhead;
long int arpos, arlen;

static struct exec objhead;
static struct nlist objentry;
char archname[64];		/* name of archive library */


TIMETYPE lookarch(filename)
register CHARSTAR filename;
{
	register int i;
	CHARSTAR p, q, s_end;
	char s[17];
	int nc, nsym, objarch;

	for(p = filename; *p != LPAREN ; p++);
	q = p++;

	if(*p == LPAREN) {
		objarch = YES;
		nc = 8;
		++p;
	} else {
		objarch = NO;
		nc = 16;
		for(i = 0; i < 16; i++)
		{
			if(p[i] == RPAREN)
			{
				i--;
				break;
			}
			archmem[i] = p[i];
		}
		archmem[++i] = 0;
	}
	*q = CNULL;
	copstr(archname, filename);
	i = openarch(filename);
	*q = LPAREN;
	if(i == -1)
		return(0);
	s_end = s + nc;

	for( q = s ; q<s_end && *p!=CNULL && *p!=RPAREN ; *q++ = *p++ );

	while(q < s_end)
		*q++ = CNULL;
	while(getarch())
	{
		if(objarch)
		{
			getobj();
			nsym = objhead.a_syms / sizeof(objentry);
			for(i = 0; i<nsym ; ++i)
			{
				fread(&objentry, sizeof(objentry),1,arfd);
				if( (objentry.n_type & N_EXT)
				   && eqstr(objentry.n_un.n_name,s,nc))
				{
					for(i = 0; i < 16; i++)
						archmem[i] = arhead.ar_name[i];
					archmem[++i] = 0;
	out:
					clarch();
					return(atol (arhead.ar_date));
				}
			}
		} else {
			if( eqstr(arhead.ar_name, s, nc)) {
				goto out;
			}
		}
	}

	clarch();
	return( 0L);
}

openarch(f)
register CHARSTAR f;
{
	struct stat buf;
	char word[SARMAG + 1];

	word[SARMAG] = '\0';

	if(stat(f, &buf) == -1)
		return(-1);
	arlen = buf.st_size;

	arfd = fopen(f, "r");
	if(arfd == NULL)
		return(-1);
	fread(word, SARMAG, 1, arfd);
	if (eqstr (word, ARMAG, SARMAG) != YES)
		fatal1("%s is not an archive", f);
/*
 *	trick getarch() into jumping to the first archive member.
 */
	arpos = SARMAG;
	sprintf (arhead.ar_size, "%-10d", -(int)sizeof(arhead));
	arhead.ar_fmag[0] = '`';
	return(0);
}



getarch()
{
	arpos += sizeof(arhead);
	arpos += (atol (arhead.ar_size) + 1 ) & ~1L;
	if(arpos >= arlen)
		return(0);
	fseek(arfd, arpos, 0);
	fread(&arhead, sizeof(arhead), 1, arfd);
	return(1);
}


getobj()
{
	long int skip;

	fread(&objhead, sizeof(objhead), 1, arfd);
	if(N_BADMAG(objhead))
		fatal1("%s is not an object module", arhead.ar_name);

	skip = objhead.a_text + objhead.a_data;
	skip += objhead.a_trsize + objhead.a_drsize;
	fseek(arfd, skip, 1);
}

eqstr(a,b,n)
register CHARSTAR a, b;
register int n;
{
	register int i;
	for(i = 0 ; i < n ; ++i) {
		if(*a++ != *b++) {
			a--;
			b--;
			if (*a == ' ' || *a == '\0') {
				if (*b == ' ' || *b == '\0')
					return(YES);
			}
			return(NO);
		}
	}
	return(YES);
}
#endif	/* sgi */
