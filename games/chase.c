char _Origin_[] = "UniSoft";

/*
 *	chase nrobots nfences
 */

#include <stdio.h>
#include <curses.h>

#define NX	60
#define NY	24
#define GNX	64	/* first power of 2 greater than NX */
#define GNY	32	/* first power of 2 greater than NY */

char grid[GNX][GNY];
int nrobots, nfences;
int kil();

main(argc, argv)
char **argv;
{
	init(argc, argv);
	lexit(doit());
}

init(argc, argv)
char **argv;
{
	register int i;
	register int x, y;
	char b[100];
	long t;

	if (argc > 1)
		nrobots = atoi(argv[1]);
	while (nrobots <= 1) {
		printf("number of robots: ");
		gets(b);
		nrobots = atoi(b);
	}
	nfences = -1;
	if (argc > 2)
		nfences = atoi(argv[2]);
	while (nfences < 0) {
		printf("number of fences: ");
		gets(b);
		nfences = atoi(b);
	}
	time(&t);
	srand(t);
	initscr();
	signal(2, kil);
	crmode();
	noecho();
	for (i = 0; i < nrobots; i++) {
		do {
			x = lrand(NX-2) + 1;
			y = lrand(NY-2) + 1;
		} while (grid[x][y]);
		grid[x][y] = '=';
	}
	for (i = 0; i < nfences; i++) {
		do {
			x = lrand(NX-2) + 1;
			y = lrand(NY-2) + 1;
		} while (grid[x][y]);
		grid[x][y] = '#';
	}
}

doit()
{
	int laststand = 0;
	int x, y;

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
		case 't': teleport(&x, &y); break;
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
	int flag;

	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		l[x][y] = 0;
	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		switch (grid[x][y]) {
		case '=':
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
	flag = 0;
	for (y = 1; y < NY-1; y++)
	for (x = 1; x < NX-1; x++)
		if (l[x][y] > 0) {
			if (x == ix && y == iy)
				munch(ix, iy);
			if (l[x][y] == 1) {
				flag++;
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
	move(iy, ix);
	refresh();
	return(!flag);
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
	move(20, NX+5); printw("# - fence");
	move(21, NX+5); printw("@ - junk heap");
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
	move(NY-1, 0);
	refresh();
	endwin();
	if (v == 0)
		printf("You have defeated all the robots\n");
	if (v == 1)
		printf("You have been eaten by robots\n");
	exit(v);
}

lrand(mod)
{
	return( (rand() >> 5) % mod );
}
