/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)ptx:ptx.c	1.4";
#

/*	permuted title index
	ptx [-t] [-i ignore] [-o only] [-w num] [-f] [-r] \
	[-g n] [-b break] [input] [output]
	Ptx reads the input file and permutes on words in it.
	It excludes all words in the ignore file.
	Alternately it includes words in the only file.
	if neither is given it excludes the words in /usr/lib/eign.

	The width of the output line can be changed to num
	characters.  If omitted 72 is default unless troff than 100.
	the -f flag tells the program to fold the output
	the -t flag says the output is for troff and the
	output is then wider.

	make: cc -O ptx.c -o ptx 
	*/

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#define DEFLTX "/usr/lib/eign"
#define TILDE 0177
#define SORT "/bin/sort"
#define	N 30
#define	MAX	N*BUFSIZ
#define LMAX	200
#define MAXT	2048
#define MASK	03777
#define SET	1

#define isabreak(c) (btable[c])

extern char *calloc(), *mktemp();
extern char *getline();
int status;


char *hasht[MAXT];
char line[LMAX];
char btable[128];
int ignore;
int only;
int llen = 72;
int gap = 3;
int gutter = 3;
int mlen = LMAX;
int wlen;
int rflag;
int halflen;
char *strtbufp, *endbufp;
char *empty = "";

char *infile;
FILE *inptr = stdin;

char *outfile;
FILE *outptr = stdout;

char *sortfile;	/* output of sort program */
FILE *sortptr;

char *bfile;	/*contains user supplied break chars */
FILE *bptr;

main(argc,argv)
int argc;
char **argv;
{
	extern char *optarg;
	extern int optind;
	register int c;
	register char *bufp;
	int pid, m, test;
	int foldf = 0;
	char *pend;
	extern onintr();

	char *xfile;
	FILE *xptr;

	test = argc;
	if(signal(SIGHUP,onintr)==SIG_IGN)
		signal(SIGHUP,SIG_IGN);
	if(signal(SIGINT,onintr)==SIG_IGN)
		signal(SIGINT,SIG_IGN);
	signal(SIGPIPE,onintr);
	signal(SIGTERM,onintr);

/*	argument decoding	*/

	xfile = DEFLTX;
	while ((m = getopt(argc, argv, "ftrw:g:o:i:b:")) != EOF){
		switch (m){

		case 'r':
			rflag++;
			break;
		case 'f':
			foldf++;
			break;

		case 'w':
				test--;
				wlen++;
				llen = atoi(optarg);
				if(llen == 0)
					diag("Wrong width:",*optarg);
				if(llen > LMAX) {
					llen = LMAX;
					msg("Lines truncated to 200 chars.",empty);
				}
				break;

		case 't':
			if(wlen == 0)
				llen = 100;
			break;
		case 'g':
				test--;
				gap = gutter = atoi(optarg);
			break;

		case 'i':
			if(only) 
				diag("Only file already given.",empty);
				ignore++;
				xfile = optarg;
				test--;
			break;

		case 'o':
			if(ignore)
				diag("Ignore file already given",empty);
			else {
				only++;
				xfile = optarg;
			}
			test--;
			break;

		case 'b':
				bfile = optarg;
				test--;
			break;

		default:
			msg("Illegal argument:",*argv);
			}
		test--;
		}

	while (optind < argc) {
		if (test > 3) {
			if (optind == 1)
				diag("Too many filenames", empty);
			else
				test--;
		}
		else if (test == 3) {
				if (*argv[optind] == '-') {
					infile = 0;
					optind++;
					outfile = argv[optind++];
				}
				else {
					infile = argv[optind];
					optind++;
					outfile = argv[optind++];
				}
			}
		else if (test == 2) {
			if (*argv[optind] == '-') {
				infile = 0;
				outfile = 0;
				optind++;
			}
			else {
				infile = argv[optind];
				outfile = 0;
				optind++;
			}
		}
	}


	/* Default breaks of blank, tab and newline */
	btable[' '] = SET;
	btable['\t'] = SET;
	btable['\n'] = SET;
	if(bfile) {
		if((bptr = fopen(bfile,"r")) == NULL)
			diag("Cannot open break char file",bfile);

		while((c = getc(bptr)) != EOF)
			btable[c] = SET;
	}

/*	Allocate space for a buffer.  If only or ignore file present
	read it into buffer. Else read in default ignore file
	and put resulting words in buffer.
	*/


	if((strtbufp = calloc(N,BUFSIZ)) == NULL)
		diag("Out of memory space",empty);
	bufp = strtbufp;
	endbufp = strtbufp+MAX;

	if((xptr = fopen(xfile,"r")) == NULL)
		diag("Cannot open  file",xfile);

	while(bufp < endbufp && (c = getc(xptr)) != EOF) {
		if(isabreak(c)) {
			if(storeh(hash(strtbufp,bufp),strtbufp))
				diag("Too many words",xfile);
			*bufp++ = '\0';
			strtbufp = bufp;
		}
		else {
			*bufp++ = (isupper(c)?tolower(c):c);
		}
	}
	if (bufp >= endbufp)
		diag("Too many words in file",xfile);
	endbufp = --bufp;

	/* open output file for sorting */

	sortfile = mktemp("/tmp/ptxsXXXXX");
	if((sortptr = fopen(sortfile, "w")) == NULL)
		diag("Cannot open output for sorting:",sortfile);

/*	get a line of data and compare each word for
	inclusion or exclusion in the sort phase
*/

	if (infile!=0 && (inptr = fopen(infile,"r")) == NULL)
		diag("Cannot open data: ",infile);
	if (outfile != 0)
		if((outptr = fopen(outfile,"w")) == NULL)
			diag("Cannot open output file:",outfile);
	while(pend=getline())
		cmpline(pend);
	fclose(sortptr);

	switch (pid = fork()){

	case -1:	/* cannot fork */
		diag("Cannot fork",empty);

	case 0:		/* child */
		if(foldf == 0)
			execl(SORT, SORT, "-d", sortfile, "-o", sortfile, 0);
		else
			execl(SORT, SORT, "-df", sortfile, "-o", sortfile, 0);

	default:	/* parent */
		while(wait(&status) != pid);
	}


	getsort();
	onintr();
}

msg(s,arg)
char *s;
char *arg;
{
	fprintf(stderr,"%s %s\n",s,arg);
	return;
}
diag(s,arg)
char *s, *arg;
{

	msg(s,arg);
	exit(1);
}


char *getline()
{

	register c;
	register char *linep;
	char *endlinep;


	endlinep= line + mlen;
	linep = line;
	/* Throw away leading white space */

	while(isspace(c=getc(inptr)))
		;
	if(c==EOF)
		return(0);
	ungetc(c,inptr);
	while(( c=getc(inptr)) != EOF) {
		switch (c) {

			case '\t':
				if(linep<endlinep)
					*linep++ = ' ';
				break;
			case '\n':
				while(isspace(*--linep));
				*++linep = '\n';
				return(linep);
			default:
				if(linep < endlinep)
					*linep++ = c;
		}
	}
	return(0);
}

cmpline(pend)
char *pend;
{

	char *pstrt, *pchar, *cp;
	char **hp;
	int flag;

	pchar = line;
	if(rflag)
		while(pchar<pend&&!isspace(*pchar))
			pchar++;
	while(pchar<pend){
	/* eliminate white space */
		if(isabreak(*pchar++))
			continue;
		pstrt = --pchar;

		flag = 1;
		while(flag){
			if(isabreak(*pchar)) {
				hp = &hasht[hash(pstrt,pchar)];
				pchar--;
				while(cp = *hp++){
					if(hp == &hasht[MAXT])
						hp = hasht;
	/* possible match */
					if(cmpword(pstrt,pchar,cp)){
	/* exact match */
						if(!ignore && only)
							putline(pstrt,pend);
						flag = 0;
						break;
					}
				}
	/* no match */
				if(flag){
					if(ignore || !only)
						putline(pstrt,pend);
					flag = 0;
				}
			}
		pchar++;
		}
	}
}

cmpword(cpp,pend,hpp)
char *cpp, *pend, *hpp;
{
	char c;

	while(*hpp != '\0'){
		c = *cpp++;
		if((isupper(c)?tolower(c):c) != *hpp++)
			return(0);
	}
	if(--cpp == pend) return(1);
	return(0);
}

putline(strt, end)
char *strt, *end;
{
	char *cp;

	for(cp=strt; cp<end; cp++)
		putc(*cp, sortptr);
	/* Add extra blank before TILDE to sort correctly
	   with -fd option */
	putc(' ',sortptr);
	putc(TILDE,sortptr);
	for (cp=line; cp<strt; cp++)
		putc(*cp,sortptr);
	putc('\n',sortptr);
}

getsort()
{
	register c;
	register char *tilde, *linep, *ref;
	char *p1a,*p1b,*p2a,*p2b,*p3a,*p3b,*p4a,*p4b;
	int w;
	char *rtrim(), *ltrim();

	if((sortptr = fopen(sortfile,"r")) == NULL)
		diag("Cannot open sorted data:",sortfile);

	halflen = (llen-gutter)/2;
	linep = line;
	while((c = getc(sortptr)) != EOF) {
		switch(c) {

		case TILDE:
			tilde = linep;
			break;

		case '\n':
			while(isspace(linep[-1]))
				linep--;
			ref = tilde;
			if(rflag) {
				while(ref<linep&&!isspace(*ref))
					ref++;
				*ref++ = 0;
			}
		/* the -1 is an overly conservative test to leave
		   space for the / that signifies truncation*/
			p3b = rtrim(p3a=line,tilde,halflen-1);
			if(p3b-p3a>halflen-1)
				p3b = p3a+halflen-1;
			p2a = ltrim(ref,p2b=linep,halflen-1);
			if(p2b-p2a>halflen-1)
				p2a = p2b-halflen-1;
			p1b = rtrim(p1a=p3b+(isspace(p3b[0])!=0),tilde,
				w=halflen-(p2b-p2a)-gap);
			if(p1b-p1a>w)
				p1b = p1a;
			p4a = ltrim(ref,p4b=p2a-(isspace(p2a[-1])!=0),
				w=halflen-(p3b-p3a)-gap);
			if(p4b-p4a>w)
				p4a = p4b;
			fprintf(outptr,".xx \"");
			putout(p1a,p1b);
	/* tilde-1 to account for extra space before TILDE */
			if(p1b!=(tilde-1) && p1a!=p1b)
				fprintf(outptr,"/");
			fprintf(outptr,"\" \"");
			if(p4a==p4b && p2a!=ref && p2a!=p2b)
				fprintf(outptr,"/");
			putout(p2a,p2b);
			fprintf(outptr,"\" \"");
			putout(p3a,p3b);
	/* ++p3b to account for extra blank after TILDE */
	/* ++p3b to account for extra space before TILDE */
			if(p1a==p1b && ++p3b!=tilde)
				fprintf(outptr,"/");
			fprintf(outptr,"\" \"");
			if(p1a==p1b && p4a!=ref && p4a!=p4b)
				fprintf(outptr,"/");
			putout(p4a,p4b);
			if(rflag)
				fprintf(outptr,"\" %s\n",tilde);
			else
				fprintf(outptr,"\"\n");
			linep = line;
			break;

		case '"':
	/* put double " for "  */
			*linep++ = c;
		default:
			*linep++ = c;
		}
	}
}

char *rtrim(a,c,d)
char *a,*c;
{
	char *b,*x;
	b = c;
	for(x=a+1; x<=c&&x-a<=d; x++)
		if((x==c||isspace(x[0]))&&!isspace(x[-1]))
			b = x;
	if(b<c&&!isspace(b[0]))
		b++;
	return(b);
}

char *ltrim(c,b,d)
char *c,*b;
{
	char *a,*x;
	a = c;
	for(x=b-1; x>=c&&b-x<=d; x--)
		if(!isspace(x[0])&&(x==c||isspace(x[-1])))
			a = x;
	if(a>c&&!isspace(a[-1]))
		a--;
	return(a);
}

putout(strt,end)
char *strt, *end;
{
	char *cp;

	cp = strt;

	for(cp=strt; cp<end; cp++) {
		putc(*cp,outptr);
	}
}

onintr()
{

	if(*sortfile)
		unlink(sortfile);
	exit(1);
}

hash(strtp,endp)
char *strtp, *endp;
{
	char *cp, c;
	int i, j, k;

	/* Return zero hash number for single letter words */
	if((endp - strtp) == 1)
		return(0);

	cp = strtp;
	c = *cp++;
	i = (isupper(c)?tolower(c):c);
	c = *cp;
	j = (isupper(c)?tolower(c):c);
	i = i*j;
	cp = --endp;
	c = *cp--;
	k = (isupper(c)?tolower(c):c);
	c = *cp;
	j = (isupper(c)?tolower(c):c);
	j = k*j;

	k = (i ^ (j>>2)) & MASK;
	return(k);
}

storeh(num,strtp)
int num;
char *strtp;
{
	int i;

	for(i=num; i<MAXT; i++) {
		if(hasht[i] == 0) {
			hasht[i] = strtp;
			return(0);
		}
	}
	for(i=0; i<num; i++) {
		if(hasht[i] == 0) {
			hasht[i] = strtp;
			return(0);
		}
	}
	return(1);
}
