/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)eqn:main.c	1.8";
# include "e.h"
#define	MAXLINE	3600	/* maximum input line */

char	in[MAXLINE];	/* input buffer */
int	eqnexit();
int	noeqn;
char	*cmdname;

main(argc,argv)
	int argc;
	char *argv[];
{
	eqnexit(eqn(argc, argv));
}

eqnexit(n) {
#ifdef gcos
	if (n)
		fprintf(stderr, "run terminated due to eqn error\n");
	exit(0);
#endif
	exit(n);
}

eqn(argc,argv)
	int argc;
	char *argv[];
{
	int c, i, type;
	int sharpsign;		/*MRW debug option*/
	char *p, *getenv();
	extern char *optarg;
	extern int   optind;

	cmdname = argv[0];
	if (p = getenv("TYPESETTER"))
		typesetter = p;
	while ((c = getopt(argc, argv, "d:s:p:f:T:#")) != EOF) {
		switch (c) {

		case 'd':
			lefteq = *optarg++;
			righteq = *optarg;
			break;
		case 's':
			szstack[0] = gsize = atoi(optarg);
			break;
		case 'p':
			deltaps = atoi(optarg);
			dps_set = 1;
			break;
		case 'f':
			strcpy(ftstack[0].name,optarg);
			break;
		case 'T':
			typesetter = optarg;
			break;
		case '#':			/*MRW debug option*/
			sharpsign = 1;
			break;
		case '?':
			fprintf(stderr, "Usage: %s [-d xy] [-p n] [-s n] [-f n] [-T dest] [files]\n", cmdname);
			return (2);			/* exit */
/*  undocumented options
 *		case 'r': res = atoi(&argv[1][2]); break;
 *		case 'm': minsize = atoi(&argv[1][2]); break;
 *		case 'e': noeqn++; break;
 *		default:
 *			dbg = 1;
 */
		}
	}
	argc -= optind - 1;
	argv += optind - 1;
	settype(typesetter);

	if (sharpsign == 1) {
		fprintf(stderr, "delimiters: lefteq=%c, righteq=%c, noeqn is %d.\n",
			lefteq, righteq, noeqn);
		fprintf(stderr, "gsize is %d, deltaps is %d, ftstack[0].name is %s.\n",
			gsize, deltaps, ftstack[0].name);
		fprintf(stderr, "typsetter is %s, res is %d, minsize is %d, ttype is %d.\n",
			typesetter, res, minsize, ttype);
	}

	lookup(deftbl, strsave(typesetter), strsave(typesetter));
	init_tbl();	/* install other keywords in tables */
	curfile = infile;
	pushsrc(File, curfile);
	if (argc <= 1) {
		curfile->fin = stdin;
		curfile->fname = strsave("-");
		getdata();
	} else
		while (argc-- > 1) {
			if (strcmp(*++argv, "-") == 0)
				curfile->fin = stdin;
			else if ((curfile->fin = fopen(*argv, "r")) == NULL)
				fatal("can't open file %s", *argv);
			curfile->fname = strsave(*argv);
			getdata();
			if (curfile->fin != stdin)
				fclose(curfile->fin);
		}
	return 0;
}

settype(s)	/* initialize data for particular typesetter */
	char *s;
{
	if (strcmp(s, "202") == 0)
		{ res = 972; minsize = 5; ttype = DEV202; }
	else if (strcmp(s, "aps") == 0)
		{ res = 723; minsize = 5; ttype = DEVAPS; }
	else if (strcmp(s, "cat") == 0)
		{ res = 432; minsize = 6; ttype = DEVCAT; }
	else
		{ res = atoi(s); minsize = 6; ttype = DEVCAT; }
}

getdata()
{
	register FILE *fin;
	int i, type, ln;
	char fname[100];
	extern int errno;

	errno = 0;
	fin = curfile->fin;
	curfile->lineno = 0;
	printf(".lf 1 %s\n", curfile->fname);
	while ((type = getline(in)) != EOF) {
		if (in[0] == '.' && in[1] == 'E' && in[2] == 'Q') {
			for (i = 11; i < 100; i++)
				used[i] = 0;
			printf("%s", in);
			if (markline) {	/* turn off from last time */
				printf(".nr MK 0\n");
				markline = 0;
			}
			display = 1;
			init();
			yyparse();
			if (eqnreg > 0) {
				if (markline)
					printf(".nr MK %d\n", markline); /* for -ms macros */
				printf(".if %gm>\\n(.v .ne %gm\n", eqnht, eqnht);
				printf(".rn %d 10\n", eqnreg);
				if (!noeqn)
					printf("\\&\\*(10\n");
			}
			printf(".EN");
			while (putchar(input()) != '\n')
				;
			printf(".lf %d\n", curfile->lineno+1);
		}
		else if (type == lefteq)
			inline();
		else if (in[0] == '.' && in[1] == 'l' && in[2] == 'f') {
			if (sscanf(in+3, "%d %s", &ln, fname) == 2) {
				free(curfile->fname);
				printf(".lf %d %s\n", curfile->lineno = ln, curfile->fname = strsave(fname));
			} else
				printf(".lf %d\n", curfile->lineno = ln);
		} else
			printf("%s", in);
	}
	return(0);
}

getline(s)
	register char *s;
{
	register c;

	while ((c=input()) != '\n' && c != EOF && c != lefteq) {
		if (s >= in+MAXLINE) {
			error("input line too long: %.20s\n", in);
			in[MAXLINE] = '\0';
			break;
		}
		*s++ = c;
	}
	if (c != lefteq)
		*s++ = c;
	*s = '\0';
	return(c);
}

inline()
{
	int ds, n, sz1 = 0;

	n = curfile->lineno;
	if (szstack[0] != 0)
		printf(".nr %d \\n(.s\n", sz1 = salloc());
	ds = salloc();
	printf(".rm %d \n", ds);
	display = 0;
	do {
		if (*in)
			printf(".as %d \"%s\n", ds, in);
		init();
		yyparse();
		if (eqnreg > 0) {
			printf(".as %d \\*(%d\n", ds, eqnreg);
			sfree(eqnreg);
			printf(".lf %d\n", curfile->lineno+1);
		}
	} while (getline(in) == lefteq);
	if (*in)
		printf(".as %d \"%s", ds, in);
	if (sz1)
		printf("\\s\\n(%d", sz1);
	printf("\\*(%d\n", ds);
	printf(".lf %d\n", curfile->lineno+1);
	if (curfile->lineno > n+3)
		fprintf(stderr, "eqn warning: multi-line %c...%c, lines %d-%d, file %s\n",
			lefteq, righteq, n, curfile->lineno, curfile->fname); 
	sfree(ds);
	if (sz1) sfree(sz1);
}

putout(p1)
	int p1;
{
	float before, after;

	dprintf(".\tanswer <- S%d, h=%g,b=%g\n",p1, eht[p1], ebase[p1]);
	eqnht = eht[p1];
	before = eht[p1] - ebase[p1] - 1.2;	/* leave room for sub or superscript */
	after = ebase[p1] - 0.2;
	if (spaceval || before > 0.01 || after > 0.01) {
		printf(".ds %d ", p1);	/* used to be \\x'0' here:  why? */
		if (spaceval != NULL)
			printf("\\x'0-%s'", spaceval);
		else if (before > 0.01)
			printf("\\x'0-%gm'", before);
		printf("\\*(%d", p1);
		if (spaceval == NULL && after > 0.01)
			printf("\\x'%gm'", after);
		putchar('\n');
	}
	if (szstack[0] != 0)
		printf(".ds %d %s\\*(%d\\s\\n(99\n", p1, DPS(gsize,gsize), p1);
	eqnreg = p1;
	if (spaceval != NULL) {
		free(spaceval);
		spaceval = NULL;
	}
}

init()
{
	synerr = 0;
	ct = 0;
	ps = gsize;
	ftp = ftstack;
	ft = ftp->ft;
	nszstack = 0;
	if (szstack[0] != 0)	/* absolute gsize in effect */
		printf(".nr 99 \\n(.s\n");
}

salloc()
{
	int i;

	for (i = 11; i < 100; i++)
		if (used[i] == 0) {
			used[i]++;
			return(i);
		}
	error(FATAL, "no eqn strings left (%d)", i);
	return(0);
}

sfree(n)
	int n;
{
	used[n] = 0;
}

nrwid(n1, p, n2)
	int n1, p, n2;
{
	printf(".nr %d 0\\w'%s\\*(%d'\n", n1, DPS(gsize,p), n2);	/* 0 defends against - width */
}

char *ABSPS(dn)	/* absolute size dn in printable form \sd or \s(dd (dd >= 40) */
	int dn;
{
	static char buf[100], *lb = buf;
	char *p;

	if (lb > buf + sizeof(buf) - 10)
		lb = buf;
	p = lb;
	*lb++ = '\\';
	*lb++ = 's';
	if (dn >= 10) {		/* \s(dd only works in new troff */
		if (dn >= 40)
			*lb++ = '(';
		*lb++ = dn/10 + '0';
		*lb++ = dn%10 + '0';
	} else {
		*lb++ = dn + '0';
	}
	*lb++ = '\0';	
	return p;
}

char *DPS(f, t)	/* delta ps (t-f) in printable form \s+d or \s-d or \s+-(dd */
	int f, t;
{
	static char buf[100], *lb = buf;
	char *p;
	int dn;

	if (lb > buf + sizeof(buf) - 10)
		lb = buf;
	p = lb;
	*lb++ = '\\';
	*lb++ = 's';
	dn = EFFPS(t) - EFFPS(f);
	if (szstack[nszstack] != 0)	/* absolute */
		dn = EFFPS(t);		/* should do proper \s(dd */
	else if (dn >= 0)
		*lb++ = '+';
	else {
		*lb++ = '-';
		dn = -dn;
	}
	if (dn >= 10) {		/* \s+(dd only works in new troff */
		*lb++ = '(';
		*lb++ = dn/10 + '0';
		*lb++ = dn%10 + '0';
	} else {
		*lb++ = dn + '0';
	}
	*lb++ = '\0';	
	return p;
}

EFFPS(n)	/* effective value of n */
	int n;
{
	if (n >= minsize)
		return n;
	else
		return minsize;
}

double EM(m, ps)	/* convert m to ems in gsize */
	double m;
	int ps;
{
	m *= (float) EFFPS(ps) / gsize;
	if (m <= 0.001 && m >= -0.001)
		return 0;
	else
		return m;
}

double REL(m, ps)	/* convert m to ems in ps */
	double m;
	int ps;
{
	m *= (float) gsize / EFFPS(ps);
	if (m <= 0.001 && m >= -0.001)
		return 0;
	else
		return m;
}
