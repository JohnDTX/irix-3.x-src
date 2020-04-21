#include "defs.h"
/****************************************************************************

 TRIX DEBUGGER

****************************************************************************/
MSG BADMOD;
MSG NOFORK;
MSG ADWRAP;
SYMPTR symbol;
int mkfault;
char *lp;
int maxoff;
int (*sigint)();
int (*sigqit)();
char * errflg;
char lastc;
long int dot;
int dotinc;
typedef /* algol-like statement definitions */ union {
	long float d;
	struct{
		long int sa;
		short sb,sc;
	} s ;
} L_JUNK; /* GB */


scanform(icount,ifp,itype,ptype)
long int icount;
char * ifp;
{
	char * fp;
	char modifier;
	int fcount, init=1;
	long int savdot;

	while( icount)
	{
		fp=ifp;
		if( init==0 && findsym(shorten(dot),ptype)==0 && maxoff)
		{
			printf("\n%s:%16t",symbol->symc);
		}
		savdot=dot;
		init=0;

		/*now loop over format*/
		while( *fp && errflg==0)
		{
			if( digit(modifier = *fp))
			{
				fcount=0;
				while( digit(modifier = *fp++))
				{
					fcount *= 10;
					fcount += modifier-'0';
				}
				fp--;
			} else {
				fcount=1;
			}

			if( *fp==0 ){
				break;
			}
			fp=exform(fcount,fp,itype,ptype);
		}
		dotinc=dot-savdot;
		dot=savdot;

		if( errflg)
		{
			if( icount<0)
			{
				errflg=0;
				break;
			} else {
				error(errflg);
			}
		}
		if( --icount)
		{
			dot=inkdot(dotinc);
		}
		if( mkfault ){
			error(0);
		}
	}
}

char * exform(fcount,ifp,itype,ptype)
int fcount;
char * ifp;
{
	/* execute single format item `fcount' times
 	 * sets `dotinc' and moves `dot'
 	 * returns address of next format item
 	 */
	unsigned w;
	long int savdot, wx;
	char * fp;
	char c, modifier;
	L_JUNK fw;

	fp = ifp;
	while( fcount>0)
	{
		fp = ifp;
		c = *fp;
		if ( (c>= 'A' && c <= 'Z') || c == 'f' || c == 'p')
			dotinc = 4;
		else
			dotinc = 2;
		if ((itype == NSP) || (c == 'a'))
		{
			wx = dot;
			w = dot;
		}
		else
		{
			w = get(dot,itype);
			if (dotinc == 4)
				wx = itol68(w, get(inkdot(2), itype));
			else
				wx = w;
		}
		if( c=='F')
		{
			fw.s.sb=get(inkdot(4),itype);
			fw.s.sc=get(inkdot(6),itype);
		}
		if( errflg ){
			return(fp);
		}
		if( mkfault ){
			error(0);
		}
		modifier = *fp++;

		if( charpos()==0 && modifier!='a' ){
			printf("%16m");
		}

		switch(modifier) {

		case SPACE:
		case TB:
			break;

		case 't':
		case 'T':
			printf("%T",fcount);
			return(fp);

		case 'r':
		case 'R':
			printf("%M",fcount);
			return(fp);

		case 'a':
			psymoff(dot,ptype,":%16t");
			dotinc=0;
			break;

		case 'p':
			psymoff(wx,ptype,"%16t");
			break;

		case 'u':
			printf("%-8u",w);
			break;

		case 'U':
			printf("%-16U",wx);
			break;

		case 'c':
		case 'C':
			if (itype == NSP) w <<= 8;
			if( modifier=='C')
			{
				printesc((w>>8)&LOBYTE);
			} else {
				printc((w>>8)&LOBYTE);
			}
			dotinc=1;
			break;

		case 'b':
		case 'B':
			printf("%-8o", (w>>8)&LOBYTE);
			dotinc=1;
			break;

		case 's':
		case 'S':
			savdot=dot;
			dotinc=1;
			while( (c = (get(dot,itype)>>8)&LOBYTE) && errflg==0)
			{
				dot=inkdot(1);
				if( modifier == 'S')
				{
					printesc(c);
				} else {
					printc(c);
				}
				endline();
			}
			dotinc=dot-savdot+1;
			dot=savdot;
			break;

		case 'x':
		case 'w':
			printf("%-8x",w);
			break;

		case 'X':
		case 'W':
			printf("%-16X", wx);
			break;

		case 'Y':
			printf("%-24Y", wx);
			break;

		case 'q':
			printf("%-8q", w);
			break;

		case 'Q':
			printf("%-16Q", wx);
			break;

		case 'o':
			printf("%-8o", w);
			break;

		case 'O':
			printf("%-16O", wx);
			break;

		case 'i':
			printins(0,itype,w);
			printc(EOR);
			break;

		case 'I':
			printins(1,itype,w);
			printc(EOR);
			break;

		case 'd':
			printf("%-8d", w);
			break;

		case 'D':
			printf("%-16D", wx);
			break;

		case 'f':
			fw.d = 0;
			fw.s.sa = wx;
			printf("%-16.9f", fw.d);
			dotinc=4;
			break;

		case 'F':
			fw.s.sa = wx;
			printf("%-32.18F", fw.d);
			dotinc=8;
			break;

		case 'n':
		case 'N':
			printc('\n');
			dotinc=0;
			break;

		case '"':
			dotinc=0;
			while( *fp != '"' && *fp)
			{
				printc(*fp++);
			}
			if( *fp ){
				fp++;
			}
			break;

		case '^':
			dot=inkdot(-dotinc*fcount);
			return(fp);

		case '+':
			dot=inkdot(fcount);
			return(fp);

		case '-':
			dot=inkdot(-fcount);
			return(fp);

		default:
			error(BADMOD);
		}
		if( itype!=NSP)
		{
			dot=inkdot(dotinc);
		}
		fcount--;
		endline();
	}
	return(fp);
}

unox()
{
	int rc, status, unixpid;
	char *argp = lp;

	while (lastc != EOR) rdc();
	if ((unixpid = fork()) == 0)
	{
		signal(SIGINT, sigint);
		signal(SIGQUIT, sigqit);
		*lp = 0;
		execl("/bin/sh", "sh", "-c", argp, 0);
		exit(16);
	}
	else if (unixpid == -1) error(NOFORK);
	else
	{
		signal(SIGINT, SIG_IGN);
		while(((rc = wait(&status)) != unixpid) && (rc != -1));
		signal(SIGINT, sigint);
		prints("!");
		lp--;
	}
}

printesc(c)
{
	c &= STRIP;
	if( c<SPACE || c>'~' || c=='@')
	{
		printf("@%c",(c=='@' ? '@' : (c&(~(c&0140)))|0140));
	} else {
		printc(c);
	}
}

long int inkdot(incr)
{
	return(dot + incr);
}
