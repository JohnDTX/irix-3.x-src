#include "opt.h"

/*
 * yet more optimizations
 */

/*
 * jsrmul - remove unnecessary calls to lmul
 */
jsrmul()
{
#ifdef JSRMULOPT
	register struct node *p, *p2, *p3;
	struct node *p4, *p5, *p6, *p7;
	int c;
	long n;
	char lregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
		if (p->op!=MOV || p->subop!=LONG) continue;
		if (p2->op!=MOV || p2->subop!=BYTE) continue;
		if ((p3 = p2->forw) == 0) break;
		if (p3->op!=EXT || p3->subop!=WORD) continue;
		if ((p4 = p3->forw) == 0) break;
		if (p4->op!=EXT || p4->subop!=LONG) continue;
		if ((p5 = p4->forw) == 0) break;
		if (p5->op!=MOV || p5->subop!=LONG) continue;
		if ((p6 = p5->forw) == 0) break;
		if (p6->op!=JSR) continue;
		if ((p7 = p6->forw) == 0) break;
		if (p7->op!=ADDQ || p7->subop!=LONG) continue;
		/* movl #xxx,sp@- */
		dualop(p);
		if (numeric(regs[RT1]) == 0 ||
		   (n = numcvt(&regs[RT1][1])) > 0xFFFFL ||
		   n < 0xFFFF0000L || strcmp(regs[RT2], "sp@-"))
			continue;
		/* movb X,D0 */
		dualop(p2);
		if (isreg(regs[RT2]) != 0) continue;
		/* extw D0 */
		singop(p3);
		if (isreg(regs[RT1]) != 0) continue;
		/* extl D0 */
		singop(p4);
		if (isreg(regs[RT1]) != 0) continue;
		/* movl D0,sp@- */
		dualop(p5);
		if (isreg(regs[RT1]) != 0 ||
		    strcmp(regs[RT2], "sp@-")) continue;
		/* jsr [lu]mul */
		singop(p6);
		if ((c=regs[RT1][0]) != 'l' && regs[RT1][0] != 'u')
			continue;
		if (strcmp(&regs[RT1][1], "mul")) continue;
		/* addql #8,sp */
		dualop(p7);
		if (numeric(regs[RT1]) == 0 || numcvt(&regs[RT1][1]) != 8)
			continue;
		release(p);
		release(p4);
		release(p5);
		release(p7);
		p6->op = (c == 'l') ? MULS : MULU;
		p6->subop = 0;
		(void) sprintf(lregs, "#0x%lx,d0", n);
		newcode(p6, lregs);
	}
	for (p = first.forw; p2 = p->forw; p = p2) {
		if (p->op!=MOV || p->subop!=LONG) continue;
		if (p2->op!=MOV || p2->subop!=LONG) continue;
		if ((p3 = p2->forw) == 0) break;
		if (p3->op!=JSR) continue;
		if ((p4 = p3->forw) == 0) break;
		if (p4->op!=ADDQ || p4->subop!=LONG) continue;
		/* movl #xxx,sp@- */
		dualop(p);
		if (numeric(regs[RT1]) == 0 ||
		   (n = numcvt(&regs[RT1][1])) > 0xFFFFL ||
		   n < 0xFFFF0000L || strcmp(regs[RT2], "sp@-"))
			continue;
		/* movl X,sp@- */
		dualop(p2);
		if (strcmp(regs[RT2], "sp@-")) continue;
		/* jsr [lu]mul */
		singop(p3);
		if ((c=regs[RT1][0]) != 'l' && regs[RT1][0] != 'u')
			continue;
		if (strcmp(&regs[RT1][1], "mul")) continue;
		/* addql #8,sp */
		dualop(p4);
		if (numeric(regs[RT1]) == 0 || numcvt(&regs[RT1][1]) != 8)
			continue;
		newcode(p3, (c == 'l') ? "smul" : "usmul");
	}
#endif
}

#define BTSTOPT

/*
 * insert btsts if possible
 */
btst()
{
#ifdef BTSTOPT
	register struct node *p, *p2, *p3, *p4;
	struct node *p5;
	int i, r;
	char lregs[MAXSIZE];

	if ((p = first.forw) == 0)
		return;
	for (; p2 = p->forw; p = p2) {
	    if ((p3 = p2->forw) == 0) break;
	    /* test a long Ax@ or _label */
	    if (Kflg == 0 &&
		p->op==MOV &&  p->subop==LONG &&
		p2->op==AND && p2->subop==LONG &&
		p3->op==CBR && (p3->subop==JEQ||p3->subop==JNE) &&
		(p3->misc || freed0(p3))) {
		    /* movl Ax@,D0 or movl _label,D0 */
		    dualop(p);
		    if (((regs[RT1][0]=='a' && regs[RT1][2]=='@') ||
		      regs[RT1][0]=='_' ||
		      (regs[RT1][0]=='.' && regs[RT1][1]=='L') ||
		      (regs[RT1][0]>='0' && regs[RT1][0]<='9')) &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p2);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1) {
			    dualop(p);
			    if ((i>=0 && i<=7 && btstpx(regs[RT1], 3)) ||
			       (i>=8 && i<=15 && btstpx(regs[RT1], 2)) ||
			       (i>=16 && i<=23 && btstpx(regs[RT1], 1)) ||
			       (i>=24 && i<=31 && btstpx(regs[RT1], 0))) {
				    (void) sprintf(lregs, "#%d,%s", i&7,
					    regs[RT1]);
				    newcode(p, lregs);
				    p->op = BTST;
				    p->subop = 0;
				    release(p2);
				    p2 = p3;
				    continue;
			    }
			}
		    }
	    }

	    /*
	     * Same as above, except allow a TSTL D0 between the AND and
	     * the BRAnch
	     */
	    if (Kflg == 0 &&
		p->op==MOV &&  p->subop==LONG &&		/* movl */
		p2->op==AND && p2->subop==LONG &&		/* andl */
		p3->op==TST && p3->subop==LONG &&		/* tstl */
		(p4 = p3->forw) != 0 &&
		p4->op==CBR && (p4->subop==JEQ||p4->subop==JNE) && /* bra */
		(p4->misc || freed0(p4))) {
		    /* movl Ax@,D0 or movl _label,D0 */
		    dualop(p);
		    if (((regs[RT1][0]=='a' && regs[RT1][2]=='@') ||
		      regs[RT1][0]=='_' ||
		      (regs[RT1][0]=='.' && regs[RT1][1]=='L') ||
		      (regs[RT1][0]>='0' && regs[RT1][0]<='9')) &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p2);
			if (isreg(regs[RT2]) == 0 &&
				 (i = btst32(regs[RT1])) != -1) {
			    /* tstl d0 */
			    singop(p3);
			    if (isreg(regs[RT1])==0) {
				dualop(p);
				if ((i>=0 && i<=7 && btstpx(regs[RT1], 3)) ||
				   (i>=8 && i<=15 && btstpx(regs[RT1], 2)) ||
				   (i>=16 && i<=23 && btstpx(regs[RT1], 1)) ||
				   (i>=24 && i<=31 && btstpx(regs[RT1], 0))) {
					(void) sprintf(lregs, "#%d,%s", i&7,
						regs[RT1]);
					newcode(p, lregs);
					p->op = BTST;
					p->subop = 0;
					release(p2);
					release(p3);
					p2 = p4;
					continue;
				}
			    }
			}
		    }
	    }

	    /* test a word Ax@ or _label */
	    if (Kflg == 0 &&
	        p->op==MOV &&  p->subop==WORD &&
		p2->op==EXT && p2->subop==LONG &&
		p3->op==AND && p3->subop==LONG &&
		(p4 = p3->forw) != 0 &&
		p4->op==CBR && (p4->subop==JEQ||p4->subop==JNE) &&
		(p4->misc || freed0(p4))) {
		    /* movw Ax@,D0 or movw _label,D0 */
		    dualop(p);
		    if (((regs[RT1][0]=='a' && regs[RT1][2]=='@') ||
		      regs[RT1][0]=='_' ||
		      (regs[RT1][0]=='.' && regs[RT1][1]=='L') ||
		      (regs[RT1][0]>='0' && regs[RT1][0]<='9')) &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p3);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1) {
			    dualop(p);
			    if ((i>=0 && i<=7 && btstpx(regs[RT1], 1)) ||
			       (i>=8 && i<=15 && btstpx(regs[RT1], 0))) {
				    (void) sprintf(lregs, "#%d,%s", i&7,
					    regs[RT1]);
				    newcode(p, lregs);
				    p->op = BTST;
				    p->subop = 0;
				    release(p2);
				    release(p3);
				    p2 = p4;
				    continue;
			    }
			}
		    }
	    }
	    /* test a word Ax@ or _label */
	    if (Kflg == 0 &&
	        p->op==MOV &&  p->subop==WORD &&
		p2->op==AND && p2->subop==LONG &&
		p3->op==CBR && (p3->subop==JEQ||p3->subop==JNE) &&
		(p3->misc || freed0(p3))) {
		    /* movw Ax@,D0 or movw _label,D0 */
		    dualop(p);
		    if (((regs[RT1][0]=='a' && regs[RT1][2]=='@') ||
		      regs[RT1][0]=='_' ||
		      (regs[RT1][0]=='.' && regs[RT1][1]=='L') ||
		      (regs[RT1][0]>='0' && regs[RT1][0]<='9')) &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p2);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1) {
			    dualop(p);
			    if ((i>=0 && i<=7 && btstpx(regs[RT1], 1)) ||
			       (i>=8 && i<=15 && btstpx(regs[RT1], 0))) {
				    (void) sprintf(lregs, "#%d,%s", i&7,
					    regs[RT1]);
				    newcode(p, lregs);
				    p->op = BTST;
				    p->subop = 0;
				    release(p2);
				    p2 = p3;
				    continue;
			    }
			}
		    }
	    }
	    /* test a signed byte Ax@ or _label */
	    if ( p->op==MOV &&  p->subop==BYTE &&
		p2->op==EXT && p2->subop==WORD &&
		p3->op==EXT && p3->subop==LONG &&
		(p4 = p3->forw) != 0 &&
		p4->op==AND && p4->subop==LONG &&
		(p5 = p4->forw) != 0 &&
		p5->op==CBR && (p5->subop==JEQ||p5->subop==JNE) &&
		(p5->misc || freed0(p5))) {
		    /* movb Ax@,D0 or movb _label,D0 */
		    dualop(p);
		    if (((regs[RT1][0]=='a' && regs[RT1][2]=='@') ||
		      regs[RT1][0]=='_' ||
		      (regs[RT1][0]=='.' && regs[RT1][1]=='L') ||
		      (regs[RT1][0]>='0' && regs[RT1][0]<='9')) &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p4);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1) {
			    dualop(p);
			    if (i>=0 && i<=7 && btstpx(regs[RT1], 0)) {
				    (void) sprintf(lregs, "#%d,%s", i,
					    regs[RT1]);
				    newcode(p, lregs);
				    p->op = BTST;
				    p->subop = 0;
				    release(p2);
				    release(p3);
				    release(p4);
				    p2 = p5;
				    continue;
			    }
			}
		    }
	    }
	    /* test an unsigned byte Ax@ or _label */
	    if ( p->op==MOV &&  p->subop==BYTE &&
		p2->op==AND && p2->subop==LONG &&
		p3->op==CBR && (p3->subop==JEQ||p3->subop==JNE) &&
		(p3->misc || freed0(p3))) {
		    /* movb Ax@,D0 or movb _label,D0 */
		    dualop(p);
		    if (((regs[RT1][0]=='a' && regs[RT1][2]=='@') ||
		      regs[RT1][0]=='_' ||
		      (regs[RT1][0]=='.' && regs[RT1][1]=='L') ||
		      (regs[RT1][0]>='0' && regs[RT1][0]<='9')) &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p2);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1) {
			    dualop(p);
			    if (i>=0 && i<=7 && btstpx(regs[RT1], 0)) {
				    (void) sprintf(lregs, "#%d,%s", i,
					    regs[RT1]);
				    newcode(p, lregs);
				    p->op = BTST;
				    p->subop = 0;
				    release(p2);
				    p2 = p3;
				    continue;
			    }
			}
		    }
	    }
	    /* test a long D register */
	    if ( p->op==MOV &&  p->subop==LONG &&
		p2->op==AND && p2->subop==LONG &&
		p3->op==CBR && (p3->subop==JEQ||p3->subop==JNE) &&
		(p3->misc || freed0(p3))) {
		    /* movl Dx,D0 */
		    dualop(p);
		    if ((r = isreg(regs[RT1])) >= 3 && r <= 7 &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p2);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1) {
			    (void) sprintf(lregs, "#%d,d%d", i, r);
			    newcode(p, lregs);
			    p->op = BTST;
			    p->subop = 0;
			    release(p2);
			    p2 = p3;
			    continue;
			}
		    }
	    }
	    /* test a word D register */
	    if ( p->op==MOV &&  p->subop==WORD &&
		p2->op==EXT && p2->subop==LONG &&
		p3->op==AND && p3->subop==LONG &&
		(p4 = p3->forw) != 0 &&
		p4->op==CBR && (p4->subop==JEQ||p4->subop==JNE) &&
		(p4->misc || freed0(p4))) {
		    /* movl Dx,D0 */
		    dualop(p);
		    if ((r = isreg(regs[RT1])) >= 3 && r <= 7 &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p3);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1 && i <= 15) {
			    (void) sprintf(lregs, "#%d,d%d", i, r);
			    newcode(p, lregs);
			    p->op = BTST;
			    p->subop = 0;
			    release(p2);
			    release(p3);
			    p2 = p4;
			    continue;
			}
		    }
	    }
	    /* test a byte D register */
	    if ( p->op==MOV &&  p->subop==BYTE &&
		p2->op==EXT && p2->subop==WORD &&
		p3->op==EXT && p3->subop==LONG &&
		(p4 = p3->forw) != 0 &&
		p4->op==AND && p4->subop==LONG &&
		(p5 = p4->forw) != 0 &&
		p5->op==CBR && (p5->subop==JEQ||p5->subop==JNE) &&
		(p5->misc || freed0(p5))) {
		    /* movl Dx,D0 */
		    dualop(p);
		    if ((r = isreg(regs[RT1])) >= 3 && r <= 7 &&
		      isreg(regs[RT2])==0) {
			/* andl #1<<n,d0 */
			dualop(p4);
			if (isreg(regs[RT2]) == 0 &&
			  (i = btst32(regs[RT1])) != -1 && i <= 7) {
			    (void) sprintf(lregs, "#%d,d%d", i, r);
			    newcode(p, lregs);
			    p->op = BTST;
			    p->subop = 0;
			    release(p2);
			    release(p3);
			    release(p4);
			    p2 = p5;
			    continue;
			}
		    }
	    }
	}
}

/*
 * is the passed numeric string a btst candidate
 */
btst32(str)
char *str;
{
	register i;
	long l, m;

	if (numeric(str)) {
		m = 1;
		l = numcvt(str+1);
		for (i = 0; i < 32; i++) {
			if (l == m)
				break;
			m <<= 1;
		}
		if (l == m)
			return(i);
	}
	return(-1);
}

/*
 * assemble a btst byte offset destination
 */
btstpx(cp, amt)
register char *cp;
{
	if (*cp == 'a' && *(cp+2) == '@') {
		cp += 3;
		if (*cp == 0) {
			if (amt) {
				*cp++ = '('; *cp++ = amt+'0';
				*cp++ = ')'; *cp = 0;
			}
			return(1);
		}
		if (*cp == '(') {
			while (*cp && *cp != ')') cp++;
			if (*cp == 0) return(0);
			if (amt) {
				*cp++ = '+'; *cp++ = amt+'0';
				*cp++ = ')'; *cp = 0;
			}
			return(1);
		}
		return(0);
	}
	if (*cp == '_' || (*cp == '.' && *(cp+1) == 'L') ||
	   (*cp >= '0' && *cp <= '9')) {
		while (*cp) cp++;
		if (amt) {
			*cp++ = '+'; *cp++ = amt+'0';
			*cp = 0;
		}
		return(1);
	}
	return(0);
#endif
}

/*
 * return true if A0 is free
 */
freea0(p)
register struct node *p;
{
	register struct node *p2;

	for (;;) {
		if (p->misc) return(1);
		switch(p->op) {
		case ADD:
		case SUB:
		case AND:
		case OR:
		case CMP:
			dualop(p);
			if (regs[RT1][0]=='a' && regs[RT1][1]=='0') return(0);
			if (regs[RT2][0]=='a' && regs[RT2][1]=='0') return(0);
			break;
		case ADDQ:
		case SUBQ:
			dualop(p);
			if (regs[RT2][0]=='a' && regs[RT2][1]=='0') return(0);
			break;
		case EXT:
		case ASL:
		case ASR:
		case LSR:
		case LSL:
			break;
		case MOV:
			dualop(p);
			if (isareg(regs[RT1])==0) return(0);
			if (isareg(regs[RT2])==0) return(1);
			if (regs[RT1][0]=='a' && regs[RT1][1]=='0') return(0);
			if (regs[RT2][0]=='a' && regs[RT2][1]=='0') return(0);
			break;
		case MOVEM:
			return(1);
		case DLABEL:
		case LABEL:
			if (p->misc == 0) return(0);
			return(1);
		case MOVEQ:
			break;
		case CLR:
		case JSR:
			singop(p);
			if (regs[RT1][0]=='a' && regs[RT1][1]=='0') return(0);
			break;
		case CBR:
		case JBR:
			if (p->labno == 0)
				break;
			for (p2 = p->forw; p2; p2 = p2->forw) {
				if (p2->misc)
					break;
				if (p2->op==LABEL && p->labno==p2->labno)
					return(0);
			}
			break;
		case TST:
			singop(p);
			if (regs[RT1][0]=='a' && regs[RT1][1]=='0') return(0);
			break;
		default:
			return(0);
		}
		if ((p = p->forw) == 0) return(0);
	}
}

/*
 * return true if D0 is free
 */
freed0(p)
register struct node *p;
{
#ifdef OLDD
	register struct node *p2, *p3;

	/* at end already */
	if (p->misc) return(1);
	if ((p2 = p->forw) == 0) return(0);
	/* look for an immediate 'mov X,D0' */
	if (p2->op == MOVEQ ||
	   (p2->op == MOV &&
	   (p2->subop==LONG || p2->subop==WORD || p2->subop==BYTE))) {
		dualop(p2);
		if (isreg(regs[RT2]) == 0) 
			return(1);
	}
	if ((p2->op==LABEL || p2->op==DLABEL) && p2->misc)
		return(1);
	return(0);
#else
	register struct node *p2;

	for (;;) {
		if (p->misc) return(1);
		switch(p->op) {
		case ADD:
		case SUB:
		case AND:
		case OR:
		case CMP:
			dualop(p);
			if (regs[RT1][0]=='d' && regs[RT1][1]=='0') {
				/*
				 * d0 is only a source.  its not free.
				 */
				return(0);
			}
			if (regs[RT2][0]=='d' && regs[RT2][1]=='0') {
				/*
				 * d0 is both source and destination.
				 * its not free.
				 */
				return(0);
			}
			break;
		case ADDQ:
		case SUBQ:
			dualop(p);
			if (regs[RT2][0]=='d' && regs[RT2][1]=='0') {
				/*
				 * d0 is both source and destination.
				 * its not free.
				 */
				return(0);
			}
			break;
		case CLR:
			singop(p);
			if (regs[RT1][0]=='d' && regs[RT1][1]=='0' &&
			    regs[RT1][2] == 0) {
				/*
				 * d0 is only a destination.  its free.
				 */
				return(1);
			}
			break;
		case EXT:
			singop(p);
			if (regs[RT1][0]=='d' && regs[RT1][1]=='0') {
				/*
				 * d0 is both a source and destination.
				 * its not free.
				 */
				return(0);
			}
			break;
		case ASL:
		case ASR:
		case LSR:
		case LSL:
			dualop(p);
			if (regs[RT1][0]=='d' && regs[RT1][1]=='0') {
				/*
				 * d0 is a source.  its not free.
				 */
				return(0);
			}
			if (regs[RT2][0]=='d' && regs[RT2][1]=='0') {
				/*
				 * d0 = d0 << C; so d0 is not free,
				 * because it is both destination and
				 * source.
				 */
				return(0);
			}
			break;
		case MOV:
			dualop(p);
			if (isreg(regs[RT1])==0) {
				/*
				 * d0 is a source.  its not free.
				 */
				return(0);
			}
			if (isreg(regs[RT2])==0) {
				/*
				 * d0 is the destination.  its free.
				 */
				return(1);
			}
			break;
		case MOVEM:
			/*
			 * Define d0 being free here, because C doesn't
			 * save or restore d0 across procedure calls,
			 * which this instruction is used for.
			 */
			return(1);
		case DLABEL:
		case LABEL:
			if (p->misc == 0) {
				return(0);
			}
			return(1);
		case MOVEQ:
			dualop(p);
			if (isreg(regs[RT2])==0) {
				/*
				 * d0 is the destination.  its free.
				 */
				return(1);
			}
			break;
		case JSR:
			singop(p);
			/* check for illegal opcode - why??? */
			if (regs[RT1][0]=='d' && regs[RT1][1]=='0') {
				return(0);
			}
			/*
			 * Because the C compiler may pass values into
			 * procedure via d0, we can't assume that the
			 * register is free here.  SIGH.
			 */
			break;
		case CBR:
		case JBR:
			if (p->labno == 0)
				break;
			for (p2 = p->forw; p2; p2 = p2->forw) {
				if (p2->misc)
					break;
				if (p2->op==LABEL && p->labno==p2->labno) {
					if (p2 = p2->forw) {
						return(freed0(p2));
					}
					return(0);
				}
			}
			break;
		case TST:
			singop(p);
			if (regs[RT1][0]=='d' && regs[RT1][1]=='0') {
				/*
				 * d0 is the source.  its not free.
				 */
				return(0);
			}
			break;
		default:
			return(0);
		}
		if ((p = p->forw) == 0)
			return(0);
	}
#endif
}
