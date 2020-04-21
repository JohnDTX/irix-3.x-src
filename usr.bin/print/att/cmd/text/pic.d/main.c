/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)pic:main.c	1.10";

#define	APS	1
#include	<stdio.h>
#include	<signal.h>
#include	"pic.h"
#include	"y.tab.h"

extern	double	atof();
obj	**objlist = 0;	/* store the elements here */
int	nobjlist = 0;		/* size of objlist array */
int	nobj	= 0;

Attr	*attr;	/* attributes stored here as collected */
int	nattrlist = 0;
int	nattr	= 0;	/* number of entries in attr_list */

Text	*text	= 0;	/* text strings stored here as collected */
int	ntextlist = 0;		/* size of text[] array */
int	ntext	= 0;
int	ntext1	= 0;	/* record ntext here on entry to each figure */

float	curx	= 0;
float	cury	= 0;

int	hvmode	= R_DIR;	/* R => join left to right, D => top to bottom, etc. */

int	codegen	= 0;	/* 1=>output for this picture; 0=>no output */

float	deltx	= 6;	/* max x value in output, for scaling */
float	delty	= 6;	/* max y value in output, for scaling */
int	dbg	= 0;
int	lineno	= 0;
char	*filename	= "-";
int	synerr	= 0;
char	*cmdname;
int	crop	= 1;	/* trim off exterior white space if non-zero */
extern int	useDline;	/* if set, use \D for all lines */

/* You may want to change this if you don't have a 202... */

#ifdef	APS
	int	devtype	= DEVAPS;
	int	res	= 723;
	char	*typesetter	= "aps";	/* default typesetter */
#else
	int	devtype	= DEV202;
	int	res	= 972;	/* default is 202 */
	char	*typesetter	= "202";	/* default typesetter */
#endif

float	xmin	= 30000;	/* min values found in actual data */
float	ymin	= 30000;
float	xmax	= -30000;	/* max */
float	ymax	= -30000;

main(argc, argv)
	char *argv[];
{
	char *getenv(), *p, buf[20];
	extern int fpecatch();
	extern char *optarg;
	extern int optind;
	int c;		/* command line option */

	signal(SIGFPE, fpecatch);
	cmdname = argv[0];
	if (p = getenv("TYPESETTER"))
		typesetter = p;
#ifdef DEBUG
#define OPTSTR "d:DT:"
#else
#define OPTSTR "DT:"
#endif
	while ((c = getopt(argc, argv, OPTSTR)) != EOF)
		switch (c) {
#ifdef DEBUG
		case 'd':
			dbg = atoi(optarg);
			if (dbg == 0)
				dbg = 1;
			break;
#endif
		case 'D':
			useDline = !useDline;
			break;
		case 'T':
			typesetter = optarg;
			break;
		case '?':
			fprintf (stderr, "usage: %s [-D] [-T term] [--] [-] [file] ...\n", cmdname);
			exit (2);
			break;
		}
	settype(typesetter);
	setdefaults();
	objlist = (obj **) grow(objlist, "objlist", nobjlist += 1000, sizeof(obj *));
	text = (Text *) grow(text, "text", ntextlist += 1000, sizeof(Text));
	attr = (Attr *) grow(attr, "attr", nattrlist += 100, sizeof(Attr));

	sprintf(buf, "/%d/", getpid());
	pushsrc(String, buf);
	definition("pid");

	pushsrc(File, curfile = infile);
	if (optind >= argc) {
		curfile->fin = stdin;
		curfile->fname = "-";
		getdata(curfile);
	} else
		for (; optind < argc; optind++) {
			if (strcmp (argv [optind], "-") == 0) {
				curfile->fin = stdin;
				curfile->fname = "-";
				getdata (curfile);
			} else {
				if ((curfile->fin = fopen(argv [optind], "r")) == NULL) {
					fprintf(stderr, "%s: can't open %s\n", cmdname, argv [optind]);
					exit(1);
				}
				curfile->fname = argv [optind];
				getdata(curfile);
				fclose(curfile->fin);
			}
		}
	exit(0);
}

fpecatch()
{
	fatal("floating point exception");
}

settype(s)	/* set data for typesetter */
	char *s;
{
	if (strcmp(s, "aps") == 0) {
		res = 723;
		devtype = DEVAPS;
	} else if (strcmp(s, "202") == 0) {
		res = 972;
		devtype = DEV202;
	} else if (strcmp(s, "i10") == 0) {
		res = 240;
		devtype = DEVI10;
	} else {
#ifdef APS
		fprintf(stderr, "%s: unknown typesetter %s; aps assumed\n", cmdname, s);
#else
		fprintf(stderr, "%s: unknown typesetter %s; 202 assumed\n", cmdname, s);
#endif
	}
}

char *grow(ptr, name, num, size)	/* make array bigger */
	char *ptr, *name;
	int num, size;
{
	char *p;

	if (ptr == NULL)
		p = malloc(num * size);
	else
		p = realloc(ptr, num * size);
	if (p == NULL)
		fatal("can't grow %s to %d", name, num * size);
	return p;
}

static struct {
	char *name;
	float val;
} defaults[] ={
	"scale", SCALE,
	"lineht", HT,
	"linewid", HT,
	"moveht", HT,
	"movewid", HT,
	"dashwid", HT10,
	"boxht", HT,
	"boxwid", WID,
	"circlerad", HT2,
	"arcrad", HT2,
	"ellipseht", HT,
	"ellipsewid", WID,
	"arrowht", HT5,
	"arrowwid", HT10,
	"textht", 0,		/* 6 lines/inch is also a useful value */
	"textwid", 0,
	NULL, 0
};

setdefaults()	/* set default sizes for variables like boxht */
{
	int i;
	YYSTYPE v;

	for (i = 0; defaults[i].name != NULL; i++) {
		v.f = defaults[i].val;
		makevar(tostring(defaults[i].name), VARNAME, v);
	}
}


checkscale(s)	/* if s is "scale", adjust default variables */
	char *s;
{
	int i;
	float scale;

	if (strcmp(s, "scale") == 0) {
		scale = getfval("scale");
		for (i = 1; defaults[i].name != NULL; i++)
			setfval(defaults[i].name, defaults[i].val * scale);
	}
}

getdata(cfp)
	register Infile *cfp;
{
	char buf[1000], buf1[50];
	char *p;
	extern int errno;

	errno = 0;	/* bug in isatty() */
	cfp->lineno = 0;
	while (fgets(buf, sizeof buf, cfp->fin) != NULL) {
		cfp->lineno++;
		if (*buf == '.' && *(buf+1) == 'P' && *(buf+2) == 'S') {
			for (p = &buf[3]; *p == ' '; p++)
				;
			if (*p++ == '<') {
				Infile svfile;
				Infile *svcurfile = curfile;
				svfile = *curfile;
				sscanf(p, "%s", buf1);
				if ((curfile->fin=fopen(buf1, "r")) == NULL)
					fatal("can't open %s", buf1);
				curfile->fname = buf1;
				curfile->lineno = 0;
				getdata(curfile);
				fclose(curfile->fin);
				*curfile = svfile;
				continue;
			}
			reset();
			yyparse();
			/* yylval.i now contains 'E' or 'F' from .PE or .PF */

			deltx = (xmax - xmin) / getfval("scale");
			delty = (ymax - ymin) / getfval("scale");
			if (buf[3] == ' ') {	/* next things are wid & ht */
				if (sscanf(&buf[4],"%f%f",&deltx,&delty) < 2)
					delty = deltx * (ymax-ymin) / (xmax-xmin);
			}
			dprintf("deltx = %g, delty = %g\n", deltx, delty);
			if (codegen && !synerr) {
				openpl(&buf[3]);	/* puts out .PS, with ht & wid stuck in */
				print();	/* assumes \n at end */
				closepl(yylval.i);	/* does the .PE/F */
			}
			fflush(stdout);
		}
		else
			fputs(buf, stdout);
	}
}

reset()
{
	obj *op;
	int i;
	struct symtab *p;
	extern int nstack;

	for (i = 0; i < nobj; i++) {
		op = objlist[i];
		if (op->o_type == BLOCK)
			freesymtab(op->o_dotdash);	/* funny place */
		free(objlist[i]);
	}
	nobj = 0;
	nattr = 0;
	for (i = 0; i < ntext; i++)
		if (text[i].t_val)
			free(text[i].t_val);
	ntext = ntext1 = 0;
	codegen = synerr = 0;
	nstack = 0;
	curx = cury = 0;
	hvmode = R_DIR;
	xmin = ymin = 30000;
	xmax = ymax = -30000;
}
