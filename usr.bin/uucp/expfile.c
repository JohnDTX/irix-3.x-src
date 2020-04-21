/* @(#)expfile.c	1.3 */
#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>

/*
 * expand file name expansion is based on first characters
 *	/	-> fully qualified pathname. no
 *		   processing necessary
 *	~	-> prepended with login directory
 *	~/	-> prepended with PUBDIR
 *	default	-> prepended with current directory
 *	file	-> filename to expand
 * returns: 
 *	0	-> ok
 *      FAIL	-> no Wrkdir name available
 */
expfile(file)
register char *file;
{
	register char *fpart;
	register char *up;
	int uid;
	char *strcpy(), *strcat();
	char user[20];
	char full[100];

	switch(file[0]) {
#ifdef FOWARD
	case '!':
		return(0);
#endif
	case '/':
		return(0);
	case '~':
		for (fpart = file + 1, up = user; *fpart != '\0'
			&& *fpart != '/'; fpart++)
				*up++ = *fpart;
		*up = '\0';
		if (gninfo(user, &uid, full) != 0) {
			strcpy(full, PUBDIR);
		}
	
		strcat(full, fpart);
		strcpy(file, full);
		return(0);
	default:
		strcpy(full, Wrkdir);
		strcat(full, "/");
		strcat(full, file);
		strcpy(file, full);
		if (Wrkdir[0] == '\0')
			return(FAIL);
		else
			return(0);
	}
}

/*
 * check if directory name
 *	name	-> file to check
 * return:
 *	FALSE	-> not directory
 *	TRUE	-> is direcctory
 */
isdir(name)
char *name;
{
	register int ret;
	struct stat s;

	ret = stat(name, &s);
	if (ret < 0)
		return(FALSE);
	if ((s.st_mode & S_IFMT) == S_IFDIR)
		return(TRUE);
	return(FALSE);
}

/*
 * make all necessary directories
 *	name	-> directory to make
 * return: 
 *	0	-> success
 * 	FAIL	-> failure
 */
mkdirs(name)
register char *name;
{
	register int ret;
	register char *p;
	int mask;
	char cmd[100], dir[100];
	char *strchr();

	for (p = dir + 1;; p++) {
		strcpy(dir, name);
		if ((p = strchr(p, '/')) == NULL)
			return(0);
		*p = '\0';
		if (isdir(dir))
			continue;
		sprintf(cmd, "mkdir %s", dir);
		DEBUG(4, "mkdir - %s\n", dir);
		mask = umask(0);
		ret = shio(cmd, CNULL, CNULL, User);
		umask(mask);
		if (ret != 0)
			return(FAIL);
	}
}

/*
 * expand file name and check return
 * print error if it failed.
 *	file	-> file name to check
 * returns: 
 *      0	-> ok
 *      FAIL	-> if expfile failed
 */
ckexpf(file)
char *file;
{

	if (expfile(file) == 0)
		return(0);

	/*
	 * could not expand file name
	 * the gwd routine failed
	 */
	fprintf(stderr, "Can't expand filename (%s). Pwd failed.\n", file+1);
	return(FAIL);
}
