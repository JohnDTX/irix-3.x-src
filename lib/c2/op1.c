#include "opt.h"

char	revbr[] = { JNE, JEQ, JGT, JLT, JGE, JLE, JHIS, JLOS, JHI, JLO };
int	isn	= 20000;

opsetup()
{
	register struct optab *optp, **ophp;
	register char *p;
	register n;

	for (optp = optab; p = optp->opstring; optp++) {
		ophp = &ophash[(((p[0]<<3)+p[1])&077777) % OPHS];
		while (*ophp++)
			if (ophp >= &ophash[OPHS])
				ophp = ophash;
		*--ophp = optp;
	}
	for (optp = optab; n = optp->opcode; optp++) {
		ophp = &opcdhash[(n<<1) % OPCDHS];
		while (*ophp++)
			if (ophp >= &opcdhash[OPCDHS])
				ophp = opcdhash;
		*--ophp = optp;
	}
}

oplook()
{
	register struct optab *optp;
	register char *lp, *op;
	static char tmpop[32];
	register struct optab **ophp;

	op = tmpop;
	*op = *(op+1) = *(op+2) = 0;
	lp = line;
	while (*lp=='\t' || *lp==' ') lp++;
	while (*lp && *lp!=' ' && *lp!='\t') *op++ = *lp++;
	*op++ = 0;
	while (*lp=='\t' || *lp==' ') lp++;
	curlp = lp;
	ophp = &ophash[(((tmpop[0]<<3)+tmpop[1])&077777) % OPHS];
	while (optp = *ophp) {
		op = optp->opstring;
		lp = tmpop;
		while (*lp == *op++)
			if (*lp++ == 0)
				return(optp->opcode);
		op--;
		if (*op==0 && *(lp+1)==0) {
			if (*lp=='l')
				return(optp->opcode + (LONG<<8));
			if (*lp=='b')
				return(optp->opcode + (BYTE<<8));
			if (*lp=='w')
				return(optp->opcode + (WORD<<8));
		}
		ophp++;
		if (ophp >= &ophash[OPHS])
			ophp = ophash;
	}
	curlp = line;
	return(0);
}

refcount()
{
	register struct node *p, *lp;
	static struct node *labhash[LABHS];
	register struct node **hp;
	struct node *thp;

	for (hp = labhash; hp < &labhash[LABHS];)
		*hp++ = 0;
	for (p = first.forw; p!=0; p = p->forw)
		if (p->op==LABEL) {
			labhash[p->labno % LABHS] = p;
			p->refc = 0;
		}
	for (p = first.forw; p!=0; p = p->forw) {
		if (p->op==JBR || p->op==CBR || p->op==JSW || p->op==XFER) {
			p->ref = 0;
			lp = labhash[p->labno % LABHS];
			if (lp==0 || p->labno!=lp->labno)
			for (lp = first.forw; lp!=0; lp = lp->forw) {
				if (lp->op==LABEL && p->labno==lp->labno)
					break;
			}
			if (lp) {
				thp = nonlab(lp)->back;
				if (thp!=lp) {
					p->labno = thp->labno;
					lp = thp;
				}
				p->ref = lp;
				lp->refc++;
			}
		}
	}
	for (p = first.forw; p!=0; p = lp) {
	  lp = p->forw;
	  if (p->op==LABEL && p->refc==0 && lp && lp->op!=0 && lp->op!=JSW) decref(p);
	}
}

iterate()
{
	register struct node *p, *rp, *p1;

	nchange = 0;
	for (p = first.forw; p!=0; p = p->forw) {
		if ((p->op==JBR||p->op==CBR||p->op==JSW) && p->ref) {
			rp = nonlab(p->ref);
			if (rp->op==JBR && rp->labno && p->labno!=rp->labno) {
				nbrbr++;
				p->labno = rp->labno;
				decref(p->ref);
				rp->ref->refc++;
				p->ref = rp->ref;
				nchange++;
			}
		}
		if (p->op==CBR && (p1 = p->forw)->op==JBR) {
			rp = p->ref;
			do
				rp = rp->back;
			while (rp->op==LABEL);
			if (rp==p1) {
				decref(p->ref);
				p->ref = p1->ref;
				p->labno = p1->labno;
				p1->forw->back = p;
				p->forw = p1->forw;
				p->subop = revbr[p->subop];
				p1->ref = freenodes;
				freenodes = p1;
				nchange++;
				nskip++;
			}
		}
		if (p->op==JBR || p->op==JMP) {
			while (p->forw && p->forw->op!=LABEL && p->forw->op!=DLABEL
				&& p->forw->op!=EROU && p->forw->op!=END
				&& p->forw->op!=0 && p->forw->op!=DATA
				&& p->forw->op!=BSS) {
				nchange++;
				iaftbr++;
				if (p->forw->ref)
					decref(p->forw->ref);
				p1 = p->forw;
				p->forw = p->forw->forw;
				p->forw->back = p;
				p1->ref = freenodes;
				freenodes = p1;
			}
			rp = p->forw;
			while (rp && rp->op==LABEL) {
				if (p->ref == rp) {
					p->back->forw = p->forw;
					p->forw->back = p->back;
					p->ref = freenodes;
					freenodes = p;
					p = p->back;
					decref(rp);
					nchange++;
					njp1++;
					break;
				}
				rp = rp->forw;
			}
			xjump(p);
			p = codemove(p);
		}
	}
}

xjump(ap)
 struct node *ap;
{
	register struct node *p1, *p2, *p3;

	p1 = ap;
	if ((p2 = p1->ref)==0)
		return;
	for (;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		while ((p2 = p2->back) && p2->op==LABEL);
		if (!equop(p1, p2) || p1==p2)
			return;
		p3 = insertl(p2);
		p1->op = JBR;
		p1->subop = 0;
		p1->ref = p3;
		p1->labno = p3->labno;
		p1->code = 0;
		nxjump++;
		nchange++;
	}
}

struct node *
insertl(ap)
struct node *ap;
{
	register struct node *lp, *op;

	op = ap;
	if (op->op == LABEL) {
		op->refc++;
		return(op);
	}
	if (op->back->op == LABEL) {
		op = op->back;
		op->refc++;
		return(op);
	}
	lp = (struct node *)getnode();
	lp->op = LABEL;
	lp->labno = isn++;
	lp->ref = 0;
	lp->code = 0;
	lp->refc = 1;
	lp->lineno = 0;
	lp->misc = 0;
	lp->back = op->back;
	lp->forw = op;
	op->back->forw = lp;
	op->back = lp;
	return(lp);
}

struct node *
codemove(ap)
struct node *ap;
{
	register struct node *p1, *p2, *p3;
	struct node *t, *tl;
	int n;

	p1 = ap;
	if (p1->op!=JBR || (p2 = p1->ref)==0)
		return(p1);
	while (p2->op == LABEL)
		if ((p2 = p2->back) == 0)
			return(p1);
	if (p2->op!=JBR && p2->op!=JMP)
		goto ivloop;
	p2 = p2->forw;
	p3 = p1->ref;
	while (p3) {
		if (p3->op==JBR || p3->op==JMP) {
			if (p1==p3)
				return(p1);
			ncmot++;
			nchange++;
			p1->back->forw = p2;
			p1->forw->back = p3;
			p2->back->forw = p3->forw;
			p3->forw->back = p2->back;
			p2->back = p1->back;
			p3->forw = p1->forw;
			decref(p1->ref);
			return(p2);
		} else
			p3 = p3->forw;
	}
	return(p1);
ivloop:
	if (p1->forw->op!=LABEL)
		return(p1);
	p3 = p2 = p2->forw;
	n = 16;
	do {
		if ((p3 = p3->forw) == 0 || p3==p1 || --n==0)
			return(p1);
	} while (p3->op!=CBR || p3->labno!=p1->forw->labno);
	do 
		if ((p1 = p1->back) == 0)
			return(ap);
	while (p1!=p3);
	p1 = ap;
	tl = insertl(p1);
	p3->subop = revbr[p3->subop];
	decref(p3->ref);
	p2->back->forw = p1;
	p3->forw->back = p1;
	p1->back->forw = p2;
	p1->forw->back = p3;
	t = p1->back;
	p1->back = p2->back;
	p2->back = t;
	t = p1->forw;
	p1->forw = p3->forw;
	p3->forw = t;
	p2 = insertl(p1->forw);
	p3->labno = p2->labno;
	p3->ref = p2;
	decref(tl);
	if (tl->refc<=0)
		nrlab--;
	loopiv++;
	nchange++;
	return(p3);
}

comjump()
{
	register struct node *p1, *p2, *p3;

	for (p1 = first.forw; p1!=0; p1 = p1->forw)
		if (p1->op==JBR && (p2 = p1->ref) && p2->refc > 1)
			for (p3 = p1->forw; p3!=0; p3 = p3->forw)
				if (p3->op==JBR && p3->ref == p2)
					backjmp(p1, p3);
}

backjmp(ap1, ap2)
struct node *ap1, *ap2;
{
	register struct node *p1, *p2, *p3, *ptemp;

	p1 = ap1;
	p2 = ap2;
	for(;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		p2 = p2->back;
		if (equop(p1, p2)) {
			p3 = insertl(p1);
			p2->back->forw = p2->forw;
			p2->forw->back = p2->back;
			ptemp = p2;
			p2 = p2->forw;
			ptemp->ref = freenodes;
			freenodes = ptemp;
			decref(p2->ref);
			p2->labno = p3->labno;
			p2->ref = p3;
			nchange++;
			ncomj++;
		} else
			return;
	}
}
