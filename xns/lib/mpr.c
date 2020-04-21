prw(w, cc)
register short *w;
register cc;
{

	while (cc>0) {
		printf("%x ", *w++);
		cc -= 2;
	}
	printf("\n");
}

mprw(m, w, cc)
char *m;
{
	printf("%s ", m);
	prw(w, cc);
}

prc(s, cc)
register char *s;
register cc;
{

	while (cc--)
		printf("%x ", *s++ & 0377);
	printf("\n");
}

mprc(m, s, cc)
char *m, *s;
{
	printf("%s ", m);
	prc(s, cc);
}
