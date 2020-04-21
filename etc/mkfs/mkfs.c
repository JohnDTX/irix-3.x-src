char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";

/*	@(#)mkfs.c	1.10	*/
/*	mkfs	COMPILE:	cc -O mkfs.c -s -i -o mkfs
 * Make a file system prototype.
 * usage: mkfs -B filsys size[:inodes] [gap blocks/cyl]
 *        mkfs -B filsys proto [gap blocks/cyl]
 *	  mkfs [-E] ...
 */

#ifndef STANDALONE
#endif STANDALONE
#include "a.out.h"

#include "sys/param.h"
#ifndef STANDALONE
#include "signal.h"
#endif STANDALONE
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/ino.h"
#include "sys/filsys.h"
#include "sys/fblk.h"
#include "sys/dir.h"
#ifndef STANDALONE
#include "sys/stat.h"
#endif STANDALONE
#include "stdio.h"

/* file system block size */
#ifdef m68000
#	if (FsTYPE == 3)
#		define FSBSIZE	(BSIZE*2)
#	else
#		define	FSBSIZE BSIZE
#	endif
#else m68000
#	if (vax || u3b) && (FsTYPE == 3)
#		define FSBSIZE	(BSIZE*2)
#	else
#		define	FSBSIZE BSIZE
#	endif
#endif m68000
/* boot-block size */
#define BOOTSIZE	512

#ifndef SUPERBOFF
#define SUPERBOFF 512
#endif  SUPERBOFF
/* super-block size */
#define SBSIZE	512

#define	NIDIR	(FSBSIZE/sizeof(daddr_t))
#define	NFB	(NIDIR+500)	/* NFB must be greater than NIDIR+LADDR */
#define	NDIRECT	(FSBSIZE/sizeof(struct direct))
#define	NBINODE	(FSBSIZE/sizeof(struct dinode))
#ifndef NADDR
#define NADDR	13
#endif  NADDR
struct inode
{
	dev_t	i_dev;
	ino_t	i_number;
	ushort 	i_mode;
	short  	i_nlink;
	ushort 	i_uid;
	ushort 	i_gid;
	dev_t	i_rdev;
	off_t	i_size;
	time_t	i_atime;
	time_t	i_mtime;
	time_t	i_ctime;
	daddr_t i_faddr[NADDR];
};
#define	LADDR	10
#define	STEPSIZE	7
#define	CYLSIZE		400
#define	MAXFN	1000

time_t	utime;
#ifndef STANDALONE
FILE 	*fin;
#endif STANDALONE
int	fsi;
int	fso;
char	*charp;
char	buf[FSBSIZE];

char work0[FSBSIZE];
struct fblk *fbuf = (struct fblk *)work0;

#ifndef STANDALONE
struct exec head;
#endif STANDALONE
char	string[50];

char work1[FSBSIZE];
struct filsys *filsys = (struct filsys *)work1;
#ifndef STANDALONE
char	*fsys;
char	*proto;
#else STANDALONE
char	fsys[50];
char	proto[50];
#endif STANDALONE
int	f_n = CYLSIZE;
int	f_m = STEPSIZE;
int	error;
ino_t	ino;
#ifndef STANDALONE
int	sflag;
#endif STANDALONE
daddr_t	maxbn;			/* for UniSoft-added -s option */
long	getnum();
daddr_t	alloc();

/* ARGSUSED */

bell_main(argc, argv)
int argc;
char *argv[];
{
	int f, c;
	long n, nb;
#ifndef STANDALONE
	struct stat statarea;
	struct {
		daddr_t tfree;
		ino_t tinode;
		char fname[6];
		char fpack[6];
	} ustatarea;

	/*
	 * open relevent files
	 */

	time(&utime);
	if(argc < 3) {
		printf("usage: %s [-s] filsys proto [gap blocks/cyl]\n       %s [-s] filsys blocks[:inodes] [gap blocks/cyl]\n", argv[0], argv[0]);
		exit(1);
	}
	if (strcmp("-s",argv[1]) == 0) {
		sflag++;
		argv++;
		argc--;
	}
	fsys = argv[1];
	if(stat(fsys, &statarea) < 0) {
		printf("%s: cannot stat\n",fsys);
		exit(1);
	}
	proto = argv[2];
	fso = creat(fsys, 0666);
	if(fso < 0) {
		printf("%s: cannot create\n", fsys);
		exit(1);
	}
	fsi = open(fsys, 0);
	if(fsi < 0) {
		printf("%s: cannot open\n", fsys);
		exit(1);
	}
	if((statarea.st_mode & S_IFMT) == S_IFBLK)
		if(ustat(statarea.st_rdev,&ustatarea) >= 0) {
			printf("*** MOUNTED FILE SYSTEM\n");
			exit(1);
		}
/* set magic number for file system type */
	filsys->s_magic = FsMAGIC;
	filsys->s_type = (FSBSIZE == 1024) ? Fs2b : Fs1b;
	fin = fopen(proto, "r");
	if(fin == NULL) {
#else STANDALONE
loop:
	printf("\n\nStandalone mkfs\n");
	do {
		finit();
		printf("\ndevice:  ");	gets(fsys);
		fsi = open(fsys, 0);
		fso = open(fsys, 1);
	} while ((fsi < 0) || (fso < 0));
retry:
	printf("size[:inodes]  ");	gets(proto);
	nb = 0;
#endif STANDALONE
		n = 0;
		for(f=0; c=proto[f]; f++) {
			if(c<'0' || c>'9') {
				if(c == ':') {
					nb = n;
					n = 0;
					continue;
				}
#ifndef STANDALONE
				printf("%s: cannot open\n", proto);
				exit(1);
#else STANDALONE
				printf("invalid number\n");
				printf("enter 1 number or\n");
				printf("enter 2 numbers separated by a ':'\n");
				goto retry;
#endif STANDALONE
			}
			n = n*10 + (c-'0');
		}
#ifdef STANDALONE
		if (!n) {
			printf("size must be non-zero\n");
			goto retry;
		}
#endif STANDALONE
		if(!nb) {
			nb = n / (FSBSIZE/DEV_BSIZE);
			n = nb/(NBINODE*4);
		} else {
			nb /= (FSBSIZE/DEV_BSIZE);
			n /= NBINODE;
		}
		filsys->s_fsize = nb;
		if(n <= 0)
			n = 1;
		if(n > 65500/NBINODE)
			n = 65500/NBINODE;
		filsys->s_isize = n + 2;

		charp = "d--777 0 0 $ ";
		goto f3;
#ifndef STANDALONE
	}

	/*
	 * get name of boot load program
	 * and read onto block 0
	 */
	getstr();
	bootcopy(string, fso);

	/*
	 * get total disk size
	 * and inode block size
	 */
	nb = getnum();
	filsys->s_fsize = nb / (FSBSIZE/DEV_BSIZE);
	n = getnum();
	n /= NBINODE;
	filsys->s_isize = n + 2;

f3:
	if(argc >= 5) {
		f_m = atoi(argv[3]);
		f_n = atoi(argv[4]);
#else STANDALONE
f3:
	printf("gap:           ");	gets(proto);
	f_m = atoi(proto);
	printf("blocks/cyl:    ");	gets(proto);
	f_n = atoi(proto);
#endif STANDALONE
		if(f_n <= 0 || f_n >= MAXFN)
			f_n = CYLSIZE;
		if(f_m <= 0 || f_m > f_n)
			f_m = STEPSIZE;
#ifndef STANDALONE
	}
#endif STANDALONE
	filsys->s_dinfo[0] = f_m;
	filsys->s_dinfo[1] = f_n;
	f_m /= (FSBSIZE/DEV_BSIZE);
	f_n /= (FSBSIZE/DEV_BSIZE);
	if (f_n == 0)
		f_n = 1;

	printf("bytes per logical block = %d\n", FSBSIZE);
#ifndef STANDALONE
	printf("total logical blocks = %ld\n", filsys->s_fsize);
	printf("total inodes = %ld\n", n*NBINODE);
#else STANDALONE
	printf("total logical blocks = %D\n", filsys->s_fsize);
	printf("total inodes = %D\n", n*NBINODE);
#endif STANDALONE
	printf("gap (physical blocks) = %d\n", filsys->s_dinfo[0]);
	printf("cylinder size (physical blocks) = %d \n", filsys->s_dinfo[1]);

	if(filsys->s_isize >= filsys->s_fsize) {
#ifndef STANDALONE
		printf("%ld/%ld: bad ratio\n", filsys->s_fsize, filsys->s_isize-2);
		exit(1);
#else STANDALONE
		printf("%D/%D: bad ratio\n", filsys->s_fsize, filsys->s_isize-2);
		goto retry;
#endif STANDALONE
	}
	filsys->s_tinode = 0;
	for(c=0; c<FSBSIZE; c++)
		buf[c] = 0;
	for(n=2; n!=filsys->s_isize; n++) {
		wtfs(n, buf);
		filsys->s_tinode += NBINODE;
	}
	ino = 0;

	bflist();

	maxbn = (daddr_t) filsys->s_isize;
	cfile((struct inode *)0);

	filsys->s_time = utime;

/* write super-block onto file system */
	if (lseek(fso, (long)SUPERBOFF, 0) < 0) {
		printf("lseek error: super-block\n");
#ifndef STANDALONE
		exit(1);
#else STANDALONE
		exit();
#endif STANDALONE
	}
	if(write(fso, (char *)filsys, SBSIZE) != SBSIZE) {
		printf("write error: super-block\n");
#ifndef STANDALONE
		exit(1);
#else STANDALONE
		exit();
#endif STANDALONE
	}

#ifndef STANDALONE
	if (sflag)
		printf("\nmax block number used (excluding free list blocks) is %d\n", maxbn);
	exit(error);
#else STANDALONE
	printf("\nmax block number used (excluding free list blocks) is %d\n", maxbn);
	goto loop;
#endif STANDALONE
}

bootcopy(bootprog, ofd)
	char *bootprog;
{
	register int f;
	struct exec head;
	char buf[BOOTSIZE];
	int c;

	f = open(bootprog, 0);
	if(f < 0) {
		printf("cannot open boot file %s\n", bootprog);
		exit(1);
	}
	read(f, (char *)&head, sizeof head);
/*
	if (N_BADMAG(head))
	{
		printf("%s: bad format\n", string);
		exit(1);
	}
 */
	c = head.a_text + head.a_data;
	if(c > BOOTSIZE) {
		printf("%s: too big\n", bootprog);
		exit(1);
	}
	read(f, buf, c);

/* write boot-block to file system */
	(void)lseek(ofd, 0L, 0);
	if(write(ofd, buf, BOOTSIZE) != BOOTSIZE) {
		printf("write error: boot-block\n");
		exit(1);
	}

	close(f);
}

cfile(par)
struct inode *par;
{
	struct inode in;
	int dbc, ibc;
	char db[FSBSIZE];
	daddr_t ib[NFB];
	int i, f;

	/*
	 * get mode, uid and gid
	 */
	in.i_mode = gmode();
	in.i_uid = getnum();
	in.i_gid = getnum();

	/*
	 * general initialization prior to
	 * switching on format
	 */

	ino++;
	in.i_number = ino;
	for(i=0; i<FSBSIZE; i++)
		db[i] = 0;
	for(i=0; i<NFB; i++)
		ib[i] = (daddr_t)0;
	in.i_nlink = 1;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_faddr[i] = (daddr_t)0;
	if(par == (struct inode *)0) {
		par = &in;
		in.i_nlink--;
	}
	dbc = 0;
	ibc = 0;
	switch(in.i_mode&S_IFMT) {

	case S_IFREG:
		/*
		 * regular file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) {
			printf("%s: cannot open\n", string);
			error = 1;
			break;
		}
		while((i=read(f, db, FSBSIZE)) > 0) {
			in.i_size += i;
			newblk(&dbc, db, &ibc, ib);
		}
		close(f);
		break;

	case S_IFBLK:
	case S_IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */

		i = getnum() & 0377;
		f = getnum() & 0377;
		in.i_faddr[0] = (i<<8) | f;
		break;

	case S_IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		par->i_nlink++;
		in.i_nlink++;
		entry(in.i_number, ".", &dbc, db, &ibc, ib);
		entry(par->i_number, "..", &dbc, db, &ibc, ib);
		in.i_size = 2*sizeof(struct direct);
		for(;;) {
			getstr();
			if(string[0]=='$' && string[1]=='\0')
				break;
			entry(ino+1, string, &dbc, db, &ibc, ib);
			in.i_size += sizeof(struct direct);
			cfile(&in);
		}
		break;

	}
	if(dbc != 0)
		newblk(&dbc, db, &ibc, ib);
	iput(&in, &ibc, ib);
}

int
gmode()
{
	register int mode;
	register int i;
	register char *cp;
	unsigned int c;

	getstr();
	cp = string;

	mode = gmodec(*cp++, "-bcd", S_IFREG, S_IFBLK, S_IFCHR, S_IFDIR, 0, 0, 0);
	mode |= gmodec(*cp++, "-u", 0, S_ISUID, 0, 0, 0, 0, 0);
	mode |= gmodec(*cp++, "-g", 0, S_ISGID, 0, 0, 0, 0, 0);

	for (i = 9; (i -= 3) >= 0;) {
		c = *cp++ - '0';
		if (c > 7) {
			printf("%c/%s: bad octal mode digit\n", c, string);
			error = 1;
			c = 0;
		}
		mode |= c << i;
	}
	if (*cp != 000) {
		printf("bad mode %s\n", string);
		error = 1;
	}

	return mode;
}

/* ARGSUSED */
gmodec(c, s, m0, m1, m2, m3, m4, m5, m6)
char c, *s;
{
	int i;

	for(i=0; s[i]; i++)
		if(c == s[i])
			return((&m0)[i]);
	printf("%c/%s: bad mode\n", c, string);
	error = 1;
	return(0);
}

long getnum()
{
	int i, c;
	long n;

	getstr();
	n = 0;
	for(i=0; c=string[i]; i++) {
		if(c<'0' || c>'9') {
			printf("%s: bad number\n", string);
			error = 1;
			return((long)0);
		}
		n = n*10 + (c-'0');
	}
	return(n);
}

getstr()
{
	int i, c;

loop:
	switch(c=getch()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		printf("EOF\n");
#ifndef STANDALONE
		exit(1);
#else STANDALONE
		exit();
#endif STANDALONE

	case ':':
		while(getch() != '\n');
		goto loop;

	}
	i = 0;

	do {
		string[i++] = c;
		c = getch();
	} 
	while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
	string[i] = '\0';
}

rdfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	if (lseek(fsi, (long)(bno*FSBSIZE), 0) < 0) {
#ifndef STANDALONE
		printf("lseek error: %ld\n", bno);
		exit(1);
#else STANDALONE
		printf("lseek error: %D\n", bno);
		exit();
#endif STANDALONE
	}
	n = read(fsi, bf, FSBSIZE);
	if(n != FSBSIZE) {
#ifndef STANDALONE
		printf("read error: %ld\n", bno);
		exit(1);
#else STANDALONE
		printf("read error: %D\n", bno);
		exit();
#endif STANDALONE
	}
}

wtfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	if (lseek(fso, (long)(bno*FSBSIZE), 0) < 0) {
#ifndef STANDALONE
		printf("lseek error: %ld\n", bno);
		exit(1);
#else STANDALONE
		printf("lseek error: %D\n", bno);
		exit();
#endif STANDALONE
	}
	n = write(fso, bf, FSBSIZE);
	if(n != FSBSIZE) {
#ifndef STANDALONE
		printf("write error: %ld\n", bno);
		exit(1);
#else STANDALONE
		printf("write error: %D\n", bno);
		exit();
#endif STANDALONE
	}
	if (bno > maxbn)
		maxbn = bno;
}

daddr_t alloc()
{
	int i;
	daddr_t bno;

	filsys->s_tfree--;
	bno = filsys->s_free[--filsys->s_nfree];
	if(bno == 0) {
		printf("out of free space\n");
#ifndef STANDALONE
		exit(1);
#else STANDALONE
		exit();
#endif STANDALONE
	}
	if(filsys->s_nfree <= 0) {
		rdfs(bno, (char *)fbuf);
		filsys->s_nfree = fbuf->df_nfree;
		for(i=0; i<NICFREE; i++)
			filsys->s_free[i] = fbuf->df_free[i];
	}
	return(bno);
}

bfree(bno)
daddr_t bno;
{
	int i;

	filsys->s_tfree++;
	if(filsys->s_nfree >= NICFREE) {
		fbuf->df_nfree = filsys->s_nfree;
		for(i=0; i<NICFREE; i++)
			fbuf->df_free[i] = filsys->s_free[i];
		wtfs(bno, (char *)fbuf);
		filsys->s_nfree = 0;
	}
	filsys->s_free[filsys->s_nfree++] = bno;
}

entry(in, str, adbc, db, aibc, ib)
ino_t in;
char *str;
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	struct direct *dp;
	int i;

	dp = (struct direct *)db;
	dp += *adbc;
	(*adbc)++;
	dp->d_ino = in;
	for(i=0; i<DIRSIZ; i++)
		dp->d_name[i] = 0;
	for(i=0; i<DIRSIZ; i++)
		if((dp->d_name[i] = str[i]) == 0)
			break;
	if(*adbc >= NDIRECT)
		newblk(adbc, db, aibc, ib);
}

newblk(adbc, db, aibc, ib)
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	int i;
	daddr_t bno;

	bno = alloc();
	wtfs(bno, db);
	for(i=0; i<FSBSIZE; i++)
		db[i] = 0;
	*adbc = 0;
	ib[*aibc] = bno;
	(*aibc)++;
	if(*aibc >= NFB) {
		printf("file too large\n");
		error = 1;
		*aibc = 0;
	}
}

getch()
{

	if(charp)
		return(*charp++);
#ifndef STANDALONE
	return(getc(fin));
#else STANDALONE
	return(NULL);	/* for LINT */
#endif STANDALONE
}

bflist()
{
	struct inode in;
	daddr_t ib[NFB];
	int ibc;
	char flg[MAXFN];
	int adr[MAXFN];
	int i, j;
	daddr_t f, d;

	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+f_m)%f_n;
	}

	ino++;
	in.i_number = ino;
	in.i_mode = S_IFREG;
	in.i_uid = 0;
	in.i_gid = 0;
	in.i_nlink = 0;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_faddr[i] = (daddr_t)0;

	for(i=0; i<NFB; i++)
		ib[i] = (daddr_t)0;
	ibc = 0;
	bfree((daddr_t)0);
	filsys->s_tfree = 0;
	d = filsys->s_fsize-1;
	while(d%f_n)
		d++;
	for(; d > 0; d -= f_n)
		for(i=0; i<f_n; i++) {
			f = d - adr[i];
			if(f < filsys->s_fsize && f >= filsys->s_isize)
				if(badblk(f)) {
					if(ibc >= NIDIR) {
						printf("too many bad blocks\n");
						error = 1;
						ibc = 0;
					}
					ib[ibc] = f;
					ibc++;
				} else {
					bfree(f);
				}
		}
	iput(&in, &ibc, ib);
}

iput(ip, aibc, ib)
register struct inode *ip;
register int *aibc;
daddr_t *ib;
{
	register struct dinode *dp;
	daddr_t d;
	register int i,j,k;
	daddr_t ib2[NIDIR];	/* a double indirect block */

	filsys->s_tinode--;
	d = FsITOD(0, ip->i_number);
	if(d >= filsys->s_isize) {
		if(error == 0)
			printf("ilist too small\n");
		error = 1;
		return;
	}
	rdfs(d, buf);
	dp = (struct dinode *)buf;
	dp += FsITOO(0, ip->i_number);

	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_atime = utime;
	dp->di_mtime = utime;
	dp->di_ctime = utime;

	switch(ip->i_mode&S_IFMT) {

	case S_IFDIR:
	case S_IFREG:
		/* handle direct pointers */
		for(i=0; i<*aibc && i<LADDR; i++) {
			ip->i_faddr[i] = ib[i];
			ib[i] = 0;
		}
		/* handle single indirect block */
		if(i < *aibc)
		{
			for(j=0; i<*aibc && j<NIDIR; j++, i++)
				ib[j] = ib[i];
			for(; j<NIDIR; j++)
				ib[j] = 0;
			ip->i_faddr[LADDR] = alloc();
			wtfs(ip->i_faddr[LADDR], (char *)ib);
		}
		/* handle double indirect block */
		if(i < *aibc)
		{
			for(k=0; k<NIDIR && i<*aibc; k++)
			{
				for(j=0; i<*aibc && j<NIDIR; j++, i++)
					ib[j] = ib[i];
				for(; j<NIDIR; j++)
					ib[j] = 0;
				ib2[k] = alloc();
				wtfs(ib2[k], (char *)ib);
			}
			for(; k<NIDIR; k++)
				ib2[k] = 0;
			ip->i_faddr[LADDR+1] = alloc();
			wtfs(ip->i_faddr[LADDR+1], (char *)ib2);
		}
		/* handle triple indirect block */
		if(i < *aibc)
		{
			printf("triple indirect blocks not handled\n");
		}
		break;

	case S_IFBLK:
		break;

	case S_IFCHR:
		break;

	default:
		printf("bad mode %o\n", ip->i_mode);
#ifndef STANDALONE
		exit(1);
#else STANDALONE
		exit();
#endif STANDALONE
	}

	ltol3(dp->di_addr, ip->i_faddr, NADDR);
	wtfs(d, buf);
}

/* ARGSUSED */
badblk(bno)
daddr_t bno;
{

	return(0);
}
