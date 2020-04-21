char _Origin_[] = "UC Berkeley";

#
/*
/* Aliens -- an animated video game
/*      the original version is from 
/*      Fall 1979                       Cambridge               Jude Miller
/*
/* Score keeping modified and general terminal handling (termcap routines
/* from UCB's ex) added by Rob Coben, BTL, June, 1980.
/*  Changes to use a nodelay pipe with unikx 3.0 added July 1980,
/*	by Clem Hergenhan, BTL.
/*
/* If MSG is defined, the program uses the inter-process message facility
/* of UNIX/TS Augmented to optimize reading the tty while updating the
/* screen. Otherwise, the child process (reading the tty) writes
/* a pipe which has no delay set for the reader.
/* If your system has a non-blocking read (read-without-wait), it should
/* be implemented, and the child process could be dispensed with entirely.
/*
/* That's what I've done (David Ungar, UCB).
*/
static char sccsid[] = "%W%";

#include <termio.h>
#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>

/*
 * defines
 */
#define YES	1
#define NO	0
#define MAXSCORE  "/usr/games/lib/alienslog"
#define EMPTY 'E'
#define FULL 'F'
#define LEFT ','
#define LLEFT 'z'
#define RIGHT '/'
#define LRIGHT 'c'
#define HALT '.'
#define LHALT 'x'
#define FIRE ' '
#define DELETE '\177'
#define ABORT '\34'
#define QUIT 'q'
#define GAME1 '1'
#define GAME2 '2'
#define GAME3 '3'
#define GAME4 '4'
#define REDRAW 12
#define BOMB_CNT 4
#define BOMB_MAX 20
#define BUF_SIZE 32
#define MINCOL 1
#define MIN 1
#define TIME 1
#define puts(str) fputs(str, stdout)
/*
/* global variables
*/
	int	fputchar();
        int scores,bases,game;
        int i,j,danger,max_danger;
        int flip,flop,left,al_num,b;
        int al_cnt,bmb_cnt;
	int slow = 0;
	int scorefile;
	int repeat;
	char lastch;
        char outbuf[BUF_SIZE + 1];
        char combuf[2];
        char inbuf[2];
	struct termio ttchar1;
	struct termio ttchar2;
        long timein;
        int timehi,timelo;      /* do not break up this declaration */
	int fd1 = 1;
        int nleft = BUF_SIZE;
        int ppid,cpid;
        char *nextfree = &outbuf[0];
       char barrinit [4] [81]  = {
                "         \43\43\43\43\43\43\43\43          \43\43\43\43\43\43\43\43          \43\43\43\43\43\43\43\43          \43\43\43\43\43\43\43\43         ",
                "        \43\43\43\43\43\43\43\43\43\43        \43\43\43\43\43\43\43\43\43\43        \43\43\43\43\43\43\43\43\43\43        \43\43\43\43\43\43\43\43\43\43        ",
                "        \43\43\43    \43\43\43        \43\43\43    \43\43\43        \43\43\43    \43\43\43        \43\43\43    \43\43\43        ",
                "        \43\43\43    \43\43\43        \43\43\43    \43\43\43        \43\43\43    \43\43\43        \43\43\43    \43\43\43        "
        };
        char barr [4] [80];
        struct
        {
                int row;
                int col;
        }
        al[55];
        struct {
                int row;
                int col;
                int vel;
        } bas;
        struct bem {
                int row;
                int col;
        } bem;
        struct bmb {
                int row;
                int col;
        }bmb[BOMB_CNT];
        struct {
                int val;
                int col;
                int vel;
        }shp;
        int scorsave[8];
/*
 * suspend -- Stop the game
 */

suspend()
{
	ioctl(0, TCSETA, &ttchar1);
#ifdef	SIGTSTP
	kill(0, SIGTSTP);
	signal(SIGTSTP, suspend);
#endif
	ioctl(0, TCSETA, &ttchar2);
	redraw();
}

/*
 * redraw -- draw the screen again
 */
redraw()
{
	register int i;

	clr();
	pos(0,0);
	printf("Score: %u",scores);
	pos(0,18);
	printf("I N V A S I O N   O F   T H E   A L I E N S !");
	pos(0,70);
	printf("Lasers: %d",bases);
	for (i=0;i<55;i++)   if (al[i].row!=0) {
		pos(al[i].row,al[i].col);
		ds_obj(((al[i].col+(al[i].row/2))&1) + (2*(i/22)));
	}
	for (i=0;i<=3;i++) {
		pos(i+19,0);
		puts(barr[i]);
	}
}

/*
/* pos -- positions cursor to a display position.  Row 0 is top-of-screen
/*      row 23 is bottom-of-screen.  The leftmost column is 0; the rightmost
/*      is 79.
*/
        pos(row,col)
        register int row,col;
        {
		extern char *vs_cm;
                tputs(tgoto(vs_cm,col,row), 1, fputchar);
                return;
        };

/*
/* clr -- issues an escape sequence to clear the display
*/
        clr()
        {
		extern char *vs_cl;
                tputs(vs_cl, 1, fputchar);
                return;
        };
/*
/* ds_obj -- display an object
*/
        ds_obj(class)
        register int class;
        {
                if ((game==4)&&(class>=0)&&(class<=5))   class = 6;
                switch (class)
                {
                        case 0: puts(" OXO ");
                                break;
                        case 1: puts(" XOX ");
                                break;
                        case 2: puts(" \\o/ ");
                                break;
                        case 3: puts(" /o\\ ");
                                break;
                        case 4: puts(" \"M\" ");
                                break;
                        case 5: puts(" wMw ");
                                break;
                        case 6: puts("     ");
                                break;
                        case 7: puts(" xx|xx ");
                                break;
                };
                return;
        };
/*
 * instructions -- print out instructions
 */
instruct() {
        clr();
        pos(0,0);
        printf("Attention: Alien invasion in progress!\15\n\n");
        printf("        Type:   <,>     to move the laser base left\15\n");
        printf("                <z>     as above, for lefties\15\n");
        printf("                <.>     to halt the laser base\15\n");
        printf("                <x>     for lefties\15\n");
        printf("                </>     to move the laser base right\15\n");
        printf("                <c>     for lefties\15\n");
        printf("                <space> to fire a laser beam\15\n\n");
        printf("                <1>     to play \"Bloodbath\"\15\n");
        printf("                <2>     to play \"We come in peace\"\15\n");
        printf("                <3>     to play \"Invasion of the Aliens\"\15\n");
        printf("                <4>     to play \"Invisible Alien Weasels\"\15\n");
        printf("                <q>     to quit\15\n\n");
        return;
}
/*
 * over -- game over processing
 */
over() {
	struct passwd *getpwuid(), *p;
        /*
         * display the aliens if they were invisible
         */
        if (game==4) {
                game = 3;       /* remove the cloak of invisibility */
                for (i=0;i<55;i++)   if (al[i].row!=0) {
                        pos(al[i].row,al[i].col);
                        ds_obj(((al[i].col+(al[i].row/2))&1) + (2*(i/22)));
                }
                game = 4;       /* be tidy */
        }
        /* high score to date processing */
	p = getpwuid(getuid());
        if ((scorefile=open(MAXSCORE,2)) != -1) {
                read(scorefile,scorsave,sizeof(scorsave));
                if (scorsave[(game*2)-2]<scores) {
                        scorsave[(game*2)-2] =scores;
			scorsave[(game*2)-1] = getuid();
                        lseek(scorefile,0l,0);
                        write(scorefile,scorsave,sizeof(scorsave));
			close(scorefile);
                }
		else {
			setpwent();
			p = getpwuid(scorsave[(game*2)-1]);
			scores = scorsave[(game*2)-2];
		}
        }
        pos(9,20);
        puts(" __________________________ ");
        pos(10,20);
        puts("|                          |");
        pos(11,20);
        puts("| G A M E   O V E R        |");
        pos(12,20);
        puts("|                          |");
        pos(13,20);
	printf("| Game type : %d            |",game);
	pos(14,20);
        printf("| High Score to date: %-5u|",scores);
        pos(15,20);
	if (p)
		printf("| Player : %-8s        |",p->pw_name);
	else
		printf("| Player : unknown     |");
        pos(16,20);
        puts("|__________________________|");
        leave();
}
/*
 * leave -- flush buffers,kill the Child, reset tty, and delete tempfile
 */
leave() {
        pos(23,0);
        sleep(1);
        ioctl(0, TCSETAW, &ttchar1);
        exit(0);
}
/*
/* init -- does global initialization and spawns a child process to read
/*      the input terminal.
*/
        init()
        {
                nice(2);       /* decrease priority */
                time(&timein);  /* get start time */
                time(&timehi);  /* get it again for seeding rand */
                srand(timelo);  /* start rand randomly */
		/*
		 * verify CRT and get proper cursor control sequence.
		 */
		vsinit();
                /*
                 * setup raw mode, no echo
                 */
		ioctl(0, TCGETA, &ttchar1);
		if (ttchar1.c_cflag & CBAUD < B9600)
			slow = 1;
		ttchar2 = ttchar1;
		/* turn off ICANON to detect user input */
		/* turn off ICANON, et al) */
		ttchar2.c_lflag = ISIG;
		ttchar2.c_iflag = 0;
		ttchar2.c_oflag = 0;
		ttchar2.c_cc[VMIN] = 1;
		ttchar2.c_cc[VTIME] = 1;
                ioctl(0, TCSETA, &ttchar2);
		signal(SIGINT, leave);

                /*
                 * New game starts here
                 */
                game = 0;
                instruct();
                while (game==0) poll();
                scores = 0;
                bases = 3;
                danger = 11;
                max_danger = 22;
                return;
        };

/*
/* tabl -- tableau draws the starting game tableau.
*/
        tabl()
{
                clr();
                pos(0,0);
                printf("Score: %u",scores);
                pos(0,18);
                puts("I N V A S I O N   O F   T H E   A L I E N S !");
                pos(0,70);
                printf("Lasers: %d",bases);

                /* initialize alien co-ords, display */

                al_cnt = 55;
                for (j=0;j<=4;j++)
                {
                        pos(danger-(2*j),0);
                        for (i=0;i<=10;i++)
                        {
                                ds_obj(((i+j)&1)+(2*(j/2)));
                                puts(" ");
                                al[(11*j)+i].row = danger - (2*j);
                                al[(11*j)+i].col = (6*i);
                        };
                };
                if (danger<max_danger)   danger++;
                al_num = 54;
                flip = 0;
                flop = 0;
                left = 0;
                /*
                 * initialize laser base position, velocity
                 */
                bas.row = 23;
                bas.col = 72;
                bas.vel = 0;
                bem.row = 0;
                /*
                 * initialize bomb arrays (row = 0 implies empty)
                 */
                for (i=0;i<BOMB_CNT;i++)   bmb[i].row = 0;
                b = 0;
                bmb_cnt = 0;
                /*
                 * initialize barricades
                 */
                for (i=0;i<=3;i++) {
                        pos(i+19,0);
                        puts(&barrinit [i] [0]);
                        for (j=0;j<80;j++)   barr [i] [j] = barrinit [i] [j];
                }
                /*
                 * initialize mystery ships
                 */
                shp.vel = 0;
                return;
        };
/*
/* poll -- read characters from input and set global flags
*/
        poll() {
		int retval;
		int recv_cv;
		unsigned count, t;
                /* if (slow)   sleep(10);  /* make it slower */
                if (game==1) {
                        if (bas.col<=1)   bas.vel = 1;
                        if (bas.col>=72)  bas.vel = -1;
                }
		fflush(stdout);
	top:
		retval =ioctl(0, FIONREAD, &count);
		/* stall awhile
		retval =ioctl(0, FIONREAD, &t);
		count += t;
		*/
		fflush(stdout);
		if (!count)
			return;
		read(0, combuf, 1);
		fflush(stdout);

		switch (combuf[0]&0177) {       /* do case char */
			case LLEFT:     ;
			case LEFT:      if (game==1)   break;
					bas.vel = -1;
					break;
			case LRIGHT:    ;
			case RIGHT:     if (game==1)   break;
					bas.vel = 1;
					break;
			case LHALT:     ;
			case HALT:      if (game==1)   break;
					bas.vel = 0;
					break;
			case FIRE:      if (bem.row==0)   bem.row = 22;
					break;
			case DELETE:    over();
					break;
			case ABORT:     over();
					break;
			case QUIT:      over();
					break;
			case GAME1:     if (game!=0)   break;
					game = 1;
					break;
			case GAME2:     if (game!=0)   break;
					game = 2;
					break;
			case GAME3:     if (game!=0)   break;
					game = 3;
					break;
			case GAME4:     if (game!=0)   break;
					game = 4;
					break;
			case REDRAW:	redraw();
					break;
		}
		goto top;
        }
/*
/* base -- move the laser base left or right
*/
        base()
        {
        bas.col += bas.vel;
        if (bas.col<1)   bas.col = 1;
        else if (bas.col>72)   bas.col = 72;
        pos(bas.row,bas.col);
        ds_obj(7);
        };

/*
/* beam -- activate or advance the laser beam if required
*/
        beam()
        {
	register struct bem *bp;
        /*
         * display beam
         */
	bp = &bem;
        switch (bp->row) {
                case 0:         return;
                case 21:        pos(21,bp->col);
                        puts("|");
                        break;
                case 22:bp->col = bas.col + 3;
                        pos(22,bp->col);
                        puts("|");
                        break;
                default:        pos(bp->row,bp->col);
                        puts("|\10\12\12 ");
                        break;
        }
        /*
         * check for contact with an alien
         */
        for (i=0;i<55;i++) {
                if ((al[i].row==bp->row)&&((al[i].col+1)<=bp->col)
                        &&((al[i].col+3)>=bp->col)) {
                        /*
                         * contact!
                         */
                        scores = scores + (i/22) + 1;   /* add points */
                        pos(0,7);
                        printf("%d",scores);
                        /* printf("\7");     don't ring the bell */
                        pos(bp->row+1,bp->col);
                        puts(" ");
                        pos(al[i].row,al[i].col);
                        ds_obj(6);      /* erase beam and alien */
                        bp->row=0;
                        al[i].row=0;    /* clear beam and alien state */
                        al_cnt--;
                        return;
                }
        }
        /*
         * check for contact with a bomb
         */
        for (i=0;i<BOMB_CNT;i++) {
                if ((bp->row==bmb[i].row)&&(bp->col==bmb[i].col)) {
                        pos(bp->row,bp->col);
                        puts(" \10\12 ");
                        bp->row = 0;
                        bmb_cnt--;
                        bmb[i].row = 0;
                        return;
                }
        }
        /*
         * check for contact with a barricade
         */
        if ((bp->row>=19)&&(bp->row<=22)&&(barr[bp->row-19][bp->col]!=' ')) {
                pos(bp->row,bp->col);
                puts(" \10\12 ");
                barr[bp->row-19][bp->col] = ' ';
                bp->row = 0;
                return;
        }
        /*
         * check for contact with a mystery ship
         */
        if ((shp.vel!=0)&&(bp->row==1)&&(bp->col>(i=shp.col-shp.vel))&&(bp->col<i+7)) {
                /*
                 * contact!
                 */
                pos(1,i);
                puts("        ");    /* erase ship */
                shp.vel = 0;
                scores += shp.val/3;
                pos(0,7);
                printf("%d",scores);
        }
        /*
         * update beam position
         */
        if ((--bp->row)==0) {
                pos(1,bp->col);
                puts(" ");
                pos(2,bp->col);
                puts(" ");
        }
        return;
        };

/*
/* bomb -- advance the next active bomb
*/
        bomb() {
		register struct bmb *bp;
                if (bmb_cnt==0)   return;
                while (1) {
                        if ((++b)>=BOMB_CNT)   b = 0;
                        if (bmb[b].row!=0)   break;
                }
		bp = &bmb[b];
                /*
                 * now advance the bomb, check for hit, and display
                 */
                bp->row++;
                if (bp->row==23) {
                        if ((bp->col>bas.col)&&
                            (bp->col<=(bas.col+5))) {
                                /*
                                 * the base is hit!
                                 */
                                bases--;
                                pos(0,70);
                                printf("Lasers: %d",bases);
                                /*
                                 * make heart-rending noise
                                 */
                                for (i=0;i<10;i++) {
					puts("\7\7\7\7\7\7\7\7\7\7");
					puts("\7\7\7\7\7\7\7\7\7\7");
					if (!slow) {
						puts("\7\7\7\7\7\7\7\7\7\7");
						puts("\7\7\7\7\7\7\7\7\7\7");
						puts("\7\7\7\7\7\7\7\7\7\7");
						puts("\7\7\7\7\7\7\7\7\7\7");
						puts("\7\7\7\7\7\7\7\7\7\7");
						puts("\7\7\7\7\7\7\7\7\7\7");
						puts("\7\7\7\7\7\7\7\7\7\7");
						puts("\7\7\7\7\7\7\7\7\7\7");
					}
				}
                                if (bases==0) {
                                        /*
                                         * game over
                                         */
                                        over();
                                }
                                sleep(2);
                                pos(23,bas.col);
                                puts("       ");
                                bas.col = 72;
                        }
                }
                if((bp->row>=19)&&(bp->row<23)&&(barr[bp->row-19][bp->col]!=' ')) {
                        /*
                         * the bomb has hit a barricade
                         */
                        pos(bp->row-1,bp->col);
                        puts(" \10\12*\10 ");
                        barr[bp->row-19][bp->col] = ' ';
                        bp->row = 0;
                        bmb_cnt--;
                        return;
                }
                pos(bp->row,bp->col);
                puts("*");
                pos(bp->row-1,bp->col);
                puts(" ");
                if (bp->row==23) {
                        bmb_cnt--;
                        pos(bp->row,bp->col);
                        puts(" ");
                        bp->row = 0;
                }
                return;
        }
/*
/* ship -- create or advance a mystery ship if desired
*/
        ship() {
		extern int vs_cols;

                if (shp.vel==0) {
                        if ((i=rand())&32) {
                                /*
                                 * create a mystery ship
                                 * this occurs about once every minute
                                 */
                                if (i&16) {
                                        shp.vel = -1;
                                        shp.col = vs_cols - 8;
                                }
                                else {
                                        shp.vel = 1;
                                        shp.col = MINCOL;
                                }
                                shp.val = 90;
                        }
                }
                else {
                        /*
                         * update an existing mystery ship
                         */
                        pos(1,shp.col);
                        if (game!=4)   printf(" <=%2d=> ",shp.val/3);
                        shp.val--;
                        shp.col += shp.vel;
                        if (((i=shp.col)>(vs_cols-8))||(i<MINCOL))   {
                                /*
                                 * remove the mystery ship
                                 */
                                pos(1,shp.col-shp.vel);
                                puts("        ");
                                shp.vel = 0;
                        }
                }
        }

/*
/* alien -- advance the next alien
*/
        alien()
        {
                while (1)
                {
                        if ((al_num = al_num + 1) >= 55)
                        {
                                if (al_cnt==0)   return; /* check if done */
                                flop = 0;
                                if (flip) { left = (left+1) % 2;
                                        flop = 1;
                                }
                                flip = 0;
                                al_num = 0;
                        }
                        if ((i = al[al_num].row)>0)   break;
                }
                if (i>=23)
                {

                        /* game over, aliens have overrun base */
                        over();
                }

                if (left)   al[al_num].col--;
                else   al[al_num].col++;
                if (((j = al[al_num].col)==0)||(j==75))   flip = 1;
                pos(i,j);
                if (flop) {
                        ds_obj(6);
                        i = ++al[al_num].row;
                        pos(i,j);
                }
                ds_obj(((j+(i/2))&1) + (2*(al_num/22)));
                /*
                 * check for bomb release
                 */
                if ((game==1)||(game==2))   return;     /* disable bombs */
                for (i=al_num-11;i>=0;i -= 11) {
                        if (al[i].row!=0)   return;
                }
                if ((al[al_num].col>=bas.col)&&(al[al_num].col<(bas.col+3))&&
                   (al[al_num].row<=BOMB_MAX)) {
                        for (i=0;i<BOMB_CNT;i++) {
                                if (bmb[i].row==0) {
                                        bmb[i].row = al[al_num].row;
                                        bmb[i].col = al[al_num].col + 2;
                                        bmb_cnt++;
                                        break;
                                }
                        }
                }
                return;
        };

/*
/* main -- scheduler and main entry point for aliens
*/
char bps[] = { 0,5,8,10,13,15,20,30,60,120,180,230,480,960,960,960 };
        main()
        {
		unsigned count;
		struct termio ttyb;
		int suspend();

		ioctl(0, TCGETA, &ttyb);
                init();
#ifdef SIGTSTP
		signal(SIGTSTP, suspend);
#endif
                while (1)
                {
                        tabl();
                        while (1)
                        {
                                poll();
                                beam();
                                base();
                                bomb();
                                ship();
/* take out the stall to try it
				if (!slow)
					puts("\0\0\0\0\0\0\0\0\0\0");
*/
				poll();
                                alien();
                                alien();
                                if (al_cnt==0)   break;
                        };
                };
        };
/* terminal type from environment and /etc/termcap ala ex & vi */
char	*vs_cl	= "";		/* clear screen sequence */
char	*vs_cm	= "";		/* cursor positioning sequence */
int	vs_rows;
int	vs_cols;
vsinit()
{
	char buf[1024];
	char tspace[256], *aoftspace;
	char *tgetstr();
	extern char *UP, *BC;	/* defined in tgoto.c (from ex's termlib */
	char *cp;

	if ((cp = (char *)getenv("TERM"))==0 || tgetent(buf, cp) <= 0)
		goto error;
	aoftspace = tspace;
	vs_cl = tgetstr("cl",&aoftspace);
	vs_cm = tgetstr("cm",&aoftspace);
	BC = tgetstr("bc",&aoftspace);
	UP = tgetstr("up",&aoftspace);
	vs_rows = tgetnum("li");
	vs_cols = tgetnum("co");
	if(vs_cl == 0 || *vs_cl == '\0' || vs_cm == 0 || *vs_cm == '\0') {
error:
	   puts("\nAliens is designed for CRT's with addressable cursors.\n");
	   puts("Verify that the TERM environment variable is a proper\n");
	   puts("type and is export-ed, and try it again...\n\n");
	   exit(1);
	}
}

fputchar(c)
int	c;
{
	putchar((char) c);
}
