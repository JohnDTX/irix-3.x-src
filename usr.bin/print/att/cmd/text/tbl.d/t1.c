/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)tbl:t1.c	1.5";
 /* t1.c: main control and input switching */



#
# include "t..c"
#include <signal.h>
# ifdef gcos
/* required by GCOS because file is passed to "tbl" by troff preprocessor */
# define _f1 _f
extern FILE *_f[];
# endif

# define ever (;;)

main(argc,argv)
	char *argv[];
{
# ifdef unix
int badsig();
signal(SIGPIPE, badsig);
# endif
# ifdef gcos
if(!intss()) tabout = fopen("qq", "w"); /* default media code is type 5 */
# endif
exit(tbl(argc,argv));
}


tbl(argc,argv)
	char *argv[];
{
char line[512];
/* required by GCOS because "stdout" is set by troff preprocessor */
tabin=stdin; tabout=stdout;
setinp(argc,argv);
while (gets1(line))
	{
	fprintf(tabout, "%s\n",line);
	if (prefix(".TS", line))
		tableput();
	}
fclose(tabin);
return(0);
}
int sargc;
char **sargv;
setinp(argc,argv)
	char **argv;
{
	extern char *optarg;
	extern int optind;
	int c;
	int errflg;

	sargc = argc;
	sargv = argv;
	while ((c = getopt (sargc, sargv, "T:")) != EOF)
		switch (c) {
		case 'T':
			if (strcmp (optarg, "X") != 0) {
				fprintf (stderr, "%s: Invalid argument to T option--X expected\n", sargv[0]);
				errflg++;
			} else
				pr1403 = 1;
			break;
		case '?':
			errflg++;
		}
	if (errflg) {
		fprintf (stderr, "usage: %s [-T X] [--] [file] ... [-]\n", sargv[0]);
		exit (2);
	}
	if (optind < sargc)
		swapin();
}
swapin()
{
	extern int optind;

	if (optind >= sargc) return(0);
# ifdef unix
/* file closing is done by GCOS troff preprocessor */
	if (tabin!=stdin) fclose(tabin);
# endif
	if (match(sargv [optind], "-"))
		tabin=stdin;
	else
	tabin = fopen(ifile= sargv [optind], "r");
	iline=1;
# ifdef unix
/* file names are all put into f. by the GCOS troff preprocessor */
	fprintf(tabout, ".ds f. %s\n",ifile);
# endif
	if (tabin==NULL)
		error("Can't open file");
	optind++;
	return(1);
}
# ifdef unix
badsig()
{
signal(SIGPIPE, 1);
 exit(0);
}
# endif
