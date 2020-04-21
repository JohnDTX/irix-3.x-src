/*
 * df(1) - report disk free block counts and other statistics
 *
 * authors: Brendan Eich (filesystem independent part)
 *	    Donovan Fong (filesystem dependent part)
 *
 * $Source: /d2/3.7/src/bin/RCS/df.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 14:50:20 $
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <mntent.h>
#include <string.h>

#include <sys/param.h>
#define	KSHIFT		10
#define	BBTOK(bb)	((bb) >> (KSHIFT - BBSHIFT))

#include <sys/fstyp.h>
#include <sys/statfs.h>

#ifdef DEBUG
char	MTAB[] = "./mtab";
#else
char	MTAB[] = MOUNTED;
#endif

void	put_head(/* void */);
void	put_line(/* mntp */);

char	do_free_scan = 0;		/* force scan of freelist */
char	do_inode_stats = 0;		/* print used and free file counts */
char	halfK_units = 0;		/* print in half-K units */

int
main(argc, argv)
	register int argc;
	register char **argv;
{
	register char c;
	register int i, j;
	register struct mntent *mntp;
	register FILE *mtabp;

	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		for (j = 1; argv[i][j] != '\0'; j++) {
			switch (c = argv[i][j]) {

			case 'k':
				break;

			case 'b':
				halfK_units++;
				break;

			case 'f':
				do_free_scan++;
				break;

			case 'q':
				break;
			
			case 'i':
				do_inode_stats++;
				break;

			default:
				fprintf(stderr,
				    "df: unknown option -%c.\n", c);
				return usage();
			}
		}
	}

	if ((mtabp = setmntent(MTAB, "r")) == NULL) {
		return df_perror("cannot open", MTAB);
	}

	put_head();
	while ((mntp = getmntent(mtabp)) != NULL) {
		if (!strcmp(mntp->mnt_type, MNTTYPE_IGNORE)) {
			continue;
		}
		if (i < argc) {
			/*
			 * Check for argv/mtab overlap.
			 */
			for (j = i; j < argc; j++) {
				if (!strcmp(argv[j], mntp->mnt_fsname)
				    || !strcmp(argv[j], mntp->mnt_dir)) {
					put_line(mntp);
					argv[j][0] = '\0';
				}
			}
		} else {
			put_line(mntp);
		}
	}
	endmntent(mtabp);

	while (i < argc) {
		if (argv[i][0] != '\0') {
			struct mntent ment;

			ment.mnt_fsname = argv[i];
			ment.mnt_dir = ment.mnt_type = NULL;
			put_line(&ment);
		}
		i++;
	}
	return 0;
}

int
usage()
{
	fprintf(stderr, "usage: df [-bfikq] [filesystem ...]\n");
	return -1;
}

/*
 * With the -i (do_inode_stats) options, the header looks like this:

Filesystem        Type kbytes    use  avail %use   iuse ifree %iuse  Mounted on
|                 |    |     |      |      |    |      |     |     | |
123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789

 * The vertical bars indicate where the fields start.
 */

void
put_head()
{
	printf("Filesystem        Type kbytes    use  avail %%use");
	if (do_inode_stats) {
		printf("   iuse ifree %%iuse");
	}
	printf("  Mounted on\n");
}

#define PERCENT(amount, total) \
	(((amount) * 100 + (total) / 2) / (total))
#define	ZERO_PERCENT(amt, tot) \
	((amt) = 0, (tot) = 1)

void
put_line(mntp)
	register struct mntent *mntp;
{
	struct statfs sfsb;
	register long kbytes;
	register long used;

	/*
	 * Get filesystem statistics.  If do_free_scan, get free counts
	 * the hard way.
	 */
	if (stat_entry(mntp, &sfsb) != 0
	    || do_free_scan && scan_freelist(mntp, &sfsb) != 0) {
		return;
	}

	/*
	 * Compute size in kbytes and scale used and avail counts.
	 * Statfs returns counts in 512-byte units.
	 */
	kbytes = BBTOK(sfsb.f_blocks);
	if (! halfK_units) {
		sfsb.f_bfree = BBTOK(sfsb.f_bfree);
		sfsb.f_blocks = kbytes;
	}

	/*
	 * Print the new line, calculating percentages.
	 */
	if (sfsb.f_blocks > 0) {
		used = sfsb.f_blocks - sfsb.f_bfree;
	} else {
		ZERO_PERCENT(used, sfsb.f_blocks);
	}
	printf("%-18.17s%-5.4s%6ld%7ld%7ld%4ld%%",
	    mntp->mnt_fsname, mntp->mnt_type, kbytes, used, sfsb.f_bfree,
	    PERCENT(used, sfsb.f_blocks));
	if (do_inode_stats) {
		register long iused;

		if (sfsb.f_files > 0) {
			iused = sfsb.f_files - sfsb.f_ffree;
		} else {
			ZERO_PERCENT(iused, sfsb.f_files);
		}
		printf(" %6ld%6ld%5ld%%", iused, sfsb.f_ffree,
		    PERCENT(iused, sfsb.f_files));
	}
	if (mntp->mnt_dir != NULL) {
		printf("  %s", mntp->mnt_dir);
	} else {
		printf("  %-.6s", sfsb.f_fname);
	}
	putchar('\n');
}

int
stat_entry(mntp, sfsp)
	register struct mntent *mntp;
	register struct statfs *sfsp;
{
	register char *name;
	register int fd;
	register short fstyp;
	static char typename[FSTYPSZ];

	/*
	 * Open the filesys's root directory, or if we weren't given the
	 * root, open the filesystem device.
	 */
	name = mntp->mnt_dir != NULL ? mntp->mnt_dir : mntp->mnt_fsname;
	fd = open(name, 0);
	if (fd < 0) {
		return df_perror("cannot open", name);
	}

	/*
	 * If no directory is given, assume mntp->mnt_fsname is
	 * unmounted and loop over type indices trying to statfs it.
	 */
	if (mntp->mnt_dir == NULL) {
		register short nfstyp;

		nfstyp = sysfs(GETNFSTYP);
		for (fstyp = 1; fstyp < nfstyp; fstyp++) {
			if (fstatfs(fd, sfsp, sizeof *sfsp, fstyp) == 0)
				break;
		}
		if (fstyp == nfstyp) {
			fstyp = 0;
		}
	} else {
		fstyp = 0;
		if (fstatfs(fd, sfsp, sizeof *sfsp, fstyp) == 0) {
			fstyp = sfsp->f_fstyp;
		}
	}
#ifdef DEBUG
	pstatfs(mntp->mnt_fsname, sfsp);
#endif

	/*
	 * Now get the filesystem type's name and set it in mntp if it's
	 * not already set.  Check for a match if it is set.
	 */
	if (fstyp == 0 || fstyp != sfsp->f_fstyp
	    || sysfs(GETFSTYP, fstyp, typename) < 0
	    || close(fd) < 0) {
		return df_perror("cannot get status from", name);
	}
	if (mntp->mnt_type == NULL) {
		mntp->mnt_type = typename;
	} else if (strcmp(mntp->mnt_type, typename)) {
		fprintf(stderr,
		    "df: %s is type %s but its %s entry is type %s.\n",
		    mntp->mnt_fsname, typename, MTAB, mntp->mnt_type);
	}
	return 0;
}

int
df_perror(s1, s2)
	char *s1, *s2;
{
	fprintf(stderr, "df: %s ", s1);
	perror(s2);
	return errno;
}

/*
 * Filesystem-dependent code.
 */
#include <sys/fs.h>
#include <sys/inode.h>

int
scan_freelist(mntp, sfsp)
	register struct mntent *mntp;
	register struct statfs *sfsp;
{
	register int fd;
	register daddr_t bfree;
	union {
		char		block[BBSIZE];
		struct filsys	bell;
		struct efs	efs;
	} super;
	daddr_t efs_freescan(), bell_freescan();

	if (strcmp(mntp->mnt_type, MNTTYPE_EFS) != 0
#ifdef MNTTYPE_EFS2
	    && strcmp(mntp->mnt_type, MNTTYPE_EFS2) != 0
#endif
	    && strcmp(mntp->mnt_type, MNTTYPE_BELL) != 0) {
		return 0;
	}
	if ((fd = open(mntp->mnt_fsname, 0)) < 0) {
		return df_perror("cannot open", mntp->mnt_fsname);
	}
	if (bread(fd, SUPERB, (char *) &super, sizeof super)) {
		return df_perror("cannot read superblock on",
		    mntp->mnt_fsname);
	}
	if (super.efs.fs_magic == EFS_MAGIC
#ifdef EFS2_MAGIC
	    || super.efs.fs_magic == EFS2_MAGIC
#endif
	    ) {
		bfree = efs_freescan(fd, &super.efs, mntp->mnt_fsname);
	} else if (super.bell.s_magic == FsMAGIC) {
		bfree = bell_freescan(fd, &super.bell, mntp->mnt_fsname);
	} else {
		fprintf(stderr, "df: unrecognizable superblock on %s.\n",
		    mntp->mnt_fsname);
		return -1;
	}
	if (bfree < 0) {
		fprintf(stderr,
		    "df: cannot scan freelist for filesystem %s.\n",
		    mntp->mnt_fsname);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "%s freescan %ld, bfree %ld\n",
	    mntp->mnt_fsname, bfree, sfsp->f_bfree);
#endif
	sfsp->f_bfree = bfree;
	return 0;
}

daddr_t
bell_freescan(fd, sp, filesys)
	int fd;
	struct filsys *sp;
	char *filesys;
{
	register daddr_t freeblocks;
	register int i;
	register daddr_t b;
	struct fblk fbuf;

	freeblocks = 0;

	/*
	 * copy superblock free info to private fblk buf.
	 */
	fbuf.df_nfree = sp->s_nfree;
	bcopy((char *)sp->s_free, (char *)fbuf.df_free, sizeof sp->s_free);
	b = 0;

	for (;;) {
		if (fbuf.df_nfree == 0)
			break;

		if (fbuf.df_nfree < 0 || fbuf.df_nfree > NICFREE) {
			fprintf(stderr,
			    "df: bad free count for block %ld on %s.\n",
			    b, filesys);
			return -1;
		}

		freeblocks += fbuf.df_nfree;

		b = fbuf.df_free[0];
		if (b == 0) {
			freeblocks--;
			break;
		}

		if (b < sp->s_isize || b >= sp->s_fsize) {
			fprintf(stderr, "df: bad free block %ld on %s.\n",
			    b, filesys);
			return -1;
		}

		if (bread(fd, b, (char *) &fbuf, sizeof fbuf)) {
			fprintf(stderr,
			    "df: cannot read free block %ld on %s.",
			    b, filesys);
			return -1;
		}
	}

	return freeblocks;
}

long lseek();

int
bread(fd, bno, buf, cnt)
	register int fd, cnt;
	register daddr_t bno;
	register char *buf;
{
	register int n;

	lseek(fd, BBTOB(bno), 0);
	if ((n = read(fd, buf, cnt)) != cnt) {
		return errno;
	}
	return 0;
}

daddr_t
efs_freescan(fd, sp)
	int fd;
	register struct efs *sp;
{
	daddr_t bmblocks;
	char bmbuf[BBSIZE];
	register char *cp;
	register daddr_t fsize;
	register char b;
	register int ib, ic;
	register daddr_t f;

	f = 0;
	fsize = sp->fs_size;
	if (lseek(fd, EFS_BITMAPBOFF, 0) < 0)
		return -1;

	bmblocks = BTOBB(sp->fs_bmsize);
	while (--bmblocks >= 0) {
		if (read(fd, bmbuf, sizeof bmbuf) != sizeof bmbuf)
			return -1;
		cp = bmbuf;
		for (ic = BBSIZE; --ic >= 0;) {
			b = *cp++;
			for (ib = BITSPERBYTE; --ib >= 0;) {
				if (--fsize < 0)
					break;
				if (b & 01)
					f++;
				b >>= 1;
			}
		}
	}

	return f;
}

#ifdef DEBUG
pstatfs(name, sfsp)
	char *name;
	register struct statfs *sfsp;
{
	fprintf(stderr, "\
statfs(%s) = {\n\
	fstyp %d,\n\
	bsize %ld,\n\
	frsize %ld,\n\
	blocks %ld,\n\
	bfree %ld,\n\
	files %ld,\n\
	ffree %ld,\n\
	fname %.6s,\n\
	fpack %.6s\n\
}\n",
	    name,
	    sfsp->f_fstyp,
	    sfsp->f_bsize,
	    sfsp->f_frsize,
	    sfsp->f_blocks,
	    sfsp->f_bfree,
	    sfsp->f_files,
	    sfsp->f_ffree,
	    sfsp->f_fname,
	    sfsp->f_fpack);
}
#endif
