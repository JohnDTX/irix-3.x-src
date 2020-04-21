#ifdef V7
char _Version_[] = "(C) UniSoft Corp., Version 2.4";
#endif

#ifdef PWB
char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version III.1.4";
char _Origin_[] = "UniSoft Systems";
#endif

/*
 *	 C object code improver
 */

/* took out B switch GB (SGI) 8/20/83. jsr is translated to jbsr now always.
   (yes, NOT the other way around!) */


#include "opt.h"

FILE *infile,*outfile;
char *outfname;
int maxiter;
int sline;
int eline;
int lflag;

main(argc, argv)
char **argv;
{
	register int niter, isend;
	int nflag;

	nflag = 0;
	if (argc>1 && argv[1][0]=='-') {
		argv++;
		argc--;
		for (isend=1; argv[0][isend]; isend++) {
			switch (argv[0][isend]){
			case 'B': /* never do this GB SGI Bflg++;*/  break;
			case 'K': Kflg++; break;
			case 'P': Pflg++; break;
			case 'S': Sflg++; break;
			case 'd': debug++; break;
			case 'l': lflag++; break;
			case 's': nflag++; break;
			case 'k': skflg++; break;
			default:
			    fprintf(stderr, "usage:c2 [-kKPSdls] [if [of] ]\n");
			    exit(-1);
			}
		}
	}
	if (argc>1) {
		if ((infile = fopen(argv[1], "r")) == NULL) {
			fprintf(stderr,"c2: can't find %s\n", argv[1]);
			exit(1);
		}
		if (Bflg)
			genjsr(infile);
	} else
		infile = stdin;
	if (argc>2) {
		outfname = argv[2];
		if ((outfile = fopen(outfname, "w")) == NULL) {
			fprintf(stderr,"c2: can't create %s\n", outfname);
			exit(1);
		}
	} else
		outfile = stdout;

	freenodes = 0;
	maxiter = 0;
	opsetup();
	do {
		isend = input();
		movedat();
		nchange = niter = 0;
		preopt();
		aregopt();
		dregopt();
		do {
			refcount();
			do {
				if (debug) printf("iterate\n");
				iterate();
				clearreg();
				niter++;
			} while (nchange);
			if (debug) printf("comjump\n");
			comjump();
			if (debug) printf("rmove\n");
			rmove();
		} while (nchange || jumpsw());
		lastopt();
		output();
		(void) fflush(outfile);
		if (niter > maxiter)
			maxiter = niter;
	} while (isend);
	(void) fflush(outfile);
	if (nflag)
		summary();
	exit(0);
}

input()
{
	register struct node *p, *lastp;
	register int op;
	int subop;

	lastp = &first;
	for (;;) {
		op = getline();
		subop = (op>>8)&0377;
		op &= 0377;
		switch (op) {
	
		case LABEL:
			p = (struct node *)getnode();
			if (line[0]=='.' && line[1]=='L') {
				p->labno = getnum(line+2);
				p->op = LABEL;
				p->code = 0;
			} else {
				p->op = DLABEL;
				p->labno = 0;
				p->code = (char *)copy(line);
			}
			break;
	
		case JSW:
			p = (struct node *)getnode();
			p->op = JSW;
			p->subop = 0;
			if (*curlp=='.' && *(curlp+1)=='L') {
			  p->labno = getnum(curlp+2);
			  while (*curlp && *curlp!='-') curlp++;
			  if (*curlp == 0) goto notjsw;
			  p->code = (char *)copy(curlp);
			  break;
			}
		notjsw:	p->op = p->labno = 0;
			p->code = (char *)copy(line);
			break;

		case JBR:
		case CBR:
		case JMP:
		case XFER:
			p = (struct node *)getnode();
			p->op = op;
			p->subop = subop;
			if (*curlp=='.' && *(curlp+1)=='L') {
				p->labno = getnum(curlp+2);
				p->code = 0;
			} else if (*curlp=='p' && *(curlp+1)=='c' && *(curlp+2)=='@') {
				p->op = p->subop = p->labno = 0;
				p->code = (char *)copy(line);
			} else {
				p->labno = 0;
				p->code = (char *)copy(curlp);
			}
			break;

		default:
			p = (struct node *)getnode();
			p->op = op;
			p->subop = subop;
			p->labno = 0;
			p->code = (char *)copy(curlp);
			break;

		}
		p->lineno = sline;
		p->misc = 0;
		p->forw = 0;
		p->back = lastp;
		lastp->forw = p;
		lastp->misc = eline;
		lastp = p;
		p->ref = 0;
		if (op==EROU)
			return(1);
		if (op==EOSUB) {
			p->op=0;
			return(1);
		}
		if (op==END)
			return(0);
	}
}

getline()
{
	register char *l, *lp;
	register FILE *fp;
	register c, ftab;

	sline = 0;
	eline = 0;
	fp = infile;
  again:
	l = lp = line;
	ftab = 0;
	while ((c = getc(fp)) != EOF) {
		if (c==':' && !ftab) {
			*lp = 0;
			return(LABEL);
		}
		if (c=='\n') {
			if (lp==l) goto again;
			*lp++ = 0;
			if (*l=='|' && l[2]==' ') {
				if (l[1] == 'l') {
					sline = atoi(l+3);
					goto again;
				}
				if (l[1] == 'e') {
					eline = 1;
					goto again;
				}
			}
			*lp++ = 0;
			if (*l=='|' && strcmp(l, "| end")==0)
				return(EOSUB);
			return(oplook());
		}
		if (c=='\t') ftab++;
		*lp++ = c;
	}
	*lp = 0;
	return(END);
}

getnum(ap)
char *ap;
{
	register char *p;
	register n, c;

	p = ap;
	n = 0;
	while ((c = *p++) >= '0' && c <= '9')
		n = n*10 + c - '0';
	if (*--p!=0 && *p!='-')
		return(0);
	return(n);
}

output()
{
	register struct optab *op, **ophp;
	register struct node *t;
	register int byte, topcode;
	struct node *temp;

	t = first.forw;
	while (t) {
	if (debug)
printf("output: t->op=%d, t->subop=%d, t->code=%d, t->labno=%d, t->misc=%d\n",
t->op, t->subop, t->code, t->labno, t->misc);
	switch (t->op) {

	case END:
		return;

	case LABEL:
		fprintf(outfile,".L%d:", t->labno);
		if (lflag && t->lineno)
			fprintf(outfile, "\t| line %d", t->lineno);
		putc('\n', outfile);
		break;

	case DLABEL:
		fputs(t->code, outfile);
		putc(':', outfile);
		if (lflag && t->lineno)
			fprintf(outfile, "\t| line %d", t->lineno);
		putc('\n', outfile);
		cfree(t->code);
		break;

	default:
		byte = t->subop;
		if (byte==BYTE || byte==WORD || byte==LONG) t->subop = 0;
		topcode = t->op | (t->subop<<8);
		ophp = &opcdhash[(topcode<<1) % OPCDHS];
		while (op = *ophp) {
			if (op->opcode == topcode) {
				if (debug)
					printf("output: opcode = %d\n",
						op->opcode);
				if (t->op==CBR || t->op==JBR || t->op==XFER) {
					  putc('\t', outfile);
					  putc('j', outfile);
					  fputs(op->opstring+1, outfile);
				} else {
					  putc('\t', outfile);
					  fputs(op->opstring, outfile);
				}
				if (byte==LONG) putc('l', outfile);
				else if (byte==BYTE) putc('b', outfile);
				else if (byte==WORD) putc('w', outfile);
				break;
			}
			ophp++;
			if (ophp >= &opcdhash[OPCDHS])
				ophp = opcdhash;
		}
		if (t->op==JSW) {
			fprintf(outfile,"\t.L%d%s",t->labno,t->code);
			if (lflag && t->lineno)
				fprintf(outfile, "\t| line %d", t->lineno);
			putc('\n', outfile);
			cfree(t->code);
		} else if (t->code) {
			putc('\t', outfile);
			fputs(t->code, outfile);
			if (lflag && t->lineno)
				fprintf(outfile, "\t| line %d", t->lineno);
			putc('\n', outfile);
			cfree(t->code);
		} else if (t->op==JBR || t->op==CBR || t->op==XFER) {
			fprintf(outfile,"\t.L%d", t->labno);
			if (lflag && t->lineno)
				fprintf(outfile, "\t| line %d", t->lineno);
			putc('\n', outfile);
		} else
			putc('\n', outfile);
		break;

	case 0:
		if (t->code) {
			fputs(t->code, outfile);
			if (lflag && t->lineno)
				fprintf(outfile, "\t| line %d", t->lineno);
			cfree(t->code);
		}
		putc('\n', outfile);
		break;
	}
	temp = t->forw;
	t->ref = freenodes;
	freenodes = t;
	t = temp;
	}
}

char *
copy(p)
register char *p;
{	register char *onp;
	register int n = strlen(p);

	if (n==0) return(0);
	onp = (char *)calloc((unsigned)(n+1),1);	
	if (onp == 0) {
		fprintf(stderr, "c2:out of string storage space\n");
		exitt(-1);
	}
	(void) strcpy(onp,p);
	return(onp);
}

exitt(a)
{
	if (outfname)
		(void) unlink(outfname);
	exit(a);
}

#ifdef TRACE
trace()
{
	register struct node *t;
	register struct optab *op;
	register int byte;
	struct node *temp;
	int tsubop;

	summary();
	t = first.forw;
	while (t) {
	switch (t->op) {

	case END:
		return;

	case LABEL:
		fprintf(outfile,".L%d:\n", t->labno);
		break;

	case DLABEL:
		fprintf(outfile,"%s:\n", t->code);
		break;

	default:
		tsubop = t->subop;
		byte = t->subop;
		if (byte==BYTE || byte==WORD || byte==LONG) tsubop = 0;
		for (op = optab; op->opstring!=0; op++) {
			if (op->opcode == (t->op | (tsubop<<8))) {
				if (t->op==CBR || t->op==JBR || t->op==XFER)
				  fprintf(outfile,"\tj%s", op->opstring+1);
				else fprintf(outfile,"\t%s", op->opstring);
				if (byte==BYTE) fprintf(outfile,"b");
				if (byte==WORD) fprintf(outfile,"w");
				if (byte==LONG) fprintf(outfile,"l");
				break;
			}
		}
		if (t->op==JSW)
			fprintf(outfile,"\t.L%d%s\n",t->labno,t->code);
		else if (t->code)
			fprintf(outfile,"\t%s\n", t->code);
		else if (t->op==JBR || t->op==CBR || t->op==XFER)
			fprintf(outfile,"\t.L%d\n", t->labno);
		else
			fprintf(outfile,"\n");
		break;

	case 0:
		if (t->code)
			fprintf(outfile,"%s", t->code);
		fprintf(outfile,"\n");
		break;
	}
	t = t->forw;
	}
}
#endif

/*
 * summarize optimization results
 */
summary()
{
	register FILE *fp;

	fp = stdout;
	if (maxiter) fprintf(fp,"%d iterations\n", maxiter);
	if (nbrbr) fprintf(fp,"%d jumps to jumps\n", nbrbr);
	if (iaftbr) fprintf(fp,"%d inst. after jumps\n", iaftbr);
	if (njp1) fprintf(fp,"%d jumps to .+2\n", njp1);
	if (nrlab) fprintf(fp,"%d redundant labels\n", nrlab);
	if (nxjump) fprintf(fp,"%d cross-jumps\n", nxjump);
	if (ncmot) fprintf(fp,"%d code motions\n", ncmot);
	if (nrevbr) fprintf(fp,"%d branches reversed\n", nrevbr);
	if (redunm) fprintf(fp,"%d redundant moves\n", redunm);
	if (nsaddr) fprintf(fp,"%d simplified addresses\n", nsaddr);
	if (loopiv) fprintf(fp,"%d loops inverted\n", loopiv);
	if (nredunj) fprintf(fp,"%d redundant jumps\n", nredunj);
	if (ncomj) fprintf(fp,"%d common seqs before jmp's\n", ncomj);
	if (nskip) fprintf(fp,"%d skips over jumps\n", nskip);
	if (nrtst) fprintf(fp,"%d redundant tst's\n", nrtst);
}
