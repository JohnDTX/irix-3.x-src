#include "opt.h"

long findequ();

/*
 * do post processing on the opcodes
 */
lastopt()
{
	optmovml();	/* remove modify MOVMLs where appropriate */
	rmmisc();	/* remove misc code */
	opts2();	/* optimize single instructions */
	dbra();		/* insert dbras if possible */
	lea();		/* replace inefficient lea's */
	seqmov();	/* compress sequential move instructions */
	rmfinal();	/* one last scan */
}

/*
 * final remove of misc code
 *	remove stack probe instruction if '-P' specified
 *	remove unnecessary '.TEXT'
 *	remove 'or #0,X'
 *	'clrl dx' to 'moveq 0,dX'
 *	'mov #0,Ax' to 'sub Ax,Ax' (same exec time but one less word)
 *	'link ; unlk'
 *	'jsr' to 'bsr'
 */
rmfinal()
{
	register struct node *p, *p2, *p3;
	register r;
	char lregs[MAXSIZE];
	register segment;

	if ((p = first.forw) == 0)
		return;
	segment = 0;
	for (; p2 = p->forw; p = p2) {
		if (p->op==TEXT) {
			if (segment == TEXT)
				release(p); else
				segment = TEXT;
			continue;
		}
		if (p->op==DATA || p->op==BSS) {
			segment = 0;
			continue;
		}
		/* 'mov #0,Ax' to 'sub Ax,Ax' */
		if (p->op==MOV && (p->subop==LONG || p->subop==WORD)) {
			dualop(p);
			if ((r=isareg(regs[RT2])) >= 0 && r <= 7 &&
			    numeric(regs[RT1]) &&
			    numcvt(&regs[RT1][1]) == 0L) {
				(void) sprintf(lregs, "a%d,a%d", r, r);
				newcode(p, lregs);
				p->op = SUB;
				p->subop = LONG;
				continue;
			}
		}
		/* 'clrl dx' to 'moveq 0,dX' */
		if (p->op==CLR && p->subop==LONG) {
			singop(p);
			if ((r=isreg(regs[RT1])) >= 0 && r <= 7) {
				(void) sprintf(lregs, "#0,%s", regs[RT1]);
				newcode(p, lregs);
				p->op = MOVEQ;
				p->subop = 0;
				continue;
			}
		}
		/* turn jsr into bsr */
		if (Bflg && p->op == JSR) {
			singop(p);
			if (regs[RT1][0] == '_' && findglob(regs[RT1])) {
				p->op = BSR;
				continue;
			}
		}
		/* remove 'or #0,X' */
		if (p->op==OR) {
			dualop(p);
			if (numeric(regs[RT1]) &&
			    numcvt(&regs[RT1][1]) == 0L && p2->op!=CBR) {
				release(p);
				continue;
			}
		}
		if (p->op == LINK)
			segment = TEXT;
		if (p->op == LINK && p2->op == UNLK) {
			release(p);
			release(p2);
			break;
		}
		if ((p3 = p2->forw) == 0) break;
		if (Pflg && p->op == LINK && p2->op == TST && p2->subop==BYTE) {
			singop(p2);
			if (strncmp(regs[RT1], "sp@(-.M", 7)==0 ||
			    strcmp(regs[RT1], "sp@(-132)")==0) {
				release(p2);
				p2 = p;
				continue;
			}
		}
		if (p->op == LINK && p3->op == UNLK && p2->op == 0 &&
		    p2->subop == 0) {
			release(p);
			release(p3);
			break;
		}
	}
}

/*
 * remove misc code
 *	'movl X,dx; tstl dx; beq lll' to 'movl X,dx; beq lll'
 * 	'movl d0,d1; movl d1,d0'
 *	'movw X,d0; andl #65535,d0' to 'clrl d0; movw X,d0'
 *	'movw X,d0; andl #255,d0'   to 'clrl d0; movb X,d0'
 *	'movb X,d0; andl #255,d0'   to 'clrl d0; movb X,d0'
 *	'andl <=#0xFFFF,d0; cmpl #0xFFFF,d0 to 'andl #0xFFFF,d0; cmpw ...'
 */
rmmisc()
{
	register struct node *p, *p2, *p3;
	register int r1, r2;
	int r3, r4;
	long n1, n2;
	char lregs[MAXSIZE], tregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		/* movl X,dx; tstl dx; beq lll */
		if (p->op==TST &&  p->subop==LONG &&
		    p2->op==CBR && jmpcond(p2) &&
		    (p2->misc || freed0(p2)) && (p3=p->back) &&
		    ((p3->op==MOV && p3->subop==LONG) || p3->op==MOVEQ)) {
		    /* tstl Dx */
		    singop(p);
		    if ((r1=isreg(regs[RT1]))>=0 && r1 <= 7) {
			/* movl X,dx */
			dualop(p3);
			if (isreg(regs[RT2])==r1) {
				release(p);
				continue;
			}
		    }
		}
		if (p->op == MOV && p2->op == MOV &&
		    ((p->subop == LONG && p2->subop == LONG) ||
		     (p->subop == WORD && p2->subop == WORD) ||
		     (p->subop == BYTE && p2->subop == BYTE))) {
			dualop(p);
			if ((r1 = isreg(regs[RT1])) < 0) continue;
			if ((r2 = isreg(regs[RT2])) < 0) continue;
			dualop(p2);
			if ((r3 = isreg(regs[RT1])) < 0) continue;
			if ((r4 = isreg(regs[RT2])) < 0) continue;
			if (r1 != r4 || r2 != r3) continue;
			/* Ignore mov Ax,D0 to set condition codes */
			if (r3 >= 8 && r4 <= 2) continue;
			/* Ignore movw XX,Ax */
			if (p2->subop == WORD && r4 >= 8) continue;
			release(p2);
			p2 = p;
			continue;
		}
		if (p->op == MOV && p2->op == AND && p2->subop == LONG) {
			dualop(p);
			/* mov X,Dx */
			if ((r1 = isreg(regs[RT2])) >= 0 && r1 <= 7) {
			    strcpy(lregs, regs[RT1]);
			    dualop(p2);
			    /* andl #255,Dx */
			    if (p->subop == LONG && isareg(lregs) == -1 &&
			        numeric(regs[RT1]) &&
				numcvt(&regs[RT1][1]) == 255L &&
				r1 == isreg(regs[RT2]) && recalc(lregs, "+3")) {
				    p->op = CLR;
				    p->subop = LONG;
				    (void) sprintf(tregs, "d%d", r1);
				    newcode(p, tregs);
				    p2->op = MOV;
				    p2->subop = BYTE;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p2, tregs);
				    continue;
			    } else
			    if (p->subop == WORD && isareg(lregs) == -1 &&
			        numeric(regs[RT1]) &&
				numcvt(&regs[RT1][1]) == 255L &&
				r1 == isreg(regs[RT2]) && recalc(lregs, "+1")) {
				    p->op = CLR;
				    p->subop = LONG;
				    (void) sprintf(tregs, "d%d", r1);
				    newcode(p, tregs);
				    p2->op = MOV;
				    p2->subop = BYTE;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p2, tregs);
				    continue;
			    } else
			    if (p->subop == BYTE && numeric(regs[RT1]) &&
				numcvt(&regs[RT1][1]) == 255L &&
				r1 == isreg(regs[RT2])) {
				    p->op = CLR;
				    p->subop = LONG;
				    (void) sprintf(tregs, "d%d", r1);
				    newcode(p, tregs);
				    p2->op = MOV;
				    p2->subop = BYTE;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p2, tregs);
				    continue;
			    } else
			    /* andl #65535,Dx */
			    if (p->subop == LONG && numeric(regs[RT1]) &&
				numcvt(&regs[RT1][1]) == 65535L &&
				r1 == isreg(regs[RT2]) && recalc(lregs, "+2")) {
				    p->op = CLR;
				    p->subop = LONG;
				    (void) sprintf(tregs, "d%d", r1);
				    newcode(p, tregs);
				    p2->op = MOV;
				    p2->subop = WORD;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p2, tregs);
				    continue;
			    } else
			    if (p->subop == WORD && numeric(regs[RT1]) &&
				numcvt(&regs[RT1][1]) == 65535L &&
				r1 == isreg(regs[RT2])) {
				    p->op = CLR;
				    p->subop = LONG;
				    (void) sprintf(tregs, "d%d", r1);
				    newcode(p, tregs);
				    p2->op = MOV;
				    p2->subop = WORD;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p2, tregs);
				    continue;
			    }
			}
		}
		if ( p->op == MOV &&  p->subop == LONG &&
		    p2->op == ASR && p2->subop == LONG &&
		    p3->op == AND && p3->subop == LONG) {
			dualop(p);
			/* movl X,Dx */
			if ((r1 = isreg(regs[RT2])) >= 0 && r1 <= 7 &&
			 isreg(regs[RT1]) == -1) {
			  strcpy(lregs, regs[RT1]);
			  dualop(p2);
			  if (isreg(regs[RT2]) == r1 && numeric(regs[RT1]) &&
			   numcvt(&regs[RT1][1]) == 8L) {
			    dualop(p3);
			    /* andl #255,Dx */
			    if (numeric(regs[RT1]) &&
				numcvt(&regs[RT1][1]) == 255L &&
				r1 == isreg(regs[RT2]) &&
				btstpx(lregs, 2)) {
				    p->op = CLR;
				    p->subop = LONG;
				    (void) sprintf(tregs, "d%d", r1);
				    newcode(p, tregs);
				    p2->op = MOV;
				    p2->subop = BYTE;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p2, tregs);
				    release(p3);
				    continue;
			    }
			  }
			}
		}
		if (p->op == MOV && p->subop == WORD && p2->op == ASR &&
		    p2->subop==LONG && p3->op == AND && p3->subop == LONG) {
			dualop(p);
			/* movw X,Dx */
			if ((r1 = isreg(regs[RT2])) >= 0 && r1 <= 7 &&
			 isreg(regs[RT1]) == -1) {
			  strcpy(lregs, regs[RT1]);
			  dualop(p2);
			  if (isreg(regs[RT2]) == r1 && numeric(regs[RT1]) &&
			   numcvt(&regs[RT1][1]) == 8L) {
			    dualop(p3);
			    /* andl #255,Dx */
			    if (numeric(regs[RT1]) &&
				numcvt(&regs[RT1][1]) == 255L &&
				r1 == isreg(regs[RT2])) {
				    p->op = CLR;
				    p->subop = LONG;
				    (void) sprintf(tregs, "d%d", r1);
				    newcode(p, tregs);
				    p2->op = MOV;
				    p2->subop = BYTE;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p2, tregs);
				    release(p3);
				    continue;
			    }
			  }
			}
		}
		if (p->op == MOV && p->subop == LONG && p2->op == AND &&
		    p2->subop == LONG) {
			dualop(p);
			/* movl X,Dx */
			if ((r1 = isreg(regs[RT2])) >= 0 && r1 <= 7) {
			    strcpy(lregs, regs[RT1]);
			    dualop(p2);
			    /* andl #xnnn,Dx */
			    if (numeric(regs[RT1]) &&
				(n1 = numcvt(&regs[RT1][1])) <= 65535L &&
				n1 >= 0 && r1 == isreg(regs[RT2]) &&
				recalc(lregs, "+2")) {
				    p->subop = WORD;
				    (void) sprintf(tregs, "%s,d%d", lregs, r1);
				    newcode(p, tregs);
				    continue;
			    }
			}
		}
		if (p->op == AND && p->subop == LONG && p2->op == CMP &&
		    p2->subop == LONG) {
			dualop(p);
			/* andl #0xnnn,Dx */
			if (numeric(regs[RT1]) &&
			    (n1 = numcvt(&regs[RT1][1])) <= 0x7FFFL &&
			    n1 >= 0 && (r1=isreg(regs[RT2])) >= 0 && r1 <= 7) {
			        dualop(p2);
			        /* cmpl #0xnnn,Dx */
			        if (numeric(regs[RT1]) &&
				    (n2 = numcvt(&regs[RT1][1])) <= 0x7FFFL &&
				    n2 >= 0 && r1 == isreg(regs[RT2]))
				        p2->subop = WORD;
			}
		}
	}
}

/*
 * replace code with dbra instructions
 * NOTE: The dbra instruction only works on 16 bits of a data register.
 */
dbra()
{
	register struct node *p, *p2, *p3, *p4;
	int r;
	char lregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if (p->op==SUBQ && p->subop==WORD  &&
		    p2->op==CMP && p2->subop==WORD &&
		    (p3->op==CBR || p3->op==XFER) && p3->subop==JNE &&
		    p3->labno!=0) {
			/* subqw #1,Dx */
			dualop(p);
			if (numeric(regs[RT1])==0) continue;
			if (numcvt(&regs[RT1][1]) != 1) continue;
			if ((r = isreg(regs[RT2])) < 3 || r > 7) continue;
			/* cmpw #-1,Dx */
			dualop(p2);
			if (numeric(regs[RT1])==0) continue;
			if (numcvt(&regs[RT1][1]) != -1) continue;
			if (isreg(regs[RT2]) != r) continue;
			/* matched */
			release(p);
			release(p2);
			(void) sprintf(lregs, "d%d,.L%d", r, p3->labno);
			newcode(p3, lregs);
			p3->op = DB;
			p3->subop = JRA;
			p2 = p3;
		}
		if (p->op==SUBQ && p->subop==WORD  &&
		    p2->op==CMP && p2->subop==WORD &&
		    p3->op==CBR && p3->subop==0 &&
		    p3->labno!=0 && (p4 = p3->forw)!=0 &&
		    p4->op==JBR && p4->subop==0 &&
		    p4->labno!=0) {
			/* subqw #1,Dx */
			dualop(p);
			if (numeric(regs[RT1])==0) continue;
			if (numcvt(&regs[RT1][1]) != 1) continue;
			if ((r = isreg(regs[RT2])) < 3 || r > 7) continue;
			/* cmpw #-1,Dx */
			dualop(p2);
			if (numeric(regs[RT1])==0) continue;
			if (numcvt(&regs[RT1][1]) != -1) continue;
			if (isreg(regs[RT2]) != r) continue;
			/* matched */
			release(p);
			release(p2);
			(void) sprintf(lregs, "d%d,.L%d", r, p4->labno);
			newcode(p3, lregs);
			p3->op = DB;
			p3->subop = JRA;
			r = p3->labno;
			p3->labno = p4->labno;
			p4->labno = r;
			p2 = p3;
		}
	}
}

/*
 * optimize moveml instructions
 */
char *movmldat[] = {
	"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"
};

optmovml()
{
	register struct node *p;
	register int nreg;
	char *reg1, *reg2;
	int i;

	for (p = first.forw; p != 0; p = p->forw) {
		if (p->op != MOVEM || p->subop != LONG)
			continue;
		dualop(p);
		nreg = regmask(p);
		reg1 = reg2 = 0;
		for (i = 0; i < 16; i++) {
			if (nreg & 1) {
				if (reg1 == 0)
					reg1 = movmldat[i];
				else if (reg2 == 0)
					reg2 = movmldat[i];
				else
					goto next;
			}
			nreg >>= 1;
		}
		if (reg1 == 0 && reg2 == 0) {
			release(p);
		} else
		if (reg1 != 0 && reg2 == 0)
			movmlrep(p, reg1);
		else
			movmladd(p, reg2, reg1);
	next:
		continue;
	}
}

/*
 * optimize single instructions
 */
opts2()
{
	register struct node *p;
	register long n1;
	int nbsr, njsr;

	nbsr = njsr = 0;
	for (p = first.forw; p != 0; p = p->forw) {
		switch (p->op) {
		case BSR: nbsr++; break;
		case JSR: njsr++; break;
		case AND:
		case OR:
			dualop(p);
			/* check for 'and/or x,x' */
			if (strcmp(regs[RT1], regs[RT2])==0) {
				release(p);
				continue;
			}
			break;
		case MOV:
			dualop(p);
			/* compress long constant moves to Ax registers */
			if (p->subop != LONG) break;
			if (numeric(regs[RT1])==0) break;
				/* move to Ax register */
			if (isreg(regs[RT2]) < 8) break;
			n1 = numcvt(&regs[RT1][1]);
			if ((n1&0xFFFF8000L)==0||(n1&0xFFFF8000L)==0xFFFF8000L)
				p->subop = WORD;
			break;
		case ADD:
		case SUB:
			/* compress long adds/subs to Ax registers */
			if (p->subop != LONG) break;
			dualop(p);
			if (numeric(regs[RT1])==0) break;
			if (isareg(regs[RT2]) < 0) break;
			n1 = numcvt(&regs[RT1][1]);
			if ((n1&0xFFFF8000L)==0||(n1&0xFFFF8000L)==0xFFFF8000L)
				p->subop = WORD;
			break;
		}
	}
	/* remove 'tstb sp@(.Mnn)' if possible (always from kernel) */
	if (Kflg ||
	   (njsr == 0 && nbsr == 0 && (n1 = findequ('F', first.forw)) >= 0 &&
	    n1 < 100)) {
		for (p = first.forw; p != 0; p = p->forw) {
			if (p->op == TST) {
				if (p->subop == BYTE) {
				    singop(p);
				    if (strncmp(regs[RT1], "sp@(-.M", 7)==0 ||
				        strcmp(regs[RT1], "sp@(-132)")==0)
					    release(p);
				}
				break;
			}
		}
	}
}

/*
 * replace one moveml instructions with two movl instructions
 */
movmladd(p, regstr1, regstr2)
register struct node *p;
char *regstr1, *regstr2;
{
	register struct node *q;
	char locbuf1[MAXSIZE], locbuf2[MAXSIZE];

	if (movmldir() == 0) {
		(void) sprintf(locbuf1, "%s,sp@-", regstr1);
		(void) sprintf(locbuf2, "%s,sp@-", regstr2);
	} else {
		(void) sprintf(locbuf1, "sp@+,%s", regstr2);
		(void) sprintf(locbuf2, "sp@+,%s", regstr1);
	}
	newcode(p, locbuf1);
	p->subop = LONG;
	p->op = MOV;
	q = (struct node *)getnode();
	q->op = MOV;
	q->subop = LONG;
	q->labno = 0;
	q->code = 0;
	newcode(q, locbuf2);
	q->ref = 0;
	q->forw = p->forw;
	p->forw->back = q;
	p->forw = q;
	q->back = p;
}

/*
 * replace code in a movml instruction
 */
movmlrep(p, regstr)
register struct node *p;
char *regstr;
{
	char locbuf[MAXSIZE];

	if (movmldir() == 0)
		(void) sprintf(locbuf, "%s,sp@-", regstr);
	else
		(void) sprintf(locbuf, "sp@+,%s", regstr);
	newcode(p, locbuf);
	p->subop = LONG;
	p->op = MOV;
}

/*
 * return zero for "movml ???,a6@-."
 */
movmldir()
{
	return(strncmp(regs[RT2], "a6@(-.", 6));
}

/*
 * recalculate an operand offset based on a long move changing into a word move
 */
recalc(str, inc)
register char *str;
register char *inc;
{
	register char *cp;

	if (isreg(cp = str) >= 0)
		return(1);
	while(*cp++) ;
	cp--;
		/* simple label. append "inc" */
	if (*str == '_' || *str == '.') {
		while(*cp++ = *inc++) ;
		return(1);
	}
		/* Ax@(nnnn). append "inc" to nnnn */
	if (*str++ == 'a' && *str >= '0' && *str++ <= '7' && *str++ == '@' &&
	    *str == '(' && *--cp == ')') {
		while (*cp++ = *inc++) ;
		*(cp-1) = ')';
		*cp = 0;
		return(1);
	}
	return(0);

}

/*
 * return the register mask associated with this code segment
 */
regmask(p)
struct node *p;
{
	register char *cp;

	if (movmldir()==0) {
		cp = regs[RT1];
		if (*cp++ == '#' && *cp == '0' && *(cp+1) == 'x')
			return((short)numcvt(cp));
	} else {
		if (strncmp(regs[RT1], "a6@(-.", 6))
			return(-1);
		cp = regs[RT2];
		if (*cp++ == '#' && *cp == '0' && *(cp+1) == 'x')
			return((short)numcvt(cp));
	}
	return(findequ('S', p));
}

/*
 * find the value of the associated '.Fx =' or '.Sx =' string
 */
long
findequ(c, p)
char c;
register struct node *p;
{
	register char *cp;

	for (; p != 0; p = p->forw) {
		if (p->op != 0 || (cp = p->code)==0) continue;
		if (*cp++ != '.' || *cp++ != c) continue;
		while (*cp >= '0' && *cp <= '9') cp++;
		if (*cp++ != ' ' || *cp++ != '=' || *cp++ != ' ') continue;
		return((short)numcvt(cp));
	}
	return(-1);
}

/*
 * return the number pointed to by cp
 */
long
numcvt(cp)
register char *cp;
{
	register c;
	long n;
	int isneg=0;

	n = 0;
	if (*cp == '-') { isneg++; cp++; }
	if (*cp == '0' && *(cp+1)=='x') {
		cp += 2;
		for (;;) {
			if ((c = *cp++) >= '0' && c <= '9')
				n = (n << 4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				n = (n << 4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				n = (n << 4) + c - 'A' + 10;
			else if (c == 0)
				return(isneg?-n:n);
			else
				break;
		}
	} else {
		while ((c = *cp++) >= '0' && c <= '9')
			n = n * 10 + c - '0';
		if (c == 0)
			return(isneg?-n:n);
	}
	return(-1);
}

/*
 * return non zero if the passed string is a simple number
 */
numeric(cp)
register char *cp;
{
	register c;

	if (*cp++ != '#')
		return(0);
	if (*cp == '-') cp++; /* GB */
	if (*cp == '0' && *(cp+1)=='x') {
		cp += 2;
		for (;;) {
			if (((c = *cp++) >= '0' && c <= '9') ||
			     (c >= 'a' && c <= 'f') ||
			     (c >= 'A' && c <= 'F'))
				continue;
			if (c == 0) return(1);
			break;
		}
	} else {
		while (*cp >= '0' && *cp <= '9')
			cp++;
		if (*cp == 0) return(1);
	}
	return(0);
}

/*
 * release resources for an instruction
 */
release(p2)
register struct node *p2;
{
	register struct node *p;

	p = p2->back;
	if (p2->code) {
		cfree(p2->code);
		p2->code = 0;
	}
	p2->forw->back = p;
	p->forw = p2->forw;
	p2->ref = freenodes;
	freenodes = p2;
}
