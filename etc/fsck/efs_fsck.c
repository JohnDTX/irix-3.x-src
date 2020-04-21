# include "efs_fsck.h"
# include "mntent.h"

# define DEBUG fsck_debug

/* patchable flags */
# include "dprintf.h"
short fsck_debug = 0;
short fsck_promptflag = 0;
char Dirc = 0;

# define MINSECTORS	1
# define MAXSECTORS	2000
# define MINHEADS	1
# define MAXHEADS	2000
# define MINCYLS	1
# define MAXCYLS	32000

/* efs - fsck.c */
char	*lfname =	"lost+found";

main(argc, argv)
	int argc;
	char **argv;
{
	char filename[50];
	register i;
	register int n;
	int svargc, argvix;
	int ix;
	char **argx;
	struct stat statbuf;

	auto int fsmagic, fsbshift;
	auto char *checker;

	chkblki = 0;
	pipedev  = -1;
	id = ' ';
	if ('0' <= argv[0][0] && argv[0][0] <= '9')
		id = argv[0][0];

	setbuf(stdin,NULL);
	setbuf(stdout,NULL);
	sync();

	svargc = argc;
	for(i = 1; --argc < 0, argv[i] && *argv[i] == '-'; i++) {
		switch(argv[i][1]) {
		case 'b':
			while(argc && isdigit(argv[i+1][0])) {
				chkblks[chkblki++] = atoi(argv[++i]);
				--argc;
			}
			break;
		case 't':
		case 'T':
			tflag++;
			if(*argv[++i] == '-' || --argc <= 0)
				iderrexit("Bad -t option\n");
			strcpy(scrfile,argv[i]);
			if (stat(scrfile,&statbuf) == 0 &&
				sfiletype(&statbuf) != S_IFREG )
				iderrexit("Illegal scratch file <%s>\n",
					scrfile);
			break;
		case 's':	/* salvage flag */
			stype(argv[i]+2);
			sflag++;
			break;
		case 'S':	/* conditional salvage */
			stype(argv[i]+2);
			csflag++;
			break;
		case 'n':	/* default no answer flag */
		case 'N':
			nflag++;
			yflag = 0;
			break;
		case 'y':	/* default yes answer flag */
		case 'Y':
			yflag++;
			nflag = 0;
			break;
		case 'q':
			qflag++;
			break;
		case 'D':
			Dirc++;
			break;
		case 'F':
		case 'f':
			fast++;
			break;
		default:
			iderrexit("%c option?\n",*(argv[i]+1));
		}
	}

	if(nflag && sflag)
		iderrexit("Incompatible options: -n and -s\n");
	if(nflag && qflag)
		iderrexit("Incompatible options: -n and -q\n");
	if(sflag && csflag)
		sflag = 0;
	if(csflag) nflag++;

	if (argc == 0) {	/* use fstab as default checklist */
		register FILE *fstabp;
		register struct mntent *mntp;

		if ((fstabp = setmntent(MNTTAB,"r")) == NULL) {
			iderrexit("Can't open checklist file: %s\n", MNTTAB);
		}
		n = 0;
		while ((mntp = getmntent(fstabp)) != NULL) {
			if (!strcmp(mntp->mnt_type, MNTTYPE_EFS)
			    || !strcmp(mntp->mnt_type, MNTTYPE_BELL))
				n++;
		}
		argx = (char **)calloc(svargc + n + 1, sizeof *argx);
		if (argx == NULL) {
			iderrexit("Out of memory\n");
		}
		for (n = 0; n < svargc; n++) {
			argx[n] = argv[n];
		}
		rewind(fstabp);
		while ((mntp = getmntent(fstabp)) != NULL) {
			char *name, *strchr();

			name = hasmntopt(mntp, MNTOPT_NOFSCK);
			if (name != NULL
			    || strcmp(mntp->mnt_type, MNTTYPE_EFS)
			    && strcmp(mntp->mnt_type, MNTTYPE_BELL)) {
				continue;
			}
			/*
			 * If the filesystem is not mounted on /, then
			 * we prefer to check the raw device.
			 */
			if (strcmp(mntp->mnt_dir, "/")
			    && (name = hasmntopt(mntp, MNTOPT_RAW)) != NULL
			    && (name = strchr(name, '=')) != NULL) {
				name++;
			} else {
				name = mntp->mnt_fsname;
			}
			argx[n] = (char *)malloc(strlen(name) + 1);
			(void) strcpy(argx[n], name);
			argc++, n++;
		}
		argx[n] = NULL;
		svargc = n;
		argv = argx;
		endmntent(fstabp);
	}
	ix = argvix = svargc - argc;	/* position of first fs argument */
	while(argc > 0) {
		initbarea(&sblk);
		if(checksb(argv[ix]) == NO) {
			argc--; ix++;
			continue;
		}
		if (get_superblk_type(sblk.b_un.b_buf,
				&fsmagic, &fsbshift) == 0
		 && !(fsmagic == EFS_MAGIC && fsbshift == BBSHIFT)
		 && get_fsck(fsmagic, fsbshift, &checker) == 0) {
			close(dfile.rfdes);
			if(argvix < svargc - argc) {
				for(n = 0; n < argc; n++)
					argv[argvix + n]
						 = argv[svargc - argc + n];
				argv[argvix + n] = NULL;
			}
			if(execvp(checker, argv) == -1)
				iderrexit("Cannot exec %s\n", checker);
		}
		if(!initdone) {
			initmem();
			initdone++;
		}
		check(argv[ix++]);
		argc--;
	}
	exit(0);
}


initmem()
{
	struct stat statbuf;
	char *sbrk();

	/* memsize = (MEMSIZE)sbrk(sizeof(int)); */
	/* memsize = MAXDATA - memsize - sizeof(int); */
	memsize = MAXDATA;
	while(memsize >= 2*sizeof(BUFAREA) &&
		(membase = sbrk(memsize)) == (char *)-1)
		memsize -= 1024;
	if(memsize < 2*sizeof(BUFAREA))
		iderrexit("Can't get memory\n");

# ifdef DEBUG
	if (!DEBUG)
# endif DEBUG
	setsigs();

	/* Check if standard input is a pipe. If it is, record pipedev so
	 * we won't ever check it */
	if (fstat(0, &statbuf) == -1)
		iderrexit("Can't fstat standard input\n");
	if (sfiletype(&statbuf) == S_IFIFO)
		pipedev = statbuf.st_dev;
}

check(dev)
	char *dev;
{
	if(pipedev != -1) {
		strcpy(devname,dev);
		strcat(devname,"\t");
	}
	else
		devname[0] = '\0';
	if(setup(dev) == NO)
		return;

	Phase1();

	if(!fast) {
		Phase2();
		Phase3();
		Phase4();
	}

	Phase5();

	if(fixfree) {
		Phase6();
	}

	idprintf("%ld files %ld blocks %ld free\n",
		n_files,n_blks,n_free);
	if(dfile.mod) {
		time(&superblk.fs_time);
		sbdirty();
	}

	sb_clean();

	ckfini();
	sync();
	if(dfile.mod && hotroot) {
		idprintf("***** REBOOTING UNIX . . . *****\n");
		sleep(2);
		reboot(RB_AUTOBOOT | RB_NOSYNC);
	}
	if(dfile.mod)
		idprintf("***** FILE SYSTEM WAS MODIFIED *****\n");
}
sb_clean()
{
	extern long efs_supersum();
	register long sum;

	if (superblk.fs_dirty) {
		idprintf("SUPERBLK MARKED DIRTY");
		if (reply("CLEAN") == YES) {
			superblk.fs_dirty = 0;
			sbdirty();
		}
	}
	sum = efs_supersum(&superblk);
	if (superblk.fs_checksum != sum) {
		idprintf("CHECKSUM WRONG IN SUPERBLK");
		if (reply("FIX") == YES) {
			superblk.fs_checksum = sum;
			sbdirty();
		}
	}
}

blkerr(s,blk)
daddr_t blk;
char *s;
{
	idprintf("%ld %s I=%u\n",blk,s,inum);
	setstate(CLEAR);	/* mark for possible clearing */
}


descend()
{
	register DINODE *dp;
	register char *savname;
	off_t savsize;

	setstate(FSTATE);
	if((dp = ginode()) == NULL)
		return;
	if(Dirc && !pss2done)
		ckinode(dp,BBLK);
	savname = thisname;
	*pathp++ = '/';
	savsize = filsize;
	filsize = dp->di_size;
	ckinode(dp,DATA);
	thisname = savname;
	*--pathp = 0;
	filsize = savsize;
}


direrr(s)
char *s;
{
	register DINODE *dp;
	int n;

	idprintf("%s ",s);
	pinode();
	if((dp = ginode()) != NULL && ftypeok(dp)) {
		newline();
		idprintf("%s=%s",DIR?"DIR":"FILE",pathname);
		if(DIR) {
			if (fsck_promptflag) {
				if (reply("REMOVE") != YES)
					return NO;
			}
			if(dp->di_size > EMPT) {
				if((n = chkempt(dp)) == NO) {
					printf(" (NOT EMPTY)\n");
				}
				else if(n != SKIP) {
					printf(" (EMPTY)");
					if(!nflag) {
						printf(" -- REMOVED\n");
						sleep(2);
						return(YES);
					}
					else
						newline();
				}
			}
			else {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					sleep(2);
					return(YES);
				}
				else
					newline();
			}
		}
		else if(REG||LNK)
			if (fsck_promptflag) {
				if (reply("REMOVE") != YES)
					return NO;
			}
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					sleep(2);
					return(YES);
				}
				else
					newline();
			}
	}
	else {
		newline();
		idprintf("NAME=%s",pathname);
		if (fsck_promptflag) {
			if (reply("REMOVE") != YES)
				return NO;
		}
		if (dp) {
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					sleep(2);
					return(YES);
				}
				else
					newline();
			}
			else
				printf(" (NOT EMPTY)\n");
		} else {
			printf(" (NON-EXISTENT)");
			if(!nflag) {
				printf(" -- REMOVED\n");
				sleep(2);
				return(YES);
			}
			else
				newline();
		}
	}
	return(reply("REMOVE"));
}


adjust(lcnt)
register short lcnt;
{
	register DINODE *dp;
	register n;

	if((dp = ginode()) == NULL)
		return;
	if(dp->di_nlink == lcnt) {
		if((n = linkup()) == NO)
			clri("UNREF",NO);
		if(n == REM)
			clri("UNREF",REM);
	}
	else {
		idprintf("LINK COUNT %s",
			(lfdir==inum)?lfname:(DIR?"DIR":"FILE"));
		pinode();
		newline();
		idprintf("COUNT %d SHOULD BE %d",
			dp->di_nlink,dp->di_nlink-lcnt);
		if(reply("ADJUST") == YES) {
			dp->di_nlink -= lcnt;
			inodirty();
		}
	}
}


/*
 * clri() --
 * possibly clear the inode,
 * depending on circumstances
 * and user responses.
 */
clri(s,flg)
char *s;
{
	register DINODE *dp;
	int n;

	if((dp = ginode()) == NULL)
		return;

	if(flg == YES) {
		if(!FIFO || !qflag || nflag) {
			idprintf("%s %s",s,DIR?"DIR":"FILE");
			pinode();
		}
		if(DIR) {
			if(dp->di_size > EMPT) {
				if((n = chkempt(dp)) == NO) {
					printf(" (NOT EMPTY)\n");
				}
				else if(n != SKIP) {
					printf(" (EMPTY)");
					if (fsck_promptflag && !replyfix())
						return;
					if(!nflag) {
						printf(" -- REMOVED\n");
						clrinode(dp);
						return;
					}
					else
						newline();
				}
			}
			else {
				printf(" (EMPTY)");
				if (fsck_promptflag && !replyfix())
					return;
				if(!nflag) {
					printf(" -- REMOVED\n");
					clrinode(dp);
					return;
				}
				else
					newline();
			}
		}
		if(REG||LNK)
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if (fsck_promptflag && !replyfix())
					return;
				if(!nflag) {
					printf(" -- REMOVED\n");
					clrinode(dp);
					return;
				}
				else
					newline();
			}
			else
				printf(" (NOT EMPTY)\n");
		if (FIFO && !nflag) {
			if (fsck_promptflag && !replyfix())
				return;
			if(!qflag)	printf(" -- CLEARED");
			newline();
			clrinode(dp);
			return;
		}
	}

	if (flg == REM) {
		clrinode(dp);
	}
	else
	if (reply("CLEAR") == YES) {
		clrinode(dp);
	}
}


clrinode(dp)		/* quietly clear inode */
register DINODE *dp;
{
	extern int pass4();

	n_files--;
	pfunc = pass4;
	ckinode(dp,ADDR);
	zapino(dp);
	inodirty();
}

setup(dev)
	char *dev;
{
	extern dev_t pipedev;	/* non-zero iff standard input is a pipe,
				 * which means we can't check pipedev */
	register n;
	register BUFAREA *bp;
	daddr_t bcnt, nscrblk;
	dev_t rootdev;
	register MEMSIZE msize;
	char *mbase;
	off_t smapsz, lncntsz, totsz;
	struct {
		daddr_t	tfree;
		ino_t	tinode;
		char	fname[6];
		char	fpack[6];
	}	ustatarea;
	struct stat statarea;
	daddr_t cylsize;

	if (stat("/", &statarea) < 0)
		iderrexit("Can't stat root\n");

	rootdev = statarea.st_dev;
	if (stat(dev, &statarea) < 0) {
		iderror("Can't stat %s\n", dev);
		return(NO);
	}

	hotroot = 0;
	rawflg = 0;
	if (sfiletype(&statarea) == S_IFBLK) {
		if (rootdev == statarea.st_rdev) {
			hotroot++;
		}
		else
		if (ustat(statarea.st_rdev,&ustatarea) >= 0) {
			if (!nflag) {
				iderror("%s is a mounted file system, ignored\n",
					dev);
				return(NO);
			}
			hotroot++;
		}
		if (pipedev == statarea.st_rdev) {
			iderror("%s is pipedev, ignored", dev);
			return(NO);
		}
	}
	else
	if (sfiletype(&statarea) == S_IFCHR) {
		rawflg++;
	}
	else
# ifdef DEBUG
	if (!DEBUG)
# endif DEBUG
	{
		iderror("%s is not a block or character device\n", dev);
		return(NO);
	}

	newline();
	printf("\n%c %s", id, dev);
	if (nflag && !csflag || dfile.wfdes == -1)
		printf(" (NO WRITE)");
	newline();

	pss2done = 0;
	fixfree = 0;
	dfile.mod = 0;
	n_files = n_blks = n_free = 0;
	muldup = enddup = &duplist[0];
	badlnp = &badlncnt[0];
	lfdir = 0;
	rplyflag = 0;
	initbarea(&fileblk);
	initbarea(&inoblk);
	sfile.wfdes = sfile.rfdes = -1;
	rmscr = 0;

	if (getblk(&sblk, SUPERB) == NULL
	 || sb_sizecheck(&superblk) == NO) {
		ckfini();
		return(NO);
	}
	superblk.fs_ipcg = superblk.fs_cgisize * EFS_INOPBB;
	inode_blocks = superblk.fs_cgisize * superblk.fs_ncg;
	max_inodes = inode_blocks * EFS_INOPBB - 1;
	bmapsz = howmany(superblk.fs_size, BITSPERBYTE);
	bmapsz = roundup(bmapsz, sizeof *lncntp);
	bitmap_blocks = bmapsz + BSIZE - 1 >> BSHIFT;
	fmin = superblk.fs_firstcg;
	fmax = superblk.fs_size;
	data_blocks = fmax - fmin - inode_blocks;

	idprintf("File System: %.6s Volume: %.6s\n\n",
			superblk.fs_fname, superblk.fs_fpack);

	smapsz = roundup(howmany((long)(max_inodes+1),STATEPB),sizeof(*lncntp));
	lncntsz = (long)(max_inodes+1) * sizeof(*lncntp);
	if(bmapsz > smapsz+lncntsz)
		smapsz = bmapsz-lncntsz;
	totsz = bmapsz+smapsz+lncntsz;
	msize = memsize;
	mbase = membase;
	if (rawflg) {
		if (msize < (MEMSIZE)(NINOBLK*BSIZE) + 2*sizeof(BUFAREA)) {
			rawflg = 0;
		}
		else {
			msize -= (MEMSIZE)NINOBLK*BSIZE;
			mbase += (MEMSIZE)NINOBLK*BSIZE;
			niblk = NINOBLK;
			startib = fmax;
		}
	}
	clear(mbase,msize);
	if ((off_t)msize < totsz) {
		bmapsz = roundup(bmapsz,BSIZE);
		smapsz = roundup(smapsz,BSIZE);
		lncntsz = roundup(lncntsz,BSIZE);
		nscrblk = (bmapsz+smapsz+lncntsz)>>BSHIFT;
		if (tflag == 0) {
			newline();
			idprintf("NEED SCRATCH FILE (%ld BLKS)\n",nscrblk);
			do {
				idprintf("ENTER FILENAME:\n");
				if((n = getline(stdin,scrfile,sizeof(scrfile))) == EOF)
					errexit("\n");
			} while(n == 0);
		}
		if (stat(scrfile,&statarea) < 0 ||
			sfiletype(&statarea) == S_IFREG)
			rmscr++;
		if ((sfile.wfdes = creat(scrfile,0666)) < 0 ||
			(sfile.rfdes = open(scrfile,0)) < 0) {
			iderror("Can't create %s\n",scrfile);
			ckfini();
			return(NO);
		}
		bp = &((BUFAREA *)mbase)[(msize/sizeof(BUFAREA))];
		poolhead = NULL;
		while (--bp >= (BUFAREA *)mbase) {
			initbarea(bp);
			bp->b_next = poolhead;
			poolhead = bp;
		}
		bp = poolhead;
		for (bcnt = 0; bcnt < nscrblk; bcnt++) {
			bp->b_bno = bcnt;
			dirty(bp);
			flush(&sfile,bp);
		}
		blkmap = freemap = statemap = (char *) NULL;
		lncntp = (short *) NULL;
		smapblk = bmapsz / BSIZE;
		lncntblk = smapblk + smapsz / BSIZE;
		fmapblk = smapblk;
	}
	else {
		if (rawflg && (off_t)msize > totsz+BSIZE) {
			niblk += (unsigned)((off_t)msize-totsz)>>BSHIFT;
			if (niblk > MAXRAW)
				niblk = MAXRAW;
			msize = memsize - (niblk*BSIZE);
			mbase = membase + (niblk*BSIZE);
		}
		poolhead = NULL;
		blkmap = mbase;
		statemap = &mbase[(MEMSIZE)bmapsz];
		freemap = statemap;
		lncntp = (short *)&statemap[(MEMSIZE)smapsz];
	}
	return(YES);
}

sb_sizecheck(sp)
	register struct efs *sp;
{
	register int bmsize;

	/* not checked
	if (!(MINSECTORS <= sp->fs_sectors && sp->fs_sectors < MAXSECTORS
	 && MINHEADS <= sp->fs_heads && sp->fs_heads < MAXHEADS))
		return supersizerr("garbage geometry %dx%d\n",
			sp->fs_sectors, sp->fs_heads);
	 */
	bmsize = howmany(sp->fs_size, BITSPERBYTE);
	if (sp->fs_bmsize != bmsize)
		return supersizerr("bitmap size %d should be %d\n",
			sp->fs_bmsize, bmsize);
	bmsize = howmany(bmsize, BBSIZE);
	if (BITMAPB+bmsize > sp->fs_firstcg)
		return supersizerr("bitmap ends at %d, overlaps firstcg %d\n",
			BITMAPB+bmsize, sp->fs_firstcg);
	if (sp->fs_cgfsize <= sp->fs_cgisize)
		return supersizerr("cgfsize %ld <= cgisize %ld\n",
			sp->fs_cgfsize, sp->fs_cgisize);
	if (sp->fs_size != sp->fs_firstcg + sp->fs_ncg*sp->fs_cgfsize)
		return supersizerr("fsize %ld != firstcg %d + %dx%ld\n",
			sp->fs_size,
			sp->fs_firstcg, sp->fs_ncg, sp->fs_cgfsize);

	return(YES);
}

int
supersizerr(a)
	struct { int x[6]; } a;
{
	iderror("Size check: ");
	error(a);
	return (NO);
}

checksb(dev)
char *dev;
{
	if ((dfile.rfdes = open(dev,0)) < 0) {
		iderror("Can't open %s\n", dev);
		return(NO);
	}
	if ((dfile.wfdes = open(dev,1)) < 0)
		dfile.wfdes = -1;
	if (getblk(&sblk,SUPERB) == NULL) {
		ckfini();
		return(NO);
	}
return(YES);
}

DINODE *
ginode()
{
	register DINODE *dp;
	register char *mbase;
	register daddr_t iblk;

	if(inum > max_inodes)
		return(NULL);
	iblk = ITOD(&superblk, inum);
	if(rawflg) {
		mbase = membase;
		if(iblk < startib || iblk >= startib+niblk) {
			if(inoblk.b_dirty)
				bwrite(&dfile,mbase,startib,niblk*BSIZE);
			inoblk.b_dirty = 0;
			if(bread(&dfile,mbase,iblk,niblk*BSIZE) == NO) {
				startib = fmax;
				return(NULL);
			}
			startib = iblk;
		}
		dp = (DINODE *)&mbase[(unsigned)((iblk-startib)<<BSHIFT)];
	} else
	if(getblk(&inoblk,iblk) != NULL)
		dp = inoblk.b_un.b_dinode;
	else
		return(NULL);
	return(dp + ITOO(&superblk, inum));
}

int
replyfix()
{
	return reply("POSSIBLY AUTO FIX") == YES;
}

reply(s)
char *s;
{
	char line[80];

	rplyflag = 1;
	line[0] = '\0';
	newline();
	idprintf("%s? ",s);
	if(nflag || dfile.wfdes < 0) {
		printf(" no\n\n");
		return(NO);
	}
	if(yflag) {
		printf(" yes\n\n");
		return(YES);
	}
	while (line[0] == '\0') {
		if(getline(stdin,line,sizeof(line)) == EOF)
			errexit("\n");
		newline();
		if(line[0] == 'y' || line[0] == 'Y')
			return(YES);
		if(line[0] == 'n' || line[0] == 'N')
			return(NO);
		idprintf("Answer 'y' or 'n' (yes or no)\n");
		line[0] = '\0';
	}
return(NO);
}


getline(fp,loc,maxlen)
FILE *fp;
char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while((n = getc(fp)) != '\n') {
		if(n == EOF)
			return(EOF);
		if(!isspace(n) && p < lastloc)
			*p++ = n;
	}
	*p = 0;
	return(p - loc);
}

stype(p)
register char *p;
{
	if(*p == 0)
		return;
	if (*(p+1) == 0) {
		if (*p == '3') {
			cylsize = 200;
			stepsize = 5;
			return;
		}
		if (*p == '4') {
			cylsize = 418;
			stepsize = 7;
			return;
		}
	}
	cylsize = atoi(p);
	while(*p && *p != ':')
		p++;
	if(*p)
		p++;
	stepsize = atoi(p);
	if(stepsize <= 0 || stepsize > cylsize ||
	cylsize <= 0 || cylsize > MAXCYL) {
		iderror("Invalid -s argument, defaults assumed\n");
		cylsize = stepsize = 0;
	}
}


dostate(s,flg)
{
	register char *p;
	register unsigned byte, shift;
	BUFAREA *bp;

	byte = ((unsigned)inum)/STATEPB;
	shift = LSTATE * (((unsigned)inum)%STATEPB);
	if(statemap != NULL) {
		bp = NULL;
		p = &statemap[byte];
	}
	else if((bp = tgetblk(smapblk+(byte/BSIZE))) == NULL)
		iderrexit("Fatal I/O error\n");
	else
		p = &bp->b_un.b_buf[byte%BSIZE];
	switch(flg) {
		case 0:
			*p &= ~(SMASK<<(shift));
			*p |= s<<(shift);
			if(bp != NULL)
				dirty(bp);
			return(s);
		case 1:
			return((*p>>(shift)) & SMASK);
	}
	return(USTATE);
}


/*
 * this is the same kind of bitmap
 * used by the efs...
 */
maphack(map, blk, flg)
	register struct bitmap *map;
	register daddr_t blk;
	int flg;
{
	register char *p;
	register unsigned n;
	register BUFAREA *bp;
	off_t byte;

	byte = blk >> BYTESHIFT;
	n = 1 << (unsigned)(blk & BYTEMASK);
	p = map->incore;
	blk = map->swap;

	if(p != NULL) {
		bp = NULL;
		p += (unsigned)byte;
	}
	else {
		if ((bp = tgetblk(blk+(byte>>BSHIFT))) == NULL)
			iderrexit("Fatal I/O error\n");
		else
			p = &bp->b_un.b_buf[(unsigned)(byte&BMASK)];
	}

	switch (flg) {

	case BMSET: /* set */
		*p |= n;
		break;

	case BMGET: /* get */
		n &= *p;
		bp = NULL;
		break;

	case BMCLR: /* clear */
		*p &= ~n;
	}

	if(bp != NULL)
		dirty(bp);
	return(n);
}


dolncnt(val,flg)
short val;
{
	register short *sp;
	register BUFAREA *bp;

	if(lncntp != NULL) {
		bp = NULL;
		sp = &lncntp[(unsigned)inum];
	}
	else if((bp = tgetblk(lncntblk+((unsigned)inum/SPERB))) == NULL)
		iderrexit("Fatal I/O error\n");
	else
		sp = &bp->b_un.b_lnks[(unsigned)inum%SPERB];
	switch(flg) {
		case 0:
			*sp = val;
			break;
		case 1:
			bp = NULL;
			break;
		case 2:
			(*sp)--;
	}
	if(bp != NULL)
		dirty(bp);
	return(*sp);
}


BUFAREA *
getblk(bp,blk)
register daddr_t blk;
register BUFAREA *bp;
{
	register struct filecntl *fcp;

	if(bp == NULL) {
		bp = search(blk);
		fcp = &sfile;
	}
	else
		fcp = &dfile;
	if(bp->b_bno == blk)
		return(bp);
	flush(fcp,bp);
	if(bread(fcp,bp->b_un.b_buf,blk,BSIZE) != NO) {
		bp->b_bno = blk;
		return(bp);
	}
	bp->b_bno = (daddr_t)-1;
	return(NULL);
}

BUFAREA *
tgetblk(blk)
	daddr_t blk;
{
	register BUFAREA *bp;
	register struct filecntl *fcp;

	bp = search(blk);
	fcp = &sfile;
	if(bp->b_bno == blk)
		return(bp);
	flush(fcp,bp);
	if(bread(fcp,bp->b_un.b_buf,blk,BSIZE) != NO) {
		bp->b_bno = blk;
		return(bp);
	}
	bp->b_bno = (daddr_t)-1;
	return(NULL);
}


flush(fcp,bp)
struct filecntl *fcp;
register BUFAREA *bp;
{
	if(bp->b_dirty) {
		if(bp->b_bno == SUPERB) {
			if(fcp->wfdes < 0) {
				bp->b_dirty = 0;
				return;
			}
			bp->b_dirty = 0;
			if (bwrite(fcp, bp->b_un.b_buf, bp->b_bno, BSIZE) == NO)
				fcp->mod = 1;
			return;
		}
		bwrite(fcp,bp->b_un.b_buf,bp->b_bno,BSIZE);
		bp->b_dirty = 0;
	}
}

rwerr(s,blk)
char *s;
daddr_t blk;
{
	newline();
	idprintf("CAN NOT %s: BLK %ld",s,blk);
	if(reply("CONTINUE") == NO)
		iderrexit("Program terminated\n");
}


sizechk(dp)
register DINODE *dp;
{
	off_t nblks;

	nblks = howmany(dp->di_size, BSIZE);
	if (dp->di_nx > EXTSPERDINODE) {
		nblks += dp->di_x[0].ex_length;
		/* NOT THIS
		nblks += howmany(dp->di_nx * sizeof (struct extent), BSIZE);
		 */
	}
	if(!qflag) {
		if(nblks != filsize)
			idprintf("POSSIBLE %s SIZE ERROR I=%u\n\n",
				DIR?"DIR":"FILE",inum);
		if(DIR && (dp->di_size % sizeof(DIRECT)) != 0)
			idprintf("DIRECTORY MISALIGNED I=%u\n\n",inum);
	}
}


ckfini()
{
	flush(&dfile,&fileblk);
	flush(&dfile,&sblk);
	flush(&dfile,&inoblk);
	close(dfile.rfdes);
	close(dfile.wfdes);
	close(sfile.rfdes);
	close(sfile.wfdes);
	if(rmscr) {
		unlink(scrfile);
	}
}


pinode()
{
	register DINODE *dp;
	register char *p;
	char uidbuf[200];
	char *ctime();

	printf(" I=%u ",inum);
	if((dp = ginode()) == NULL)
		return;
	printf(" OWNER=");
	if(getpw((int)dp->di_uid,uidbuf) == 0) {
		for(p = uidbuf; *p != ':'; p++);
		*p = 0;
		printf("%s ",uidbuf);
	}
	else
		printf("%d ",dp->di_uid);
	printf("MODE=%o\n",dp->di_mode);
	idprintf("SIZE=%ld ",dp->di_size);
	p = ctime(&dp->di_mtime);
	printf("MTIME=%12.12s %4.4s ",p+4,p+20);
}


BUFAREA *
search(blk)
daddr_t blk;
{
	register BUFAREA *pbp, *bp;

	for(bp = (BUFAREA *) &poolhead; bp->b_next; ) {
		pbp = bp;
		bp = pbp->b_next;
		if(bp->b_bno == blk)
			break;
	}
	pbp->b_next = bp->b_next;
	bp->b_next = poolhead;
	poolhead = bp;
	return(bp);
}


findino(dirp)
register DIRECT *dirp;
{
	register char *p1, *p2;

	if(dirp->d_ino == 0)
		return(KEEPON);
	for(p1 = dirp->d_name,p2 = srchname;*p2++ == *p1; p1++) {
		if(*p1 == 0 || p1 == &dirp->d_name[DIRSIZ-1]) {
			if(dirp->d_ino >= ROOTINO && dirp->d_ino <= max_inodes)
				parentdir = dirp->d_ino;
			return(STOP);
		}
	}
	return(KEEPON);
}


mkentry(dirp)
register DIRECT *dirp;
{
	register ino_t in;
	register char *p;

	if(dirp->d_ino)
		return(KEEPON);
	dirp->d_ino = orphan;
	in = orphan;
	p = &dirp->d_name[DIRSIZ];
	while(p != &dirp->d_name[6])
		*--p = 0;
	while(p > dirp->d_name) {
		*--p = (in % 10) + '0';
		in /= 10;
	}
	return(ALTERD|STOP);
}


chgdd(dirp)
register DIRECT *dirp;
{
	if(dirp->d_name[0] == '.' && dirp->d_name[1] == '.' &&
	dirp->d_name[2] == 0) {
		dirp->d_ino = lfdir;
		return(ALTERD|STOP);
	}
	return(KEEPON);
}


/*
 * linkup() --
 */
linkup()
{
	register DINODE *dp;
	register lostdir;
	register ino_t pdir;
	register ino_t *blp;
	int n;

	if((dp = ginode()) == NULL)
		return(NO);
	lostdir = DIR;
	pdir = parentdir;
	if(!FIFO || !qflag || nflag) {
		idprintf("UNREF %s ",lostdir ? "DIR" : "FILE");
		pinode();
	}
	if(DIR) {
		if(dp->di_size > EMPT) {
			if((n = chkempt(dp)) == NO) {
				printf(" (NOT EMPTY)");
				if (fsck_promptflag && !replyfix())
					return NO;
				if(!nflag) {
					printf(" MUST reconnect\n");
					goto connect;
				}
				else
					newline();
			}
			else if(n != SKIP) {
				printf(" (EMPTY)");
				if (fsck_promptflag && !replyfix())
					return NO;
				if(!nflag) {
					printf(" Cleared\n");
					return(REM);
				}
				else
					newline();
			}
		}
		else {
			printf(" (EMPTY)");
			if (fsck_promptflag && !replyfix())
				return NO;
			if(!nflag) {
				printf(" Cleared\n");
				return(REM);
			}
			else
				newline();
		}
	}
	if(REG||LNK)
		if(!dp->di_size) {
			printf(" (EMPTY)");
			if (fsck_promptflag && !replyfix())
				return NO;
			if(!nflag) {
				printf(" Cleared\n");
				return(REM);
			}
			else
				newline();
		}
		else
			printf(" (NOT EMPTY)\n");
	if (fsck_promptflag && !replyfix())
		return NO;
	if(FIFO && !nflag) {
		if(!qflag)	printf(" -- REMOVED");
		newline();
		return(REM);
	}
	if (fsck_promptflag && !replyfix())
		return NO;
	if(FIFO && nflag)
		return(NO);
	if(reply("RECONNECT") == NO)
		return(NO);
connect:
	orphan = inum;
	if(lfdir == 0) {
		inum = ROOTINO;
		if((dp = ginode()) == NULL) {
			inum = orphan;
			return(NO);
		}
		pfunc = findino;
		srchname = lfname;
		filsize = dp->di_size;
		parentdir = 0;
		ckinode(dp,DATA);
		inum = orphan;
		if((lfdir = parentdir) == 0) {
			idprintf("SORRY. NO lost+found DIRECTORY\n\n");
			return(NO);
		}
	}
	inum = lfdir;
	if((dp = ginode()) == NULL || !DIR || getstate() != FSTATE) {
		inum = orphan;
		idprintf("SORRY. NO lost+found DIRECTORY\n\n");
		return(NO);
	}
	if(dp->di_size & BMASK) {
		dp->di_size = roundup(dp->di_size,BSIZE);
		inodirty();
	}
	filsize = dp->di_size;
	inum = orphan;
	pfunc = mkentry;
	if((ckinode(dp,DATA) & ALTERD) == 0) {
		idprintf("SORRY. NO SPACE IN lost+found DIRECTORY\n\n");
		return(NO);
	}
	declncnt();
	if((dp = ginode()) && !dp->di_nlink) {
		dp->di_nlink++;
		inodirty();
		setlncnt(getlncnt()+1);
		if(lostdir) {
			for(blp = badlncnt; blp < badlnp; blp++)
				if(*blp == inum) {
					*blp = 0L;
					break;
				}
		}
	}
	if(lostdir) {
		pfunc = chgdd;
		filsize = dp->di_size;
		ckinode(dp,DATA);
		inum = lfdir;
		if((dp = ginode()) != NULL) {
			dp->di_nlink++;
			inodirty();
			setlncnt(getlncnt()+1);
		}
		inum = orphan;
		idprintf("DIR I=%u CONNECTED. ",orphan);
		idprintf("PARENT WAS I=%u\n\n",pdir);
	}
	return(YES);
}


bread(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
register MEMSIZE size;
char *buf;
{
	if(lseek(fcp->rfdes,blk<<BSHIFT,0) < 0)
		rwerr("SEEK",blk);
	else if(read(fcp->rfdes,buf,size) == size)
		return(YES);
	rwerr("READ",blk);
	return(NO);
}


bwrite(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
register MEMSIZE size;
char *buf;
{
	if(fcp->wfdes < 0)
		return(NO);
	if(lseek(fcp->wfdes,blk<<BSHIFT,0) < 0)
		rwerr("SEEK",blk);
	else if(write(fcp->wfdes,buf,size) == size) {
		fcp->mod = 1;
		return(YES);
	}
	rwerr("WRITE",blk);
	return(NO);
}

/* VARARGS */
idprintf(a)
	struct { int x[6]; } a;
{
	printf("%c %s", id, devname);
	printf(a);
}

/* VARARGS */
iderror(a)
	struct { int x[6]; } a;
{ 
	printf("%c %s", id, devname);
	error(a);
}

/* VARARGS */
iderrexit(a)
	struct { int x[6]; } a;
{
	printf("%c %s", id, devname);
	errexit(a);
}

/* VARARGS */
error(a)
	struct { int x[6]; } a;
{
	printf(a);
}

/* VARARGS */
errexit(a)
	struct { int x[6]; } a;
{
	error(a);
	exit(8);
}

newline()
{
	printf("\n");
}

catch()
{
	ckfini();
	exit(4);
}

setsigs()
{
	extern int catch();

	register int n;
	register int osig;

	for(n = 1; n < NSIG; n++) {
		if (n == SIGCLD || n == SIGPWR)
			continue;
		if ((osig = (int)signal(n, catch)) != (int)SIG_DFL)
			signal(n, osig);
	}
}
