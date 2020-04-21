/*
 * NAME
 *	isefs - test whether a filesystem is Bell
 * SYNOPSIS
 *	isefs device
 */
#define EFS

#include <sys/param.h>
#include <sys/fs.h>

#define	perr(msg)	write(2, msg, sizeof msg)

main(argc, argv)
	int argc;
	char *argv[];
{
	register int fs;
	struct efs sb;
	extern int errno;

	if (argc <= 1)
		return -1;
	if ((fs = open(argv[1], 0)) < 0) {
		perr("isefs: can't open filesystem device\n");
		return errno;
	}
	if (lseek(fs, SUPERBOFF, 0) < 0) {
		perr("isefs: can't seek to superblock\n");
		return errno;
	}
	if (read(fs, &sb, sizeof sb) != sizeof sb) {
		perr("isefs: can't read superblock\n");
		return errno;
	}
	return sb.fs_magic != EFS_MAGIC;	/* 0 if efs, 1 otherwise */
}
