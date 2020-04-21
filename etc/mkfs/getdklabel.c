# include "sys/param.h"
# include "sys/dklabel.h"
# include "sys/stat.h"
# define D_FS(x)	((x)&07)
# include "ctype.h"

int
getdklabel(devname, _label, _part)
    char *devname;
    register struct disk_label *_label;
    register int *(_part);
{
    extern char *label_dev();

    union
    {
	char b[512];
	struct disk_label l;
    } junk;

    register char *cp;
    int fd;
    int mypart;

    cp = devname+strlen(devname);
    if( --cp < devname || (unsigned)(mypart = *cp-'a') >= 8 )
	return -1;

    if( (cp = label_dev(devname)) == 0 )
	return -1;

    if( (fd = open(cp)) < 0 )
	return -1;

    if( lseek(fd, 0L, 0) < 0 )
    {
	close(fd);
	return -1;
    }
    if( read(fd, junk.b, sizeof junk.b) < 0 )
    {
	close(fd);
	return -1;
    }

    *_label = junk.l;
    *_part = mypart;
    close(fd);
    return 0;
}

char *
label_dev(name)
	char *name;
{
	extern char *cook_dev();

	struct stat stat1;
	register char *cp,  *zp;
	dev_t orgdev;

	if (stat(name,  &stat1) < 0)
		return 0;
	if ((cp = cook_dev(name)) == 0)
		return 0;
	if (stat(cp,  &stat1) < 0 || (stat1.st_mode&S_IFMT) != S_IFBLK)
		return 0;
	orgdev = stat1.st_rdev;
	zp = cp + strlen(cp);
	if (!(--zp >= cp && islower(*zp)))
		return 0;
	*zp = 'h';
	if (stat(cp,  &stat1) < 0 || (stat1.st_mode&S_IFMT) != S_IFBLK)
		return 0;
	if (major(stat1.st_rdev) != major(orgdev)
	 || D_FS(stat1.st_rdev) != 'h'-'a')
		return 0;
	return cp;
}
