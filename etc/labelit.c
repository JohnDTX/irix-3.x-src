char _Origin_[] = "System V";
static char sccsid[] = "@(#)labelit.c	1.5";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/etc/RCS/labelit.c,v 1.1 89/03/27 15:38:06 root Exp $";
/*
 * $Log:	labelit.c,v $
 * Revision 1.1  89/03/27  15:38:06  root
 * Initial check-in for 3.7
 * 
 * Revision 1.9  87/08/26  15:07:46  brendan
 * Recompute EFS superblock checksum after modifying checked fields!
 * 
 * Revision 1.8  86/03/04  19:35:08  kipp
 * *** empty log message ***
 * 
 * Revision 1.7  85/08/09  17:09:50  fong
 * changed to use superblk.o
 * 
 * Revision 1.4  85/03/12  01:17:36  bob
 * Fixed typo.
 * 
 * Revision 1.3  85/03/12  01:02:08  bob
 * Fixed SCR 587 by taking out message "... hit DEL to ..." and the sleep(10).
 * Also cleaned up some error messages.
 * 
 * Revision 1.2  85/03/08  23:40:06  bob
 * *** empty log message ***
 * 
 */

#include <sys/param.h>
# ifdef EFS
# include "sys/fs.h"
# include "sys/efs_ino.h"
# else  EFS
#include <sys/sysmacros.h>
#include <sys/filsys.h>

long	efs_supersum();		/* imported from libtoyfs.a */
# endif EFS

#ifndef	RT
#include <signal.h>
#include <sys/types.h>
#endif

#include <stdio.h>

#define DEV 1
#define FSNAME 2
#define VOLUME 3
#define	TAPE	4
 /* write fsname, volume # on disk superblock */
struct {
	char fill1[DEV_BSIZE];
	union {
		char fill2[DEV_BSIZE];
		struct filsys fs;
# ifdef EFS
		struct efs xfs;
# endif EFS
	} f;
} super;

# include "fincfrec.h"
struct Tphdr Tape_hdr;

int	IsTape;

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

main(argc, argv) char **argv; {
int fsi, fso;
long curtime;
int i;
int magic, fsbshift;

	signal(SIGALRM, sigalrm);

#ifdef RT
	setio(-1,1);	/* use physical io */
#endif

	if(argc!=4 && argc!=2 && argc!=5)  {
showusage:
#ifdef	RT
		fprintf(stderr,"Usage: labelit /dev/??? [fsname volume [-n]]\n");
#else
		fprintf(stderr,"Usage: labelit /dev/r??? [fsname volume [-n]]\n");
#endif
		exit(2);
	}
	if(argc==5) {
		if(strcmp(argv[TAPE], "-n"))
			goto showusage;
		else
			IsTape	= 1;
		printf("labelit:skipping label check\n");
		goto do_it;
	}

	if((fsi = open(argv[DEV],0)) < 1) {
		fprintf(stderr, "labelit: cannot open device\n");
		exit(2);
	}

	if(IsTape) {
		alarm(5);
		read(fsi, &Tape_hdr, sizeof(Tape_hdr));
		alarm(0);
		if(!(equal(Tape_hdr.t_magic, "Volcopy", 7)||
		    equal(Tape_hdr.t_magic,"Finc",4))) {
			fprintf(stderr, "labelit: tape not labelled\n");
			exit(2);
		}
		printf("%s tape volume: %s, reel %d of %d reels\n",
			Tape_hdr.t_magic, Tape_hdr.t_volume, Tape_hdr.t_reel, Tape_hdr.t_reels);
		printf("Written: %s", ctime(&Tape_hdr.t_time));
		if(argc==2 && Tape_hdr.t_reel>1)
			exit(0);
	}
	if((i=read(fsi, &super, sizeof(super))) != sizeof(super))  {
		fprintf(stderr, "labelit: cannot read superblock\n");
		exit(2);
	}
	if (get_superblk_type(&super.f, &magic, &fsbshift) < 0) {
		fprintf(stderr, "labelit: unrecognizable superblock\n");
		exit(2);
	}

#define	S	super.f.fs
	printf("Current fsname: %.6s, Current volname: %.6s,",
		S.s_fname, S.s_fpack, S.s_fsize);
# ifdef EFS
	if (magic == EFS_MAGIC) {
		printf(" Blocks: %ld, Inodes: %d\nFS Units: 512b, ",
			super.f.xfs.fs_size,
			(super.f.xfs.fs_cgisize * super.f.xfs.fs_ncg)
				* EFS_INOPBB);
		printf("Date last mounted: %s", ctime(&super.f.xfs.fs_time));
	}
	else
# endif EFS
	if (magic == FsMAGIC && S.s_type == Fs2b) {
		printf(" Blocks: %ld, Inodes: %d\nFS Units: 1Kb, ",
			S.s_fsize * 2, (S.s_isize - 2) * 16);
		printf("Date last mounted: %s", ctime(&S.s_time));
	}
	else {
		printf(" Blocks: %ld, Inodes: %d\nFS Units: 512b, ",
			S.s_fsize, (S.s_isize - 2) * 8);
		printf("Date last mounted: %s", ctime(&S.s_time));
	}
	if(argc==2)
		exit(0);
do_it:
	printf("NEW fsname = %.6s, NEW volname = %.6s\n",
	  argv[FSNAME], argv[VOLUME]);
	sprintf(super.f.fs.s_fname, "%.6s", argv[FSNAME]);
	sprintf(super.f.fs.s_fpack, "%.6s", argv[VOLUME]);
# ifdef EFS
	if (super.f.xfs.fs_magic == EFS_MAGIC) {
		sprintf(super.f.xfs.fs_fname, "%.6s", argv[FSNAME]);
		sprintf(super.f.xfs.fs_fpack, "%.6s", argv[VOLUME]);
	}
	super.f.xfs.fs_checksum = efs_supersum(&super.f.xfs);
# endif EFS

	close(fsi);
	fso = open(argv[DEV],1);
	if(IsTape) {
		strcpy(Tape_hdr.t_magic, "Volcopy");
		sprintf(Tape_hdr.t_volume, "%.6s", argv[VOLUME]);
		if(write(fso, &Tape_hdr, sizeof(Tape_hdr)) < 0)
			goto cannot;
	}
	if(write(fso, &super, sizeof(super)) < 0) {
cannot:
		fprintf(stderr, "labelit: cannot write label\n");
		exit(2);
	}
	exit(0);
}
equal(s1, s2, ct)
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
