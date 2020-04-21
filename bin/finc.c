char _Origin_[] = "System V";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/finc.c,v 1.1 89/03/27 14:50:27 root Exp $";
/*
 * $Log:	finc.c,v $
 * Revision 1.1  89/03/27  14:50:27  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  85/02/04  15:48:59  bob
 * Changed to use DEV_BSIZE instead of BSIZE to work with our implementation.
 * 
 */

/*
 *	@(#)finc.c	1.2
 *
 *	finc - fast incremental backup
 *
 *	given an input filesystem and selection criteria, finc will
 *	write a copy of the ilist and all blocks owned by selected
 *	files to tape, as an incremental backup.
 *
 *	strategy:
 *
 *	- finc processes his command line, for selection criteria.
 *	- prepare for the datacopy by reading the input filesystem
 *	  ilist and selecting files.
 *	- for any selected file, add its block number to the list
 *	  pointed to by Blks.
 *	- sort Blks to optimize seek time.
 *	- do the datacopy but first write a volcopy style tape label.
 *	- followed by a vector of block numbers in the order they will
 *	  appear on tape.
 *	- followed by the data blocks themselves, as read from the fs.
 *
 *	data areas:
 *
 *	- Blklist points to a vector of requested block numbers. A block
 *	  number is requested by its appearance in the inode or indirect
 *	  blocks of a selected file.  Blks is a pointer incremented over
 *	  Blklist. Blklist has nBlks entries.
 *	- An Iblk structure holds info allowing the collection of block
 *	  numbers from an indirect block, and their addition to the Blklist.
 *
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/filsys.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define EQ(x,y)	(strcmp(x,y)==0)
#ifdef m68000
#define	OBLKFACT	200
#define	ONE_REEL	67200   /* OBLKFACT*BSIZE blocks per 2400 reel at 1600 bpi */
#endif
#ifdef vax
#define	OBLKFACT	10
#define	ONE_REEL	67200   /* OBLKFACT*BSIZE blocks per 2400 reel at 1600 bpi */
#endif
#ifdef u370
#define	OBLKFACT	7
#define	ONE_REEL	35000   /* blocks per 2400 reel at 6250 bpi */
#endif
#ifdef u3b
#define	OBLKFACT	4
#define	ONE_REEL	52800   /* OBLKFACT*BSIZE blocks per 2400 reel at 1600 bpi */
#endif


#define	INITIO	-1l	/* must be -1 so first ++g_bufn results in 0 */
#define	FLUSHIO	-2l

#define	ALLOC	0
#define	FREE	1

#define	DATA	1
#define	SINGLE	2
#define	DOUBLE	3
#define	TRIPLE	4
#define	A_DAY	86400l

#define	F_DEBUG	1

/*
	Data Structures
*/


/*
	The Blklist is a vector of block numbers which we plan to
	write on the finc tape.
*/
long	*pBlk, *Blklist;
long	maxBlks, nBlks;


/*
	The Iblk structure holds information on indirect blocks.
	If a file is to be backed up, then we have to grab all
	of its corporeal blocks, including those that are indirectly
	addressed.
*/
struct	Iblk {
	long	single;
	long	dubble;
	long	triple;
	};
struct	Iblk	*pIb, *Iblklist;
long	maxIb, nIb;
long	g_select = 0;


char	*usage = "[-a #]  [-c #]  [-m #]  [-n file]  /inputdev /outputdev";

extern	int	errno;
extern	int	sys_nerr;
extern	char	*sys_errlist[];	

struct	stat	statb;

struct	Thdr	{
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time;
	long	t_length;
	long	t_dens;
	long	t_reelblks;
	long	t_blksize;
	long	t_nblocks;
	char	t_fill[472];
	} Thdr;


int	g_bufn,		/* internal buffer allocation counter */
	g_ilb,		/* ilist size in blocks */
	g_fstblk,	/* first non-ilist block */
	g_lstblk,	/* last filesystem block */
	g_inodes,	/* ilist size in inodes */
	g_inopb,	/* number of inodes in a block */
	g_bsize,	/* actual filesystem block size */
	g_fpo;		/* output tape file pointer */
	g_fpi;		/* input filesystem file pointer */

char	*g_cmd,		/* our name, as invoked by the user */
	*g_idev,	/* name of input filesystem we are processing */
	*g_odev,	/* name of output device we are processing */
	*g_path,	/* optional prefix path for generated names */
	*g_bp,		/* a global char * buffer pointer */
	*g_obuf;	/* output tape buffer */

int	criteria;	/* count of user selection criteria */

char	runopt,		/* flags - run-time options */
	as,		/* sign of number for -a option */
	ms,		/* "        "      "  -m option */
	cs;

long	atime,		/* abs(number) of days from -a option */
	mtime,
	ctime,		/* "         "           "  -c option */
	ntime,
	today;		/* today as returned by time() */

main(argc,argv)
int argc;
char **argv;
{

	extern	char	*optarg;
	extern	int	optind;
	int	stat();
	int	c;

	g_cmd = argv[0];
	g_idev = "";
	g_odev = "";
	if (argc < 3)
		goto bad;
	runopt=0;
	time(&today);
	while((c=getopt(argc,argv,"da:m:c:n:")) != EOF)
		switch(c) {

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
			if ((ctime = atoi(optarg)) == 0) 
				errorx("bad value %s for option c\n",optarg);
			++criteria;
			break;
		case 'n':
			if (stat(optarg,&statb) < 0) 
				errorx("n option, %s stat failed\n",optarg);
			++criteria;
			ntime = statb.st_mtime;
			break;

		default:
		bad:	errorx("Usage: %s %s\n",g_cmd,usage);

		}
	if (criteria == 0) 
		error("No selection criteria.\n");


	g_idev = argv[optind++];
	g_odev = argv[optind];

#ifdef u370
	if (stat(g_idev,&statb) < 0 || (statb.st_mode&S_IFMT) == S_IFCHR)
		errorx("Input %s is non-existent or a raw device (char special)\n",g_idev);
#else
	if (stat(g_idev,&statb) < 0 || (statb.st_mode&S_IFMT) != S_IFCHR)
		errorx("Input %s is non-existent or not a raw device (char special)\n",g_idev);
#endif

	if (stat(g_odev,&statb) < 0 || (statb.st_mode&S_IFMT) != S_IFCHR) 
		errorx("Output %s is non-existent or not raw (char special)\n",g_odev);

	if ((g_fpi=open(g_idev,O_RDONLY)) <= 0)
		errorx("Open failed for input, %s\n",g_idev);

	if ((g_fpo=open(g_odev,O_RDONLY)) <= 0) 
		errorx("Open failed for output %s\n",g_odev);

	sync();

	choose();

	datacopy();

	exit(0);

}

/*
	Inode scan phase - selects files to "finc" and accumulates
	their block numbers in the Blklist.
*/
int choose()
{

	int	bcomp();
	char	*clrbuf();


	super();

	addblk(0);  /* boot block */
	addblk(1);  /* super block */

	ilist();

	indir();
	qsort((char *)Blklist, nBlks, sizeof(long), bcomp);

	return;

}


int bcomp(a,b)
register long *a, *b;
{

	return( *a - *b );

}

/*
	Tape phase - writes header, block index, and selected
	blocks onto the output tape.
*/
datacopy()
{
	int	i;
	char	*bp, *clrbuf(), *putblk(), *malloc();


	if (nBlks == 0) {
		error("No data meets selection criteria\n");
		return;
	}
	else
		error("%d files (%d blocks) selected\n", g_select, nBlks);

	if ((g_obuf=malloc(g_bsize*OBLKFACT)) == 0)
		errorx("insufficient memory for output buffer\n");

	header();
	index();
	flshbuf();	/* to clear index data */

	bp = clrbuf();
	pBlk = Blklist;
	for (i=0; i<nBlks; i++, pBlk++) { 
		seer( (*pBlk), bp);
		if (runopt&F_DEBUG)
			error("cp blk %d to %8.8x\n",*pBlk,bp);
		bp = putblk();
	}

	flshbuf();
	error("datacopy done\n");

	return;

}

/*
	reads the super block of the input filesystem to get those
	important fields which are needed by everyone else. Also
	gets storage for the data structures whose sizes are 
	based on super block values.
*/
int super()
{

	char	*bp, *malloc(), *getbuf();
	struct	filsys	*fs;

	g_bsize = DEV_BSIZE;
	bp=getbuf(ALLOC);
	seer(1, bp);
	fs = (struct filsys *)bp;

	g_fstblk = fs->s_isize;
	g_lstblk = fs->s_fsize;
	g_ilb = g_fstblk - 2;
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

	if (g_fstblk<=0 ||g_lstblk<=0 ||g_ilb<=0 ||
		g_fstblk>g_lstblk ||g_ilb>g_lstblk) 
		errorx("Invalid super block\n");
	if (runopt&F_DEBUG)
		error("bsize:%d  fstblk:%d  lstblk:%d  ilb:%d  inodes:%d  inopb:%d\n",
		g_bsize, g_fstblk, g_lstblk, g_ilb, g_inodes, g_inopb);

	
	getbuf(FREE,bp);


	maxBlks = (g_lstblk > ONE_REEL ? ONE_REEL : g_lstblk);
	maxIb = maxBlks*0.30;

	if ((pBlk = Blklist = (long *)malloc(maxBlks*sizeof(long))) == 0)
		errorx("insufficient memory for Blklist\n");

	if ((pIb = Iblklist = (struct Iblk *)malloc(maxIb*(sizeof *pIb))) == 0)
		errorx("insufficient memory for Iblk area\n");

	nIb = nBlks = 0;

	if (runopt & F_DEBUG)
		error("maxBlks:%d  maxIb:%d  obuf:%d\n",
		maxBlks, maxIb, g_bsize*OBLKFACT);

	return;

}

/*
	reads the ilist blocks and examines each inode.
*/
int ilist()
{

	int	ipb, block; 
	int	found = 0;
	ino_t	curi = 0;
	char	*bp, *getbuf();
	struct	dinode	*dip;

	bp=getbuf(ALLOC);

	for (block=2; block<g_fstblk; block++) {

		seer(block, bp);
		dip = (struct dinode *)bp;
		found = 0;

		for (ipb=0; ipb<g_inopb; ipb++, dip++) {
			curi++;
			if (dip->di_mode == 0 || curi == 1)
				continue;
			if (select(dip)) {
				if (runopt&F_DEBUG)
					error("select inode %d\n",curi);
				saveblks(dip);
				++found;
				++g_select;
			}
		}
		if (found)
			addblk(block);
	}

	getbuf(FREE,bp);

	return;
}

/*
	returns pointer to next g_bsize'd buffer in g_obuf.
	does actual i/o if required.
*/
char *putblk()
{

	int	i;

	if (g_bufn == INITIO)
		++g_bufn;

	if (g_bufn == OBLKFACT-1 || g_bufn == FLUSHIO) {
		i = write(g_fpo, g_obuf, g_bsize*OBLKFACT);
		if (runopt&F_DEBUG)
			error("writing a block\n");
		if (i<=0 || i != g_bsize*OBLKFACT)
			errorx("i/o failed to tape. err=%s\n",
			errno<sys_nerr?sys_errlist[errno]:"??");
		g_bufn = INITIO;  /* MUST be -1 */
	}

	return(&g_obuf[g_bsize*(++g_bufn)]);

}


/*
	returns pointer to start of g_obuf, and sets flag
	to initialize buffer management.
*/
char *clrbuf()
{
	g_bufn = INITIO;
	return(g_obuf);
}


/*
	intended to force out the g_obuf to tape
	*/
flshbuf()
{
	g_bufn = FLUSHIO;
	putblk();
	return;
}

/*
	we copy the block index to tape. It is used by frec(1M)
	to calculate a tape blocks physical block number.
*/
index()
{

	int	i;
	long	l = 0;

	g_bp = clrbuf();

	indxput(INITIO);

	i = nBlks;
	pBlk = Blklist;
	while (i--)
		indxput(*pBlk++);

	indxput(FLUSHIO);

	return;

}

/*
	we use the large g_obuf, and g_bp as set above, to fill
	as many g_bsize sized buffers as are needed with the
	block index.
*/
indxput(l)
long	l;
{

	static	long	maxbufd;
	static	long	*bufp;

	if (l == INITIO) {
		maxbufd = g_bsize / sizeof(l);
		bufp = (long *)g_bp;
		return(1);
	}

	if (l == FLUSHIO) {
		g_bp = putblk();
		indxput(INITIO);
		return(1);
	}

	*bufp++ = l;

	if (! --maxbufd)
		indxput(FLUSHIO);

	return(1);

}

/*
	When we read the ilist we saved the block pointers that were
	indirect. Now for each file's set (of 3 possible), we add the
	blocks it references to the Blklist list.
*/
int indir()
{

	int	i;
	char	*bp, *getbuf();

	pIb = Iblklist;
	for (i=0; i<nIb; i++, pIb++) {

		if ((*pIb).single) {
			bp=getbuf(ALLOC);
			seer((*pIb).single, bp);
			ptrblk(bp,DATA);
			getbuf(FREE,bp);
		}

		if ((*pIb).dubble) {
			bp=getbuf(ALLOC);
			seer((*pIb).dubble, bp);
			ptrblk(bp,SINGLE);
			getbuf(FREE,bp);
		}

		if ((*pIb).triple) {
			bp=getbuf(ALLOC);
			seer((*pIb).triple, bp);
			ptrblk(bp, DOUBLE);
			getbuf(FREE,bp);
		}

	}

	return;

}

/*
	actually reads the block numbers in a Unix indirect block
*/
ptrblk(buf,type)
daddr_t	buf[NINDIR];
int	type;
{

	int	i = 0;
	char	*bp, *getbuf();

	while (i < NINDIR) {
		if (*buf)
			switch (type) {

			case DATA:
				addblk(*buf);
				break;

			case SINGLE:
				addblk(*buf);
				bp=getbuf(ALLOC);
				seer(*buf, bp);
				ptrblk(bp, DATA);
				getbuf(FREE,bp);
				break;

			case DOUBLE:
				addblk(*buf);
				bp=getbuf(ALLOC);
				seer(*buf, bp);
				ptrblk(bp, SINGLE);
				getbuf(FREE,bp);
				break;

			}
		buf++;
		i++;
	}

	return;

}

/*
	get g_bsize bytes of storage, or free it.
*/
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


/*
	seek to a block, and read in g_bsize bytes there.
*/
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

	/*if (runopt&F_DEBUG)
		error("seek to %d (%dblks), read %d\n",s,s/g_bsize,r);*/

	if (l<0 || r< 0)
		errorx("i/o failed. block:%d   err=%s\n",
			blk, errno<sys_nerr?sys_errlist[errno]:"??");

	return(r);

}

/*
	be choosy about the inodes we look at. See if they pass.
*/
select(dip)
struct dinode *dip;
{

	register int ok = 0;

	if (criteria == 0)
		return(1);

	if (atime) 
		ok += (scomp((int)((today-dip->di_atime)/A_DAY),atime,as)) ? 1 : 0;

	if (mtime) 
		ok += (scomp((int)((today-dip->di_mtime)/A_DAY),mtime,ms)) ? 1 : 0;

	if (ctime) 
		ok += (scomp((int)((today-dip->di_ctime)/A_DAY),ctime,cs)) ? 1 : 0;

	if (ntime) 
		ok += (dip->di_mtime > ntime) ? 1 : 0;

	return(ok == criteria);

}

scomp(a,b,s)
register a,b;
register char s;
{

	if (s == '+')
		return(a > b);
	if (s == '-')
		return(a < (b * -1));
	return(a == b);

}

/*
	for a selected inode, extract all direct blocks and
	save any indirect block pointers.
*/
saveblks(dip)
struct	dinode	*dip;
{

	long	l[NADDR];
	int	i;

	if (dip->di_mode == 0 || dip->di_size == 0)  
		return;

	l3tol(l,dip->di_addr,NADDR);

	for (i=0; i<NADDR-3 ; i++) {  
		if (l[i] == 0)
			continue;
		addblk(l[i]);
	}

	if (l[NADDR-3] || l[NADDR-2] || l[NADDR-1])
		addindb(l[NADDR-3], l[NADDR-2], l[NADDR-1]);

	return;
}

addblk(blk)
long	blk;
{

	if (runopt&F_DEBUG)
		error("in addblk- nBlk:%d  blk:%d\n",nBlks,blk);

	Blklist[nBlks] = blk;
	if (nBlks++ > maxBlks)
		exceed(maxBlks,"Blk - block save list");

	return;
}

addindb(s,d,t)
long	s,d,t;
{

	if (s)
		addblk(s);
	if (d)
		addblk(d);
	if (t)
		addblk(t);
	pIb->single = s;
	pIb->dubble = d;
	pIb->triple = t;
	pIb++;
	if (nIb++ > maxIb)
		exceed(maxIb,"Iblk - indirect block save list");

	return;
}

/*
	Validate the tape header and update it to our needs.
*/
int header()
{

	int	i;

	if ((i=read(g_fpo, &Thdr, sizeof(struct Thdr))) <= 0) 
		errorx("can't read label on %s\n",g_odev);

	if (i < sizeof(struct Thdr)   ||
	(!EQ(Thdr.t_magic,"Label")    &&
	 !EQ(Thdr.t_magic,"Finc")     &&
	 !EQ(Thdr.t_magic,"Volcopy")) ) 
		errorx("tape was not labelled by labelit(1)\n");

	close(g_fpo);
	g_fpo = open(g_odev,O_WRONLY);

	strcpy(Thdr.t_magic,"Finc");
	Thdr.t_reels = 1;
	Thdr.t_reel = 1;
	Thdr.t_time = today;
	Thdr.t_reelblks = nBlks;
	Thdr.t_blksize = g_bsize*OBLKFACT;
	Thdr.t_nblocks = OBLKFACT;

	if (write(g_fpo, &Thdr, sizeof(struct Thdr)) <= 0)
		errorx("write of label failed\n");

	return;

}

exceed(max, s)
long	max;
char	*s;
{

	errorx("program limit of %d exceeded for %s\n",max,s);

}


error(s,  e,z,  g,o,l,d,m,a,n)
{

	fprintf(stderr,"%s: %s->%s: ", g_cmd, g_idev, g_odev);

	fprintf(stderr, s,  e,z,  g,o,l,d,m,a,n);
	
}

errorx(s,  e,z,  g,o,l,d,m,a,n)
{

	error(s,  e,z,  g,o,l,d,m,a,n);
	exit(1);

}
