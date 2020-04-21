char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";

/*	@(#)frec.c	1.2	*/
/*

	frec - fast recover

	This program will recover files, specified by their inode
	numbers, from a filesystem dump tape created by volcopy(1)
	or finc(1).

	strategy:

	- process command line operands and save requested inode numbers
	  in Ireq[]
	- read ilist from tape and for each inode requested save its
	  block numbers in nodes of the Bdata list.
	- driven by the Bdata list, read blocks from the tape:
		- if a block is data, write it into its recovery file.
		- if a block is an indirect block, add the blocks it
		  refernces to the Bdata list.
	- delete recovered blocks from the list and continue spinning
	  tapes until the list is empty.

	dependencies:

	- the input tape header should be in the format of labelit, with
		info from finc(1) or volcopy(1).



*/
/*eject*/
#define	EQ(x,y)	(strcmp(x,y)==0)
#define	OK	1
#define	NOTOK	0
#define	DIRECT	10
#define	INITIO	-1
#define	NODATA	-1
#define	DATA	0
#define	SINGLE	1
#define	DOUBLE	2
#define	TRIPLE	3
#define	HEAD	9
#define	MAXL	256	/* max length of a request line from a -f file */

#include <sys/param.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/filsys.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

extern	int	errno;
/*eject*/
char	*usage = "[-p /path]  [-f file]  tape-dev  inumber:newname  ...";

/*
 *	This structure must be synchronised with volcopy and finc
 */
struct Thdr {
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

struct	Ireq {
	ino_t	inum;
	off_t	residual;
	long	lastblk;
	ushort	uid;
	ushort	gid;
	char	*fp;
	int	status;
	};
struct	Ireq	*pIreq, *Ireqlist;
long	nIreq = 0;

struct	Bdata {
	int	type;
	ino_t	inum; 
	daddr_t	blkno;
	off_t	count;
	int	bindex;
	struct	Ireq	*pip;
	struct	Bdata	*rlink;
	struct	Bdata	*llink;
	}; 
struct	Bdata	*g_Bhead;

daddr_t	*Blklist;
long	nBlks;
/*eject*/
struct	stat	statb;

int	Finc = 0;	/* type of tape */
int	Vcop = 0;

int	g_bufn,		/* index into buffer pool */
	g_bsize = BSIZE,	/* BSIZE of tape */
	g_nindir,	/* NINDIR of tape */
	g_curb = -1,	/* current real or relative block number */
	g_blksize,	/* from tape hdr, blksize in bytes */
	g_nblocks,	/* " "  " " blocking factor in BSIZE's */
	g_reelblks,	/* size of reel in blocks, tape capacity */
	g_reel,		/* reel number */
	g_ilb,		/* ilist size in blocks, from super block */
	g_inodes,	/* number of inodes in the ilist */
	g_inopb,	/* inodes per block */
	g_fpi,		/* input file pointer */
	g_fstblk,	/* real block number of first block on tape */
	g_lstblk;	/* real blk no. of last block on tape */

char	*g_cmd,		/* command name as we were invoked */
	*g_ibuf,	/* our buffer pool pointer */
	*g_idev,	/* input device name */
	*g_path,	/* prefixing path string from command line */
	*g_fsname[15];	/* from tape super block */

/*eject*/
main(argc,argv)
int argc;
char **argv;
{

	int	c, filect, errcnt, getopt(), recover(), icomp(), savereq();
	char	*malloc(), *fgets(), str[MAXL];
	FILE	*ifile, *fopen();
	struct	Bdata	*Bp, *delnode();
	struct 	Ireq	*pi;
	extern	int	optind;
	extern	char	*optarg;

	g_cmd = argv[0];
	g_idev = "";
	if (argc < 3)
		goto bad;
	g_path = "";
	ifile = NULL;
	while ((c=getopt(argc,argv,"f:p:")) != EOF)
		switch (c) {
		case 'f':
			if ((ifile=fopen(optarg,"r")) == NULL) 
				errorx("open failed for -f %s file\n", optarg);
			for(filect=0; fgets(str,MAXL,ifile)!=NULL; filect++)
				;
			rewind(ifile);
			break;

		case 'p':
			g_path = optarg;
			if ((stat(g_path,&statb) == 0) && ((statb.st_mode&S_IFMT) == S_IFDIR))
				break;
			errorx("-p path %s isn't a directory or just plain isn't\n",g_path);
		default:
		bad:	errorx("usage: %s %s\n",g_cmd,usage);
		}

	g_idev = argv[optind++];


	init();

	if (Finc)
		index();

	super();

	if ((pIreq=Ireqlist=(struct Ireq *)malloc((unsigned)((argc+filect)*sizeof(struct Ireq)))) == 0)
		errorx("insufficient area for Ireq array\n");

	errcnt=0;
	if (ifile != NULL)
		while (fscanf(ifile,"%255s",str)!=EOF)
			errcnt+=savereq(str);
	for ( ; optind<argc; optind++) 
		errcnt+=savereq(argv[optind]);
	if (errcnt < 0) 
		errorx("usage: %s %s\n",g_cmd,usage);

	qsort((char *)Ireqlist, nIreq, sizeof(struct Ireq), icomp);

	ilist();

	recover();

	exit(0);

}
/*eject*/
int init()
{

	char	*malloc();

	if (tpopen() == NOTOK)
		exit(1);
	if (Thdr.t_reel != 1) {
		error("wrong reel. need reel 1 and you mounted %d\n",Thdr.t_reel);
		newtp(1);
	}
	g_reel = 1;
	g_reelblks = Thdr.t_reelblks;
	g_blksize = Thdr.t_blksize;
	g_nblocks = Thdr.t_nblocks;
	g_bsize = g_blksize / g_nblocks;

	if ((g_ibuf=(char *)malloc(g_blksize)) <= 0) 
		errorx("insufficient memory for buffer area\n");

	return;

}
/*eject*/
int super()
{

	long	bs;
	char	*getblk();
	struct	filsys	*fs;

	clrbuf();
	getblk(); /* skip boot block */
	fs = (struct filsys *)getblk();
	strncpy(g_fsname,fs->s_fname,6);
	g_fstblk = fs->s_isize;
	g_lstblk = fs->s_fsize;
	g_ilb = g_fstblk - 2;
#if FsTYPE == 3
	g_bsize = (fs->s_magic!=FsMAGIC ? BSIZE :
			(fs->s_type==1 ? 512 :
			(fs->s_type==2 ? 1024 :
			errorx("unknown filesystem BSIZE on tape\n"))));
#else
	g_bsize = BSIZE;
#endif

	g_nblocks = g_blksize / g_bsize;
	g_nindir = g_bsize/sizeof(daddr_t);
#ifdef debug
	error("reelblks:%d  blksize:%d  nblocks:%d  bsize:%d\n",
		g_reelblks, g_blksize, g_nblocks, g_bsize);
#endif

	g_Bhead = (struct Bdata *)malloc(sizeof(struct Bdata));
	g_Bhead->blkno = 0;
	g_Bhead->rlink = g_Bhead->llink = g_Bhead;
	g_Bhead->type = HEAD;

	g_inopb = g_bsize/sizeof(struct dinode);
	g_inodes = g_ilb*g_inopb;

	if (g_fstblk<=0 ||g_lstblk<=0 ||g_ilb<=0 ||
		g_fstblk>g_lstblk ||g_ilb>g_lstblk) 
		errorx("Invalid super block on tape\n");
#ifdef debug
	error("bsize:%d  fstblk:%d  lstblk:%d  ilb:%d  inodes:%d  inopb:%d\n",
	g_bsize, g_fstblk, g_lstblk, g_ilb, g_inodes, g_inopb);
#endif

	return;

}
/**/
ilist()
{

	int	ipb, block;
	int	curi = 0;
	char	*getblk();
	struct	Ireq	*pi;
	struct	dinode	*dip;

	pi = Ireqlist;
	for (block=0; block<g_ilb && pi<pIreq; block++) {
		dip=(struct dinode *)getblk();
		if (Finc) {
			block = Blklist[g_curb] - 2;
			curi = (block*g_inopb);
		}
		for (ipb=0; ipb<g_inopb && pi<pIreq; ipb++) {
			curi++;
			if (curi == pi->inum) {
				saveblks(dip,pi);
#ifdef debug
				error("select inode %d  blk %d\n",
					curi,(Finc?Blklist[g_curb]:g_curb));
#endif
				pi++;
			}
			dip++;
		}

	/*
	check if we have exhausted our ilist, in mapped blocks 
	*/
	if (Finc && Blklist[g_curb+1] >= g_fstblk)  
		break;
	}

	for (pi=Ireqlist; pi<pIreq; pi++)
		if (pi->status == 0)
			error("file %s, inode %d is not on the tape\n",
			pi->fp,pi->inum);

	return;
}

int recover()
{

	int	i, Vrecover(), Frecover();
	struct	Bdata	*Bp;

	Bp = g_Bhead->rlink;

	while (g_Bhead->rlink != g_Bhead) {
		switch(Finc ? Frecover(Bp) : Vrecover(Bp)) {

		case NODATA:
			if (Bp == g_Bhead)
				Bp = Bp->rlink;
			else {
				error("recovery failed for file %s, inode %d, a[%d]:%d\n",
				Bp->pip->fp, Bp->inum, Bp->bindex, Bp->blkno);
				Bp = delnode(Bp);
			}
			break;

		default:
			Bp = delnode(Bp);
			break;
		}
	}

}
/**/
int Vrecover(Bp)
struct	Bdata	*Bp;
{

	int	blksontp; /* number of blocks on a tape reel */
	char	*bufp, *getblk();

	if (Bp->blkno == 0)
		return(NODATA);


	if (Bp->blkno < g_curb || Bp->blkno * g_bsize / BSIZE > g_reel*g_reelblks) 
		newtp((Bp->blkno * g_bsize / BSIZE / g_reelblks)+1);

	blksontp = (g_reelblks*(g_reel+1) < g_lstblk) ? g_reelblks*(g_reel+1) : g_lstblk;
	for (bufp=getblk(); g_curb<blksontp; bufp=getblk() )
		if (g_curb == Bp->blkno) {
			if (Bp->type == DATA)
				datablk(Bp,bufp);
			else
			if (Bp->type==SINGLE
			||Bp->type==DOUBLE
			||Bp->type==TRIPLE)
				ptrblk(Bp,bufp);
			return(Bp->type);
		}

	return(NODATA);

}
/**/
int Frecover(Bp)
register struct	Bdata	*Bp;
{

	char	*bufp, *getblk();
	register daddr_t	*pa, *pz;


	if (Bp->blkno == 0)
		return(NODATA);

	if (Bp->blkno < Blklist[g_curb]) 
		newtp(1);

	for (pa=Blklist,pz=pa+nBlks; pa<pz && *pa != Bp->blkno; pa++)
		;
	if (pa == pz)
		return(NODATA);

	for (;;) {
		bufp = getblk();
		if (Blklist[g_curb] == Bp->blkno) {
			if  (Bp->type == DATA)
				datablk(Bp,bufp);
			else
			if (Bp->type==SINGLE
			||Bp->type==DOUBLE
			||Bp->type==TRIPLE)
				ptrblk(Bp,bufp);
			return(Bp->type);
		}
	}

}

datablk(Bp,bp)
struct	Bdata	*Bp;
char	*bp;
{

	int	fp = -1;
	char	*errmsg = "";

	if ((fp=open(Bp->pip->fp,O_WRONLY)) < 0)  
		errmsg ="Open";
	else
	if (lseek(fp, (long)(Bp->bindex*g_bsize), 0) < 0)
		errmsg = "Seek";
	else
	if (write(fp, bp, Bp->count) < 0)
		errmsg = "Write";

	if (fp >= 0) 
		close(fp);

	if (*errmsg) 
		error("%s failed(%d), can not recover block %d of %s\n",
		errmsg, errno, Bp->bindex, Bp->pip->fp);

	return;

}

ptrblk(Bp,buf)
struct	Bdata	*Bp;
daddr_t	buf[NINDIR];
{
	struct	Bdata	*new, *addnod();
	struct	Ireq	*pi;
	int	i = 0;
	int	done = 0;

	pi = Bp->pip;
	while (i < g_nindir && !done) {
		if (*buf) 
			switch(Bp->type) {

			case SINGLE:
				new = addnod(*buf);
				new->pip = Bp->pip;
				new->type = DATA;
				new->blkno = *buf;
				new->bindex = Bp->bindex+i;
				new->count = (new->bindex==pi->lastblk ? ++done, pi->residual : g_bsize);
				break;

			case DOUBLE:
				new = addnod(*buf);
				new->pip = Bp->pip;
				new->type = SINGLE;
				new->blkno = *buf;
				new->bindex = Bp->bindex + (i*g_nindir);
				new->count = g_bsize;
				break;

			case TRIPLE:
				error("Sorry - triple indirect files can not be recovered\n");
				errorx("File %s inode %d not recovered\n",
				Bp->pip->fp, Bp->inum);
				/*
				new = addnod(*buf);
				new->pip = Bp->pip;
				new->type = DOUBLE;
				new->blkno = *buf;
				new->bindex = 
				new->count = g_bsize;
				break;
				*/

			}
		buf++;
		i++;
	}
	return;
}

int savereq(str)
char	*str;
{

	int	inew;
	char	c, *strchr(), *p, *fnew, *malloc();
	struct	Ireq	*pi;

	fnew=strchr(str,':');
	c = *++fnew;
	if (fnew == NULL || c == '\0' || c == '\n') {
		error("invalid syntax for request %s\n",str);
		return(-1);
	}

	if ((inew=atoi(str)) <= 0 || inew > g_inodes) {
		error("requested inumber %d out of range\n",inew);
		return(-1);
	}

	for (pi=Ireqlist; pi<pIreq; pi++)
		if (pi->inum == inew) {
			error("duplicate request for inumber %d ignored\n",inew);
			return(0);
		}

	if ( *g_path && *fnew != '/' && !(*fnew=='.'&&*(fnew+1)=='/')) {
		p = (char *) malloc(strlen(g_path)+1+strlen(fnew)+1+1);
		strcpy(p,g_path);
		strcat(p,"/");
		strcat(p,fnew);
	}
	else {
		p = (char *) malloc(strlen(fnew) +1);
		strcpy(p,fnew);
	}

	pIreq->inum = inew;
	pIreq->fp = p;
	pIreq->status = 0;
	pIreq++;
	nIreq++;

	return(0);

}
/**/
newtp(reel)
int reel;
{

	FILE	*user;
	char	newtape[24], savevol[6];
	int	tpopen();

	if (reel > Thdr.t_reels)
		errorx("reel %d > total reels %d, aborting!\n", reel, Thdr.t_reels);
	newtape[0]= '\0';
	strncpy(savevol,Thdr.t_volume,6);
	user = fopen("/dev/tty","r");
	if (Vcop && reel != Thdr.t_reel)
		error("prepare to mount reel %d of %s %s\n",reel,g_fsname,Thdr.t_magic);

again:
	if (g_fpi > 0)
		close(g_fpi);

	if (Vcop && reel != Thdr.t_reel)
	while(1) {
		error("Tape Ready? Hit Return to proceed, or `/dev/rmt_' if you swapped drives -");
		fgets(newtape, 24, user);
		newtape[strlen(newtape)-1] = '\0';
		if ((strlen(newtape) == 0) ||
		   ((stat(newtape,&statb)==0) && ((statb.st_mode & S_IFMT) == S_IFCHR) ))
			break;
		error("%s is not a character special file. Try again.\n", newtape);
	}

	if(*newtape)
		g_idev = newtape;

	if (tpopen()==NOTOK)
		goto again;

	if(Thdr.t_reel != reel || strncmp(Thdr.t_volume,savevol,6) != 0) {
		error("Wrong tape mounted. Requested reel %d of %s not reel %d\n",
		reel, savevol, Thdr.t_reel);
		goto again;
	}

	fclose(user);
	g_reel = Thdr.t_reel;

	if (Finc) 
		index();
	else {
 /* Volcopy */
		g_curb = ((g_reel - 1) * g_reelblks * BSIZE / g_bsize) - 1;
		clrbuf();
	}

	return;
}

int tpopen()
{

	int	i;
	char	*ctime();

	if (stat(g_idev,&statb) < 0 || (statb.st_mode&S_IFMT) != S_IFCHR) {
		error("input %s is non-existent or not a raw device\n",g_idev);
		return(NOTOK);
	}

	if ((g_fpi=open(g_idev,O_RDONLY)) < 0) {
		error("open failed(%d) for %s\n",errno,g_idev);
		return(NOTOK);
	}

	if ((i=read(g_fpi,&Thdr,sizeof(struct Thdr))) < 0 
		|| i != sizeof(struct Thdr)) {
		error("read failed(%d) on read of tape header.\n",
			(i==-1)?errno:i);
		return(NOTOK);
	}

	if ( !EQ(Thdr.t_magic,"Volcopy") && !EQ(Thdr.t_magic,"Finc") ) {
		error("tape was not written by finc or volcopy\n");
		return(NOTOK);
	}

	error("%s of %.6s (reel %d/%d) made on %s",
	Thdr.t_magic,Thdr.t_volume,Thdr.t_reel,Thdr.t_reels,ctime(&Thdr.t_time));

	if (EQ(Thdr.t_magic,"Finc"))
		Finc++;
	else
		Vcop++;

	return(OK);

}
/**/
index()
{

	int	quantity, ntotal;
	char	*getblk(), *malloc();
	daddr_t	*pb, *pB;

	ntotal = nBlks = Thdr.t_reelblks;
	if (nBlks <= 0 )
		errorx("defective header on Finc tape. No block count\n");

	Blklist = (daddr_t *) malloc((unsigned)(nBlks*sizeof(daddr_t)));
	
	clrbuf();
	pB = Blklist;
	quantity = 0;
	while (ntotal--) {
		if (! quantity) {
			pb = (daddr_t *) getblk();
			quantity = g_bsize/sizeof(*pb);
		}

		*pB++ = *pb++;
		--quantity;
	}

	g_curb = -1;
	clrbuf();
	return;

}

int saveblks(dip,pi)
struct	dinode	*dip;
struct	Ireq	*pi;
{

	daddr_t	l[NADDR];
	struct	Bdata	*Bp, *addnod();
	int	i, size;

	pi->status++;  /* found, although not yet accepted. */
	if (dip->di_mode == 0) { 
		error("file %s, inode %d was unallocated\n",pi->fp,pi->inum);
		return(NOTOK);
	}


	if (make(dip,pi) != OK)
		return(NOTOK);

	if ((dip->di_mode&IFMT)==IFDIR)
		return(OK);

	l3tol(l,dip->di_addr,NADDR);
	if (l[NADDR-1]) {
		error("Sorry! file %s inode %d is triple indirect.\n",
			pi->fp,pi->inum);
		return(NOTOK);
	}

	size = dip->di_size;
	pi->lastblk = ((size + g_bsize - 1) / g_bsize) - 1;
	if ((pi->residual=size%g_bsize) == 0)
		pi->residual = g_bsize;

#ifdef u370
	/* Small Block UNIX/370 */
	if (dip->di_sbmode == SBAUSE) {
		Bp = addnod((daddr_t) 0);
		Bp->inum = pi->inum;
		Bp->pip = pi;
		Bp->type = DATA;
		Bp->blkno = (daddr_t) 0;
		Bp->bindex = (daddr_t) 0;
		Bp->count = size;
		datablk(Bp,dip->di_data);
		delnode(Bp);
		size = 0;

	}
#endif

	for (i=0; i<NADDR-3 && size>0; i++) {
		if (l[i] == 0)
			continue;
		Bp = addnod(l[i]);
		Bp->inum = pi->inum;
		Bp->pip = pi;
		Bp->type = DATA;
		Bp->blkno = l[i];
		Bp->bindex = i;
		Bp->count = (size > g_bsize ? size -= g_bsize, g_bsize : size);
	}

	if ( l[NADDR-3] ) {
		Bp = addnod(l[NADDR-3]);
		Bp->inum = pi->inum;
		Bp->pip = pi;
		Bp->type = SINGLE;
		Bp->blkno = l[NADDR-3];
		Bp->bindex = DIRECT;
		Bp->count = g_bsize;
	}
	if ( l[NADDR-2] ) {
		Bp = addnod(l[NADDR-2]);
		Bp->inum = pi->inum;
		Bp->pip = pi;
		Bp->type = DOUBLE;
		Bp->blkno = l[NADDR-2];
		Bp->bindex = DIRECT + g_nindir;
		Bp->count = g_bsize;
	}
	/*if ( l[NADDR-1] ) {
		Bp = addnod(l[NADDR-1]);
		Bp->inum = pi->inum;
		Bp->pip = pi;
		Bp->type = TRIPLE;
		Bp->blkno = l[NADDR-1];
		Bp->bindex = DIRECT + g_nindir*g_nindir;
		Bp->count = g_bsize;
	}*/

	return(OK);

}
/*eject*/
int make(dip,pi)
struct	dinode	*dip;
struct	Ireq	*pi;
{

	register int fp, exist;
	struct	utimbuf	{
		time_t	actime;
		time_t	modtime;
		} ft;

	exist = (stat(pi->fp,&statb) != -1);
	switch(dip->di_mode&IFMT) {

	case IFREG:
		if (!exist) {
			pave(dip,pi);
			fp=creat(pi->fp, (int)dip->di_mode);
			if (fp != -1) {
				close(fp);
				fp=0;
			}
		}
		break;
	case IFDIR:
		if (!exist) {
			pave(dip,pi);
			fp=mkdir(pi->fp);
		}
		break;
	default:
		if (!exist) {
			pave(dip,pi);
			fp=mknod(pi->fp, (int)dip->di_mode, (int)dip->di_uid);
			break;
		}
	}
	if (fp == -1) {
		error("creat/mknod failed(%d) for %s, inode %d\n",
		errno, pi->fp, pi->inum);
		return(NOTOK);
	}
	if (chmod(pi->fp, (int)dip->di_mode) < 0) {
		error("chmod failed(%d) for %s, inode %d\n",
		errno, pi->fp, pi->inum);
		return(NOTOK);
	}

	ft.actime = dip->di_atime;
	ft.modtime = dip->di_mtime;
	if (utime(pi->fp, &ft) < 0) {
		error("utime failed(%d) for %s, inode %d\n",
		errno, pi->fp, pi->inum);
		return(NOTOK);
	}

	if (chown(pi->fp, (int)dip->di_uid, (int)dip->di_gid)<0) {
		error("chown failed(%d) for %s, inode %d\n",
		errno, pi->fp, pi->inum);
		return(NOTOK);
	}

	return(OK);
}
/*eject*/
pave(dip,pi)
struct	dinode	*dip;
struct	Ireq	*pi;
{
	register char *np;
	register mstat=0;

	np = pi->fp;
	np = (*np=='.'&&*np+1=='/')?np+2:np;
	np = (*np=='/')?np+1:np;
	for (; *np && mstat != -1 ; ++np)
		if(*np == '/') {
			*np = '\0';
			if(stat(pi->fp, &statb) == -1)
				mstat=mkdir(pi->fp);
			*np = '/';
		}

	return;
}


mkdir(namep)
register char *namep;
{
	static status;
	register pid;

	if(pid = fork())
		while(wait(&status) != pid);
	else {
		close(2);
		execl("/bin/mkdir", "mkdir", namep, 0);
		exit(2);
	}
	return ((status>>8) & 0377)? -1 : 0;
}
/*eject*/
/*
 * the next two functions massage the Bdata link-list. addnode() links in
 * a new node, maintaining the list in right ascending order. delnode()
 * well, I'm sure you can guess what delnode does.
 */

struct	Bdata
*addnod(blk)
daddr_t	blk;
{

	char	*malloc();
	struct	Bdata	*Bp, *new;

	if ((new = (struct Bdata *)malloc(sizeof(struct Bdata))) <= 0) 
		errorx("insufficient space for new block node.\n");

	for (Bp=g_Bhead->rlink; Bp->type != HEAD && blk>Bp->blkno; Bp=Bp->rlink)
		;

	new->rlink = Bp;
	new->llink = Bp->llink;
	(Bp->llink)->rlink = new;
	Bp->llink = new;

	return(new);

}


struct	Bdata
*delnode(Bp)
struct	Bdata	*Bp;
{

	(Bp->llink)->rlink = Bp->rlink;
	(Bp->rlink)->llink = Bp->llink;

	return(Bp->rlink);

}
/**/
char *getblk()
{

	int i;

	if (g_bufn >= g_nblocks || g_bufn == INITIO) {
		g_bufn = 0;
		i = read(g_fpi, g_ibuf, g_blksize);
		if (i < 0 || i != g_blksize) {
			errorx("read failed(%d), %d=read(%d,%d,%d), for block %d\n",
			errno,i,g_fpi,g_ibuf,g_blksize,g_curb+1);
		}
	}
	g_curb++;
	return(&g_ibuf[g_bsize*g_bufn++]);

}

/*
 * clrbuf() forces the next call to getblk() to perform i/o, preventing
 *    getblk() from returning a pointer to a buffer left over from a
 *    previous read. clrbuf() should therefore be called when a new tape
 *    is mounted.
 */
clrbuf()
{

	g_bufn = INITIO;

}



error(s,  e,z,  g,o,l,d,m,a,n)
{

	fprintf(stderr,"%s: %s: ", g_cmd, g_idev);

	fprintf(stderr, s,  e,z,  g,o,l,d,m,a,n);
	
}

errorx(s,  e,z,  g,o,l,d,m,a,n)
{

	error(s,  e,z,  g,o,l,d,m,a,n);
	exit(1);

}

int icomp(a,b)
register struct Ireq *a, *b;
{

	return( (*a).inum - (*b).inum);

}
/*eject*/
/*
 * The following routines may prove helpful in debugging.
 * prtall("string") prints the list of Bdata nodes.
 * prtnode(Bdata ptr) prints one node.
 * sprinkling calls to these routines at various points in the program
 * will result in a trace of link-list activity.
 */

prtall(rtn)
char	*rtn;

{

	struct	Bdata	*Bp;

	error("%s Bdata dump\n",rtn);
	Bp = g_Bhead;
	do {
		prtnode(Bp);
	} while((Bp=Bp->rlink) != g_Bhead);
	error("\n");

	return;

}


prtnode(Bp)
struct	Bdata	*Bp;

{

	fprintf(stderr,
	"\nBp:%x\n  type:%d  inum:%d  blkno:%d  count:%d  bindex:%d (%x:%x)\n",
	Bp,Bp->type,Bp->inum,Bp->blkno,Bp->count,Bp->bindex,Bp->rlink,Bp->llink);
	/*
	fprintf(stderr,
	"\nIreq:%x\n  inum:%d  res:%d  lstblk:%d  into:%s\n",
	Bp->pip, Bp->pip->inum, Bp->pip->residual, Bp->pip->lastblk, Bp->pip->fp);
	*/

	return;

}
