# include <sys/param.h>
# include <sys/fs.h>

# define SECTORSHIFT	9
# define SECTORSIZE	(1<<SECTORSHIFT)

/*
 * get_superblk_type() --
 *
 *	sb		ptr to purported superblock
 *	_magic		cell for magic number
 *	_fsbshift	cell for fs block shift
 *
 * determine the type of a purported superblock.
 * send back the magic number, and the fs block shift.
 *
 * return 0 on success, -1 on failure.
 */
int
get_superblk_type(sb, _magic, _fsbshift)
	char *sb;
	int (*_magic), *(_fsbshift);
{
	register int magic, fsbshift;

	magic = 0;
	fsbshift = 0;

	if (((struct filsys *)sb)->s_magic == FsMAGIC
	 && ((struct filsys *)sb)->s_type == Fs1b) {
		magic = FsMAGIC;
		fsbshift = SECTORSHIFT;
	}
	else
	if (((struct filsys *)sb)->s_magic == FsMAGIC
	 && ((struct filsys *)sb)->s_type == Fs2b) {
		magic = FsMAGIC;
		fsbshift = SECTORSHIFT+1;
	} else if (((struct efs *)sb)->fs_magic == EFS_MAGIC) {
		magic = EFS_MAGIC;
		fsbshift = BBSHIFT;
	}

	if (_magic != 0)
		*_magic = magic;
	if (_fsbshift != 0)
		*_fsbshift = fsbshift;

	if (magic == 0 || fsbshift == 0)
		return -1;
	return 0;
}

/*
 * read_superblk() --
 *
 *	fd		file descriptor for fs
 *	sp		area for superblock of fs
 *	_magic		cell for magic number
 *	_fsbshift	cell for fs block shift
 *
 * read in superblock, call get_superblk_type.
 *
 * return 0 on success, -1 on failure.
 */
int
read_superblk(fd, sp, _magic, _fsbshift)
	int fd;
	char *sp;
	int *(_magic), *(_fsbshift);
{
	extern off_t lseek();

	if (lseek(fd, SUPERBOFF, 0) < 0
	 || read(fd, sp, SECTORSIZE) < 0
	 || get_superblk_type(sp, _magic, _fsbshift) < 0)
		return -1;
	return 0;
}

/*
 * get_checker() --
 *
 *	magic		magic number
 *	fsbshift	fs block shift
 *	_checker	cell for ptr to name of fs check program
 *
 * given magic and fsbshift, determine name of appropriate checker.
 *
 * return 0 on success, -1 on failure.
 */
int
get_fsck(magic, fsbshift, _checker)
	int magic, fsbshift;
	char **(_checker);
{
	register char *checker;

	checker = 0;
	if (magic == FsMAGIC && fsbshift == SECTORSHIFT)
		checker = "/etc/fsck1b.bell";
	else
	if (magic == FsMAGIC && fsbshift == SECTORSHIFT+1)
		checker = "/etc/fsck.bell";
	else if (magic == EFS_MAGIC && fsbshift == BBSHIFT)
		checker = "/etc/fsck.efs";

	if (_checker != 0)
		*_checker = checker;
	if (checker == 0)
		return -1;
	return 0;
}

/*
 * efs_supersum() --
 *
 *	sp		pointer to efs superblock
 *
 * returns the efs checksum of the given superblock.
 */
long
efs_supersum(sp)
	register struct efs *sp;
{
	extern long efs_cksum();

	return efs_cksum((unsigned short *)sp,
		(unsigned short *)&sp->fs_checksum - (unsigned short *)sp);
}

/*
 * efs_cksum() --
 *
 *	src		ptr to data to be summed
 *	len		its length in shortwords
 *
 * returns the efs checksum of the given area.
 */
long
efs_cksum(src, len)
	register unsigned short *src;
	int len;
{
	register long a;

	a = 0;
	while (--len >= 0) {
		a ^= *src++;
		a <<= 1;
	}

	return a;
}
