# define EFS

/*
 * ieck [-break|-look] filsys inumber ...
 */
#include <sys/param.h>
#include <sys/types.h>
#ifdef EFS
# include <sys/fs.h>
# include <sys/inode.h>
# define ITOD(fs, i)	EFS_ITOBB(fs, i)
# define ITOO(fs, i)	EFS_ITOO(fs, i)
#else  EFS
# include <sys/ino.h>
# include <sys/filsys.h>
#endif EFS

int bsize, inosize, nbinode;

#ifdef	DEBUG
#define	debug(x,y)	printf(x,y)
#else
#define	debug(x,y)
#endif

struct filsys sblock;

char	*buf;
int	status;

main(argc, argv)
char *argv[];
{
	extern char *malloc();

	auto int magic, fsbshift;
	daddr_t min_iblock, max_iblock;
	char breakit = 0, lookatit = 0;

	register i, f;
	unsigned n;
	int ioff;
	daddr_t iblock;

	if(argc < 3) {
		printf("usage: ieck [-break|-look] filsys inumber ...\n");
		exit(4);
	}
	if (!strcmp(argv[1], "-break")) {
		breakit++, --argc, argv++;
	} else if (!strcmp(argv[1], "-look")) {
		lookatit++, --argc, argv++;
	}
	f = open(argv[1], 2);
	if(f < 0) {
		printf("cannot open %s\n", argv[1]);
		exit(4);
	}
	debug("SUPERBOFF=%ld\n",(long)SUPERBOFF);
	if (read_superblk(f, (char *)&sblock, &magic, &fsbshift) < 0) {
		printf("bad superblock of %s\n", argv[1]);
		exit(4);
	}
	bsize = 1 << fsbshift;
	inosize = sizeof (struct dinode);
	min_iblock = 2;
	max_iblock = sblock.s_isize;
# ifdef EFS
	if (magic == EFS_MAGIC) {
		register struct efs *sp;

		sp = (struct efs *)&sblock;
		sp->fs_ipcg = sp->fs_cgisize * EFS_INOPBB;
		inosize = sizeof (struct efs_dinode);
		min_iblock = sp->fs_firstcg;
		max_iblock = sp->fs_firstcg
				+ (sp->fs_ncg-1) * sp->fs_cgfsize
				+ sp->fs_cgisize;
	}
# endif EFS
	if ((buf = malloc(bsize)) == 0) {
		printf("bsize too big!\n");
		exit(4);
	}
	debug("BSIZE is %d\n", bsize);
	nbinode = bsize / inosize;
	debug("ISIZE is %d\n", inosize);
	debug("nbinode is %d\n",nbinode);
	for(i=2; i<argc; i++) {
		if(!isnumber(argv[i])) {
			printf("%s: is not a number\n", argv[i]);
			status = 1;
			continue;
		}
		n = atoi(argv[i]);
		if(n == 0) {
			printf("%s: is zero\n", argv[i]);
			status = 1;
			continue;
		}
		iblock = ((n-1)/nbinode + 2);
# ifdef EFS
		if (magic == EFS_MAGIC)
			iblock = ITOD((struct efs *)&sblock, n);
# endif EFS
		if (!(min_iblock <= iblock && iblock < max_iblock)) {
			printf("%s: invalid inumber\n", argv[i]);
			status = 1;
			continue;
		}
		lseek(f, iblock * bsize, 0);
		if(read(f, buf, bsize) != bsize) {
			printf("%s: read error\n", argv[i]);
			status = 1;
		}
	}
	if(status)
		exit(status);
	for(i=2; i<argc; i++) {
		n = atoi(argv[i]);
		printf("inode %u", n);
		iblock = ((n-1)/nbinode + 2);
# ifdef EFS
		if (magic == EFS_MAGIC)
			iblock = ITOD((struct efs *)&sblock, n);
# endif EFS
		lseek(f, iblock * bsize, 0);
		read(f, buf, bsize);
		ioff = (n-1)%nbinode;
# ifdef EFS
		if (magic == EFS_MAGIC) {
			ioff = ITOO((struct efs *)&sblock, n);
{
	struct efs_dinode *dp = (struct efs_dinode *) (buf + ioff*inosize);

	if (dp->di_numextents <= EFS_DIRECTEXTENTS) {
		printf(" has no indirect extents");
	} else {
		struct extent *ex = dp->di_u.di_extents;
		int ni = ex->ex_offset;

		ex += ni;
		ni = EFS_DIRECTEXTENTS - ni;
		if (breakit) {
			while (--ni >= 0) {
				ex->ex_length = 13;
				ex++;
			}
			printf(" broken");
		} else if (lookatit) {
			printf(":");
			while (--ni >= 0) {
				printf(" [%x,%d,%d,%d]", ex->ex_magic,
				    ex->ex_bn, ex->ex_length, ex->ex_offset);
				ex++;
			}
		} else {
			bzero((char *) ex, ni * sizeof *ex);
			printf(" fixed");
		}
	}
}
		}
# endif EFS
		lseek(f, iblock * bsize, 0);
		if (write(f, buf, bsize) != bsize)
			printf(": write error\n");
		else
			printf("\n");
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(c < '0' || c > '9')
			return(0);
	return(1);
}
