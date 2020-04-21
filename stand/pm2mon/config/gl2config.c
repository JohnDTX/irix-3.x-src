#define DC4 1
#include "Qdevices.h"
#include "dcdev.h"
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>

struct _config {
	char *name;
	unsigned char config;
	} configs[5] = {
		{ "rs170", ((PROM|OPTCLK) >> 7) },
		{ "non-interlaced", ((PIPE4|PROM) >> 7) },
		{ "interlaced", 0 },
		{ "",0 }
	    };

main(argc,argv) 
int argc;
char **argv;
{

	/* make the dc4 configuration proms */

	/* switches :

		-l <type> - type of low system
		-h <type> - type of high system
		-o <file> - output file name (for pseudo .s file)

	*/

	struct _config *configptr,*hightype,*lowtype;
	char lowsystem[80],highsystem[80],outfile[200],buf[200];
	int out;
	lowsystem[0]=highsystem[0]=outfile[0] = 0;
	argv++; argc--;

	while (argc>0) {

	    if ((*argv)[0] == '-') {
		switch ((*argv)[1] ) {

		    case 'l':	if (*(argv+1)[0] != '-') {
				    ++argv;
				    strcpy(lowsystem,*argv);
				    argc--;
				}
				break;

		    case 'h':	if (*(argv+1)[0] != '-') {
				    ++argv;
				    strcpy(highsystem,*argv);
				    argc--;
				}
				break;
				
		    case 'o':	if (*(argv+1)[0] != '-') {
				    ++argv;
				    strcpy(outfile,*argv);
				    argc--;
				}
				break;

		    default:
			printf("illegal switch %s - ignored\n",&(*argv[1]));

		}
	    }
	    else printf("spurious command argument %s - ignored\n",*argv);
	    ++argv;
	    argc--;
	}

	do {
	    if (!(*outfile)) {
		printf("enter output file name:");
		gets(buf);
		if (!isalnum(buf[0])) 
			printf(
		    "entered name %s has non-alphanumeric first character\n");
		else 
		    strcpy(outfile,buf);

	    }
	    if (*outfile)
		if ((out = open(outfile,O_CREAT|O_WRONLY,0777))<0 ) {
		    printf("cant open file  %s\n",outfile);
		    *outfile = 0;
		}

	} while (!(*outfile));

	do {
	    if (!*lowsystem) {
		printf("enter low system name:");
		gets(buf);
		if (!isalnum(buf[0]))  {
		    if (buf[0] == '?') showmodes();
		    else
			printf(
		    "entered name %s has non-alphanumeric first character\n");
		}
		else  {
		    strcpy(lowsystem,buf);
		    tolower(lowsystem);
		}
	    }

	    if (!*highsystem) {
		printf("enter high system name:");
		gets(buf);
		if (!isalnum(buf[0])) {
		    if (buf[0] == '?') showmodes();
		    else
			printf(
		    "entered name %s has non-alphanumeric first character\n");
		}
		else {
		    strcpy(highsystem,buf);
		    tolower(highsystem);
		}
	    }

	    for (configptr = configs;*(configptr->name);configptr++) {
	        if (strncmp(configptr->name,lowsystem,strlen(lowsystem)) == 0) {
		    /* found the type! */
		    lowtype = configptr;
		    break;
	        }
	    }
	    if (!(configptr->name[0])) {
		printf("no such system name: %s\n",lowsystem);
		showmodes();
		lowsystem[0]=0;
	    }
    
	    for (configptr = configs;*(configptr->name);configptr++) {
	        if (strncmp(configptr->name,highsystem,strlen(highsystem)) == 0) {
		    /* found the type! */
		    hightype = configptr;
		    break;
	        }
	    }
	    if (!(configptr->name[0])) {
		printf("no such system name: %s\n",highsystem);
		highsystem[0]=0;
		showmodes();
	    }
	
	} while ((!*lowsystem) || (!*highsystem)) ;


	/* ok, hightype and lowtype are found, and the file is open */

	/* make the prom .s file.  The structure is :

		magic byte 0
		magic byte 1
		low config byte
		high config byte
	*/

	buf[0] = (char)DC4_PROMVAL0;
	buf[1] = (char)DC4_PROMVAL1;
	buf[2] = lowtype->config;
	buf[3] = hightype->config;
	if ((write(out,buf,4)) != 4) 
		printf("write failed.\n");
	close(out);
}

tolower(cptr) 
register char *cptr;
{
	register char c;
	do {
	    c = *cptr;
	    if (isupper(c)) 
		c = c - 'A' + 'a';
	    *cptr = c;
	}
	while (*++cptr);
}

showmodes() {
	register struct _config *config = configs;

	printf("\nsystem choices:\n");
	while (*(config->name)) {
		printf("\t%s\n",config->name);
		config++;
	}
}
