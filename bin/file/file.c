char _Origin_[] = "System V";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/file/RCS/file.c,v 1.1 89/03/27 14:54:03 root Exp $";
/*
 * $Log:	file.c,v $
 * Revision 1.1  89/03/27  14:54:03  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  86/05/30  17:25:46  paulm
 * Changes to allow file to distinguish between .o files ("relocatable")
 * and final a.out files that are not shared text ("executable").
 * 
 * Revision 1.2  85/04/29  18:21:03  bob
 * Added symbolic links.
 * 
 */

/*	@(#)file.c	1.5	*/

#include	<stdio.h>
#include	<ctype.h>
#include	<signal.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>

/*
**	Types
*/

#define	BYTE	0
#define	SHORT	2
#define	LONG	4
#define	STR	8

/*
**	Opcodes
*/

#define	EQ	0
#define	GT	1
#define	LT	2
#define	STRC	3	/* string compare */
#define	ANY	4
#define	SUB	64	/* or'ed in */

/*
**	Misc
*/

#define	NENT	200
#define	BSZ	128
#define	FBSZ	512
#define	reg	register

#ifndef	S_IFLNK
#define	lstat	stat
#endif	S_IFLNK

/*
**	Structure of magic file entry
*/

struct	entry	{
	char	e_level;	/* 0 or 1 */
	long	e_off;		/* in bytes */
	char	e_type;
	char	e_opcode;
	union	{
		long	num;
		char	*str;
	}	e_value;
	char	*e_str;
};

typedef	struct entry	Entry;

Entry	*mtab;
char	fbuf[FBSZ];
char	*mfile = "/etc/magic";
char	*fort[] = {
	"function","subroutine","common","dimension","block","integer",
	"real","data","double",0};
char	*asc[] = {
	"sys","mov","tst","clr","jmp",0};
char	*c[] = {
	"int","char","float","double","struct","extern",0};
char	*as[] = {
	"globl","byte","even","text","data","bss","comm",0};
char	*strchr();
char	*malloc();
long	atolo();
int	i = 0;
int	fbsz;
int	ifd;

#define	prf(x)	printf("%s:%s", x, strlen(x)>6 ? "\t" : "\t\t");

main(argc, argv)
char **argv;
{
	reg	char	*p;
	reg	int	ch;
	reg	FILE	*fl;
	reg	int	cflg = 0, eflg = 0, fflg = 0;
	auto	char	ap[128];
	extern	int	optind;
	extern	char	*optarg;

	while((ch = getopt(argc, argv, "cf:m:")) != EOF)
	switch(ch) {
	case 'c':
		cflg++;
		break;

	case 'f':
		fflg++;
		if ((fl = fopen(optarg, "r")) == NULL) {
			fprintf(stderr, "cannot open %s\n", optarg);
			goto use;
		}
		break;

	case 'm':
		mfile = optarg;
		break;

	case '?':
		eflg++;
		break;
	}
	if(!cflg && !fflg && (eflg || optind == argc)) {
use:
		fprintf(stderr,
			"usage: file [-c] [-f ffile] [-m mfile] file...\n");
		exit(2);
	}
	if(cflg) {
		reg	Entry	*ep;

		mkmtab(1);
		printf("level	off	type	opcode	value	string\n");
		for(ep = mtab; ep->e_off != -1L; ep++) {
			printf("%d\t%d\t%d\t%d\t", ep->e_level, ep->e_off,
				ep->e_type, ep->e_opcode);
			if(ep->e_type == STR)
				printf("%s\t", ep->e_value.str);
			else
				printf("%d\t", ep->e_value.num);
			printf("%s", ep->e_str);
			if(ep->e_opcode & SUB)
				printf("\tsubst");
			printf("\n");
		}
		exit(0);
	}
	for(; fflg || optind < argc; optind += !fflg) {
		reg	int	l;

		if(fflg) {
			if((p = fgets(ap, 128, fl)) == NULL) {
				fflg = 0;
				optind--;
				continue;
			}
			l = strlen(p);
			if(l > 0)
				p[l - 1] = '\0';
		} else
			p = argv[optind];
		prf(p);
		type(p);
		if(ifd)
			close(ifd);
	}
	exit(0);
}

type(file)
char	*file;
{
	int	j,nl;
	char	ch;
	struct	stat	mbuf;

	ifd = -1;
	if(lstat(file, &mbuf) < 0) {
		printf("cannot open\n");
		return;
	}
	switch (mbuf.st_mode & S_IFMT) {
	case S_IFCHR:
		printf("character");
		goto spcl;

	case S_IFDIR:
		printf("directory\n");
		return;

	case S_IFIFO:
		printf("fifo\n");
		return;

#ifdef	S_IFLNK
	case S_IFLNK:
		printf("symbolic link\n");
		return;
#endif	S_IFLNK

	case S_IFBLK:
		printf("block");

spcl:
		printf(" special (%d/%d)\n", major(mbuf.st_rdev),
							minor(mbuf.st_rdev));
		return;
	}
	ifd = open(file, 0);
	if(ifd < 0) {
		printf("cannot open for reading\n");
		return;
	}
	fbsz = read(ifd, fbuf, FBSZ);
	if(fbsz == 0) {
		printf("empty\n");
		goto out;
	}
	if(ckmtab())
		goto out;
	i = 0;
	if(ccom() == 0)
		goto notc;
	while(fbuf[i] == '#') {
		j = i;
		while(fbuf[i++] != '\n') {
			if(i - j > 255) {
				printf("data\n"); 
				goto out;
			}
			if(i >= fbsz)
				goto notc;
		}
		if(ccom() == 0)
			goto notc;
	}
check:
	if(lookup(c) == 1) {
		while((ch = fbuf[i++]) != ';' && ch != '{')
			if(i >= fbsz)
				goto notc;
		printf("c program text");
		goto outa;
	}
	nl = 0;
	while(fbuf[i] != '(') {
		if(fbuf[i] <= 0)
			goto notas;
		if(fbuf[i] == ';'){
			i++; 
			goto check; 
		}
		if(fbuf[i++] == '\n')
			if(nl++ > 6)goto notc;
		if(i >= fbsz)goto notc;
	}
	while(fbuf[i] != ')') {
		if(fbuf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= fbsz)
			goto notc;
	}
	while(fbuf[i] != '{') {
		if(fbuf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= fbsz)
			goto notc;
	}
	printf("c program text");
	goto outa;
notc:
	i = 0;
	while(fbuf[i] == 'c' || fbuf[i] == '#') {
		while(fbuf[i++] != '\n')
			if(i >= fbsz)
				goto notfort;
	}
	if(lookup(fort) == 1){
		printf("fortran program text");
		goto outa;
	}
notfort:
	i = 0;
	if(ascom() == 0)
		goto notas;
	j = i-1;
	if(fbuf[i] == '.') {
		i++;
		if(lookup(as) == 1){
			printf("assembler program text"); 
			goto outa;
		}
		else if(j != -1 && fbuf[j] == '\n' && isalpha(fbuf[j+2])){
			printf("[nt]roff, tbl, or eqn input text");
			goto outa;
		}
	}
	while(lookup(asc) == 0) {
		if(ascom() == 0)
			goto notas;
		while(fbuf[i] != '\n' && fbuf[i++] != ':')
			if(i >= fbsz)
				goto notas;
		while(fbuf[i] == '\n' || fbuf[i] == ' ' || fbuf[i] == '\t')
			if(i++ >= fbsz)
				goto notas;
		j = i - 1;
		if(fbuf[i] == '.'){
			i++;
			if(lookup(as) == 1) {
				printf("assembler program text"); 
				goto outa; 
			}
			else if(fbuf[j] == '\n' && isalpha(fbuf[j+2])) {
				printf("[nt]roff, tbl, or eqn input text");
				goto outa;
			}
		}
	}
	printf("assembler program text");
	goto outa;
notas:
	for(i=0; i < fbsz; i++)
		if(fbuf[i]&0200) {
			if (fbuf[0]=='\100' && fbuf[1]=='\357') {
				printf("troff output\n");
				goto out;
			}
			printf("data\n"); 
			goto out; 
		}
	if (mbuf.st_mode&((S_IEXEC)|(S_IEXEC>>3)|(S_IEXEC>>6)))
		printf("commands text");
	else if(english(fbuf, fbsz))
		printf("English text");
	else
		printf("ascii text");
outa:
	while(i < fbsz)
		if((fbuf[i++]&0377) > 127) {
			printf(" with garbage\n");
			goto out;
		}
	printf("\n");
out:
	utime(file, &mbuf.st_atime);
}

mkmtab(cflg)
reg	int	cflg;
{
	reg	Entry	*ep;
	reg	FILE	*fp;
	reg	int	lcnt = 0;
	auto	char	buf[BSZ];
	auto	Entry	*mend;

	ep = (Entry *) calloc(sizeof(Entry), NENT);
	if(ep == NULL) {
		fprintf(stderr, "no memory for magic table\n");
		exit(2);
	}
	mtab = ep;
	mend = &mtab[NENT];
	fp = fopen(mfile, "r");
	if(fp == NULL) {
		fprintf(stderr, "cannot open magic file <%s>.\n", mfile);
		exit(2);
	}
	while(fgets(buf, BSZ, fp) != NULL) {
		reg	char	*p = buf;
		reg	char	*p2;
		reg	char	opc;

		if(*p == '\n' || *p == '#')
			continue;
		lcnt++;
			

			/* LEVEL */
		if(*p == '>') {
			ep->e_level = 1;
			p++;
		}
			/* OFFSET */
		p2 = strchr(p, '\t');
		if(p2 == NULL) {
			if(cflg)
				fprintf(stderr, "fmt error, no tab after %son line %d\n", p, lcnt);
			continue;
		}
		*p2++ = NULL;
		ep->e_off = atolo(p);
		while(*p2 == '\t')
			p2++;
			/* TYPE */
		p = p2;
		p2 = strchr(p, '\t');
		if(p2 == NULL) {
			if(cflg)
				fprintf(stderr, "fmt error, no tab after %son line %d\n", p, lcnt);
			continue;
		}
		*p2++ = NULL;
		if(*p == 's') {
			if(*(p+1) == 'h')
				ep->e_type = SHORT;
			else
				ep->e_type = STR;
		} else if (*p == 'l')
			ep->e_type = LONG;
		while(*p2 == '\t')
			*p2++;
			/* OP-VALUE */
		p = p2;
		p2 = strchr(p, '\t');
		if(p2 == NULL) {
			if(cflg)
				fprintf(stderr, "fmt error, no tab after %son line %d\n", p, lcnt);
			continue;
		}
		*p2++ = NULL;
		if(ep->e_type != STR) {
			opc = *p++;
			switch(opc) {
			case '=':
				ep->e_opcode = EQ;
				break;

			case '>':
				ep->e_opcode = GT;
				break;

			case '<':
				ep->e_opcode = LT;
				break;

			case 'x':
				ep->e_opcode = ANY;
				break;

			default:
				p--;
			}
		}
		if(ep->e_opcode != ANY) {
			if(ep->e_type != STR)
				ep->e_value.num = atolo(p);
			else {
				ep->e_value.str = malloc(strlen(p) + 1);
				strcpy(ep->e_value.str, p);
			}
		}
		while(*p2 == '\t')
			*p2++;
			/* STRING */
		ep->e_str = malloc(strlen(p2) + 1);
		p = ep->e_str;
		while(*p2 != '\n') {
			if(*p2 == '%')
				ep->e_opcode |= SUB;
			*p++ = *p2++;
		}
		*p = NULL;
		ep++;
		if(ep >= mend) {
			fprintf(stderr, "file: magic tab overflow - increase NENT in file.c.\n");
			exit(2);
		}
	}
	ep->e_off = -1L;
}

long
atolo(s)
reg	char	*s;
{
	reg	char	c;
	reg	char	*fmt = "%ld";
	auto	long	j = 0L;

	if(*s == '0') {
		s++;
		if(*s == 'x') {
			s++;
			fmt = "%lx";
		} else
			fmt = "%lo";
	}
	sscanf(s, fmt, &j);
	return(j);
}

ckmtab()
{

	reg	Entry	*ep;
	reg	char	*p;
	reg	int	lev1 = 0;
	auto	long	val;
	static	char	init = 0;

	/*
	|| Special kludge to handle 407 files.  The magic file
	|| isn't flexible enough to express the difference between
	|| final 407 files (produced by cc --n) and intermediate .o
	|| files.
	*/
#define	Long(ptr, idx)	(* ( (long *) & ((long *) ptr)[idx] ))
	if (Long(fbuf, 0) == 0407) {
		char *desc;
		/*
		|| Decide whether to call it 'relocatable' or
		|| 'executable' based on whether there is any
		|| text or data relocation info.
		*/
		if (Long(fbuf, 5) != 0 || Long(fbuf, 6) != 0)
			desc = "relocatable";
		else
			desc = "executable";
		printf("%s", desc);
		if (Long(fbuf, 4) > 0)
			printf(" not stripped");
		putchar('\n');
		return(1);
	}

	if(!init) {
		mkmtab(0);
		init = 1;
	}
	for(ep = mtab; ep->e_off != -1L; ep++) {
		if(lev1) {
			if(ep->e_level != 1)
				break;
			putchar(' ');
		} else if(ep->e_level == 1)
			continue;
		p = &fbuf[ep->e_off];
		switch(ep->e_type) {
		case STR:
		{
			if(strncmp(p,ep->e_value.str,strlen(ep->e_value.str)))
				continue;
			if(ep->e_opcode & SUB)
				printf(ep->e_str, ep->e_value.str);
			else
				printf(ep->e_str);
			lev1 = 1;
		}

		case BYTE:
			val = (long)(*(char *) p);
			break;

		case SHORT:
			val = (long)(*(short *) p);
			break;

		case LONG:
			val = (*(long *) p);
			break;
		}
		switch(ep->e_opcode & ~SUB) {
		case EQ:
			if(val != ep->e_value.num)
				continue;
			break;
		case GT:
			if(val <= ep->e_value.num)
				continue;
			break;

		case LT:
			if(val >= ep->e_value.num)
				continue;
			break;
		}
		if(ep->e_opcode & SUB)
			printf(ep->e_str, val);
		else
			printf(ep->e_str);
		lev1 = 1;
	}
	if(lev1) {
		putchar('\n');
		return(1);
	}
	return(0);
}

lookup(tab)
reg	char **tab;
{
	reg	char	r;
	reg	int	k,j,l;

	while(fbuf[i] == ' ' || fbuf[i] == '\t' || fbuf[i] == '\n')
		i++;
	for(j=0; tab[j] != 0; j++) {
		l = 0;
		for(k=i; ((r=tab[j][l++]) == fbuf[k] && r != '\0');k++);
		if(r == '\0')
			if(fbuf[k] == ' ' || fbuf[k] == '\n' || fbuf[k] == '\t'
			    || fbuf[k] == '{' || fbuf[k] == '/') {
				i=k;
				return(1);
			}
	}
	return(0);
}

ccom()
{
	reg	char	cc;

	while((cc = fbuf[i]) == ' ' || cc == '\t' || cc == '\n')
		if(i++ >= fbsz)
			return(0);
	if(fbuf[i] == '/' && fbuf[i+1] == '*') {
		i += 2;
		while(fbuf[i] != '*' || fbuf[i+1] != '/') {
			if(fbuf[i] == '\\')
				i += 2;
			else
				i++;
			if(i >= fbsz)
				return(0);
		}
		if((i += 2) >= fbsz)
			return(0);
	}
	if(fbuf[i] == '\n')
		if(ccom() == 0)
			return(0);
	return(1);
}

ascom()
{
	while(fbuf[i] == '/') {
		i++;
		while(fbuf[i++] != '\n')
			if(i >= fbsz)
				return(0);
		while(fbuf[i] == '\n')
			if(i++ >= fbsz)
				return(0);
	}
	return(1);
}

english (bp, n)
char *bp;
{
#	define NASC 128
	reg	int	j, vow, freq, rare;
	reg	int	badpun = 0, punct = 0;
	auto	int	ct[NASC];

	if (n<50)
		return(0); /* no point in statistics on squibs */
	for(j=0; j<NASC; j++)
		ct[j]=0;
	for(j=0; j<n; j++)
	{
		if (bp[j]<NASC)
			ct[bp[j]|040]++;
		switch (bp[j])
		{
		case '.': 
		case ',': 
		case ')': 
		case '%':
		case ';': 
		case ':': 
		case '?':
			punct++;
			if(j < n-1 && bp[j+1] != ' ' && bp[j+1] != '\n')
				badpun++;
		}
	}
	if (badpun*5 > punct)
		return(0);
	vow = ct['a'] + ct['e'] + ct['i'] + ct['o'] + ct['u'];
	freq = ct['e'] + ct['t'] + ct['a'] + ct['i'] + ct['o'] + ct['n'];
	rare = ct['v'] + ct['j'] + ct['k'] + ct['q'] + ct['x'] + ct['z'];
	if(2*ct[';'] > ct['e'])
		return(0);
	if((ct['>']+ct['<']+ct['/'])>ct['e'])
		return(0);	/* shell file test */
	return (vow*5 >= n-ct[' '] && freq >= 10*rare);
}
