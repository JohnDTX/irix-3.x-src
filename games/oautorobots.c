char _Origin_[] = "UniSoft";

/*
 *	autorobots
 */

#include <stdio.h>
#include <curses.h>
#include <setjmp.h>

#define NX	60
#define NY	24
#define GNX	64	/* first power of 2 greater than NX */
#define GNY	32	/* first power of 2 greater than NY */
#define NLIST	200	/* maximum number of robots */

#define SCOREFILE	"/usr/games/lib/arscore"
#define LOGFILE		"/usr/games/lib/arlog"

#define T_1	1
#define T_2	2
#define T_3	3
#define T_4	4
#define T_5	5
#define T_6	6
#define T_7	7
#define T_8	8
#define T_9	9
#define T_q	10
#define T_014	11

char cswitch[128];
char grid[GNX][GNY];
int nrobots, score;
struct list {
	struct list *link;
	int x;
	int y;
} *list;
int kil();

jmp_buf jb;

main(argc, argv)
char **argv;
{
	cswitch['1'] = T_1;	cswitch['n'] = T_1;
	cswitch['2'] = T_2;	cswitch['m'] = T_2;
	cswitch['3'] = T_3;	cswitch[','] = T_3;
	cswitch['4'] = T_4;	cswitch['h'] = T_4;
	cswitch['5'] = T_5;	cswitch['j'] = T_5;	cswitch[' '] = T_5;
	cswitch['6'] = T_6;	cswitch['k'] = T_6;
	cswitch['7'] = T_7;	cswitch['y'] = T_7;
	cswitch['8'] = T_8;	cswitch['u'] = T_8;
	cswitch['9'] = T_9;	cswitch['i'] = T_9;
	cswitch['q'] = T_q;
	cswitch[014] = T_014;
	init();
	setjmp(jb);
	score = 0;
	border();
	for (nrobots = 10; ; nrobots += 10) {
		list = (struct list *)malloc(sizeof(struct list) * nrobots);
		doit();
		free(list);
		clinsides();
	}
}

init()
{
	long t;

	time(&t);
	srand(t);
	initscr();
	signal(2, kil);
	crmode();
	noecho();
}

#define KK	100
doit()
{
	register int i, x, y;
	register int tx, ty;
	int mtx, mty;
	register struct list *lp, *tlp;
	register char *gp;
	int kk, delay;
	int r;

	kk = nrobots;
	delay = 0;
	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		grid[x][y] = 0;
	for (i = 0; i < nrobots; i++) {
		do {
			x = lrand(NX-2) + 1;
			y = lrand(NY-2) + 1;
		} while (grid[x][y]);
		grid[x][y] = '=';
		list[(short)i].link = &list[(short)(i+1)];
		list[(short)i].x = x;
		list[(short)i].y = y;
	}
	lp = &list[(short)(nrobots-1)];
	lp->link = &list[0];
	do {
		x = lrand(NX-2) + 1;
		y = lrand(NY-2) + 1;
	} while (grid[x][y]);
	grid[x][y] = 'I';
	insides(x, y);
	for (;;) {	/* flush any type ahead */
		ioctl(0, FIONREAD, &mtx);
		if (mtx <= 0)
			goto GETCHAR;
		getchar();
	}
LOOP:	ioctl(0, FIONREAD, &mtx);
	if (mtx) {
GETCHAR:	grid[x][y] = 0;
		mvaddch(y, x, ' ');
		switch (cswitch[getchar()&0x7F]) {
			case T_7:
				if (x > 1) --x; if (y > 1) --y; break;
			case T_8:
				if (y > 1) --y; break;
			case T_9:
				if (x < NX-2) x++; if (y > 1) --y; break;
			case T_4:
				if (x > 1) --x; break;
			case T_5:
				break;
			case T_6:
				if (x < NX-2) x++; break;
			case T_1:
				if (x > 1) --x; if (y < NY-2) y++; break;
			case T_2:
				if (y < NY-2) y++; break;
			case T_3:
				if (x<NX-2) x++; if (y < NY-2) y++; break;
			case T_q:
				lexit(2);
			case T_014:
				border(); insides(x, y); goto LOOP;
			default:	break;
		}
		gp = &grid[x][y];
		if (*gp)
			munch(x, y);
		*gp = 'I';
		mvaddch(y, x, 'I');
		refresh();
		goto LOOP;
	}
	if ((delay += kk) < KK)
		goto LOOP;
	delay = 0;
	/* move a robot */
	tlp = lp->link;
	tx = tlp->x;
	ty = tlp->y;
	gp = &grid[tx][ty];
	if (*gp == '@') {
UNLINK:		score += nrobots;
		move(22, NX+5); printw("%d", score);
		move(y, x);
		refresh();
		if (lp == tlp)
			return;
		--kk;
		lp->link = tlp->link;
		goto LOOP;
	}
	*gp = 0;
	mvaddch(ty, tx, ' ');
	if (tx < x) tx++;
	else if (tx > x) --tx;
	if (ty < y) ty++;
	else if (ty > y) --ty;
	if (tx == x && ty == y)
		munch(x, y);
	gp = &grid[tx][ty];
	if (*gp == '=') {
		*gp = '@';
		mvaddch(ty, tx, '@');
		goto UNLINK;
	}
	if (*gp == '@')
		goto UNLINK;
	*gp = '=';
	mvaddch(ty, tx, '=');
	move(y, x);
	if ((r++ & 3) == 0 || kk < 4)
		refresh();
	lp = tlp;
	lp->x = tx;
	lp->y = ty;
	goto LOOP;
}

border()
{
	register int x, y;

	clear();
	/* border */
	for (x = 1; x < NX-1; x++) {
		mvaddch(0, x, '-');
		mvaddch(NY-1, x, '-');
	}
	for (x = 0; x < NY; x++) {
		mvaddch(x, 0, '|');
		mvaddch(x, NX-1, '|');
	}
	/* instructions */
	move(1, NX+5); printw("MOVES");
	move(3, NX+5); printw("y u i");
	move(4, NX+6); printw("\\|/");
	move(5, NX+5); printw("h-j-k");
	move(6, NX+6); printw("/|\\");
	move(7, NX+5); printw("n m ,");
	move(9, NX+5); printw("COMMANDS");
	move(11, NX+5); printw("q - quit");
	move(12, NX+5); printw("^L - redraw");
	move(14, NX+5); printw("KEY");
	move(16, NX+5); printw("I - you");
	move(17, NX+5); printw("= - robot");
	move(18, NX+5); printw("@ - junk heap");
	move(21, NX+5); printw("SCORE");
	move(22, NX+5); printw("%d", score);
}

insides(ix, iy)
{
	register int x, y;

	/* robots */
	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		if (grid[x][y])
			mvaddch(y, x, grid[x][y]);
	/* I */
	mvaddch(iy, ix, 'I');
	move(iy, ix);
	refresh();
}

clinsides(ix, iy)
{
	register int x, y;

	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		if (grid[x][y]) {
			grid[x][y] = 0;
			mvaddch(y, x, ' ');
		}
}

kil()
{
	signal(2, 1);
	lexit(2);
}

munch(x, y)
{
	int c;

	if (x > NX-9)
		x = NX-9;
	move(y, x);
	printw("*MUNCH*");
	refresh();
	sleep(2);
	ioctl(0, TCFLSH, 0);
	move(y, x);
	printw("       ");
	clinsides();
	scorit();
	move(17, 10);
	printw("Quit?");
	refresh();
	c = getchar();
	if (c == 'y' || c == 'Y' || c == EOF)
		lexit(1);
	free(list);
	longjmp(jb, 1);
}

lexit(v)
{
	signal(2, 1);
	move(NY-1, 0);
	refresh();
	endwin();
	printf("\n");
	exit(v);
}

struct scores {
	char name[10];
	int score;
} scores[10];

scorit()
{
	int me, n, i;
	FILE *fp;
	char *p, *getenv();

	if ((fp = fopen(SCOREFILE, "r")) == NULL)
		n = 0;
	else {
		for (n = 0; n < 10; n++)
			if (fscanf(fp, "%s %d", scores[(short)n].name,
				&scores[(short)n].score) != 2)
					break;
		fclose(fp);
	}
	for (me = 0; me < n; me++)
		if (score > scores[(short)me].score)
			break;
	if (me < 10)
		for (i = n; i > me; --i)
			scores[(short)i] = scores[(short)(i-1)];
	if ((p = getenv("LOGNAME")) == NULL)	/* System III */
		if ((p = getenv("USER")) == NULL)	/* v7 vax */
			if ((p = getenv("NAME")) == NULL)	/* v7 pdp */
				p = "(NULL)";
	strncpy(scores[(short)me].name, p, 10);
	scores[(short)me].score = score;
	if (n < 10)
		n++;
	if ((fp = fopen(SCOREFILE, "w")) == NULL)
		goto LOG;
	move(5, 10);
	printw("HIGH SCORES TO DATE");
	for (i = 0; i < n; i++) {
		fprintf(fp, "%8.8s %10d\n", scores[(short)i].name,
			scores[(short)i].score);
		move(i+6, 10);
		printw("%2d %8.8s %10d", i+1, scores[(short)i].name,
			scores[(short)i].score);
	}
	fclose(fp);
LOG:
	if (access(LOGFILE, 2))
		return;
	if ((fp = fopen(LOGFILE, "a")) == NULL)
		return;
	fprintf(fp, "%10d\n", score);
	fclose(fp);
}

lrand(mod)
{
	return( (rand() >> 5) % mod );
}
