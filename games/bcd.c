char _Origin_[] = "System III";

#include <stdio.h>

int chtab[] = {
00000, /*   */
03004, /* ! */
02404, /* " */
02040, /* sharp */
02042, /* $ */
02104, /* % */
00001, /* & */
03002, /* ' */
02201, /* ( */
02202, /* ) */
02102, /* * */
00005, /* + */
02044, /* , */
00002, /* - */
02041, /* . */
00014, /* / */
00004, /* 0 */
00010, /* 1 */
00020, /* 2 */
00040, /* 3 */
00100, /* 4 */
00200, /* 5 */
00400, /* 6 */
01000, /* 7 */
02000, /* 8 */
04000, /* 9 */
02200, /* : */
02402, /* ; */
02401, /* < */
02204, /* = */
02400, /* > */
03000, /* ? */
02100, /* at */
 011,
 021,
 041,
0101,
0201,
0401,
01001,
02001,
04001,
012,
022,
042,
0102,
0202,
0402,
01002,
02002,
02002,
024,
044,
0104,
0204,
0404,
01004,
02004,
04004,
02020, /* [ */
03001, /* \ */
02101, /* ] */
00006, /* ^ */
02024 /* _ */
};

char s[140];

main(argc, argv)
char *argv[];
{
	register char *sp = &s[0];
	register char *spp;
	register int i, j, c, l;
	char _obuf[BUFSIZ];

	setbuf(stdout, _obuf);
	if (argc<2) {
		printf("Enter data: ");
		fflush(stdout);
		while (sp < &s[128] && (c=getchar())!='\0' && c!='\n')
			*sp++ = c;
	} else {
		while (--argc > 0) {
			spp = *++argv;
			while (sp < &s[128] && (*sp++ = *spp++)) ;
			*(sp-1) = ' ';
		}
	}
	*sp = 0;
	sp = &s[0];
	puts("\n\n\n\n");
	puts(" ________________________________");
	puts("________________\n");
	spp = sp;
	while(*spp++);
	spp--;
	l = spp - sp;
	putchar('/');
	puts(sp);
	i = 49 - l;
	while(--i>0) putchar(' ');
	puts("|\n");
	j = 0;
	spp = sp;
	while (j++<12) {
		putchar('|');
		i = 0;
		spp = sp;
		while (i<48) {
			if(i>l) c = 0;
			else c = *spp++ - 040;
			i++;
			if (c>='a'-040) c = c - 040;
			if (c<0 | c>137) c = 0;
			if ((chtab[c]>>(j-1))&1) 
				puts("[]");
			else
				putchar(j>3?'0'+j-3:' ');
		}
		puts("|\n");
	}
	putchar('|');
	puts("____________");
	puts("____________________________________");
	puts("|\n");
	puts("\n\n\n\n");
	fflush(stdout);
	exit(0);
}

puts(ss) char *ss; {
	int i;
	char t;
	i = 0;
	while(t = *ss++) {
		if(t >= 'a' && t <= 'z')
			t += 'A'-'a';
		putchar(t);
	}
}
