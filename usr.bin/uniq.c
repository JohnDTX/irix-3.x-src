char _Origin_[] = "System V";

/*
 * Deal with duplicated lines in a file
 */
static	char Sccsid[] = "@(#)uniq.c	1.3";

#include <stdio.h>
#include <ctype.h>
int	fields;
int	letters;
int	linec;
char	mode;
int	uniq;
char	*skip();

main(argc, argv)
int argc;
char *argv[];
{
	static char b1[1000], b2[1000];
	FILE *temp;
	register char *c;

	while(argc > 1) {
		if(*argv[1] == '-') {
			if (isdigit(argv[1][1])) {
				c= &argv[1][1];
				while (*c !='\0' && isdigit(*c))
					c++;
				if (*c != '\0') {
					fprintf(stderr, "Expects number after '+'\n");
					fprintf(stderr,
						"Usage: uniq [-udc] [+n] [-n] [input [output]]\n");
					exit(2);
				}
				fields = atoi(&argv[1][1]);
			} else {
				mode = argv[1][1];
				if (mode != 'u' && mode != 'd' && mode != 'c') {
					fprintf(stderr, "Bad option\n");
					fprintf(stderr,
						"Usage: uniq [-udc] [+n] [-n] [input [output]]\n");
					exit(2);
				}
			}
			argc--;
			argv++;
			continue;
		}
		if(*argv[1] == '+') {
			c= &argv[1][1];
			while (*c !='\0' && isdigit(*c))
				c++;
			if (*c != '\0') {
				fprintf(stderr, "Expects number after '+'\n");
				fprintf(stderr,
					"Usage: uniq [-udc] [+n] [-n] [input [output]]\n");
				exit(2);
			}
			letters = atoi(&argv[1][1]);
			argc--;
			argv++;
			continue;
		}
		if ( (temp = fopen(argv[1], "r")) == NULL)
			printe("cannot open %s\n", argv[1]);
		else {  fclose(temp);
			freopen(argv[1], "r", stdin);
		     }
		break;
	}
	if(argc > 2 && freopen(argv[2], "w", stdout) == NULL)
		printe("cannot create %s\n", argv[2]);

	if(gline(b1))
		exit(0);
	for(;;) {
		linec++;
		if(gline(b2)) {
			pline(b1);
			exit(0);
		}
		if(!equal(b1, b2)) {
			pline(b1);
			linec = 0;
			do {
				linec++;
				if(gline(b1)) {
					pline(b2);
					exit(0);
				}
			} while(equal(b1, b2));
			pline(b2);
			linec = 0;
		}
	}
}

gline(buf)
register char buf[];
{
	register c;

	while((c = getchar()) != '\n') {
		if(c == EOF)
			return(1);
		*buf++ = c;
	}
	*buf = 0;
	return(0);
}

pline(buf)
register char buf[];
{

	switch(mode) {

	case 'u':
		if(uniq) {
			uniq = 0;
			return;
		}
		break;

	case 'd':
		if(uniq) break;
		return;

	case 'c':
		printf("%4d ", linec);
		break;
	}
	uniq = 0;
	fputs(buf, stdout);
	putchar('\n');
}

equal(b1, b2)
register char b1[], b2[];
{
	register char c;

	b1 = skip(b1);
	b2 = skip(b2);
	while((c = *b1++) != 0)
		if(c != *b2++) return(0);
	if(*b2 != 0)
		return(0);
	uniq++;
	return(1);
}

char *
skip(s)
register char *s;
{
	register nf, nl;

	nf = nl = 0;
	while(nf++ < fields) {
		while(*s == ' ' || *s == '\t')
			s++;
		while( !(*s == ' ' || *s == '\t' || *s == 0) ) 
			s++;
	}
	while(nl++ < letters && *s != 0) 
			s++;
	return(s);
}

printe(p,s)
char *p,*s;
{
	fprintf(stderr, p, s);
	exit(1);
}
