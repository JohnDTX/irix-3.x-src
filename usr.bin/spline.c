#ifndef	lint
char	_Origin_[]	= "System V";
static char	sccsid[] = "@(#)spline.c	1.1";
/* $Source: /d2/3.7/src/usr.bin/RCS/spline.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 17:40:40 $ */
#endif

#include <stdio.h>
#include <math.h>

#define	NP	1000
#define	INF	1.e37
#define	lfloat	long float

struct proj {
	int	lbf,ubf;
	lfloat	a,b,lb,ub,quant,mult,val[NP];
} x,y;
lfloat	*diag, *r;
lfloat	dx = 1.;
lfloat	ni = 100.;
int	n;
int	auta;
int	periodic;
lfloat	konst = 0.0;
lfloat	zero = 0.;

/*
 * Spline fit technique
 * let x,y be vectors of abscissas and ordinates
 *  h	be vector of differences h =x -x
 *  y 	be vector of 2nd derivs of a x-1 function
 * If the points are numbered 0,1,2,...,n+1 then y satisfies
 * (R W Hamming, Numerical Methods for Engineers and Scientists,
 * 2nd Ed, p349ff)
 *	hiyi-1+2(hi+hi+1)yi+hi+1yi+1
 *
 *	= 6[(yi+1-yi)/hi+1-(yi-yi-1)/hi]   i=1,2,...,n
 *
 * where y0 = yn+1 = 0
 * This is a symmetric tridiagonal system of the form
 * 
 * 	| a1 h2		      |	 |y1|	   |b1|
 * 	| h2 a2	h3	      |	 |y2|	   |b2|
 * 	|    h3	a3 h4	      |	 |y3|  =   |b3|
 * 	|	  .	      |	 | .|	   | .|
 * 	|	     .	      |	 | .|	   | .|
 * It can be triangularized into
 * 	| d1 h2		      |	 |y1|	   |r1|
 * 	|    d2	h3	      |	 |y2|	   |r2|
 * 	|	d3 h4	      |	 |y3|  =   |r3|
 * 	|	   .	      |	 | .|	   | .|
 * 	|	      .	      |	 | .|	   | .|
 * where
 * 	d1 = a1
 * 
 * 	r0 = 0
 * 		   2
 * 	di = ai	- hi/di-1	1<i<=n
 * 
 * 	ri = bi	- hiri-1/di-1	1<=i<=n
 * 
 * the back solution is
 * 	yn = rn/dn
 * 
 * 	yi = (ri-hi+1yi+1)/di	1<=i<n
 * 
 * superficially, di and ri don't have to be stored for they can be
 * recalculated backward by the formulas
 *
 * 	di-1 = hi^2/(ai-di)	1<i<=n
 * 
 * 	ri-1 = (bi-ri)di-1/hi	1<i<=n
 * 
 * unhappily it turns out that the recursion forward for d
 * is quite strongly geometrically convergent--and is wildly
 * unstable going backward.
 * There's similar trouble with r, so the intermediate
 * results must be kept.
 * 
 * Note that n-1 in the program below plays the role of n+1 in the theory
 * 
 * Other_boundary_conditions
 * 
 * The boundary conditions are easily generalized to handle
 * 
 * 	y0 = ky1, yn+1 = kyn
 * 
 * for some constant k.  The above analysis was for k = 0;
 * k = 1 fits parabolas perfectly as well as stright lines;
 * k = 1/2 has been recommended as somehow pleasant.
 * 
 * All that is necessary is to add h1 to a1 and hn+1 to an.
 * 
 * 
 * Periodic_case
 * 
 * To do this, add 1 more row and column thus
 * 
 * 	| a1 h2		   h1 |	 |y1|	  |b1|
 * 	| h2 a2	h3	      |	 |y2|	  |b2|
 * 	|    h3	a4 h4	      |	 |y3|	  |b3|
 * 	|		      |	 | .|  =  | .|
 * 	|	      .	      |	 | .|	  | .|
 * 	| h1		h0 a0 |	 | .|	  | .|
 * 
 * where h0 == hn+1
 * 
 * The same diagonalization procedure works, except for
 * the effect of the 2 corner elements.  Let si be the part
 * of the last element in the i "diagonalized" row that
 * arises from the extra top corner element.
 * 
 * 		s1 = h1
 * 
 * 		si = -si-1hi/di-1	2<=i<=n+1
 * 
 * After "diagonalizing", the lower corner element remains.
 * Call ti the bottom element that appears in the i colomn
 * as the bottom element to its left is eliminated
 * 
 * 		t1 = h1
 * 
 * 		ti = -ti-1hi/di-1
 * 
 * Evidently ti = si.
 * Elimination along the bottom row
 * introduces further corrections to the bottom right element
 * and to the last element of the right hand side.
 * Call these corrections u and v.
 * 
 * 	u1 = v1	= 0
 * 
 * 	ui = ui-1-si-1*ti-1/di-1
 * 
 * 	vi = vi-1-ri-1*ti-1/di-1	2<=i<=n+1
 * 
 * The back solution is now obtained as follows
 * 
 * 	yn+1 = (rn+1+vn+1)/(dn+1+sn+1+tn+1+un+1)
 * 
 * 	yi = (ri-hi+1*yi+1-si*yn+1)/di	1<=i<=n
 * 
 * Interpolation in the interval xi<=x<=xi+1 is by the formula
 * 
 * 	y = yix+ + yi+1x- -(hi^2+1/6)[yi(x+-x^3+)+yi+1(x--x^3-)]
 * where
 * 	x+ = xi+1-x
 * 
 * 	x- = x-xi
 */

lfloat
rhs(i)
{
	int	j;
	lfloat	zz;

	j = i == n-1 ? 0 : i;
	zz = (y.val[i]-y.val[i-1])/(x.val[i]-x.val[i-1]);
	return 6*((y.val[j+1]-y.val[j])/(x.val[i+1]-x.val[i]) - zz);
}

spline()
{
	lfloat	d,s,u,v,hi,hi1;
	lfloat	h;
	lfloat	D2yi,D2yi1,D2yn1,x0,x1,yy,a;
	int	end;
	lfloat	corr;
	int	i,j,m;

	if (n<3)
		return 0;
	if (periodic)
		konst = 0;
	d = 1;
	r[0] = 0;
	s = periodic?-1:0;
	for (i=0;++i<n-!periodic;) {	/* triangularize */
		hi = x.val[i]-x.val[i-1];
		hi1 = i == n-1?x.val[1]-x.val[0]:
			x.val[i+1]-x.val[i];
		if (hi1*hi <= 0)
			return 0;
		u = i == 1?zero:u-s*s/d;
		v = i == 1?zero:v-s*r[i-1]/d;
		r[i] = rhs(i)-hi*r[i-1]/d;
		s = -hi*s/d;
		a = 2*(hi+hi1);
		if (i == 1)
			a += konst*hi;
		if (i == n-2)
			a += konst*hi1;
		diag[i]	= d = i == 1? a: a - hi*hi/d;
	}
	D2yi = D2yn1 = 0;
	for (i=n-!periodic;--i >= 0;) {	/* back	substitute */
		end = i == n-1;
		hi1 = end?x.val[1]-x.val[0]:
			x.val[i+1]-x.val[i];
		D2yi1 = D2yi;
		if (i>0) {
			hi = x.val[i]-x.val[i-1];
			corr = end?2*s+u:zero;
			D2yi = (end*v+r[i]-hi1*D2yi1-s*D2yn1)/
				(diag[i]+corr);
			if (end)
				D2yn1 = D2yi;
			if (i>1) {
				a = 2*(hi+hi1);
				if (i == 1)
					a += konst*hi;
				if (i == n-2)
					a += konst*hi1;
				d = diag[i-1];
				s = -s*d/hi;
			}
		}
		else D2yi = D2yn1;
		if (!periodic) {
			if (i == 0)
				D2yi = konst*D2yi1;
			if (i == n-2)
				D2yi1 = konst*D2yi;
		}
		if (end)
			continue;
		m = hi1>0?ni:-ni;
		m = 1.001*m*hi1/(x.ub-x.lb);
		if (m <= 0)
			m = 1;
		h = hi1/m;
		for (j=m;j>0||i == 0 && j == 0;j--) {	/* interpolate */
			x0 = (m-j)*h/hi1;
			x1 = j*h/hi1;
			yy = D2yi*(x0-x0*x0*x0)+D2yi1*(x1-x1*x1*x1);
			yy = y.val[i]*x0+y.val[i+1]*x1 -hi1*hi1*yy/6;
			printf("%lf ",x.val[i]+j*h);
			printf("%lf\n",yy);
		}
	}
	return 1;
}

readin()
{
	for (n=0;n<NP;n++) {
		if (auta)
			x.val[n] = n*dx+x.lb;
		else if (!getfloat(&x.val[n]))
			break;
		if (!getfloat(&y.val[n]))
			break;
	}
}

getfloat(p)
lfloat	*p;
{
	char	buf[30];
	register c;
	int	i;
	extern lfloat	_latof();

	for (;;) {
		c = getchar();
		if (c == EOF) {
			*buf = '\0';
			return 0;
		}
		*buf = c;
		switch (*buf) {
		  case ' ':
		  case '\t':
		  case '\n':
			  continue;
		}
		break;
	}
	for (i=1;i<30;i++) {
		c = getchar();
		if (c == EOF) {
			buf[i] = '\0';
			break;
		}
		buf[i] = c;
		if ('0' <= c && c <= '9')
			continue;
		switch (c) {
		  case '.':
		  case '+':
		  case '-':
		  case 'E':
		  case 'e':
			continue;
		}
		break;
	}
	buf[i] = ' ';
	*p = _latof(buf);
	return 1;
}

getlim(p)
struct proj *p;
{
	int	i;

	for (i=0;i<n;i++) {
		if (!p->lbf && p->lb>(p->val[i]))
			p->lb = p->val[i];
		if (!p->ubf && p->ub<(p->val[i]))
			p->ub = p->val[i];
	}
}


main(argc,argv)
char	*argv[];
{
	extern char	*malloc();
	int	i;

	x.lbf = x.ubf = y.lbf = y.ubf = 0;
	x.lb = INF;
	x.ub = -INF;
	y.lb = INF;
	y.ub = -INF;
	while (--argc > 0) {
		argv++;
again:		switch (argv[0][0]) {
		  case '-':
			argv[0]++;
			goto again;
		  case 'a':
			auta = 1;
			numb(&dx,&argc,&argv);
			break;
		  case 'k':
			numb(&konst,&argc,&argv);
			break;
		  case 'n':
			numb(&ni,&argc,&argv);
			break;
		  case 'p':
			periodic = 1;
			break;
		  case 'x':
			if (!numb(&x.lb,&argc,&argv))
				break;
			x.lbf = 1;
			if (!numb(&x.ub,&argc,&argv))
				break;
			x.ubf = 1;
			break;
		  default:
			fprintf(stderr,	"Bad argument\n");
			exit(1);
		}
	}
	if (auta && !x.lbf)
		x.lb = 0;
	readin();
	getlim(&x);
	getlim(&y);
	i = (n+1)*sizeof(dx);
	diag = (lfloat *)malloc((unsigned)i);
	r = (lfloat *)malloc((unsigned)i);
	if (r == NULL || !spline())
		for (i=0; i<n; i++) {
			printf("%lf ",x.val[i]);
			printf("%lf\n",y.val[i]);
		}
}

numb(np,argcp,argvp)
int	*argcp;
lfloat	*np;
char	***argvp;
{
	lfloat	_latof();
	char	c;

	if (*argcp <= 1)
		return 0;
	c = (*argvp)[1][0];
	if (!('0' <= c && c <= '9' || c == '-' || c ==  '.' ))
		return 0;
	*np = _latof((*argvp)[1]);
	(*argcp)--;
	(*argvp)++;
	return 1;
}
