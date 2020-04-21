char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";

/*	sadp.c 1.6 of 6/28/82	*/
/*      sadp.c - For VAX and PDP11 machines,
		disk profiler profiles rp06, rm05 and general disk drives.
		It reads system buffer header pool, physical buffer header
		pool and swap buffer header pool once every second,
		to examine disk drive's I/O queue.
		For 3b20s system, it profiles the regular disk drives,
		it reads the circular output queue for each drive
		once every second.
	usage : sadp [-th][-d device[-drive]] s [n]
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#ifndef u3b
#include <sys/sysmacros.h>
#endif
#include <sys/buf.h>
#include <sys/elog.h>
#include <nlist.h>
#include <sys/iobuf.h>
#ifdef u3b
#include <sys/dma.h>
#include <sys/dir.h>
#include <sys/vtoc.h>
#include <sys/macro.h>
#include <sys/dfc.h>
#endif
#include <time.h>
#include <sys/utsname.h>
#include <sys/var.h>
#include <ctype.h>


/* cylinder profiling */
#define BLANK ' '
#define BLOB '*'
#define TRACE '.'
#define BRK '='
#define FOOT '-'
#define CYLNO   1
#define SEEKD   2
#define	CHPERCYL	103	/*the number of 8 cylinder chunks on a disk*/

#ifndef u3b
#define NDRIVE 32
#define cylin b_resid;
#define V 0
#define HPS 1
#define HMS 2
#define GDS 3
#define HPU 4
#define HMU 5
#define GDU 6
#define GDCNT 7
#define SBF 8
#define PBF 9
 
#ifdef vax
struct nlist setup[] = {
	{"_v"},
	{"_hpstat"},
	{"_hmstat"},
	{"_gdstat"},
	{"_hputab"},
	{"_hmutab"},
	{"_gdutab"},
	{"_gd_cnt"},
	{"_sbuf"},
	{"_pbuf"},
	{"_swbuf1"},
	{"_swbuf2"},
	{0},
};
struct buf bp[2];   /*   for swap buffer   */
#define SWP1 10
#define SWP2 11
#define M 13

#else
struct nlist setup[] = {
	{"_v"},
	{"_hpstat"},
	{"_hmstat"},
	{"_gdstat"},
	{"_hputab"},
	{"_hmutab"},
	{"_gdutab"},
	{"_gd_cnt"},
	{"_buf"},
	{"_pbuf"},
	{"_swbuf"},
	{0},
};
#define NSWP 3
struct buf bp[NSWP];
#define SWP 10
#define M 11
#endif
 
struct iobuf dkp[NDRIVE];
struct iotime ib[NDRIVE];
struct var tbl;
 
char *sbuf,*phybuf;
char devnm[3][5]={
	"rp06",
	"rm05",
	"disk"
};
 
int nonblk;
int index;
int index1,gdcnt;
unsigned temp1;
#else
#define NDRIVE 100
#define DSKINFO 0
#define DSKCNT  1
struct nlist setup[] = {
	{"dskinfo"},
	{"dsk_cnt"},
	{0},
};

char *dskloc;
char devnm[1][5]={
	"disk"
};
uint blkno,cptr;
int dskcnt;
struct {
	uint    iocnt;
	uint    lstptr;
	uint    bknum;
} linfo[8];
#endif
int fflg,dflg,tflg,hflg,errflg;
int s,n,ct;
static int ub = 8;
int sdist;
int m;
int dev;
int temp;
int f;
int i;
int n1,dleng,dashb,k;
int dashf;
int dn;
int drvlist[NDRIVE];
struct HISTDATA {
	long    hdata[CHPERCYL];
};
struct utsname name;
 
char *nopt;
char empty[30];
char drive[30];
char *malloc();
long lseek();
long dkcyl[8][CHPERCYL];
long skcyl[8][CHPERCYL];
long iocnt[8];
 
static char Sccsid[]="@(#)sadp.c	1.6";
main(argc,argv)
int argc;
char **argv;
{
	unsigned sleep();
	unsigned actlast,actcurr;
	extern  int     optind;
	extern  char    *optarg;
	int c,j;
	char *ctime(),*stime;
	long time(),curt;
	long skdist[8];
	long disk[8];


	while ((c = getopt(argc,argv,"thd:")) != EOF)
		switch(c) {
		case 't':
			tflg++;
			break;
		case 'h':
			hflg++;
			break;
		case 'd':
			dleng = strlen(optarg);
			if (dleng == 5){
				errflg++;
				break;
			}
			if (dleng > 4){
			for (i=5,n1=5;i<dleng;i++){
				if (optarg[i] == ','){
					if (n1 == i){
					errflg++;
					break;
					}
					if (getdrvn() != 0) {
						errflg++;
						break;
					}
					if (dashf != 0) {
						if (dashb >= dn){
						errflg++;
						break;
						}
					for (j=dashb;j<dn+1;j++)
						drvlist[j] = 1;
					dashb = 0;
					dashf = 0;
					}
					else
						drvlist[dn] = 1;
				n1 = i+1;
				}
				else {
				if (optarg[i] == '-'){
					if (dashf != 0) {
						errflg++;
						break;
					}
					if (getdrvn() != 0) {
						errflg++;
						break;
					}
					drvlist[dn] = 1;
					dashb = dn;
					dashf = 1;
					n1=i+1;
				}
				else { 
					if (i == dleng-1){
					i++;
					if (getdrvn() != 0) {
						errflg++;
						break;
					}
					if (dashf != 0)
						for (j=dashb;j<dn+1;j++)
							drvlist[j] = 1;
					else
						drvlist[dn] =1;
					}
				}
				}
			}
			}
			else {
				if (dleng != 4){
					errflg++;
					break;
				}

				for (i=0;i<8;i++)
					drvlist[i] = 1;
			}
			if (errflg)
				break;
#ifndef u3b
			temp = 3;
#else
			temp = 1;
#endif
			for (i=0;i<temp;i++)
				if (strncmp(optarg,devnm[i],4) == 0){
					dev = i;
					break;
				}
			if (i == temp){
				errflg++;
				break;
			}
			dflg++;
			break;
		case '?':
			errflg++;
			break;
		}
	if (errflg)
		exit1();

/*      if no frequency arguments present, exit */
	if (optind == argc)
		exit1();
/*      if a non-dash field is presented as an argument,
	check if it is a numerical arg.
*/
	nopt = argv[optind];
	if (tstdigit(nopt) != 0)
		exit1();
/*      for frequency arguments, if only s is presented , set n to 1
*/
	if ((optind +1) == argc) {
		s = atoi(argv[optind]);
		n = 1;
	}
/*      if both s and n are specified, check if 
	arg n is numeric.
*/
	if ((optind +1) < argc) {
		nopt = argv[optind + 1];
		if (tstdigit(nopt) != 0)
			exit1();
		s = atoi(argv[optind]);
		n = atoi(argv[optind+1]);
	}
	if ( s <= 0 )
		extmsg("bad value of s");
	if ( n <= 0 )
		extmsg("bad value of n");
	ct = s;
/*      get entries defined in setup from /unix */

	nlist("/unix",setup);
#ifdef vax
	for(i=0;i<M;i++)
		setup[i].n_value &= ~(0x80000000);
#endif
/*      open /dev/kmem  */

	if((f= open("/dev/kmem",0)) == -1)
		extmsg("cannot open /dev/kmem");

#ifndef u3b
	if (dflg == 0){
		for (i=0;i<3;i++){
			if (setup[i+HPU].n_value != 0 ){
				dev = i;
				break;
			}
		}
		if (i == 4)
			extmsg("None of rp06, rm05 and disk is defined");
		for (i=0;i<8;i++)
			drvlist[i] =1;
	}
	else 
		if (setup[dev+HPU].n_value == 0){
			fprintf(stderr,"Device %s is not defined\n",devnm[dev]);
			exit(2);
		}

/*      get gdcnt for gd drives */
	if (dev == GDU-HPU){
		lseek(f,(long)setup[GDCNT].n_value,0);
		if (read(f,&gdcnt,sizeof gdcnt) == -1)
			extmsg("cannot read general disk counter");
		ub = gdcnt * 8;
	}
/*      get storage from memory for sysbuf pool ,
	physical buy pool , and for queue cnt array     */
	lseek(f,(long)setup[0].n_value,0);
	read(f,&tbl,sizeof tbl);
	sbuf = malloc(sizeof (struct buf)* (tbl.v_buf + tbl.v_sabuf));
	if (sbuf == NULL)
		exit2();
	phybuf = malloc(sizeof (struct buf) * tbl.v_pbuf);
	if (phybuf == NULL)
		exit2();
#else
	if (dflg == 0){
		for (i=0;i<8;i++)
			drvlist[i] = 1;
		dev = 0;
	}

	lseek(f,(long)setup[DSKCNT].n_value,0);
	if (read(f,&dskcnt,sizeof dskcnt) == -1)
		extmsg("cannot read disk counter");
	dskloc = malloc(sizeof (struct dskinfo) * dskcnt);
	if (dskloc == NULL)
		exit2();
#endif

#ifdef vax
	/*  VAX only: system buffers found thru indirection  */
	if(setup[SBF].n_value == 0)
		extmsg("no 1st ptr to SBF");
	lseek(f, (long)setup[SBF].n_value, 0);
	if(read(f, &setup[SBF].n_value, sizeof (int)) == -1) 
		extmsg("no 2nd ptr to SBF");
	setup[SBF].n_value &= ~(0x80000000);
#endif

/*      get current I/O count for each drive    */
	for (;;){
		s = ct;
		for (k=0,i=0;k<ub;k++){
			if (drvlist[k] == 0)
				continue;
			for (j=0;j<CHPERCYL;j++){
				dkcyl[i][j] = 0;
				skcyl[i][j] = 0;
			}
			iocnt[i] = 0;
			disk[i] = 0;
			skdist[i] = 0;
			i++;
			if (i == 8){
				ub = k+1;  /*  only 8 drives are allowed */
				break;
			}
		}
/*      if no drives are selected or illegal drive number is specified, exit    */
		if (i == 0) 
			exit1();

/*      get i/o count for each disk     */
#ifndef u3b
		lseek(f,(long)setup[dev+HPS].n_value,0);
		if(read(f,ib,(sizeof (struct iotime)*ub)) == -1)
			extmsg("cannot read device status table");
		for(k=0,i=0;k<ub;k++)
		{
			if (drvlist[k] == 0)
				continue;
			iocnt[i] = ib[k].io_cnt;
			i++;
		}
#else
/*      initialize linfo table  */
		if (dsktbl(dskloc) == -1)
			exit3();
		getiocnt(dskloc);
/*      save io count into linfo table  */
		for (i=0;i<8;i++)
			linfo[i].iocnt = iocnt[i];
		ldlinfo(dskloc);
#endif
 
	for (;;){

#ifndef u3b
/*      take a snapshot of buffer header pool , swap buffer pool and
	physical buffer header  */

		lseek(f,(long)setup[HPU+dev].n_value,0);
		if( read(f,dkp,(sizeof (struct iobuf)* ub)) == -1){
			perror("sadp");
			extmsg("cannot read disk drive iobuf");
		}
 
/*      read system buffer header pool   */
		lseek(f,(long)setup[SBF].n_value,0);
		if (read(f,sbuf,tbl.v_buf*sizeof(struct buf)) == -1){
			perror("sadp");
			extmsg("cannot read system buffer pool");
		}

/*      read physical buffer header pool   */
		lseek(f,(long)setup[PBF].n_value,0);
		if (read(f,phybuf,tbl.v_pbuf*sizeof(struct buf))== -1){
			perror("sadp");
			extmsg("cannot read physical buffer pool");
		}
#ifdef vax
		for (m=SWP1;m<SWP2+1;m++){
			lseek(f,(long)setup[m].n_value,0);
			if (read(f,bp[m-SWP1],sizeof (struct buf)) == -1){
				fprintf(stderr,"cannot read phy bufhdr - %d\n",m);
				perror("sadp");
				exit(1);
			}
		}
#else
		lseek(f,(long)setup[SWP].n_value,0);
		if(read(f,bp,sizeof bp) == -1){
			fprintf(stderr,"cannot read swap bufhdr  %o\n",
			setup[5].n_value);
			perror("sadp");
			exit(1);
		}
#endif
		for (k=0,i=0;k<ub;k++){
			if (drvlist[k] == 0)
				continue;

/*      trace disk queue for I/O location, seek distance	*/

			nonblk = 0;
			if(dkp[k].b_actf != NULL ) {
#ifdef vax

				temp1 = (unsigned)dkp[k].b_actf - 0x80000000; 
				actlast = (unsigned)dkp[k].b_actl - 0x80000000;
#else
				temp1 = (unsigned)dkp[k].b_actf; 
				actlast = (unsigned)dkp[k].b_actl;
#endif
				index1 = 0;
				index = (int)(temp1 -setup[SBF].n_value)/
					(sizeof (struct buf));
				if ((testbuf() == -1) ||
					 (testdev() == -1) ||
					 ((unsigned)dkp[k].b_actf ==
					    (unsigned)dkp[k].b_actl)) {
					i++;
					continue;
				}
				sdist = temp;

#ifdef vax
				while ((temp1 + 0x80000000) != NULL) {
#else
				while (temp1 != NULL){
#endif
					nonblk = 0;
					actcurr =temp1;
					index1 =index;
					index =(int)(temp1 -setup[SBF].n_value)/
						(sizeof (struct buf));
					if ((testbuf() == -1) ||
						 (testdev() == -1))
						break;
					sdist = temp - sdist;
					if (sdist < 0)
						sdist = -sdist;
					skcyl[i][(sdist+7) >> 3]++;
					sdist = temp;

					if (actcurr == actlast)
						break;
				}
			}
		i++;
		}
#endif
#ifdef u3b
/*      start to snapshot the dskinfo table     */
		if (dsktbl(dskloc) == -1)
			exit3();
		getsample(dskloc);
#endif
		if (--s)
			sleep(1);
		else{

/*      at the end of sampling, get the present I/O count,
	and system name */
#ifndef u3b

			lseek(f,(long)setup[dev+HPS].n_value,0);
			read(f,ib,(sizeof (struct iotime)*ub));
#else
			if (dsktbl(dskloc) == -1)
				exit3();
			getiocnt(dskloc);
#endif
			uname(&name);

/*      print the report, there are two parts:
	cylinder profile, seeking distance profile */
			curt = time((long *) 0);
			stime = ctime (&curt);
			printf("\n\n%s\n",stime);
			printf("%s %s %s %s %s\n",
				name.sysname,
				name.nodename,
				name.release,
				name.version,
				name.machine);
			for (k=0,i=0;k<ub;k++){
			if (drvlist[k] == 0)
				continue;
				for (j=0;j<CHPERCYL;j++){
					disk[i] = disk[i] +dkcyl[i][j];
					skdist[i] = skdist[i] + skcyl[i][j];

				}
			i++;
			}
			if ((tflg == 0) && (hflg == 0))
				tflg = 1;
			if (tflg){
				printf("\nCYLINDER ACCESS PROFILE\n");
				for (k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
					continue;
					if (disk[i] != 0){
#ifndef u3b
						iocnt[i] = ib[k].io_cnt - iocnt[i];
#else
						iocnt[i] = iocnt[i] - (long)linfo[i].iocnt;
#endif
						printf("\n%s-%d:\n",
							devnm[dev],k);
						printf("Cylinders\tTransfers\n");
						for (j=0;j<CHPERCYL;j++){
							if (dkcyl[i][j] > 0)
								printf("%3d - %3d\t%ld\n",
								j*8,j*8+7,dkcyl[i][j]);
						}
						printf("\nSampled I/O = %ld, Actual I/O = %ld\n",
						disk[i],iocnt[i]);
						if (iocnt[i] > 0)
						printf("Percentage of I/O sampled = %2.2f\n",
						((float)disk[i] /(float)iocnt[i]) * 100.0);
					}
					i++;
				}

				printf("\n\n\nSEEK DISTANCE PROFILE\n");
				for (k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
						continue;
					if (skdist[i] != 0){
						printf("\n%s-%d:\n",
							devnm[dev],k);
						printf("Seek Distance\tSeeks\n");
						for(j=0;j<CHPERCYL;j++)
	
							if (skcyl[i][j] > 0){
								if (j == 0)
									printf("        0\t%ld\n",
									skcyl[i][j]);
								else
									printf("%3d - %3d\t%ld\n",
								j*8-7,j*8,skcyl[i][j]);
							}
						printf("Total Seeks = %ld\n",skdist[i]);
					}
					i++;
				}
			}
			if (hflg){
				for(k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
						continue;
					if (disk[i] != 0) {
						cylhdr(CYLNO,disk[i]);
						cylhist(disk[i],dkcyl[i]);
						cylftr(CYLNO);
					}
					i++;
				}
				for(k=0,i=0;k<ub;k++){
					if (drvlist[k] == 0)
						continue;
					if (skdist[i] != 0){
						cylhdr(SEEKD,skdist[i]);
						cylhist(skdist[i],skcyl[i]);
						cylftr(SEEKD);
					}
					i++;
				}
			}

			break;
		}
	}
	if (--n)
		continue;
	exit(0);
	}
}


#ifndef u3b
/*      determine if the I/O is from system buffer pool,
	or swap buffer pool or physical buffer  */
 
int testbuf()
{
	if ((temp1 <setup[SBF].n_value) || (index > (tbl.v_buf + tbl.v_sabuf))){
		index = (int)(temp1 -setup[PBF].n_value)/
			(sizeof (struct buf));
		if (index < tbl.v_pbuf){
			nonblk = 1;
			return(0);

		}
#ifdef vax
		for (m=SWP1;m<SWP2;m++){
			if (temp1 == setup[m].n_value){
				m = m-SWP1;
				nonblk =2;
				return(0);
			}
		}
	return(-1);

#else
	index = (int)(temp1 -setup[SWP].n_value)/
		(sizeof (struct buf));
	if (index < NSWP) {
		m = index;
		nonblk = 2;

		return(0);
	}
	else return(-1);
#endif
	}
	return(0);
}

/*      varify the I/O, get the cylinder number */

ckbits(x)
	register struct buf *x;
{
	register p;
	for (p=0;p<index;p++,x++)
		continue;
	if((x->b_flags & B_BUSY) &&
	    ((x->b_flags & B_DONE) == 0)){
		temp = x->cylin;
#ifdef vax
		temp1 = (unsigned)x->av_forw -
		    0x80000000;
#else
		temp1 = (unsigned)x->av_forw;
#endif
		return(0);
	}
	else
		return(-1);

}
int testdev()
{
	if((nonblk == 0) && (ckbits(sbuf) != -1))
		goto endtest;
	else {
		if ((nonblk == 1) && (ckbits(phybuf) != -1))
			goto endtest;

		else {

			if ((nonblk == 2) &&
			    ((bp[m].b_flags & B_BUSY) &&
			    ((bp[m].b_flags & B_DONE) == 0))){
				temp = bp[m].cylin;
#ifdef vax
				temp1 = (unsigned)bp[m].av_forw - 0x80000000;
#else
				temp1 = (unsigned)bp[m].av_forw;
#endif
			}
			else
				return(-1);
		}
	}
endtest:
	dkcyl[i][temp >> 3]++;
	return(0);
}
#else
 
dsktbl(x)
register struct dskinfo *x;
{
	lseek (f,(long)setup[DSKINFO].n_value,0);
	if (read(f,x,dskcnt*sizeof (struct dskinfo)) == -1)
		return(-1);
	return(0);
}
getiocnt(x)
register struct dskinfo *x;
{
	register p,i;
	for (p=0,i=0;p<ub;p++,x++)
		if (drvlist[p] == 0)
			continue;
		else{
#ifdef u3b
			iocnt[i] = x->devinfo.io_bcnt;
#else
			iocnt[i] = x->blkcnt;
#endif
			i++;
		}
	return;
}
 
ldlinfo(x)
register struct dskinfo *x;
{
	register i,k;
	for (i=0,k=0;k<ub;k++,x++)
		if (drvlist[k] == 0)
			continue;
		else {
			linfo[i].lstptr = (int)x->index == 0? 9: (x->index -1);
			linfo[i].bknum = x->outq[linfo[i].lstptr].blknum;
			i++;
		}
}

getsample(x)
register struct dskinfo *x;
{
	register i,k;
	int cntflg=0;
register j;
	for (i=0,k=0;k<ub;k++,x++){
		if (drvlist[k] == 0)
			continue;
		if (linfo[i].bknum == 0 && x->index == 0 &&
			x->outq[x->index].blknum == 0){
			i++;
			continue;
		}
		if (linfo[i].bknum == x->outq[linfo[i].lstptr].blknum)
			if (x->index == ((linfo[i].lstptr + 1) % 10)){
				i++;
				continue;
			} else {
				cptr = linfo[i].lstptr;
				cntflg = 1;
			}
		else
			cptr = x->index;
		blkno = x->outq[cptr].blknum;
		getcyl(blkno,x);
		if (cntflg)
			cntflg = 0;
		else
		dkcyl[i][temp >> 3]++;
		sdist = temp;
		while (cptr != x->index){
			blkno = x->outq[cptr].blknum;
			getcyl(blkno,x);
			dkcyl[i][temp >> 3]++;
			sdist = temp -sdist;
			if (sdist < 0)
				sdist = -sdist;
			skcyl[i][(sdist+7) >> 3]++;
			sdist = temp;
		}
		linfo[i].lstptr = (int)cptr == 0? 9: (cptr - 1);
		linfo[i].bknum = blkno;
		i++;
	}
}

getcyl(y,x)
register struct dskinfo *x;
register uint y;
{
	int res;
	int bpercyl;	/*blocks per cylinder*/
/* There are over 32600 tracks on a 675Mb disk
   and about 15485 tracks on a 300 Mb disk. In
   the following line of code the number 20000
   is used to test which type of disk this is.
   There are 608 blocks per cylinder on a 300
   Mb disk and 1280 blocks per cylinder on a
   675 Mb disk. */
	if (x->numtrks > 20000)
		bpercyl = 1280;
	else
		bpercyl = 608;
	res = (y % bpercyl) == 0? 0: 1;
	temp = (int)(blkno / bpercyl) + res;
	cptr = (cptr +1) % 10;
	return(0);
}
exit3()
{
	fprintf(stderr,"cannot read dskinfo table\n");
	exit(3);
}
#endif
 
/*      get drive number routine	*/
getdrvn()
{
	extern char *optarg;
	char *strcpy();
	char *strncat();
 
	strcpy(drive,empty);
	strncat(drive,&optarg[n1],i-n1);
	if (tstdigit(drive) != 0)
		return (-1);
	dn =atoi(drive);
	if (dn >= NDRIVE)
		return(-1);
	return(0);
}

exit1()
{
	fprintf(stderr,"usage:  sadp [-th][-d device[-drive]] s [n]\n");
	exit(1);
}

exit2()
{
	fprintf(stderr,"sadp: can't get memory, TRY AGAIN!!\n");
	exit(2);
}

extmsg(msg)
char	*msg;
{
	fprintf(stderr, "sadp: %s\n", msg);
	exit(4);
}

int tstdigit(ss)
char *ss;
{
	int kk,cc;
	kk=0;
	while ((cc = ss[kk]) != '\0' ){
		if (isdigit(cc) == 0)
			return(-1);
		kk++;
	}
	return(0);
}

/*      the following routines are obtained from iostat */
/* cylinder profiling histogram */
/*.s'cylhist'Output Cylinder Histogram'*/
cylhist(at, dp)
long at;
register struct HISTDATA *dp;
{
	register ii;
	int maxrow;
	long graph[CHPERCYL];
	long    max, max2;
	long    data;
	long    scale;

	max = 0;
	for(ii = 0; ii < CHPERCYL; ii++) {
		if(data = dp->hdata[ii]) {
			maxrow = ii;
			if(data > max) {
				max2 = max;
				max = data;
			} 
			else if (data > max2)
				max2 = data;
		}
	}
	maxrow++;

	/* determine scaling */
	scale = (max2 * 100)/at;
	for(ii = 24; ii > 2; ii /= 2) {
		if(scale > ii)
			break;
	}
	scale = 24 / ii;


	for(ii = 0; ii < maxrow; ii++) {
		if(dp->hdata[ii])
			graph[ii] = (scale * 100 * dp->hdata[ii]) / at;
		else
			graph[ii] = -1;
	}

	prthist(graph, maxrow, scale, (long) (max*100*scale/at));
}
/*.s'prthist'Print Histogram'*/

prthist(array, mrow, scale, gmax)
	long array[], scale, gmax;
register mrow;
{
	long    line;

	line = 50;
	/* handle overflow in scaling */
	if(gmax > 51) {
		line = 52;
		printf("\n%2ld%% -|", gmax/scale);
		pline(line--, array, mrow, BLOB);
		printf("\n     %c", BRK);
		pline(line--, array, mrow, BRK);
	} 
	else if ( gmax = 51 )
		line = 51;
	while( line > 0) {
		if((line & 07) == 0) {
			printf("\n%2ld%% -|", line/scale);
		} 
		else {
			printf("\n     |");
		}
		pline(line--, array, mrow, BLOB);
	}
	printf("\n 0%% -+");
	line = -1;
	pline( line, array, mrow, FOOT);
}
/*.s'pline'Print Histogram Line'*/
pline(line, array, mrow, dot)
	long line, array[];
int mrow;
char dot;
{
	register ii;
	register char *lp;
	char lbuff[132];

	lp = lbuff;
	for(ii = 0; ii < mrow; ii++)
		if(array[ii] < line)
			if(line == 1 && array[ii] == 0)
				*lp++ = TRACE;
			else
				*lp++ = BLANK;
		else
			*lp++ = dot;
	*lp++ = 0;
	printf("%s", lbuff);
}
/*.s'cylhdr'Print Cylinder Profiling Headers'*/

cylhdr( flag, total)
	long total;
{
	if(fflg)
		printf("\014\n");
	if( flag == CYLNO)
		printf("\nCYLINDER ACCESS HISTOGRAM\n");
	if (flag == SEEKD)
		printf("\nSEEK DISTANCE HISTOGRAM\n");
	printf("\n%s-%d:\n",
		devnm[dev],k);
	printf("Total %s = %ld\n", flag==CYLNO ? "transfers" : "seeks", total);
}
/*.s'cylftr'Print Histogram Footers'*/

cylftr( flag )
{
	if (flag == CYLNO)
		printf("\n      \t\t\tCylinder number, granularity=8");
	else
		printf("\n      =<<\t<\t<\t<\t<\t<\t<\t<\t<");
	printf("\n      081\t8\t1\t2\t2\t3\t4\t4\t5");
	printf("\n        6\t0\t4\t0\t7\t3\t0\t6\t2");
	printf("\n         \t \t4\t8\t2\t6\t0\t4\t8");
	printf("\n");
}
