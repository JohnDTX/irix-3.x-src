#include "opt.h"

/*
 * do pre processing on the opcodes
 */
preopt()
{
	kernel();	/* optimize kernel stuff */
	andorsh();	/* compress multiple AND/OR/SHIFT combinations */
	btst();		/* insert btsts if possible */
	extwl();	/* remove unnecesary EXTW/EXTL instructions */
	jsrmul();	/* fix 'jsr lmul's */
	spfix();	/* remove unnecessary ADDL #XXX,sp code */
	spsetup();	/* remove unnecessary stack set up code */
	addlax();	/* movl Ax,[ad]x;addl #xxx,[ad]x;movl [ad]x,Ax */
			/* movl dn,d0, addl #x,d0, movl d0,a0, movl a0@,d0 */
			/* stack parameter set up optimization */
	cmpl0rm();	/* modify cmpl #0,ax ; beq lll */
	opts1();	/* optimize single instructions */
}

#define ADDLAX

/*
 * reduce Ax moves to a Ax and Dx registers to just increment Ax
 * movl dn,d0, addl #x,d0, movl d0,a0, mov a0@,d0
 * movw X,d0; extl d0; movl d0,sp@- to movw X,a0; pea a0@
 */
addlax()
{
#ifdef ADDLAX
	register struct node *p, *p2, *p3, *p4;
	register int r;
	int rt, rr;
	char lregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	/* movl dn,d0, addl #x,d0, movl d0,a0, mov a0@,d0 */
	for (; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if (p->op==MOV && p->subop==LONG && p2->op==ADD &&
		    p2->subop==LONG && p3->op==MOV && p3->subop==LONG &&
		    (p4=p3->forw) && p4->op==MOV) {
			/* movl Dx,D0 */
			dualop(p);
			if ((r = isreg(regs[RT1])) < 2 || r > 7) continue;
			if (isreg(regs[RT2]) != 0) continue;
			/* addl #xxx,d0 */
			dualop(p2);
			if (regs[RT1][0] != '#') continue;
			if (isreg(regs[RT2]) != 0) continue;
			/* movl D0,A0 */
			dualop(p3);
			if (isreg(regs[RT1]) != 0) continue;
			if (isareg(regs[RT2]) != 0) continue;
			/* movl A0@,D0 */
			dualop(p4);
			if (regs[RT1][0]!='a' || regs[RT1][1]!='0' ||
			    regs[RT1][2]!='@') continue;
			if (isreg(regs[RT2]) != 0) continue;
			dualop(p);
			(void) sprintf(lregs, "%s,a0", regs[RT1]);
			newcode(p, lregs);
			dualop(p2);
			(void) sprintf(lregs, "%s,a0", regs[RT1]);
			newcode(p2, lregs);
			release(p3);
			continue;
		}
	}
	for (p = first.forw; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if (p->op==MOV && p->subop==LONG && p2->op==ADD &&
		    p2->subop==LONG && p3->op==MOV && p3->subop==LONG &&
		    p3->misc) {
			/* movl Ax,D0 */
			dualop(p);
			if ((r = isareg(regs[RT1])) < 0 || r > 7) continue;
			if (isreg(regs[RT2]) != 0) continue;
			/* addl #xxx,d0 */
			dualop(p2);
			if (regs[RT1][0] != '#') continue;
			if (isreg(regs[RT2]) != 0) continue;
			/* movl D0,Ax */
			dualop(p3);
			if (isreg(regs[RT1]) != 0) continue;
			if (isareg(regs[RT2]) == r) {
				dualop(p2);
				(void) sprintf(lregs, "%s,a%d", regs[RT1], r);
				newcode(p2, lregs);
				fixareg(p3, r);
				release(p);
				release(p3);
				continue;
			}
		}
	}
	for (p = first.forw; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if (p->op == MOV && p->subop == LONG &&
		    (p2->op == ADD || p2->op == ADDQ) &&
		    p2->subop==LONG && p3->op==MOV && p3->subop==LONG &&
		    p3->misc) {
			/* movl xxx,D0 */
			dualop(p);
			r = isreg(regs[RT1]);
			if (! ((r >= 2 && r <= 16) || numeric(regs[RT1]) ||
			       regs[RT1][0] == '_' || (regs[RT1][0] == '.' &&
			       regs[RT1][1]=='L')) )
				       continue;
			if (isreg(regs[RT2]) != 0) continue;
			/* addl #xxx,d0 */
			dualop(p2);
			if (regs[RT1][0] != '#') continue;
			if (isreg(regs[RT2]) != 0) continue;
			/* movl D0,Ax */
			dualop(p3);
			if (isreg(regs[RT1]) == 0 &&
			    (r = isareg(regs[RT2])) >= 0 && r <= 5) {
				dualop(p);
				(void) sprintf(lregs, "%s,a%d", regs[RT1], r);
				newcode(p, lregs);
				dualop(p2);
				(void) sprintf(lregs, "%s,a%d", regs[RT1], r);
				newcode(p2, lregs);
				fixareg(p3, r);
				release(p3);
			}
		}
	}
	for (p = first.forw; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if (p->op == MOV && p->subop == LONG &&
		    (p2->op == ADD || p2->op == ADDQ) &&
		    p2->subop==LONG && p3->op==MOV && p3->subop==LONG &&
		    p3->misc) {
			/* movl Ax,A0 */
			dualop(p);
			if ((r = isareg(regs[RT1])) == -1) continue;
			if (isareg(regs[RT2]) != 0) continue;
			/* addl #xxx,Ax */
			dualop(p2);
			if (regs[RT1][0] != '#') continue;
			if ((rt = isareg(regs[RT2])) == -1) continue;
			/* movl A0,Ax */
			dualop(p3);
			if (isareg(regs[RT1]) != 0) continue;
			if ((rr = isareg(regs[RT2])) == -1) continue;
			dualop(p);
			if (r == rt && r != rr) {
			    /* movl a5,a0 ; add #1,a5 ; movl a0,a4 */
			    dualop(p);
			    (void) sprintf(lregs, "%s,a%d", regs[RT1], rr);
			    newcode(p, lregs);
			    fixareg(p3, rr);
			    release(p3);
			}
		}
	}
	/* 'movw X,d0; extl d0; movl d0,sp@-' to 'movw X,a0; pea a0@' */
	for (p = first.forw; p2 = p->forw; p = p2) {
		if ( p->op==MOV && p->subop==WORD && p->lineno &&
		    p2->op==EXT && p2->subop==LONG &&
		    ((p3 = p2->forw) != 0) &&
		    p3->op==MOV && p3->subop==LONG) {
			dualop(p);
			if (isreg(regs[RT2]) == 0) {
			    singop(p2);
			    if (isreg(regs[RT1]) == 0) {
				dualop(p3);
				if (isreg(regs[RT1]) == 0 &&
				    strcmp(regs[RT2], "sp@-") == 0) {
					dualop(p);
					(void) sprintf(lregs, "%s,a0",
						regs[RT1]);
					newcode(p, lregs);
					release(p2);
					p2 = p3;
					p3->op = PEA;
					p3->subop = 0;
					newcode(p3, "a0@");
					continue;
				}
			    }
			}
		}
	}
	/* 'movw X,d0; extl d0; movl d0,sp@-' to 'movw X,a0; pea a0@' */
	for (p = first.forw; p2 = p->forw; p = p2) {
		if ( p->op==MOV && p->subop==WORD &&
		    p2->op==EXT && p2->subop==LONG &&
		    ((p3 = p2->forw) != 0) &&
		    p3->op==MOV && p3->subop==LONG &&
		    ((p4 = p3->forw) != 0) && freea0(p4)) {
			dualop(p);
			if (isreg(regs[RT2]) == 0) {
			    singop(p2);
			    if (isreg(regs[RT1]) == 0) {
				dualop(p3);
				if (isreg(regs[RT1]) == 0 &&
				    strcmp(regs[RT2], "sp@-") == 0) {
					dualop(p);
					(void) sprintf(lregs, "%s,a0",
						regs[RT1]);
					newcode(p, lregs);
					release(p2);
					p2 = p3;
					p3->op = PEA;
					p3->subop = 0;
					newcode(p3, "a0@");
					continue;
				}
			    }
			}
		}
	}
}

/*
 * fix succeeding "a" register instructions
 */
fixareg(p, r)
register struct node *p;
{
	char lregs[MAXSIZE];

	if ((p = p->forw) == 0) return;
	switch (p->op) {
	case MOV:
	    dualop(p);
	    if (isreg(regs[RT1]) == 0 || isareg(regs[RT1]) == 0) {
		(void) sprintf(lregs, "a%d,%s", r, regs[RT2]);
		if (p->subop == BYTE) {
		    fprintf(stderr, "c2: Ax byte optimization error on '%s'\n",
			lregs);
		} else
		    newcode(p, lregs);
	    }
	    break;
	case PEA:
	    singop(p);
	    if (regs[RT1][0] == 'a' && regs[RT1][1] == '0')
		p->code[1] = r+'0';
	    break;
	}
#endif ADDLAX
}

/*
 * remove redundant and and/or/shift instructions
 */
andorsh()
{
	register struct node *p, *p2;
	register r;
	long n1, n2;
	char lregs1[MAXSIZE], lregs2[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if (((p->op == ASR && p2->op == ASR) ||
		     (p->op == ASL && p2->op == ASL) ||
		     (p->op == LSR && p2->op == LSR) ||
		     (p->op == LSL && p2->op == LSL)) &&
		     p->subop == LONG && p2->subop == LONG) {
			dualop(p);
			if (numeric(regs[RT1]) &&
			   (r=isreg(regs[RT2])) >= 0 && r <= 7) {
				n1 = numcvt(&regs[RT1][1]);
				dualop(p2);
				if (numeric(regs[RT1]) &&
				   (isreg(regs[RT2])) == r) {
					n1 += numcvt(&regs[RT1][1]);
					if (n1 <= 8) {
						release(p);
						(void) sprintf(lregs1, "#%d,%s",
							n1, regs[RT2]);
						newcode(p2, lregs1);
						continue;
					}
				}
			}
		}
		if ((p->op == AND && p2->op == AND) ||
		    (p->op == OR  && p2->op == OR)) {
			dualop(p);
			strcpy(lregs1, regs[RT1]);
			strcpy(lregs2, regs[RT2]);
			dualop(p2);
			if (strcmp(lregs2, regs[RT2])) continue;
			if ((r = isreg(lregs2)) < 0 || r != isreg(regs[RT2]))
				continue;
			if (p->subop != p2->subop) continue;
			if (numeric(lregs1)==0 || numeric(regs[RT1])==0)
				continue;
			n1 = numcvt(lregs1+1);
			n2 = numcvt(&regs[RT1][1]);
			(void) sprintf(lregs1, "#0x%lx,%s",
				(p->op==AND) ? (n1&n2) : (n1|n2), lregs2);
			newcode(p, lregs1);
			release(p2);
			p2 = p;
		}
	}
}

#define CMPL0RM

/*
 * remove unnecessary cmpl #0,Ax ; beq label
 * and replace with movl Ax,D0
 */
cmpl0rm()
{
	register struct node *p, *p2, *p3;
	register int r;
	char lregs[MAXSIZE];

#ifdef CMPL0RM
	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
	    if ( p->op==MOV &&  p->subop==LONG &&
		p2->op==CMP && p2->subop==LONG &&
		(p3 = p2->forw) != 0 && p3->misc &&
		p3->op==CBR && jmpcond(p3)) {
		    /* movl xxx,Ax */
		    dualop(p);
		    if (isareg(regs[RT2]) != -1) {
			/* cmpl #0,Ax */
			dualop(p2);
			if (numeric(regs[RT1]) &&
			    numcvt(&regs[RT1][1]) == 0 &&
			    (r = isareg(regs[RT2])) != -1) {
				p2->op = MOV;
				(void) sprintf(lregs, "a%d,d0", r);
				newcode(p2, lregs);
				p2 = p3->forw;
				continue;
			}
		  }
	    }
	    if ( p->op==CMP && p->subop==LONG &&
		p2->op==CBR && jmpcond(p2) &&
		p2->misc) {
		    /* cmpl #0,Ax */
		    dualop(p);
		    if (numeric(regs[RT1]) &&
		        numcvt(&regs[RT1][1]) == 0 &&
		        (r = isareg(regs[RT2])) != -1) {
			    p->op = MOV;
			    (void) sprintf(lregs, "a%d,d0", r);
			    newcode(p, lregs);
			    continue;
		    }
	    }
	}
#endif CMPL0RM
}

/*
 * remove unnecessary extw and extl instructions
 */
extwl()
{
	register struct node *p, *p2, *p3, *p4;
	char lregs1[MAXSIZE], lregs2[MAXSIZE];
	long n1;
	int r;

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if (p->op==MOV && p->subop==BYTE && p2->op==EXT &&
		    p2->subop==WORD && p3->op==EXT && p3->subop==LONG &&
		    (p4 = p3->forw)!=0 && p4->op==AND && p4->subop==LONG) {
			dualop(p);
			strcpy(lregs1, regs[RT1]);
			strcpy(lregs2, regs[RT2]);
			singop(p2);
			if (strcmp(lregs2, regs[RT1])) goto next1;
			singop(p3);
			if (strcmp(lregs2, regs[RT1])) goto next1;
			dualop(p4);
			if (strcmp(lregs2, regs[RT2])) goto next1;
			if ((r = isreg(lregs2)) < 0 || r > 7 ||
			    r != isreg(regs[RT2]))
				goto next1;
			if (numeric(regs[RT1])==0) goto next1;
			n1 = numcvt(&regs[RT1][1]);
			if ((n1 & 0xFFL) == n1) {
				release(p2);
				release(p3);
				p2 = p4;
			} else if ((n1 & 0xFFFFL) == n1) {
				release(p3);
				p2 = p4;
			}
			continue;
		}
	next1:
		if (p->op==MOV && p->subop==WORD && p2->op==EXT &&
		    p2->subop==LONG && p3->op==ASR && p3->subop==LONG &&
		    (p4 = p3->forw)!=0 && p4->op==AND && p4->subop==LONG) {
			dualop(p);
			strcpy(lregs1, regs[RT1]);
			strcpy(lregs2, regs[RT2]);
			singop(p2);
			if (strcmp(lregs2, regs[RT1])) continue;
			dualop(p3);
			if (numeric(regs[RT1])==0) continue;
			if (numcvt(&regs[RT1][1]) != 8) continue;
			if (strcmp(lregs2, regs[RT2])) continue;
			dualop(p4);
			if (strcmp(lregs2, regs[RT2])) continue;
			if ((r = isreg(lregs2)) < 0 || r > 7 ||
			    r != isreg(regs[RT2]))
				continue;
			if (numeric(regs[RT1])==0) continue;
			n1 = numcvt(&regs[RT1][1]);
			if (n1 != 0xFFL) continue;
			if (lregs1[0]=='_' || lregs1[0]=='0' ||
			   (lregs1[0]=='.' && lregs1[1]=='L')) {
				newcode(p2, p->code);
				p2->op = MOV;
				p2->subop = BYTE;
				newcode(p, regs[RT2]);
				p->op = CLR;
				p->subop = LONG;
				release(p3);
				release(p4);
			} else {
				release(p2);
				p2 = p4;
			}
			continue;
		}
	}
	for (p = first.forw; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if (p->op==MOV  && p->subop==WORD  &&
		    p2->op==EXT && p2->subop==LONG &&
		    p3->op==AND && p3->subop==LONG) {
			dualop(p);
			strcpy(lregs1, regs[RT1]);
			strcpy(lregs2, regs[RT2]);
			singop(p2);
			if (strcmp(lregs2, regs[RT1])) continue;
			dualop(p3);
			if (strcmp(lregs2, regs[RT2])) continue;
			if ((r = isreg(lregs2)) < 0 || r > 7 ||
			    r != isreg(regs[RT2]))
				continue;
			if (numeric(regs[RT1])==0) continue;
			n1 = numcvt(&regs[RT1][1]);
			if ((n1 & 0xFFFFL) == n1) {
				release(p2);
				p2 = p3;
			}
		}
	}
}

/*
 * optimize single instructions
 */
opts1()
{
	register struct node *p;
	/* register char *cp; */
	/* char lregs1[MAXSIZE]; */

	for (p = first.forw; p != 0; p = p->forw) {
		/* change 'mov #0,X' to 'clr X' */
		if (p->op == MOV) {
			dualop(p);
			if ((Kflg == 0 || strcmp(regs[RT2],"sp@-") == 0 ||
			    strcmp(regs[RT2],"sp@") == 0) &&
			    numeric(regs[RT1]) && numcvt(&regs[RT1][1])==0) {
				/* ignore moves to A register */
				if (isareg(regs[RT2]) == -1) {
					newcode(p, regs[RT2]);
					p->op = CLR;
					continue;
				}
			}
		    continue;
		}
		/* This is not really faster
		if (p->op == ADDQ && p->subop == LONG) {
			dualop(p);
			if (strcmp(regs[RT2], "sp")==0 && numeric(regs[RT1])) {
				if (numcvt(&regs[RT1][1]) == 4) {
					newcode(p, "sp@+");
					p->op = TST;
				}
			}
			continue;
		} */
		/* not really faster
		if (p->op == PEA) {
			singop(p);
			cp = regs[RT1];
			if (*cp++ == 'a' && *cp >= '0' && *cp++ <= '7' &&
			    *cp++ == '@' && *cp == 0) {
				(void) sprintf(lregs1, "a%c,sp@-",
					regs[RT1][1]);
				newcode(p, lregs1);
				p->op = MOV;
				p->subop = LONG;
			}
			continue;
		} */
	}
}

/*
 * fix stack referencing instructions
 * Note:  The spfix code is incompatable with adbs stack trace mechanism
 */
spfix()
{
	register struct node *p, *q;
	char *cp, lregs1[MAXSIZE];
	long n1;

	if (Sflg == 0)
		return;
	p = first.forw;
	while (p) {
	    if ((p->op==ADDQ || p->op==ADD) && p->subop==LONG) {
		dualop(p);
		if (numeric(regs[RT1])==0 || (n1=numcvt(&regs[RT1][1]))<4 ||
		    strcmp(regs[RT2], "sp")) {
			p = p->forw;
			continue;
		}
		for (q = p->forw; q; q = q->forw) {
		    switch(q->op) {
		    case MOV:
			dualop(q);
			if (strcmp(regs[RT2], "sp@-")==0) {
				n1 -= 4;
				(void) sprintf(lregs1, "%s,sp@", regs[RT1]);
				newcode(q, lregs1);
				if (n1 == 0) {
					release(p);
				} else {
					(void) sprintf(lregs1, "#%ld,sp", n1);
					newcode(p, lregs1);
					if (n1 <= 8) p->op = ADDQ;
				}
				p = q->forw;
				goto next;
			}
			if (anysp(regs[RT1]) || anysp(regs[RT2])) {
				p = q;
				goto next;
			}
			break;
		    case PEA:
			singop(q);
			cp = regs[RT1];
			if (*cp++ == 'a' && *cp >= '0' && *cp++ <= '7' &&
			    *cp++ == '@' && *cp == 0) {
				n1 -= 4;
				(void) sprintf(lregs1, "a%c,sp@", regs[RT1][1]);
				newcode(q, lregs1);
				q->op = MOV;
				q->subop = LONG;
				if (n1 == 0) {
					release(p);
				} else {
					(void) sprintf(lregs1, "#%ld,sp", n1);
					newcode(p, lregs1);
					if (n1 <= 8) p->op = ADDQ;
				}
				p = q->forw;
				goto next;
			}
			p = q;
			goto next;
		    case ADDQ: case SUBQ:
		    case ADD:  case SUB:
		    case ASR:  case ASL:
		    case LSR:  case LSL:
		    case AND:  case OR:
			dualop(q);
			if (anysp(regs[RT1]) || anysp(regs[RT2])) {
				p = q;
				goto next;
			}
			break;
		    case CLR:
		    case EXT:
			singop(q);
			if (anysp(regs[RT1])) {
				p = q;
				goto next;
			}
			break;
		    default:
			p = q;
			goto next;
		    }
		}
		return;
	    }
	    p = p->forw;
	next:
	    continue;
	}
}

#define SPSETUP

/*
 * remove unnecessary stack set up code
 */
spsetup()
{
#ifdef SPSETUP
	register struct node *p, *p2, *p3, *p4;
	char lregs[MAXSIZE];
	int r;

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if ((p3 = p2->forw) == 0) break;
		if ((p4 = p3->forw) == 0) break;
		if (p->op==MOV && p->subop==LONG && p2->op==ADDQ &&
		    p2->subop==LONG && p3->op==MOV && p3->subop==LONG &&
		    p4->lineno) {
			dualop(p);
			/* movl d7,d0 */
			if ((r = isreg(regs[RT1])) >= 3 && r <= 7 && 
			    isreg(regs[RT2]) == 0) {
				dualop(p2);
				/* addql #n,d7 */
				if (isreg(regs[RT2]) == r) {
				    dualop(p3);
				    /* movl d0,sp@- */
				    if (isreg(regs[RT1]) == 0) {
					(void) sprintf(lregs, "d%d,sp@-", r);
					newcode(p, lregs);
					release(p3);
					continue;
				    }
				}
			}
		}
		if (p->op==MOV && p->subop==LONG && p2->op==ADDQ &&
		    p2->subop==LONG && p3->op==PEA &&
		    p4->lineno) {
			dualop(p);
			/* movl a5,a0 */
			if ((r = isareg(regs[RT1])) >= 2 && r <= 5 && 
			    isareg(regs[RT2]) == 0) {
				dualop(p2);
				/* addql #n,a5 */
				if (isareg(regs[RT2]) == r) {
				    singop(p3);
				    /* pea a0@ */
				    if (regs[RT1][0] == 'a' &&
					regs[RT1][1] == '0' &&
					regs[RT1][2] == '@' &&
					regs[RT1][3] == 0) {
					    (void) sprintf(lregs, "a%d@", r);
					    newcode(p, lregs);
					    p->op = PEA;
					    p->subop = 0;
					    release(p3);
					    continue;
				    }
				}
			}
		}
	}
#endif SPSETUP
}


/*
 * check for 'sp' appearing in a string
 */
anysp(sp)
register char *sp;
{
	while (*sp) {
		if (*sp++ == 's' && *sp == 'p')
			return(1);
	}
	return(0);
}

/*
 * insert new code for an instruction
 */
newcode(p, str)
register struct node *p;
char *str;
{
	if (p->code) cfree(p->code);
	p->code = (char *)copy(str);
}
