char _Origin_[] = "System V";

/*	@(#)dd.c	1.2	*/
/*
**	convert and copy
*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/dd.c,v 1.1 89/03/27 14:50:19 root Exp $";
/*
 * $Log:	dd.c,v $
 * Revision 1.1  89/03/27  14:50:19  root
 * Initial check-in for 3.7
 * 
 * Revision 1.4  85/04/15  10:00:20  bob
 * Fixed to detect when request for memory fails.
 * 
 * Revision 1.3  85/04/11  15:19:04  bob
 * Enhanced to use lseek() to implement seek option on input files that are
 * regular files.
 * 
 * Revision 1.2  85/03/22  13:20:54  bob
 * Fixed so that bs=1b will use a block size of 512, not BSIZE which changed to
 * 1024.
 * 
 */

#ifndef	STANDALONE
#include	<stdio.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#else	STANDALONE
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/inode.h>
#include	"saio.h"
#endif	STANDALONE
#include	<sys/sysmacros.h>

#define	LCASE	01
#define	UCASE	02
#define	SWAB	04
#define NERR	010
#define SYNC	020
#ifdef STANDALONE
#define	perror(a)		printf("%s\n", a)
#endif STANDALONE

unsigned ibs = 512;
unsigned obs = 512;
unsigned bs;
unsigned cbs;
unsigned ibc;
unsigned obc;
unsigned cbc;

int	fflag;
int	cflag;
int	nifr;
int	nipr;
int	nofr;
int	nopr;
int	ntrunc;
int	ibf;
int	obf;
int	skip;
int	seekn;
int	count;
int	nspace;
char	multin;
char	multout;

char	*string;
#ifdef STANDALONE
#define	MAXBS	32768
char	line[100];
char	inbuffer[MAXBS];
char	outbuffer[MAXBS];
#else	STANDALONE
struct	stat	stat_em;
#endif STANDALONE
char	*ifile;
char	*ofile;
char	*ibuf;
char	*obuf;
char	*sbrk();
char	*op;

/* This is an EBCDIC to ASCII conversion table
*/
char etoa [] =
{
	0000,0001,0002,0003,0234,0011,0206,0177,
	0227,0215,0216,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0235,0205,0010,0207,
	0030,0031,0222,0217,0034,0035,0036,0037,
	0200,0201,0202,0203,0204,0012,0027,0033,
	0210,0211,0212,0213,0214,0005,0006,0007,
	0220,0221,0026,0223,0224,0225,0226,0004,
	0230,0231,0232,0233,0024,0025,0236,0032,
	0040,0240,0241,0242,0243,0244,0245,0246,
	0247,0250,0325,0056,0074,0050,0053,0174,
	0046,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0041,0044,0052,0051,0073,0176,
	0055,0057,0262,0263,0264,0265,0266,0267,
	0270,0271,0313,0054,0045,0137,0076,0077,
	0272,0273,0274,0275,0276,0277,0300,0301,
	0302,0140,0072,0043,0100,0047,0075,0042,
	0303,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0304,0305,0306,0307,0310,0311,
	0312,0152,0153,0154,0155,0156,0157,0160,
	0161,0162,0136,0314,0315,0316,0317,0320,
	0321,0345,0163,0164,0165,0166,0167,0170,
	0171,0172,0322,0323,0324,0133,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0135,0346,0347,
	0173,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0350,0351,0352,0353,0354,0355,
	0175,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0356,0357,0360,0361,0362,0363,
	0134,0237,0123,0124,0125,0126,0127,0130,
	0131,0132,0364,0365,0366,0367,0370,0371,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0372,0373,0374,0375,0376,0377,
};
/* This is an ASCII to EBCDIC conversion table
*/
char atoe [] =
{
0000,0001,0002,0003,0067,0055,0056,0057,
0026,0005,0045,0013,0014,0015,0016,0017,
0020,0021,0022,0023,0074,0075,0062,0046,
0030,0031,0077,0047,0034,0035,0036,0037,
0100,0132,0177,0173,0133,0154,0120,0175,
0115,0135,0134,0116,0153,0140,0113,0141,
0360,0361,0362,0363,0364,0365,0366,0367,
0370,0371,0172,0136,0114,0176,0156,0157,
0174,0301,0302,0303,0304,0305,0306,0307,
0310,0311,0321,0322,0323,0324,0325,0326,
0327,0330,0331,0342,0343,0344,0345,0346,
0347,0350,0351,0255,0340,0275,0232,0155,
0171,0201,0202,0203,0204,0205,0206,0207,
0210,0211,0221,0222,0223,0224,0225,0226,
0227,0230,0231,0242,0243,0244,0245,0246,
0247,0250,0251,0300,0117,0320,0137,0007,
0040,0041,0042,0043,0044,0025,0006,0027,
0050,0051,0052,0053,0054,0011,0012,0033,
0060,0061,0032,0063,0064,0065,0066,0013,
0070,0071,0072,0073,0004,0024,0076,0341,
0101,0102,0103,0104,0105,0106,0107,0110,
0111,0121,0122,0123,0124,0125,0126,0127,
0130,0131,0142,0143,0144,0145,0146,0147,
0150,0151,0160,0161,0162,0163,0164,0165,
0166,0167,0170,0200,0212,0213,0214,0215,
0216,0217,0220,0152,0233,0234,0235,0236,
0237,0240,0252,0253,0254,0112,0256,0257,
0260,0261,0262,0263,0264,0265,0266,0267,
0270,0271,0272,0273,0274,0241,0276,0277,
0312,0313,0314,0315,0316,0317,0332,0333,
0334,0335,0336,0337,0352,0353,0354,0355,
0356,0357,0372,0373,0374,0375,0376,0377,
};
char	atoibm[] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};

#ifndef STANDALONE
int (*signal())();
#endif STANDALONE

/* ARGSUSED */
main(argc, argv)
char **argv;
{
	register char *ip;
	register c;
	int	(*conv)();
	int	ebcdic(), ibm(), ascii(), null(), cnull(), term();
	int	a;
	int	rc;

#ifndef STANDALONE
	conv = null;
	for(c=1; c<argc; c++) {
		string = argv[c];
#else STANDALONE
loop:
	conv = null;
	finit();
	printf("\n\nStandalone dd\n");
	ibs = obs = BSIZE;
	bs = cbs = skip = seekn = count = fflag = 0;
	ifile = ofile = (char *)0;
	nifr = nipr = nofr = nopr = ntrunc = nspace = 0;
#endif STANDALONE
		if(match("ibs=")) {
			ibs = number();
#ifndef STANDALONE
			continue;
#else STANDALONE
			if (ibs > MAXBS) {
				ibs = MAXBS;
				printf("ibs=%d  (maximum allowed)\n", MAXBS);
			}
#endif STANDALONE
		}
		if(match("obs=")) {
			obs = number();
#ifndef STANDALONE
			continue;
#else STANDALONE
			if (obs > MAXBS) {
				obs = MAXBS;
				printf("obs=%d  (maximum allowed)\n", MAXBS);
			} else if (obs % PBSIZE) {
				obs = (obs >> PBSHIFT) << PBSHIFT;
				printf("obs=%d (must be multiple of %d)\n",
					obs, PBSIZE);
			}
#endif STANDALONE
		}
		if(match("cbs=")) {
			cbs = number();
#ifndef STANDALONE
			continue;
#endif STANDALONE
		}
		if (match("bs=")) {
			bs = number();
#ifndef STANDALONE
			continue;
#else STANDALONE
			if (bs > MAXBS) {
				bs = MAXBS;
				printf("bs=%d  (maximum allowed)\n", MAXBS);
			} else if (bs % PBSIZE) {
				bs = (bs >> PBSHIFT) << PBSHIFT;
				printf("bs=%d (must be multiple of %d)\n",
					bs, PBSIZE);
			}
#endif STANDALONE
		}
		if(match("if=")) {
#ifndef STANDALONE
			ifile = string;
			continue;
#else STANDALONE
			ifile = (char *)strncpy(inbuffer, string, 100);
#endif STANDALONE
		}
		if(match("of=")) {
#ifndef STANDALONE
			ofile = string;
			continue;
#else STANDALONE
			ofile = (char *)strncpy(outbuffer, string, 100);
#endif STANDALONE
		}
		if(match("skip=")) {
			skip = number();
#ifndef STANDALONE
			continue;
#endif STANDALONE
		}
		if(match("seek=")) {
			seekn = number();
#ifndef STANDALONE
			continue;
#endif STANDALONE
		}
		if(match("count=")) {
			count = number();
#ifndef STANDALONE
			continue;
#endif STANDALONE
		}
#ifdef STANDALONE
		cflag = 0;
#endif STANDALONE
		if(match("conv=")) {
		cloop:
#ifndef STANDALONE
#define	ddmatch(s)	match(s)
#else STANDALONE
/* this should parse the string just typed in */
#define	ddmatch(s)	((c=(strncmp(s, string, strlen(s)) == 0)), string+=strlen(s), c)
#endif STANDALONE
			if(ddmatch(","))
				goto cloop;
			if(*string == '\0')
#ifndef STANDALONE
				continue;
#else STANDALONE
				goto endcloop;
#endif STANDALONE
			if(ddmatch("ebcdic")) {
				conv = ebcdic;
				goto cloop;
			}
			if(ddmatch("ibm")) {
				conv = ibm;
				goto cloop;
			}
			if(ddmatch("ascii")) {
				conv = ascii;
				goto cloop;
			}
			if(ddmatch("lcase")) {
				cflag |= LCASE;
				goto cloop;
			}
			if(ddmatch("ucase")) {
				cflag |= UCASE;
				goto cloop;
			}
			if(ddmatch("swab")) {
				cflag |= SWAB;
				goto cloop;
			}
			if(ddmatch("noerror")) {
				cflag |= NERR;
				goto cloop;
			}
			if(ddmatch("sync")) {
				cflag |= SYNC;
				goto cloop;
			}
		}
#ifdef STANDALONE
endcloop:
		multin = multout = 0;
#endif STANDALONE
		if(match("multi=")) {
		mloop:
			if(ddmatch(","))
				goto mloop;
			if(*string == '\0')
#ifndef STANDALONE
				continue;
#else STANDALONE
				goto endmloop;
#endif STANDALONE
			if(ddmatch("in")) {
				multin++;
				goto mloop;
			}
			if(ddmatch("out")) {
				multout++;
				goto mloop;
			}
		}
#ifndef STANDALONE
		fprintf(stderr,"dd: bad arg %s\n", string);
		exit(2);
	}
#else STANDALONE
endmloop:
#endif STANDALONE
	if(conv == null && cflag&(LCASE|UCASE))
		conv = cnull;
	if (ifile)
		ibf = open(ifile, 0);
	else
#ifndef STANDALONE
		ibf = dup(0);
#else STANDALONE
	{
		printf("must specify an input file\n");
		goto loop;
	}
#endif STANDALONE
	if(ibf < 0) {
#ifndef STANDALONE
		fprintf(stderr,"dd: cannot open %s\n", ifile);
		exit(2);
#else STANDALONE
		printf("cannot open %s\n", ifile);
		goto loop;
#endif STANDALONE
	}
	if (ofile)
#ifndef STANDALONE
		obf = creat(ofile, 0666);
	else
		obf = dup(1);
#else STANDALONE
		obf = open(ofile, 2);
	else {
		printf("must specify an output file\n");
		goto loop;
	}
#endif STANDALONE
	if(obf < 0) {
#ifndef STANDALONE
		fprintf(stderr,"dd: cannot create %s\n", ofile);
		exit(2);
#else STANDALONE
		printf("cannot open %s\n", ofile);
		goto loop;
#endif STANDALONE
	}
	if (bs) {
		ibs = obs = bs;
		if (conv == null)
			fflag++;
	}
	if(ibs == 0 || obs == 0) {
#ifndef STANDALONE
		fprintf(stderr,"dd: counts cannot be zero\n");
		exit(2);
#else STANDALONE
		printf("counts cannot be zero\n");
		goto loop;
#endif STANDALONE
	}
#ifndef STANDALONE
	ibuf = sbrk(ibs);
#else STANDALONE
	ibuf = inbuffer;
#endif STANDALONE
	if (fflag)
		obuf = ibuf;
	else
#ifndef STANDALONE
		obuf = sbrk(obs);
	sbrk((unsigned)64);	/* For good measure */
	if((int)ibuf == -1 || (int)obuf == -1) {
		fprintf(stderr,"dd: not enough memory\n");
		exit(2);
	}
#else STANDALONE
		obuf = outbuffer;
#endif STANDALONE
	ibc = 0;
	obc = 0;
	cbc = 0;
	op = obuf;

#ifndef STANDALONE
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, term);
			/* use lseek() for skip'ing ordinary files - SGI */
	if (fstat(ibf,&stat_em) >= 0 && (stat_em.st_mode&S_IFMT) == S_IFREG) {
		lseek(ibf,skip * (long) ibs,1);
		skip = 0;
	} else
#endif
		while(skip) {
			(void)read(ibf, ibuf, (int)ibs);
			skip--;
		}
#ifdef STANDALONE
	for (c=0; c<obs; c++)
		outbuffer[c] = 0;
	c = 0;
#endif STANDALONE
	while(seekn) {
#ifndef STANDALONE
		lseek(obf, (long)obs, 1);
#else STANDALONE
		if (lseek(obf, (long)(c*obs), 0) < 0) {
			printf("lseek failed\n");
			goto loop;
		}
		if (write(obf, outbuffer, (int)obs) != obs) {
			printf("write failed\n");
			goto loop;
		}
		c++;
#endif STANDALONE
		seekn--;
	}

	while(1) {
		if(ibc-- == 0) {
			rc = 0;
			if(count == 0 || nifr+nipr != count) {
				if(cflag&(NERR|SYNC))
				for(ip=ibuf+ibs; ip>ibuf;)
					*--ip = 0;
				rc = read(ibf, ibuf, (int)ibs);
			} else
				multin = 0;
			if(rc == -1) {
				perror("dd read error");
				if((cflag&NERR) == 0) {
					flsh();
					term(2);
				}
				ibc = 0;
				for(c=0; c<ibs; c++)
					if(ibuf[c] != 0)
						ibc = c;
				stats();
			} else
				ibc = rc;
			if(ibc == 0) {
				flsh();
				if (!multin || !more(&ibf, ifile, 0)) {
#ifndef STANDALONE
					term(0);
#else STANDALONE
					stats();
					goto loop;
#endif STANDALONE
				}
				rc = read(ibf, ibuf, (int)ibs);
				if (rc <= 0) {
					perror("dd read error");
					if((cflag&NERR) == 0) {
						flsh();
						term(2);
					}
					ibc = 0;
					for(c=0; c<ibs; c++)
						if(ibuf[c] != 0)
							ibc = c;
					stats();
				} else
					ibc = rc;
			}
			if(ibc != ibs) {
				nipr++;
				if(cflag&SYNC)
					ibc = ibs;
			} else
				nifr++;
			ip = ibuf;
			if (ibc % 2) ibuf[ibc] = 0;
			c = (ibc + 1) >> 1;
			if(cflag&SWAB && c)
				do {
					a = *ip++;
					ip[-1] = *ip;
					*ip++ = a;
				} while(--c);
			ip = ibuf;
			if (fflag) {
				obc = ibc;
				flsh();
				ibc = 0;
			}
			continue;
		}
		c = 0;
		c |= *ip++;
		(*conv)(c);
	}
}

flsh()
{
	register c;

	if(obc) {
		if(obc == obs)
			nofr++;
		else
			nopr++;
#ifdef STANDALONE
		for (; (obc % PBSIZE); obc++)
			obuf[obc] = 0;
#endif STANDALONE
		c = write(obf, obuf, (int)obc);
		if(c != obc) {
			if (!multout || !more(&obf, ofile, 2)) {
				perror("dd write error");
				term(2);
			}
			c = write(obf, obuf, (int)obc);
			if (c != obc) {
				perror("dd write error");
				term(2);
			}
		}
		obc = 0;
	}
}

match(s)
char *s;
{
	register char *cs;

#ifndef STANDALONE
	cs = string;
	while(*cs++ == *s)
		if(*s++ == '\0')
			goto true;
	if(*s != '\0')
		return(0);
#else STANDALONE
	printf(s);
	gets(line);
	if (line[0] == '\0')
		return(0);
	cs = line + 1;
	goto true;	/* for LINT */
#endif STANDALONE

true:
	cs--;
	string = cs;
	return(1);
}

number()
{
	register char *cs;
	register n;

	cs = string;
	n = 0;
	while(*cs >= '0' && *cs <= '9')
		n = n*10 + *cs++ - '0';
	for(;;)
	switch(*cs++) {

	case 'k':
		n *= 1024;
		continue;

	case 'w':
		n *= 2;
		continue;

	case 'b':
		n *= 512;
		continue;

	case '*':
	case 'x':
		string = cs;
		n *= number();

	case '\0':
		return(n);
	}
	/* never gets here */
}

more(fd, fname, mode)
int *fd;
char *fname;
int mode;
{
	if (close(*fd) < 0)
		return(0);
	printf("\nat end of %s file - type return to continue  ",
		(mode==0) ? "input":"output");
	(void) getchar();
	if ((*fd = open(fname, mode)) < 0)
		return(0);
	return(1);
}

cnull(cc)
{
	register c;

	c = cc;
	if(cflag&UCASE && c>='a' && c<='z')
		c += 'A'-'a';
	if(cflag&LCASE && c>='A' && c<='Z')
		c += 'a'-'A';
	null(c);
}

null(c)
{

	*op = c;
	op++;
	if(++obc >= obs) {
		flsh();
		op = obuf;
	}
}

ascii(cc)
{
	register c;

	c = 0;
	c |= etoa[0377 & cc];
	if(cbs == 0) {
		cnull(c);
		return;
	}
	if(c == ' ')
		nspace++;
	else {
		while(nspace > 0) {
			null(' ');
			nspace--;
		}
		cnull(c);
	}
	if(++cbc >= cbs) {
		null('\n');
		cbc = 0;
		nspace = 0;
	}
}

ebcdic(cc)
{
	register c;

	c = cc;
	if(cflag&UCASE && c>='a' && c<='z')
		c += 'A'-'a';
	if(cflag&LCASE && c>='A' && c<='Z')
		c += 'a'-'A';
	c = atoe[c] & 0377;
	if(cbs == 0) {
		null(c);
		return;
	}
	if(cc == '\n') {
		while(cbc < cbs) {
			null(atoe[' ']);
			cbc++;
		}
		cbc = 0;
		return;
	}
	if(cbc == cbs)
		ntrunc++;
	cbc++;
	if(cbc <= cbs)
		null(c);
}

ibm(cc)
{
	register c;

	c = cc;
	if(cflag&UCASE && c>='a' && c<='z')
		c += 'A'-'a';
	if(cflag&LCASE && c>='A' && c<='Z')
		c += 'a'-'A';
	c = atoibm[c] & 0377;
	if(cbs == 0) {
		null(c);
		return;
	}
	if(cc == '\n') {
		while(cbc < cbs) {
			null(atoibm[' ']);
			cbc++;
		}
		cbc = 0;
		return;
	}
	if(cbc == cbs)
		ntrunc++;
	cbc++;
	if(cbc <= cbs)
		null(c);
}

/* ARGSUSED */
term(c)
{

	stats();
#ifndef STANDALONE
	exit(c);
#else STANDALONE
	exit();
#endif STANDALONE
}

stats()
{
#ifndef STANDALONE
	fprintf(stderr,"%u+%u records in\n", nifr, nipr);
	fprintf(stderr,"%u+%u records out\n", nofr, nopr);
	if(ntrunc)
		fprintf(stderr,"%u truncated records\n", ntrunc);
#else STANDALONE
	printf("%u+%u records in\n", nifr, nipr);
	printf("%u+%u records out\n", nofr, nopr);
	if(ntrunc)
		printf("%u truncated records\n", ntrunc);
#endif STANDALONE
}
