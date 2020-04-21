#include "opt.h"

/*
 * Kernel optimizations:
 *	- these optimizations are for deleting commonly used simple
 *	  subroutines, and replacing them with inline equivalents
 */

/*
 * Make a new node, with the given opcode, subcode, and instruction code
 * field.  Insert it after the given node, and return a pointer to the
 * new node.
 */
struct node *
makenode(p, op, subop, code)
	register struct node *p;
	int op, subop;
	char *code;
{
	register struct node *p2;

	p2 = (struct node *)getnode();
	p2->op = op;
	p2->subop = subop;
	p2->labno = 0;
	p2->code = (char *)copy(code);
	p2->ref = 0;
	p2->back = p;
	p2->forw = p->forw;
	p->forw = p2;
	if (p2->forw)
	    p2->forw->back = p2;
	return (p2);
}

/*
 * Generate an inline strncmp, using a0 and a1 for temporaries.
 */
struct node *
gen_strncmp(p, amount, s1, s2)
	register struct node *p;
	int amount;
	char *s1, *s2;
{
	static int newlabel = 1;
	char buf[MAXSIZE];
	char label[MAXSIZE];

	(void) sprintf(label, "%d$", newlabel++);
	/*
	 * Generate lea's into a0 and a1
	 */
	(void) sprintf(buf, "%s,a0", s1);
	p = makenode(p, LEA, 0, buf);
	(void) sprintf(buf, "%s,a1", s2);
	p = makenode(p, LEA, 0, buf);

	/*
	 * We want the strncmp to bomb out as soon as possible.  If the
	 * amount is not a multiple of four bytes, then generate a
	 * compare word.
	 */
	if (amount & 3) {
		p = makenode(p, CMPM, WORD, "a0@+,a1@+");
		amount -= 2;
		if (amount == 0)
			return (p);
		p = makenode(p, CBR, JNE, label);
	}
	if (amount & 3) {
		printf("||||HUH\n");
		abort();
	}
	while (amount) {
		p = makenode(p, CMPM, LONG, "a0@+,a1@+");
		amount -= 4;
		if (amount)
			p = makenode(p, CBR, JNE, label);
	}
	p = makenode(p, DLABEL, 0, label);
	return (p);
}

kernel()
{
    register struct node *p, *p2, *p3, *p4;
    struct node *p5, *p6, *p7;
    char buf[MAXSIZE];

    if ((p = first.forw) == 0)
	    return;
    if (skflg == 0)
	    return;
    for (; p2 = p->forw; p = p2) {

	/*
	 * look for "x = spl?();
	 */
	if (p->op == JSR) {
	    singop(p);
	    if ((regs[RT1][0] == '_') &&
		(regs[RT1][1] == 's') &&
		(regs[RT1][2] == 'p') &&
		(regs[RT1][3] == 'l') &&
		((regs[RT1][4] >= '0') && (regs[RT1][4] <= '7')) &&
		(regs[RT1][5] == 0) &&
		(p2->op == MOV) &&
		((p2->subop == WORD) || (p2->subop == LONG))) {
	        char level, destreg;

		level = regs[RT1][4];
		dualop(p2);
		if (isreg(regs[RT1]) == 0) {	/* mov[wl] d0, ... */
		    /*
		     * See if destination is another register.
		     */
		    destreg = isreg(regs[RT2]);
		    if ((destreg >= 0) && (destreg < 8)) {
			/*
			 * jsr _spl?		==> movw sr, d?
			 * mov[wl] d0,d?	==> movw #0x2?00, sr
			 */
			(void) sprintf(buf, "sr,d%d", destreg);
			newcode(p, buf);
			p->op = MOV;
			p->subop = WORD;
			(void) sprintf(buf, "#0x2%c00,sr", level);
			newcode(p2, buf);
			p2->op = MOV;
			p2->subop = WORD;
			continue;
		    } else
		    if ((regs[RT2][0]=='a' && regs[RT2][2]=='@') ||
			 (regs[RT2][0]=='_') ||
			 (regs[RT2][0]=='.' && regs[RT2][1]=='L') ||
			 (regs[RT2][0]>='0' && regs[RT2][0]<='9')) {
			/*
			 * jsr _spl?			==> movw sr, somewhere
			 * mov[wl] d0, somewhere	==> movw #0x2?00, sr
			 */
			if ((p2->subop == WORD) ||
			    ((p2->subop == LONG) && newaddr(regs[RT2], 2))) {
			    (void) sprintf(buf, "sr,%s", regs[RT2]);
			    newcode(p, buf);
			    p->op = MOV;
			    p->subop = WORD;
			    (void) sprintf(buf, "#0x2%c00,sr", level);
			    newcode(p2, buf);
			    p2->op = MOV;
			    p2->subop = WORD;
			    continue;
			}
		    }
		}
	    }
	}

	/*
	 * look for "splx(s)"
	 */
	if ((p->op == MOV) && ((p->subop == WORD) || (p->subop == LONG)) &&
	    (p2->op == JSR) && (p3 = p2->forw)) {
	    /* make sure 1st instruction is: "mov[wl] *, sp@-" */
	    dualop(p);
	    if (strcmp(regs[RT2], "sp@-") == 0) {
		/* make sure 2nd instruction is: "jsr _splx" */
		singop(p2);
		if ((strcmp(regs[RT1], "_splx") == 0) &&
		    ((p3->op == ADDQ) && (p3->subop == LONG))) {
		    /* make sure 3rd instruction is: "addql #4,sp" */
		    dualop(p3);
		    if ((strcmp(regs[RT1], "#4") == 0) &&
			(strcmp(regs[RT2], "sp") == 0)) {
			int srcreg;

			/*
			 * Decode MOV instruction.  See if source is a
			 * data register or something else.
			 */
			dualop(p);
			srcreg = isreg(regs[RT1]);
			if ((srcreg >= 0) && (srcreg < 8)) {
			    /*
			     * mov[wl] d?,sp@-		==> movw d?,sr
			     * jsr _splx		==>
			     * addql #4,sp		==>
			     */
			    (void) sprintf(buf, "d%d,sr", srcreg);
			    newcode(p, buf);
			    p->op = MOV;
			    p->subop = WORD;
			    release(p2);
			    release(p3);
			    p2 = p->forw;
			    continue;
			} else
			if ((regs[RT1][0]=='a' && regs[RT1][2]=='@') ||
			     (regs[RT1][0]=='_') ||
			     (regs[RT1][0]=='.' && regs[RT1][1]=='L') ||
			     (regs[RT1][0]>='0' && regs[RT1][0]<='9')) {
			    if ((p->subop == WORD) ||
				((p->subop == LONG) && newaddr(regs[RT1], 2))) {
				/*
				 * mov[wl] A,sp@-	==> movw A,sr
				 * jsr _splx		==>
				 * addql #4,sp		==>
				 */
				(void) sprintf(buf, "%s,sr", regs[RT1]);
				newcode(p, buf);
				p->op = MOV;
				p->subop = WORD;
				release(p2);
				release(p3);
				p2 = p->forw;
				continue;
			    }
			}
		    }
		}
	    }
	}

	/*
	 * Look for "x = swapw(y);"
	 */
	if ((p->op == MOV) && (p2->op == JSR) && (p3 = p2->forw)) {
	    /* check for "movl *,sp@-" */
	    dualop(p);
	    if (strcmp(regs[RT2], "sp@-") == 0) {
		/* check for "jsr _swapw" */
		singop(p2);
		if ((strcmp(regs[RT1], "_swapw") == 0) &&
		    (p3->op == ADDQ) && (p3->subop == LONG)) {
		    /* check for "addql #4,sp" */
		    dualop(p3);
		    if ((strcmp(regs[RT1], "#4") == 0) &&
			(strcmp(regs[RT2], "sp") == 0)) {
			int srcreg;

			/*
			 * Rewrite it.
			 */
			dualop(p);
			srcreg = isreg(regs[RT1]);
			if (srcreg == 0) {
			    /*
			     * movl d0,sp@-	==> swap d0
			     * jbsr _swapw	==>
			     * addql #4,sp	==>
			     */
			    newcode(p, "d0");
			    p->op = SWAP;
			    p->subop = 0;
			    release(p2);
			    release(p3);
			    p2 = p->forw;
			    continue;
			} else {
			    /*
			     * movl *,sp@-	==> movl *, d0
			     * jbsr _swapw	==> swap d0
			     * addql #4,sp	==>
			     */
			    (void) sprintf(buf, "%s,d0", regs[RT1]);
			    newcode(p, buf);
			    p->op = MOV;
			    p->subop = LONG;
			    newcode(p2, "d0");
			    p2->op = SWAP;
			    p2->subop = 0;
			    release(p3);
			    p = p2->forw;
			    continue;
			}
		    }
		}
	    }
	}

	/*
	 * Look for "x = swapw(y);", using pea instead of movl
	 */
	if ((p->op == PEA) && (p2->op == JSR) && (p3 = p2->forw)) {
	    /* check for "jsr _swapw" */
	    singop(p2);
	    if ((strcmp(regs[RT1], "_swapw") == 0) &&
		(p3->op == ADDQ) && (p3->subop == LONG)) {
		/* check for "addql #4,sp" */
		dualop(p3);
		if ((strcmp(regs[RT1], "#4") == 0) &&
		    (strcmp(regs[RT2], "sp") == 0)) {
		    /*
		     * Rewrite it.
		     */
		    dualop(p);
		    if (pea_numeric(regs[RT1])) {
			unsigned long value;

			/*
			 * pea num	==> movl #SWAPW(num), d0
			 * jbsr _swapw	==>
			 * addql #4,sp	==>
			 */
			value = numcvt(regs[RT1]);
			value = (value << 16) | (value >> 16);
			(void) sprintf(buf, "#0x%x,d0", value);
			newcode(p, buf);
			p->op = MOV;
			p->subop = LONG;
			release(p2);
			release(p3);
			p2 = p->forw;
			continue;
		    }
		}
	    }
	}

	/*
	 * Look for "x = swapb(y);"
	 */
	if ((p->op == MOV) && (p2->op == JSR) && (p3 = p2->forw)) {
	    /* check for "movl *,sp@-" */
	    dualop(p);
	    if (strcmp(regs[RT2], "sp@-") == 0) {
		/* check for "jsr _swapb" */
		singop(p2);
		if ((strcmp(regs[RT1], "_swapb") == 0) &&
		    (p3->op == ADDQ) && (p3->subop == LONG)) {
		    /* check for "addql #4,sp" */
		    dualop(p3);
		    if ((strcmp(regs[RT1], "#4") == 0) &&
			(strcmp(regs[RT2], "sp") == 0)) {
			int srcreg;

			/*
			 * Rewrite it.
			 */
			dualop(p);
			srcreg = isreg(regs[RT1]);
			if (srcreg == 0) {
			    /*
			     * movl d0,sp@-	==> rorw #8,d0
			     * jbsr _swapb	==>
			     * addql #4,sp	==>
			     */
			    newcode(p, "#8,d0");
			    p->op = ROR;
			    p->subop = WORD;
			    release(p2);
			    release(p3);
			    p2 = p->forw;
			    continue;
			} else {
			    /*
			     * movl *,sp@-	==> movl *,d0
			     * jbsr _swapb	==> rorw #8,d0
			     * addql #4,sp	==>
			     */
			    (void) sprintf(buf, "%s,d0", regs[RT1]);
			    newcode(p, buf);
			    p->op = MOV;
			    p->subop = LONG;
			    newcode(p2, "#8,d0");
			    p2->op = ROR;
			    p2->subop = WORD;
			    release(p3);
			    p = p2->forw;
			    continue;
			}
		    }
		}
	    }
	}

	/*
	 * Look for "x = swapb(y);", using pea instead of movl
	 */
	if ((p->op == PEA) && (p2->op == JSR) && (p3 = p2->forw)) {
	    /* check for "jsr _swapb" */
	    singop(p2);
	    if ((strcmp(regs[RT1], "_swapb") == 0) &&
		(p3->op == ADDQ) && (p3->subop == LONG)) {
		/* check for "addql #4,sp" */
		dualop(p3);
		if ((strcmp(regs[RT1], "#4") == 0) &&
		    (strcmp(regs[RT2], "sp") == 0)) {
		    /*
		     * Rewrite it.
		     */
		    dualop(p);
		    if (pea_numeric(regs[RT1])) {
			unsigned short value;

			/*
			 * pea num	==> movl #SWAPB(num), d0
			 * jbsr _swapb	==>
			 * addql #4,sp	==>
			 */
			value = numcvt(regs[RT1]);
			value = (value << 8) | (value >> 8);
			(void) sprintf(buf, "#0x%x,d0", value);
			newcode(p, buf);
			p->op = MOV;
			p->subop = LONG;
			release(p2);
			release(p3);
			p2 = p->forw;
			continue;
		    }
		}
	    }
	}

	/*
	 * Look for "x = swapitall(y);"
	 */
	if ((p->op == MOV) && (p2->op == JSR) && (p3 = p2->forw)) {
	    /* check for "movl *,sp@-" */
	    dualop(p);
	    if (strcmp(regs[RT2], "sp@-") == 0) {
		/* check for "jsr _swapitall" */
		singop(p2);
		if ((strcmp(regs[RT1], "_swapitall") == 0) &&
		    (p3->op == ADDQ) && (p3->subop == LONG)) {
		    /* check for "addql #4,sp" */
		    dualop(p3);
		    if ((strcmp(regs[RT1], "#4") == 0) &&
			(strcmp(regs[RT2], "sp") == 0)) {
			int srcreg;

			/*
			 * Rewrite it.
			 */
			dualop(p);
			srcreg = isreg(regs[RT1]);
			if (srcreg == 0) {
			    /*
			     * movl d0,sp@-	==> rorw #8,d0
			     * jbsr _swapitall	==> swap d0
			     * addql #4,sp	==> rorw #8, d0
			     */
			    newcode(p, "#8,d0");
			    p->op = ROR;
			    p->subop = WORD;
			    newcode(p2, "d0");
			    p2->op = SWAP;
			    p2->subop = 0;
			    newcode(p3, "#8,d0");
			    p3->op = ROR;
			    p3->subop = WORD;
			    p2 = p3;
			    continue;
			} else {
			    /*
			     * movl *,sp@-	==> movl *,d0
			     * jbsr _swapb	==> rorw #8,d0
			     * addql #4,sp	==> swap d0
			     *			==> rorw #8,d0
			     */
			    (void) sprintf(buf, "%s,d0", regs[RT1]);
			    newcode(p, buf);
			    p->op = MOV;
			    p->subop = LONG;
			    newcode(p2, "#8,d0");
			    p2->op = ROR;
			    p2->subop = WORD;
			    newcode(p3, "d0");
			    p3->op = SWAP;
			    p3->subop = 0;
			    p = p2->forw;
			    /* add new node for final rotate */
			    p4 = (struct node *)getnode();
			    p4->op = ROR;
			    p4->subop = WORD;
			    p4->labno = 0;
			    p4->code = (char *)copy("#8,d0");
			    p4->ref = 0;
			    p4->back = p3;
			    p4->forw = p3->forw;
			    p3->forw = p4;
			    if (p4->forw)
				p4->forw->back = p4;
			    p2 = p4;
			    continue;
			}
		    }
		}
	    }
	}

	/*
	 * Look for "x = swapitall(y);", using pea instead of movl
	 */
	if ((p->op == PEA) && (p2->op == JSR) && (p3 = p2->forw)) {
	    /* check for "jsr _swapitall" */
	    singop(p2);
	    if ((strcmp(regs[RT1], "_swapitall") == 0) &&
		(p3->op == ADDQ) && (p3->subop == LONG)) {
		/* check for "addql #4,sp" */
		dualop(p3);
		if ((strcmp(regs[RT1], "#4") == 0) &&
		    (strcmp(regs[RT2], "sp") == 0)) {
		    /*
		     * Rewrite it.
		     */
		    dualop(p);
		    if (pea_numeric(regs[RT1])) {
			union {
				unsigned long value;
				char parts[4];
			} v0, v1;;

			/*
			 * pea num	==> movl #SWAPITALL(num), d0
			 * jbsr _swapb	==>
			 * addql #4,sp	==>
			 */
			v0.value = numcvt(regs[RT1]);
			v1.parts[0] = v0.parts[3];
			v1.parts[1] = v0.parts[2];
			v1.parts[2] = v0.parts[1];
			v1.parts[3] = v0.parts[0];
			(void) sprintf(buf, "#0x%x,d0", v1.value);
			newcode(p, buf);
			p->op = MOV;
			p->subop = LONG;
			release(p2);
			release(p3);
			p2 = p->forw;
			continue;
		    }
		}
	    }
	}

	/*
	 * Delete fixed length string compares, if the constant is small
	 * Compiler generates stuff like:
	 *	pea	0xe
	 *	pea	a5@(0x77e)
	 *	pea	a2@(0x16)
	 *	jbsr	_strncmp
	 *	addw	#12,sp
	 *	tstl	d0
	 *	b{eq|ne} somewhere
	 */
	if ((p->op == PEA) && (p2->op == PEA) && (p3 = p2->forw) &&
	    (p3->op == PEA) && (p4 = p3->forw) &&
	    (p4->op == JSR) && (p5 = p4->forw) &&
	    (p5->op == ADD) && (p5->subop == WORD) && (p6 = p5->forw) &&
	    (p6->op == TST) && (p6->subop == LONG) && (p7 = p6->forw) &&
	    ((p7->op == CBR) || (p7->op == JBR) || (p7->op == XFER))) {
	    int amount;
	    char s1[MAXSIZE], s2[MAXSIZE];

	    singop(p);
	    /* check for "pea constant", and that constant is even and small */
	    if ((pea_numeric(regs[RT1])) && (amount = numcvt(regs[RT1])) &&
		(amount > 0) && ((amount & 1) == 0) && (amount <= 32)) {
		/* make sure a0/a1 are not used */
		singop(p2);
		if ((regs[RT1][0] != 'a') ||
		    ((regs[RT1][1] != '0') && (regs[RT1][1] != '1'))) {
		    strcpy(s1, regs[RT1]);			/* save */
		    /* make sure a0/a1 are not used */
		    singop(p3);
		    if ((regs[RT1][0] != 'a') ||
			((regs[RT1][1] != '0') && (regs[RT1][1] != '1'))) {
			strcpy(s2, regs[RT1]);			/* save */
			/* check for "jsr _strncmp" */
			singop(p4);
			if (strcmp(regs[RT1], "_strncmp") == 0) {
			    /* check for "addw #12,sp" */
			    dualop(p5);
			    if ((strcmp(regs[RT1], "#12") == 0) &&
				(strcmp(regs[RT2], "sp") == 0)) {
				/* check for "tstl d0" */
				singop(p6);
				if (isreg(regs[RT1]) == 0) {
				    /*
				     * Release old code, and generate new code.
				     */
				    release(p2);
				    release(p3);
				    release(p4);
				    release(p5);
				    release(p6);
				    p2 = gen_strncmp(p, amount, s1, s2);
				    continue;
				}
			    }
			}
		    }
		}
	    }
	}
    }
}

/*
 * assemble a word offset destination
 */
newaddr(cp, amt)
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
}

/*
 * return non zero if the passed string is a simple number
 */
pea_numeric(cp)
register char *cp;
{
	register c;

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
