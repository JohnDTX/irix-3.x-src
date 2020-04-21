char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";

# include	"../hdr/defines.h"
#ifdef TRACE
#define tr(x,y,w,z) fprintf(stderr,"%s %s %s %s\n",x,y,w,z);
#else	
#define tr(x,y,w,z) /* x y w z */
#endif
SCCSID(@(#)help.c	5.7);

/*
	Program to locate helpful info in an ascii file.
	The program accepts a variable number of arguments.

	The file to be searched is determined from the argument. If the
	argument does not contain numerics, the search 
	will be attempted on '/usr/lib/help/cmds', with the search key
	being the whole argument.
	If the argument begins with non-numerics but contains
	numerics (e.g, zz32) the file /usr/lib/help/helploc 
	will be checked for a file corresponding to the non numeric prefix,
	That file will then be seached for the mesage. If /usr/lib/help/helploc
	does not exist or the prefix is not found there the search will
	be attempted on '/usr/lib/help/<non-numeric prefix>', 
	(e.g,/usr/lib/help/zz), with the search key being <remainder of arg>, 
	(e.g., 32).
	If the argument is all numeric, or if the file as
	determined above does not exist, the search will be attempted on
	'/usr/lib/help/default' with the search key being
	the entire argument.
	In no case will more than one search per argument be performed.

	File is formatted as follows:

		* comment
		* comment
		-str1
		text
		-str2
		text
		* comment
		text
		-str3
		text

	The "str?" that matches the key is found and
	the following text lines are printed.
	Comments are ignored.

	If the argument is omitted, the program requests it.
*/
#define HELPLOC "/usr/lib/help/helploc"
struct stat Statbuf;
char Error[128];

char	dftfile[]   =   "/usr/lib/help/default";
char	helpdir[]   =   "/usr/lib/help/";
char	hfile[64];
char	*repl();
FILE	*iop, *fdfopen();
char	line [512];


main(argc,argv)
int argc;
char *argv[];
{
	register int i;
	extern int Fcnt;
	char *ask();

	/*
	Tell 'fatal' to issue messages, clean up, and return to its caller.
	*/
	Fflags = FTLMSG | FTLCLN | FTLJMP;

	if (argc == 1)
		findprt(ask());		/* ask user for argument */
	else
		for (i = 1; i < argc; i++)
			findprt(argv[i]);

	exit(Fcnt ? 1 : 0);
}


findprt(p)
char *p;
{
	register char *q;
	char key[150];
	char *strcpy();
	if (setjmp(Fjmp))		/* set up to return here from */
		return;			/* 'fatal' and return to 'main' */
	if (size(p) > 50)
		fatal("argument too long (he2)");

	q = p;

	while (*q && !numeric(*q))
		q++;

	if (*q == '\0') {		/* all alphabetics */
		strcpy(key,p);
		sprintf(hfile,"%s%s",helpdir,"cmds");
		if (!exists(hfile))
			strcpy(hfile,dftfile);
	}
	else
		if (q == p) {		/* first char numeric */
			strcpy(key,p);
			strcpy(hfile,dftfile);
		}
	else {				/* first char alpha, then numeric */
		strcpy(key,p);		/* key used as temporary */
		*(key + (q - p)) = '\0';
		if(!lochelp(key,hfile))
			sprintf(hfile,"%s%s",helpdir,key);
		else
			cat(hfile,hfile,"/",key,0);
		tr(hfile,helpdir,key,NULL);
		strcpy(key,q);
		if (!exists(hfile)) {
			strcpy(key,p);
			strcpy(hfile,dftfile);
		}
	}

	iop = xfopen(hfile,0);

	/*
	Now read file, looking for key.
	*/
	while ((q = fgets(line,512,iop)) != NULL) {
		repl(line,'\n','\0');		/* replace newline char */
		if (line[0] == '-' && equal(&line[1],key))
			break;
	}

	if (q == NULL) {	/* endfile? */
		printf("\n");
		sprintf(Error,"%s not found (he1)",p);
		fatal(Error);
	}

	printf("\n%s:\n",p);

	while (fgets(line,512,iop) != NULL && line[0] == '-')
		;
	do {
		if (line[0] != '*')
			printf("%s",line);
	} while (fgets(line,512,iop) != NULL && line[0] != '-');

	fclose(iop);
}


char *
ask()
{
	static char resp[51];

	iop = stdin;

	printf("msg number or comd name? ");
	fgets(resp,51,iop);
	return(repl(resp,'\n','\0'));
}

/* lochelp finds the file which cojntains the help messages 
if none found returns 0
*/
lochelp(ky,fi)
	char *ky,*fi; /*ky is key  fi is found file name */
{
	FILE *fp;
	char locfile[513];
	char *hold;
	if(!(fp = fopen(HELPLOC,"r")))
	{
		/*no lochelp file*/
		return(0); 
	}
	while(fgets(locfile,512,fp)!=NULL)
	{
		hold=(char *)strtok(locfile,"\t ");
		if(!(strcmp(ky,hold)))
		{
			hold=(char *)strtok(0,"\n");
			strcpy(fi,hold); /* copy file name to fi */
			return(1); /* entry found */
		}
	}
	return(0); /* no entry found */
}


clean_up()
{
	fclose(iop);
}
