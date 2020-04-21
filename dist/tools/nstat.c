#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define NULL 0

#define copybuff(obuff, buff) \
	(buff->st_dev = obuff->st_dev, buff->st_ino = obuff->st_ino, \
	buff->st_mode = obuff->st_mode, buff->st_nlink = obuff->st_nlink, \
	buff->st_uid = obuff->st_uid, buff->st_gid = obuff->st_gid, \
	buff->st_rdev = obuff->st_rdev, buff->st_size = obuff->st_size, \
	buff->st_atime = obuff->st_atime, buff->st_mtime = obuff->st_mtime, \
	buff->st_ctime = obuff->st_ctime)

extern int	stat ();
extern int	oldstat ();
int		convstat ();

static int	failed;

caught (sig)
	int		sig;
{
	if (sig == SIGSYS) failed = 1;
}

nstat (path, buff)
	char		*path;
	register struct stat	*buff;
{
	static int	(*statfunc) () = NULL;
	int		r, (*savesig) ();

	if (statfunc != NULL) return (statfunc (path, buff));
	failed = 0;
	savesig = signal (SIGSYS, caught);
	r = (statfunc = stat) (path, buff);
	if (failed) r = (statfunc = convstat) (path, buff);
	signal (SIGSYS, savesig);
	return (r);
}

convstat (path, buff)
	char		*path;
	register struct stat	*buff;
{
	struct oldstat	st;
	register struct oldstat	*obuff = &st;
	int		r;

	r = oldstat (path, obuff);
	copybuff (obuff, buff);
	return (r);
}
