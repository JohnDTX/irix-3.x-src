/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sh:expand.c	1.18"
/*
 *	UNIX shell
 *
 */

#include	"defs.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>



/*
 * globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 */
extern int	addg();


expand(as, rcnt)
	char	*as;
{
	int	count; 
	DIR	*dirf;
	BOOL	dir = 0;
	char	*rescan = 0;
	char 	*slashsav = 0;
	register char	*s, *cs;
	int quotflag = 0;
	int quotflag2 = 0;
	char *s2 = 0;
	struct argnod	*schain = gchain;
	struct stat statb;
	BOOL	slash;

	if (trapnote & SIGSET)
		return(0);
	s = cs = as;
	/*
	 * check for meta chars
	 */
	{
		register BOOL open;

		slash = 0;
		open = 0;
		do
		{
			switch (*cs++)
			{
			case 0:
				if (rcnt && slash)
					break;
				else
					return(0);

			case '/':
				slash++;
				open = 0;
				continue;

			case '[':
				open++;
				continue;

			case ']':
				if (open == 0)
					continue;

			case '?':
			case '*':
				if (rcnt > slash)
					continue;
				else
					cs--;
				break;


			case '\\':
				cs++;
			default:
				continue;
			}
			break;
		} while (TRUE);
	}

	for (;;)
	{
		if (cs == s)
		{
			s = nullstr;
			break;
		}
		else if (*--cs == '/')
		{
			cs--;
			if(cs >= s && *cs == '\\') {
				*cs = 0; /* null out backslash before / */
				slash++; /* increment slash count because
					    slash will be unquoted in expansion
					    string */
				quotflag = 1;
			} else
				*++cs = 0;
			if (s == cs)
				s = "/";
			else {
				s2 = cpystak(s);
				trim(s2);
				s = s2;
			}
			break;
		}
	}

	if ((dirf = opendir(*s ? s : ".")) != 0)
		dir++;

	/* Let s point to original string because it will be trimmed later */
	if(s2)
		s = as;
	count = 0;
	if (*cs == 0) {
		slashsav = cs++; /* remember where first slash in as is */
		if(quotflag)
			cs++; /* advance past / */
	}
	if (dir)		/* check for rescan */
	{
		register char *rs;
		struct dirent *e;

		rs = cs;
		do /* find next / in as */
		{
			if (*rs == '/')
			{
				if(*--rs != '\\') {
					rescan = ++rs;
					*rs = 0;
				} else {
					quotflag2 = 1;
					*rs = 0;
					rescan = rs + 1; /* advance past / */
				}
				gchain = 0;
			}
		} while (*rs++);

		while ((e = readdir(dirf)) && (trapnote & SIGSET) == 0)
		{
			if (e->d_name[0] == '.' && *cs != '.')
				continue;

			if (gmatch(e->d_name, cs))
			{
				addg(s, e->d_name, rescan, slashsav);
				count++;
			}
		}
		(void)closedir(dirf);

		if (rescan)
		{
			register struct argnod	*rchain;

			rchain = gchain;
			gchain = schain;
			if (count)
			{
				count = 0;
				while (rchain)
				{
					count += expand(rchain->argval, slash + 1);
					rchain = rchain->argnxt;
				}
			}
			if(quotflag2)
				*--rescan = '\\';
			else
				*rescan = '/';
		}
	}

	if(slashsav) {
		if(quotflag)
			*slashsav = '\\';
		else
			*slashsav = '/';
	}
	return(count);
}

/*
	opendir -- C library extension routine

*/

extern int	open(), close(), fstat();

#define NULL	0

DIR *
opendir(filename)
char		*filename;	/* name of directory */
{
	static DIR direct;
	static char dirbuf[DIRBUF];
	register DIR	*dirp = &direct;		/* static storage */
	register int	fd;		/* file descriptor for read */
	struct stat	sbuf;		/* result of fstat() */

	if (stat(filename, &sbuf) < 0
	  || (sbuf.st_mode & S_IFMT) != S_IFDIR 
	  || (fd = open(filename, 0)) < 0 )	
		return NULL;		

	dirp->dd_fd = fd;
	dirp->dd_loc = dirp->dd_size = 0;	/* refill needed */
	dirp->dd_buf = dirbuf;
	return dirp;
}

/*
	closedir -- C library extension routine

*/


extern int	close();

closedir(dirp) 
register DIR	*dirp;		/* stream from opendir() */
{
	return close(dirp->dd_fd);
}

gmatch(s, p)
register unsigned char	*s, *p;
{
	register unsigned char scc;
	unsigned char c;

	scc = *s++;
	switch (c = *p++)
	{
	case '[':
		{
			BOOL ok;
			int lc = -1;
			int notflag = 0;

			ok = 0;
			if (*p == '!')
			{
				notflag = 1;
				p++;
			}
			while (c = *p++)
			{
				if (c == ']')
					return(ok ? gmatch(s, p) : 0);
				else if (c == MINUS && lc > 0 && *p!= ']')
				{
					if (notflag)
					{
						if (scc < lc || scc > *(p++))
							ok++;
						else
							return(0);
					}
					else
					{
						if (lc <= scc && scc <= (*p++))
							ok++;
					}
				}
				else
				{
					if(c == '\\') /* skip to quoted character */
						c = *p++;
					lc = c;
					if (notflag)
					{
						if (scc && scc != lc)
							ok++;
						else
							return(0);
					}
					else
					{
						if (scc == lc)
							ok++;
					}
				}
			}
			return(0);
		}

	case '\\':	
		c = *p++; /* skip to quoted character and see if it matches */
	default:
		if (c != scc)
			return(0);

	case '?':
		return(scc ? gmatch(s, p) : 0);

	case '*':
		while (*p == '*')
			p++;

		if (*p == 0)
			return(1);
		--s;
		while (*s)
		{
			if (gmatch(s++, p))
				return(1);
		}
		return(0);

	case 0:
		return(scc == 0);
	}
}

static int
addg(as1, as2, as3, as4)
char	*as1, *as2, *as3, *as4;
{
	register char	*s1, *s2;

	s2 = locstak() + BYTESPERWORD;
	s1 = as1;
	if(as4) {
		while (*s2 = *s1++)
			s2++; 
	/* Restore first slash before the first metacharacter if as1 is not "/" */
		if(as4 + 1 == s1)
			*s2++ = '/';
	}
/* add matched entries, plus extra \\ to escape \\'s */
	s1 = as2;
	while (*s2 = *s1++) {
		if(*s2 == '\\')
			*++s2 = '\\';
		s2++;
	}
	if (s1 = as3)
	{
		*s2++ = '/';
		while (*s2++ = *++s1);
	}
	makearg(endstak(s2));
}

makearg(args)
	register struct argnod *args;
{
	args->argnxt = gchain;
	gchain = args;
}


