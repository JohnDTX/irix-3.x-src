int basetype,curclass,curtype,nargs,_par_lineno,retval;
int stringsused;
int curnum;

#ifndef OLDWAY
int localspace;
#endif
 
char lastname[],tokenbuf[],lasttoken[];
char *ipfnm,*opfnm;
char *progname;
int nfuncs;

#define N_ISWRP 0x42
#define WRAPPING_C 3
#define WRAPPING_FORTRAN 1

struct ftn_s {
	char centry[64];
	char fentry[64];
	int  class,type,nargs;} ;
	
struct ftn_s ftn;
