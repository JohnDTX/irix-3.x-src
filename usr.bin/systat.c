char _Origin_[] = "UniSoft Systems";

#include <stdio.h>
#include <a.out.h>

#define TABSIZE 32768
#define VIRT	0x100000

int	table[TABSIZE];
int mflag;
int zflag;
int ifound;
int mem;

struct nlist setup[] = {
#define	SADDR	0
	{"_profadd"},
#define	SBASE	1
	{"_profbas"},
	{""}
};

struct {
	char name[10];
	long ents;
} n, b;

FILE *ib;
int profaddr;
int profbase;

main(argc, argv)
int argc;
char *argv[];
{
	register long *ip;
	register i, k, sw, ttotal;
	register char *cp, *cp2, *nextaddr;
	register long dl;
	double total, nidle, nrest, total1, nrest1;
	char buf[50], _obuf[BUFSIZ];

	setbuf(stdout, _obuf);
	if ((mem = open("/dev/kmem", 0)) < 0) {
		printf("Can't open /dev/kmem\n");
		exit();
	}
	if ((ib = fopen("nlist", "r")) == NULL) {
		printf("Can't open %s\n", "nlist");
		exit();
	}

	nlist("/unix", setup);
	if (setup[SADDR].n_type == 0) {
		printf("no namelist\n");
		exit(1);
	}

	lseek(mem, (long)setup[SADDR].n_value, 0);
	read(mem, (char *)&profaddr, sizeof(profaddr));
	lseek(mem, (long)setup[SBASE].n_value, 0);
	read(mem, (char *)&profbase, sizeof(profbase));

	profbase <<= 11;
	if (phys(0, VIRT, 0x20000, profbase) < 0) {
		printf("cannot access physical memory\n");
		exit();
	}
	if (argc > 1)
		if (strcmp(argv[1], "-z")==0)
			zflag++;
		else
			mflag++;
	ttotal = 0;
	ip = (long *)VIRT;
	for (i=0; i<0x20000>>2; i++)
		table[i] = *ip++;
	if (zflag) {
		ip = (long *)VIRT;
		for (i=0; i<0x20000>>2; i++)
			*ip++ = 0;
	}
	ip = &table[0];
	for (i = 0; i < TABSIZE; i++)
		ttotal += *ip++;
	total = ttotal;
	printf("total = %.0f\n", total);
	total1 = total / 100.0;
	printf("routine   freq       total  system\n");
	fflush(stdout);
	for (sw = 0; sw < 2; sw++) {
	nextaddr = 0;
	n.ents = 0;
	nrest = total - nidle;
	nrest1 = nrest / 100.0;
	if (total1 == 0) total1 = 1.0;
	if (nrest1 == 0) nrest1 = 1.0;
	if (nrest == 0) nrest = 1.0;
	for (k = 0; k < TABSIZE; k++) {
		dl = table[k];
top:
		if ((cp = (char *)(k<<1)) >= nextaddr) {
			if (sw==0 && ifound==0 && cmp("_waitloc", n.name)) {
				ifound = 1;
				nidle = n.ents;
				for (i=0; i<8; i++)
					n.name[i] = ' ';
				break;
			}
			if (sw && mflag == 0)
			if (ifound==1 || (cmp(n.name, "_waitloc") == 0))
				printf("%-10s%-8d%7.2f%%%7.2f%%\n",
					n.name, n.ents, n.ents/total1,
					n.ents/nrest1);
			else {
				ifound = 1;
				printf("%-10s%-8d%7.2f%%\n",
					n.name, n.ents, n.ents/total1);
			}
			for (i = 0; i <= 8; i++)
				n.name[i] = b.name[i];
			n.ents = 0;
			cp = buf;
			while ((i = getc(ib)) != EOF) {
				if (i == '\n')
					break;
				*cp++ = i;
			}
			if (i < 0)
				break;
			*cp = 0;
			nextaddr = (char *)htoi(buf);
			cp = buf;
			while (*cp++ != ' ');
			while (*cp++ != ' ');
			cp2 = b.name;
			while (*cp2++ = *cp++);
			b.ents = 0;
			goto top;
		}
		n.ents += dl;
		if (sw && mflag)
			printf("%-10s%-8d\n", n.name, dl);
	}
	ifound = 0;
	if (sw == 0)
		rewind(ib);
	}
}

htoi(cp)
register char *cp;
{
	register c, i;

	i = 0;
	while (c = *cp++) {
		if (c >= '0' && c <= '9') {
			i = (i << 4) + c - '0';
		} else if (c >= 'a' && c <= 'f') {
			i = (i << 4) + c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			i = (i << 4) + c - 'A' + 10;
		} else
			break;
	}
	return(i);
}

cmp(p1, p2)
register char *p1, *p2;
{
	while (*p1 == *p2++)
		if (*p1++ == '\0')
			return(1);
	return(0);
}
