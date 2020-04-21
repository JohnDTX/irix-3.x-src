char _Origin_[] = "System V";

static char sccsid[] = "@(#)cut.c	1.3";
#
/* cut : cut and paste columns of a table (projection of a relation) */
/* Release 1.5; rewrote for clarity; -f and -s fixed. */
# include <stdio.h>	/* make: cc cut.c */
# include <sys/types.h>
# include <sys/nami.h>
# define NFIELDS 1024	/* max no of fields or resulting line length */
# define BACKSPACE 8
main(argc, argv)
int argc; char **argv;
{
	int del = '\t';
	int i, j, c_count, f_count, poscnt, r, s, t;
	int endflag, supflag, cflag, fflag, backflag, filenr;
	int sel[NFIELDS];
	register int c;
	register char *p1;
	char *p2, *n1;
	int nodelims;
	char outbuf[NFIELDS], nodelbuf[NFIELDS]; 
				/* holds lines w/ no delimiters (-f) */
	FILE *inptr;
	endflag = supflag = cflag = fflag = 0;
 
 
while (argc > 1 && argv[1][0] == '-') {
    for (i = 1; (c = argv[1][i]) != '\0'; i++) {
	switch(c) {
	    case 'd' : 	del = argv[1][++i];
		       	if (del == '\0') diag("no delimiter\n");
			break;
	    case 's': 	supflag++ ;
		      	break;
	    case 'c': 	cflag++ ;
			break;
	    case 'f': 	fflag++ ;
			break;
	    default :   usage();
	}
	if (!endflag && (cflag || fflag)) {
	    endflag = 1;
	    r = s = t = 0;
	    do {	
		c = argv[1][++i];
		switch(c) {
		    case '-' : 	if (r) diag("bad list for c/f option\n");
				r = 1;
				if (t == 0)  
				    s = 1;
				else { 
				    s = t; 
				    t = 0;
				}
				continue;
		    case '\0' :
		    case ','  : if (t >= NFIELDS) diag("bad list for c/f option\n");
				if (r) { 
				    if (t == 0) t = NFIELDS - 1;
				    if (t<s) diag("bad list for c/f option\n");
				    for(j = s; j <= t; j++) sel[j] = 1;
				} else 
				    sel[t] = (t > 0 ? 1 : 0);
				r = s = t = 0;
				if (c == '\0') {
				    i--; 
				    break;
				}
				continue;
		    default : 	if (c< '0' || c> '9') diag("bad list for c/f option\n");
				t = 10*t + c - '0';
				continue;
		}
		for (j = t = 0; j < NFIELDS; j++) t += sel[j];
		if (t == 0) diag("no fields\n");
	    } while (c != '\0');
	}
    }
    --argc;
    ++argv;
} /* end options */

if (!(cflag || fflag)) usage();
if (cflag) {
    supflag = 0;		/* can't use -s and -c together */
    nodelims = 0;
} else /* fflag */
    nodelims = 1;

--argc;
filenr = 1;
do {	/* for all input files */
    if (argc > 0) inptr = fopen(argv[filenr], "r");
    else inptr = stdin;
  
    if (inptr == NULL) {
	char buf[MAXPATHLEN+32];
	sprintf(buf,"cannot open %s\n", argv[filenr]);
	diag(buf);
    }

    endflag = 0;
    do {  /* for all lines of a file */
	  /* c_count = current # of chars read (include backspaces)
	     f_count = current field # 
	     poscnt = current # of chars read (excluding backspaces, \n)
	     backflag = current # of back-to-back backspaces. 
	  */
	c_count = poscnt = 0;
	f_count = 1;
	backflag = 0;
	n1 = &nodelbuf[0] - 1 ;
	p1 = &outbuf[0] - 1 ;
	p2 = p1;
	do { 	/* for all char of the line */
	    c = fgetc(inptr);
	    if (c == EOF) {
		endflag = 1;
		break;
	    }
	    *++p1 = c;
	    if (c == BACKSPACE) 
		backflag++;
 	    else { 
		if (!backflag) poscnt++; 
		    else backflag-- ;
	    }
	    if (backflag > 1) 
		diag("too many backspaces\n");
	    if (poscnt == NFIELDS -1)
		diag("line too long\n");

	    if (cflag) {
		if (c != '\n') 
		    c_count++;
		if (sel[poscnt]) p2 = p1; 
		    else p1 = p2;
	    } else {  /* fflag */
		if (nodelims)
		    *++n1 = c;
		if (sel[f_count]) p2 = p1; 
		    else p1 = p2;
		if (c == del) {
		    nodelims = 0;
		    f_count++;
		    if (f_count == NFIELDS - 1)
			diag("line too long\n");
		}
	    }
	} while (c != '\n');

		
	/* If line has positive length and ends with EOF */
	if (endflag && (poscnt > 0))
	    diag("bad line format\n");

	 /* If line has positive length and ends with \n */
	if (!endflag && (poscnt > 0)) {
	    /* suppress trailing delimiter and \n */
	    if ((*p1 == del) || (*p1 == '\n')) {
	    	*p1 = '\0';
	    } else {
	    	*++p1 = '\0'; 
	    }
	    if ((*n1 == del) || (*n1 == '\n')) {
	    	*n1 = '\0';
	    } else {
		*++n1 = '\0'; 
	    }
	    if (!nodelims)
	    	puts(outbuf);
	    else if (!supflag && fflag) 
	    	puts(nodelbuf);
	}
    } while (!endflag) ;
    fclose(inptr);
} while(++filenr <= argc);
exit(0);
}

diag(s)
char *s;
{

    write(2, "cut: ", 6);
    while(*s)
	write(2,s++,1);
    exit(2);
}

usage()
{

    printf("cut {-c<list> | -f<list> [-s] [-d<char>]} file ...\n");
    exit(2);
}
