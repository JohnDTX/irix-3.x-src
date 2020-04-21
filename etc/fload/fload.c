#include <fcntl.h>
#include <signal.h>
#include <fpopcodes.h>
#ifdef NOTDEF
#include "fpregs.h"
#endif
#include <stdio.h>

#define SKYCOMREG (unsigned short *)0x80040
#define SKYSTATREG (short *)0x80042
#define SKYDTREG (unsigned long *)0x80044
int nosky();
static unsigned long ucode[4096];
char *mcfilenm = "/lib/skyffp.fas";
main()
{
	/* fload.c - load the microcode for the
	   sky floating point board. */

	int fpunit;	
	int nbytes,verbose=0;

	/* set up to catch bus errors and segmentation faults */
	signal(SIGBUS, nosky);
	signal(SIGSEGV, nosky);

	if (phys(1,0x80000,0x1000,0x1000000) == (-1)) {
		skyerror("phys call failed");
	}

	if ((fpunit = open(mcfilenm,O_RDONLY)) < 0) {
		fprintf(stderr,"cant open microcode file %s\n",mcfilenm);
		skyerror("");
	}

	/* microcode file open... read it */
	nbytes = read(fpunit,ucode,4096*4);

	if (verbose) fprintf(stderr,"read 0x%x bytes\n",nbytes);

	close(fpunit);


	ffload(ucode);

	ffinit();

	fftest();

	fprintf(stderr,"fload: skyffp at mbio 40 - initialized and tested.\n");
	exit(0);

}

ffinit() {

	/* initialize the sky board */

	*SKYSTATREG = 0x80;
	*SKYCOMREG = 0x1000;
	*SKYCOMREG = 0x1000;
	*SKYCOMREG = 0x1001;
	*SKYSTATREG = 0x40;
}	

fftest() {

	/* do minimal sky board testing.  These are the tests 
	   suggested by the Sky ffp system integration manual, Rev K,
	   ppg 6 */

	/* restore context of all bits set */
	int i;

	*SKYCOMREG = HW_CTXRSTR;
	for (i=0;i<8;i++) *SKYDTREG = 0xffffffff;

	/* read back context and check it */
	*SKYCOMREG = HW_CTXSV;
	for (i=0;i<8;i++) 
		if (*SKYDTREG != 0xffffffff) {
			skyerror("board test failure");
		}

	/* last, execute the ln(1) function */
	*SKYCOMREG = HW_LN;
	*SKYDTREG = 0x3f800000;	/* float 1.0 */
	while (*SKYSTATREG >0 ) ;
	if (*SKYDTREG) {
		skyerror("board function test failure");
	}
}

nosky() {
	/* got a bus error or segmentation violation when accessing
	   sky board */

	skyerror("skyffp not installed");
}


skyerror(cptr) char *cptr;
{
	/* cant initialize the sky board for some reason */
	if (!cptr) cptr = "error initializing skyffp";
	fprintf(stderr,"fload: %s.\n",cptr);
	exit(-1);
}

