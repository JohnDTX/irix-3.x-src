char _Origin_[] = "Caltech";

#include <stdio.h>
#include <signal.h>
#include <curses.h>
/* rain 11/3/1980 EPS/CITHEP */
/* cc rain.c -o rain -O -ltermlib */
#define cursor(col,row) tputs(tgoto(CM,col,row), 1, fputchar)
extern char *UP;
struct termio old_tty;
char *LL, *TE, *TI;
main(argc,argv)
int argc;
char *argv[];
{
    extern fputchar();
    char *malloc();
    char *getenv();
    char *tgetstr(), *tgoto();
    int onsig();
    register int x, y, j;
    static int xpos[5], ypos[5];
    register char *CM, *BC, *DN, *ND;
    char *tcp;
    register char *term;
    char tcb[100];
    struct termio cur_tty;

    setbuf(stdout,malloc(BUFSIZ));
    if (!(term=getenv("TERM"))) {
	fprintf(stderr,"%s: TERM: parameter not set\n",*argv);
	exit(1);
    }
    if (tgetent(malloc(1024),term)<=0) {
	fprintf(stderr,"%s: %s: unknown terminal type\n",*argv,term);
	exit(1);
    }
    tcp=tcb;
    if (!(CM=tgetstr("cm",&tcp))) {
	fprintf(stderr,"%s: terminal not capable of cursor motion\n",*argv);
	exit(1);
    }
    if (!(BC=tgetstr("bc",&tcp))) BC="\b";
    if (!(DN=tgetstr("dn",&tcp))) DN="\n";
    if (!(ND=tgetstr("nd",&tcp))) ND=" ";
    TE=tgetstr("te",&tcp);
    TI=tgetstr("ti",&tcp);
    UP=tgetstr("up",&tcp);
    if (!(LL=tgetstr("ll",&tcp))) strcpy(LL=malloc(10),tgoto(CM,0,23));
    for (j=SIGHUP;j<=SIGTERM;j++)
	if (signal(j,SIG_IGN)!=SIG_IGN) signal(j,onsig);
    ioctl(1,TCGETA,&old_tty);
    ioctl(1,TCGETA,&cur_tty);
    cur_tty.c_iflag &= ~ICRNL;
    cur_tty.c_oflag &= ~ONLCR;
    cur_tty.c_lflag &= ~ECHO;
    ioctl(1,TCSETA,&cur_tty);
    if (TI) tputs(TI,1, fputchar);
    tputs(tgetstr("cl",&tcp),1,fputchar);
    fflush(stdout);
    for (j=5;--j>=0;) {
	xpos[j]=(int)(rand()%76)+2;
	ypos[j]=(int)(rand()%20)+2;
    }
    for (j=0;;) {
	x=(int)(rand()%76)+2;
	y=(int)(rand()%20)+2;
	cursor(x,y); fputchar('.');
	cursor(xpos[j],ypos[j]); fputchar('o');
	if (j==0) j=4; else --j;
	cursor(xpos[j],ypos[j]); fputchar('O');
	if (j==0) j=4; else --j;
	cursor(xpos[j],ypos[j]-1);
	fputchar('-');
	tputs(DN,1, fputchar); tputs(BC,1, fputchar); tputs(BC,1, fputchar);
	fputs("|.|",stdout);
	tputs(DN,1, fputchar); tputs(BC,1, fputchar); tputs(BC,1, fputchar);
	fputchar('-');
	if (j==0) j=4; else --j;
	cursor(xpos[j],ypos[j]-2); fputchar('-');
	tputs(DN,1, fputchar); tputs(BC,1, fputchar); tputs(BC,1, fputchar);
	fputs("/ \\",stdout);
	cursor(xpos[j]-2,ypos[j]);
	fputs("| O |",stdout);
	cursor(xpos[j]-1,ypos[j]+1);
	fputs("\\ /",stdout);
	tputs(DN,1, fputchar); tputs(BC,1, fputchar); tputs(BC,1, fputchar);
	fputchar('-');
	if (j==0) j=4; else --j;
	cursor(xpos[j],ypos[j]-2); fputchar(' ');
	tputs(DN,1, fputchar); tputs(BC,1, fputchar); tputs(BC,1, fputchar);
	fputchar(' '); tputs(ND,1, fputchar); fputchar(' ');
	cursor(xpos[j]-2,ypos[j]);
	fputchar(' '); tputs(ND,1, fputchar); fputchar(' ');
	tputs(ND,1, fputchar); fputchar(' ');
	cursor(xpos[j]-1,ypos[j]+1);
	fputchar(' '); tputs(ND,1, fputchar); fputchar(' ');
	tputs(DN,1, fputchar); tputs(BC,1, fputchar); tputs(BC,1, fputchar);
	fputchar(' ');
	xpos[j]=x; ypos[j]=y;
	fflush(stdout);
    }
}
onsig(n)
int n;
{
    tputs(LL,1, fputchar);
    if (TE) tputs(TE,1, fputchar);
    fflush(stdout);
    ioctl(1, TCSETA, &old_tty);	/*reset old terminal mode*/
    kill(getpid(),n);
    _exit(0);
}
fputchar(c)
char c;
{
    putchar(c);
}
