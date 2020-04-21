/*	nprom: program for burning proms
 *
 * modification history:
 *
 *	..
 *
 *	5/13/85	(bert) changed test for "too much data"
 */
#define PROMFILE "/dev/prom"
#define	SIZE	(0xFFFF+1)	/* Total bytes of Prom memory */
# define STX 02
# define ETX 03
# include <sgtty.h>
# include <stdio.h>
# include <ctype.h>

struct sgttyb vec;
struct tchars tchar;
extern int tty_ld;

FILE	*fpr, *fpw;
int	dp;
int	junk;
unsigned char data[SIZE];
char	*hex();
int dflg;		/* Debug output */
int vflg;		/* Verbose flag */

extern struct trans {
	char	*type;
	unsigned short	code;
} tab[];

main (argc,argv)
int argc;
char *argv[];

{
	struct trans *pt;
	char	*dname;
	unsigned short	code, codeset = 0;
	unsigned short bdmsk;
	int	i, r;
	int	ndata, devmax, maxa, width, v, fsn;
	short	sum, osum;
	int xfmt = 0x50;	/* Binary */
	int start = 0;		/* address offset */
	int bflg = 0;		/* blankcheck flag */
	int nflg = 0;		/* no, echo family&pinout */
	int ssize = 0;		/* Set Size */
	int wsize = 0;		/* Word Size */

	/* scan command line for arguments */

	while( argc > 1 ){
		if( argv[1][0] != '-' ) {
			fprintf(stderr, "Bad option %s, not a switch\n",
				&argv[1][0]);
			exit(1);
		}
		switch (argv[1][1]) {
		case 't':
			if(codeset) {
		set:		fprintf(stderr,"Can only set code once\n");
				exit(1);
			}
			dname = &argv[1][2];
			for(pt = tab; pt->type[0] != '\0'; pt++) {
				if(equstr(pt->type,dname)) {
					code = pt->code;
					break;
				}
			}
			if(pt->type[0] == '\0') {
				fprintf(stderr,"Unknown type %s\n",dname);
				fprintf(stderr, "Try one of:\n");
				for(pt = tab; pt->type[0] != '\0'; ) {
					fprintf(stderr,"%s, ",pt->type);
					if((++pt - tab)%7 == 0)
						fprintf(stderr,"\n");
				}
				fprintf(stderr,"\n");
				exit(1);
			}
			codeset++;
			break;
		case 'c':
			if(codeset)
				goto set;
			sscanf(&argv[1][2],"%x",&code);
			codeset++;
			break;
		case 's':
			sscanf(&argv[1][2],"%d",&start);
			break;
		case 'b':
			bflg++; break;
		case 'n':
			nflg++; break;
		case 'd':
			dflg++; break;
		case 'v':
			vflg++; break;
		case 'S':
			sscanf(&argv[1][2], "%d", &ssize);
			if(!wsize) wsize = 16;
			break;
		case 'W':
			sscanf(&argv[1][2], "%d", &wsize);
			break;
		default:
			fprintf(stderr,"Unknown option: %s\n", argv[1]);
			fprintf(stderr,
"Usage: nprom [-tss][-cxx][-s##][-v][-n][-d][-b][-S##][-W##] < bindata\n"
			);
			exit(1);
		}
		++argv;
		--argc;
	}
	if(nflg||dflg||vflg)
		printf("family&pinout = %#x\n",code);

	dp = open(PROMFILE, 2);
	if (dp < 0) {
		fprintf(stderr, "cannot open promfile %s\n", PROMFILE);
		exit(1);
	}
	vec.sg_ispeed = B9600;
	vec.sg_ospeed = B9600;
	vec.sg_flags = ANYP|TANDEM|CRMOD;
	ioctl(dp, TIOCSETP, &vec);
	if((fpr = fdopen(dp, "r")) == NULL) {
		fprintf(stderr, "Cannot fdopen, fpr");
		exit(1);
	}
	if((fpw = fdopen(dp, "w")) == NULL) {
		fprintf(stderr, "Cannot fdopen, fpw");
		exit(1);
	}
	dprintf(fpw, "H\r");		aprmt("initial prompt");
	dprintf(fpw, "%2.2XA\r", xfmt);	aprmt("Setting format");
	dprintf(fpw, "%4.4X;\r",0);	aprmt("Setting block size");
	dprintf(fpw, "%4.4X:\r",0);	aprmt("Setting device addr");
	if(codeset) {
		dprintf(fpw, "%s@\r", hex(code, 4));
		aprmt("Setting family&pinout");
	}
	if(ssize) {
		dprintf(fpw, "E1]\r1\r");  aprmt("Setting set size1");
		dprintf(fpw, "E2]\r08\r"); aprmt("Setting word size1");
		dprintf(fpw,"R\r");
		fflush(fpw);
		if((fsn = fscanf(fpr,"%x/%x/%x",&devmax,&width,&v)) != 3) {
			fprintf(stderr,"Cannot read R response1\n");
			exit(1);
		}
		aprmt("Reading device size1");

		if(vflg) printf("Devmax=%x\n", devmax);

		dprintf(fpw, "E1]\r%d\r", ssize);
				aprmt("Setting set size2");
		dprintf(fpw, "E2]\r%2.2D\r", wsize);
				aprmt("Setting word size2");
	}
	dprintf(fpw,"R\r");
	fflush(fpw);
	if((fsn = fscanf(fpr,"%x/%x/%x",&maxa,&width,&v)) != 3) {
		fprintf(stderr,"Cannot read R response2\n");
		exit(1);
	}
	aprmt("Reading device size2");
	if(nflg||dflg||vflg) {
		printf("maxaddr = 0x%x, width = %d, unprog'd state = %d\n",
			maxa, width, v);
		if(nflg) exit(0);
	}
	maxa = 0xFFFF;
	if(maxa > SIZE) {
		fprintf(stderr,"Recompile with SIZE = %d",maxa+1);
		exit(1);
	}
	if(start >= maxa) {
	    fprintf(stderr,"Start (%d) exceeds device end (%d)\n",start,maxa);
	    exit(1);
	}
	ndata = read(0, data, sizeof data);
	if(ssize)
		for(i = (ndata+devmax)&~devmax; ndata < i; )
			data[ndata++] = v?0xFF:0; /* Fill end with 1's or 0's */
	if(ndata*8 > maxa * width) {
		printf("Too much data: %x*8 > %x*%x\n", ndata, maxa, width);
		printf("Devmax: %x\n", devmax);
	/*	exit(1);	/***/
	}
	dprintf(fpw,"0000<\r"); aprmt ("Setting addr of ram");
	dprintf(fpw,"%s;\r", hex(ndata, 4));
				aprmt("setting device size");
	dprintf(fpw,"%s:\r", hex(start, 4));
				aprmt("setting device start address");
	dprintf(fpw,"I\r%c$A0000,\r",STX);
	sum = 0;
	for(i = 0; i <ndata;i++) {
		fprintf(fpw,"%s ", hex(data[i], 2));
		if(dflg&&vflg) {
			printf("%s ", hex(data[i], 2));
			if(i && (i& 0xF)==0) {
				printf("\n");
				chkrd();
			}
		} else {
			if((dflg || vflg) && i && (i&0x3ff)==0) {
				printf("%x ", i); fflush(stdout);
			}
			chkrd();
		}
		sum += data[i];
	}
	dprintf(fpw,"%c$S%s,",ETX, hex(sum, 4));
				aprmt("DATA I/O checksum");
	printf("\ndata loaded in. now testing and burning device\n");
	if(ssize) {
		dprintf(fpw,"%s;\r", "0000"/*hex(devmax+1, 4)*/);
				aprmt("Resetting device size");
	}
	if(!ssize) {
		dprintf(fpw,"T\r");	aprmt("illegal bit test");
	}
	if (bflg) {			/* Run blank check */
		dprintf(fpw,"B\r");	aprmt("blank check");
	}
					/* BURN IT! */
	dprintf(fpw,"P\r");	aprmt("Programing");
					/* Verify it */
	dprintf(fpw,"V\r");	aprmt("Verify");
	if(ssize) {
		dprintf(fpw, "E1]\r%d\r", 1);
				aprmt("Resetting set size1");
		dprintf(fpw, "E2]\r08\r");
				aprmt("Resetting word size1");
	}
	printf("\n");
	exit(0);
}

aprmt(s)
char *s;
{
	char c;
	long ercd;

	fflush(fpw);
	while(((c = xgetc(fpr)) == '\n') || (c == '\r'));
	if (c == '>')
		if((c = xgetc(fpr)) == '\n' || c == '\r')
			return;
	fflush(stdout);
	fprintf(stderr, "Failure: ");
	fprintf(stderr,"%s\n",s);
	if(c != 'F')
		fprintf(stderr,"Bad error char was %c:%o\n",c,c);
	fprintf(fpw,"F\r");
	fflush(fpw);
	if(fscanf(fpr,"%lx",&ercd) != 1) {
		fprintf(stderr,"couldn't read error code\n");
		exit(1);
	}
	fprintf (stderr,"Error code: %lx", ercd);
	fprintf(fpw,"X\r");
	fflush(fpw);
	fprintf(stderr,"%c",getc(fpr));
	fprintf(stderr,"%c",getc(fpr));
	fprintf(stderr,"%c",getc(fpr));
	while(c=getc(fpr)) {
		fprintf(stderr,"%c",c);
		if(c == '\n')
			break;
	}
	exit (1);
}

xgetc(f)
FILE *f;
{
	long cnt;
	register c;
	static lastc;

	ioctl(fileno(f), FIONREAD, cnt);
	if(cnt == 0) {
		fprintf(stderr, "xgetc called with no chars to read\n");
		exit(1);
	}
	c = getc(f);
	if(dflg)
		putchar(c);
	return c;
}

equstr(s1, s2)
	char	*s1, *s2;
{
	while(*s1++ == *s2) {
		if(*s2++==0) {
			return(1);
		}
	}
	return(0);
}

char *hexchars = "0123456789ABCDEF";

char *hex(n, w)
register unsigned short n;
register w;
{
	static char buf[32];
	register char *cp = &buf[32];

	*--cp = 0;
	while(w--) {
		*--cp = hexchars[n & 0xF];
		n >>= 4;
	}
	return cp;
}

chkrd()
{
	long count, i;
	register c;

again:
	ioctl(fileno(fpr), FIONREAD, &count);
	if(count) {
		for(i = 0; i < count; i++)
			if((c = getc(fpr)) == 'F') {
				ungetc(c, fpr);
				fprintf(stderr, "chkrd got %d\n", count);
				aprmt("chkrd");
				exit(1);
			}
		goto again;
	}
}

dprintf(of, fmt, a1, a2)
FILE *of;
char *fmt;
{
	register char *cp;
	char buf[128];

	fprintf(of, fmt, a1, a2);
	if(dflg) {
		printf("++");
		sprintf(buf, fmt, a1, a2);
		for(cp = buf; *cp; cp++)
			if(isprint(*cp))
				putchar(*cp);
			else
				printf("{%x}", *cp);
		fflush(stdout);
	}
}
