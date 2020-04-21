/*	@(#)qsort.c	1.3	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/

static 	qses, (*qscmp)();

void
qsort(a, n, es, fc)
char	*a;
unsigned n, es;
int	(*fc)();
{
	void qs1();

	qscmp = fc;
	qses = es;
	qs1(a, a+n*es);
}

static void
qs1(a, l)
char	*a, *l;
{
	register char *i, *j;
	register int es;
	register char *lp, *hp;
	register int c;
	void	qsexc(), qstexc();
	unsigned n;

	es = qses;
start:
	if((n=l-a) <= es)
		return;
	n = es * (n / (2*es));
	hp = lp = a+n;
	i = a;
	j = l-es;
	while(1) {
		if(i < lp) {
			if((c = (*qscmp)(i, lp)) == 0) {
				qsexc(i, lp -= es);
				continue;
			}
			if(c < 0) {
				i += es;
				continue;
			}
		}

loop:
		if(j > hp) {
			if((c = (*qscmp)(hp, j)) == 0) {
				qsexc(hp += es, j);
				goto loop;
			}
			if(c > 0) {
				if(i == lp) {
					qstexc(i, hp += es, j);
					i = lp += es;
					goto loop;
				}
				qsexc(i, j);
				j -= es;
				i += es;
				continue;
			}
			j -= es;
			goto loop;
		}

		if(i == lp) {
			if(lp-a >= l-hp) {
				qs1(hp+es, l);
				l = lp;
			} else {
				qs1(a, lp);
				a = hp+es;
			}
			goto start;
		}

		qstexc(j, lp -= es, i);
		j = hp -= es;
	}
}

static void
qsexc(i, j)
char *i, *j;
{
	register char *ri, *rj, c;
	register int n;

	n = qses;
	ri = i;
	rj = j;
	do {
		c = *ri;
		*ri++ = *rj;
		*rj++ = c;
	} while(--n);
}

static void
qstexc(i, j, k)
char *i, *j, *k;
{
	register char *ri, *rj, *rk;
	register int c, n;

	n = qses;
	ri = i;
	rj = j;
	rk = k;
	do {
		c = *ri;
		*ri++ = *rk;
		*rk++ = *rj;
		*rj++ = c;
	} while(--n);
}
