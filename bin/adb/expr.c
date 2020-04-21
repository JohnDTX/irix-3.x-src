#include "defs.h"
/****************************************************************************

 DEBUGGER - expression evaluation

****************************************************************************/
MSG BADSYM;
MSG BADVAR;
MSG BADKET;
MSG BADSYN;
MSG NOCFN;
MSG NOADR;
MSG BADLOC;

REGLIST reglist[];
int fcor;
SYMPTR symbol;

char *lp;
int hexf;
char * errflg;
char isymbol[256];

char lastc;
unsigned *endhdr;

long int dot;
long int ditto;
int dotinc;
long int var[];
long int expv;

expr(a) /* term | term dyadic expr | */
{
	int rc;
	long int lhs;

	rdc();
	lp--;
	rc = term(a);
	while (rc)
	{
		lhs = expv;
		switch (readchar())
		{
		case '+':
			term(a|1);
			expv += lhs;
			break;

		case '-':
			term(a|1);
			expv = lhs - expv;
			break;

		case '#':
			term(a|1);
			expv = round(lhs,expv);
			break;

		case '*':
			term(a|1);
			expv *= lhs;
			break;

		case '%':
			term(a|1);
			expv = lhs/expv;
			break;

		case '&':
			term(a|1);
			expv &= lhs;
			break;

		case '|':
			term(a|1);
			expv |= lhs;
			break;

		case ')':
			if ((a&2) == 0) error(BADKET);

		default:
			lp--;
			return(rc);
		}
	}
	return(rc);
}

term(a) /* item | monadic item | (expr) | */
{
	int word = 0;
	switch (readchar())
	{
	case '*':
		term(a|1);
		word = get(expv, DSP);
		expv = itol68(word, get(expv+2, DSP));
		return(1);

	case '@':
		term(a|1);
		word = get(expv, ISP);
		expv = itol68(word, get(expv+2, ISP));
		return(1);

	case '-':
		term(a|1);
		expv = -expv;
		return(1);

	case '~':
		term(a|1);
		expv = ~expv;
		return(1);

	case '(':
		expr(2);
		if (*lp != ')') error(BADSYN);
		else {
			lp++;
			return(1);
		}

	default:
		lp--;
		return(item(a));
	}
}

/* name [ . local ] | number | . | ^ | <var | <register | 'x | | */
item(a)
{
	int base, d, frpt, regptr;
	char savc;
	char hex;
	long int frame;
	SYMPTR symp;

	hex = FALSE;
	readchar();
	if (symchar(0))
	{
		readsym();
		if (lastc == '.') error(BADSYM);
		symp = lookupsym(isymbol);
		if (!symp && hexf && hexdigit(isymbol[0])) expv = nval();
		else if (symp) expv = symp->vals;
		else error(BADSYM);
		lp--;
	}
	else if (digit(lastc))
	{
		expv = 0;
		base = ((lastc == '0') ? 8 : (hexf ? 16 : 10));
		if (base == 16) hex = TRUE;
		else hex = FALSE;
		while (hex ? hexdigit(lastc) : digit(lastc))
		{
			expv *= base;
			if ((d = convdig(lastc)) >= base) error(BADSYN);
			expv += d;
			readchar();
			if (expv == 0 && (lastc == 'x' || lastc == 'X'))
			{
				hex = TRUE;
				base = 16;
				readchar();
			}
			else if (expv == 0 && (lastc == 'd' || lastc == 'D'))
			{
				hex = FALSE;
				base = 10;
				readchar();
			}
		}
		lp--;
	}
	else if (lastc == '.')
	{
		readchar();
		if (symchar(0)) error(BADSYM);
		else expv = dot;
		lp--;
	}
	else if (lastc == '"') expv = ditto;
	else if (lastc == '+') expv = inkdot(dotinc);
	else if (lastc == '^') expv = inkdot(-dotinc);
	else if (lastc == '<')
	{
		savc = rdc();
		if ((regptr = getroffs(savc)) != 16)
			expv = reglist[regptr].rval;
		else if ((base=varchk(savc)) != -1) expv = var[base];
		else error(BADVAR);
	}
	else if (lastc == '\'')
	{
		d = 4;
		expv = 0;
		while (quotchar())
			if (d--)
			{
#ifdef m68000
				expv = (expv<<8) | lastc;
#else
				if (d == 1) expv <<= 16;
				expv |= ((d & 1) ? lastc : lastc<<8);
#endif
			}
			else error(BADSYN);
	}
	else if (a) error(NOADR);
	else
	{
		lp--;
		return(0);
	}
	return(1);
}

nval()
{
	int i, d, val = 0;

	for (i = 0; i < 256; i++) if (isymbol[i])
	{
		val *= 16;
		if ((d = convdig(isymbol[i])) >= 16) error(BADSYN);
		val += d;
	}
	else break;
	return(val);
}

readsym()
{
	register char *p = isymbol;

	do
	{
		if (p < &isymbol[255]) *p++ = lastc;
		readchar();
	} while (symchar(1));
	*p++ = 0;
}

SYMPTR
lookupsym(symstr)
char *symstr;
{
	SYMPTR symp;

#ifdef DEBUG
	printf("looking for symbol %s\n",symstr);
#endif
	symset();
	while((symp = symget()) != NULL) {
#ifdef DEBUG
		printf("checking symbol %s....",symp->symc);
#endif
		if ((symp->symf & SYMCHK) == symp->symf) {
			if (eqsym(symp->symc, symstr, '_')) {
#ifdef DEBUG
				printf("equal.\n");
#endif
				return(symp);
			}
#ifdef DEBUG
			else printf("not equal.\n");
#endif
		}
#ifdef DEBUG
		else printf(" - illegal type. ignored.\n");
#endif
	}
#ifdef DEBUG
	printf("\t%s not found\n",symstr);
#endif
	return(NULL);
}

hexdigit(c)
char c;
{
	return((c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F'));
}

convdig(c)
char c;
{
	if (digit(c)) return(c-'0');
	else if (hexdigit(c))
	{
		if (c >= 'a') return(c-'a'+10);
		else return(c-'A'+10);
	}
	else return(16);
}

digit(c)
char c;
{
	return(c>='0' && c<='9');
}

letter(c)
char c;
{
	return((c>='a' && c<='z') || (c>='A' && c<='Z'));
}

symchar(dig)
{
	if( lastc=='\\' ){
		readchar();
		return(TRUE);
	}
	return( letter(lastc) || lastc=='_' || dig && digit(lastc) );
}

varchk(name)
{
	if( digit(name) ){
		return(name-'0');
	}
	if( letter(name) ){
		return((name&037)-1+10);
	}
	return(-1);
}

eqsym(s1, s2, c)
register char * s1, *s2;
char c;
{
	if( eqstr(s1,s2))
	{
		return(TRUE);
	} else if ( *s1==c)
	{
		return(eqstr(++s1,s2));
	}
	else {
		return(FALSE);
	}
}
