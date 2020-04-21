#include "opt.h"

/* set RT1 (source) and RT2 (dest) "registers" from assy. language code */
dualop(ap)
struct node *ap;
{
	register char *p1, *p2;
	register struct node *p;

	p = ap;
	p1 = p->code;
	p2 = regs[RT1];
	while (*p1 && *p1!=',')
		*p2++ = *p1++;
	*p2++ = 0;
	p2 = regs[RT2];
	*p2 = 0;
	if (*p1++ !=',')
		return;
	while (*p1==' ' || *p1=='\t')
		p1++;
	while (*p2++ = *p1++);
}

/* return register number of reg that already contains operand passed as
 * arg, -1 otherwise.
 */
findrand(as)
register char *as;
{
	if (*as == 0)
		return(-1);
	if (regs[0][0] == *as && equstr(&regs[0][0], as))
		return(0);
	if (regs[1][0] == *as && equstr(&regs[1][0], as))
		return(1);
	if (regs[2][0] == *as && equstr(&regs[2][0], as))
		return(2);
	if (regs[3][0] == *as && equstr(&regs[3][0], as))
		return(3);
	if (regs[4][0] == *as && equstr(&regs[4][0], as))
		return(4);
	if (regs[5][0] == *as && equstr(&regs[5][0], as))
		return(5);
	if (regs[6][0] == *as && equstr(&regs[6][0], as))
		return(6);
	if (regs[7][0] == *as && equstr(&regs[7][0], as))
		return(7);
	if (regs[8][0] == *as && equstr(&regs[8][0], as))
		return(8);
	if (regs[9][0] == *as && equstr(&regs[9][0], as))
		return(9);
	if (regs[10][0] == *as && equstr(&regs[10][0], as))
		return(10);
	if (regs[11][0] == *as && equstr(&regs[11][0], as))
		return(11);
	if (regs[12][0] == *as && equstr(&regs[12][0], as))
		return(12);
	if (regs[13][0] == *as && equstr(&regs[13][0], as))
		return(13);
/*
	for (i = 0; i<NREG; i++) {
		if (equstr(regs[i], as)) {
			if (debug)
				printf("findrand(%s) returning %d\n", as, i);
			return(i);
		}
	}
	if (debug)
		printf("findrand(%s) returning -1\n", as);
*/
	return(-1);
}

/* return register number if operand corresponds to register, else -1 */
isreg(as)
char *as;
{
	register char *s;

	s = as;
	if (s[0]=='d' && s[1]>='0' && s[1]<='7' && s[2]==0)
		return(s[1]-'0');
	if (s[0]=='a' && s[1]>='0' && s[1]<='5' && s[2]==0)
		return(s[1]-'0'+8);
	return(-1);
}

/* return Ax register number if operand corresponds to register, else -1 */
isareg(s)
register char *s;
{
	if (s[0]=='a' && s[1]>='0' && s[1]<='7' && s[2]==0)
		return(s[1]-'0');
	if (s[0]=='s' && s[1]=='p' && s[2]==0)
		return(7);
	return(-1);
}

/* look for operands of the form an@+ or an@-; if found, reset register contents
 * table for register n and for any regs based on address register n.
 */
source(ap)
char *ap;
{
	register char *p1, *p2;
	register int i;

	p1 = ap;
	p2 = p1;
	if (*p1==0)
		return(0);
	while (*p2++);
	if (*(p2-2)=='+' || *(p2-2)=='-') {
		if (*p1++=='a' && *p1>='0' && *p1<='5') {
		  regs[(i = *p1 - '0' + 8)][0] = 0;
		  areg(i);
		}
		return(1);
	}
	return(0);
}

/* g <> 0 if address is ok as replacement */
repladdr(p, f, g)
struct node *p;
{
	register r;
	int r1;
	register char *p1, *p2;
	static char rt1[MAXSIZE], rt2[MAXSIZE];

	if (f)
		r1 = findrand(regs[RT2]);
	else
		r1 = -1;
	r = findrand(regs[RT1]);
	if (r>=0 || r1>=0) {
		p2 = regs[RT1];
		for (p1 = rt1; *p1++ = *p2++;);
		if (regs[RT2][0]) {
			p1 = rt2;
			*p1++ = ',';
			for (p2 = regs[RT2]; *p1++ = *p2++;);
		} else
			rt2[0] = 0;
		if (r>=0) {
			if (r>7) {
			  if (!g) return;
			  rt1[0] = 'a';
			  rt1[1] = r - 8 + '0';
			} else { rt1[0] = 'd'; rt1[1] = r + '0'; }
			rt1[2] = 0;
			nsaddr++;
		}
		if (r1>=0) {
			if (r1>7) {
			  if (!g) return;
			  rt2[1] = 'a';
			  rt2[2] = r1 - 8 + '0';
			} else { rt2[1] = 'd'; rt2[2] = r1 + '0'; }
			rt2[3] = 0;
			nsaddr++;
		}
		strcat((char *)rt1, (char *)rt2);
		p->code = copy(rt1);
	}
}

movedat()
{
	register struct node *p1;
	register seg;

	if (first.forw == 0)
		return;
	seg = -1;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
		if (p1->op==TEXT||p1->op==DATA||p1->op==BSS) {
			if (p1->op == seg ||
			    (p1->forw && (p1->forw->op==TEXT||p1->forw->op==DATA||p1->forw->op==BSS))) {
				p1->back->forw = p1->forw;
				p1->forw->back = p1->back;
				p1->ref = freenodes;
				freenodes = p1;
				p1 = p1->back;
				continue;
			}
			seg = p1->op;
		}
	}
}

redunbr(p)
register struct node *p;
{
	register struct node *p1;
	register char *ap1, *ap2;

	if ((p1 = p->ref) == 0)
		return;
	p1 = nonlab(p1);
	if (p1->op==TST) {
		singop(p1);
		savereg(RT2, "#0");
	} else if (p1->op==CMP)
		dualop(p1);
	else
		return;
	if (p1->forw->op!=CBR)
		return;
	if (p1->back->op != JMP && p1->back->op != JBR)
		return;
	ap1 = findcon(RT1);
	ap2 = findcon(RT2);
	p1 = p1->forw;
	if (compare(p1->subop, ap1, ap2)) {
		nredunj++;
		nchange++;
		decref(p->ref);
		p->ref = p1->ref;
		p->labno = p1->labno;
		p->ref->refc++;
	}
}

char *
findcon(i)
{
	register char *p;
	register r;

	p = regs[i];
	if (*p=='#')
		return(p);
	if ((r = isreg(p)) >= 0)
		return(regs[r]);
	if (equstr(p, conloc))
		return(conval);
	return(p);
}

compare(op, acp1, acp2)
char *acp1, *acp2;
{
	register char *cp1, *cp2;
	register n1, n2;

	cp1 = acp1;
	cp2 = acp2;
	if (*cp1++ != '#' || *cp2++ != '#')
		return(0);
	n1 = 0;
	while (*cp2 >= '0' && *cp2 <= '9') {
		n1 *= 10;
		n1 += *cp2++ - '0';
	}
	n2 = n1;
	n1 = 0;
	while (*cp1 >= '0' && *cp1 <= '9') {
		n1 *= 10;
		n1 += *cp1++ - '0';
	}
	if (*cp1=='+')
		cp1++;
	if (*cp2=='+')
		cp2++;
	do {
		if (*cp1++ != *cp2)
			return(0);
	} while (*cp2++);
	switch(op) {

	case JEQ:
		return(n1 == n2);
	case JNE:
		return(n1 != n2);
	case JLE:
		return(n1 <= n2);
	case JGE:
		return(n1 >= n2);
	case JLT:
		return(n1 < n2);
	case JGT:
		return(n1 > n2);
	case JLO:
		return((unsigned)n1 < (unsigned)n2);
	case JHI:
		return((unsigned)n1 > (unsigned)n2);
	case JLOS:
		return((unsigned)n1 <= (unsigned)n2);
	case JHIS:
		return((unsigned)n1 >= (unsigned)n2);
	}
	return(0);
}

setcon(ar1, ar2)
char *ar1, *ar2;
{
	register char *cl, *cv, *p;

	cl = ar2;
	cv = ar1;
	if (*cv != '#')
		return;
	if (!natural(cl))
		return;
	p = conloc;
	while (*p++ = *cl++);
	p = conval;
	while (*p++ = *cv++);
}

equstr(p1, p2)
register char *p1, *p2;
{
	do {
		if (*p1++ != *p2)
			return(0);
	} while (*p2++);
	return(1);
}

natural(ap)
char *ap;
{
	register char *p;

	p = ap;
	if (*p=='*' || *p=='(' || *p=='-'&&*(p+1)=='(')
		return(0);
	while (*p++);
	p--;
	if (*--p == '+' || *p ==')' && *--p != '5')
		return(0);
	return(1);
}
