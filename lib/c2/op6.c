#include "opt.h"

/*
 * yet more optimizations
 */

/* type fields */
#define ARIDA	5	/* Address register indirect plus displacement */

struct idecode {
	int	i_type;		/* operand type */
	int	i_reg;		/* operand register */
	long	i_offs;		/* operand offset */
};

/* #define AREGOPT */

/*
 * optimize address register utilization
 */
aregopt()
{
#ifdef AREGOPT
	register struct node *p, *p2;
	register int r, rt;
	char lregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	while (p2 = p->forw)
		p = p2;
	for (; p != first.forw; p = p->back) {
	    if (p->op != MOV || p->misc == 0)
	        continue;
	    dualop(p);
	    /* mov d0,dx */
	    if (isreg(regs[RT1]) != 0 || (r = isreg(regs[RT2])) < 3 || r > 7)
		continue;
	    if ((p2 = p->forw) == 0) continue;
	    /* verification loop */
	    for (p2 = p->back; p2 != first.forw; p2 = p2->back) {
		switch(p2->op) {
		case MOV: case MOVEQ:
			dualop(p2);
			/* mov[q] xxx,d[1-7] */
			if ((rt = isreg(regs[RT2])) >= 1 &&
			     rt <= 7 && rt != r)
				break;
			/* mov[q] xxx,d0 */
			if (rt == 0 &&
			    (regs[RT1][0] == '#' || regs[RT1][0] == 'a' ||
			     regs[RT1][0] == '_' ||
			     (regs[RT1][0] == '.' && regs[RT1][1]=='L') ||
			     ((rt = isreg(regs[RT1])) >= 3 && rt <= 7 &&
			      rt != r))) {
			    /* replacement loop */
			    if ((p2 = p->forw) != 0 && p2->op == MOV) {
				/* movl d0,xxx */
				dualop(p2);
				if (isreg(regs[RT1]) == 0) {
				    (void) sprintf(lregs, "d%d,%s", r,
					    regs[RT2]);
				    newcode(p2, lregs);
				}
			    }
			    for (p2=p->back; p2 != first.forw; p2 = p2->back) {
				switch(p2->op) {
				case MOV: case MOVEQ:
				    dualop(p2);
				    if (isreg(regs[RT2]) != 0)
					break;
				    /* mov xxx,d0 */
				    (void) sprintf(lregs, "%s,d%d", regs[RT1],
					    r);
				    newcode(p2, lregs);
				    release(p);
				    p = p2;
				    goto next;
				case ADD: case SUB:
				case AND: case OR:
				case ASR: case ASL:
				case LSR: case LSL:
				case ADDQ: case SUBQ:
					dualop(p2);
					if (isreg(regs[RT2]) != 0)
						break;
					(void) sprintf(lregs, "%s,d%d",
						regs[RT1], r);
					newcode(p2, lregs);
					break;
				case EXT:
				case NEG:
				case NOT:
					singop(p2);
					if (isreg(regs[RT1]) != 0)
						break;
					(void) sprintf(lregs, "d%d", r);
					newcode(p2, lregs);
					break;
				default:
					fprintf(stderr,
						"c2:aregopt error: op=%d\n",
						p2->op);
					goto next;
				}
			    }
			}
			goto next;
		case ADD: case SUB:
		case AND: case OR:
		case ADDQ: case SUBQ:
			dualop(p2);
			if ((regs[RT1][0] == '_' || regs[RT1][0] == '#' ||
			    (regs[RT1][0] == '.' && regs[RT1][1] == 'L')) &&
			    (rt = isreg(regs[RT2])) >= 0 && rt != r)
				break;
			goto next;
		case ASR: case ASL:
		case LSR: case LSL:
			dualop(p2);
			if ((regs[RT1][0] == '#' || isreg(regs[RT1]) == 1) &&
			    (rt = isreg(regs[RT2])) >= 0 && rt != r)
				break;
			goto next;
		case EXT:
		case NEG:
		case NOT:
			singop(p2);
			if ((rt = isreg(regs[RT1])) >= 0 && rt != r)
				break;
			goto next;
		default:
			goto next;
		}
	    }
	next:
	    continue;
	}
#endif AREGOPT
}

#define DREGOPT

/*
 * optimize data register utilization
 */
dregopt()
{
	register struct node *p, *p2, *p3, *p4;
	register int r, rt;
	char lregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	while (p = p->forw) {
	    if (p->op != MOV || p->subop!=LONG || p->lineno==0)
	        continue;
	    dualop(p);
	    /* mov XXX,dx */
	    if (isreg(regs[RT2]) != 0) continue;
	    if ((p2 = p->forw) == 0) continue;
	    /* addl NNN,d0 */
	    if (p2->op != ADD || p2->subop != LONG) continue;
	    dualop(p2);
	    if (isreg(regs[RT2]) != 0) continue;
	    if ((p3 = p2->forw) == 0) continue;
	    /* movl d0,a0 */
	    if (p3->op != MOV || p3->subop != LONG) continue;
	    dualop(p3);
	    if (isreg(regs[RT1]) || isareg(regs[RT2])) continue;
	    if ((p4 = p3->forw) == 0) continue;
	    /* mov xxx,a0@ */
	    if (p4->op != MOV || p4->misc == 0) continue;
	    dualop(p4);
	    if (regs[RT1][0]=='a' && regs[RT1][1]=='0') continue;
	    if (strcmp(regs[RT2], "a0@") == 0) {
		    dualop(p);
		    (void) sprintf(lregs, "%s,a0", regs[RT1]);
		    newcode(p, lregs);
		    dualop(p2);
		    (void) sprintf(lregs, "%s,a0", regs[RT1]);
		    newcode(p2, lregs);
		    release(p3);
	    }
	}
#ifdef DREGOPT
	p = first.forw;
	while (p2 = p->forw)
		p = p2;
	for (; p != first.forw; p = p->back) {
	    if (p->op != MOV || p->misc == 0)
	        continue;
	    dualop(p);
	    /* mov d0,dx */
	    if (isreg(regs[RT1]) != 0 || (r = isreg(regs[RT2])) < 3 || r > 7)
		continue;
	    if ((p2 = p->forw) == 0) continue;
	    /* check for non sequential switch statement */
	    if (p2->op==JBR && p2->labno) {
		for (p3 = p2->forw; p3; p3 = p3->forw) {
			if (p3->op==LABEL && p3->labno==p2->labno) {
			    if ((p4 = p3->forw) != 0)
				if (freed0(p4) == 0)
					goto next;
			    break;
			}
		}
	    }
	    /* verification loop */
	    for (p2 = p->back; p2 != first.forw; p2 = p2->back) {
		switch(p2->op) {
		case MOV: case MOVEQ:
			dualop(p2);
			/* mov[q] xxx,d[3-7] */
			rt = isreg(regs[RT2]);
			if ((rt = isreg(regs[RT2])) >= 1 &&
			     rt <= 7 && rt != r)
				break;
			/* mov[q] xxx,d0 */
			if (rt == 0 &&
			    (regs[RT1][0] == '#' || regs[RT1][0] == 'a' ||
			     regs[RT1][0] == '_' ||
			     (regs[RT1][0] == '.' && regs[RT1][1]=='L') ||
			     (regs[RT1][0] >= '0' && regs[RT1][0] <= '9') ||
			     ((rt = isreg(regs[RT1])) >= 3 && rt <= 7 &&
			      rt != r))) {
			   /* forward replacement loop */
			   if ((p3 = p->forw) != 0) {
			  again:
			    switch (p3->op) {
				case MOV:
				  /* movl d0,xxx */
				  dualop(p3);
				  if (isreg(regs[RT1]) == 0) {
				    (void) sprintf(lregs, "d%d,%s", r,
					    regs[RT2]);
				    newcode(p3, lregs);
				    if ((p3 = p3->forw) != 0)
					goto again;
				  }
				break;
				case CMP:
				  /* cmp d0,xxx or cmp xxx,d0 */
				  dualop(p3);
				  if (isreg(regs[RT1]) == 0) {
				    (void) sprintf(lregs, "d%d,%s", r,
					    regs[RT2]);
				    newcode(p3, lregs);
				  } else
				  if (isreg(regs[RT2]) == 0) {
				    (void) sprintf(lregs, "%s,d%d", regs[RT1],
					    r);
				    newcode(p3, lregs);
				  }
				break;
			    }
			    }
			    for (p2=p->back; p2 != first.forw; p2 = p2->back) {
				switch(p2->op) {
				case MOV: case MOVEQ:
				    dualop(p2);
				    if (isreg(regs[RT2]) != 0)
					break;
				    /* mov xxx,d0 */
				    (void) sprintf(lregs, "%s,d%d", regs[RT1],
					    r);
				    newcode(p2, lregs);
				    release(p);
				    p = p2;
				    goto next;
				case ADD: case SUB:
				case AND: case OR:
				case ASR: case ASL:
				case LSR: case LSL:
				case ADDQ: case SUBQ:
					dualop(p2);
					if (isreg(regs[RT2]) != 0)
						break;
					(void) sprintf(lregs, "%s,d%d",
						regs[RT1], r);
					newcode(p2, lregs);
					break;
				case EXT:
				case NEG:
				case NOT:
					singop(p2);
					if (isreg(regs[RT1]) != 0)
						break;
					(void) sprintf(lregs, "d%d", r);
					newcode(p2, lregs);
					break;
				default:
					fprintf(stderr,
						"c2:dregopt error: op=%d\n",
						p2->op);
					goto next;
				}
			    }
			}
			goto next;
		case ADD: case SUB:
		case AND: case OR:
		case ADDQ: case SUBQ:
			dualop(p2);
			if ((regs[RT1][0] == '_' || regs[RT1][0] == '#' ||
			    (regs[RT1][0] == '.' && regs[RT1][1] == 'L') ||
			    ((rt = isreg(regs[RT1])) >= 3 && rt <= 7 &&
			      rt != r)) &&
			    (rt = isreg(regs[RT2])) >= 0 && rt != r)
				break;
			goto next;
		case ASR: case ASL:
		case LSR: case LSL:
			dualop(p2);
			if ((regs[RT1][0] == '#' || isreg(regs[RT1]) == 1) &&
			    (rt = isreg(regs[RT2])) >= 0 && rt <= 7 &&
			     rt != r)
				break;
			goto next;
		case EXT:
		case NEG:
		case NOT:
			singop(p2);
			if ((rt = isreg(regs[RT1])) >= 0 && rt <= 7 &&
			     rt != r)
				break;
			goto next;
		default:
			goto next;
		}
	    }
	next:
	    continue;
	}
#endif DREGOPT
}

#define LEAOPT

/*
 * replace inefficient lea instructions
 */
lea()
{
#ifdef LEAOPT
	register struct node *p, *p2;
	register int r;
	char lregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if (p->op == LEA && p2->op == MOV && p2->subop == LONG &&
		    p2->misc) {
			dualop(p);
			if (isareg(regs[RT2]) == 0) {
			    dualop(p2);
			    if (isareg(regs[RT1]) == 0 &&
			       (r = isareg(regs[RT2])) >= 2 && r <= 5) {
				dualop(p);
				(void) sprintf(lregs, "%s,a%d", regs[RT1], r);
				newcode(p, lregs);
				fixareg(p2, r);
				release(p2);
				p2 = p;
			    }
			}
		}
	}
#endif LEAOPT
}

/*
 * compress sequential move instructions to succeeding addresses
 */
seqmov()
{
	register struct node *p, *p2;
	struct idecode id1, id2, id3, id4;

	if (Kflg || (p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if ( p->op == MOV &&  p->subop == WORD &&
		    p2->op == MOV && p2->subop == WORD) {
			dualop(p);
			opdecode(&id1, regs[RT1]);
			opdecode(&id2, regs[RT2]);
			if (id1.i_type == ARIDA && id2.i_type == ARIDA) {
			    dualop(p2);
			    opdecode(&id3, regs[RT1]);
			    opdecode(&id4, regs[RT2]);
			    if (id3.i_type == ARIDA && id4.i_type == ARIDA &&
				id1.i_type == id3.i_type &&
				id2.i_type == id4.i_type &&
				id1.i_reg == id3.i_reg &&
				id2.i_reg == id4.i_reg &&
				id1.i_offs + 2 == id3.i_offs &&
				id2.i_offs + 2 == id4.i_offs) {
					p->subop = LONG;
					release(p2);
					p2 = p;
					continue;
			    }
			}
		}
	}
}

/*
 * decode an instruction field
 */
opdecode(ip, str)
register struct idecode *ip;
char *str;
{
	register char *cp;
	register int sign;
	long n;

	cp = str;
	ip->i_type = -1;
	if (*cp++ == 'a' && *cp >= '0' && *cp++ <= '7' && *cp++ == '@' &&
	    *cp == 0) {
		ip->i_type = ARIDA;
		ip->i_reg = *(str+1) - '0' + 8;;
		ip->i_offs = 0;
	} else {
		cp = str;
		if (*cp++ == 'a' && *cp >= '0' && *cp++ <= '7' &&
		    *cp++ == '@' && *cp++ == '(') {
			n = sign = 0;
			if (*cp == '-') {
				sign = 1;
				cp++;
			}
			while (*cp && *cp != ')')
				n = n * 10 + *cp++ - '0';
			if (*cp++ == ')' && *cp == 0) {
				ip->i_type = ARIDA;
				ip->i_reg = *(str+1) - '0' + 8;;
				ip->i_offs = (sign == 0) ? n : -n;
			}
		}
	}
}
