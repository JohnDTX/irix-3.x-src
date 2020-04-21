/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	sgi
#ident	"@(#)make:dosys.c	1.3.1.1"
#endif

# include "defs"
# include "sys/types.h"
# include "sys/stat.h"

extern char Makecall;

dosys(comstring, nohalt)
register CHARSTAR comstring;
int nohalt;
{
	register CHARSTAR p;
	register int i;
	int status;

	p = comstring;
	while(	*p == BLANK ||
		*p == TAB) p++;
	if(!*p)
		return(-1);

	if(IS_ON(NOEX) && Makecall == NO)
		return(0);

#ifdef sgi
#define	TRY_SHELL	((signed char) -2)
	if (metas(comstring)
	    || ((status = doexec(comstring)) >> 8) == TRY_SHELL) {
		/*
		 * Either comstring contains metachars or else we couldn't
		 * exec it for some reason.  Try the shell.
		 */
		status = doshell(comstring, nohalt);
	}
#else
	if(metas(comstring))
		status = doshell(comstring,nohalt);
	else
		status = doexec(comstring);
#endif

	return(status);
}



metas(s)   /* Are there are any  Shell meta-characters? */
register CHARSTAR s;
{
	while(*s)
		if( funny[*s++] & META)
			return(YES);

	return(NO);
}

doshell(comstring,nohalt)
register CHARSTAR comstring;
register int nohalt;
{
	register CHARSTAR shell;

	if((waitpid = fork()) == 0)
	{
		enbint(0);
		doclose();

		setenv();
#ifdef	sgi
	    {
		char evalshell[1024];
		char *endshell, *subst();

		shell = varptr("MAKESHELL")->varval;
		if (shell == 0 || shell[0] == CNULL) {
			endshell = &SHELLCOM[sizeof SHELLCOM - 1];
			shell = SHELLCOM;
		} else {
			endshell = subst(shell, evalshell);
			shell = evalshell;
		}
		execl(shell, "sh", 
		    (strcmp(endshell - 3, "csh") == 0) ?
			(nohalt? "-cf" : "-cef") :	/* csh */
			(nohalt? "-c"  : "-ce"),  	/* sh */
		    comstring, 0);
	    }
#else	/* !sgi */
		shell = varptr("SHELL")->varval;
		if(shell == 0 || shell[0] == CNULL)
			shell = SHELLCOM;
		execl(shell, "sh", (nohalt ? "-c" : "-ce"), comstring, 0);
#endif	/* sgi */
		fatal("Couldn't load Shell");
	} else if (waitpid == -1)
		fatal("Couldn't fork");
	return( await() );
}




await()
{
	int intrupt();
	int status;
	int pid;

	enbint(intrupt);
	while( (pid = wait(&status)) != waitpid)
		if(pid == -1)
			fatal("bad wait code");
	waitpid = 0;
	return(status);
}






doclose()	/* Close open directory files before exec'ing */
{
	register OPENDIR od;

	for (od = firstod; od != 0; od = od->nextopendir)
		if (od->dirfc != NULL)
#ifdef	sgi
			closedir(od->dirfc);
#else	/* sgi */
			fclose(od->dirfc);
#endif	/* sgi */
}





doexec(str)
register CHARSTAR str;
{
#ifdef sgi
	/*
	 * These pointers are used to restore the blanks clobbered when
	 * "argv"-izing str, so that our caller can hand str to a shell if
	 * the execv below fails, e.g. because path search is needed.
	 */
	CHARSTAR b = str;	/* beginning of str */
	CHARSTAR e;		/* end of str */
#endif
	register CHARSTAR t;
	register CHARSTAR *p;
#ifdef	sgi
	CHARSTAR argv[2000];
#else	/* sgi */
	CHARSTAR argv[200];
#endif	/* sgi */
	int status;

	while( *str==BLANK || *str==TAB )
		++str;
	if( *str == CNULL )
		return(-1);	/* no command */

	p = argv;
	for(t = str ; *t ; )
	{
		*p++ = t;
		while(*t!=BLANK && *t!=TAB && *t!=CNULL)
			++t;
		if(*t)
			for( *t++ = CNULL ; *t==BLANK || *t==TAB  ; ++t);
	}
#ifdef sgi
	e = t;	/* save the end location */
#endif

	*p = NULL;

	if((waitpid = fork()) == 0)
	{
		enbint(0);
		doclose();
		setenv();
#ifdef sgi
		execv(str, argv);
		exit(TRY_SHELL);
#else
		execvp(str, argv);
		fatal1("Cannot load %s",str);
#endif
	} else if (waitpid == -1)
		fatal("Couldn't fork");
#ifdef sgi
	status = await();
	if ((signed char)(status >> 8) == TRY_SHELL) {
		while (b < e) {
			if (*b == CNULL)
				*b = BLANK;
			b++;
		}
	}
	return status;
#else
	return( await() );
#endif
}

touch(force, name)
register int force;
register char *name;
{
        struct stat stbuff;
        char junk[1];
        int fd;

        if( stat(name,&stbuff) < 0)
                if(force)
                        goto create;
                else
                {
                        fprintf(stderr,"touch: file %s does not exist.\n",name);
                        return;
                }
        if(stbuff.st_size == 0)
                goto create;
        if( (fd = open(name, 2)) < 0)
                goto bad;
        if( read(fd, junk, 1) < 1)
        {
                close(fd);
                goto bad;
        }
        lseek(fd, 0L, 0);
        if( write(fd, junk, 1) < 1 )
        {
                close(fd);
                goto bad;
        }
        close(fd);
        return;
bad:
        fprintf(stderr, "Cannot touch %s\n", name);
        return;
create:
        if( (fd = creat(name, 0666)) < 0)
                goto bad;
        close(fd);
}
