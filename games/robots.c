char _Origin_[] = "UniSoft";

/*
 *	robots
 */

#include <stdio.h>
#include <curses.h>

#define NX	60
#define NY	24
#define GNX	64	/* first power of 2 greater than NX */
#define GNY	32	/* first power of 2 greater than NY */

#define SCOREFILE	"/usr/games/lib/robotscore"

char grid[GNX][GNY];
int nrobots, score;
int kil();

main(argc, argv)
char **argv;
{
	init();
	for (nrobots = 10; ; nrobots += 10)
		doit();
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

doit()
{
	int laststand = 0;
	register int i, x, y;
	int tx, ty;

	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		grid[x][y] = 0;
	for (i = 0; i < nrobots; i++) {
		do {
			x = lrand(NX-2) + 1;
			y = lrand(NY-2) + 1;
		} while (grid[x][y]);
		grid[x][y] = '=';
	}
	do {
		x = lrand(NX-2) + 1;
		y = lrand(NY-2) + 1;
	} while (grid[x][y]);
	grid[x][y] = 'I';
	redraw(x, y);
    for (;;) {
	grid[x][y] = 0;
	mvaddch(y, x, ' ');
	if (laststand == 0) switch (getchar()) {
		case 'y': if (x > 1) --x; if (y > 1) --y; break;
		case 'u': if (y > 1) --y; break;
		case 'i': if (x < NX-2) x++; if (y > 1) --y; break;
		case 'h': if (x > 1) --x; break;
		case 'j': case ' ': break;
		case 'k': if (x < NX-2) x++; break;
		case 'n': if (x > 1) --x; if (y < NY-2) y++; break;
		case 'm': if (y < NY-2) y++; break;
		case ',': if (x<NX-2) x++; if (y < NY-2) y++; break;
		case 't': teleport(&tx, &ty); x = tx; y = ty; break;
		case 's':	laststand++;	break;
		case 'q':	lexit(2);
		case '\014':	redraw(x, y);	continue;
		default:	continue;
	}
	if (grid[x][y])
		munch(x, y);
	grid[x][y] = 'I';
	mvaddch(y, x, 'I');
	refresh();
	if (robots(x, y))
		return(0);
    }
}

teleport(px, py)
int *px, *py;
{
	register int x, y;

	x = *px;
	y = *py;
	x = lrand(NX-2) + 1;
	y = lrand(NY-2) + 1;
	*px = x;
	*py = y;
}

robots(ix, iy)
{
	register int x, y;
	int tx, ty;
	char l[GNX][GNY];
	int nb, na;

	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		l[x][y] = 0;
	nb = 0;
	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		switch (grid[x][y]) {
		case '=':
			nb++;
			tx = x;
			ty = y;
			if (tx < ix) tx++;
			if (tx > ix) --tx;
			if (ty < iy) ty++;
			if (ty > iy) --ty;
			l[tx][ty]++;
			break;
		case '@':
			l[x][y] = 2;
			break;
		case '#':
			l[x][y] = -10000;
			break;
		}
	na = 0;
	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		if (l[x][y] > 0) {
			if (x == ix && y == iy)
				munch(ix, iy);
			if (l[x][y] == 1) {
				na++;
				if (grid[x][y] != '=') {
					grid[x][y] = '=';
					mvaddch(y, x, '=');
					refresh();
				}
			} else {
				if (grid[x][y] != '@') {
					grid[x][y] = '@';
					mvaddch(y, x, '@');
					refresh();
				}
			}
		} else if (l[x][y] == 0) {
			if (grid[x][y] == '=') {
				grid[x][y] = 0;
				mvaddch(y, x, ' ');
				refresh();
			}
		}
	score += (nb - na) * nrobots;
	move(23, NX+5); printw("%d", score);
	move(iy, ix);
	refresh();
	return(!na);
}

redraw(ix, iy)
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
	move(11, NX+5); printw("t - teleport");
	move(12, NX+5); printw("s - last stand");
	move(13, NX+5); printw("q - quit");
	move(14, NX+5); printw("^L - redraw");
	move(16, NX+5); printw("KEY");
	move(18, NX+5); printw("I - you");
	move(19, NX+5); printw("= - robot");
	move(20, NX+5); printw("@ - junk heap");
	move(22, NX+5); printw("SCORE");
	move(23, NX+5); printw("%d", score);
	/* robots/fences/I */
	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		if (grid[x][y]) {
			mvaddch(y, x, grid[x][y]);
		}
	mvaddch(iy, ix, 'I');
	move(iy, ix);
	refresh();
}

kil()
{
	signal(2, 1);
	lexit(2);
}

munch(x, y)
{
	if (x > NX-7)
		x = NX-7;
	move(y, x);
	printw("*MUNCH*");
	refresh();
	sleep(5);
	lexit(1);
}

lexit(v)
{
	signal(2, 1);
	move(NY-1, 0);
	refresh();
	endwin();
	scorit();
	printf("Your score is %d\n", score);
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
			if (fscanf(fp, "%s %d", scores[n].name,
				&scores[n].score) != 2)
					break;
		fclose(fp);
	}
	for (me = 0; me < n; me++)
		if (score > scores[me].score)
			break;
	if (me < 10)
		for (i = n; i > me; --i)
			scores[i] = scores[i-1];
	if ((p = getenv("LOGNAME")) == NULL)	/* System III */
		if ((p = getenv("USER")) == NULL)	/* v7 vax */
			if ((p = getenv("NAME")) == NULL)	/* v7 pdp */
					p = "(NULL)";
	strncpy(scores[me].name, p, 10);
	scores[me].score = score;
	if (n < 10)
		n++;
	if ((fp = fopen(SCOREFILE, "w")) == NULL)
		return;
	printf("HIGH SCORES TO DATE\n");
	for (i = 0; i < n; i++) {
		fprintf(fp, "%8.8s %10d\n", scores[i].name, scores[i].score);
		printf("%2d %8.8s %10d\n", i+1,
			scores[i].name, scores[i].score);
	}
	fclose(fp);
}

lrand(mod)
{
	return( (rand() >> 5) % mod );
}
