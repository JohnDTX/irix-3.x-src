#ifndef lint
static	char	*sccsid = "@(#)from tar.c       4.19 (Berkeley) 9/22/83";
static	char	*SccsId = "@(#)$Revision: 1.1 $";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/tar.c,v 1.1 89/03/27 14:51:17 root Exp $";
/*	tar	COMPILE:	cc -O tar.c -s -i -o tar clstat.o	*/
/* SVR3 tar     COMPILE:	cc -O tar.c -s -i -o tar		*/
#endif

/*
 *  Modified 1/84 by Dave Yost
 *   - At end of archive, tar now tries two more reads to find end of file
 *     so that mag tape is properly positioned at the beginning of the next
 *     tape file.
 *   - Recognize premature end of file as such.  These last two
 *     called for changes to getdir(), readtape(), passtape(), endtape(),
 *     and the places where they are called.
 *   - Add 'e' option to continue after tape read errors.
 *   - Add 'X' option, which is like 'x' except it compares
 *     against another tree and links to there on files from tape
 *     which are identical to their counterparts in the specified tree.
 *   - Add 'C' option to compare the tape against the file system.
 *   - Changed the linkbuf struct, putting the filename string at the
 *     end and allocating just enough space for it, instead of making
 *     it a fixed-size array which is hoped to be big enough.
 *   - Replaced some of the rflag, tflag, etc. variables with a 'work'
 *     variable that is set to the type of work to be done.
 *   - Enforce blocking factor limit of 20.  If tar can't extract it, why
 *     let it write it?
 *   - The 'cannont link' message now says what it cannot link to.
 *   - The 'x filename' message printed when extracting now holds back the
 *     newline until the file is finished extracting so you can know when
 *     it is done if you are only extracting one file and want to kill it.
 *   - Print out group owner %-2d instead of %d.
 *   - Changed default device to the logical device /dev/tar instead of
 *     /dev/rmt0.  Let'em make the appropriate device for their system.
 *   - Complain and exit if tarfile specified with 'f' option is in /dev
 *     and either doesn't exist or is a regular file.
 *   - Put in some ifdefs so this source will work on more vanilla systems.
 *   - Buffer stderr and flush it after each write.  (4.2 line buffering not
 *     used because it reverts to block buffering when tar output log is
 *     redirected to a file and because it is not portable.)
 *   - Clean up file names before writing them on the directory block:
 *     no leading '//' or './' garbage.
 *   - If a filename on tape has a leading './', pretend it doesn't.
 *   - Added 'R' option to ignore leading '/' when extracting.
 *   - Fixed bug: tar got mixed up on what its parent directory was if you
 *     tried to archive a directory starting with '/'.
 *   - Added 'U' option to unlink before each creat.
 *   - Fix bug where file descriptor was not freed up when file name too
 *     long in putfile in the IF_REG case.
 *   - Tar passes lint now.
 *
 *  Modified 7/84 by Bob Toxen of Silicon Graphics, Inc.
 *
 *  1. Tar CAN NOW BACK UP CHAR/BLOCK DEVICES AND NAMED PIPES!!!
 *  2. Renamed '-o' option to '-d'.
 *  3. Added   '-o' option to not do CHOWN or CHGRP on extracted files.
 *  4. Default blocking factor for our cartridge tape is now 400.
 *  5. Fix bug so Tar will cope with pwd returning excessively long path.
 *  6. Added   '-V' option to allow variably sized last block to avoid large
 *     write for last block for hugely blocked tapes.
 *  7. Can now have huge blocking factors.
 *  8. Fixed bug whereby Tar hung on device and named pipe files (see #1).
 *  9. Fixed bug where Tar stripped set-U/Gid & sticky bit from modes.
 * 10. Fixed bug whereby a compiler warning about NULL being redefined.
 * 11. Changed default archive file back to /dev/rmt1.
 * 12. Added support for System III and System V.
 * 13. Jazzed up conditional code & added comments.
 * 14. Allow specifying all drives from 1 to 9, don't have to have 4.2BSD MT.
 * 15. In verbose mode tells when symbolic links are changed to hard links.
 * 16. Files in directories are written to tape alphabetically!!!!!
 * 17. Fixed bug whereby if "/" is to be backed up tar tried to open ""
 *     which was interpreted as "." on V7 but is illegal on System III & V.
 * 18. Can now handle multiple tape volumes. If -e (continue on errors)
 *     is NOT specified then Tar will consider any error return to mean
 *     end of tape. Otherwise any error except EIO will mean end of tape.
 * 19. Don't try to read past end of tape if OLDMAGTAPE.
 * 20. If file name "-" is on command line then read list of files from
 *     standard input (thanks to Steve Hartwell).
 * 21. Added -a to preserve access time on files read (10/10/84).
 */

/*
 * Tape Archival Program
 */

/*
 * Conditional compilation defines:
 *
 * 1. BSD42	 compile under 4.2 BSD (also defines NODEFS & FASTDIE).
 * 2. OLDMAGTAPE don't have 4.2 BSD's mag tape driver.
 * 3. V7	 compile under Version 7 (as opposed to System III/V).
 * 4. SYS3	 compile under System III.
 * 5. SYS5	 compile under System V.
 * 6. CART	 support Silicon Graphics Cartridge Tape as default.
 * 7. DEVTAR	 default device name for archive file.
 * 8. FASTDIE	 signals kill immediately.
 * 9. NODEFS	 if undefined then define OLDMAGTAPE CART FASTDIE.
 * 10.SVR3       compile under Mips system 5
 */

#ifdef	BSD4_2
#ifndef	BSD42
#define	BSD42
#endif	BSD42
#endif	BSD4_2

#ifdef	BSD41
#define	V7
#undef	BSD
#define	BSD
#endif

#ifdef	BSD42
#define	V7
#undef	BSD
#define	BSD
#define	NODEFS
#define	FASTDIE
#endif

#ifdef	SYS3
#undef	V7
#define	USG
#endif

#ifdef  SVR3
#define SYS5
#endif
#ifdef	SYS5
#undef	V7
#define	USG
#endif

#ifndef	NODEFS
#define	OLDMAGTAPE
#define	CART
#define	FASTDIE
#endif	NODEFS

#ifndef	DEVTAR
#ifdef	BSD
#define	DEVTAR	"/dev/rmt8"
#else	BSD
#ifdef SVR3
#define DEVTAR  "/dev/tape"
#define DEVTAR2 "/dev/tape1"
#else  SVR3
#define	DEVTAR	"/dev/rmt1"
#endif
#endif
#endif

#include <sys/param.h>

#ifndef	BSD42
#include <sys/types.h>
#endif	BSD42

#ifdef	USG
#include <sys/sysmacros.h>
#endif

#include <sys/stat.h>

#ifndef SVR3
# ifdef S_IFLNK
extern int clstat();
# else  S_IFLNK
# define clstat(f,p)	stat(f,p)
# endif S_IFLNK
#endif

#ifdef	BSD42
#include <sys/dir.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#else	BSD42
#include <dirent.h>
#ifdef SVR3
#include <sys/nami.h>
#else
#define	MAXPATHLEN 1024
#endif SVR3
#endif	BSD42

#include <signal.h>
#include <errno.h>
#include <stdio.h>

#ifdef	BSD42
#ifndef	MAGTAPE
#define	MAGTAPE
#endif	MAGTAPE
#undef	OLDMAGTAPE
#endif	BSD42

/*
 * SGI: getbsize is unconditionally part of tar, so we need this.
 */
#include <sys/mtio.h>

#ifndef	V7
#define	index	strchr
#define	rindex	strrchr
#endif

#define	YES 1
#define	NO  0
#undef	MIN
#define	MIN(a,b) (((a) < (b))? (a): (b))

#define	TBLOCK	512	/* size of a tape block */

#define	NBLOCK	20	/* default blocking factor */

#ifdef	CART
#define	NBLOCKC	400	/* default SGI cartridge blocking factor */
#else
#define	NBLOCKC	20	/* default blocking factor */
#endif
#define	DFUDGE	20	/* fudge factor on growing directories */

#define	FCHR	0020000
#define	FBLK	0060000
#define	FIFO	0010000

/* even for SVR3 must keep NAMSIZ 100 and not MAXPATHLEN because TAR
 * tape block size written in stone (TBLOCK)
 */
#define	NAMSIZ	100


off_t	lseek();
time_t	time();
char	*mktemp();
char	*strcpy();

union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
		char rdev[12];
	} dbuf;
};

struct linkbuf {
	struct	linkbuf *nextp;
#ifdef SVR0
	long	inum;		/* handle long i-numbers on SV, rel 0 */
#else
	ino_t	inum;
#endif
	dev_t	devnum;
	short	count;
	char	pathname[1];	/* actually alloced larger */
};

union	hblock dblock;
union	hblock *tbuf;
char	*dblockname;		/* pointer to dblock.dbuf.name */
struct	linkbuf *ihead;
struct	stat stbuf;

/*
 *  The kind of work we are going to do is determined
 *  by the value of the 'work' variable.  Other aspects
 *  of the work are influenced by other flags.
 */
typedef int (*funcaddr_t)();       /* Pointer to a function */
funcaddr_t work;
#define NOWORK  (funcaddr_t)0
int     dorep();
int     doxtract();
int     dotable();
int     docompare();

int	vflag;
int	cflag;
int	mflag;
int	fflag;
int	dflag;		/* don't backup directory files			*/
int	pflag;
int	wflag;
int	Lflag;
int	Bflag;
int	Fflag;		/* undocumented 4.2 flag */
int	continflag;	/* continue on read errs */
char	lnflag;		/* -X  link from linkdir if identical */
int	Rflag;		/* ignore leading '/' when extracting */
int     Uflag;  	/* unlink before creating each file */
int	Vflag;		/* SGI: Variable blocking */
int	oflag;		/* SGI: don't chown files			*/
int	bflag;		/* SGI: user set blocking			*/
int	Dflag;		/* SGI: don't backup device/pipe files		*/
int	aflag;		/* SGI: restore access time			*/

int	mt;
int	term;
int	chksum;
int	recno;
int	first;
int	linkerrok;
char	*linkdir;	/* link from this directory if identical	*/
int	freemem = 1;
int	nblock;
int	isatape = 0;
int	debug;		/* activate with -q				*/
int	onintr();
int	onquit();
int	onhup();
int	onterm();
int	read();
int	write();

daddr_t	low;
daddr_t	high;
daddr_t	bsrch();

FILE	*tfile;
char	tname[] = "/tmp/tarXXXXXX";
char	*usefile;
char	*defaultdev = DEVTAR;
#ifdef SVR3
char    *defdev2 = DEVTAR2;
#endif

char	*malloc();
#ifndef SVR3
char	*sprintf();
#endif SVR3
char	*strcat();
char	*rindex();
char	*getcwd();
char	*getwd();

main(argc, argv)
	int argc;
	char *argv[];
{
	char *cp;
	char hostname[32];
	char stderrbuf[BUFSIZ];
	int  cf;
#ifdef SVR3
	char    *usefil1;
	int speed;

	usefil1 = DEVTAR2;
#endif

	setbuf(stderr, stderrbuf);
		/* must be before gethostname */
	cf = open("/usr/lib/uucp/SYSTEMNAME",0);
	if (cf >= 0) {
		hostname[0] = '\0';
		read(cf,hostname,sizeof hostname - 1);
		close(cf);
	}
	if (argc < 2)
		usage();

	tfile = NULL;
	argv[argc] = 0;			/* may be illegal! */
	argv++;
#ifdef SVR3
	for (cp = *argv++, speed = 0; *cp; cp++, speed--)
#else
	for (cp = *argv++; *cp; cp++)
#endif

		switch (*cp) {
		case 'f':
			if (*argv == 0) {
				fprintf(stderr,
			"tar: tapefile must be specified with 'f' option\n");
				usage();
			}
			usefile = *argv++;
			fflag++;
			break;

		case 'c':
			cflag++;
			work = dorep;
			break;

		case 'e':
			continflag++;
			break;

		case 'd':
			dflag++;
			Dflag++;
			break;

		case 'p':
			pflag++;
			break;

		case 'u':
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
				fprintf(stderr,
				 "tar: cannot create temporary file (%s)\n",
				 tname);
				done(1);
			}
			fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");

			/*FALL THRU*/

		case 'r':
			work = dorep;
			break;

		case 'v':
			vflag++;
			break;

		case 'w':
			wflag++;
			break;

		case 'R':
			Rflag++;
			break;

		case 'U':
			Uflag++;
			break;

		case 'C':
			work = docompare;
			break;

		case 'X':
			linkdir = *argv++;
			lnflag++;

			/*FALL THRU*/

		case 'x':
			pflag++;
			work = doxtract;
			break;

		case 't':
			work = dotable;
			break;

		case 'm':
#ifdef SVR3
			if (speed != 1) /* if speed = 1 then m= medium speed*/
#endif
				mflag++;


			break;

		case 'a':
			aflag++;
			break;

		case '-':
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			defaultdev = DEVTAR;
#ifdef SVR3

			defaultdev[strlen(defaultdev)-2] = *cp;
#else
			defaultdev[strlen(defaultdev)-1] = *cp;
#endif
			usefile = defaultdev;
#ifdef SVR3
			speed = 2;	/* next time will be 1 */
			defdev2 = DEVTAR2;
			defdev2[strlen(defdev2) -1] = *cp;
			usefil1 = defdev2;
#endif
			break;

		case 'b':
			if (*argv == 0) {
				fprintf(stderr,
			"tar: blocksize must be specified with 'b' option\n");
				usage();
			}
			nblock = atoi(*argv);
			if (nblock <= 0) {
				fprintf(stderr, "tar: invalid blocksize.\n");
				done(1);
			}
			bflag++;
			argv++;
			break;

		case 'l':
#ifdef SVR3
			if (speed == 1) {
				defaultdev[strlen(defaultdev) -1] = *cp;
				usefile = defaultdev;
				break;
			}
			else
			{
#endif
				linkerrok++;
				break;
#ifdef SVR3 
			}
#endif

		case 'H':
		case 'h':
#ifdef SVR3
			if (speed == 1) {
				defaultdev[strlen(defaultdev) -1] = *cp;
				usefile = defaultdev;
				break;
			}
			else
			{
#endif
				Lflag = 0;
				break;
#ifdef SVR3 
			}
#endif

		case 'L':
			Lflag++;
			break;

		case 'B':
			Bflag++;
			break;

		case 'F':
			Fflag++;
			break;

		case 'V':		/* SGI: Variable blocking	*/
			Vflag++;
			break;

		case 'o':		/* SGI (pre 5.2): don't chown	*/
			oflag++;
			break;

		case 'D':		/* SGI: don't back devs/pipes	*/
			Dflag++;
			break;

		case 'q':		/* SGI: bump up debugging level	*/
			debug++;
			break;

		default:
			fprintf(stderr, "tar: %c: unknown option\n", *cp);
			usage();
		}
	if (!usefile)
		usefile = defaultdev;

	if (!strncmp("/dev/", usefile, 5)) {
		struct stat stbuf;

		if (stat(usefile, &stbuf) < 0
		  || (stbuf.st_mode & S_IFMT) == S_IFREG) {
#ifdef SVR3
			if (!fflag) {
    
			    if (stat(usefil1, &stbuf) < 0) {
				fprintf(stderr,
    "tar: archive file %s does not exist or is a regular file\n", usefile);

				fprintf(stderr,
    "tar: archive file %s does not exist or is a regular file\n", usefil1);
				done(1);

			    }
			    else {
				usefile = usefil1;

			    }
			}
			else {
				fprintf(stderr,
    "tar: archive file %s does not exist or is a regular file\n", usefile);
				done(1);
			}
#else
				fprintf(stderr,
    "tar: archive file %s does not exist or is a regular file\n", usefile);
				done(1);
#endif
		}
		isatape = 1;
	}

	if (!bflag)
		nblock = getbsize(usefile);
	tbuf = (union hblock *)malloc((unsigned int)(nblock*TBLOCK));
	if (tbuf == NULL) {
		fprintf(stderr,
		    "tar: block size %d too big, can't get memory\n",
		    nblock * TBLOCK);
		done(1);
	}

	if (work == dorep)
		dorep(argv);
	else if (work != NOWORK) {
		if (strcmp(usefile, "-") == 0) {
			mt = dup(0);
			if (!Vflag)
				nblock = 1;
		} else if ((mt = open(usefile, 0)) < 0) {
			fprintf(stderr, "tar: cannot open %s\n", usefile);
			done(1);
		}
		(*work) (argv);
	} else
		usage();
	done(0);
	/* NOTREACHED */
	return;
}

usage()
{
	fprintf(stderr, 
#ifdef SVR3
"usage: tar -{txruCX}[abcdefhlmopqvwLUBDRV][0-9[lmh]] [dir] [blocksize]\
 [tapefile] file ...\n");
#else
"usage: tar -{txruCX}[12abcdefhlmopqvwLUBDRV] [dir] [blocksize]\
 [tapefile] file ...\n");
#endif

	done(1);
	/* NOTREACHED */
	return;
}

/*
 * Try to figure out the block size using a mag tape ioctl.  Sketch:
 *	if (using standard input/output archive)
 *		dup stdin or stdout;
 *	else if (open() fails && not creating a named archive)
 *		complain and die;
 *	if (get block size via ioctl() fails)
 *		return default block size;
 *	else
 *		return ioctl() block size;
 */
int
getbsize(arname)
	char *arname;
{
	register int fd, bsize;
	struct mtget mt_status;

	if (strcmp(arname, "-") == 0) {
		fd = dup(work == dorep ? 1 : 0);
	} else if ((fd = open(arname, 0)) < 0 && !(cflag && fflag)) {
		fprintf(stderr, "tar: cannot open %s\n",
			arname);
		done(1);
	}
	bsize = 0;
#ifdef SVR3
	if (fd >= 0) {
		auto int blksize;

		if (ioctl(fd, MTIOCGETBLKSIZE, &blksize) == 0)
			bsize = blksize;
	}
#else
	/*
	 * XXX this is all wrong.  Fix to be like 5.3
	 */
	mt_status.mt_type = MTBLKSIZE;
	if (fd >= 0
	    && ioctl(fd, MTIOCGET, (char *)&mt_status) >= 0) {
		bsize = mt_status.mt_blkno;
	}
#endif
	if (bsize <= 0)
		bsize = NBLOCK;
	if (fd >= 0) {
		(void) close(fd);
	}
	return bsize;
}

dorep(argv)
	char *argv[];
{
	char wdir[MAXPATHLEN];

#ifdef OLDMAGTAPE
	if (nblock != 1 && !cflag) {
		fprintf(stderr, "tar: blocked tapes cannot be updated\n");
		done(1);
	}
#endif OLDMAGTAPE
	if (cflag && tfile != NULL)
		usage();
#ifndef	FASTDIE
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, onhup);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, onquit);
#ifdef notdef
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, onterm);
#endif notdef
#endif	FASTDIE
	if (strcmp(usefile, "-") == 0) {
		if (cflag == 0) {
			fprintf(stderr,
		 "tar: can only create standard output archives\n");
			done(1);
		}
		mt = dup(1);
		if (!Vflag)
			nblock = 1;
	} else {
		mt = -1;
			/* If -c then truncate regular files! */
		if (cflag)
			mt = creat(usefile, 0666);
		if (mt < 0 && (mt = open(usefile, cflag?1:2)) < 0) {
			fprintf(stderr, "tar: cannot open %s\n", usefile);
			done(1);
		}
	}

	if (!cflag) {
		while (getdir()) {
			passtape();
			if (term)
				done(0);
		}
		backtape();
		if (tfile != NULL) {
			char buf[200];

			sprintf(buf,
"sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX; mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			fflush(tfile);
			system(buf);
			freopen(tname, "r", tfile);
			fstat(fileno(tfile), &stbuf);
			if ((stbuf.st_mode & S_IFMT) != S_IFREG)
				stbuf.st_size = 0;
			high = stbuf.st_size;
		}
	}

				/*
				 * Modified by Bob to do Steve Hartwell's
				 * enhancement of reading list of files to
				 * backup from stdin if file name is "-".
				 */
	(void) getcwd(wdir);
	while (*argv && ! term) {
		register char *cp, *cp2;
		char tempdir[MAXPATHLEN], *parent;
		char stdinfname[MAXPATHLEN+2], *filearg;

		if (!strcmp(*argv, "-C") && argv[1]) {
			argv++;
			if (chdir(*argv) < 0)
				perror(*argv);
			else
				(void) getcwd(wdir);
			argv++;
			continue;
		}
		if (!strcmp(*argv,"-")) {
			if (gets(stdinfname) == NULL) {
				argv++;
				continue;
			}
			filearg = stdinfname;
		} else
			filearg = *argv++;
		cp2 = 0;
		for (cp = filearg; *cp; cp++)
			if (*cp == '/')
				cp2 = cp;
		if (cp2) {
			*cp2 = '\0';
			if (chdir(filearg[0] ? filearg : "/") < 0) {
				perror(filearg);
				continue;
			}
			parent = getcwd(tempdir);
			*cp2 = '/';
				/* SGI: cope with "/" and can't open "" */
			if (!*++cp2)
				cp2--;
		} else {
			cp2 = filearg;
			parent = wdir;
		}
		putfile(filearg, cp2, parent);
		if (chdir(wdir) < 0) {
			fprintf(stderr, "cannot change back?: ");
			fflush(stderr);
			perror(wdir);
		}
	}
	putempty();
	putempty();
	flushtape();
	if (linkerrok == 0)
		return;
	for (; ihead != NULL; ihead = ihead->nextp) {
		if (ihead->count == 0)
			continue;
		fprintf(stderr, "tar: missing links to %s\n", ihead->pathname);
		fflush(stderr);
	}
	return;
}

endtape()
{

	if (!continflag && dblock.dbuf.name[0] == '\0')
		return YES;
	/*
	 * Assume constant expression in if causes dead code elimination.
	 */
	if (TBLOCK % sizeof(int)) {
		register char *cp;
		register char *endp;

		endp = &dblock.dummy[TBLOCK];
		for (cp = dblock.dummy; cp < endp; cp++)
			if (*cp != '\0')
				return NO;
	} else {
 		register int *ip;
 		register int *endp;

 		endp = (int *) &dblock.dummy[TBLOCK];
 		for (ip = (int *) dblock.dummy; ip < endp; ip++)
 			if (*ip != 0)
 				return NO;
	}
	return YES;
}

getdir()
{
	register struct stat *sp;
	register int continuing;
	int i;

	continuing = NO;
	for (;;) {
		int cnt;
		if ((cnt = readtape((char *) &dblock)) < 0) {
			fflush(stdout);
			if (!continflag) {
				fprintf(stderr,
			       "tar: tape error reading dir-block\n");
				done(2);
			}
			if (!continuing) {
				fprintf(stderr,
			       "tar: tape error reading dir-block.  Continuing.");
				fflush(stderr);
				continuing = YES;
			} else {
				fprintf(stderr, ".");
				fflush(stderr);
			}
			continue;
		}
		if (cnt == 0)
			premature_eof();
		if (endtape()) {
			if (continuing) {
				fprintf(stderr, "END\n");
				fflush(stderr);
			}
			return NO;
		}
		sp = &stbuf;
		sscanf(dblock.dbuf.mode, "%o", &i);
		sp->st_mode = i;
		sscanf(dblock.dbuf.uid, "%o", &i);
		sp->st_uid = i;
		sscanf(dblock.dbuf.gid, "%o", &i);
		sp->st_gid = i;
		sscanf(dblock.dbuf.size, "%lo", &(sp->st_size));
		sscanf(dblock.dbuf.mtime, "%lo", &sp->st_mtime);
		sscanf(dblock.dbuf.chksum, "%o", &chksum);
		sscanf(dblock.dbuf.rdev, "%ho", &sp->st_rdev);
		switch (sp->st_mode & ~07777) {
		  case FCHR:
		  case FBLK:
		  case FIFO:
			sp->st_size = 0;
			break;
		  default:
			if (dblock.dbuf.name[strlen(dblock.dbuf.name)] == '/')
				sp->st_size = 0;
		}
		if (chksum == checksum()) {
			if (continuing) {
				fprintf(stderr, "OK\n");
				fflush(stderr);
			}
			break;
		}
		if (!continflag) {
			fprintf(stderr, "tar: directory checksum error\n");
			done(2);
		} else {
			fflush(stdout);
			if (!continuing) {
				fprintf(stderr,
				"tar: Directory checksum error.  Continuing.");
				fflush(stderr);
				continuing = YES;
			} else {
				fprintf(stderr, ".");
				fflush(stderr);
			}
		}
	}
	{
		register char *cp;

		for (cp = &dblock.dbuf.name[0]; ; ) {
			if (cp[0] == '.' && cp[1] == '/') {
				if (cp[2] == '\0')
					break;
				cp += 2;
			} else if (Rflag && cp[0] == '/') {
				if (cp[1] == '\0')
					break;
				cp++;
			} else
				break;
		}
		dblockname = cp;
	}
	if (tfile != NULL)
		fprintf(tfile, "%s %s\n", dblockname, dblock.dbuf.mtime);
	return YES;
}

passtape()
{
	long blocks;
	char buf[TBLOCK];

	if (dblock.dbuf.linkflag == '1')
		return;
	blocks = stbuf.st_size;
	blocks += TBLOCK-1;
	blocks /= TBLOCK;

	while (blocks-- > 0) {
		int cnt;
		if ((cnt = readtape(buf)) < 0) {
			fflush(stdout);
			fprintf(stderr, "tar: tape read error while skipping data\n");
			if (continflag)
				break;
			done(2);
		}
		if (cnt == 0)
			premature_eof();
	}
	return;
}

putfile(longname, shortname, parent)
	char *longname;
	char *shortname;
	char *parent;
{
	int infile = 0;
	long blocks;
	char buf[TBLOCK];
	register char *cp;
	struct dirent *dp;
	DIR *dirp;
	int i;
	int j;
	char newparent[NAMSIZ+64];
	extern int errno;
	int special;
	unsigned int dirents;	/* number of entries in directory */
	char **dirnms;
	extern int atstrcmp();

	/* strip off trailing slashes */
	for (cp = &longname[strlen(longname)]; *--cp == '/'; *cp = '\0')
		if (cp == longname)
			break;
	/* strip leading ./ or extra leading slashes */
	for (;;)
		if (longname[0] == '.' && longname[1] == '/') {
			longname += 2;
			while (*longname == '/')
				longname++;
		} else if (longname[0] == '/' && longname[1] == '/')
			longname++;
		else
			break;

#ifdef	S_IFLNK
	if (!Lflag)
#ifdef SVR3
		
		i = lstat(shortname, &stbuf);
#else
		i = clstat(shortname, &stbuf);
#endif SVR3
	else
#endif	S_IFLNK
		i = stat(shortname, &stbuf);

	if (i < 0) {
		switch (errno) {
		case EACCES:
			fprintf(stderr, "tar: %s: cannot access file\n",
			  longname);
			break;
		case ENOENT:
			fprintf(stderr, "tar: %s: no such file or directory\n",
			    longname);
			break;
		default:
			fprintf(stderr, "tar: %s: cannot stat file\n",
			  longname);
			break;
		}
		fflush(stderr);
		return;
	}
	if (tfile != NULL && checkupdate(longname) == 0)
		return;
	if (checkw('r', longname) == 0)
		return;
	if (Fflag && checkf(shortname, stbuf.st_mode, Fflag) == 0)
		return;

	switch (stbuf.st_mode & S_IFMT) {
	  case S_IFCHR:
		special = FCHR;
		break;
	  case S_IFBLK:
		special = FBLK;
		break;
#ifdef	S_IFIFO
	  case S_IFIFO:
		special = FIFO;
		break;
#endif
	  default:
		special = 0;
	}

	switch (stbuf.st_mode & S_IFMT) {
	case S_IFDIR:
		for (i = 0, cp = buf; *cp++ = longname[i++];)
			continue;
		/* add a trailing '/' if it doesn't already have one */
		if (cp[-2] != '/') {
			cp[-1] = '/';
			cp[0] = '\0';
		} else
			--cp;
		if (!dflag) {
			if ((cp - buf) >= NAMSIZ) {
			    fprintf(stderr, "tar: %s: file name too long\n",
				    longname);
			    fflush(stderr);
			    return;
			}
			stbuf.st_size = 0;
			tomodes(&stbuf);
			strcpy(dblock.dbuf.name,buf);
			sprintf(dblock.dbuf.chksum, "%6o", checksum());
			writetape((char *)&dblock);
		}
		sprintf(newparent, "%s/%s", parent, shortname);
		if (chdir(shortname) < 0) {
			perror(shortname);
			return;
		}
		if ((dirp = opendir(".")) == NULL) {
			fprintf(stderr, "tar: %s: directory read error\n",
			    longname);
			fflush(stderr);
			if (chdir(parent) < 0) {
				fprintf(stderr, "cannot change back?: ");
				fflush(stderr);
				perror(parent);
			}
			return;
		}
		dirents = 0;
		while ((dp = readdir(dirp)) != NULL) {
			if (dp->d_ino == 0)
				continue;
			if (!strcmp(".", dp->d_name) ||
			  !strcmp("..", dp->d_name))
				continue;
			dirents++;
		}
		if (!dirents)
			goto cleanup;
		closedir(dirp);
		dirents += DFUDGE;
		dirnms = (char **) calloc(dirents, sizeof (char *));
		if (dirnms == NULL) {
			fprintf(stderr, "tar: out of memory\n");
			fflush(stderr);
			goto cleanup;
		}
		if ((dirp = opendir(".")) == NULL) {
			fprintf(stderr, "tar: %s: directory read error\n",
			  longname);
			fflush(stderr);
			return;
		}
		j = 0;
		while ((dp = readdir(dirp)) != NULL && !term) {
			if (dp->d_ino == 0)
				continue;
			if (!strcmp(".", dp->d_name) ||
			    !strcmp("..", dp->d_name))
				continue;
			if (j >= dirents) {
				fprintf(stderr,"tar: directory changed size\n");
				fflush(stderr);
				break;
			}
			dirnms[j] = malloc(strlen(dp->d_name)+1);
			if (dirnms[j] == NULL) {
				fprintf(stderr, "tar: out of memory\n");
				fflush(stderr);
				goto cleanup;
			}
			strcpy(dirnms[j++], dp->d_name);
#ifdef notdef
			/*
			|| Why?
			*/
			i = telldir(dirp);
			closedir(dirp);
			dirp = opendir(".");
			seekdir(dirp, i);
#endif
		}
		if (j != dirents-DFUDGE) {
			fprintf(stderr,"tar: directory changed size\n");
			fflush(stderr);
		}
		dirents = j;
		qsort((char *) dirnms, dirents, sizeof (*dirnms), atstrcmp);
		for (j=0; j<dirents; j++) {
			strcpy(cp, dirnms[j]);
			putfile(buf, cp, newparent);
			free(dirnms[j]);
		}
		free(dirnms);

cleanup:
		closedir(dirp);
		if (chdir(parent) < 0) {
			fprintf(stderr, "cannot change back?: ");
			fflush(stderr);
			perror(parent);
		}
		break;
#ifdef	S_IFLNK

	case S_IFLNK:
		tomodes(&stbuf);
		if (strlen(longname) >= NAMSIZ) {
			fprintf(stderr, "tar: %s: file name too long\n",
			    longname);
			fflush(stderr);
			return;
		}
		strcpy(dblock.dbuf.name, longname);
		if (stbuf.st_size + 1 >= NAMSIZ) {
			fprintf(stderr, "tar: %s: symbolic link too long\n",
			    longname);
			fflush(stderr);
			return;
		}
		i = readlink(shortname, dblock.dbuf.linkname, NAMSIZ - 1);
		if (i < 0) {
			perror(longname);
			return;
		}
		dblock.dbuf.linkname[i] = '\0';
		dblock.dbuf.linkflag = '2';
		if (vflag) {
			fprintf(stderr, "a %s ", longname);
			fprintf(stderr, "symbolic link to %s\n",
			    dblock.dbuf.linkname);
			fflush(stderr);
		}
		sprintf(dblock.dbuf.size, "%11lo", 0);
		sprintf(dblock.dbuf.chksum, "%6o", checksum());
		writetape((char *)&dblock);
		break;
#endif	S_IFLNK

	case S_IFCHR:
	case S_IFBLK:
#ifdef	S_IFIFO
	case S_IFIFO:
#endif
		if (Dflag) {
			fprintf(stderr,
			  "tar: %s is not a regular file. Not dumped\n",
			  longname);
			fflush(stderr);
			break;
		}
		stbuf.st_size = 0;
		
		/* Fall Through */

	case S_IFREG:
		if (!special && (infile = open(shortname, 0)) < 0) {
			fprintf(stderr, "tar: %s: cannot open file\n",
			  longname);
			fflush(stderr);
			return;
		}
		tomodes(&stbuf);
		if (strlen(longname) >= NAMSIZ) {
			fprintf(stderr, "tar: %s: file name too long\n",
			    longname);
			fflush(stderr);
			if (!special)
				close(infile);
			return;
		}
		strcpy(dblock.dbuf.name, longname);
		if (stbuf.st_nlink > 1) {
			struct linkbuf *lp;
			int found = 0;

			for (lp = ihead; lp != NULL; lp = lp->nextp)
				if (lp->inum == stbuf.st_ino &&
				    lp->devnum == stbuf.st_dev) {
					found++;
					break;
				}
			if (found) {
				strcpy(dblock.dbuf.linkname, lp->pathname);
				dblock.dbuf.linkflag = '1';
				sprintf(dblock.dbuf.chksum, "%6o", checksum());
				writetape( (char *) &dblock);
				if (vflag) {
					fprintf(stderr, "a %s ", longname);
					fprintf(stderr, "link to %s\n",
					    lp->pathname);
					fflush(stderr);
				}
				lp->count--;
				if (!special)
					close(infile);
				return;
			}
			lp = (struct linkbuf *)
			     malloc((unsigned)
				     (strlen(longname) + sizeof *lp));
			if (lp == NULL) {
				if (freemem) {
					fprintf(stderr,
				"tar: out of memory, link information lost\n");
					fflush(stderr);
					freemem = 0;
				}
			} else {
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;
				strcpy(lp->pathname, longname);
			}
		}
			/* size was set to 0 for specials */
		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		if (vflag) {
			fprintf(stderr, "a %s ", longname);
			switch (special) {
			  case FCHR:
			  case FBLK:
				fprintf(stderr,"%s special (%d/%d)\n",
				  special == FCHR ? "char" : "block",
				  major((int) stbuf.st_rdev),
				  minor((int) stbuf.st_rdev));
				break;
			  case FIFO:
				fprintf(stderr,"pipe\n");
				break;
			  default:
				fprintf(stderr, "%ld block%s\n",
				  blocks,blocks!=1?"s":"");
			}
			fflush(stderr);
		}
		sprintf(dblock.dbuf.chksum, "%6o", checksum());
		writetape((char *)&dblock);

				/*
				 * special files are 0 bytes long
				 */
		if (!special) {
			while ((i = read(infile, buf, TBLOCK)) > 0
			  && blocks > 0) {
				writetape(buf);
				blocks--;
			}
			close(infile);
				/* assumes order of members in stat struct */
			if (aflag)
				utime(shortname, &stbuf.st_atime);
			if (blocks != 0 || i != 0) {
				fprintf(stderr, "tar: %s: file changed size\n",
				    longname);
				fflush(stderr);
				while (--blocks >=  0)
					putempty();
			}
		}
		break;

	default:
		fprintf(stderr, "tar: %s is not a file. Not dumped\n",
		    longname);
		fflush(stderr);
		break;
	}
	return;
}

doxtract(argv)
	char *argv[];
{
	long blocks, bytes;
	char buf[TBLOCK];
	int ifile;
	int ofile;
	char cmpfile[200 + NAMSIZ];
	char *fcmpfile;
	char tmpfile[200 + NAMSIZ];
	int same;
	int special;

	if (lnflag) {
		strcpy(cmpfile, linkdir);
		fcmpfile = &cmpfile[strlen(cmpfile)];
		strcpy(fcmpfile, "/");
		fcmpfile++;
	}
	while (getdir()) {
		if (*argv && !gotit(argv, dblockname)) {
			passtape();
			continue;
		}
		if (checkw('x', dblockname) == 0) {
			passtape();
			continue;
		}
		if (Fflag) {
			char *s;

			if ((s = rindex(dblockname, '/')) == 0)
				s = dblockname;
			else
				s++;
			if (checkf(s, stbuf.st_mode, Fflag) == 0) {
				passtape();
				continue;
			}
		}

		if (checkdir(dblockname))
			continue;
		if (dblock.dbuf.linkflag == '2') {
#ifndef S_IFLNK
			goto linkit;
#else	S_IFLNK
			unlink(dblockname);
			if (symlink(dblock.dbuf.linkname, dblockname)<0) {
				fprintf(stderr, "tar: %s: symbolic link failed\n",
				    dblockname);
				fflush(stderr);
				continue;
			}
			if (vflag) {
				fprintf(stderr, "x %s symbolic link to %s\n",
				    dblockname, dblock.dbuf.linkname);
				fflush(stderr);
			}
#ifdef notdef
			/* ignore alien orders */
			if (mflag == 0) {
				struct timeval tv[2];

				tv[0].tv_sec = time(0);
				tv[0].tv_usec = 0;
				tv[1].tv_sec = stbuf.st_mtime;
				tv[1].tv_usec = 0;
				utimes(dblockname, tv);
			}
			if (pflag)
				chmod(dblockname, (int)stbuf.st_mode & 07777);
			if (!oflag)
				chown(dblockname, stbuf.st_uid, stbuf.st_gid);
#endif
			continue;
#endif	S_IFLNK
		} else if (dblock.dbuf.linkflag == '1') {
#ifndef S_IFLNK
linkit:
#endif	S_IFLNK
			unlink(dblockname);
			if (link(dblock.dbuf.linkname, dblockname) < 0) {
				fprintf(stderr, "tar: %s: cannot link to %s\n",
				    dblock.dbuf.linkname, dblockname);
				fflush(stderr);
				continue;
			}
			if (vflag) {
				fprintf(stderr, "%s %s to %s\n",
				  dblockname,
				  dblock.dbuf.linkflag == '2'
				    ? "symlink changed to hard link"
				    : "linked",
				  dblock.dbuf.linkname);
				fflush(stderr);
			}
			continue;
		}
		ifile = -1;
		same = YES;
		if (lnflag) {
			static char tmpf[] = "TarXXXXXX";
			strcpy(fcmpfile, dblockname);
			if ((ifile = open(cmpfile, 0)) >= 0) {
			    tmpfile[0] = '\0';
			    strncat(tmpfile, dblockname,
				     dirpart(dblockname));
			    strcat(tmpfile, tmpf);
			    mktemp(tmpfile);
			}
		}
		if (ifile >= 0) {
			ofile = creat(tmpfile, (int) stbuf.st_mode & 0xfff);
		} else {
			if (Uflag)
				(void)unlink(dblockname);
			switch (stbuf.st_mode & ~07777) {
			  case FCHR:
				special = S_IFCHR;
				break;
			  case FBLK:
				special = S_IFBLK;
				break;
#ifdef	S_IFIFO
			  case FIFO:
				special = S_IFIFO;
				break;
#endif
			  default:
				special = 0;
			}
			if (special) {
				ofile = mknod(dblockname,
				  (int) (stbuf.st_mode & 07777) | special,
				  stbuf.st_rdev);
			} else
				if (dblockname[strlen(dblockname)] != '/')
					ofile = creat(dblockname,
					  (int) stbuf.st_mode & 07777);
		}
		if (ofile < 0) {
			fprintf(stderr, "tar: %s - cannot %s\n",
			  dblockname, special ? "mknod" : "create");
			fflush(stderr);
			passtape();
			continue;
		}

		blocks = ((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK;
#ifndef IGNORE_SCCSID
		if (ifile >= 0) {
			struct stat stbuf;

			if (fstat(ifile, &stbuf) < 0 || bytes != stbuf.st_size)
				same = NO;
		}
#endif
		if (vflag) {
			switch (special) {
			  case 0:
				fprintf(stderr,
				  "x %s, %ld bytes, %ld block%s",
				  dblockname, bytes, blocks, blocks!=1?"s":"");
				  break;
			  case S_IFCHR:
				fprintf(stderr, "x %s, char special (%d/%d)",
				  dblockname,
				  major(stbuf.st_rdev),
				  minor(stbuf.st_rdev));
				  break;
			  case S_IFBLK:
				fprintf(stderr, "x %s, char special (%d/%d)",
				  dblockname,
				  major(stbuf.st_rdev),
				  minor(stbuf.st_rdev));
				  break;
#ifdef	S_IFIFO
			  case S_IFIFO:
				fprintf(stderr, "x %s, pipe", dblockname);
				break;
#endif
			  default:
				fprintf(stderr,
					"unrecognized case: doextract\n");
			}
			fflush(stderr);
		}
		while (blocks-- > 0) {
			int nw;

			if ((nw = readtape(buf)) < 0) {
				fflush(stdout);
				if (vflag)
					fprintf(stderr, "\n");
				if (!continflag) {
					fprintf(stderr,
					"tar: tape read error\n");
					done(2);
				}
				fprintf(stderr, "tar: **Omitting data block**\n");
				fprintf(stderr, "tar: Discard file: %s\n\n",
						dblockname);
				fflush(stderr);
				continue;
			}
			if (nw == 0) {
				if (vflag)
					fprintf(stderr, "\n");
				premature_eof();
			}
			nw = MIN(bytes, TBLOCK);
			if (write(ofile, buf, nw) < nw) {
				if (vflag)
					fprintf(stderr, "\n");
				fprintf(stderr, "tar: %s: HELP - extract write error\n",
				    dblockname);
				done(2);
			}
#ifndef IGNORE_SCCSID
			if (ifile >= 0 && same)
				same = cmprd(ifile, buf, (long)nw);
#endif
			bytes -= TBLOCK;
		}
		if (!special)
			close(ofile);
		if (ifile >= 0) {
#ifdef IGNORE_SCCSID
			same = cmpsccsid(tmpfile, cmpfile);
#endif
			close(ifile);
			unlink(dblockname);
			if (vflag) {
				fprintf(stderr, " (%s)\n", same ? "same" : "new");
				fflush(stderr);
			}
			if (link(same ? cmpfile : tmpfile, dblockname) < 0) {
				fprintf(stderr, "tar: %s - cannot link\n",
				  dblockname);
				fflush(stderr);
				if (same && link(tmpfile, dblockname) < 0) {
					fprintf(stderr,
					  "tar: %s - cannot link\n",
					  dblockname);
					fflush(stderr);
				}
			}
			unlink(tmpfile);
		} else if (vflag) {
			fprintf(stderr, "\n");
			fflush(stderr);
		}
		if (ifile < 0 || !same) {
			if (mflag == 0) {
#ifdef	BSD42
				struct timeval tv[2];

				tv[0].tv_sec = time((time_t *)0);
				tv[0].tv_usec = 0;
				tv[1].tv_sec = stbuf.st_mtime;
				tv[1].tv_usec = 0;
				utimes(dblockname, tv);
#else	BSD42
				time_t tv[2];

				tv[0] = time((time_t *)0);
				tv[1] = stbuf.st_mtime;
				utime(dblockname, tv);
#endif	BSD42
			}
			if (pflag)
				chmod(dblockname, (int)stbuf.st_mode & 07777);
			if (!oflag)
				chown(dblockname, stbuf.st_uid, stbuf.st_gid);
		}
	}
	endread();
	return;
}

docompare(argv)
	char *argv[];
{
	long blocks, bytes;
	char buf[TBLOCK];
	int ifile;
	int same;

	while (getdir()) {
		if (*argv && !gotit(argv, dblockname)) {
			passtape();
			continue;
		}
		switch (dblock.dbuf.linkflag) {
		case '1':
			printf("L ");
			goto islink;
		case '2':
			printf("S ");
		islink:
			if (vflag)
				longt(&stbuf,dblockname);
			printf("%s", dblockname);
			printf(" linked to %s\n", dblock.dbuf.linkname);
			continue;
		}
		if (stbuf.st_mode & ~07777) {
			switch (stbuf.st_mode & ~07777) {
			  case FCHR:
				printf("C ");
				break;
			  case FBLK:
				printf("B ");
				break;
			  case FIFO:
				printf("P ");
			}
			if (vflag)
				longt(&stbuf,dblockname);
			printf("%s\n", dblockname);
			continue;
		}

		if ((ifile = open(dblockname, 0)) < 0) {
			struct stat stbuf;

			if (stat(dblockname, &stbuf) >= 0)
				printf("? ");
			else
				printf("> ");
			if (vflag)
				longt(&stbuf,dblockname);
			printf("%s\n", dblockname);
			passtape();
			continue;
		}

		same = YES;
		blocks = ((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK;
		{
			struct stat stbuf;
			if (fstat(ifile, &stbuf) < 0
			    || bytes != stbuf.st_size
			   )
				same = NO;
		}

		if (same)
			while (blocks-- > 0) {
				int nw;
				if ((nw = readtape(buf)) < 0) {
					fflush(stdout);
					if (!continflag) {
						fprintf(stderr,
						"tar: tape read error\n");
						done(2);
					}
					fprintf(stderr,
					  "tar: **Omitting data block**\n");
					fprintf(stderr,
					  "tar: Discard file: %s\n\n",
							dblockname);
					fflush(stderr);
					continue;
				}
				if (nw == 0)
					premature_eof();
				nw = MIN(bytes, TBLOCK);
				if (ifile >= 0 && same)
					same = cmprd(ifile, buf, (long)nw);
				bytes -= TBLOCK;
			}
		else
			passtape();
		close(ifile);

		if (same)
			printf("= ");
		else
			printf("! ");
		if (vflag)
			longt(&stbuf,dblockname);
		printf("%s\n", dblockname);
	}
	endread();
	return;
}

dotable()
{
	char buf[TBLOCK];

	while (getdir()) {
		if (vflag)
			longt(&stbuf,dblockname);
		printf("%s", dblockname);
		switch (dblock.dbuf.linkflag) {
		  case '1':
			printf(" linked to %s", dblock.dbuf.linkname);
			break;
		  case '2':
			printf(" symbolic link to %s", dblock.dbuf.linkname);
		}
		printf("\n");
		passtape();
	}
	endread();
	return;
}

putempty()
{
	static char zerobuf[TBLOCK];

	writetape(zerobuf);
	return;
}

longt(st,name)
	register struct stat *st;
	char	*name;
{
	register char *cp;
	char *ctime();

	pmode(st);
	printf(" %3d/%-2d ", st->st_uid, st->st_gid);
	switch (st->st_mode & ~07777) {
	  case 0:
		if (name[strlen(name)-1] == '/')
			printf("dir    ");	/* dirs don't have size */
		else
			printf("%7ld", st->st_size);
		break;
	  case FCHR:
		printf("c:%2d/%-2d", major(st->st_rdev), minor(st->st_rdev));
		break;
	  case FBLK:
		printf("b:%2d/%-2d", major(st->st_rdev), minor(st->st_rdev));
		break;
#ifdef	S_IFIFO
	  case FIFO:
		printf( "pipe   ");
		break;
#endif
	  default:
#ifdef DEBUG
		fprintf(stderr,
			"debug: unrecognized case: longt(st,name)\n");
		fflush(stderr);
#endif
		break;
	}
	cp = ctime(&st->st_mtime);
	printf(" %-12.12s %-4.4s ", cp+4, cp+20);
	return;
}

#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	STXT	01000
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode(st)
	register struct stat *st;
{
	register int **mp;

	for (mp = &m[0]; mp < &m[9];)
		select(*mp++, st);
	return;
}

select(pairp, st)
	int *pairp;
	struct stat *st;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (st->st_mode&*ap++)==0)
		ap++;
	printf("%c", *ap);
	return;
}

checkdir(name)
	register char *name;
{
	register char *cp;
	extern int errno;

	/*
	 * Quick check for existance of directory.
	 */
	if ((cp = rindex(name, '/')) == 0)
		return (0);
	*cp = '\0';
	if (access(name, 0) >= 0) {
		*cp = '/';
		return (cp[1] == '\0');
	}
	*cp = '/';

	/*
	 * No luck, try to make all directories in path.
	 * Assume "/" exists. Otherwise we will test "" which fails on
	 * System V, breaking tar.
	 */
	for (cp = name+1; *cp; cp++) {
		if (*cp != '/')
			continue;
		*cp = '\0';
		if (access(name, 0) < 0) {
			errno = 0;
			if (mkdir(name, 07777) < 0) {
				fflush(stdout);
				fprintf(stderr,"tar:can't mkdir ");
				if (errno)
					perror(name);
				else
					fprintf(stderr,"%s\n",name);
				*cp = '/';
					/*
					 * Nonzero to suppres creation of
					 * zero-length file preventing
					 * subsequent mkdir by hand after
					 * fixing problem.
					 */
				return -1;
			}
				/* .../me/ */
			if (pflag && cp[1] == '\0')
				chmod(name, (int)stbuf.st_mode & 07777);
			if (!oflag)
				chown(name, stbuf.st_uid, stbuf.st_gid);
		}
		*cp = '/';
	}
	return (cp[-1]=='/');	/* TRUE if only directory (*?/)		*/
}

onintr()
{
	signal(SIGINT, SIG_IGN);
	term++;
	return;
}

onquit()
{
	signal(SIGQUIT, SIG_IGN);
	term++;
	return;
}

onhup()
{
	signal(SIGHUP, SIG_IGN);
	term++;
	return;
}

#ifdef	notdef
onterm()
{
	signal(SIGTERM, SIG_IGN);
	term++;
	return;
}
#endif	notdef

tomodes(sp)
	register struct stat *sp;
{
	register char *cp;
	int i;

	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		*cp = '\0';
	switch (sp->st_mode & S_IFMT) {
	  case S_IFCHR:
		i = FCHR;
		break;
	  case S_IFBLK:
		i = FBLK;
		break;
#ifdef	S_IFIFO
	  case S_IFIFO:
		i = FIFO;
		break;
#endif
	  default:
		i = 0;
		break;
	}
	if (i)
		sp->st_size = 0;
	sprintf(dblock.dbuf.mode, "%6o ", (sp->st_mode & 07777) | i);
	sprintf(dblock.dbuf.uid, "%6o ", sp->st_uid);
	sprintf(dblock.dbuf.gid, "%6o ", sp->st_gid);
	sprintf(dblock.dbuf.size, "%11lo ", sp->st_size);
	sprintf(dblock.dbuf.mtime, "%11lo ", sp->st_mtime);
	sprintf(dblock.dbuf.rdev, "%11lo ", sp->st_rdev);
	return;
}

checksum()
{
	register i;
	register char *cp;

	for (cp = dblock.dbuf.chksum;
	     cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return (i);
}

checkw(c, name)
	char *name;
{
	if (!wflag)
		return (1);
	printf("%c ", c);
	if (vflag)
		longt(&stbuf, name);
	printf("%s: ", name);
	return (response() == 'y');
}

response()
{
	char c;
	int i;

	c = getchar();
	if (c != '\n')
		while ((i=getchar()) != '\n' && i != EOF)
			continue;
	else
		c = 'n';
	return (c);
}

checkf(name, mode, howmuch)
	char *name;
	u_short mode;
	int howmuch;
{
	int l;

	if ((mode & S_IFMT) == S_IFDIR)
		return (strcmp(name, "SCCS") != 0);
	if ((l = strlen(name)) < 3)
		return (1);
	if (howmuch > 1 && name[l-2] == '.' && name[l-1] == 'o')
		return (0);
	if (strcmp(name, "core") == 0 ||
	    strcmp(name, "errs") == 0 ||
	    (howmuch > 1 && strcmp(name, "a.out") == 0))
		return (0);
	/* SHOULD CHECK IF IT IS EXECUTABLE */
	return (1);
}

checkupdate(arg)
	char *arg;
{
	char name[100];
	long mtime;
	daddr_t seekp;
	daddr_t lookup();

	rewind(tfile);
	if ((seekp = lookup(arg)) < 0)
		return (1);
	fseek(tfile, seekp, 0);
	fscanf(tfile, "%s %lo", name, &mtime);
	return (stbuf.st_mtime > mtime);
}

done(n)
{
	unlink(tname);
	exit(n);
	/* NOTREACHED */
	return;
}

gotit(list, name)
	register char **list;
	register char *name;
{
	for (; *list; ++list)
		if (prefix(*list, name))
			return YES;
	return NO;
}

prefix(s1, s2)
	register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return (0);
	if (*s2)
		return (*s2 == '/');
	return (1);
}

#define	N	200
int	njab;

daddr_t
lookup(s)
	char *s;
{
	register i;
	daddr_t a;

	for (i=0; s[i]; i++)
		if (s[i] == ' ')
			break;
	a = bsrch(s, i, low, high);
	return (a);
}

daddr_t
bsrch(s, n, l, h)
	daddr_t l, h;
	char *s;
{
	register i, j;
	char b[N];
	daddr_t m, m1;

	njab = 0;

loop:
	if (l >= h)
		return (-1L);
	m = l + (h-l)/2 - N/2;
	if (m < l)
		m = l;
	fseek(tfile, m, 0);
	fread(b, 1, N, tfile);
	njab++;
	for (i=0; i<N; i++) {
		if (b[i] == '\n')
			break;
		m++;
	}
	if (m >= h)
		return (-1L);
	m1 = m;
	j = i;
	for (i++; i<N; i++) {
		m1++;
		if (b[i] == '\n')
			break;
	}
	i = cmp(b+j, s, n);
	if (i < 0) {
		h = m;
		goto loop;
	}
	if (i > 0) {
		l = m1;
		goto loop;
	}
	return (m);
}

cmp(b, s, n)
	char *b, *s;
{
	register i;

	if (b[0] != '\n')
		exit(2);
	for (i=0; i<n; i++) {
		if (b[i+1] > s[i])
			return (-1);
		if (b[i+1] < s[i])
			return (1);
	}
	return (b[i+1] == ' '? 0 : -1);
}

endread()
{
#ifndef	OLDMAGTAPE
	register int cnt;

	/* try to find end of file.  Give up after two tries */
	if ((cnt = readtape((char *) NULL)) != 0
	  && (cnt < 0 || readtape((char *) NULL) < 0)) {
		fflush(stdout);
		fprintf(stderr, "tar: tape read error at end\n");
		fflush(stderr);
	}
#endif
}

premature_eof()
{
	fflush(stdout);
	fprintf(stderr, "tar: tape premature EOF\n");
	done(2);
	return;
}

/* SGI: Variable blocking */
readtape(buffer)
	char *buffer;
{
	register int i;
	static int haderr;

	if (recno >= nblock || first == 0 || buffer == (char *) NULL) {
		if ((i = bread(mt, (char *) tbuf,
		  TBLOCK*(nblock?nblock:NBLOCK), buffer)) < 0) {
			fflush(stdout);
			fprintf(stderr, "tar: tape read error\n");
			fflush(stderr);
			if (continflag && !haderr) {
				haderr = 1;
				return -1;
			}
			done(3);
		}
		haderr = 0;
		if (i == 0)
			return 0;
		if (buffer == (char *) NULL)
			return i;
		if ((i % TBLOCK) != 0) {
			fprintf(stderr, "tar: tape blocksize error\n");
			done(3);
		}
		i /= TBLOCK;
		if (first == 0) {
#ifdef OLDMAGTAPE
			if (work == dorep && i != 1) {
				fprintf(stderr, "tar: Cannot update blocked tapes\n");
				done(1);
			}
#endif OLDMAGTAPE
			if (i != nblock && i != 1) {
				fprintf(stderr, "tar: blocksize = %d\n", i);
				fflush(stderr);
				nblock = i;
			}
		}
		nblock = i;
		recno = 0;
	}
	first = 1;
	bcopy((char *)&tbuf[recno++], buffer, TBLOCK);
	return (TBLOCK);
}

writetape(buffer)
	char *buffer;
{
	first = 1;
	if (nblock == 0)
		nblock = 1;
	if (recno >= nblock) {
		if (myio(write, mt, (char *) tbuf,
		  TBLOCK*nblock, buffer) != TBLOCK*nblock) {
			perror("tar: tape write error");
			done(2);
		}
		recno = 0;
	}
	bcopy(buffer, (char *)&tbuf[recno++], TBLOCK);
	if (recno >= nblock) {
		if (myio(write, mt, (char *) tbuf,
		  TBLOCK*nblock, buffer) != TBLOCK*nblock) {
			perror("tar: tape write error");
			done(2);
		}
		recno = 0;
	}
	return;
}

char	*eot_prompt = "\n\007End of Tape:\n\
  Mount next reel and type c (and RETURN) to continue\n";
char	*err_prompt = "\n\007Tape I/O Error or End of Tape:\n\
  Mount next reel and type c (and RETURN) to continue\n";

myio(fn, fd, ptr, n, eot)
int	(*fn)();
int	fd;
char	*ptr;
int	n;
char	*eot;		/* NULL -> don't change tapes */
{
	int	i, ch_count;
	int	tty;
	char	*prompt, c, sav_c;
	int tape_open = 1;

	i = (*fn)(fd, ptr, n);
	if (i != n && eot != NULL) {
		if (!isatape) {
			return i;	/* return short count if not tape */
		}
		if ((errno == ENOSPC) && (strcmp(usefile,"-") != 0))
			prompt = eot_prompt;
		else
			prompt = err_prompt;
		/*
		 * Here we're either at EOT, in which case we rewind tape
		 * (if the rewind device); or we've gotten an error.
		 */
		close(mt);			/* possible tape rewind */
		tape_open = 0;
		if ((tty = open("/dev/tty", 2)) < 0 && !isatty(tty = 0)) {
			fprintf(stderr, "can't prompt for new tape\n");
			done(2);
		}
		for (;;) {
			/* look for a c followed by a RETURN */
			do {
				write(tty, prompt, strlen(prompt));
				fflush(stdout);
				ch_count = 0;
				do {
					if (read(tty, &c, 1) < 0)
						done(2);
					if (ch_count++ == 0)
						sav_c = c;
				} while (c != '\n');
			} while ((sav_c != 'c') || (ch_count > 2));

			/* reopen mt and begin again */
			if (tape_open == 1) {
				close(mt);
				tape_open = 0;
			}
			if (strcmp(usefile,"-") == 0) {
				mt = dup(fn == read ? 0 : 1);
			} else {
			    if (fn == read)
				mt = open(usefile,0);
			    else {
				mt = open(usefile,1);
				if (mt < 0)
					mt = creat(usefile,0666);
			    }
			}
			if (mt < 0) {
				fprintf(stderr,"tar: can't %s ",
				  fn == read ? "open" : "creat");
				perror(usefile);
				fflush(stderr);
			} else
				tape_open = 1;
			if (mt >=0) {
				i = myio(fn, mt, ptr, n, eot);
				break;
			}
		}
		if (tty > 0)
			(void) close(tty);
	}
	return i;
}

#ifndef OLDMAGTAPE

backtape()
{
#ifdef	MAGTAPE
	static int mtdev = 1;
	static struct mtop mtop = {MTBSR, 1};
	struct mtget mtget;

	if (mtdev == 1)
		mtdev = ioctl(mt, MTIOCGET, &mtget);
	if (mtdev == 0) {
		if (ioctl(mt, MTIOCTOP, &mtop) < 0) {
			fprintf(stderr, "tar: tape backspace error\n");
			done(4);
		}
	} else
#endif	MAGTAPE
		lseek(mt, (long) -TBLOCK*nblock, 1);
	recno--;
	return;
}
#else	OLDMAGTAPE

backtape()
{
	lseek(mt, (long) -TBLOCK, 1);
	if (recno >= nblock) {
		recno = nblock - 1;
		if (read(mt, (char *) tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "tar: tape read error after seek\n");
			done(2);
		}
		lseek(mt, (long) -TBLOCK, 1);
	}
	return;
}
#endif	OLDMAGTAPE

/* SGI: Variable blocking */
flushtape()
{
	if (recno || !Vflag)
		write(mt, (char *) tbuf, TBLOCK*(Vflag?recno:nblock));
	return;
}

bread(fd, buf, size, eot)
	int fd;
	char *buf;
	int size;
	char *eot;	/* NULL -> don't switch tapes on error */
{
	int count;
	static int lastread = 0;

	if (!Bflag)
		return (myio(read, fd, buf, size, eot));
	for (count = 0; count < size; count += lastread) {
		if (lastread < 0) {
			if (count > 0)
				return (count);
			return (lastread);
		}
		lastread = myio(read, fd, buf, size - count, eot);
		if (debug)
			if (debug > 1 || lastread <= 0)
				fprintf(stderr,
			    "bread(fd=%d,buf=%u,bytes=%d,eot=%d) returned %d\n",
				  fd, buf, size - count, eot, lastread);
		if (lastread <= 0)
			if (lastread < 0)
				return lastread;
			else
				return count;
		buf += lastread;
	}
	return (count);
}

# ifdef notdef
#ifndef BSD42
bcopy(from, to, count)
	register char *to, *from;
	register int count;
{
	do {
		*to++ = *from++;
	} while (--count);
	return;
}
#endif	BSD42
# endif notdef

/*  Compare the next 'nw' characters of ifile with buf.
 *   return 1 if same, else 0
 */
cmprd(ifile, buf, num)
	int ifile;
	char *buf;
	long num;
{
	register int nr;
	char ibuf[BUFSIZ];

	for (; (nr = MIN(num, BUFSIZ)) > 0; num -= nr) {
		if (read(ifile, ibuf, nr) < nr)
			return NO;
		if (bufcmp(buf, ibuf, nr))
			return NO;
	}
	return YES;
}

#ifdef	BSD42

char *
getcwd(buf)
	char *buf;
{
	if (getwd(buf) == NULL) {
		fprintf(stderr, "tar: %s\n", buf);
		done(1);
	}
	return (buf);
}

#else	BSD42

char *
getcwd(buf)
	char *buf;
{
	int pipdes[2];
	int child, who;

	pipe(pipdes);
	if ((child = fork()) == 0) {
		close(1);
		dup(pipdes[1]);
		execl("/bin/pwd", "pwd", 0);
		execl("/usr/bin/pwd", "pwd", 0);
badpwd:
		fprintf(stderr, "tar: pwd failed!\n");
		printf("/\n");
		exit(1);
	}
	while ((who = wait((int *)NULL)) != -1) {
		if (who == child)
			break;
	}
	buf[0] = '\0';
	read(pipdes[0], buf, MAXPATHLEN);
	{
		register char *s;

		for (s = buf; *s != '\n'; )
			if (*s)
				s++;
			else
				break;
		if (*s != '\n')
			goto badpwd;
		*s = '\0';
	}
	close(pipdes[0]);
	close(pipdes[1]);
	return buf;
}

#endif	BSD42

bufcmp(cp1, cp2, num)
	register char *cp1;
	register char *cp2;
	register int num;
{
	if (num <= 0)
		return 0;
	do
		if (*cp1++ != *cp2++)
			return *--cp2 - *--cp1;
	while (--num);
	return 0;
}

dirpart(str)
	char *str;
{
	register char *cp;
	char *rindex();

	if (cp = rindex(str, '/'))
		return cp - str + 1;
	else
		return 0;
}

#ifdef IGNORE_SCCSID

cmpsccsid(f1, f2)
	char *f1;
	char *f2;
{
	char buf[1000];
	static char tmplate[] = "/usr/tmp/cmpXXXXXX";
	static char tmpbuf[sizeof tmplate];
	static char *tmpfile;
	int retval;

	if (!tmpfile) {
		tmpfile = tmpbuf;
		strcpy(tmpfile, tmplate);
		mktemp(tmpfile);
	}
	sprintf(buf, "grep -v '(#)' '%s' > '%s' ;\
grep -v '(#)' '%s' | cmp -s - '%s'", f1, tmpfile, f2, tmpfile);
	retval = !system(buf);
	unlink(tmpfile);
	return retval;
}
#endif IGNORE_SCCSID

#ifdef notdef
#ifndef BSD42
mkdir(name, mode)
	char *name;
	int mode;
{
	register int pid, rp;
	int status;

	if ((pid = fork()) == 0) {
		umask(07777 & ~mode);
		execl("/bin/mkdir", "mkdir", name, 0);
		execl("/usr/bin/mkdir", "mkdir", name, 0);
		fprintf(stderr, "tar: cannot find mkdir!\n");
		done(2);
	}
	while ((rp = wait(&status)) >= 0 && rp != pid)
		continue;
	return (status ? -1 : 0);
}
#endif	BSD42
#endif	notdef

atstrcmp(a,b)
char	**a;
char	**b;
{
	int	i;

	i = strcmp(*a,*b);
	return i;
}
