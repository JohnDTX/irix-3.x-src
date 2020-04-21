/*
 *	fsba (file system block analyzer) determines the 
 *	number of extra sectors (1 sector has 512 bytes) needed  
 *	when the file system logical block size is increased  
 *	from 512 bytes/block to 1024 bytes/block.
 */
char _Origin_[] = "System V";

/* @(#)fsba.c	1.2 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/fsba.c,v 1.1 89/03/27 14:50:32 root Exp $";
/*
 * $Log:	fsba.c,v $
 * Revision 1.1  89/03/27  14:50:32  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  85/04/30  10:02:23  bob
 * Added symbolic links.
 * 
 */
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ino.h>
#include <sys/stat.h>
#include <sys/filsys.h>
#define OBLKSIZ 512
#define NBLKSIZ 1024
#define OINDSIZ OBLKSIZ/sizeof (int)
#define NINDSIZ NBLKSIZ/sizeof (int)
#define MAXINO  INOPB * 10
char *dash ="--------------------------------------------";

main(argc, argv)
int argc;
char *argv[];
{
	register struct dinode *inop;
	struct dinode *getino();
	int blkold, blknew, blkwaste;
	int filnum;
	int old, new, waste;
	int nino ;
	float pwaste;
	extern int f_blks;

	if(argc == 1) {
		fprintf(stderr, "usage : %s file-system1 [file-system2 ...] \n", argv[0]);
		exit(1);
	}
	while (--argc) {
		printf("\n\n");
		blkold = blknew = blkwaste = filnum =  0;
		if ((nino = init(*++argv)) == 0)
			continue;
		printf(" total inodes :          %d\n", nino);
		while ((inop = getino()) != NULL) {
			switch(inop->di_mode & S_IFMT) {
			case S_IFDIR:	/* directory */
			case S_IFREG:   /* regular file */
#ifdef	S_IFLNK
			case S_IFLNK:   /* symbolic link */
#endif	S_IFLNK
				old = fileblk(inop->di_size, 1);
				new= fileblk(inop->di_size, 2) * 2;
				waste = new - old;
				blkold += old;
				blknew += new;
				blkwaste += waste;
			case S_IFBLK:   /* block special */
			case S_IFCHR:   /* character special */
			case S_IFIFO:   /* pipe */
				filnum++;
				break;
			default:
				/* must be free inode */
				break;
			}
		}
		pwaste = 100.0 * blkwaste / blkold;
		printf(" allocated inodes :      %d\n", filnum);
		printf(" free inodes :           %d\n", nino-filnum);
		printf("%s\n", dash);
		printf(" for old file system (512bytes/block)\n");
		printf(" allocated sectors :     %d\n", blkold);
		printf(" free sectors :          %d\n", f_blks-blkold);
		printf("%s\n", dash);
		printf(" for new file system (1024bytes/block)\n");
		printf(" allocated sectors :     %d\n", blknew);
		printf(" free sectors :          %d\n", f_blks-blknew);
		if(f_blks < blknew) 
			printf(" the entire volume size is not big enough\n");
		printf("%s\n", dash);
		printf(" extra sectors needed for new file system :\n");
		printf("                         %d   (%4.1f%%)\n", blkwaste, pwaste);
		printf(" * 1 sector contains 512 bytes\n");
	}
	exit(0);
}

/* n argument is byte size of a file, filblk function
 * returns the number of blocks allocated 
 */
fileblk(n, s)
long n;
int s;
{
	int db, bt, blksiz, indirt;

	if(s == 1) {	/* 512 byte/block file system */
		blksiz = OBLKSIZ;
		indirt = OINDSIZ;
	}
	if(s == 2) {	/* 1024 byte/block file system */
		blksiz = NBLKSIZ;
		indirt = NINDSIZ;
	}
	db = round(n, blksiz); 	/* compute the data blocks */
	if(db <= 10)		/* direct addresses */
		return(db);
	if(db <= 10 + indirt)	/* single indirect address */
		return(db + 1);
	if(db <= 10 + indirt * indirt) {	/* double indirect address */
		bt = round(db -10, indirt);
		return(db +1 + bt);
	}
	/* triple indirect address */
	bt = round(db-10, indirt);
	bt += round(bt, indirt);
	return(db + 1 + bt);
}

round(d1, d2)
register int d1, d2;
{
	return((d1%d2)? d1/d2 + 1 : d1/d2);
}



struct dinode *
getino()	/* read 10 blocks of inodes at a time */
{
	static struct dinode buf[MAXINO];
	static int ino_index = 0;
	extern n_inodes;
	extern int fp;

	if (n_inodes-- == 0)
		return(0);
	if ((ino_index == MAXINO) || (ino_index == 0)) {
		if (n_inodes >= MAXINO) {
			if (read(fp, buf, sizeof buf) < sizeof buf) {
				printf("read inode blocks error\n");
				return(0);
			}
			ino_index = 0;
		}
		else {
	   		if(read(fp, buf, sizeof(struct dinode) * (n_inodes + 1))
				< sizeof(struct dinode) * (n_inodes + 1)) {
	   			printf("read inode blocks error\n");
	   			return(0);
			}
			ino_index = 0;
		}
	}
	return(&buf[ino_index++]);	/* return the address of next inode */
}



#include <fcntl.h>
int fp;
int n_inodes;
int f_blks;

init(f)
char *f;
{
	struct filsys super;
	struct stat sbuf;

	if (stat(f, &sbuf) != 0) {
		fprintf(stderr, " can't stat file %s\n", f);
		return(0);
	}
	if (((sbuf.st_mode & S_IFMT) != S_IFCHR) &&
		((sbuf.st_mode & S_IFMT) != S_IFBLK)) {
		fprintf(stderr, " : block special or character special \n");
		return(0);
	}

	if ((fp = open(f, O_RDONLY)) < 0) {
		fprintf(stderr, " can't open file %s \n", f);
		return(0);
	}
	if (lseek(fp, (long)512, 0) < 0) {
		fprintf(stderr, " can't seek file %s\n", f);
		return(0);
	}
	if (read(fp, &super, sizeof super) < sizeof super) {
		fprintf(stderr, " can't read superblock of %s\n", f);
		return(0);
	}
	if (super.s_magic != FsMAGIC)
		super.s_type = Fs1b;
	switch (super.s_type) {
	case Fs1b:   
	     break;
	case Fs2b:   
	     printf(" %s is a 1k byte block file system already\n", f);
	     return(0);
	default:
	     fprintf(stderr, " %s: file system size error\n", f);
	     return(0);
	}
	f_blks = super.s_fsize - super.s_isize;
	printf(" file-sys name :         %s   (%s)\n", super.s_fname, f);
	n_inodes = (super.s_isize - 2) * INOPB;
	return(n_inodes);
}
