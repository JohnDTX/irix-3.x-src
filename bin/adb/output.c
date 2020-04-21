#include "defs.h"
/****************************************************************************

 DEBUGGER - output routines

****************************************************************************/
int mkfault;
FILE *infile;
FILE *outfile;
int maxpos;
int hexa;

char printbuf[MAXLIN+10];
char *printptr = printbuf;
char *digitptr;

eqstr(s1, s2)
register char * s1, *s2;
{
	while( *s1++ == *s2)
	{
		if( *s2++ == 0 )
		{
			return(1);
		}
	}
	return(0);
}

length(s)
char * s;
{
	int n = 0;
	while (*s++) n++;
	return(n);
}

printc(c)
char c;
{
	char d;
	char * q;
	int posn, tabs, p;

	if( mkfault)
	{
		return;
	} else if ( (*printptr=c)==EOR || ((printptr-printbuf) >= MAXLIN))
	{
		tabs=0;
		posn=0;
		q=printbuf;
		for( p=0; p<printptr-printbuf; p++)
		{
			d=printbuf[p];
			if( (p&7)==0 && posn)
			{
				tabs++;
				posn=0;
			}
			if( d==SPACE)
			{
				posn++;
			} else {
				while( tabs>0 ){
					*q++=TB;
					tabs--;
				}
				while( posn>0 ){
					*q++=SPACE;
					posn--;
				}
				*q++=d;
			}
		}
		*q++=c;
		fwrite(printbuf, 1, q-printbuf, outfile);
		printptr=printbuf;
	} else if ( c==TB)
	{
		*printptr++=SPACE;
		while( (printptr-printbuf)&7 ){
			*printptr++=SPACE;
		}
	} else if ( c)
	{
		printptr++;
	}
}

charpos()
{
	return(printptr-printbuf);
}

flushbuf()
{
	if( printptr!=printbuf)
	{
		printc
		    (EOR);
	}
}

printf(fmat, a1)
char * fmat;
char * *a1;
{
	char * fptr, *s;
	int *vptr;
	long int *dptr;
	long float *rptr;
	float *fltptr; /* GB */
	int width, prec;
	char c, adj;
	int x, decpt, n;
	long int lx;
	char digits[64];

	fptr = fmat;
	vptr = ( int *)&a1;
	while( c = *fptr++)
	{
		if( c!='%')
		{
			printc(c);
		} else {
			if( *fptr=='-' ){
				adj='l';
				fptr++;
			} else {
				adj='r';
			}
			width=convert(&fptr);
			if( *fptr=='.' ){
				fptr++;
				prec=convert(&fptr);
			} else {
				prec = -1;
			}
			digitptr = digits;
			rptr=(long float *)vptr;
			fltptr=(float *)vptr;
			dptr = (long int *)&a1;
			lx = *dptr;
			x = *vptr++;
			s = 0;
			switch (c = *fptr++)
			{
			case 'd':
			case 'u':
				printnum(x, c, 10);
				break;
			case 'o':
				printoct(x, 0);
				break;
			case 'q':
				lx = x;
				printoct(lx, -1);
				break;
			case 'x':
				printnum(x, c, 16);
				break;
			case 'Y':
				printdate(lx);
				break;
			case 'D':
			case 'U':
				printnum(x, c, 10);
				break;
			case 'O':
				printoct(lx, 0);
				break;
			case 'Q':
				printoct(lx, -1);
				break;
			case 'X':
				printnum(x, c, 16);
				break;
			case 'c':
				printc(x);
				break;
			case 's':
				s = (char *)x;
				break;
			case 'f':
				vptr += 3;
				s=(char *)ecvt(*fltptr, prec, &decpt, &n);
				goto doflt;
			case 'F':
				vptr += 7;
				s=(char *)_d_ecvt(*rptr, prec, &decpt, &n);
doflt:
				*digitptr++=(n?'-':'+');
				*digitptr++ = (decpt<=0 ? '0' : *s++);
				if( decpt>0 ){
					decpt--;
				}
				*digitptr++ = '.';
				while( *s && prec-- ){
					*digitptr++ = *s++;
				}
				while( *--digitptr=='0' );
				digitptr += (digitptr-digits>=3 ? 1 : 2);
				if( decpt)
				{
					*digitptr++ = 'e';
					printnum(decpt,'d',10);
				}
				s=0;
				prec = -1;
				break;
			case 'm':
				vptr--;
				break;
			case 'M':
				width = x;
				break;
			case 'T':
			case 't':
				if (c=='T') width = x;
				else vptr--;
				if (width) width -= charpos() % width;
				break;
			default:
				printc(c);
				vptr--;
			}
			if (s == NULL)
			{
				*digitptr = NULL;
				s = digits;
			}
			n = length(s);
			n = (prec<n && prec>=0 ? prec : n);
			width -= n;
			if (adj=='r') while (width-- > 0) printc(SPACE);
			while (n--) printc(*s++);
			while (width-- > 0) printc(SPACE);
			digitptr = digits;
		}
	}
}

printdate(tvec)
long int tvec;
{
	register int i;
	register char * timeptr;
	timeptr = (char *)ctime(&tvec);
	for( i=20; i<24; i++ ){
		*digitptr++ = *(timeptr+i);
	}
	for( i=3; i<19; i++ ){
		*digitptr++ = *(timeptr+i);
	}
}

prints(s)
char *s;
{
	printf("%s", s);
}

newline()
{
	printc(EOR);
}

convert(cp)
register char * *cp;
{
	register char c;
	int n;
	n=0;
	while( ((c = *(*cp)++)>='0') && (c<='9') ){
		n=n*10+c-'0';
	}
	(*cp)--;
	return(n);
}

printnum(n, fmat, base)
register int n;
char fmat;
{
	register char k;
	register int *dptr;
	int digs[15], dnum = 0, i;

	dptr = digs;
	if (hexa) dnum = ((base==10) ? 0 : ((fmat=='x') ? 4 : 8));
	if (base == 16) {
		*digitptr++ = '0';
		*digitptr++ ='x';
	}
	else if ((n < 0) && (fmat=='d' || fmat=='D'))
	{
		n = -n;
		*digitptr++ = '-';
	}
	i = (dnum ? dnum : n);
	while (i)
	{
		*dptr++ = ((unsigned)n) % base;
		n = ((unsigned)n)/base;
		if (dnum) i--;
		else i = n;
	}
	if (dptr == digs) *dptr++ = NULL;
	while (dptr != digs)
	{
		k = *--dptr;
		*digitptr++ = (k+ ((k <= 9) ? '0' : ('A' - 10)));
	}
}

printoct(o, s)
long o;
int s;
{
	int i;
	long int po = o;
	char digs[12];

	if( s)
	{
		if( po<0)
		{
			po = -po;
			*digitptr++='-';
		} else {
			if( s>0 ){
				*digitptr++='+';
			}
		}
	}
	for( i=0;i<=11;i++)
	{
		digs[i] = po&7;
		po >>= 3;
	}
	digs[10] &= 03;
	digs[11]=0;
	for( i=11;i>=0;i--)
	{
		if( digs[i] ){
			break;
		}
	}
	for( i++;i>=0;i--)
	{
		*digitptr++=digs[i]+'0';
	}
}

iclose()
{
	if( infile!=stdin)
	{
		fclose(infile);
		infile=stdin;
	}
}

oclose()
{
	if( outfile!=stdout)
	{
		flushbuf();
		fclose(outfile);
		outfile=stdout;
	}
}

endline()
{
	if( charpos()>=maxpos)
	{
		printf("\n");
	}
}
