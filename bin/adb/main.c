char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "UniSoft Systems";

#include <fperr.h>
#include "defs.h"
/****************************************************************************

 DEBUGGER - main entry loop 

****************************************************************************/

#include <setjmp.h>

MSG NOEOR;
int mkfault;
int executing;
FILE *infile = stdin;
FILE *outfile = stdout;
char *lp;
int maxoff = 32768;
int maxpos = 80;
int (*sigint)();
int (*sigqit)();
int wtflag;
#ifdef	sgi
/*
 * kflag is non-zero if user wants us to adb a kernel.  All it does is set
 * up the map to known values
 */
int kflag;
long int maxfile = 0x7fffffff;
#else
long int maxfile = 1L<<24;
#endif
long int txtsiz;
long int datsiz;
long int datbas;
long int stksiz;
char * errflg;
int exitflg;
int magic;
long int entrypt;
char lastc;
int eof;
jmp_buf env;
int lastcom;
long int var[36];
char * symfil;
char * corfil;
char printbuf[];
char *printptr;
char outbuf[512];
int argcount;

long
round(a, b)
long a, b;
{
	return(((a + b - 1)/b) * b);
}

chkerr()
{
	if (errflg || mkfault) error(errflg);
}

error(n)
char *n;
{
	errflg = n;
	iclose();
	oclose();
	longjmp(env);
}

fault(sig)
{
	signal(sig, fault);
	fseek(infile, 0L, 2);
	error("\nadb");
}

main(argc, argv)
int argc;
char **argv;
{
	ioctl(0, TCGETA, &adbtty);
	ioctl(0, TCGETA, &subtty);
	fpsigset(0, CONTINUE_AFTER_FPERROR);
	/*
 ioctl(0, TIOCGETC, &adbtch);
 ioctl(0, TIOCGETC, &subtch);
 */

#ifdef	sgi
	while (argc > 1)
	{
		register char *ap;

		ap = argv[1];
		if (*ap != '-')
			break;
		while (*++ap)
			switch (*ap)
			{
			case 'w':
				wtflag = 2;
				break;
			case 'k':
				kflag = 1;
				break;
			default:
				goto loopend;
			}
		argc--;
		argv++;
	}
loopend:
#else
	while (argc > 1)
		if (eqstr("-w", argv[1]))
		{
			wtflag = 2;
			argc--;
			argv++;
		}
		else break;
#endif
	if (argc > 1) symfil = argv[1];
	if (argc > 2) corfil = argv[2];
	argcount = argc;

	setbout(); /* setup a.out file */
	setcor(); /* setup core file */

	var[VARB] = datbas; /* setup variables */
	var[VARD] = datsiz;
	var[VARE] = entrypt;
	var[VARM] = magic;
	var[VARS] = stksiz;
	var[VART] = txtsiz;

	printf("ready\n");
	if ((sigint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
	{
		sigint = fault;
		signal(SIGINT, fault);
	}
	sigqit = signal(SIGQUIT, SIG_IGN);
	setjmp(env);

	if (executing == TRUE) delbp();
	executing = FALSE;

	while(1)
	{
		flushbuf();
		if (errflg)
		{
			printf("%s\n", errflg);
			exitflg = (int)errflg;
			errflg = 0;
		}
		if (mkfault)
		{
			mkfault=0;
			printc(EOR);
			prints(DBNAME);
		}
		lp = 0;
		rdc();
		lp--;
		if (eof)
		{
			if (infile != stdin)
			{
				iclose();
				eof=0;
				longjmp(env);
			} else done();
		} else exitflg = 0;
		command(0, lastcom);
		if (lp && (lastc != EOR)) error(NOEOR);
	}
}

done()
{
	endpcs();
	exit(exitflg);
}

