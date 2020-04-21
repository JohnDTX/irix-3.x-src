#include "defs.h"
/****************************************************************************

 DEBUGGER - command interpretation

****************************************************************************/
MSG NOPCS;
MSG BADEQ;
MSG NOMATCH;
MSG BADVAR;
MSG BADCOM;

struct map txtmap;
struct map datmap;
int executing;
char *lp;
int fcor;
int fsym;
int mkfault;
char * errflg;

char lastc;
char eqformat[128] = "X";
char pgformat[128] = "i";
char dtformat[128] = "i";
REGLIST reglist[];
long int dot;
long int ditto;
int dotinc;
int lastcom = '=';
long int var[];
long int locval;
long int locmsk;
int pid;
long int expv;
long int adrval;
int adrflg;
long int cntval;
int cntflg;

command(buf,defcom)
char *buf;
char defcom;
{
	int itype, ptype, modifier, reg;
	char longpr, eqcom, wformat[1], savc;
	long w, savdot;
	char *format, *savlp = lp;

	if (buf)
		if (*buf == EOR) return(FALSE);
		else lp = buf;

	do
	{
		if (adrflg = expr(0))
		{
			dot = expv;
			ditto = dot;
		}
		adrval = dot;

		if ((rdc() == ',') && expr(0))
		{
			cntflg = TRUE;
			cntval = expv;
		}
		else
		{
			cntflg = FALSE;
			cntval = 1;
			lp--;
		}

		if (!eol(rdc())) lastcom = lastc;
		else
		{
			if (adrflg == 0) dot = inkdot(dotinc);
			lp--;
			lastcom = defcom;
		}

		switch( lastcom & STRIP)
		{

		case '/':
			itype=DSP;
			ptype=DSYM;
			format=dtformat;
			goto trystar;

		case '=':
			itype=NSP;
			ptype=ASYM;
			format=eqformat;
			goto trypr;

		case '?':
			itype=ISP;
			ptype=ISYM;
			format=pgformat;
			goto trystar;

trystar:
			if( rdc()=='*' ){
				lastcom |= QUOTE;
			} else {
				lp--;
			}
			if( lastcom&QUOTE)
			{
				itype |= STAR;
				ptype = (DSYM+ISYM)-ptype;
			}

trypr:
			longpr=FALSE;
			eqcom=lastcom=='=';
			switch (rdc())
			{
			case 'm':
				{
					int fcount;
					struct map *smap;
					union {
						struct map *m;
						int *mp;
					}amap;

					if (eqcom) error(BADEQ);
					smap = (itype&DSP ? &datmap : &txtmap);
					amap.m = smap;
					fcount = 3;
					if (itype&STAR) amap.mp += 3;
					while (fcount-- && expr(0)) *(amap.mp)++ = expv;
					if (rdc() == '?') smap->ufd = fsym;
					else if (lastc == '/') smap->ufd = fcor;
					else lp--;
				}
				break;

			case 'L':
				longpr=TRUE;
			case 'l':
				/*search for exp*/
				if( eqcom ){
					error(BADEQ);
				}
				dotinc=2;
				savdot=dot;
				expr(1);
				locval=expv;
				if( expr(0) ){
					locmsk=expv;
				} else {
					locmsk = -1L;
				}
				for(;;){
					w=leng(get(dot,itype));
					if( longpr)
					{
						w=itol68(w, get(inkdot(2),itype));
					}
					if( errflg || mkfault || (w&locmsk)==locval)
					{
						break;
					}
					dot=inkdot(dotinc);
				}
				if( errflg)
				{
					dot=savdot;
					errflg=NOMATCH;
				}
				psymoff(dot,ptype,"");
				break;

			case 'W':
				longpr=TRUE;
			case 'w':
				if (eqcom) error(BADEQ);
				wformat[0] = lastc;
				expr(1);
				do
				{
					savdot=dot;
					psymoff(dot,ptype,":%16t");
					exform(1,wformat,itype,ptype);
					errflg=0;
					dot=savdot;
					if (longpr) put(dot,itype,expv);
					put((longpr?inkdot(2):dot),itype,(expv<<16));
					savdot=dot;
					printf("=%8t");
					exform(1,wformat,itype,ptype);
					newline();
				} while (expr(0) && (errflg==0));
				dot=savdot;
				chkerr();
				break;

			default:
				lp--;
				getformat(format);
				if( !eqcom)
				{
					psymoff(dot,ptype,":%16t");
				}
				scanform(cntval,format,itype,ptype);
			}
			break;

		case '>':
			lastcom = 0;
			savc = rdc();
			if ((reg = getroffs(savc)) != 16)
			{
				if (!pid) error(NOPCS);
				reglist[reg].rval = dot;
				putreg(pid, reg, reglist[reg].rval);
			}
			else if ((modifier = varchk(savc)) != -1) var[modifier] = dot;
			else error(BADVAR);
			break;

		case '!':
			lastcom = 0;
			unox();
			break;

		case '$':
			lastcom = 0;
			printtrace(nextchar());
			break;

		case ':':
			if (!executing)
			{
				executing = TRUE;
				subpcs(nextchar());
				executing = FALSE;
				lastcom = 0;
			}
			break;

		case 0:
			prints(DBNAME);
			break;

		default:
			error(BADCOM);

		}
		flushbuf();
	} while (rdc() == ';');

	if (buf) lp = savlp;
	else lp--;
	return(adrflg && (dot != 0));
}

