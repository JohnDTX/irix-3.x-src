#include "opt.h"

char *skydtreg="0x1044";

rmove()
{
	register struct node *p, *ptemp;
	register int r, r1;

	for (p=first.forw; p!=0; p = p->forw) {
	if (debug) {
		for (r=0; r<NREG; r++)
			if (regs[r][0])
				printf("%d: %s ", r, regs[r]);
		printf("-\n");
	}

	switch (p->op) {

	case MOV:
		if (p->subop==BYTE || p->subop==WORD)
			goto badmov;
		dualop(p);
		if ((r = findrand(regs[RT1])) >= 0) {
			if ((r == isreg(regs[RT2])) && p->forw->op!=CBR) {
				p->forw->back = p->back;
				p->back->forw = p->forw;
				p->ref = freenodes;
				freenodes = p;
				redunm++;
				continue;
			}
		}
		repladdr(p, 0, 1);
		r = isreg(regs[RT1]);
		r1 = isreg(regs[RT2]);
		dest(regs[RT2]);
		if (r >= 0)
			if (r1 >= 0)
				savereg(r1, (char *)regs[r]);
			else
				{ 
/* ** SGI GB - dont mark sky h/w register as static
*/
#ifdef juniper
				if ((!equstr(regs[RT2],skydtreg)) && 
					((strlen(regs[RT2]) != 8)||
					 	(!index(regs[RT2],':'))))
#else
				if (!equstr(regs[RT2],skydtreg))
#endif
					savereg(r, (char *)regs[RT2]);
				}
		else
/** SGI GB - dont mark sky h/w register 
	as static 
			if (r1 >= 0)
*/
#ifdef juniper
			if ((r1 >= 0) && (!equstr(regs[RT1],skydtreg)) && 
				((strlen(regs[RT1]) != 8)||
					 	(!index(regs[RT1],':'))))
#else
			if ((r1 >= 0)&& (!equstr(regs[RT1],skydtreg)))
#endif
				savereg(r1, (char *)regs[RT1]);
			else
				setcon(regs[RT1], regs[RT2]);
		(void) source(regs[RT1]);
		continue;

	case AND:
	case OR:
	case MULS:
	case MULU:
	case DIVS:
	case DIVU:
	case ADD:
	case ADDQ:
	case SUB:
	case SUBQ:
	case EOR:
	badmov:
		dualop(p);
		if (p->subop==LONG)
			repladdr(p, 0, 1);
		(void) source(regs[RT1]);
		dest(regs[RT2]);
		ccloc[0] = 0;
		continue;

	case CLR:
	case NOT:
	case NEG:
	case EXT:
		singop(p);
		dest(regs[RT1]);
		if (p->op==CLR)
			if ((r = isreg(regs[RT1])) >= 0)
				savereg(r, "#0");
			else
				setcon("#0", regs[RT1]);
		ccloc[0] = 0;
		continue;

	case TST:
		singop(p);
		if (p->subop==LONG)
			repladdr(p, 0, 0);
		(void) source(regs[RT1]);
		if (regs[RT1][0] && equstr(regs[RT1], ccloc)) {
			p->back->forw = p->forw;
			p->forw->back = p->back;
			ptemp = p;
			p = p->back;
			ptemp->ref = freenodes;
			freenodes = ptemp;
			nrtst++;
			nchange++;
		}
		continue;

	case BTST:
		dualop(p);
		(void) source(regs[RT2]);
		ccloc[0] = 0;
		continue;

	case CMP:
		dualop(p);
		(void) source(regs[RT1]);
		(void) source(regs[RT2]);
		if (p->subop==LONG && (isreg(regs[RT2]) >= 0 ||
		    (regs[RT1][0]=='#' && findrand(regs[RT1])==-1 &&
		    isareg(regs[RT2]) == -1)))
			repladdr(p, 1, 1);
		ccloc[0] = 0;
		continue;

	case CBR:
		ccloc[0] = 0;
		continue;

	case JBR:
		redunbr(p);

	default:
		clearreg();
	}
	}
}

jumpsw()
{
	register struct node *p, *p1;
	register t;
	int nj;

	t = 0;
	nj = 0;
	for (p=first.forw; p!=0; p = p->forw)
		p->refc = ++t;
	for (p=first.forw; p!=0; p = p1) {
		p1 = p->forw;
		if (p->op == CBR && p1->op==JBR && p->ref && p1->ref
		 && iabs(p->refc - p->ref->refc) > iabs(p1->refc - p1->ref->refc)) {
			if (p->ref==p1->ref)
				continue;
			p->subop = revbr[p->subop];
			t = (int)p1->ref;
			p1->ref = p->ref;
			p->ref = (struct node *)t;
			t = p1->labno;
			p1->labno = p->labno;
			p->labno = t;
			nrevbr++;
			nj++;
		}
	}
	return(nj);
}

iabs(x)
{
	return(x<0? -x: x);
}

equop(ap1, p2)
struct node *ap1, *p2;
{
	register char *cp1, *cp2;
	register struct node *p1;

	p1 = ap1;
	if (p1->op!=p2->op || p1->subop!=p2->subop)
		return(0);
	if (p1->op>0 && p1->op<MOV)
		return(0);
	cp1 = p1->code;
	cp2 = p2->code;
	if (cp1==0 && cp2==0)
		return(1);
	if (cp1==0 || cp2==0)
		return(0);
	while (*cp1 == *cp2++)
		if (*cp1++ == 0)
			return(1);
	return(0);
}

decref(p)
register struct node *p;
{
	if (p->op != LABEL) return;
	if (--p->refc <= 0) {
		nrlab++;
		p->back->forw = p->forw;
		p->forw->back = p->back;
		p->ref = freenodes;
		freenodes = p;
	}
}

struct node *
nonlab(ap)
struct node *ap;
{
	register struct node *p;

	p = ap;
	while (p && p->op==LABEL)
		p = p->forw;
	return(p);
}

struct node *
getnode()
{
	register struct node *p;

	if (p = freenodes)
		freenodes = p->ref;
	else {
		p = (struct node *)calloc(1,sizeof first);
		if (p == 0) {
			fprintf(stderr, "c2:out of node storage space\n");
			exitt(-1);
		}
	}
	p->subop = 0;
	p->lineno = 0;
	p->misc = 0;
	p->code = 0;
	p->ref = 0;
	return(p);
}

clearreg()
{
	regs[0][0] = regs[1][0] = regs[2][0] = regs[3][0] = 0;
	regs[4][0] = regs[5][0] = regs[6][0] = regs[7][0] = 0;
	regs[8][0] = regs[9][0] = regs[10][0] = regs[11][0] = 0;
	regs[12][0] = regs[13][0] = 0;
/*
	register int i;

	for (i=0; i<NREG; i++)
		regs[i][0] = '\0';
*/
	conloc[0] = 0;
	ccloc[0] = 0;
}

savereg(ai, s)
register char *s;
{
	register char *p;

	if (source(s))
		return;
	p = regs[ai];
 	while (*p++ = *s) {
		if (*s++ == ',')
			break;
	}
	*--p = '\0';
}

/* look through register contents table, clearing entries based on value
 * of address register n (n passed as arg).
 */
areg(reg)
  {	register int i;
	register char *p;

	for (i = 0; i < RT2; i++) {
	  p = regs[i];
	  if (*p++ == 'a' && *p == (reg - 8 + '0')) regs[i][0] = 0;
	}
}

/* update register contents table assuming operand passed as argument
 * was affected by instruction
 */
dest(as)
char *as;
{
	register char *s;
	register int i;

	s = as;
	(void) source(s);

	/* if dest was a register, update registers table entry */
	if ((i = isreg(s)) >= 0) regs[i][0] = 0;

	/* if dest was an address reg, clear table entries for regs that
	 * were loaded using that address reg.
	 */
	if (i >= 8) areg(i);

	/* clear any regs that claim to have copy of dest's value */
	while ((i = findrand(s)) >= 0) regs[i][0] = 0;

	/* if there is any indirection, we don't know anything afterward */
	if (*s++=='a' && *s>='0' && *s++<='5' && *s=='@') {
		if (regs[0][0] != '#') regs[0][0] = 0;
		if (regs[1][0] != '#') regs[1][0] = 0;
		if (regs[2][0] != '#') regs[2][0] = 0;
		if (regs[3][0] != '#') regs[3][0] = 0;
		if (regs[4][0] != '#') regs[4][0] = 0;
		if (regs[5][0] != '#') regs[5][0] = 0;
		if (regs[6][0] != '#') regs[6][0] = 0;
		if (regs[7][0] != '#') regs[7][0] = 0;
		if (regs[8][0] != '#') regs[8][0] = 0;
		if (regs[9][0] != '#') regs[9][0] = 0;
		if (regs[10][0] != '#') regs[10][0] = 0;
		if (regs[11][0] != '#') regs[11][0] = 0;
		if (regs[12][0] != '#') regs[12][0] = 0;
		if (regs[13][0] != '#') regs[13][0] = 0;
		/*
		for (i=0; i<NREG; i++) {
			if (regs[i][0] != '#') regs[i][0] = 0;
		}
		*/
		conloc[0] = 0;
	}
}

/* set RT1 (source) "register" from assy. language code */
singop(ap)
struct node *ap;
{
	register char *p1, *p2;

	p1 = ap->code;
	p2 = regs[RT1];
	while (*p2++ = *p1++);
	regs[RT2][0] = 0;
}
