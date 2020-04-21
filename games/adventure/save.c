/* save (III)   J. Gillogly
 * save user core image for restarting
 * usage: save(<command file (argv[0] from main)>,<output file>)
 * bugs
 *   -  impure code (i.e. changes in instructions) is not handled
 *      (but people that do that get what they deserve)
 */

#include <stdio.h>
#include "b.out.h"
# ifndef W2_0
# include <sys/var.h>
# else   W2_0
struct var
{
	int v_doffset;
};
# define uvar(x)	
# endif  W2_0
int filesize;                    /* accessible to caller         */

char *sbrk();

save(cmdfile,outfile)                   /* save core image              */
char *cmdfile,*outfile;
{       register char *c;
	register int i,fd;
	int fdaout;
	struct bhdr bhdr;
	int counter;
	char buff[512],pwbuf[120];
	struct var v;
	fdaout=getcmd(cmdfile);         /* open command wherever it is  */
	if (fdaout<0) return(-1);       /* can do nothing without text  */
	if ((fd=open(outfile,0))>0)     /* this restriction is so that  */
	{       printf("Can't use an existing file\n"); /* we don't try */
		fflush(stdout);
		close(fd);              /* to write over the commnd file*/
		return(-1);
	}
	if ((fd=creat(outfile,0755))== -1)
	{       printf("Cannot create %s\n",outfile);
		fflush(stdout);
		return(-1);
	}
	/* can get the text segment from the command that we were
	 * called with, and change all data from uninitialized to
	 * initialized.  It will start at the top again, so the user
	 * is responsible for checking whether it was restarted
	 * could ignore sbrks and breaks for the first pass
	 */
	uvar(&v);
	read(fdaout,&bhdr,sizeof bhdr);	   /* get the header       */
	bhdr.bsize = 0;                    /* no data uninitialized        */
	bhdr.rtsize = 0;                   /* no data uninitialized        */
	bhdr.rdsize = 0;                   /* no data uninitialized        */
	bhdr.ssize = 0;                    /* throw away symbol table      */
	switch (bhdr.fmagic)               /* find data segment            */
	{   case 0407:                     /* non sharable code            */
		c = (char *) bhdr.tsize;   /* data starts right after text */
		c += bhdr.entry;           /* add physical offset          */
		c += v.v_doffset;	   /* add data offset */
		bhdr.dsize=sbrk(0)-c;      /* current size (incl allocs)   */
		break;
	}

	filesize=sizeof bhdr+bhdr.tsize+bhdr.dsize;
	write(fd,&bhdr,sizeof bhdr);       /* make the new header          */
	if (bhdr.fmagic==0413)
		lseek(fd, 1024L, 0);       /* Start on 1K boundary	   */
	counter=bhdr.tsize;      	   /* size of text                 */
	while (counter>512)                /* copy 512-byte blocks         */
	{       read(fdaout,buff,512);     /* as long as possible          */
		write(fd,buff,512);
		counter -= 512;
	}
	if (counter > 0) {
		read(fdaout,buff,counter); /* then pick up the rest        */
		write(fd,buff,counter);
	}
	write(fd,c,bhdr.dsize);            /* write all data in 1 glob     */
	close(fd);
	return(0);
}

char	*execat(), *getenv();

getcmd(command)         /* get command name (wherever it is) like shell */
char *command;
{
	char *pathstr;
	register char *cp;
	char fname[128];
	int fd;

	if ((pathstr = getenv("PATH")) == NULL)
		pathstr = ":/bin:/usr/bin";
	cp = strchr(command, '/')? "": pathstr;

	do {
		cp = execat(cp, command, fname);
		if ((fd=open(fname,0))>0)
			return(fd);
	} while (cp);

	printf("Couldn't open %s\n",command);
	fflush(stdout);
	return(-1);
}

static char *
execat(s1, s2, si)
register char *s1, *s2;
char *si;
{
	register char *s;

	s = si;
	while (*s1 && *s1 != ':' && *s1 != '-')
		*s++ = *s1++;
	if (si != s)
		*s++ = '/';
	while (*s2)
		*s++ = *s2++;
	*s = '\0';
	return(*s1? ++s1: 0);
}
