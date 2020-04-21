# include "sys/types.h"
# include "sys/sysmacros.h"
# include "sys/dir.h"
# include "sys/stat.h"

# define filetype(sp)	((sp)->st_mode&S_IFMT)

# include "ctype.h"


/*
 * try to associate a cooked dev with
 * some garbage name.
 */
char *
cook_dev(name)
	register char *name;
{
	extern char *dev_name();
	extern char *basename();

	register char *bp;
	struct stat stat1;

	if ((name = dev_name(name, 1)) == 0)
		return 0;

	if (stat(name, &stat1) < 0) {
		free(name);
		return 0;
	}

	if (filetype(&stat1) == S_IFCHR
	 && *(bp = basename(name)) == 'r') {
		strcpy(bp, bp+1);
		stat(name, &stat1);
	}

	if (filetype(&stat1) != S_IFBLK) {
		free(name);
		return 0;
	}

	return name;
}

char *
raw_dev(name)
	register char *name;
{
	extern char *basename();

	register char *bp;
	struct stat stat1;

	if ((name = dev_name(name, 1)) == 0)
		return 0;

	if (stat(name, &stat1) < 0) {
		free(name);
		return 0;
	}

	if (filetype(&stat1) == S_IFBLK
	 && *(bp = basename(name)) != '/') {
		bstrcpy(bp+1, bp);
		*bp = 'r';
		stat(name, &stat1);
	}

	if (filetype(&stat1) != S_IFCHR) {
		free(name);
		return 0;
	}

	return name;
}

static char *
dev_name(name, slop)
	register char *name;
	int slop;
{
	extern char *malloc();
	extern char *basename();

	register int l;
	register char *bp, *dp;

	l = strlen(name) + slop+1;

	if ((bp = basename(name)) == name && *bp != '/') {
		l += 5;
		if ((dp = malloc(l)) == 0)
			return 0;
		strcpy(dp, "/dev/");
		strcpy(dp+5, name);
	}
	else {
		if ((dp = malloc(l)) == 0)
			return 0;
		strcpy(dp, name);
	}

	return dp;
}

static
bstrcpy(tgt, src)
	register char *tgt, *src;
{
	register char *osrc;

	osrc = src;
	while (*src++ != 0)
		;
	tgt += src-osrc;
	while (src > osrc)
		*--tgt = *--src;
}
