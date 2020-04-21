/* @(#)gtcfile.c	1.2 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/gtcfile.c,v $*/
/* $Revision: 1.1 $*/
/* $Date: 89/03/27 18:30:27 $*/

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "uust.h"


/*
 *	gtcfile
 */
#define MAXCOUNT 20	/* maximun number of commands per C. file */


FILE *Cfp = NULL;
char Cfile[NAMESIZE];

/*
 * get a Cfile descriptor
 * return
 *	an open file dexcriptor
 */
#define NSYS	20
struct presys{
	char	pre_name[NAMESIZE];
	char	pre_id[2];
	char	pre_grade;
}presys[NSYS];
int	nsys	= 0;

/*
 * get a Cfile descriptor
 * return:
 *	an open file descriptor
 */
FILE *
gtcfile(sys, Grade)
register char *sys;
{
	static int cmdcount = 0;
	register int i;
	struct presys *p;
	struct presys *csys();
	extern int errno;
	char *strcpy();
	char	sqnm[5];

	/*
	 */
	DEBUG(4,"gtcfile0: sys - %s\n",sys);
	if((p = csys(sys, Grade)) == NULL){

		DEBUG(4,"gtcfile1: sys - %s\n",sys);
		if(nsys != 0){
			clscfile();
		}
		cmdcount = 1;
		if(jobid == 0){
			getseq(sqnm);
			subjob[0] = '@';
			DEBUG(4,"gtcfile2: sqnm - %d\n",sqnm);
		}
		updjb(); sprintf(Cfile, "%c.%.6s%c%.1s%.4d", CMDPRE, sys, Grade, subjob, jobid); 
                DEBUG(4,"gtcfile3: jobid - %d\n",jobid);
		Cfp = fopen(Cfile, "a+");
		ASSERT(Cfp != NULL, "CAN'T OPEN", Cfile, 0);
		insys(sys, subjob[0], Grade);
	}else {
		DEBUG(4,"gtcfile4: presys[nsys-1] : %s\n",presys[nsys-1].pre_name);
		clscfile();
		sprintf(Cfile, "%c.%.6s%c%.1s%.4d", CMDPRE, sys, Grade, p->pre_id, jobid); 
		Cfp = fopen(Cfile, "a+");
		ASSERT(Cfp != NULL, "CAN'T OPEN", Cfile, 0);
	}
	return(Cfp);
}
struct presys *
csys(p, g)
register char *p, g;
{
	register int i;
	static int jid = 0;

	if(jobid != jid){
		DEBUG(4,"csys0 - jobid: %d\n",jobid);
		jid = jobid;
		nsys = 0;
	}
	for(i=0;i<nsys;i++){
		DEBUG(4,"csys1 - presys: %s\n",presys[i].pre_name);
		if(strcmp(p,&presys[i].pre_name[0]) == SAME)
			if(g == presys[i].pre_grade)
				return(&presys[i]);
	}
	return(NULL);
}

insys(p, c, g)
register char *p;
char	c;
{

	strcpy(presys[nsys].pre_name, p);
	presys[nsys].pre_grade = g;
	presys[nsys++].pre_id[0] = c;
}

/*
 * close cfile
 * return
 *	none
 */
clscfile()
{
	if (Cfp == NULL)
		return;
	fclose(Cfp);
	chmod(Cfile, 0666);  /* for debugging purposes only */
	/* chmod(Cfile, ~WFMASK & 0777); */
	logent(Cfile, "QUE'D");
	US_CRS(Cfile);
	Cfp = NULL;
	return;
}
