char _Origin_[] = "System V";
/*	@(#)lpfx.c	1.3	*/
/* $Source: /d2/3.7/src/usr.bin/cflow/RCS/lpfx.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 17:43:45 $ */

#include <stdio.h>
/* #include "lerror.h" */
#include "manifest.h"
#include "lmanifest"
# ifndef LDS
#	define LDS	LST
# endif  LDS
/* #include "lpass2.h" */
#define FNSIZE LFNM
#define NSIZE LCHNM

typedef struct {
	union rec r;
	char fname[FNSIZE + 1];
	} funct;

typedef struct LI {
	struct LI *next;
	funct fun;
	} li;

/*
 * lpfx - read lint1 output, sort and format for dag
 *
 *	options -i_ -ix (inclusion)
 *
 *	while (read lint1 output into "funct" structures)
 *		if (this is a filename record)
 *			save filename
 *		else
 *			read arg records and throw on floor
 *			if (this is to be included)
 *				copy filename into "funct"
 *				insert into list
 *	format and print
 */


main(argc, argv)
	int argc;
	char **argv;
	{
	extern int optind;
	extern char *optarg;
	funct fu;
	char filename[FNSIZE + 1], *strncpy();
	int uscore, fmask, c;
	void rdargs(), insert(), putout();

	fmask = LDS | LDX | LRV;
	uscore = 0;
	while ((c = getopt(argc, argv, "i:")) != EOF)
		if (c == 'i')
			if (*optarg == '_')
				uscore = 1;
			else if (*optarg == 'x')
				fmask &= ~(LDS | LDX);
			else
				goto argerr;
		else
		argerr:
			(void)fprintf(stderr, "lpfx: bad option %c ignored\n", c);
	while (0 < fread((char *)&fu.r, sizeof(fu.r), 1, stdin))
		if (fu.r.l.decflag & LFN)
			{
			(void)strncpy(filename, fu.r.f.fn, FNSIZE);
			filename[FNSIZE] = '\0';
			}
		else
			{
			rdargs(&fu);
			if (((fmask & LDS) ? ISFTN(fu.r.l.type.aty) :
			      !(fu.r.l.decflag & fmask)) &&
			    ((uscore) ? 1 : (*fu.r.l.name != '_')))
				{
				(void)strncpy(fu.fname, filename, FNSIZE);
				fu.fname[FNSIZE] = '\0';
				insert(&fu);
				}
			}
	putout();
	}

/*
 * rdargs - read arg records and throw on floor
 *
 *	if ("funct" has args)
 *		get absolute value of nargs
 *		if (too many args)
 *			panic and die
 *		read args into temp array
 */

void rdargs(pf)
	register funct *pf;
	{
	struct ty atype[50];
	void exit();

	if (pf->r.l.nargs)
		{
		if (pf->r.l.nargs < 0)
			pf->r.l.nargs = -pf->r.l.nargs;
		if (pf->r.l.nargs > 50)
			{
			(void)fprintf(stderr, "lpfx: PANIC nargs=%d\n", pf->r.l.nargs);
			exit(1);
			}
		if (fread((char *)atype, sizeof(ATYPE), pf->r.l.nargs, stdin) <= 0)
			{
			(void)perror("lpfx.rdargs");
			exit(1);
			}
		}
	}

/*
 * insert - insertion sort into (singly) linked list
 *
 *	stupid linear list insertion
 */

static li *head = NULL;

void insert(pfin)
	register funct *pfin;
	{
	register li *list_item, *newf;
	char *malloc();
	void exit();

	if ((newf = (li *)malloc(sizeof(li))) == NULL)
		{
		(void)fprintf(stderr, "lpfx: out of heap space\n");
		exit(1);
		}
	newf->fun = *pfin;
	if (list_item = head)
		if (newf->fun.r.l.fline < list_item->fun.r.l.fline)
			{
			newf->next = head;
			head = newf;
			}
		else
			{
			while (list_item->next &&
			  list_item->next->fun.r.l.fline < newf->fun.r.l.fline)
				list_item = list_item->next;
			while (list_item->next &&
			  list_item->next->fun.r.l.fline == newf->fun.r.l.fline &&
			  list_item->next->fun.r.l.decflag < newf->fun.r.l.decflag)
					list_item = list_item->next;
			newf->next = list_item->next;
			list_item->next = newf;
			}
	else
		{
		head = newf;
		newf->next = NULL;
		}
	}

/*
 * putout - format and print sorted records
 *
 *	while (there are records left)
 *		copy name and null terminate
 *		if (this is a definition)
 *			if (this is a function**)
 *				save name for reference formatting
 *			print definition format
 *		else if (this is a reference)
 *			print reference format
 *
 *	** as opposed to external/static variable
 */

void putout()
	{
	register li *pli;
	char lname[NSIZE+1], *prtype(), *strncpy(), *strcpy(), name[NSIZE+1];
	
	pli = head;
	name[0] = lname[0] = '\0';
	while (pli != NULL)
		{
		(void)strncpy(name, pli->fun.r.l.name, NSIZE);
		name[NSIZE] = '\0';
		if (pli->fun.r.l.decflag & (LDI | LDC | LDS))
			{
			if (ISFTN(pli->fun.r.l.type.aty))
				(void)strcpy(lname, name);
			(void)printf("%s = %s, <%s %d>\n", name, prtype(pli),
			    pli->fun.fname, pli->fun.r.l.fline);
			}
		else if (pli->fun.r.l.decflag & (LUV | LUE | LUM))
			(void)printf("%s : %s\n", lname, name);
		pli = pli->next;
		}
	}

static char *types[] = {
	"undef", "farg", "char", "short", "int", "long", "float",
	"double", "struct", "union", "enum", "moety", "uchar",
	"ushort", "uint", "ulong"};

/*
 * prtype - decode type fields
 *
 *	strictly arcana
 */

char *prtype(pli)
	register li *pli;
	{
	static char bigbuf[64];
	char buf[32], *shift(), *strcpy(), *strcat();
	register char *bp;
	register int typ;

	typ = pli->fun.r.l.type.aty;
	(void)strcpy(bigbuf, types[typ & 017]);
	*(bp = buf) = '\0';
	for (typ >>= 4; typ > 0; typ >>= 2)
		switch (typ & 03)
			{
			case 1:
				bp = shift(buf);
				buf[0] = '*';
				break;
			case 2:
				*bp++ = '(';
				*bp++ = ')';
				*bp = '\0';
				break;
			case 3:
				*bp++ = '[';
				*bp++ = ']';
				*bp = '\0';
				break;
			}
	(void)strcat(bigbuf, buf);
	return(bigbuf);
	}

char *shift(s)
	register char *s;
	{
	register char *p1, *p2;
	char *rp;

	for (p1 = s; *p1; ++p1)
		;
	rp = p2 = p1++;
	while (p2 >= s)
		*p1-- = *p2--;
	return(++rp);
	}
