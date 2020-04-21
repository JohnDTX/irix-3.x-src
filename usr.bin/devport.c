/*
 *	devport - assign serial ports to digitizing tablet or dial box
 *
 *
 */
#include "device.h"
#include "stdio.h"

main(argc,argv)
int argc;
char **argv;
{
#ifdef gl2
    int i;

    if(argc == 1) 
	pusage();
    gbegin();
    for(i=1; i<argc; i++) {
	if(strcmp(argv[i],"-t") == 0) {
	    if(++i == argc) 
		pusage();
	    else
		devport(BPAD0,atoi(argv[i]));
	} else if(strcmp(argv[i],"-d") == 0) {
	    if(++i == argc) 
		pusage();
	    else
		devport(DIAL0,atoi(argv[i]));
	} else {
	    fprintf(stderr,"unrecognized option %s\n",argv[i]);
	    exit(1);
	}
    }
    exit(0);
#endif
#ifdef gl1
    fprintf(stderr,"devport is not supported for gl1\n");
    exit(1);
#endif
}

pusage()
{
    fprintf(stderr,"usage: devport [-t portno] [-d portno]\n");
    exit(1);
}
