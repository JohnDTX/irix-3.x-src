/* $Source: /d2/3.7/src/bin/RCS/du.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 14:50:22 $ */
char _Origin_[] = "System V";

/*	@(#)du.c	1.3	*/
/*	du	COMPILE:	cc -O du.c -s -i -o du clstat.o	*/
/*				cc -DFsTYPE=2 for 1K file systems

/*
**	du -- summarize disk usage
**		du [-aLrs] [name ...]
*/

#include	<sys/param.h>
# ifdef EFS
# include	<sys/fs.h>
# include	<sys/inode.h>
# endif EFS
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<dirent.h>
#include	<stdio.h>

# ifdef S_IFLNK
# define c_stat(f,p)	(!Lflag?clstat(f,p):stat(f,p))
# else  S_IFLNK
# define c_stat(f,p)	stat(f,p)
# endif S_IFLNK

#define EQ(x,y)	(strcmp(x,y)==0)
#define ML	500
#define DIRECT	10	/* Number of direct blocks */

#ifdef u370
#define	FMT	"%ld	%s\n"
#else
#define	FMT	"%6d %s\n"
#endif

struct 	{
	dev_t	dev;
	long	ino;
} ml[ML];
int	linkc = 0;

struct	stat	Statb;

char	path[1024];

int	aflag = 0;
int	Lflag = 0;
int	rflag = 0;
int	sflag = 0;
long	descend();

main(argc, argv)
char **argv;
{
	register long totblocks = 0, blocks = 0;
	short total = 0;

#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv("du", &argv, 0);
#endif
	if(--argc && argv[1][0] == '-' && argv[1][1] != '\0') {
		argv++;
		while(*++*argv)
			switch(**argv) {
			case 'L':
				Lflag++;
				continue;

			case 'a':
				aflag++;
				continue;

			case 'r':
				rflag++;
				continue;

			case 's':
				sflag++;
				continue;

			default:
				fprintf(stderr, "usage: du [-aLrs] [name ...]\n");
				exit(2);
			}
		argc--;
	}
	if(argc == 0) {
		argc = 1;
		argv[1] = ".";
	}
	total = argc > 1;
	while(argc--) {
		strcpy(path, *++argv);
		blocks = descend(path);
		totblocks += blocks;
		if(sflag)
			printf(FMT, blocks, path);
	}
	if(total)
		printf(FMT, totblocks, "Total");

	exit(0);
}

long descend(name)
char *name;
{
	register DIR	*dirf;		/* open directory */
	register struct	dirent	*dp;	/* current dirent */
	register char	*c1, *c2;
	long blocks = 0;
	long	offset;
	int	i;
	char	*endofname;
	long nblock();

	if(c_stat(name,&Statb)<0) {
		if(rflag)
			fprintf(stderr, "du: bad status < %s >\n", name);
		return(0);
	}
	if(Statb.st_nlink>1 && (Statb.st_mode&S_IFMT)!=S_IFDIR && linkc<ML) {
		for(i = 0; i <= linkc; ++i) {
			if(ml[i].ino==Statb.st_ino && ml[i].dev==Statb.st_dev)
				return 0;
		}
		ml[linkc].dev = Statb.st_dev;
		ml[linkc].ino = Statb.st_ino;
		++linkc;
	}
	blocks = nblock(Statb.st_dev, Statb.st_size);

	if((Statb.st_mode&S_IFMT)!=S_IFDIR) {
		if(aflag)
			printf(FMT, blocks, name);
		return(blocks);
	}

	for(c1 = name; *c1; ++c1);
	endofname = c1;
	if(Statb.st_size > 32000)
		fprintf(stderr, "Huge directory < %s >--call administrator\n", name);
	
	if((dirf = opendir(name)) == NULL) {
		if(rflag)
			fprintf(stderr, "du: cannot open < %s >\n", name);
		return(0);
	}
	offset = 0;

	/*
	|| Foreach entry in the current directory ...
	*/
	while((dp = readdir(dirf)) != NULL) {
		
		if(dp->d_ino==0
		   || EQ(dp->d_name, ".")
		   || EQ(dp->d_name, ".."))
			continue;
		if (dp->d_ino == -1)
			continue;
		c1 = endofname;
		if (c1[-1] != '/')
			*c1++ = '/';
		c2 = dp->d_name;
		for(i=0; i<dp->d_reclen; i++)
			if(*c2)
				*c1++ = *c2++;
			else
				break;
		*c1 = '\0';
		if(i == 0) { /* null length name */
			fprintf(stderr,"bad dir entry <%s>\n",dp->d_name);
			return(0);
		}
		/*
		|| Recursively call 'descend' for this entry.  If the
		|| recursion is getting too deep, close the directory
		|| and reopen it upon return to prevent running out
		|| of file descriptors.
		*/
		if(dirf->dd_fd > 10) {
			offset = telldir(dirf);	/* remember current position */
			closedir(dirf);
			dirf = NULL;
		}
		blocks += descend(name);
		if(dirf == NULL) {
			if((dirf = opendir(name)) == NULL) {
				if(rflag)
					fprintf(stderr, "du: cannot open < %s >\n", name);
				return(0);
			}
			if(offset) {
				seekdir(dirf, (long)offset);
				offset = 0;
			}
		}
	}
	if(dirf)
		closedir(dirf);
	*endofname = '\0';
	if(!sflag)
		printf(FMT, blocks, name);
	return(blocks);
}

long
nblock(dev, size)
	dev_t dev;
	off_t size;
{
# define ONE_K		(1<<ONE_K_SHIFT)
# define ONE_K_SHIFT	10
	return (size + ONE_K-1) >> ONE_K_SHIFT;
}
