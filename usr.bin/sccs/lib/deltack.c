#include <errno.h>
#include "../hdr/had.h"
#include "../hdr/defines.h"
#include "filehand.h"
#define FOREVER 1
#define MAXLIST 15
#define MAXLIST2 20
#define MAXLENCMR 12
char * Sccsids="@(#)deltack.c	1.11";
char *strtok(),*strcpy(),*strchr(),*malloc();
char errorlog[FILESIZE];		 /* log cmts errors here */
FILE *efd;

/*
*
*deltack(pfile,mrs,nsid,apl) peforms the nessesary validations on a delta
*involving the CMF FRED file of active CMR's
*
*/

deltack(pfile,mrs,nsid,apl)
	char pfile[];	/* the pfile name */

	char *mrs;		/* list of mrs from pfile */
	char *nsid;		/* sid id */
	char *apl;		/* application from the file */
	{
	 static char type[10],sthold[10];
	 char hold[302],*h,*fred;
	/*check for the existance of the p.file*/
	if(!pfile)
		{
		 error("Pfile non existant at deltack ");
		 return(0);
		}
	/*if no application serious error */
	if(!apl)
		{
		 error ("no application found with -fz flag");
		 return(0);
		}
	/*check for and retrieve FRED given the application name*/
	if(!getinfo(&fred,apl))
		{
		 error("no FRED file or system name in admin directory ");
		 return(0);
		}
	strcpy(errorlog,fred);
	*(errorlog + strlen(errorlog) - 11) = NULL;
	strcat(errorlog,"LOG");
	if(!(mrs))
		{
		 if (efd=fopen(errorlog,"a"))
			{fprintf(efd,"***CASSI REPORTS ERROR: no CMRS in pfile: %s\n",pfile);
			 fclose(efd);
			}
		 error("CMRs not on P.file -serious inconsistancy");
		 return(0);
		}
	 if(!promdelt(mrs,sthold,type,fred))
		{
		 return(0);
		}
	/*now build the chpost line  */
	strcpy(hold,mrs);
	h=strtok(hold,",\0");
	msg(apl,pfile,h,sthold,type,nsid,fred);
	while(h=strtok(0,",\0 "))
		{
		 msg(apl,pfile,h,sthold,type,nsid,fred);
		}
	return(1);
	}



/*
*
*promdelt(cmrs,stat,type,fred) allows one to modify the cmrs list and 
*enrter the type and status for the input to the MR system via the net
*
*/
promdelt(cmrs,stat,type,fred)
	char *cmrs,*stat,*type,*fred;
	{
	 extern char had[];
	 extern char * Cassin;
	 extern char * Sflags[];
	 static char hold[300],nold[300], *cmrlist[MAXLIST2 + 1];
	 char answ[100];
	 int i,j,numcmrs,fdflag=0,eqflag=0,holdnum,badflag=0;
	 char *h;
	 /*place the cmrs list in the array of pointers and remove the commas */
	 strcpy(hold,cmrs);
	 strtok(hold,",");
   	 cmrlist[0]=hold;
	 for(i=1;i<MAXLIST;i++)
		{
		 if((cmrlist[i]=strtok(0,",\0"))==(char *)NULL)
			{
			 break;
			}
		 }
	numcmrs=i;
	/* remove invalid cmrs from the list if now none set flag */
	for(i=0;cmrlist[i];i++)
		{
		 if(!verif(cmrlist[i],fred))
			{
			 /* a 'sd' cmr has been found */
			 if(numcmrs > 1)
				{
				 for(j=i;j<numcmrs;j++)
					{
					 cmrlist[j] = cmrlist[j + 1];
					}
				 numcmrs--;
				}
			 else /* there is a last cmr to delete set flag */
				{
				 cmrlist[0] = NULL;
				 badflag = 1;
				 numcmrs--;
				}
			}
		}
	if(HADZ) /*force delta flag on */
	{
		if (badflag) /* no legal cmrs in list */
		{
		 if (efd=fopen(errorlog,"a"))
			{fprintf(efd,"***CASSI REPORTS ERROR: no CMRS at sd\n");
			 fclose(efd);
			}
			fatal("no CMR's left, delta forbidden\n");
		}
		strcpy(stat,"sd");
		if (Sflags[TYPEFLAG - 'a'])
			strcpy(type,Sflags[TYPEFLAG - 'a']);
		else
			strcpy(type,"sw");
			/* rebuild cmr comma seperated list*/
		cat(nold,cmrlist[0],0);
		for(i=1;i<numcmrs;i++)
		{
			cat(nold,",",cmrlist[i],0);
		}
		strcpy(cmrs,nold);
		return(1);
	}
	while(FOREVER)
	  {
	   if(!badflag)
		{
		 printf("the CMRs for this delta now are:\n");
		 for(i=0;cmrlist[i + 1];i++)
			{
			 printf(" %s,",cmrlist[i]);
			}
		 printf("%s\n",cmrlist[i]);
		 printf("OK ??");
		 gets(answ);  
		 if((!strcmp(answ,"y")) || (!strcmp(answ,"ye")) || (!strcmp(answ,"yes")))
			{
			 break;
			}
		 }
	   else
		 {
		   printf("you must input a least 1 valid cmr number\n");
		 }
		 /*now prompt for new cmrs to add to the list*/
		while(FOREVER)
			{
			 eqflag = 0;
			 printf("enter new CMR number or 'CR' ");
			 gets(answ);
			 if(!strcmp(answ[0],NULL))
				if(!badflag)
					{
					 break;
					}
				else
					{
					 continue;
					}
			 h=malloc((unsigned)(strlen(answ) + 6));
			 strcpy(h,answ);
			 /*check for duplicate */
			 for (i=0;i<numcmrs;i++)
				{
				 if(!strcmp(h,cmrlist[i]))
					{
					 eqflag=1;
					 break;
					}
				}
			 if(eqflag==1)
				{
				 printf(" \n duplicate CMR number ignored\n");
				 continue;
				}
			 /*now verify that the cmr is in FRED */
			 if(!verif(h,fred))
				{
				 printf(" \n invalid CMR ignored \n");
				 continue;
				}
			 /*the addition is valid*/
			cmrlist[numcmrs] = h;
			badflag = 0; /* turn off the no cmrs found indicator */
			if(++numcmrs > MAXLIST2)
				{
				 printf(" \n to many CMRs added no more allowed \n");
				 break;
				}
			}
		/*now delete mrs from list */
	holdnum = numcmrs;
		while(FOREVER)
			{
			 fdflag = 0;
			 printf(" \n CMR number to delete or (CR) ? ");
			 gets(answ);
			 if(!(*answ))
				{
				 break;
				}
			 /*if one left break */
			 if(numcmrs==1)
				{
				 printf("\n only one CMR left can't delete more\n");
				 break;
				}
			 /*check if request is on list */
			 for(i=0;i<numcmrs;i++)
				{
				 if (!strcmp(answ,cmrlist[i]))
					{
					 fdflag=1;
					 for(j=i;j<numcmrs;j++)
						{
						 cmrlist[j]=cmrlist[j+1];
						}
					 break;
					}
				}
			if(fdflag==0)
				{
				 printf("\n not on list request ignored\n");
 				continue;
				}
			else
				{
				 numcmrs--;
				 /* we have oneless cmr */
				}
			}
		}
		/*here ends the cmr loop */
		/*set type to proper value*/
		if ( Sflags[TYPEFLAG - 'a'])
			strcpy(type,Sflags[TYPEFLAG - 'a']);
		else
			strcpy(type,"sw");
		/*set status*/
		strcpy(stat,"sd");
	/*reformat the cmrlist into a comma separated cmr list*/
	cat(nold,cmrlist[0],0);
	for(i=1;i<numcmrs;i++)
		{
		 cat(nold,nold,",",cmrlist[i],0);
		}
	 strcpy(cmrs,nold);
	return(1);
	}



/*
*
*msg(syst,cmrs,stats,types,sids) formats a message and calls cmrpost
*
*/
msg(syst,name,cmrs,stats,types,sids,fred)
	char *syst,*name,*cmrs,*stats,*types,*sids,*fred;
	{
	 FILE *fd;
	 extern char *Sflags[];
	 char holdit[300],*k;
	 char pname[FILESIZE],*ptr,holdfred[100],dir[100],path[FILESIZE];
	 struct stat stbuf;
	 int noexist = 0;
	 extern char *strchr();
	 /* if -fm flag contains a value substitute a the value for name */
	 if(k=Sflags[MODFLAG - 'a'])
	 {
		name = k;
	 }
	 if(*name != '/') /* not full path name */
		{
		 curdir(path);
		 cat(pname,path,"/",name,0);
		}
	else
		{
		 strcpy(pname,name);
		}
	abspath(pname);				/* get rid of . and .. */
/******** the net is replaced by psudonet ******
*	  sprintf(holdit,"netq %s chpost  %s q %s %s MID=%s MFS=%s q q",syst,cmrs,pname,types,sids,stats); 
*	 system(holdit);
************************************************/
	 /* build the name of the  termLOG file */
	 strcpy(holdfred,fred);
	 ptr=strchr(holdfred,'.');
	 *ptr=NULL;
	 strcat(holdfred,"source");
	 strcpy(dir,holdfred);
	 strcat(holdfred,"/termLOG");
	 if(stat(holdfred,&stbuf) == -1)
		noexist = 1; /*new termLOG */
	 if(!(fd=fopen(holdfred,"a")))
		{
		 if (efd=fopen(errorlog,"a"))
			{fprintf(efd,"***CASSI REPORTS ERROR: can't write to FRED : %s\n",pname);
			 fclose(efd);
			}
		 fatal(" Cassi Interface Msg not writable\n");
		 return(0);
		}
	 fprintf(fd,"%s chpost %s q %s %s MID=%s MFS=%s q q\n",syst,cmrs,pname,types,sids,stats);
	 fclose(fd);
	 if(noexist) /*new termLOG make owner of /BD/source owner of file */
	 {
		if(stat(dir,&stbuf) == -1)
		{
		 if (efd=fopen(errorlog,"a"))
			{fprintf(efd,"***CASSI REPORTS ERROR: can't write to BD/source : %s\n",pname);
			 fclose(efd);
			}
			fatal("Cassi BD/source not writeable\n");
		}
		chmod(holdfred,0666);
		chown(holdfred,(int)stbuf.st_uid,(int)stbuf.st_gid); 
	}
	 return(1);
}

/*
*
*verif(cmr,fred) calls the verification prog and returns 0 if failed
* 
*/

verif(cmr,fred)
	char *cmr,*fred;
	{
	int res;
	char *cmrpass[2];
	/* if length of cmr number not = MAXLENCMR error 
	*    all cmr numbers are 12 characters long 
	*/
	if (strlen(cmr) != MAXLENCMR)
		{
			return(0);
		}
	cmrpass[0] = cmr;
	cmrpass[1] = NULL;
	res=sweep(SEQVERIFY,fred,NULL,'\n',WHITE,40,cmrpass,NULL,NULL,
		 (int(*)()) NULL, (int (*)()) NULL);
	if(res != FOUND)
		{
		 return(0);
		}
	else
		{
		 return(1);
		}
	}
/*
*
*getinfo(freddy,sys)
*    get the name of the fred file and the system name 
*/
getinfo(freddy,sys)
	char **freddy,*sys;
	{
	char *h;
	extern int errno;
	extern char *gf();
	struct stat buf;
	FILE *fd;
	*freddy=gf(sys);
	if(!(**freddy))
		{
		 printf("got to bad FRED file %s\n",*freddy);
		 return(0);
		}
	 if(stat(*freddy,&buf))
		{
		return(0);
		}
	 return(1);
	}

/*

@(#)SCCSLOC=/sccs/src/cmd/sccs/lib/s.deltack.c' 

*/
static char Sccsid[] = " @(#)deltack.c	1.11	DELTA DATE 6/10/82	EXTR DATE 6/14/82";
/*

@(#)SCCSLOC=/sccs/src/cmd/sccs/lib/s.deltack.c' 

*/
