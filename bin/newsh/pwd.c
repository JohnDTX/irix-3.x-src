/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)sh:pwd.c	1.14"
/* 
*	UNIX shell
 */

#include		"mac.h"
#include	<sys/types.h>
#include	<dirent.h>
#include	<sys/stat.h>

#define	DOT		'.'
#define	NULL	0
#define	SLASH	'/'
#define MAXPWD	512

extern char	longpwd[];

static char cwdname[MAXPWD];
static int 	didpwd = FALSE;

cwd(dir)
	register char *dir;
{
	register char *pcwd;
	register char *pdir;

	/* First remove extra /'s */

	rmslash(dir);

	/* Now remove any .'s */

	pdir = dir;
	if(*dir == SLASH)
		pdir++;
	while(*pdir) 			/* remove /./ by itself */
	{
		if((*pdir==DOT) && (*(pdir+1)==SLASH))
		{
			movstr(pdir+2, pdir);
			continue;
		}
		pdir++;
		while ((*pdir) && (*pdir != SLASH)) 
			pdir++;
		if (*pdir) 
			pdir++;
	}
	/* take care of trailing /. */
	if(*(--pdir)==DOT && pdir > dir && *(--pdir)==SLASH) {
		if(pdir > dir) {
			*pdir = NULL;
		} else {
			*(pdir+1) = NULL;
		}
	
	}
	
	/* Remove extra /'s */

	rmslash(dir);

	/* Now that the dir is canonicalized, process it */

	if(*dir==DOT && *(dir+1)==NULL)
	{
		return;
	}

	if(*dir==SLASH)
	{
		/* Absolute path */

		pcwd = cwdname;
	 	if (pcwd >= &cwdname[MAXPWD])
		{
			didpwd=FALSE;
			return;
		}
		*pcwd++ = *dir++;
		didpwd = TRUE;
	}
	else
	{
		/* Relative path */

		if (didpwd == FALSE) 
			return;
			
		pcwd = cwdname + length(cwdname) - 1;
		if(pcwd != cwdname+1)
		{
			*pcwd++ = SLASH;
		}
	}
	while(*dir)
	{
		if(*dir==DOT && 
		   *(dir+1)==DOT &&
		   (*(dir+2)==SLASH || *(dir+2)==NULL))
		{
			/* Parent directory, so backup one */

			if( pcwd > cwdname+2 )
				--pcwd;
			while(*(--pcwd) != SLASH)
				;
			pcwd++;
			dir += 2;
			if(*dir==SLASH)
			{
				dir++;
			}
			continue;
		}
	 	if (pcwd >= &cwdname[MAXPWD])
		{
			didpwd=FALSE;
			return;
		}
		*pcwd++ = *dir++;
		while((*dir) && (*dir != SLASH))
		{  
	 		if (pcwd >= &cwdname[MAXPWD])
			{
				didpwd=FALSE;
				return;
			}
			*pcwd++ = *dir++;
		} 
		if (*dir) 
		{
	 		if (pcwd >= &cwdname[MAXPWD])
			{
			didpwd=FALSE;
			return;
			}
			*pcwd++ = *dir++;
		}
	}
	if (pcwd >= &cwdname[MAXPWD])
	{
		didpwd=FALSE;
		return;
	}
	*pcwd = NULL;

	--pcwd;
	if(pcwd>cwdname && *pcwd==SLASH)
	{
		/* Remove trailing / */

		*pcwd = NULL;
	}
	return;
}

/*
 *	Print the current working directory.
 */

cwdprint()
{
	if (didpwd == FALSE)
		pwd();

	prs_buff(cwdname);
	prc_buff(NL);
	return;
}

/*
 *	This routine will remove repeated slashes from string.
 */

static
rmslash(string)
	char *string;
{
	register char *pstring;

	pstring = string;
	while(*pstring)
	{
		if(*pstring==SLASH && *(pstring+1)==SLASH)
		{
			/* Remove repeated SLASH's */

			movstr(pstring+1, pstring);
			continue;
		}
		pstring++;
	}

	--pstring;
	if(pstring>string && *pstring==SLASH)
	{
		/* Remove trailing / */

		*pstring = NULL;
	}
	return;
}

/*
 *	Find the current directory the hard way.
 */



static char dotdots[] =
"../../../../../../../../../../../../../../../../../../../../../../../..";

extern char		*movstrn();

static
pwd()
{
	struct stat		cdir;	/* current directory status */
	struct stat		tdir;
	struct stat		pdir;	/* parent directory status */
	DIR			*pdfd;	/* parent directory stream */

	struct dirent	*dir;
	char 			*dot = dotdots + sizeof(dotdots) - 3;
	int				index = sizeof(dotdots) - 2;
	int				cwdindex = MAXPWD - 1;
	int 			i;
	
	cwdname[cwdindex] = 0;
	dotdots[index] = 0;

	if(stat(dot, &pdir) < 0)
	{
		error("pwd: cannot stat .");
	}

	dotdots[index] = '.';

	for(;;)
	{
		cdir = pdir;

		if ((pdfd = opendir(dot)) == 0)
		{
			error("pwd: cannot open ..");
		}

		if(fstat(pdfd->dd_fd, &pdir) < 0)
		{
			(void)closedir(pdfd);
			error("pwd: cannot stat ..");
		}

		if(cdir.st_dev == pdir.st_dev)
		{
			if(cdir.st_ino == pdir.st_ino)
			{
				didpwd = TRUE;
				(void)closedir(pdfd);
				if (cwdindex == (MAXPWD - 1))
					cwdname[--cwdindex] = SLASH;

				movstr(&cwdname[cwdindex], cwdname);
				return;
			}

			do
			{
				if ((dir = readdir(pdfd)) == 0)
				{
					(void)closedir(pdfd);
					error("pwd: read error in ..");
				}
			}
			while (dir->d_ino != cdir.st_ino);
		{
			char name[MAXPWD];
			
			movstr(dot, name);
			i = length(name) - 1;

			name[i++] = '/';

			do
			{
				if ((dir = readdir(pdfd)) == 0)
				{
					(void)closedir(pdfd);
					error("pwd: read error in ..");
				}
				movstr(dir->d_name, &name[i]);
				stat(name, &tdir);
			}		
			while(tdir.st_ino != cdir.st_ino || tdir.st_dev != cdir.st_dev);
		}
		(void)closedir(pdfd);

		for (i = 0;; i++)
			if (dir->d_name[i] == 0)
				break;

		if (i > cwdindex - 1)
				error(longpwd);
		else
		{
			cwdindex -= i;
			movstrn(dir->d_name, &cwdname[cwdindex], i);
			cwdname[--cwdindex] = SLASH;
		}

		dot -= 3;
		if (dot<dotdots) 
			error(longpwd);
	}
}
