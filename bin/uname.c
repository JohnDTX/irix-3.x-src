char _Origin_[] = "System V";

/*	@(#)uname.c	1.2	*/

/*
 * Must be set-uid root as it needs to read disk labels.
 */

#include <stdio.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/dklabel.h>
#include <sys/kversion.h>

#define ERROR		fprintf(stdout,"<error>"); xstat = 1

struct utsname	unstr, *un;
int	xstat = 0;	

main(argc, argv)
char **argv;
int argc;
{
	int	sflg=0, nflg=0, rflg=0, vflg=0, mflg=0, errflg=0;
	int	kflg=0, hflg=0, gflg=0, tflg=0;
	int	optlet;
	char	buf[100];	/* that should be big enough */
	int	needsp;

	un = &unstr;
	uname(un);

	if (argc == 1)
	    sflg++;
	else
	    while((optlet=getopt(argc, argv, "asnrvmkhgt")) != EOF)
		switch(optlet) {
		    case 'a':
			sflg++; nflg++; rflg++; vflg++; mflg++;
			kflg++; hflg++; gflg++; tflg++;
			break;
		    case 's':
			sflg++;
			break;
		    case 'n':
			nflg++;
			break;
		    case 'r':
			rflg++;
			break;
		    case 'v':
			vflg++;
			break;
		    case 'm':
			mflg++;
			break;
		    case 'k':
			kflg++;
			break;
		    case 'h':
			hflg++;
			break;
		    case 'g':
			gflg++;
			break;
		    case 't':
			tflg++;
			break;
		    case '?':
			errflg++;
	}
	if(errflg) {
		fprintf(stderr, "usage: uname [-snrvmkhgta]\n");
		exit(1);
	}
	if(sflg) {
		fprintf(stdout, "%.9s", un->sysname);
		needsp = 1;
	}
	if(nflg) {
		if(needsp) putchar(' ');
		fprintf(stdout, "%.9s", un->nodename);
		needsp = 1;
	}
	if(rflg) {
		if(needsp) putchar(' ');
		fprintf(stdout, "%.9s", un->release);
		needsp = 1;
	}
	if(vflg) {
		if(needsp) putchar(' ');
		fprintf(stdout, "%.9s", un->version);
		needsp = 1;
	}
	if(mflg) {
		if(needsp) putchar(' ');
		fprintf(stdout, "%.9s", un->machine);
		needsp = 1;
	}
	if(kflg) {
		if(needsp) putchar(' ');
		if (getversion(KVERS_KERNEL,buf) < 0) {
		    ERROR;
		}
		else
		    fprintf(stdout, "%s", buf);
		needsp = 1;
	}
	if(hflg) {
		if(needsp) putchar(' ');
		if (getversion(KVERS_GLTYPE,buf) < 0) {
		    ERROR;
		}
		else
		    fprintf(stdout, "%s", buf);
		needsp = 1;
	}
	if(gflg) {
		if(needsp) putchar(' ');
		if (getversion(KVERS_GL,buf) < 0) {
		    ERROR;
		}
		else
		    fprintf(stdout, "%s", buf);
		needsp = 1;
	}
	if(tflg) {
		if(needsp) putchar(' ');
		model();
	}
	putchar('\n');
	exit(xstat);
}

#define	MODEL	"/etc/model"	/* the file which holds the model # */

model()
{
	FILE	*fd;
	char	buf[ 512 ];

	if ( ( fd = fopen( MODEL, "r" ) ) == NULL )
	{
		ERROR;
		fclose( fd );
		return;
	}

	if ( fgets( buf, 512, fd ) == NULL )
	{
		ERROR;
		fclose( fd );
		return;
	}
	buf[ strlen( buf ) - 1 ] = '\0';
	fprintf( stdout, "%s", buf );
}
