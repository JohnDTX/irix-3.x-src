char _Origin_[] = "System V";

static	char Sccsid[] = "@(#)split.c	1.3";

#include <stdio.h>
char	*strrchr();
unsigned count = 1000;
int	fnumber;
char	fname[100];
char	*ifil;
char	*ofil;
char	*tail;
FILE	*is;
FILE	*os;

main(argc, argv)
char *argv[];
{
	register i, c, f;
	int iflg = 0;

	for(i=1; i<argc; i++)
		if(argv[i][0] == '-')
			switch(argv[i][1]) {
		
			case '\0':
				iflg = 1;
				continue;
		
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				count = atoi(argv[i]+1);
				continue;
			}
		else if(iflg)
			ofil = argv[i];
		else {
			ifil = argv[i];
			iflg = 2;
		}
	if(iflg != 2)
		is = stdin;
	else
		if((is=fopen(ifil,"r")) == NULL) {
			fprintf(stderr,"cannot open input\n");
			exit(1);
		}
	if(ofil == 0)
		ofil = "x";
	if ((tail = strrchr(ofil, '/')) == NULL)
		tail = ofil;
	else
		tail++;
	if(strlen(tail)>12) {
		fprintf(stderr,"more than 12 characters in output file name\n");
		exit(1); }

loop:
	f = 1;
	for(i=0; i<count; i++)
	do {
		c = getc(is);
		if(c == EOF) {
			if(f == 0)
				fclose(os);
			exit(0);
		}
		if(f) {
			for(f=0; ofil[f]; f++)
				fname[f] = ofil[f];
			fname[f++] = fnumber/26 + 'a';
			fname[f++] = fnumber%26 + 'a';
			fname[f] = '\0';
			if(++fnumber > 676) {
				fprintf(stderr,"more than aa-zz output files needed, aborting split\n");
				exit(1); }
			if((os=fopen(fname,"w")) == NULL) {
				fprintf(stderr,"Cannot create output\n");
				exit(1);
			}
			f = 0;
		}
		putc(c, os);
	} while(c != '\n');
	fclose(os);
	goto loop;
}
