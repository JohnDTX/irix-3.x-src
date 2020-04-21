/* @(#)getpwinfo.c	1.2 */
#include "uucp.h"
#include <pwd.h>


char *strcpy();

/*
 * get passwd file info for uid
 *	uid	-> uid #	
 *	path	-> address of buffer to return working directory
 *	name	-> address of buffer to return Ascii user name
 * return:
 *	0	-> sucess
 *	FAIL	-> failure
 */
guinfo(uid, name, path)
int uid;
char *path, *name;
{
	register struct passwd *pwd;
	struct passwd *getpwuid();

	if ((pwd = getpwuid(uid)) == NULL) {

		/*
		 * can not find uid in passwd file
		 */
		*path = '\0';
		return(FAIL);
	}

	strcpy(path, pwd->pw_dir);
	strcpy(name, pwd->pw_name);
	return(0);
}

/*
 * get passwd file info for name
 *	name	-> Ascii user name
 *	uid	-> address of integer to return uid # in
 *	path	-> address of buffer to return working directory in
 * returns:
 *	0	-> sucess
 *	FAIL	-> failure
 */
gninfo(name, uid, path)
char *path, *name;
int *uid;
{
	register struct passwd *pwd;
	struct passwd *getpwnam();

	if ((pwd = getpwnam(name)) == NULL) {

		/*
		 * can not find name in passwd file
		 */
		*path = '\0';
		return(FAIL);
	}

	strcpy(path, pwd->pw_dir);
	*uid = pwd->pw_uid;
	return(0);
}


