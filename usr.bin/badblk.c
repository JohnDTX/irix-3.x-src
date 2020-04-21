char _Origin_[] = "UniSoft Systems of Berkeley";

#include <stdio.h>
#include <sys/param.h>
#include <sys/altblk.h>
#include <sys/uioctl.h>


/*
 * badblk [-w] [-mN] /dev/rXYZ [#S] - program to set or update 
 *			bad block information
 *               where  -w indicates write/verify option
 *			-m sets the max number of alternate blocks to N
 *		 	/dev/rXYZ is device name
 *			#S is one or more block numbers separated by blanks
 *
 * If program invoked with [-w] option, write/verify performed
 * to determine if bad block; otherwise only read done.
 *
 * If program invoked with [-mN] option, the number of alternate blocks
 * will be set to N. Program panics if N > NICALT.
 * 
 * If program invoked with no specific block numbers, and no bad block
 * verification has been done before, then each block on disk
 * is checked (either just read or write/verify) and bad block information
 * in block 0 is set up from scratch.
 *
 * If program invoked with no specific block numbers, but block 0 already
 * contain bad block information set up earlier, then a verification
 * on the whole disk is done, and any new bad blocks not already on
 * block 0 table will be added.
 * 
 * If program invoked with device name plus block number/s, then
 * only indicated block/s is/are updated in block 0. 
 * 
 * After alternate tracks are assigned, block 0 is updated and 
 * the updated blocks are verified to make sure alternate blocks
 * are good.  If alternate blocks are not good, new alternate
 * block numbers are assigned.
 */

typedef char bool;
#define TRUE	1
#define FALSE	0
#define UPDATE	2
#define DEOF	0	/*end of disk file*/


#define BADBLOCK -1
#define ERROR -1



bool    wrflag;		/*write/verify flag*/
bool	whole;		/*set up information for whole disk*/
bool	frstime;	/*flag for first time bad block processiing*/
int     fildes;		/*file descriptor for device*/
int	nblk;		/*number of block*/
int	nbad;		/*number of bad block*/
char 	chbuf[BSIZE];	/*buffer for verification*/
struct altblk	althead;	/*buffer for bad block info*/
int 	ntry;		/*number of tries to get all good alternate blocks*/
char    **bdpar;		/*pointer to first bad block number*/
int	nbdbk;		/*number of bad block in parameter*/
int	maxapar;	/*max number of alternate blocks specified by user*/
int	info[NICALT];	/*buffer for bad block numbers*/





main(argc,argv)
	int argc;
	char *argv[];
{
	if (getpar(argc,argv) == NULL)	/*get parameters*/
		panic("Usage: badblk [-w] [-mN] /dev/rXYZ [#S]");

	else {
		if (whole)	        /*set up whole disk info*/
			setall();
		else update();		/*up date specific block/s*/
	}

	wrhead();	/*write header info back in block 0*/
	ntry = 0;	/*number of tries*/

	while (verify() == ERROR)
		wrhead();	/*keep verifying until all updates are
					good*/
	prstat();	/*print job statistic*/
}

/**********************************************************************/

int getpar(argc,argv)  	/*get parameter*/
	int argc;
	char *argv[];
{
	register char *cp;
	register int i;



	if (argc > 1) {
		argv++;
		cp = *argv;	/*points to first argument*/
		wrflag = FALSE;
		maxapar = -1;
		while (*cp++ == '-') {
			if (*cp == 'w') { 	/*see if write/verify*/
				wrflag = TRUE;
				for (i=0;i<BSIZE;i++)
					chbuf[i] = (char) i;
						/*set up check buffer*/
			}
			else if (*cp++ == 'm') {  /*max bad track allowed*/
					sscanf(cp,"%d\n",&maxapar);
					if (maxapar > NICALT)
						panic("Max block # too large"); 
			     }
			     else panic("Invalid parameter");
			argv++; 
			argc--;
			cp = *argv;  	/*points to next argument*/
				
 		}

		/*open device and read in block 0*/
		if ((fildes = open(*argv,UPDATE)) == ERROR)
			panic("Cannot open device");
		else if (read(fildes,&althead,BSIZE) == DEOF)
						/*read block 0*/
				panic("Cannot read bad block information");

		if (argc > 2) {
			nbdbk = argc - 2;	/*number of bad block in param*/
			whole = FALSE;
			bdpar = ++argv;		/*sets address of first param*/
		}
		else whole = TRUE;

		if (althead.a_magic == ALTMAGIC) {
			frstime = FALSE;
			fprintf(stdout, "Disk has %d bad blocks\n",
						althead.a_count);
			if (maxapar < 0)
				maxapar = althead.a_nicbad;
		} else {
			frstime = TRUE;
			althead.a_magic = ALTMAGIC;
			althead.a_count = 0;
			althead.a_maxalt = -1;
			fprintf(stdout, "Disk has no current bad block info\n");
			if (maxapar < 0)
				maxapar = NICALT;
		}
		return (1);
	}
	else return (NULL);

}

/*********************************************************************/
 
setall()	/*set up block 0*/
{
	register int offset;
	int blksz;
	      

	nblk = 0;
	nbad = althead.a_count - 1;
	while ((blksz = checkbk(++nblk)) != DEOF) {
		fprintf(stderr,"%d\r",nblk);
		if (blksz == BADBLOCK) {
			nbad++;
			if (!frstime) { /*not first time*/
				if ((offset = findbk(nblk)) == ERROR)
					panic("Cannot locate block in table");
				else if
				      (althead.a_map[-offset].a_altbk != nblk)
						/*not already there*/
					resetbk(offset);
			}
			else {
				althead.a_map[-nbad].a_altbk = nblk;
				althead.a_map[-nbad].a_index = nbad;
				info[nbad] = nblk;
			}
		}
	}
	if (frstime) {
		althead.a_count = nbad + 1;
		althead.a_maxalt = nbad;
	}
}

/*********************************************************************/
		
int checkbk(blkno)	/*check if bad block*/
	register int blkno;
{

	register int i;
	char	buf2[BSIZE];

	lseek(fildes,blkno*BSIZE,0);

	if (wrflag) {	/*write/verify operation*/
		if ((i = write(fildes,chbuf,BSIZE)) > 0) {
			lseek(fildes,-BSIZE,1);	/*backup pointer*/
			if ((i = read(fildes,buf2,BSIZE)) > 0) 
				i = compbuf(buf2);          
		}
	}
	else i = read(fildes,buf2,BSIZE);      
	  
	return(i);
}

/*********************************************************************/
		

int compbuf(bptr)	/*compare buffer with chbuf*/
	char *bptr;
{
	register char *p1,*p2;
	register int n;

	p1 = bptr;
	p2 = chbuf;
	n = 0;


	while ((*p1++ == *p2++) && (n++ < BSIZE));
	return(n);

		
}



/*********************************************************************/

update()
{
	register int offset;

	nbad = 0;

	while (nbdbk != 0) {
		sscanf(*bdpar,"%d",&nblk);	/*convert to decimal from char*/
		if ((offset = findbk(nblk)) == ERROR)
			/*do binary search to find block*/
			panic("ERROR in locating block offset");
		else resetbk(offset);
			/*reset all entries in block 0 beyond found
					block*/
		nbad++;
		bdpar++;
		nbdbk--;
	}
}


/*********************************************************************/

int verify()	/*verify if new alternate tracks are good*/
{
	register int i,n;
	bool errflg;
	struct altblk bad1;

	ntry++;
	lseek(fildes,0,0);	/*return to block 0*/
	read(fildes,&bad1,BSIZE);
	errflg = FALSE;
	if (!frstime || !whole) { /*verify only those updated*/
		for (i=0; i<nbad; i++) {
			if (checkbk(info[i]) == ERROR)	{
				errflg = TRUE;
				if ((n = findbk(info[i])) == ERROR)
					panic("Cannot find block in table");
				else resetbk(n);
			}
		}
	}
	else	/*verify whole*/
		for (i=0;i<bad1.a_count && !errflg;i++)
			if ((n = checkbk(bad1.a_map[-i].a_altbk)) == ERROR)	{
				errflg = TRUE;
				if ((n = findbk(bad1.a_map[-i].a_altbk)) == ERROR)
					panic("Cannot find block in table");
				else resetbk(n);
			}	
	if (errflg) return(ERROR);
	else return(0);
}

/*********************************************************************/

int findbk(bn)	/*find block given block number bn*/ 
	register int bn;
{
	register int lb,ub;
	register mid;

	if (althead.a_count == 0)
		return(0);
	lb = 0;
	ub = althead.a_count - 1;
	mid = (ub - lb) / 2;
	while (ub != lb) {

		if (althead.a_map[-mid].a_altbk == bn) 
			break;
		if (althead.a_map[-mid].a_altbk > bn)
			ub = mid;
		else
			lb = mid + 1;
		mid = ((ub - lb) /2) + lb;
	}

	if (althead.a_map[-mid].a_altbk < bn)
		mid++;
	return(mid);


}


/*********************************************************************/

resetbk(offset)
	register int offset;
{
	register int i;

	althead.a_maxalt++;
	info[nbad] = nblk;
	if (althead.a_map[-offset].a_altbk == nblk) 
		althead.a_map[-offset].a_index = althead.a_maxalt;
	else {
		for (i=althead.a_count-1; i>=offset;i--)
			althead.a_map[-(i+1)] = althead.a_map[-i];
		althead.a_map[-offset].a_altbk = nblk;
		althead.a_map[-offset].a_index = althead.a_maxalt;
		althead.a_count++;
	}
}




panic(cp)
	char *cp;
{

	fprintf(stderr, "%s\n", cp);
	exit(1);
}


		
/*********************************************************************/

wrhead()
{
	lseek(fildes,0,0);	/*back to block 0*/
	althead.a_nicbad = maxapar; /*reset max allowed*/
	write(fildes,&althead,BSIZE);
	if (ioctl(fildes,UIOCBDBK,0) < 0)   /*disk driver bad block update*/
		panic("Cannot update bad block information");
}

/*********************************************************************/

prstat()
{
	if (whole) {
		fprintf(stderr,"%d blocks checked\n",nblk);
		fprintf(stderr,"%d bad block/s on disk\n",nbad+1);
	}
	else fprintf(stderr,"%d bad block/s updated\n",nbad);
	if (ntry > 1) 
		fprintf(stderr,"%d tries to get all good blocks\n",ntry);
}

