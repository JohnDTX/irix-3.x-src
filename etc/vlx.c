char _Origin_[] = "System V";

/*	@(#)vlx.c	1.1	*/
/*
 * VAX 11/780 LSI RX01 archive program
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
struct rt_dat {
	ushort	rt_yr:5;	/* Year - 1972 */
	ushort	rt_dy:5;	/* day */
	ushort	rt_mo:5;	/* month */
};

struct rt_ent {
	char	pad;		/* unused */
	char	type;		/* Type of entry, or end of seg */
	ushort	fname[3];	/* Name, 3 words in rad50 form */
	short	fsiz;		/* Size of file */
	char	chan;		/* Only used in temporary files */
	char	job;		/* Only used in temporary files */
	struct rt_dat cdate;	/* Creation Date */
};
#define RT_TEMP 1
#define RT_NULL 2
#define RT_FILE 4
#define RT_ESEG 8
#define RTB	512
struct rt_head {
	short	numseg;	/* number of segments available */
	short	nxtseg;	/* segment no of next log. seg */
	short	lstseg;	/* highest seg currenltly open */
	ushort	entpad;	/* extra words/dir. entry - assume 0 */
	short	stfile;	/* block no where files begin */
};
struct	rt_dir {
	struct rt_head	head;
	struct rt_ent	entry[72];
	struct rt_ent	last[1];
} rt_dir[4];

typedef struct fldope {
	int	startad;
	int	count;
	struct	rt_ent	*rtdope;
} dope_t;
dope_t *lookup();

char	*man = "rxtd";
char zeroes[512];
extern char *val;
extern char table[256];
int	segcnt;
int	ffd;
char	*defdev = "/dev/conflp";
char	*opt = "vf";
long	lseek();
int	rcmd(), dcmd(), xcmd(), tcmd();
int	(*comfun)();
char	vflg;
char	**namv;
int	namc;
int	file;

main(argc, argv)
char *argv[];
{
	register char *cp;
	register i;

	if (argc < 2) {
		printf("usage: vlx %s%s [files ...]\n", opt, man);
		exit(1);
	}
	cp = argv[1];
	for (cp = argv[1]; *cp; cp++)
	switch(*cp) {
	case 'v':
		vflg++;
		continue;
	case 'r':
		setcom(rcmd);
		continue;
	case 'd':
		setcom(dcmd);
		continue;
	case 'x':
		setcom(xcmd);
		continue;
	case 't':
		setcom(tcmd);
		continue;
	case 'f':
		defdev = argv[2];
		argv++;
		argc--;
		continue;
	default:
		fprintf(stderr, "vlx: bad option `%c'\n", *cp);
		exit(1);
	}
	namv = argv+2;
	namc = argc-2;
	if (comfun == 0) {
		fprintf(stderr, "vlx: one of %s must be specified\n", man);
		exit(1);
	}
	if ((ffd = open(defdev,2)) < 0)
		fprintf(stderr, "Can not open floppy %s\n", defdev);
	lread(6*RTB, 2*RTB, &rt_dir[0]);
	segcnt = rt_dir[0].head.lstseg;
	if (segcnt > 4) {
		fprintf(stderr, "Directory too big\n");
		exit(2);
	}
	for (i = 1; i < segcnt; i++)
		lread((6+2*i)*RTB, 2*RTB, &rt_dir[i]);
	(*comfun)();
}

setcom(fun)
int (*fun)();
{

	if (comfun != 0) {
		fprintf(stderr, "vlx: only one of %s allowed\n", man);
		exit(1);
	}
	comfun = fun;
}

tcmd()
{
	dope_t *dope;
	register struct rt_ent *de;
	register i;

	if (namc==0) for (i = 0; i < segcnt; i++) {
		for (de = rt_dir[i].entry; de < rt_dir[i].last; de++)
			if (rtls(de))
				break;
	} else for (i = 0; i < namc; i++) {
		if (dope = lookup(namv[i]))
			rtls(dope->rtdope);
	}
}
rtls(de)
register struct rt_ent *de;
{
	int month,day,year;
	char name[12], ext[4];

	if (vflg)
		switch(de->type) {
		case RT_ESEG:
			return(1);
		case RT_TEMP:
			printf("Tempfile:\n");
		case RT_FILE:
			unrad50(2, de->fname, name);
			unrad50(1, &(de->fname[2]), ext);
			day = de->cdate.rt_dy;
			year = de->cdate.rt_yr + 72;
			month = de->cdate.rt_mo;
			printf("%6.6s  %3.3s	%.2d/%.2d/%.2d	%d\n",name,
				ext,month,day,year,de->fsiz);
			break;
		case RT_NULL:
			printf("%-25.9s	%d\n","<UNUSED>",de->fsiz);
			break;
		}
	else {
		switch(de->type) {
		case RT_ESEG:
			return(1);
		case RT_TEMP:
		case RT_FILE:
			sunrad50(name, de->fname);
			printf("%s\n",name);
			break;
		case RT_NULL:
			;
		}
	}
	return(0);
}

xcmd()
{
	register struct rt_ent *de;
	register i;
	char name[12];

	if (namc==0) for (i = 0; i < segcnt; i++) {
		for (de = rt_dir[i].entry; de < rt_dir[i].last; de++) {
			if (de->type == RT_NULL)
				continue;
			if (de->type == RT_ESEG)
				break;
			sunrad50(name, de->fname);
			rtx(name);
		}
	} else for (i = 0; i < namc; i++)
		rtx(namv[i]);
}

rtx(name)
char *name;
{
	register dope_t *dope;
	register startad, count;
	int file; char buff[512];

	if (dope = lookup(name)) {
		if (vflg)
			rtls(dope->rtdope);
		file = creat(name, 0666);
		if (file < 0) {
			printf("cannot create %s\n", name);
			return;
		}
		count = dope->count;
		startad = dope->startad;
		for ( ; count > 0 ; count -= 512) {
			lread(startad, 512, buff);
			write(file, buff, 512);
			startad += 512;
		}
		close(file);
	}
}

static dope_t result;
dope_t *
lookup(name)
char *name;
{
	register i;
	ushort rname[3];
	register struct rt_ent *de;
	register index;

	srad50(name, rname);
	/*
	 * Search for name, accumulate blocks in index
	 */
	for (i = 0; i < segcnt; i++) {
		index = 0;
		for (de = rt_dir[i].entry; de < rt_dir[i].last; de++) {
			if (de->type == RT_ESEG)
				break;
			switch(de->type) {
			case RT_FILE:
			case RT_TEMP:
				if (samename(rname,de->fname))
					goto found;
			case RT_NULL:
				index += de->fsiz;
			}
		}
	}
notfound:
	if (comfun != rcmd)
		fprintf(stderr, "vlx: %s not found\n", name);
	return((dope_t *) 0);
found:
	result.count = de->fsiz * 512;
	result.startad = 512 * (rt_dir[i].head.stfile + index);
	result.rtdope = de;
	return(&result);
}
static
samename(a,b)
ushort a[3],b[3];
{
	return( a[0]==b[0] && a[1]==b[1] && a[2]==b[2] );
}

rad50(cp, out)
register unsigned char *cp;
ushort *out;
{
	register index;
	register temp;

	for (index = 0;*cp; index++) {
		temp = (050*050) * table[*cp++];
		if (*cp) {
			temp += 050 * table[*cp++];
			if (*cp) 
				temp += table[*cp++];
		}
		out[index] = temp;
	}
}
#define reduce(x, p, q) \
	(x = v[p/q], p %= q);

unrad50(count, in, cp)
ushort *in;
register char *cp;
{
	register i, temp;
	register unsigned char *v = (unsigned char *)val;
	
	for (i = 0; i < count; i++) {
		temp = in[i];
		reduce(*cp++, temp, (050*050));
		reduce(*cp++, temp, 050);
		reduce(*cp++, temp, 1);
	}
	*cp=0;
}

srad50(name, rname)
register char * name;
register ushort *rname;
{
	register index; register char *cp;
	char file[7],ext[4];

	for (cp = name; *cp++; );
	while(cp >= name && *--cp != '/');
	cp++;
	/*
	 * Change to rad50
	 */
	for (index = 0; *cp; ) {
		file[index++] = *cp++;
		if (*cp=='.') {
			cp++;
			break;
		}
		if (index>=6) {
			break;
		}
	}
	file[index] = 0;
	for (index = 0; *cp; ) {
		ext[index++] = *cp++;
		if (*cp=='.' || index>=3) {
			break;
		}
	}
	ext[index]=0;
	rname[0] = 0;
	rname[1] = 0;
	rname[2] = 0;
	rad50((unsigned char *)file, rname);
	rad50((unsigned char *)ext, rname+2);
}
sunrad50(name, rname)
ushort rname[3];
register char *name;
{
	register char *cp, *cp2;
	char ext[4];

	unrad50(2, rname, name);
	unrad50(1, rname + 2, ext);
	/* Jam name and extension together with a dot
	  deleting white space */
	for (cp = name; *cp++;);--cp; while(*--cp==' ' && cp>=name);
	*++cp = '.';cp++;
	for (cp2=ext; *cp2!=' ' && cp2 < ext + 3;) {
		*cp++ = *cp2++;
	}
	*cp=0;
	if (cp[-1]=='.') cp[-1] = 0;
}

static char *oval = " ABCDEFGHIJKLMNOPQRSTUVWXYZ$.@0123456789";
static char *val = " abcdefghijklmnopqrstuvwxyz$.@0123456789";
static char table[256] = {
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
0, 29, 29, 29, 27, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 29, 
30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 29, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29, 29, 
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
0, 29, 29, 29, 27, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 29, 
30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 29, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29 };
		
lread(startad, count, obuff)
register startad, count;
register char * obuff;
{
	while( (count -= 512) >= 0) {
		lseek(ffd,(long) (startad), 0);
		read(ffd, obuff, 512);
		obuff += 512;
		startad += 512;
	}
}
lwrite(startad, count, obuff)
register startad, count;
register char * obuff;
{
	while( (count -= 512) >= 0) {
		lseek(ffd,(long) (startad), 0);
		write(ffd, obuff, 512);
		obuff += 512;
		startad += 512;
	}
}

rcmd()
{
	register int i;

	for (i = 0; i < namc; i++)
		rtr(namv[i]);
}

rtr(name)
char *name;
{
	register dope_t *dope;
	register struct rt_ent *de;
	register i;
	struct stat buf; register struct stat *bufp = &buf;

	if (stat(name, bufp) < 0) {
		fprintf(stderr, "File not found %s\n", name);
		return(1);
	}
	rtd(name);
	for (i = 0; i < segcnt; i++) {
		for (de = rt_dir[i].entry; de < rt_dir[i].last; de++) {
			if (de->type == RT_ESEG)
				break;
			if (de->type == RT_NULL) {
				if (bufp->st_size <= (de->fsiz * 512)) {
					if (vflg)
						printf("a - %s\n",name);
					mkent(de, bufp, name, i);
					if (dope = lookup(name)) {
						wflop(name,bufp->st_size,dope);
						return(0);
					}
					return(7);
				}
			}
		}
	}
	printf("%s too big for floppy\n", name);
	return(0);
}

mkent(de, bufp, name, sn)
register struct rt_ent *de;
register struct stat *bufp;
char *name;
{
	struct tm *localtime(); register struct tm *timp;
	register struct rt_ent *workp; int count;
	
	count = (bufp->st_size + 511) >> 9;
	if (count < de->fsiz) {
		for (workp = de; workp < rt_dir[sn].last; workp++)
			if (workp->type == RT_ESEG)
				break;
		workp++;
		if (workp >= rt_dir[sn].last) {
			fprintf(stderr, "Directory full on %s\n", defdev);
			return;
		}
		for (; workp > de; workp--)
			*workp = workp[-1];
		de[1].fsiz -= count;
		de->fsiz = count;
	}
	srad50(name, de->fname);
	timp = localtime(&bufp->st_mtime);
	de->cdate.rt_dy = timp->tm_mday + 1;
	de->cdate.rt_mo = timp->tm_mon + 1;
	de->cdate.rt_yr = timp->tm_year - 72;
	de->type = RT_FILE;
	de->pad = 0;
	de->chan = 0;
	de->job = 0;
	lwrite((6+2*sn)*RTB, 2*RTB, &rt_dir[sn]);
}

wflop(name,ocount,dope)
char *name;
register dope_t *dope;
long ocount;
{
	register file, n, startad = dope->startad, count = ocount;
	char buff[512];
	
	file = open(name,0);
	if (file < 0) {
		printf("vlx: couldn't open %s\n",name);exit(1);}
	for ( ; count >= 512; count -= 512) {
		read(file, buff, 512);
		lwrite(startad, 512, buff);
		startad += 512;
	}
	read(file, buff, count);
	close(file);
	if (count <= 0) return;
	for (n = count; n < 512; n ++) buff[n] = 0;
	lwrite(startad, 512, buff);
	count = (dope->rtdope->fsiz * 512 - ocount) / 512 ;
	if (count <= 0) return;
	for ( ; count > 0 ; count--) {
		startad += 512;
		lwrite(startad, 512, zeroes);
	}
}
dcmd()
{
	register int i;

	if (namc)
		for (i = 0; i < namc; i++)
			rtd(namv[i]);
}

rtd(name)
char *name;
{
	register dope_t *dope;
	register struct rt_ent *de;

	if (dope = lookup(name)) {
		if (vflg)
			printf("d - %s\n",name);
		de = dope->rtdope;
		de->type = RT_NULL;
		de->fname[0] = 0;
		de->fname[1] = 0;
		de->fname[2] = 0;
		* ((ushort *) & (de->cdate)) = 0;
		scrunch();
	}
}

scrunch() {
	register struct rt_ent *de, *workp;
	register i;

	for (i = 0; i < segcnt; i++) {
		for (de = rt_dir[i].entry; de < rt_dir[i].last; de++) {
			if (de->type == RT_ESEG)
				break;
			if (de->type==RT_NULL && de[1].type==RT_NULL) {
				(de+1)->fsiz += de->fsiz;
				for (workp = de; ; workp++) {
					*workp = workp[1];
					if (workp->type == RT_ESEG)
						break;
				}
				de--;
			}
		}
		lwrite((6+2*i)*RTB, 2*RTB, &rt_dir[i]);
	}
}
