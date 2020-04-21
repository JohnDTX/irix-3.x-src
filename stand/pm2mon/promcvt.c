#include <a.out.h>
#include <stdio.h>
#include <fcntl.h>

#define fatal(a) {fprintf(stderr,"FATAL:%s\n",a);exit(-1);}
#define werror(a,b) {fprintf(stderr,a,b);}

#define BUFSIZE 0x8000

char ipfilenm[128],opfilenm[128];
char buf[BUFSIZE + 1];
main(argc,argv)
int argc;
char *argv[];
{
	/* read a.out file from standard output.
	   remove header.
	   pad text portion to 32768 bytes and write to stdout.
	   add on the data portion.
	*/
/* header of a.out files 
struct bhdr {
	long	fmagic;
	long	tsize;
	long	dsize;
	long	bsize;
	long	ssize;
	long	rtsize;
	long	rdsize;
	long	entry;
};
*/

	struct bhdr header;
	char *bufptr;
	int in,out;
	int i,verbose=0,num;
	char switchc;
	unsigned long pad,dorg,bsize,tsize,dsize,tptr;
	*ipfilenm = *opfilenm = '\0';
	dorg = 0;
	bsize = 0x8000;
	i = 1;
	while (i<argc){ 

	    if (argv[i][0] == '-')
		switch (switchc = argv[i][1]) {

		    case 'o':	/* output file follows */
				if ( i++ < argc)
				    strcpy(opfilenm,argv[i]);
				else fatal("no output file name\n");
				break;

		    case 'd':	/* data origin follows */
				if ( i++ < argc)
				    sscanf(argv[i],"%x",&dorg);
				else fatal("no data origin\n");
				break;

		    case 'v':   verbose++;
				break;
		    
		    case 'b':	/* prom size follows */
				if ( i++ < argc)
				    sscanf(argv[i],"%x",&bsize);
				else fatal("no blocksize\n");
				break;

		    default:
				werror("unrecognized switch -%c",switchc);
			
		}
	    else {
		if (!*ipfilenm) strcpy(ipfilenm,argv[i]);
		else fatal("too may input filenames\n")
		}
	    i++;
	}

	if (!*ipfilenm) strcpy(ipfilenm,"a.out");
	if (!*opfilenm) strcpy(opfilenm ,"a.prom");
	if (!dorg) dorg = 0xf90000;

	if ((in = open(ipfilenm,O_RDONLY)) < 0) {
	    fprintf(stderr,"cant open input file %s\n",ipfilenm);
	    exit(-1);
	}
	if ((out = open(opfilenm,O_WRONLY|O_CREAT,0666)) < 0) {
	    fprintf(stderr,"cant open output file %s\n",opfilenm);
	    exit(-1);
	}

	if (verbose) fprintf(stderr,"converting prom file %s to %s\n",
			     ipfilenm,opfilenm);


	num = read(in,&header,sizeof header);

	if (verbose) fprintf(stderr,"read %x header bytes \n",num);

	if (verbose) fprintf(stderr,"tsize = %x, dsize = %x\n",header.tsize,header.dsize);

	/* read the text segment */
	if (verbose) fprintf(stderr,"TEXT:");
	tsize = header.tsize;
	tptr = header.entry;
	while (tsize > BUFSIZE) {
	     read(in,buf,BUFSIZE);
	     write(out,buf,BUFSIZE);
	     if (verbose) fprintf(stderr,"*");
	     tsize -= BUFSIZE;
	     tptr += BUFSIZE;
	}
	if (tsize) {
	    num = read(in,buf,tsize);
	    write(out,buf,tsize);
	    tptr += tsize;
	}
	if (verbose) fprintf(stderr,"\n");
	pad = (dorg - tptr)%bsize ;

	if (pad < 0) fatal("data org overlaps text org\n")
	else 
	  for( bufptr=buf ; bufptr< &buf[BUFSIZE]; bufptr++) *bufptr='\0';
	if (verbose) fprintf(stderr,"padding %x bytes to data org\n",pad);
	if (verbose) fprintf(stderr,"PAD:");
	while (pad > BUFSIZE) {
	     write(out,buf,BUFSIZE);
	     if (verbose) fprintf(stderr,"*");
	     pad -= BUFSIZE;
	}
	if (pad) {
	    write(out,buf,pad);
	}
	if (verbose) fprintf(stderr,"\n");

	/* read the data segment */
	if (verbose) fprintf(stderr,"DATA:");
	while (header.dsize > BUFSIZE) {
	     read(in,buf,BUFSIZE);
	     write(out,buf,BUFSIZE);
	     if (verbose) fprintf(stderr,"*");
	     header.dsize -= BUFSIZE;
	}
	if (header.dsize) {
	    num = read(in,buf,header.dsize);
	    write(out,buf,header.dsize);
	}
	if (verbose) fprintf(stderr,"\n");

	close(in);
	close(out);
	exit(0);
}
