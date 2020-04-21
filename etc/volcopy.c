# define EFS
char _Origin_[] = "System V";

/*	volcopy	COMPILE:	cc -O volcopy.c -s -i -o volcopy	*/
/*	u370	COMPILE:	cc -O -b1,1 volcopy.c -s -i -o volcopy	*/

/*	@(#)volcopy.c	4.9	*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/etc/RCS/volcopy.c,v 1.1 89/03/27 15:38:53 root Exp $";
/*
 * $Log:	volcopy.c,v $
 * Revision 1.1  89/03/27  15:38:53  root
 * Initial check-in for 3.7
 * 
 * Revision 1.7  86/04/04  15:24:54  root
 * no longer part of the distribution, teb
 * 
 * Revision 1.6  85/07/11  18:12:34  fong
 * *** empty log message ***
 * 
 * Revision 1.5  85/07/11  17:13:14  fong
 * added efs support
 * 
 * Revision 1.4  85/03/12  01:22:45  bob
 * Fixed SCR 587 by removing "hit DEL to abort" and sleep(10).
 * 
 * Revision 1.3  85/03/08  23:40:14  bob
 * *** empty log message ***
 * 
 */

# define DEBUG
short volcopy_debug = 0;

#define LOG
#define AFLG 0 
#ifdef RT
#include <rt/types.h>
#include <rt/param.h>
#include <rt/fmgr/filsys.h>
#include <rt/stat.h>
#else
#include <sys/param.h>

# ifdef EFS
# include "sys/fs.h"
# else  EFS
#include <sys/filsys.h>
# endif EFS

#include <sys/stat.h>
#include <signal.h>
#endif
#include <stdio.h>
#include <signal.h>
#define FILE_SYS 1
#define DEV_FROM 2
#define FROM_VOL 3
#define DEV_TO 4
#define TO_VOL 5
#define	T_TYPE	0xfd187e20	/* like FsMAGIC */
#ifdef RT
#define IFTAPE(s) (EQ(s,"/dev/mt",7)||EQ(s,"mt",2))
#else
#ifndef	sgi
#define IFTAPE(s) (EQ(s,"/dev/rmt",8)||EQ(s,"rmt",3)||EQ(s,"/dev/rtp",8)||EQ(s,"rtp",3))
#else
#define IFTAPE(s)	(EQ(s,"/dev/rmt",8) || EQ(s,"rmt",3) || \
			   EQ(s,"/dev/mt",7) || EQ(s,"mt",2) || \
			   EQ(s,"/dev/rqic",9) || EQ(s,"rqic",4))
#endif	sgi
#endif	RT

#ifdef	u370
#define BLKSIZ	4096
#else
#define	BLKSIZ	512	/* use physical blocks */
#endif
#define _2_DAYS 172800L
#undef	MAX
#define MAX 1000000L
#ifdef	u370
#define Ft800x10	2L	/* u370 */
#define Ft1600x10	4L	/* u370 */
#define Ft6250x50	15L	/* u370 */
#else
#define Ft800x10	15L
#define Ft1600x4	22L
#define Ft1600x10	28L
#define Ft6250x10	90L
#define Ft6250x50	120L
#ifdef	sgi
#define	Ft10000x400	200L		/* streaming cartridge tape size */
#endif
#endif

struct Tphdr {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time;
	long	t_length;
	long	t_dens;
	long	t_reelblks;	/* u370 added field */
	long	t_blksize;	/* u370 added field */
	long	t_nblocks;	/* u370 added field */
	char	t_fill[468];
	int	t_type;		/* does tape have nblocks field? (u3b) */
} Tape_hdr;

int	K_drive = 0;		/* Kennedy tape drive? (4 blks/rec max) */
int	first = 0;
char	**args;
int	Nblocks = 0;
long	Reelblks = MAX;
int	Reels = 1;
int	reel = 1;
int	Reelsize = 0;
long	rblock = 0, reeloff = 0;
long	saveFs;
int	Bpi = 0;
int	bufflg = 0;
long	Fs;
short	*Buf;
int	Magic;
char	EFin, EFout;
long	Fstype;
int	sts;
int	p_in, p_out;
int	streaming;
extern	int errno;
/*

filesystem copy with propagation of volume ID and filesystem name:

  volcopy [-options]  filesystem /dev/from from_vol /dev/to to_vol

options are:
	-feet - length of tape
	-bpi  - recording density
	-reel - reel number (if not starting from beginning)
	-buf  - use double buffered i/o (if dens >= 1600 bpi)
	-a    - ask "y or n" instead nothing"
	-s    - inverse of -a

  Examples:

  volcopy root /dev/rrp2 pk5 /dev/rrp12 pk12

  volcopy u3 /dev/rrp15 pk1 /dev/rmt0 tp123

  volcopy u5 /dev/rmt0 -  /dev/rrp15 -

In the last example, dashed volume args mean "use label that's there."

From/to devices are printed followed by `?'.
With '-a' option, a positive user response is required to continue.
With '-s' option, -a is cancelled, some override questions are bypassed.
 */

long	Block;
char *Totape, *Fromtape;
FILE	*Devtty;
char	*Tape_nm;
int	pid;

struct filsys	Superi, Supero, *Sptr;
# ifdef EFS
struct efs	EfsTemp;
# endif EFS

extern int	read(), write();

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

sigint()
{
	if(pid != 1){
		if(asks("Want Shell?   "))
			system("sh");
		else if(asks("Want to quit?    ")) {
			if(pid) kilchld();
			exit(2);
		}
	}
	signal(SIGINT, sigint);
}
char kilcmd[] = "kill -9 000000";
kilchld()
{
	sprintf(&kilcmd[8],"%d",pid -1);
	system(kilcmd);
}
char *tapeck();

main(argc, argv) char **argv;
{
	int	fsi, fso;
	struct	stat statb;
	long	tvec;
	int	i, altflg = AFLG;
	FILE	*fb, *popen();
	char	vol[12], dev[12], c;
	int	fsbshift;

	signal(SIGALRM, sigalrm);

	if (argc < 6)
		goto bad;
	while(argv[1][0] == '-') {
		if(EQ(argv[1], "-bpi", 4))
			if((c = argv[1][4]) >= '0' & c <= '9')
				Bpi = getbpi(&argv[1][4]);
			else {
				++argv;
				--argc;
				Bpi = getbpi(&argv[1][0]);
			}
		else if(EQ(argv[1], "-feet", 5))
			if((c = argv[1][5]) >= '0' & c <= '9')
				Reelsize = atoi(&argv[1][5]);
			else {
				++argv;
				--argc;
				Reelsize = atoi(&argv[1][0]);
			}
		else if(EQ(argv[1],"-reel",5))
			if((c = argv[1][5]) >= '0' & c <= '9')
				reel = atoi(&argv[1][5]);
			else {
				++argv;
				--argc;
				reel = atoi(&argv[1][0]);
			}
		else if(EQ(argv[1],"-buf",4))
			bufflg++;
		else if(EQ(argv[1],"-a",2))
			altflg++;
		else if(EQ(argv[1],"-stream",7))
			streaming++;
		else if(EQ(argv[1],"-s",2))
			altflg = 0;
		else {
			fprintf(stderr, "<%s> invalid option\n",
				argv[1]);
			exit(1);
		}
		++argv;
		--argc;
	}
	args = argv;

	Devtty = fopen("/dev/tty", "r");
	time(&tvec);
			/* get mandatory inputs */
	if(argc!=6){
	bad:	fprintf(stderr,"Usage: volcopy [options] fsname");
		fprintf(stderr," /devfrom volfrom /devto volto\n");
		exit (9);
	}

	if((fsi = open(argv[DEV_FROM],0)) < 1)
		fprintf(stderr, "%s: ",argv[DEV_FROM]), err("cannot open");
	if((fso = open(argv[DEV_TO],0)) < 1)
		fprintf(stderr, "%s: ",argv[DEV_TO]), err("cannot open");

#ifdef RT
	if((fstat(fsi, &statb)<0) || (((statb.st_mode&S_IFMT)!=S_IFBLK)
		&& ((statb.st_mode & S_IFMT) != S_IFREC)
		&& ((statb.st_mode & S_IFMT) != S_IFCHR)))
		err("From device not character, block, or record special");
	if(fstat(fso, &statb)<0 || (((statb.st_mode&S_IFMT)!=S_IFBLK)
		&& ((statb.st_mode & S_IFMT) != S_IFREC)
		&& ((statb.st_mode & S_IFMT) != S_IFCHR)))
		err("To device not character, block, or record special");
#else
# ifdef DEBUG
	if (!volcopy_debug)
# endif DEBUG
	if(fstat(fsi, &statb)<0 || (statb.st_mode&S_IFMT)!=S_IFCHR)
		err("From device not character-special");
# ifdef DEBUG
	if (!volcopy_debug)
# endif DEBUG
	if(fstat(fso, &statb)<0 || (statb.st_mode&S_IFMT)!=S_IFCHR)
		err("To device not character-special");
#endif

	Fromtape = argv[DEV_FROM]; /* this will get reset if not appropriate
			but is needed by tapeck's label processing */
	Fromtape = tapeck(argv[DEV_FROM], argv[FROM_VOL], fsi);
	Totape = tapeck(argv[DEV_TO], argv[TO_VOL], fso);
	if(Totape && Fromtape)
		err("Use dd(1) command to copy tapes");

#ifdef RT
	setio(-1,1);	/* use physical io */
#endif

#ifdef	u370
	Nblocks = (Totape||Fromtape)? 8:8; /* disk-disk can even be > 8 */
	Buf = (short *)malloc(BLKSIZ*(Nblocks+1));
	/* Force buffer to page boundary */
	Buf = (short *)((int)((char *)Buf+BLKSIZ) & ~(BLKSIZ-1));

	if ((int)Buf <= 1) {
		fprintf(stderr, "Not enough memory--get help\n");
		exit(1);
	}
#else
#ifdef	sgi
	if (streaming) {
		Nblocks = (400*1024)/BLKSIZ;
		printf("volcopy: using 400k buffering for streaming\n");
	} else
		Nblocks = (200*1024)/BLKSIZ;
	Buf = (short *)sbrk(BLKSIZ*Nblocks);
#else

#ifdef	u3b
	if (K_drive)
		Nblocks = 4;
	else
		Nblocks = ((Totape||Fromtape)&&(Bpi!=6250))? 10:152;
#else
	Nblocks = ((Totape||Fromtape)&&(Bpi!=6250))? 10:88;
#endif	u3b
#ifdef	u3b
	if(Bpi == 6250) Nblocks = 10;
#else
	if(Bpi == 6250) Nblocks = 50;
#endif	u3b
	Buf = (short *)sbrk(BLKSIZ*Nblocks);
#ifdef	u3b
	if((int)Buf == -1 && Nblocks ==152) {
		Nblocks = 32;
#else
	if((int)Buf == -1 && Nblocks ==88) {
		Nblocks = 22;
#endif	u3b
		Buf = (short *)sbrk(BLKSIZ*Nblocks);
	}
	if((int)Buf == -1) {
		fprintf(stderr, "Not enough memory--get help\n");
		exit(1);
	}
#endif	sgi
#endif	u370
	Sptr = (struct filsys *)&Buf[BLKSIZ/2];
	if(!Fromtape && !Totape) reel = 1;
	if((reel == 1) || !Fromtape){
		if(read(fsi, Buf, 2*BLKSIZ) < 2*BLKSIZ)
			err("read error on input");

		if (get_superblk_type(Sptr, &Magic, &fsbshift) < 0)
			err("unknown superblk type");
		Fstype = 1 << (fsbshift - 9);
# ifdef EFS
		if (Magic == EFS_MAGIC) {
			EfsTemp = *(struct efs *)Sptr;
			efs_to_bell(&EfsTemp, Sptr);
		}
# endif EFS
		Fs = Sptr->s_fsize * Fstype;
		strncpy(Superi.s_fname,  Sptr->s_fname,6);
		strncpy(Superi.s_fpack,  Sptr->s_fpack,6);

		Superi.s_fsize = Sptr->s_fsize;
		Superi.s_time = Sptr->s_time;
	}

	if(read(fso, Buf, 2*BLKSIZ) < 2*BLKSIZ){
		fprintf(stderr,"Read error on output\n");
		if(!Totape | !altflg) ask();
	}
	strncpy(Supero.s_fname, Sptr->s_fname,6);
	strncpy(Supero.s_fpack, Sptr->s_fpack,6);
	Supero.s_fsize = Sptr->s_fsize;
	Supero.s_time = Sptr->s_time;

	if((reel != 1) && Fromtape){	/* if this isn't reel 1,
			the TO_FS better have been initialized */
                printf("\volcopy: IF REEL 1 HAS NOT BEEN RESTORED,");
		printf(" STOP NOW AND START OVER ***\n");
		if(!asks(" Continue? ")) exit(9);
		strncpy(Superi.s_fname,argv[FILE_SYS],6);
		strncpy(Superi.s_fpack,argv[FROM_VOL],6);
	}
	if(Totape){
		Reels = Fs / Reelblks + ((Fs % Reelblks) && 1);
		printf("You will need %d reels.\n", Reels);
		printf("(\tThe same size and density");
		printf(" is expected for all reels)\n");
			/* output vol name was validated already */
		strncpy(Tape_hdr.t_volume,argv[TO_VOL],6);
		strncpy(Supero.s_fpack,argv[TO_VOL],6);
		strncpy(vol,argv[TO_VOL],6);
		strncpy(Supero.s_fname,argv[FILE_SYS],6);
	}
	if(Fromtape){
		if((Tape_hdr.t_reel != reel
			|| Tape_hdr.t_reels!=Reels)){
			fprintf(stderr, "Tape disagrees: Reel %d of %d",
				Tape_hdr.t_reel, Tape_hdr.t_reels);
			fprintf(stderr," : looking for %d of %d\n",
				reel,Reels);
			ask();
		}
		strncpy(vol,Tape_hdr.t_volume,6);
	}

	if(!EQ(argv[FILE_SYS],Superi.s_fname, 6)) {
		printf("arg. (%.6s) doesn't agree with from fs. (%.6s)\n",
			argv[FILE_SYS],Superi.s_fname);
		if(!Totape | !altflg) ask();
	}
	if(!EQ(argv[FROM_VOL],"-", 6) &
	   !EQ(argv[FROM_VOL],Superi.s_fpack, 6)) {
		printf("arg. (%.6s) doesn't agree with from vol.(%.6s)\n",
			argv[FROM_VOL],Superi.s_fpack);
		if(!Totape | !altflg) ask();
	}

	if(argv[FROM_VOL][0]=='-') argv[FROM_VOL] = Superi.s_fpack;
	if(argv[TO_VOL][0]=='-') argv[TO_VOL] = Supero.s_fpack;

	if((reel == 1) & (Supero.s_time+_2_DAYS > Superi.s_time)) {
		printf("%s less than 48 hours older than %s\n",
			argv[DEV_TO], argv[DEV_FROM]);
		printf("To filesystem dated:  %s", ctime(&Supero.s_time));
		if(!altflg) ask();
	}
	if(!EQ(argv[TO_VOL],Supero.s_fpack, 6)) {
		printf("arg.(%.6s) doesn't agree with to vol.(%.6s)\n",
			argv[TO_VOL],Supero.s_fpack);
		ask();
		strncpy(Supero.s_fpack,  argv[TO_VOL],6);
	}
	if(Superi.s_fsize > Supero.s_fsize && !Totape) {
		printf("from fs larger than to fs\n");
		ask();
	}
	if(!Totape && !EQ(Superi.s_fname,Supero.s_fname, 6)) {
		printf("warning! from fs(%.6s) differs from to fs(%.6s)\n",
			Superi.s_fname,Supero.s_fname);
		if(!altflg) ask();
	}

COPY:
	printf("From: %s, to: %s? ",
		argv[DEV_FROM], argv[DEV_TO]);
	if(altflg){
		if(!asks("(y or n) ")) {
					printf("\nvolcopy: STOP\n");
					exit(9);
					}
	}
	close(fso); close(fsi);
	sync();
	fsi = open(argv[DEV_FROM], 0);
	fso = open(argv[DEV_TO], 1);
	if(Totape) {
		Tape_hdr.t_reels = Reels;
		Tape_hdr.t_reel = reel;
		Tape_hdr.t_time = tvec;
		Tape_hdr.t_reelblks = Reelblks;
		Tape_hdr.t_blksize = BLKSIZ*Nblocks;
		Tape_hdr.t_nblocks = Nblocks;
		Tape_hdr.t_type = T_TYPE;
		write(fso, &Tape_hdr, sizeof Tape_hdr);
		strcpy(vol,argv[TO_VOL]);
	} else if(Fromtape) {
		read(fsi, &Tape_hdr, sizeof Tape_hdr);
	}
	if(reel > 1) {
		Fs = (reel -1) * Reelblks + Nblocks;
		lseek(Totape ? fsi : fso,(unsigned)(Fs * BLKSIZ),0);
		Sptr = Totape? &Superi: &Supero;
		Fs = (Sptr->s_fsize * Fstype) - Fs;
	}
	rprt(vol);

	signal(SIGINT, sigint);

	while(copy(fsi,fso))
		chgreel(Totape ? fso : fsi, dev,vol);
	printf("  END: %ld blocks.\n", Block);

#ifdef LOG
	fslog(argv);
#endif
	exit(0);
}

err(s) char *s; {
	printf("%s\n\t%d reel(s) completed\n",s,--reel);
	exit(9);
}
EQ(s1, s2, ct)
char *s1, *s2;
int ct;
{
	register i;

	for(i=0; i<ct; ++i) {
		if(*s1 == *s2) {;
			if(*s1 == '\0') return(1);
			s1++; s2++;
			continue;
		} else return(0);
	}
	return(1);
}
ask() {
	char ans[12];
	printf("Type `y' to override:     ");
	fgets(ans, 10, Devtty);
	if(EQ(ans,"y", 1))
		return;
	if(EQ(ans,"a",1))
		abort();
	exit(9);
}
asks(s)
char *s;
{
	char ans[12];
	printf(s);
	ans[0] = '\0';
	fgets(ans, 10, Devtty);
	for(;;){
		switch(ans[0]) {

		case 'a':
			if(pid == 1) {
				write(p_out,"ABORT",1);
				exit(1);
			}
			if(pid) kilchld();
			abort();
		case 'y':
			return(1);
		case 'n':
			return(0);
		default:
			printf("\n(y or n)?");
			fgets(ans, 10, Devtty);
		}
	}
}

getbpi(inp)
char *inp;
{
#ifdef u3b
	if (inp[4] == 'k' || inp[4] == 'K') {
		K_drive++;
		inp[4] = '\0';
	}
#endif
	return(atoi(inp));
}

char *tapeck(dev, vol, fd)
char *dev, *vol;
{
	char	resp[16];

	if(!IFTAPE(dev))
		return 0;
	Tape_nm = dev;
	Tape_hdr.t_magic[0] = '\0';	/* scribble on old data */
	alarm(5);
	if(read(fd, &Tape_hdr, sizeof Tape_hdr) <= 0)
		printf("\n ERR %d\n",errno);
	alarm(0);
	if(!EQ(Tape_hdr.t_magic, "Volcopy", 7)){
		fprintf(stderr,"Not a labeled tape\n");
		if(!Fromtape){
			ask();
			makelab();
			strncpy(Tape_hdr.t_volume, vol, 6);
			Supero.s_time = 0;
		}
		else err();
	}
	else if(Tape_hdr.t_reel == (char)0)
		if(Fromtape){
			fprintf(stderr,"Input tape is empty\n");
			exit(9);
		}
	if((vol[0] != '-') && (!EQ(Tape_hdr.t_volume, vol, 6))) {
		fprintf(stderr, "Header volume(%.6s) does not match %s\n",
			Tape_hdr.t_volume, vol);
		ask();
		strncpy(Tape_hdr.t_volume, vol, 6);

	}
tapein:
	if(Fromtape){
		Reels = Tape_hdr.t_reels;
		Reelsize = Tape_hdr.t_length;
		Bpi = Tape_hdr.t_dens;
#ifdef u3b
		if (Tape_hdr.t_type == T_TYPE)
			if (Tape_hdr.t_nblocks == 4)
				K_drive++; /* use Kennedy tapedrive limit */
#endif
	}
	else{
		Reels = 0;
	}
#ifdef	sgi
	Reelsize = 450;
	Bpi = 10000;			/* or so we pretend to be */
	Reelblks = Ft10000x400 * Reelsize;
#else
	if(Reelsize == 0) {
		printf("Enter size of reel in feet for <%s>:   ", vol);
		fgets(resp, 10, Devtty);
		Reelsize = atoi(resp);
	}
#ifdef	u370
	if(Reelsize <= 0 || Reelsize > 3600) {
		fprintf(stderr, "Size of reel must be > 0, <= 3600\n");
#else
	if(Reelsize <= 0 || Reelsize > 2400) {
		fprintf(stderr, "Size of reel must be > 0, <= 2400\n");
#endif	u370
		Reelsize = 0;
		goto tapein;
	}
	if(!Bpi) {
		printf("Tape density? (i.e., 800 | 1600 | 6250)?   ");
		fgets(resp, 10, Devtty);
		Bpi = getbpi(resp);
	}
	if(Bpi == 800)
		Reelblks = Ft800x10 * Reelsize;
	else if(Bpi == 1600) {
#ifdef	u3b
		if (K_drive)
			Reelblks = Ft1600x4 * Reelsize;
		else
#endif	u3b
			Reelblks = Ft1600x10 * Reelsize;
	}
	else if(Bpi == 6250)
#ifdef u3b
		Reelblks = Ft6250x10 * Reelsize;
#else
		Reelblks = Ft6250x50 * Reelsize;
#endif u3b
	else {
		fprintf(stderr, "Bpi must be 800, 1600, or 6250\n");
		Bpi = 0;
		goto tapein;
	}
#endif	sgi
	printf("\nReel %.6s",Tape_hdr.t_volume);
	Tape_hdr.t_length = Reelsize;
	printf(", %d feet",Reelsize);
	Tape_hdr.t_dens = Bpi;
	printf(", %d BPI\n",Bpi);
	return dev;
}
hdrck(fd, tvol)
char *tvol;
{
	struct Tphdr *thdr;
	int siz;

	thdr = (struct Tphdr *) Buf;
	alarm(15);	/* dont scan whole tape for label */
	if((siz = read(fd, thdr, sizeof Tape_hdr)) != sizeof Tape_hdr) {
		alarm(0);
		fprintf(stderr, "Cannot read header\n");
		if(Totape){
			ask();
			strncpy(Tape_hdr.t_volume, tvol, 6);
			return(1);
		}
		else{
			close(fd);
			return 0;
		}
	}
	alarm(0);
	Tape_hdr.t_reel = thdr->t_reel;
	if(!EQ(thdr->t_volume, tvol, 6)) {
		fprintf(stderr, "Volume is <%.6s>, not <%s>.\n",
			thdr->t_volume, tvol);
		if(asks("Want to override?   ")) {
			if(Totape) {
				strncpy(Tape_hdr.t_volume, tvol, 6);
			}
			else{
				strncpy(tvol,thdr->t_volume,6);
			}
			return 1;
		}
		return 0;
	}
	return 1;
}
makelab()
{
	int i;

	for(i = 0; i < sizeof Tape_hdr; i++)
		Tape_hdr.t_magic[i] = '\0';
	strncpy(Tape_hdr.t_magic,"Volcopy\0",8);
}
rprt(vol)
char *vol;
{
	if(Totape)
		printf("\nWriting REEL %d of %d, VOL = %.6s\n",
		  reel,Reels,vol);
	if(Fromtape)
		printf("\nReading REEL %d of %d, VOL = %.6s\n",
		  reel,Reels,vol);
}
#ifdef LOG
fslog(argv)
char *argv[];
{
	char cmd[100];

	if(access("/etc/log/filesave.log", 6) == -1) {
		fprintf(stderr,
			"volcopy: cannot access /etc/log/filesave.log\n");
		exit(0);
	}
	system("tail -200 /etc/log/filesave.log >/tmp/FSJUNK");
	system("cp /tmp/FSJUNK /etc/log/filesave.log");
	sprintf(cmd,"echo \"%s;%.6s;%.6s -> %s;%.6s;%.6s\n\" >>/etc/log/filesave.log",
		argv[DEV_FROM], Superi.s_fname, Superi.s_fpack, 
		argv[DEV_TO], Supero.s_fname,
		 Supero.s_fpack);
	system(cmd);
	system("rm /tmp/FSJUNK");
	exit(0);
}
copy(fsi,fso)
int fsi,fso;
{
	int i, cnt, pos;
	int p1[2], p2[2];
	char buf[20];

	pid = -1;
	if(bufflg && (Bpi >= 1600)) {
		if( pipe(p1) | pipe(p2)) {
			printf("\volcopy: cannot open pipe, err = %d\n",errno);
			exit(1);
		}
		pid = fork();
		if(pid) {
			close(p1[0]);
			close(p2[1]);
			p_in = p2[0];
			p_out = p1[1];
		}
		else {
			close(p1[1]);
			close(p2[0]);
			p_in = p1[0];
			p_out = p2[1];
			write(p_out,"rw",2);	/* prime the pipe */
		}
	}
	pid++;		/* pid is >0 if we forked */
			/* child has pid == 1 */

		/* copy from fsi to fso */

	while((Fs > 0) && (rblock < Reelblks)) {
		Nblocks = Fs > Nblocks ? Nblocks : Fs;

		if(pid) {
			if(pid == 1) {
				Fs -= Nblocks;
				if(Fs <= 0) goto cfin;
				Block += Nblocks;
				rblock += Nblocks;
				Nblocks = Fs > Nblocks ? Nblocks : Fs;
			}
			cnt = read(p_in,buf,1);
			if(cnt < 0 | buf[0] != 'r') {
			   if(pid == 1) {
				write(p_out,"R",1);
				exit(1);
			   }
			   else {
				piperr(buf);
			   }
			}

		}

		if((sts = read(fsi, Buf, BLKSIZ * Nblocks)) != BLKSIZ * Nblocks) {
			printf("Read error %d block %ld...\n", sts, Block);
			for(i=0; i != Nblocks * (BLKSIZ/2); ++i) Buf[i] = 0;
			if(!Fromtape)
				lseek(fsi,(long)((Block+Nblocks) * BLKSIZ), 0);
		}
		/*
		 * This code was moved out of main (which did the
		 * first read & write) so that the same number of
		 * blocks would be written on each tape--a change
		 * required for finc, frec, and ff compatibility.
		 */
		if(!first && pid != 1 && reel == 1){
			first++;
			strncpy(Sptr->s_fpack,  args[TO_VOL],6);
			strncpy(Sptr->s_fname,  args[FILE_SYS],6);
		}
		if(pid) {
			write(p_out,"r",1);	/* signal read complete */
			cnt = read(p_in,buf,1);
			if(cnt < 0 | buf[0] != 'w') {
				if(pid == 1) {
					write(p_out,"W",1);
					exit(1);
				}
				piperr(buf);
			}
		}

		if((sts = write(fso, Buf, BLKSIZ*Nblocks)) != BLKSIZ*Nblocks) {
			printf("Write error %d, block %ld...\n",
				errno,Block);
			if(Totape) {
				if(pid == 1) {
					write(p_out,"Tape error",10);
					exit(1);
				}
		oterr:		if(asks("Want to try another tape?   ")) {
					asks("Type `y' when ready:   ");
					--reel;
					Block = reeloff;
					Fs = saveFs;
					lseek(fsi, (long)reeloff*BLKSIZ, 0);
					return(1);
				}
			} else {
				exit(9);
			}
		}

		if(pid) {
			write(p_out,"w",1);	/* signal write complete */
			if(pid != 1) {
				Fs -= Nblocks;
				Block += Nblocks;
				rblock += Nblocks;
				Nblocks = Fs > Nblocks ? Nblocks : Fs;
			}
		}
		Fs -= Nblocks;
		Block += Nblocks;
		rblock += Nblocks;
	}
	if(pid == 1) {
cfin:		write(p_out,"Done",4);
		while (cnt < 0 | buf[0] != 'D') {
			cnt = read(p_in,buf,1);
		}
		exit(0);
	}
	else if(pid) {
pfin:		cnt = read(p_in,buf,1);
/*
 * Ihcc code debugs some end condition problems
 */
		if ((Fs + Nblocks) > 0) {
			if (cnt < 0 | buf[0] != 'r') piperr(buf);
			cnt = read(p_in, buf, 1);
			if (cnt < 0 | buf[0] != 'w') piperr(buf);
			cnt = read(p_in, buf, 1);
		}
/***/
		if(cnt < 0 | buf[0] != 'D') piperr(buf);
		write(p_out,"Done",4);
		close(p_in);
		close(p_out);
	}
	return((Fs > 0) ? 1 : 0);
}

chgreel(fs,dev,vol)
int fs;
char *dev, *vol;
{
	char ctemp[21];

	rblock = 0;
	reeloff = Block;
	saveFs = Fs;
	++reel;
again:
	if((sts = close(fs)) < 0)
		printf("\n ERR %d\n",errno);
	 printf("Changing drives? (type RETURN for no, ");
#ifdef RT
		printf("`/dev/mt_' for yes: ");
#else
		printf("`/dev/rmt_' for yes: ");
#endif
	fgets(ctemp, 20, Devtty);
	ctemp[strlen(ctemp) -1] = '\0';
	if(ctemp[0] != '\0')
		while(strncmp(ctemp,"/dev/rmt",8)){
		  printf("\n'%s' is not a valid device",ctemp);
		  printf("\nenter device name `/dev/rmt?' : ");
		  fgets(ctemp, 20, Devtty);
		  ctemp[strlen(ctemp) -1] = '\0';
		}
		strcpy(dev,ctemp);
	printf("Mount tape %d\nType volume-ID when ready:   ",
		reel);
	fgets(ctemp, 10, Devtty);
	ctemp[strlen(ctemp) -1] = '\0';
	if(ctemp[0] != '\0') /*if only <cr> - use old vol-id */
		strcpy(vol,ctemp);
	if(*dev)
		Tape_nm = dev;
	if(Totape) {
		fs = open(Tape_nm, 0);
		if(fs <= 0 || fs > 10)
			printf("\n ERR %d\n",errno);
		if(!hdrck(fs, vol))
			goto again;
		Tape_hdr.t_reel = reel;
		close(fs);
		sleep(2);
		fs = open(Tape_nm, 1);
		if(fs <= 0 || fs > 10)
			printf("\n ERR %d\n",errno);
		if(write(fs, &Tape_hdr, sizeof Tape_hdr) < 0) {
 				 fprintf(stderr, "Cannot re-write header");
		 fprintf(stderr,"-%s\n","Try again!");
		  goto again;
		}
	} else {
		fs = open(Tape_nm, 0);
		if(fs <= 0 || fs > 10)
			printf("\n ERR %d\n",errno);
		if(!hdrck(fs, vol))
			goto again;
		if(Tape_hdr.t_reel != reel) {
		  fprintf(stderr,"Need reel %d,",reel);
		  fprintf(stderr," label says reel %d\n",
			Tape_hdr.t_reel);
		  goto again;
		}
	}
	rprt(vol);
}
piperr(pbuff)
char pbuff[];
{
	if(pbuff[0] == 'R')
		printf("\volcopy: read sequence error");
	
	else if(pbuff[0] == 'W')
		printf("\volcopy: write sequence error");
	
	else
		printf("\nvolcopy: pipe error = %d",errno);

	printf(" pipe buffer: %.10s\n",pbuff);
	printf(" reel %d, %d blocks\n",reel, Block);
	kilchld();
/*	if(pbuff[0] == 'A') */
		abort();
	exit(1);
}

# ifdef EFS
efs_to_bell(esp, bsp)
	register struct efs *esp;
	register struct filsys *bsp;
{
	bsp->s_fsize = esp->fs_size;
	strncpy(bsp->s_fname, esp->fs_fname, sizeof bsp->s_fname);
	strncpy(bsp->s_fpack, esp->fs_fpack, sizeof bsp->s_fpack);
	bsp->s_time = esp->fs_time;
	bsp->s_magic = EFS_MAGIC;
	bsp->s_type = Fs1b;
}

bell_to_efs(bsp, esp)
	register struct filsys *bsp;
	register struct efs *esp;
{
	esp->fs_size = bsp->s_fsize;
	esp->fs_time = bsp->s_time;
	strncpy(esp->fs_fname, bsp->s_fname, sizeof esp->fs_fname);
	strncpy(esp->fs_fpack, bsp->s_fpack, sizeof esp->fs_fpack);
	esp->fs_magic = EFS_MAGIC;
}

/*
 * bell format:
	tape
		thdr rawdata
	disk
		rawdata
 * efs format:
	tape
		thdr superblk inodes rawdata
	disk
		rawdata
 *
 */
# endif EFS
