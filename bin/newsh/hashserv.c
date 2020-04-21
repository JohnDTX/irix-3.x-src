/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sh:hashserv.c	1.10"
/*
 *	UNIX shell
 */

#include	"hash.h"
#include	"defs.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>

#define		EXECUTE		01

static char	cost;
static int	dotpath;
static int	multrel;
static struct entry	relcmd;

int		argpath();

short
pathlook(com, flg, arg)
	char	*com;
	int		flg;
	register struct argnod	*arg;
{
	register char	*name = com;
	register ENTRY	*h;

	ENTRY		hentry;
	int		count = 0;
	int		i;
	int		pathset = 0;
	int		oldpath = 0;
	struct namnod	*n;


	hentry.data = 0;

	if (any('/', name))
		return(COMMAND);

	h = hfind(name);

	if (h)
	{
		if (h->data & (BUILTIN | FUNCTION))
		{
			if (flg)
				h->hits++;
			return(h->data);
		}

		if (arg && (pathset = argpath(arg)))
			return(PATH_COMMAND);

		if ((h->data & DOT_COMMAND) == DOT_COMMAND)
		{
			if (multrel == 0 && hashdata(h->data) > dotpath)
				oldpath = hashdata(h->data);
			else
				oldpath = dotpath;

			h->data = 0;
			goto pathsrch;
		}

		if (h->data & (COMMAND | REL_COMMAND))
		{
			if (flg)
				h->hits++;
			return(h->data);
		}

		h->data = 0;
		h->cost = 0;
	}

	if (i = syslook(name, commands, no_commands))
	{
		hentry.data = (BUILTIN | i);
		count = 1;
	}
	else
	{
		if (arg && (pathset = argpath(arg)))
			return(PATH_COMMAND);
pathsrch:
			count = findpath(name, oldpath);
	}

	if (count > 0)
	{
		if (h == 0)
		{
			hentry.cost = 0;
			hentry.key = make(name);
			h = henter(hentry);
		}

		if (h->data == 0)
		{
			if (count < dotpath)
				h->data = COMMAND | count;
			else
			{
				h->data = REL_COMMAND | count;
				h->next = relcmd.next;
				relcmd.next = h;
			}
		}


		h->hits = flg;
		h->cost += cost;
		return(h->data);
	}
	else 
	{
		return(-count);
	}
}
			

static void
zapentry(h)
	ENTRY *h;
{
	h->data &= HASHZAP;
}

void
zaphash()
{
	hscan(zapentry);
	relcmd.next = 0;
}

void 
zapcd()
{
	ENTRY *ptr = relcmd.next;
	
	while (ptr)
	{
		ptr->data |= CDMARK;
		ptr = ptr->next;
	}
	relcmd.next = 0;
}


static void
hashout(h)
	ENTRY *h;
{
	sigchk();

	if (hashtype(h->data) == NOTFOUND)
		return;

	if (h->data & (BUILTIN | FUNCTION))
		return;

	prn_buff(h->hits);

	if (h->data & REL_COMMAND)
		prc_buff('*');


	prc_buff(TAB);
	prn_buff(h->cost);
	prc_buff(TAB);

	pr_path(h->key, hashdata(h->data));
	prc_buff(NL);
}

void
hashpr()
{
	prs_buff("hits	cost	command\n");
	hscan(hashout);
}


set_dotpath()
{
	register char	*path;
	register int	cnt = 1;

	dotpath = 10000;
	path = getpath("");

	while (path && *path)
	{
		if (*path == '/')
			cnt++;
		else
		{
			if (dotpath == 10000)
				dotpath = cnt;
			else
			{
				multrel = 1;
				return;
			}
		}
	
		path = nextpath(path);
	}

	multrel = 0;
}


hash_func(name)
	char *name;
{
	ENTRY	*h;
	ENTRY	hentry;

	h = hfind(name);

	if (h)
	{

		if (h->data & (BUILTIN | FUNCTION))
			return;
		else
			h->data = FUNCTION;
	}
	else
	{
		int i;

		if (i = syslook(name, commands, no_commands))
			hentry.data = (BUILTIN | i);
		else
			hentry.data = FUNCTION;

		hentry.key = make(name);
		hentry.cost = 0;
		hentry.hits = 0;
	
		henter(hentry);
	}
}

func_unhash(name)
	char *name;
{
	ENTRY 	*h;

	h = hfind(name);

	if (h && (h->data & FUNCTION))
		h->data = NOTFOUND;
}


short
hash_cmd(name)
	char *name;
{
	ENTRY	*h;

	if (any('/', name))
		return(COMMAND);

	h = hfind(name);

	if (h)
	{
		if (h->data & (BUILTIN | FUNCTION))
			return(h->data);
		else if ((h->data & REL_COMMAND) == REL_COMMAND)
		{ /* unlink h from relative command list */
			ENTRY *ptr = &relcmd;
			while(ptr-> next != h)
				ptr = ptr->next;
			ptr->next = h->next;
		}
		zapentry(h);
	}

	return(pathlook(name, 0, 0));
}


what_is_path(name)
	register char *name;
{
	register ENTRY	*h;
	int		cnt;
	short	hashval;

	h = hfind(name);

	prs_buff(name);
	if (h)
	{
		hashval = hashdata(h->data);

		switch (hashtype(h->data))
		{
			case BUILTIN:
				prs_buff(" is a shell builtin\n");
				return;
	
			case FUNCTION:
			{
				struct namnod *n = lookup(name);

				prs_buff(" is a function\n");
				prs_buff(name);
				prs_buff("(){\n");
				prf(n->namenv);
				prs_buff("\n}\n");
				return;
			}
	
			case REL_COMMAND:
			{
				short hash;

				if ((h->data & DOT_COMMAND) == DOT_COMMAND)
				{
					hash = pathlook(name, 0, 0);
					if (hashtype(hash) == NOTFOUND)
					{
						prs_buff(" not found\n");
						return;
					}
					else
						hashval = hashdata(hash);
				}
			}

			case COMMAND:					
				prs_buff(" is hashed (");
				pr_path(name, hashval);
				prs_buff(")\n");
				return;
		}
	}

	if (syslook(name, commands, no_commands))
	{
		prs_buff(" is a shell builtin\n");
		return;
	}

	if ((cnt = findpath(name, 0)) > 0)
	{
		prs_buff(" is ");
		pr_path(name, cnt);
		prc_buff(NL);
	}
	else
		prs_buff(" not found\n");
}


findpath(name, oldpath)
	register char *name;
	int oldpath;
{
	register char 	*path;
	register int	count = 1;

	char	*p;
	int	ok = 1;
	int 	e_code = 1;
	
	cost = 0;
	path = getpath(name);

	if (oldpath)
	{
		count = dotpath;
		while (--count)
			path = nextpath(path);

		if (oldpath > dotpath)
		{
			catpath(path, name);
			p = curstak();
			cost = 1;

			if ((ok = chk_access(p, S_IEXEC, 1)) == 0)
				return(dotpath);
			else
				return(oldpath);
		}
		else 
			count = dotpath;
	}

	while (path)
	{
		path = catpath(path, name);
		cost++;
		p = curstak();

		if ((ok = chk_access(p, S_IEXEC, 1)) == 0)
			break;
		else
			e_code = max(e_code, ok);

		count++;
	}

	return(ok ? -e_code : count);
}

/*
 * Determine if file given by name is accessible with permissions
 * given by mode.
 * Regflag argument non-zero means not to consider 
 * a non-regular file as executable. 
 * Partial check for write access is done via access system call
 * because cannot read inodes in user mode unless root.
 * Unfortunately, access(2) checks with respect to real id's
 * instead of effective id's.  So rest of check for access permission
 * is done via stat(2).
 */

chk_access(name, mode, regflag)
register char	*name;
int mode, regflag;
{	
	static int flag;
	static int euid; 
	static int egid;
	struct stat statb;
	int ftype;
	if(mode == S_IWRITE) {
		if(access(name, 2) == -1 && errno != EACCES) {
			return(1);
		}
	}
	if(flag == 0) {
		euid = geteuid();
		egid = getegid();
		flag = 1;
	}
	ftype = statb.st_mode & S_IFMT;
	if (stat(name, &statb) == 0) {
		ftype = statb.st_mode & S_IFMT;
		if(mode == S_IEXEC && regflag && ftype != S_IFREG)
			return(2);
		if(euid == 0) {
			if (ftype != S_IFREG || mode != S_IEXEC)
				return(0);
		    	/* root can execute file as long as it has execute 
			   permission for someone */
			if (statb.st_mode & (S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6)))
				return(0);
			return(3);
		}
		if(euid != statb.st_uid) {
			mode >> = 3;
			if(egid != statb.st_gid)
				mode >> = 3;
		}
		if(statb.st_mode & mode)
			return(0);
		return(3);
	}
	return(errno == EACCES ? 3 : 1);
}


pr_path(name, count)
	register char	*name;
	int count;
{
	register char	*path;

	path = getpath(name);

	while (--count && path)
		path = nextpath(path, name);

	catpath(path, name);
	prs_buff(curstak());
}


static
argpath(arg)
	register struct argnod	*arg;
{
	register char 	*s;
	register char	*start;

	while (arg)
	{
		s = arg->argval;
		start = s;

		if (letter(*s))		
		{
			while (alphanum(*s))
				s++;

			if (*s == '=')
			{
				*s = 0;

				if (eq(start, pathname))
				{
					*s = '=';
					return(1);
				}
				else
					*s = '=';
			}
		}
		arg = arg->argnxt;
	}

	return(0);
}
