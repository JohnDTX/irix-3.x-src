/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)grap:main.c	1.5";

#include	<stdio.h>
#include	<signal.h>
#include	"grap.h"
#include	"y.tab.h"

int	dbg	= 0;
char	*lib_defines	= "/usr/lib/dwb/grap.defines";
int	lib	= 1;		/* 1 to include lib_defines */
FILE	*tfd	= NULL;
char	*tempfile = NULL;

int	synerr	= 0;
int	codegen	= 0;   /* 1=>output for this picture; 0=>no output */
char	*cmdname;
char	*typesetter = "aps";	/* default typesetter */

Obj	*objlist = NULL;	/* all names stored here */

Point	ptmin	= { NULL, -1.0e10, -1.0e10 };
Point	ptmax	= { NULL, 1.0e10, 1.0e10 };

main(argc, argv)
	char **argv;
{
	extern int onintr(), fpecatch();
	char *p, *getenv();
	int c;
	extern char *optarg;
	extern int optind;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);
	signal(SIGFPE, fpecatch);
	if ((p = getenv("TYPESETTER")))
		typesetter = p;
	tempfile = "/tmp/grap.XXXXXX";
	cmdname = argv[0];
#ifdef DEBUG 
#define OPTSTR	"dltT:"
#else 
#define OPTSTR	"lT:"
#endif
	while ((c = getopt (argc, argv, OPTSTR)) != EOF) {
		switch (c) {
#ifdef DEBUG
		case 'd':
			dbg = 1;
			tfd = stdout;
			tempfile = "grap.temp";
			unlink(tempfile);
			break;
		case 't':
			tfd = stdout;
			break;
#endif
		case 'l':	/* turn off /usr/lib inclusion */
			lib = 0;
			break;
		case 'T':
			typesetter = optarg;
			break;
		case '?':
			fprintf (stderr, "usage: %s [-l] [-T typesetter] [--] [-] [file] ...\n", cmdname);
			exit (2);
		}
	}
	setdefaults();
	if (!dbg)
		mktemp(tempfile);
	curfile = infile;
	pushsrc(File, curfile);
	if (optind >= argc) {
		curfile->fin = stdin;
		curfile->fname = tostring("-");
		getdata();
	} else
		for (; optind < argc; optind++) {
			if (strcmp (argv [optind], "-") == 0) {
				curfile->fin = stdin;
				curfile->fname = tostring("-");
				getdata ();
			} else {
				if ((curfile->fin = fopen(argv [optind], "r")) == NULL) {
					fprintf(stderr, "%s: can't open %s\n", cmdname, argv [optind]);
					onintr();
				}
				curfile->fname = tostring(argv [optind]);
				getdata();
				fclose(curfile->fin);
				free(curfile->fname);
			}
		}
	if (!dbg)
		unlink(tempfile);
	exit(0);
}

onintr()
{
	if (!dbg)
		unlink(tempfile);
	exit(1);
}

fpecatch()
{
	yyerror("floating point exception");
	onintr();
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
	char	*name;
	double	val;
} defaults[] ={
	"frameht", FRAMEHT,
	"framewid", FRAMEWID,
	"ticklen", TICKLEN,
	"slop", SLOP,
	NULL, 0
};

setdefaults()	/* set default sizes for variables */
{
	int i;
	Obj *p;

	for (i = 0; defaults[i].name != NULL; i++) {
		p = lookup(defaults[i].name, 1);
		setvar(p, defaults[i].val);
	}
}

getdata()		/* read input */
{
	register FILE *fin;
	char buf[1000], buf1[100], *p;
	int ln;

	fin = curfile->fin;
	curfile->lineno = 0;
	printf(".lf 1 %s\n", curfile->fname);
	while (fgets(buf, sizeof buf, fin) != NULL) {
		curfile->lineno++;
		if (*buf == '.' && *(buf+1) == 'G' && *(buf+2) == '1') {
			setup();
			fprintf(stdout, ".PS%s", &buf[3]);	/* maps .G1 [w] to .PS w */
			printf(".lf %d\n", curfile->lineno+1);
			yyparse();
			fprintf(stdout, ".PE\n");
			printf(".lf %d\n", curfile->lineno+1);
			fflush(stdout);
		} else if (buf[0] == '.' && buf[1] == 'l' && buf[2] == 'f') {
			if (sscanf(buf+3, "%d %s", &ln, buf1) == 2) {
				free(curfile->fname);
				printf(".lf %d %s\n", curfile->lineno = ln, curfile->fname = tostring(buf1));
			} else
				printf(".lf %d\n", curfile->lineno = ln);
		} else
			fputs(buf, stdout);
	}
}
