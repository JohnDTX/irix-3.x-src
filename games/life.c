char _Origin_[] = "System III";

/*
	life [-r]
		play the game of life (interactive for any video terminal)
	parameters
		-r - screen wraparound
	interactive options: (# stands for any string of digits)
		#,#a - add a # by # block (default 1 by 1)
		#c - step # generations (default forever)
		#,#d - delete a # by # block (default 1 by 1)
		#f - little flier, # determines direction
		#,#g - go to #,#
		#h - move left # steps (default 1)
		#j - move down # steps (default 1)
		#k - move up # steps (default 1)
		#l - move right # steps (default 1)
		#n - step # generations (default 1)
		p - put last d or y
		q - quit
		w - toggle wraparound flag
		#,#y - yank # by # block (default 1 by 1)
		C - clear screen
		#F - big flier, # determines direction
		H - move to left margin
		J - move to bottom margin
		K - move to top margin
		L - move to right margin
		#R - generate a random screen of density # %
		#^H - move left # steps (default 1)
		#^J - move down # steps (default 1)
		#^K - move up # steps (default 1)
		#^L - move right # steps (default 1)
		^R - redraw screen
		. - repeat last a d
		; - repeat last h j k l
	not implemented
		(S or Y)? - save in a file
		(R or P)? - restore from a file
		m - map
		! - shell escape
		e - edit a file
		i - input commands from file
 */

#include <stdio.h>
#include <curses.h>

#define NX	100
#define NY	50
int nx = NX;
int ny = NY;
int m[NX][NY];
int s[NX][NY];
int cx, cy;
int px, py;
int p[NX][NY];
int ngen;

#define CTRL_G	007
#define CTRL_H	010
#define CTRL_J	012
#define CTRL_K	013
#define CTRL_L	014
#define CTRL_R	022

#define CH	'#'

int rflag;
int kil(), _kill;

#define set(x, y) m[x][y] = 1, mvaddch(y, x, CH);
#define reset(x, y) m[x][y] = 0, mvaddch(y, x, ' ');
#define copy(x, y, v) m[x][y] = v, mvaddch(y, x, (v)?CH:' ');

struct xy {
	int xy_x;
	int xy_y;
};

main(argc, argv)
char **argv;
{
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'r')
		rflag++;
	init();
	while (doit()) ;
	move(ny-1, 0);
	refresh();
	endwin();
}

char tbuf[1024];

init()
{
	register int i;
	char *cp;
	long t;

	time(&t);
	srand((int)t);
	if ((cp = (char *)getenv("TERM")) == 0 || tgetent(tbuf, cp) <= 0) {
		fprintf(stderr, "can't get terminal type\n");
		exit(1);
	}
	if ((i = tgetnum("li")) != -1)
		if ((ny = i) > NY)
			ny = NY;
	if ((i = tgetnum("co")) != -1)
		if ((nx = i-1) > NX)
			nx = NX;
	initscr();
	lclear();
	signal(2, kil);
	crmode();
	noecho();
	cx = nx / 2;
	cy = ny / 2;
	move(cy, cx);
	refresh();
}

doit()
{
	register int x, y, ch;
	int count;
	int ni, n[2];
	int tx, ty;
	static int dotch, dotn[2];
	static int semich, semin;

	_kill = 0;
	move(cy, cx);
	refresh();
	ni = 0;
	n[0] = n[1] = 0;
LOOP:	ch = getch();
NLOOP:	switch(ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n[ni] = n[ni] * 10 + ch - '0';
			goto LOOP;
		case ',':			/* dual field separator */
			if (ni) goto BEEP;
			ni++;
			goto LOOP;
		case 'h': case CTRL_H:		/* move left */
			if (ni) goto BEEP;
			if (n[0] <= 0) n[0] = 1;
			if ((cx -= n[0]) < 0) cx = 0;
			move(cy, cx);
			semich = ch;
			semin = n[0];
			break;
		case 'H':			/* move to left margin */
			move(cy, cx=0);
			break;
		case 'j': case CTRL_J:		/* move down */
			if (ni) goto BEEP;
			if (n[0] <= 0) n[0] = 1;
			if ((cy += n[0]) >= ny) cy = ny-1;
			move(cy, cx);
			semich = ch;
			semin = n[0];
			break;
		case 'J':			/* move to bottom margin */
			move(cy=ny-1, cx);
			break;
		case 'k': case CTRL_K:		/* move up */
			if (ni) goto BEEP;
			if (n[0] <= 0) n[0] = 1;
			if ((cy -= n[0]) < 0) cy = 0;
			move(cy, cx);
			semich = ch;
			semin = n[0];
			break;
		case 'K':			/* move to top margin */
			move(cy=0, cx);
			break;
		case 'l': case CTRL_L:		/* move right */
			if (ni) goto BEEP;
			if (n[0] <= 0) n[0] = 1;
			if ((cx += n[0]) >= nx) cx = nx-1;
			move(cy, cx);
			semich = ch;
			semin = n[0];
			break;
		case 'L':			/* move to right margin */
			move(cy, cx=nx-1);
			break;
		case 'g': 			/* goto */
			if ((cx = n[0]) >= nx) cx = nx - 1;
			if ((cy = n[1]) >= ny) cy = ny - 1;
			move(cy, cx);
			break;
		case 'a': 			/* add a block */
			if (n[0] <= 0) n[0] = 1;
			if (n[1] <= 0) n[1] = 1;
			if ((tx = n[0] + cx) > nx) tx = nx;
			if ((ty = n[1] + cy) > ny) ty = ny;
			for (x = cx; x < tx; x++)
			for (y = cy; y < ty; y++)
				set(x, y);
			dotch = ch;
			dotn[0] = n[0];
			dotn[1] = n[1];
COUNT:
			count = 0;
			for (x = 0; x < nx; x++)
			for (y = 0; y < ny; y++)
				if (m[x][y])
					count++;
			move(ny-1, 0);
			printw("%d:%d   ", ngen, count, x, y);
			break;
		case 'd': 			/* delete a block */
			if (n[0] <= 0) n[0] = 1;
			if (n[1] <= 0) n[1] = 1;
			if ((tx = n[0] + cx) > nx) tx = nx;
			if ((ty = n[1] + cy) > ny) ty = ny;
			px = tx - cx;
			py = ty - cy;
			for (x = cx; x < tx; x++)
			for (y = cy; y < ty; y++) {
				p[x-cx][y-cy] = m[x][y];
				reset(x, y);
			}
			dotch = ch;
			dotn[0] = n[0];
			dotn[1] = n[1];
			goto COUNT;
		case 'y': 			/* yank a block */
			if (n[0] <= 0) n[0] = 1;
			if (n[1] <= 0) n[1] = 1;
			if ((tx = n[0] + cx) > nx) tx = nx;
			if ((ty = n[1] + cy) > ny) ty = ny;
			px = tx - cx;
			py = ty - cy;
			for (x = cx; x < tx; x++)
			for (y = cy; y < ty; y++)
				p[x-cx][y-cy] = m[x][y];
			break;
		case 'p':			/* put a block */
			if ((tx=px)+cx > nx) tx = nx - cx;
			if ((ty=py)+cy > ny) ty = ny - cy;
			for (x = 0; x < tx; x++)
			for (y = 0; y < ty; y++)
				copy(x+cx, y+cy, p[x][y]);
			goto COUNT;
		case 'c': 			/* continuous step */
			if (ni) goto BEEP;
			x = n[0];
			do {
				step();
				if (_kill)
					break;
			} while (--x);
			break;
		case 'n': 			/* step N */
			if (ni) goto BEEP;
			x = n[0];
			if (x <= 0) x = 1;
			do {
				step();
				if (_kill)
					break;
			} while (--x);
			break;
		case 'q': 			/* quit */
			return (0);
		case 'C': 			/* clear */
			lclear();
			break;
		case CTRL_R:			/* redraw */
			clear();
			count = 0;
			for (x = 0; x < nx; x++)
			for (y = 0; y < ny; y++)
				if (m[x][y])
					mvaddch(y, x, CH);
			goto COUNT;
		case 'R':			/* redraw */
			if (n[0] <= 0) n[0] = 50;
			lclear();
			count = 0;
			for (x = 0; x < nx; x++)
			for (y = 0; y < ny; y++)
				if ((rand() % 100) < n[0])
					set(x, y);
			goto COUNT;
		case '.':			/* repeat last a or d */
			ch = dotch;
			ni = 1;
			n[0] = dotn[0];
			n[1] = dotn[1];
			goto NLOOP;
		case ';':			/* repeat last move */
			ch = semich;
			ni = 0;
			n[0] = semin;
			goto NLOOP;
		case 'f':
			if (flier(n[0]))
				goto BEEP;
			goto COUNT;
		case 'F':
			if (Flier(n[0]))
				goto BEEP;
			goto COUNT;
		case 'w':
			move(ny-1, 0);
			printw("wrap %s", (rflag = !rflag)? "on" : "off");
			break;
		case 'i':			/* input from file */
		case 'e':			/* edit */
		case '!':			/* shell escape */
BEEP:		default:
			putchar(CTRL_G);
	}
	move(cy, cx);
	refresh();
	return (1);
}

lclear()
{
	register int x, y;

	for (x = 0; x < nx; x++)
	for (y = 0; y < ny; y++)
		m[x][y] = 0;
	clear();
	ngen = 0;
}

struct xy flierd[8][5] = {
	1,0, 0,1, 0,2, 1,2, 2,2,
	0,0, 1,0, 0,1, 2,1, 0,2,
	0,0, 1,0, 2,0, 2,1, 1,2,
	2,0, 0,1, 2,1, 1,2, 2,2,
	1,0, 2,1, 0,2, 1,2, 2,2,
	0,0, 0,1, 2,1, 0,2, 1,2,
	0,0, 1,0, 2,0, 0,1, 1,2,
	1,0, 2,0, 0,1, 2,1, 2,2
};

flier(v)
{
	if (cx+3 > nx || cy+3 > ny)
		return (-1);
	setup(5, flierd[v%8]);
	return (0);
}

struct xy Flierd[8][9] = {
	1,0, 0,1, 0,2, 5,2, 0,3, 1,3, 2,3, 3,3, 4,3,
	0,0, 1,0, 2,0, 0,1, 3,1, 0,2, 0,3, 0,4, 1,5,
	1,0, 2,0, 3,0, 4,0, 5,0, 0,1, 5,1, 5,2, 4,3,
	2,0, 3,1, 3,2, 3,3, 0,4, 3,4, 1,5, 2,5, 3,5,
	4,0, 5,1, 0,2, 5,2, 1,3, 2,3, 3,3, 4,3, 5,3,
	1,0, 0,1, 0,2, 0,3, 0,4, 3,4, 0,5, 1,5, 2,5,
	0,0, 1,0, 2,0, 3,0, 4,0, 0,1, 5,1, 0,2, 1,3,
	1,0, 2,0, 3,0, 0,1, 3,1, 3,2, 3,3, 3,4, 2,5
};

Flier(v)
{
	v %= 8;
	if (v & 1) {
		if (cx+4 > nx || cy+6 > ny)
			return (-1);
	}
	else {
		if (cx+6 > nx || cy+4 > ny)
			return (-1);
	}
	setup(9, Flierd[v]);
	return (0);
}

setup(n, p)
register int n;
register struct xy *p;
{
	register int lx, ly;

	do {
		set(lx=cx+p->xy_x, ly=cy+p->xy_y);
		p++;
	} while (--n);
}

step()
{
	register int x, y;
	int count = 0;

	/* clear s */
	for (x = 0; x < nx; x++)
	for (y = 0; y < ny; y++)
		s[x][y] = 0;
	/* corners */
	if (m[0][0]) {
		s[0][1]++;
		s[1][0]++;
		s[1][1]++;
		if (rflag) {
			s[nx-1][0]++;
			s[nx-1][1]++;
			s[0][ny-1]++;
			s[1][ny-1]++;
			s[nx-1][ny-1]++;
		}
	}
	if (m[nx-1][0]) {
		s[nx-2][0]++;
		s[nx-2][1]++;
		s[nx-1][1]++;
		if (rflag) {
			s[0][0]++;
			s[0][1]++;
			s[nx-2][ny-1]++;
			s[nx-1][ny-1]++;
			s[0][ny-1]++;
		}
	}
	if (m[0][ny-1]) {
		s[0][ny-2]++;
		s[1][ny-2]++;
		s[1][ny-1]++;
		if (rflag) {
			s[nx-1][ny-2]++;
			s[nx-1][ny-1]++;
			s[0][0]++;
			s[1][0]++;
			s[nx-1][0]++;
		}
	}
	if (m[nx-1][ny-1]) {
		s[nx-2][ny-2]++;
		s[nx-2][ny-1]++;
		s[nx-1][ny-2]++;
		if (rflag) {
			s[0][ny-2]++;
			s[0][ny-1]++;
			s[nx-2][0]++;
			s[nx-1][0]++;
			s[0][0]++;
		}
	}
	/* edges */
	for (x = 1; x < nx-1; x++) {
		if (m[x][0]) {
			s[x-1][0]++;
			s[x+1][0]++;
			s[x-1][1]++;
			s[x][1]++;
			s[x+1][1]++;
			if (rflag) {
				s[x-1][ny-1]++;
				s[x][ny-1]++;
				s[x+1][ny-1]++;
			}
		}
		if (m[x][ny-1]) {
			s[x-1][ny-1]++;
			s[x+1][ny-1]++;
			s[x-1][ny-2]++;
			s[x][ny-2]++;
			s[x+1][ny-2]++;
			if (rflag) {
				s[x-1][0]++;
				s[x][0]++;
				s[x+1][0]++;
			}
		}
	}
	for (y = 1; y < ny-1; y++) {
		if (m[0][y]) {
			s[0][y-1]++;
			s[0][y+1]++;
			s[1][y-1]++;
			s[1][y]++;
			s[1][y+1]++;
			if (rflag) {
				s[nx-1][y-1]++;
				s[nx-1][y]++;
				s[nx-1][y+1]++;
			}
		}
		if (m[nx-1][y]) {
			s[nx-1][y-1]++;
			s[nx-1][y+1]++;
			s[nx-2][y-1]++;
			s[nx-2][y]++;
			s[nx-2][y+1]++;
			if (rflag) {
				s[0][y-1]++;
				s[0][y]++;
				s[0][y+1]++;
			}
		}
	}
	/* rest */
	for (x = 1; x < nx-1; x++)
	for (y = 1; y < ny-1; y++)
		if (m[x][y]) {
			s[x-1][y-1]++;
			s[x-1][y]++;
			s[x-1][y+1]++;
			s[x][y-1]++;
			s[x][y+1]++;
			s[x+1][y-1]++;
			s[x+1][y]++;
			s[x+1][y+1]++;
		}
	/* update m */
	for (x = 0; x < nx; x++)
	for (y = 0; y < ny; y++)
		if (m[x][y]) {
			if (s[x][y] != 2 && s[x][y] != 3) {
				m[x][y] = 0;
				mvaddch(y, x, ' ');
			}
			else
				count++;
		}
		else {
			if (s[x][y] == 3) {
				m[x][y] = 1;
				mvaddch(y, x, CH);
				count++;
			}
		}
	ngen++;
	move(ny-1, 0);
	printw("%d:%d   ", ngen, count, x, y);
	refresh();
}

kil()
{
	signal(2, kil);
	_kill++;
}
