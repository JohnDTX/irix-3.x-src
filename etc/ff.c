char _Origin_[] = "System V";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/etc/RCS/ff.c,v 1.1 89/03/27 15:37:40 root Exp $";
/*
 * $Log:	ff.c,v $
 * Revision 1.1  89/03/27  15:37:40  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  86/04/04  15:22:46  root
 * no longer part of the distribution, teb
 * 
 * Revision 1.2  85/03/08  23:39:44  bob
 * *** empty log message ***
 * 
 */

/*
 *	ff - fast(er) find
 *
 *	ff produces a list of filenames for a specified filesystem,
 *	allowing selection criteria.
 *
 *	strategy:
 *
 *	- process command line, saving any selection criteria.
 *	- read superblock of fs to obtain size, and allocate data
 *	  areas (see below).
 *	- read the ilist and save:
 *		- inumbers and inode information from selected files
 *		- block numbers of directory files.
 *	- read all directory blocks saving all directory entries
 *	- for all selected inumbers, generate their names.
 *
 *	data areas:
 *
 *	- Ireq is simply a list of saved inumbers who have been selected
 *	  for name generation.
 *	- Db holds directory block data. Specifically we need to know which
 *	  blocks in the filesystem belong to directories, and which dir is
 *	  their owner. This associates the dirnames which we read with their
 *	  parent directory (.) pointer.
 *	- Dd is an array, with inumber as an index, whose entries are the
 *	  leaf name of a file. The pinum member of Dd points to an entry's 
 *	  parent directory name, whose pinum points ...  So we go, climbing
 *	  the directory structure for each name we generate.
 *	  NOTE: Dd, being an array, MUST have as many elements as there are
 *	  inumbers. Can't compromise on space for this guy. 
 *	- Indb holds info on the rare case of indirect directories.
 *	- Link is a Dd array in which we save info on linked files. If a
 *	  given inode already has an entry in Dd we save it in Link.
 *
 *	  written by ez for the ill-fated Large UNIX Systems Development Group
 */

#include <sys/param.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/filsys.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pwd.h>

/*
	Limits - These values are expanded in line in routine super().
	They use superblock values and rough approximations to define
	the sizes of the data structures. Note that they are probably
	different at each installation and maybe even across filesystems.
	ilb is the ilist size in blocks, lstblk & fstblk are just that.
	They represent an effort to balance the needs of the progam against
	the limited amount of memory available on your mini computer.
	Note that super() will adjust these values depending on avail-
	able memory. Read the code 'afore ye play.

*/
#define	LIMITS	{	\
maxIreq = g_ilb*g_inopb*1.00;	/* % of all inodes which are used */\
maxL = 1+(g_ilb*g_inopb)*0.05;	/* % of all files which are links */\
maxDb = (lstblk-fstblk)*0.10;	/* number of blocks owned by dirs */\
maxIb = 20;			/* number of indirect dir blocks  */\
		}


#define	MAXNLIST 64	/* max specific inumbers after -i option */
#define	A_DAY	86400l	/* seconds in a 24 hour period */
#define	DIRENT	(sizeof(struct direct))
#define	EQ(x,y)	(strcmp(x,y)==0)
#define	ALLOC	0
#define	FREE	1
#define	DATA	0
#define	SINGLE	1
#define	DOUBLE	2
#define	TRIPLE	3

/*
	Formatting options for printout
*/
#define	F_INODE	1
#define	F_SIZE	2
#define	F_UID	4
#define	F_PREPATH	8

/*
	Run options
*/
#define	F_REQUEST	1
#define	F_LINKSTOO	4
#define	F_DEBUG		8

/*	Data Structures	*/

/* Dd and Link - holds data from a directory entry */
struct	Dd	{
	ino_t	inum;		/* inumber of file represented by entry */
	ino_t	pinum;		/* inumber of parent directory */
	char	dname[DIRSIZ];	/* file name */
	};
struct	Dd	*Ddlist, *Linklist;
long	nLink;
long	maxL;

/* Ireq - holds inumbers and any inode data for selected files */
struct	Ireq	{
	ino_t	inum;
	off_t	size;
	ushort	uid;
	};
struct	Ireq	*pIreq, *Ireqlist;
long	nIreq;
long	maxIreq;


/* Db - holds data on the blocks of the filesystem which are directories */
struct	Db	{
	ino_t	inum;		/* inumber of this directory */
	off_t	size;		/* # of data bytes in this directory blk */
	daddr_t	blkno;		/* block number */
	};
struct	Db	*pDb, *Dblist;
long	nDb;
long	maxDb;


/* Iblk - saves indirect blocks of directory files */
struct	Iblk	{
	ino_t	inum;
	daddr_t	single;
	daddr_t	dubble;
	daddr_t	triple;
	off_t	size;
	};
struct	Iblk	*pIb, *Iblklist;
long	nIb;
long	maxIb;

/* Nlist holds specific inode numbers requested with the -i option */
ino_t	*Nlist, *pN;


char	*usage =
	"[-IsSuU -i ilist -p path -n file -a # -m # -c #]  /inputdev";

extern	int	errno;
extern	int	sys_nerr;
extern	char	*sys_errlist[];

struct	stat	statb;

int	g_bufn,		/* internal buffer allocation counter */
	g_ilb,		/* ilist size in blocks */
	g_inodes,	/* ilist size in inodes */
	g_inopb,	/* number of inodes in a block */
	g_bsize,	/* actual filesystem block size */
	g_fpi;		/* input filesystem file pointer */

char	*g_cmd,		/* our name, as invoked by the user */
	*g_dev,		/* name of filesystem we are processing */
	*g_path,	/* optional prefix path for generated names */
	*g_buf;		/* our general purpose buffer */

off_t	g_size;

int	criteria;	/* count of user selection criteria */

int	badnames;	/* incremented for unresolved path names */

char	prtopt,		/* flags - print options */
	runopt,		/* flags - run time options */
	as,		/* sign of number for -a option */
	ms,		/* "        "      "  -m option */
	cs;

long	atime,		/* abs(number) of days from -a option */
	mtime,
	crtime,		/* "         "           "  -c option */
	ntime,
	today;		/* today as returned by time() */

main(argc,argv)
int argc;
char **argv;
{

	extern	char	*optarg;
	extern	int	optind;
	int	stat(), atoi();
	int	c;

	g_cmd = argv[0];
	g_dev = "";
	g_path = "";
	if (argc < 2)
		goto bad;
	prtopt |= F_INODE;
	runopt = 0;
	badnames=0;
	time(&today);
	while((c=getopt(argc,argv,"IsSuUlda:m:c:n:i:p:")) != EOF)
		switch(c) {
		case 'I':
			prtopt &= ~F_INODE;
			break;
		case 's':
			prtopt |= F_SIZE;
			break;
		case 'u':
			prtopt |= F_UID;
			break;
		case 'd':
			runopt |= F_DEBUG;
			break;
		case 'a':
			as = *optarg;
			if ((atime = atoi(optarg)) == 0) 
				errorx("bad value %s for option a\n",optarg);
			++criteria;
			break;
		case 'm':
			ms = *optarg;
			if ((mtime = atoi(optarg)) == 0) 
				errorx("bad value %s for option m\n",optarg);
			++criteria;
			break;
		case 'c':
			cs = *optarg;
			if ((crtime = atoi(optarg)) == 0) 
				errorx("bad value %s for option c\n",optarg);
			++criteria;
			break;
		case 'n':
			if (stat(optarg,&statb) < 0) 
				errorx("n option, %s stat failed\n",optarg);
			++criteria;
			ntime = statb.st_mtime;
			break;

		case 'i':
			runopt |= F_REQUEST;
			++criteria;
			if (inodes(optarg) == 0)
				exit(1);
			break;

		case 'p':
			prtopt |= F_PREPATH;
			g_path = optarg;
			break;

		case 'l':
			runopt |= F_LINKSTOO;
			break;

		case '?':
		default:
		bad:	errorx("Usage: %s %s\n",g_cmd,usage);

		}


	process(argv[optind]);

	if (badnames)
		error("NOTE! %d pathnames only partially resolved\n", badnames);

	if (runopt&F_DEBUG) {
		error("nLink=%d	nIreq=%d	nDb=%d	nIb=%d\n",
		maxL, maxIreq, maxDb, maxIb);
		error("nLink=%d	nIreq=%d	nDb=%d		nIb=%d\n",
		nLink, nIreq, nDb, nIb);
	}

	exit(0);

}

process(dev)
char *dev;
{
	int	bcomp();

	sync();
	if (dev == (char *) NULL)
		errorx("Usage: %s %s\n",g_cmd, usage);
	else
		g_dev = dev;

	if (stat(dev,&statb) < 0)
		errorx("Input %s is non-existent\n",g_dev);

	if ((((statb.st_mode&IFMT)==IFBLK)|((statb.st_mode&IFMT)==IFCHR))==0)
		errorx("Input device %s is not a special file\n",g_dev);

	if ((g_fpi=open(dev,O_RDONLY)) < 0)
		errorx("Open failed for input on %s\n",g_dev);

	if (super() <= 0)
		return(0);

	ilist();
	if (nIreq == 0) {
		error("No files were selected.\n");
		return(0);
	}

	if (nIb > 0)
		indir();

	qsort((char *)Dblist, (int) nDb, sizeof(struct Db), bcomp);
	readdirs();

	error("%d files selected\n", nIreq);

	if (runopt&F_LINKSTOO)
		error("%d link names detected\n", nLink);

	monikers();

	return(1);

}

int super()
{

	char	*calloc(), *malloc(), *getbuf();
	int	fstblk, lstblk, retry;
	long	have, want, ulimit();
	struct	filsys	*fs;

	g_bsize = DEV_BSIZE;
	g_buf=getbuf(ALLOC);
	seer(1, g_buf);
	fs = (struct filsys *)g_buf;

	fstblk = fs->s_isize;
	lstblk = fs->s_fsize;
	g_ilb = fstblk - 2;
#ifdef FsMAGIC
	g_bsize = (fs->s_magic!=FsMAGIC ? BSIZE :
			(fs->s_type==1 ? 512 :
			(fs->s_type==2 ? 1024 :
			errorx("unknown filesystem BSIZE\n"))));
#else
	g_bsize = BSIZE;
#endif
	g_inopb = g_bsize/sizeof(struct dinode);
	g_inodes = g_ilb*g_inopb;

	if (runopt&F_DEBUG)
		error("fstblk:%d  lstblk:%d  bsize:%d inopb:%d\n",
		fs->s_isize, fs->s_fsize, g_bsize, g_inopb);
	if (fstblk<=0 ||lstblk<=0 ||g_ilb<=0 ||fstblk>lstblk ||g_ilb>lstblk) 
		errorx("Invalid super block\n");
	
	getbuf(FREE,g_buf);

	LIMITS;

	retry = 0;
again:	have = ulimit(3,0) - sbrk(0) - 16000;	/* get memory available */
	want =	maxIreq*sizeof(struct Ireq) + maxL*sizeof(struct Dd) +
		maxDb*sizeof(struct Db) + maxIb*sizeof(struct Iblk) +
		(1+g_ilb*g_inopb)*sizeof(struct Dd);
	if (have < want) {
		if (retry++ > 4) 
			errorx("Insufficient memory. Giving up\n");
		maxL = (long)((float)maxL*0.90);
		maxDb = (long)((float)maxDb*0.90);
		maxIreq = (long)((float)maxIreq*0.90);
		goto again;
	}
	if (retry)
		error("Reducing memory demands. May fail.\n");

	if ((Ddlist=(struct Dd *)calloc(g_ilb*g_inopb+1,sizeof(struct Dd))) == 0) 
		errorx("insufficient memory for Dd area\n");

	if ((Linklist=(struct Dd *)calloc(maxL,sizeof(struct Dd))) == 0) 
		errorx("insufficient memory for Link area\n");

	if ((pIreq=Ireqlist=(struct Ireq *)malloc((unsigned)(maxIreq*sizeof(struct Ireq)))) == 0) 
		errorx("insufficient memory for Ireq area\n");

	if ((pDb=Dblist=(struct Db *)malloc((unsigned)(maxDb*sizeof(struct Db)))) == 0) 
		errorx("insufficient memory for Db area\n");

	if ((pIb=Iblklist=(struct Iblk *)malloc((unsigned)(maxIb*sizeof(struct Iblk)))) == 0) 
		errorx("insufficient memory for Iblk area\n");

	nIreq = nDb = nIb = nLink = 0;

	return(1);

}

int ilist()

{

	int	ipb, block; 
	char	*getbuf();
	ino_t	curi = 0;
	struct	dinode	*dip;

	/*
		we are positioned past the super block,
		at the first inode block.
	*/
	g_buf=getbuf(ALLOC);

	for (block=0; block<g_ilb; block++) {

		seer(block+2, g_buf);
		dip = (struct dinode *)g_buf;

		for (ipb=0; ipb<g_inopb; ipb++, dip++) {
			curi++;
			/*if (runopt&F_DEBUG)
				error("i:%d  sz:%d  ln:%d\n",
				curi, dip->di_size, dip->di_nlink);*/
			if (dip->di_mode == 0 || curi == 1)
				continue;
			if ((dip->di_mode&IFMT)==IFDIR) 
				savedir(curi,dip);
			if (select(curi,dip)) 
				saveinod(curi,dip);
		}
	}
	getbuf(FREE,g_buf);

	return;
}


saveinod(inum,dip)
int	inum;
struct	dinode	*dip;
{

	(*pIreq).inum = inum;
	if (dip != 0) {
		(*pIreq).size = dip->di_size;
		(*pIreq).uid = dip->di_uid;
	}
	pIreq++;
	if (nIreq++ > maxIreq)
		exceed(maxIreq, "Ireq - selected file list");

	return;

}

savedir(inum,dip)
ino_t	inum;
struct	dinode	*dip;
{

	daddr_t	l[NADDR];
	int	i, size;
	struct	direct	*dirp;

	if (dip->di_mode == 0 || !((dip->di_mode&IFMT)==IFDIR))  
		return;

	l3tol(l,dip->di_addr,NADDR);
	size = dip->di_size;
#ifdef u370
	/* Small Block UNIX/370 
	in SBu370 an inode with SBAUSE set contains SBASIZE data bytes
	*/
	if (dip->di_sbmode == SBAUSE) {
		dirp = (struct direct *) (dip->di_data);
		i = (size > SBASIZE ? SBASIZE : size) / DIRENT;
		while (i--)
			xdname(dirp++,inum);
		size -= SBASIZE;
	}
#endif

	for (i=0; i<NADDR-3 && size > 0 ; i++) {   
		if (l[i] == 0)
			continue;
		adddirb(inum, l[i], (off_t)(size > g_bsize ? size-=g_bsize, g_bsize : size));
	}

	if ( l[NADDR-3] || l[NADDR-2] || l[NADDR-1] ) {
		addindb(inum, dip->di_size, l[NADDR-3], l[NADDR-2], l[NADDR-1]);
	}

	return;

}

adddirb(ino,blk,sz)
ino_t	ino;
daddr_t	blk;
off_t	sz;

{

	pDb->inum = ino;
	pDb->blkno = blk;
	pDb->size = sz;
	pDb++;
	if (nDb++ > maxDb)
		exceed(maxDb, "Db - directory data block list");

	return;

}


addindb(ino, sz, s, d, t)
ino_t	ino;
off_t	sz;
daddr_t	s, d, t;
{

	pIb->inum = ino;
	pIb->single = s;
	pIb->dubble = d;
	pIb->triple = t;
	pIb->size = sz;
	pIb++;
	if (nIb++ > maxIb)
		exceed(maxIb, "Iblk - indirect directory block list");
	return;

}
addlink(i, p, d)
register ino_t i,p;
register char *d;
{

	Linklist[nLink].inum = i;
	Linklist[nLink].pinum = p;
	strncpy(Linklist[nLink].dname,d,DIRSIZ);
	if (nLink++ > maxL)
		exceed(maxL, "Link - linked file list");

}

select(curi,dip)
ino_t	curi;
struct dinode *dip;
{

	register int ok = 0;

	if (criteria == 0)
		return(1);

	if (atime) 
		ok += (scomp((long)((today-dip->di_atime)/A_DAY),atime,as)) ? 1 : 0;

	if (mtime) 
		ok += (scomp((long)((today-dip->di_mtime)/A_DAY),mtime,ms)) ? 1 : 0;

	if (crtime) 
		ok += (scomp((long)((today-dip->di_ctime)/A_DAY),crtime,cs)) ? 1 : 0;

	if (ntime) 
		ok += (dip->di_mtime > ntime) ? 1 : 0;

	if (runopt&F_REQUEST)
		ok += (ncheck(curi) ? 1 : 0);

	return(ok == criteria);

}

scomp(a,b,s)
register long a, b;
register char s;
{

	if (s == '+')
		return(a > b);
	if (s == '-')
		return(a < (b * -1));
	return(a == b);

}

readdirs()
{

	int	i, j;
	struct	direct	*dirp;

	pDb = Dblist;
	for (i=0; i<nDb; i++, pDb++) {
		seer(pDb->blkno, g_buf);
		dirp = (struct direct *)g_buf;
		for (j=0; j<pDb->size/DIRENT; j++, dirp++)
			xdname(dirp,pDb->inum);
	}

	return;

}

xdname(dirp,dotinum)
register struct	direct	*dirp;
register ino_t	dotinum;
{

	register ino_t	dino = dirp->d_ino;

	if (dino==0 || EQ(".",dirp->d_name) || EQ("..",dirp->d_name))
		return;

	if (dino > g_inodes) 
		errorx("Improbable inumber %d. Check filesystem.\n",dino);

	if (Ddlist[dino].inum == 0) {
		Ddlist[dino].inum = dino;
		Ddlist[dino].pinum = dotinum;
		strncpy(Ddlist[dino].dname, dirp->d_name, DIRSIZ);
	}
	else
		addlink(dino,dotinum,dirp->d_name);

	return;

}

monikers()
{

	register int	i;
	char	Pathname[1024];

	pIreq = Ireqlist;

	for (i=0; i<nIreq; i++, pIreq++) {
		name(pIreq->inum, Pathname, 0);
		format(Pathname, pIreq->inum, pIreq);
	}

	if (runopt&F_LINKSTOO)
		for (i=0; i<nLink; i++) {
			name(Linklist[i].pinum, Pathname, Linklist[i].dname);
			format(Pathname,Linklist[i].inum,0);
		}

	return;

}


format(Path,inum,pi)
register	char *Path;
register	ino_t	inum;
register	struct Ireq *pi;
{
	char	*uidtonam(), *nam;

	printf(Path);
	if (!prtopt)
		printf("\n");
	else {
		if (prtopt&F_INODE)
			printf("\t%d",inum);
		if (prtopt&F_SIZE)
			printf("\t%d",pi->size);
		if (prtopt&F_UID) {
			if (*(nam=uidtonam(pi->uid)) == '?')
				printf("\t%d",pi->uid);
			else
				printf("\t%8s",nam);
			}
		printf("\n");
		}

}

name(curi,Path,link)
register ino_t	curi;
register char	*Path;
char	*link;
{
	struct  P  {
		char	p[15];
		} P[50];
	int	n = 0;

	if (prtopt&F_PREPATH)
		strcpy(Path, g_path);
	else
		strcpy(Path, ".");

	if (link)
		strncpy(P[n++].p, link, DIRSIZ);

	for (; curi > 2; curi=Ddlist[curi].pinum) { 
		if (n > 50)
			exceed(50,"path components");
		if (Ddlist[curi].inum == 0) {
			strcpy(P[n++].p, "??unresolved??");
			badnames++;
			break;
		}
		strncpy(P[n++].p,Ddlist[curi].dname,DIRSIZ);
	}
	while(n > 0) { 
		strcat(Path,"/");
		strncat(Path,P[--n].p,DIRSIZ);
	}
	return;

}

/*
 * convert uid to login name; interface to getpwuid that keeps up to USIZE1
 * names to avoid unnecessary accesses to passwd file
 * returns ptr to NSZ-byte name (not necessarily null-terminated)
 * returns pointer to "?" if we cannot resolve uid
 * lifted gleefully from the Unix accounting package. "Software Tools" indeed.
 */

#define	NSZ	8	/* max length of a login name */
#define USIZE1	50
typedef	unsigned	short	uid_t;
static	usize1;
static struct ulist {
	char	uname[NSZ];
	uid_t	uuid;
	} ul[USIZE1];

char *
uidtonam(uid)
uid_t	uid;
{
	register struct ulist *up;
	struct passwd *getpwuid();
	register struct passwd *pp;

	for (up = ul; up < &ul[usize1]; up++)
		if (uid == up->uuid)
			return(up->uname);
	setpwent();
	if ((pp = getpwuid(uid)) == NULL)
		return("?");
	else {
		if (usize1 < USIZE1) {
			up->uuid = uid;
			strncpy(up->uname, pp->pw_name, sizeof(up->uname));
			usize1++;
		}
		return(pp->pw_name);
	}
}

int indir()
{

	int	i;
	char	*bp, *getbuf();

	pIb = Iblklist;
	for (i=0; i<nIb; i++, pIb++) {
		g_size = (*pIb).size - g_bsize*(NADDR-3);

		if ((*pIb).single) {
			bp=getbuf(ALLOC);
			seer((*pIb).single, bp);
			ptrblk(bp, (*pIb).inum, DATA);
			getbuf(FREE,bp);
		}

		if ((*pIb).dubble) {
			bp=getbuf(ALLOC);
			seer((*pIb).dubble, bp);
			ptrblk(bp, (*pIb).inum, SINGLE);
			getbuf(FREE,bp);
		}

		if ((*pIb).triple) {
			bp=getbuf(ALLOC);
			seer((*pIb).triple, bp);
			ptrblk(bp, (*pIb).inum, DOUBLE);
			getbuf(FREE,bp);
		}

	}

	return;

}

ptrblk(buf,inum,type)
daddr_t	buf[NINDIR];
ino_t	inum;
int	type;
{

	int	i = 0;
	char	*bp, *getbuf();

	while (i < NINDIR) {
		if (*buf)
			switch (type) {

			case DATA:
				adddirb(inum, *buf, (off_t)(g_size > g_bsize ? g_size-=g_bsize, g_bsize : g_size));
				break;

			case SINGLE:
				bp=getbuf(ALLOC);
				seer(*buf, bp);
				ptrblk(bp, inum, DATA);
				getbuf(FREE,bp);
				break;

			case DOUBLE:
				bp=getbuf(ALLOC);
				seer(*buf, bp);
				ptrblk(bp, inum, SINGLE);
				getbuf(FREE,bp);
				break;

			}
		buf++;
		i++;
	}

	return;

}

char *getbuf(type,buf)
int	type;
char 	*buf;
{

	char	*p;

	switch(type) {

	case ALLOC :
		if((p=malloc(g_bsize)) == 0)
			errorx("out of memory\n");
		return(p);

	case FREE :
		free(buf);
		return;
	}
}

int seer(blk, buf)
daddr_t	blk;
char	*buf;
{

	long	l,s;
	int	r;

#ifdef FsMAGIC
	s = (blk<2 ? blk*DEV_BSIZE : blk*g_bsize); 
#else
	s = blk*g_bsize;
#endif

	l = lseek(g_fpi, s, 0);
	r = read(g_fpi, buf, g_bsize);

	if (l<0 || r< 0)
		errorx("I/O failed for block %d. seek()=%d read()=%d,err=%d\n",
			blk,l,r,errno);
	if (runopt&F_DEBUG)
		error("seek to %d (%dblks), read %d\n",s,s/g_bsize,r);

	return(r);

}


getblk(buf)
char *buf;
{

	int i;

	if ((i=read(g_fpi, buf, g_bsize)) < 0 || i!=g_bsize)
		errorx("read failed in getblk(), %s\n",
			(errno<sys_nerr?sys_errlist[errno]:"?? errno"));

}

int inodes(args)
char *args;
{

	char	*calloc(), *strtok(), *p;
	int	nN, n, atoi();
	int	first = 0;

	nN = 0;

	if ((pN=Nlist=(ino_t *) calloc(MAXNLIST,sizeof(ino_t))) == 0) {
		error("calloc failed in inodes()\n");
		return(0);
	}

	while ((p=strtok((first++==0?args:0),", ")) != NULL) {
		if ((n=atoi(p)) <= 0)
			error("-i value, %d, ignored\n",n);
		else {
			*pN++ = n;
			if (nN++ > MAXNLIST)
				exceed(MAXNLIST,"-i inumber list");
		}
	}

	return(1);
}


int ncheck(curi)
register ino_t curi;
{

	ino_t	*p;

	for (p=Nlist; p<pN ; p++ )
		if (*p == curi)
			return(1);
	return(0);
}

exceed(max, s)
long	max;
char	*s;
{

	errorx("program limit of %d exceeded for %s\n",max,s);

}


error(s,  e,z,  g,o,l,d,m,a,n)
{

	fprintf(stderr,"%s: %s: ", g_cmd, g_dev);

	fprintf(stderr, s,  e,z,  g,o,l,d,m,a,n);
	
}

errorx(s,  e,z,  g,o,l,d,m,a,n)
{

	error(s,  e,z,  g,o,l,d,m,a,n);
	exit(1);

}


int bcomp(a,b)
struct	Db	*a, *b;

{

	return( (*a).blkno - (*b).blkno);

}
