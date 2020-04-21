#include <sys/param.h>
#include <sys/fs.h>
#include <ustat.h>
#include <sys/stat.h>

# define SECTORSHIFT	9
# define SECTORSIZE	(1<<SECTORSHIFT)

avail (name)
	char		*name;
{
	struct stat	st;
	struct ustat	ust;
	char		sb [SECTORSIZE];
	int		f, magic, fsbshift;

	stat (name, &st);
	ustat (st.st_rdev, &ust);
	f = open (name, 0);
	lseek (f, SUPERBOFF, 0);
	read (f, sb, SECTORSIZE);
	close (f);
	if (((struct filsys *)sb)->s_magic == FsMAGIC
	    && ((struct filsys *)sb)->s_type == Fs1b) {
		fsbshift = SECTORSHIFT;
	} else if (((struct filsys *)sb)->s_magic == FsMAGIC
	    && ((struct filsys *)sb)->s_type == Fs2b) {
		fsbshift = SECTORSHIFT+1;
	} else if (((struct efs *)sb)->fs_magic == EFS_MAGIC) {
		fsbshift = BBSHIFT;
	} else {
		fprintf (stderr, "bad.\n");
		exit (1);
	}
	return ((ust.f_tfree >> (fsbshift - SECTORSHIFT)) << fsbshift >> 10);
}
