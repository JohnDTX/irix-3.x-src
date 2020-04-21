char _Origin_[] = "UniSoft Systems";

#include <stdio.h>
#include <curses.h>

/*
 *	twinkle [-+[s save]] [density1] [density2]
 *		twinkle stars on the screen
 *	parameters:
 *		- = print out the present screen density in lower left hand
 *			corner
 *		+ = do not randomize before starting
 *		s = save binary density on file save
 *		density1/density2
 *			if no density's are given, density is .5
 *			if only density1 is given, density is 1/density1
 *			if both density1 and density 2 are given,
 *				density is density1/(density1+density2)
 */

#define MAXNROW	40
#define MAXNCOL	120
int nrow = 23;
int ncol = 79;

char *getenv();

#define CH	'*'

unsigned buf[MAXNCOL][MAXNROW];
int dens1=1, dens2=2;
int pflag;
int norand;
int sfile;
long den;
int kil(), _kill;

#define lrand(max)	((rand(0) & 077777) % (max))
#define mod(a, b)	(((a) & 077777) % (b))

main(argc, argv)
char **argv;
{
	long seed;
	register x, y, i;
	char *p;
	int t;

	if (argc > 1 && (*(p = argv[1]) < '0' || *p > '9')) {
		--argc;
		argv++;
		while (*p)
			switch (*p++) {
				case '-':
					pflag++;
					break;
				case '+':
					norand++;
					break;
				case 's':
					if (argc > 1) {
						if ((sfile=creat(argv[1],0644))
							< 0)
							sfile = 0;
						--argc;
						argv++;
					}
			}
	}
	if (argc > 2) {
		if ((dens1 = atoi(argv[1])) < 1)
			dens1 = 1;
		if ((dens2 = atoi(argv[2])) < 1)
			dens2 = 1;
		dens2 += dens1;
	}
	else if (argc > 1) {
		if ((dens2 = atoi(argv[1])) < 2)
			dens2 = 2;
		dens1 = 1;
	}
	time(&seed);
	srand(seed);
	if (norand == 0)
		prerand();
	initscr();
	signal(2, kil);
	gettype();
	for (;;) {
		x = lrand(ncol);
		y = lrand(nrow);
		i = mod(buf[x][y]++, dens2);
		if (i == 0 || i == dens1) {
		    mvaddch(y, x, (i == 0)? CH : ' ');
		    den += (i == 0)? 1 : -1;
		    if (pflag) {
			i = den*1000/nrow/ncol;
			move(nrow, 0);
			printw(".%3d", i);
		    }
		    refresh();
		}
		if (sfile) {
			t = den * 10000 / nrow / ncol;
			write(sfile, &t, 2);
		}
		if (_kill)
			break;
	}
	move(nrow, 0);
	refresh();
	exit(0);
}

prerand()
{
	register x, y, i;

	for (x = 0; x < ncol; x++)
	for (y = 0; y < nrow; y++) {
		i = lrand(dens2);
		buf[x][y] = i;
		if (i > 0 && i <= dens1)
			den++;
	}
}

char tbuf[1024];

gettype()
{
	register int i;
	char *t;

	if ((t = getenv("TERM"))==0 || tgetent(tbuf, t) <= 0)
		perr("Unable to determine terminal type");
	if ((i = tgetnum("li")) != -1)
		if ((nrow = i-1) > MAXNROW)
			nrow = MAXNROW;
	if ((i = tgetnum("co")) != -1)
		if ((ncol = i) > MAXNCOL)
			ncol = MAXNCOL;
}

kil()
{
	signal(2, 1);
	_kill = 1;
}

perr(mes)
char *mes;
{
	printf("%s\n", mes);
	exit(1);
}
