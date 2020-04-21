char _Origin_[] = "System V";

/* Format and interpret the error log file */
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/var.h>
#include <sys/cpuid.h>
typedef long            mem_t;
typedef long            paddr_t;
#include <sys/map.h>
#include <sys/elog.h>
#include <sys/err.h>
#include <sys/erec.h>
#include <time.h>

#undef major
#undef minor
#define dysize(x)	(((x)%4)? 365: 366)
#define writout()       ((page<=limit) && (mode==PRINT))
#define major(x)        (int)((unsigned)(x>>8)&0377)
#define minor(x)        ((int)x&0377)
#define araysz(x)       ((sizeof(x)/sizeof(x[0]))-1)
#define readrec(x)      (fread(&ercd.ebb.block,\
                        (e_hdr.e_len - sizeof(struct errhdr)),1,x) )

#define INCEPT "0301080077"      /* roughly the inception of
					error logging [mmddhhmmyy] */
#define WDLEN 16
#define MINREC 8
#define MAXREC 74
#define MAXLEN 66
#define MAXSTR 40
#define NMAJOR 16
#define NLUNIT 9
#define PGLEN 60
#define MBAREG 5
#define MEM  NMAJOR+1
#define INT  NMAJOR+2
#define YES 1

#define NO 0
#define PRINT 1
#define NOPRINT 0
#define DSEC 3  /* 2**DSEC is the number of logical
                        partitions on RP03/4/5/6 as defined
                        in the device driver. */

/* NMAJOR devices of NLUNIT possible logical units */
struct sums {
        long    soft;
        long    hard;
        long    totalio;
        long    misc;
        long    missing;
} sums[NMAJOR][NLUNIT];

union ercd {
        struct  estart start;
        struct  eend end;
        struct  etimchg timchg;
        struct  econfchg confchg;
        struct  estray stray;
        struct  eparity parity;
        struct  eb {
                struct  eblock block;
                char cregs[1024];
        } ebb;
} ercd;

struct errhdr e_hdr;

int dmsize[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int MAJ;
int MIN;
int page=1;
int print;
int mode = NOPRINT;
int line;
int n = 0;
int aflg;
int dflg;
int fflg;
int Unix = 1;
int parsum;
int straysum;
int limit = 10000;
long optdev = 0;
long readerr = 0;
FILE    *file;
time_t  atime;
time_t  stime = 0L;
time_t  etime = 017777777777L;
time_t  fftime = 017777777777L;
time_t  ltime = 0L;
char interp [MAXSTR];
char choice [MAXSTR];
long tloc;
struct regs {
        char *regname;
        char *bitcode [WDLEN];
};

char *htime[20];
char *header = "SYSTEM ERROR REPORT";
char *hd1 = "System Error Report - Selected Items";
char *hd2 = "Summary Error Report";

/* Array order directed by MMR3 */
char *msg[] = {
        "User D Space Enabled",
        "Supervisor D Space Enabled",
        "Kernel D Space Enabled",
        0,
        "22 bit mapping Enabled",
        "UNIBUS MAP relocation Enabled",
        0
};


char *lines [] = {
/* 0*/  "\n",
/* 1*/  "%s     ERROR LOGGED ON   %s\n",
/* 2*/  "       Physical Device                  %u\n",
/* 3*/  "       Logical Device                   %d (%2.2o)\n",
/* 4*/  "       Device Address                   ",
/* 5*/  "       Retry Count                      %u\n",
/* 6*/  "       Error Diagnosis                  %s\n",
/* 7*/  "       Simultaneous Bus Activity  ",
/* 8*/  "       Registers at Error time\n",
/* 9*/  "               %s          ",
/*10*/  "       Physical Buffer Start Address               0x%.8X\n",
/*11*/  "       Transfer Size in Bytes                     %11u\n",
/*12*/  "       Type of Transfer                              %8s\n",
/*13*/  "       Block No. in Logical File System      %16ld\n",
/*14*/  "       I/O Type                                      %8s\n",
/*15*/  "       Cylinder Requested              %11u\n",
/*16*/  "       Track Requested                 %11u\n",
/*17*/  "       Sector Requested                %11u\n",
/*18*/  "       Statistics on Device to date:\n",
/*19*/  "               No. of R/W Operations          %16ld\n",
/*20*/  "               No. of Other Operations        %16ld\n",
/*21*/  "               No. of Unrecorded Errors            %11u\n",
/*9b*/  "%o      ",
/*23*/  "       Sector Requested                  %4u-%u\n",
/*24*/  "       Unibus Map Utilization?                            %3.3s\n",
                0
                };

char *xlines [] = {
/* 0*/  "\n\nDEVICE CONFIGURATION CHANGE   - %s\n",
/* 1*/  "       DEVICE: %s - %s\n",
/* 2*/  "\n\nSTRAY INTERRUPT on %s\n",
/* 3*/  "       For Controller at - 0x%-.8X\n",
/* 4*/  "       At Location     ",
/* 5*/  "\n\nMEMORY PARITY ERROR at %s\n",
/* 6*/  "       Memory Address of Error - 0x%X\n\n",
/* 7*/  "                MSER        %.6o",
/* 8*/  "                MSCR        %.6o",
/* 9*/  "\n\nTIME CHANGE ***** FROM %s",
/*10*/  "\t\t   TO  %s \n\n\n\n",
/*11*/  "\nERROR LOGGING SYSTEM SHUTDOWN - %s\n\n\n",
/*12*/  "\nERROR LOGGING SYSTEM STARTED - %s \n",
/*13*/  "\n\n   System Profile:\n\n",
/*14*/	"	    %s\n",
/*15*/	"	        for %s Systems\n",
/*16*/  "	    Processor: %s\n",
/*17*/	"	    Memory Management Unit: %s\n\n",
/*18*/	"	    System:  %s\n",
/*19*/	"	    Node:    %s\n",
/*20*/	"	    Release: %s\n",
/*21*/	"	    Version: %s\n\n",
/*22*/  "	    Configured with:\n",
/*23*/	"		%12s (major device number %d)\n",
/*24*/	"	Configuration parameters:\n",
/*25*/	"	    CPU clock frequency: %d Hz\n",
/*26*/	"	    Block size: %d bytes\n",
/*27*/	"	    Page size: %d bytes\n",
/*28*/	"	    Number of buffers: %d\n",
/*29*/	"	    Maximum number of mounted systems: %d\n",
/*30*/	"	    Maximum number of inodes: %d\n",
/*31*/	"	    Maximum number of open files: %d\n",
/*32*/	"	    Maximum number of simultaneous file locks: %d\n",
/*33*/	"	    Maximum number of processes in entire system: %d\n",
/*34*/	"	    Maximum number of processes per user: %d\n",
/*35*/	"	    Maximum number of simultaneous phys() calls: %d\n",
/*36*/	"	    Maximum number of shared text processes: %d\n",
/*37*/	"	    Boundary for data section of shared text: 0x%X\n",
/*38*/	"	    User program origin: 0x%X\n",
/*39*/	"	    Start of user stack: 0x%X\n",
        0
};

char *sumlines [] = {
/* 0*/  "\n\n",
/* 1*/  "%s     UNIT  %d   \n",
/* 2*/  "                       Hard Errors             -   %ld\n",
/* 3*/  "                       Soft Errors             -   %ld\n",
/* 4*/  "                       Total I/O Operations    -   %ld\n",
/* 5*/  "                       Total Misc. Operations  -   %ld\n",
/* 6*/  "                       No. of Errors Missed    -   %ld\n",
/* 7*/  "            Total Read Errors                  -   %ld\n",
/* 8*/  "            Total Memory Parity Errors         -   %d\n",
/* 9*/  "            Total Stray Interrupts             -   %d\n",
/*10*/  "       Date of Earliest Entry: %s",
/*11*/  "       Date of Latest   Entry: %s",
/*12*/  "       Error Types: %s\n",
/*13*/  "       Limitations: ",
/*14*/  "                    ",
0
};


char *ctime();
char outbuf[BUFSIZ];

main (argc,argv)
char *argv[];
int argc;
{
        register i,j;

	aflg = dflg = fflg = 0;
        print = NO;
	/* initialize table of device names */
	initdevnames();
        while (--argc>0 && **++argv =='-') {
                switch (*++*argv) {
                case 's':       /* starting at a specified time */
                        header = hd1;
                        if((--argc <=0) || (**++argv == '-'))
				error("Date required for -s option",
								(char *)NULL);
                        if(gtime(&stime,*argv))
				error("Invalid Start time",*argv);
                        break;
                case 'e':       /* ending at a specified time */
                        header = hd1;
                        if((--argc<=0) || (**++argv =='-'))
				error("Date required for -e option\n",
								(char *)NULL);
                        if(gtime(&etime,*argv))
				error("Invalid End time.",(char *)NULL);
                        break;
                case 'a':       /* print all devices*/
                        aflg++;
                        mode = PRINT;
                        break;
                case 'p':       /* limit total no. of pages */
                        if((--argc<=0) || (**++argv == '-'))
                                error("Page limit not supplied.\n",
								(char *)NULL);
                        limit = atoi(*argv);
                        break;
                case 'f':       /* fatal errors */
                        header = hd1;
                        fflg++;
                        break;

                default:	/* -dev, -int and -mem */
                        if(j=encode(*argv)) {
                                optdev = (optdev |= j);
                                dflg++;
                                header = hd1;
                                mode = PRINT;
                                if(strlen(choice)) concat(",",choice);
                                concat(*argv,choice);
                                }
                        else
				fprintf(stderr,"errpt: %s?\n",argv);
                }
        }
	/* initialize table of sums */
        for(i=0;i<NMAJOR;i++) {
                for(j=0;j<NLUNIT;j++) {
                        sums[i][j].hard = 0;
                        sums[i][j].soft = 0;
                        sums[i][j].totalio = 0;
                        sums[i][j].misc = 0;
                        sums[i][j].missing = 0;
                }
        }
        parsum=0;
        straysum=0;
/*
        if(gtime(&atime,INCEPT))
		error("Invalid INCEPT time",INCEPT);
*/
	setbuf(stdout,outbuf);
        if (argc ==0)
                report("/usr/adm/errfile");
        else while(argc--) 
                report(*argv++);
        printsum();
        putft();
	fflush(stdout);
        exit(0);
}


/* list of device names, by major device number from config file */
static	char	devstr[256];
static	char	*dev[NMAJOR+1];	/* a list of pointers into devstr */

initdevnames()
{
	register char	*filename;	/* the dev file */
	register FILE	*devtyp;	/* stream from devtype file */
	register char cc;
	register char *cp,*dp = devstr;
	register int ii;
	int typind,bmaj,tt;
	char ss[20],devname[20];
	extern	char	*getenv();
	extern  char	*fgets();
	extern	FILE	*fopen();
	
	/* get the list of devices in the system */
	for (ii=0; ii < NMAJOR; ii++)
	    {
		/* init device strings to point somewhere */
		dev[ii] = "unknown";
	    }
	dev[NMAJOR] = 0;
	filename = getenv("MASTER");	/* list of devices in system */
	if ((!filename) || (!*filename))
	    {
		filename = "/etc/master";
	    }
	if ((devtyp = fopen(filename,"r")) == NULL)
	    {
		fprintf(stderr,
	"errpt: Could not open %s for configuration of devices in system\n",
				filename);
		exit(-1);
	    }

	/* look in config master for name of block devices */
	do  {
		fscanf(devtyp,"%s",devname);
		if (*devname != '$')	/* test for end of part 1 */
		    {
			if (*devname != '*')	/* test for comment */
			    {
				/* it is a device line */
				fscanf(devtyp,"%d %o %o %s %d %d",
						&tt,&tt,&typind,ss,&tt,&bmaj);
				if (typind & 010) /* test for block dev type */
				    {
					/* copy string into our local buffer */
					cp = devname;
					/* remember where in buffer it is */
					dev[bmaj] = dp;
					do  {
						*dp++ = *cp++;
					} while (*cp);
					*dp++ = '\0';
				    }
			    }
			/* scan to end of line */
			do  {
				cc = getc(devtyp);
			} while (cc != '\n');
		    }
	} while (*devname != '$');
	fclose(devtyp);
}
report(fp)
char *fp;
{

        if((file = fopen(fp,"r"))== NULL) {
                fprintf(stderr,"errpt: Cannot open %s as error file\n",fp);
                return;
        }
        inithdng();
        if(writout())
		puthead(header);
        putdata();
        if(writout())
		putft();
}
putdata()
{
        while(fread(&e_hdr.e_type,sizeof(struct errhdr),1,file)) {
newtry:
                switch(e_hdr.e_type) {
                
                case E_GOTS:
                        setime();
                        up();
                        break;

                case E_GORT:
                        setime();
                        up();
                        break;

                case E_STOP:
                        setime();
                        down();
                        break;

                case E_TCHG:
                        setime();
                        timecg();
                        break;

                case E_BLK:
                        setime();
                        blk();
                        break;
                
                case E_STRAY:
                        setime();
                        stray();
                        break;

                case E_PRTY:
                        setime();
                        party();
                        break;

                case E_CCHG:
                        cconfig();
                        setime();
                        break;
                default:
                        fprintf(stderr,"%d\n",e_hdr.e_len);
                        fprintf(stderr,"%d\n",e_hdr.e_type);
                        readerr++;
                        if(recov()) {
                                goto newtry;
                        }
                        fprintf (stderr,"Unrecovered read error.\n");
                }
        }

        return;
}
/* System Startup Record */

up()
{
	register int ii;
	register char *name;
	register int psz;
	struct var vv;
	static char *cpunames[] = {
		/*		  0 */	"unknown",
		/*CPU_MC68000     1 */	"Motorola 68000",
		/*CPU_NA16000     2 */	"National 16000",
		/*CPU_VAX11780    3 */	"Vax 11/780",
		/*CPU_VAX11750    4 */	"Vax 11/750",
		/*CPU_VAX11730    5 */	"Vax 11/730",
		/*CPU_PDP1170     6 */	"PDP 11/70",
					0 };
	static char *cpuvers[] = {
		/*		  0 */	"unknown",
		/*VER_MC68000     1 */	"68000",
		/*VER_MC68010     2 */	"68010",
		/*VER_MC68020     3 */	"68020",
					0 };
	static char *mmutypes[] = {
		/*		  0 */	"unknown",
		/*MMU_NONE        1 */	"none",
		/*MMU_SUN         2 */	"Sun",
		/*MMU_68451       3 */	"68451",
		/*MMU_PIX         4 */	"Pix",
		/*MMU_APPLE       5 */	"Apple",
					0 };

        if (!readrec(file)) {
                fprintf(stderr,"%ld\n",ercd.start.e_name.release);
                fprintf(stderr,"%s %s\n",ercd.start.e_name.release,
						ercd.start.e_name.sysname);
                readerr++;
                return;
        }
        if(writout()) {
                need(19);
		/* get the system parameter info */
		if (uvar(&vv) < 0)
		    {
			fprintf(stderr,"Could not perform uvar system call\n");
			/* simulate var system call with arbitary values */
			vv.v_buf = 32;
			vv.v_call = 16;
			vv.v_inode = 65320;
			vv.v_file = 32;
			vv.v_mount = 16;
			vv.v_proc = 256;
			vv.v_text = 64;
			vv.v_maxup = 16;
			vv.v_flock = 8;
			vv.v_phys = 4;
			vv.v_txtrnd = 64*1024;
			vv.v_bsize = 1024;
			vv.v_hz = 16000000;
			vv.v_pageshift = 11;
			vv.v_ustart = 0x20000;
			vv.v_uend = 0x3ffd00;
			vv.v_cputype = CPU_MC68000;
			vv.v_cpuver = VER_MC68000;
			vv.v_mmutype = MMU_SUN;
		    }
		else{
			printf(xlines[12],ctime(&e_hdr.e_time));
			printf(xlines[13]);
			printf(xlines[14],"SGI");
			if (vv.v_cputype)
			    {
				printf(xlines[15],cpunames[vv.v_cputype]);
			    }
			if (vv.v_cpuver)
			    {
				printf(xlines[16],cpuvers[vv.v_cpuver]);
			    }
			if (vv.v_mmutype)
			    {
				printf(xlines[17],mmutypes[vv.v_mmutype]);
			    }
			if ((name = ercd.start.e_name.sysname) && *name)
			    {
				printf(xlines[18],ercd.start.e_name.sysname);
			    }
			if ((name = ercd.start.e_name.nodename) && *name)
			    {
				printf(xlines[19],ercd.start.e_name.nodename);
			    }
			if ((name = ercd.start.e_name.release) && *name)
			    {
				printf(xlines[20],ercd.start.e_name.release);
			    }
			if ((name = ercd.start.e_name.version) && *name)
			    {
				printf(xlines[21],ercd.start.e_name.version);
			    }
			printf(xlines[22]);
			for (ii=0; ii < NMAJOR; ii++)
			    {
				if (strcmp(dev[ii],"unknown"))
				    {
					printf(xlines[23],dev[ii],ii);
				    }
			    }
			/* now start the uvar stuff */
			printf(lines[0]);
			printf(xlines[24]);
			printf(xlines[25],vv.v_hz);
			printf(xlines[26],vv.v_bsize);
			psz = 1;
			for (ii=0; ii < vv.v_pageshift; ii++)
				psz *= 2;
			printf(xlines[27],psz);
			printf(xlines[28],vv.v_buf);
			printf(xlines[29],vv.v_mount);
			printf(xlines[30],vv.v_inode);
			printf(xlines[31],vv.v_file);
			printf(xlines[32],vv.v_flock);
			printf(xlines[33],vv.v_proc);
			printf(xlines[34],vv.v_maxup);
			printf(xlines[35],vv.v_phys);
			printf(xlines[36],vv.v_text);
			printf(xlines[37],vv.v_txtrnd);
			printf(xlines[38],vv.v_ustart);
			printf(xlines[39],vv.v_uend);
		    }
                printf(lines[0]);
        }
}
/* System Shutdown Record */

down()
{
        if(writout()) {
                need(5);
                printf(xlines[11],ctime(&e_hdr.e_time));
        }
}
/* Time Change Record */

timecg()
{
        if(!readrec(file)) {
                readerr++;
                return;
        }
        if(writout()) {
                need(6);
                printf(xlines[9],ctime(&e_hdr.e_time));
                printf(xlines[10],ctime(&ercd.timchg.e_ntime));
        }
}
/* Handle a MERT configuration change */
cconfig() 
{

        if (!readrec(file)) {
                readerr++;
                return;
        }
        if(writout()) {
                need(7);
                printf(xlines[0],ctime(&e_hdr.e_time));
                printf(lines[0]);
                printf(xlines[1],dev[ercd.confchg.e_trudev],
                        ercd.confchg.e_cflag?"Attached":"Detached");
        
                printf(lines[0]);
        }
}

/* Stray Interrupt Record */

stray()
{
        if (!readrec(file)) {
                readerr++;
                return;
        }
        if(!wanted()) return;
        if(print==YES) {
                need(7);
                if(page<=limit) {
                printf(xlines[2],ctime(&e_hdr.e_time));
                printf(xlines[3],ercd.stray.e_saddr);
                printf(lines[7]);
                if(ercd.stray.e_sbacty == 0)
			printf("None\n");
                else
			afix(araysz(dev),(unsigned) ercd.stray.e_sbacty,dev);
                }
        }
        straysum++;
}

/* Memory Parity Record */

party()
{
        if (!readrec(file)) {
                readerr++;
                return;
                }
        if(!wanted())  return;
        if(print==YES) {
                need(9);
                if(page<=limit) {
			printf(xlines[5],ctime(&e_hdr.e_time));
			printf(xlines[6],(long)(ercd.parity.e_parreg));
			printf(lines[0]);
                }
        }
        parsum++;
}
/* copies from b1 to b2
   returns address after b1
   may not be on long address boundary when copied from b1,
   avoiding any alignment problems on any machines */
char *
longcopy(b1,b2)
register char *b1,*b2;
{
	register int ii;

	for (ii=0; ii < sizeof(long); ii++)
	    {
		*b2++ = *b1++;
	    }
	return(b1);
}
/* Device Error Record */

blk()
{
        register union ercd *z;
        register int i;
	register char *pp,*p2,*p3;
        int *mbar;
        struct vaxreg *q;
	char nbuf[512];
	long reg_addr;
	long reg_value;

        if (!readrec(file)) {
                readerr++;
                return;
        }
        z = &ercd;
        MAJ=major(ercd.ebb.block.e_dev);
        MIN=minor(ercd.ebb.block.e_dev);
        if(MAJ<0) return;
        if(!wanted()) return;
        /* Increment summary totals */
        
        if(ercd.ebb.block.e_bflags &E_ERROR)
		sums[MAJ][ercd.ebb.block.e_pos.unit].hard++;
        else
		sums[MAJ][ercd.ebb.block.e_pos.unit].soft++;
        sums[MAJ][ercd.ebb.block.e_pos.unit].totalio =
					ercd.ebb.block.e_stats.io_ops;
        sums[MAJ][ercd.ebb.block.e_pos.unit].misc =
					ercd.ebb.block.e_stats.io_misc;
        sums[MAJ][ercd.ebb.block.e_pos.unit].missing =
					ercd.ebb.block.e_stats.io_unlog;
        if(print==NO) return;
        need(31+ercd.ebb.block.e_nreg);
        if(page <= limit) {
                printf(lines[0]);
                printf(lines[1],dev[MAJ],ctime(&e_hdr.e_time));
                printf(lines[2],ercd.ebb.block.e_pos.unit);
                printf(lines[3],MIN,MIN);
                printf(lines[5],ercd.ebb.block.e_rtry);
                printf(lines[6],ercd.ebb.block.e_bflags&E_ERROR?
                        "Unrecovered":"Recovered");
                printf(lines[7]);
                if(ercd.ebb.block.e_bacty == 0)
			printf("None\n");
                else
			afix(araysz(dev),(unsigned) ercd.ebb.block.e_bacty,dev);
                printf(lines[0]);
		if (ercd.ebb.block.e_nreg > 0) {
			printf(lines[8]);
			pp = &ercd.ebb.cregs[0];
			for(i=0;i<ercd.ebb.block.e_nreg;i++) {
				/* copy out the number values */
				pp = longcopy(pp,&reg_addr);
				pp = longcopy(pp,&reg_value);
				/* copy out the corresponding strings */
				p2 = nbuf;
				while (*pp)
				    {
					*p2++ = *pp++;
				    }
				/* copy the terminating null too */
				*p2++ = '\0';
				pp++;
				p3 = p2;/* p3 keeps this location for later */
				while (*pp)
				    {
					*p2++ = *pp++;
				    }
				*p2++ = '\0';
				pp++;
				printf("\t  0x%-8X %-20s %-20s 0x%-8X\n",
						reg_addr,nbuf,p3,reg_value);
			}
			printf(lines[0]);
		}
		printf(lines[10],ercd.ebb.block.e_memadd);
                printf(lines[11],ercd.ebb.block.e_bytes);
                i=ercd.ebb.block.e_bflags;
                printf(lines[12],
                (i&E_NOIO)?"No-op":((i&E_READ)?"Read":"Write"));
                printf(lines[13],ercd.ebb.block.e_bnum);
                if(Unix) printf(lines[14],
                        i&E_PHYS? "Physical":"Buffered");
                else line--;
                /* Not valid in this implementation; the line
                   following is inserted in its place.
                printf(lines[24],i&E_MAP?"Yes":"No");
                */
                line--;
                printf(lines[0]);
                if(ercd.ebb.block.e_pos.cyl != -1) {
                        printf(lines[15],ercd.ebb.block.e_pos.cyl);
			}
                if(ercd.ebb.block.e_pos.trk != -1) {
                        printf(lines[16],ercd.ebb.block.e_pos.trk);
			}
                if(ercd.ebb.block.e_pos.cyl != -1) {
			printf(lines[17],ercd.ebb.block.e_pos.sector);
			}
                printf(lines[0]);
                printf(lines[18]);
                printf(lines[19],ercd.ebb.block.e_stats.io_ops);
                printf(lines[20],ercd.ebb.block.e_stats.io_misc);
                printf(lines[21],ercd.ebb.block.e_stats.io_unlog);
                printf(lines[0]);
        }
}
cleanse(p,q)
        register char *p;
        register int q;
{
        while(q--)
                *p++='\0';
}

afix(a,b,c)
int a;
unsigned b;
char **c;
{
        register i;
        cleanse(interp,MAXSTR);
        for(i=0;i<a;i++)  {
                if((b & (1<<i)) && (*c[i]))    {
                        if((strlen(c[i])+strlen(interp))>=MAXSTR) {
                                concat(",",interp);
                                printf("          %s\n\t\t\t",interp);
                                line++;
                                cleanse(interp,MAXSTR);
                        }
                        else {
                                if(strlen(interp)) concat(",",interp);
                        }
                        concat(c[i],interp);
                }
        }
        printf("      %s\n",interp);
}
puthead(h)
char *h;
{
        printf("\n\n   %s   Prepared on %s     Page  %d\n\n\n\n",
                h,htime,page);
        line = 6;
}
inithdng()
{

        time(&tloc);
        strcpy(htime,ctime(&tloc));
}
putft()
{
        while (line++<MAXLEN) {
                putchar('\n');
        }
        page++;
}
trnpg()
{
        if( line >= MAXLEN) page++;
        else putft();
        if(page<=limit) puthead(header);
}
need(a)                 /* acts like ".ne" command of nroff */
int a;
{
        if( line>(PGLEN-a)) trnpg();
        line += a;
}
gtime(tptr,pt)
char *pt;
time_t  *tptr;
{
        register int i;
        register int y, t;
        int d, h, m;
	char *env;
        long nt;

        t = gpair(pt++);
        if(t<1 || t>12)
                return(1);
        pt++;
        d = gpair(pt++);
        if(d<1 || d>31)
                return (1);
        pt++;
        h = gpair(pt++);
        if(h == 24) {
                h = 0;
                d++;
        }
        pt++;
        m = gpair(pt++);
        if(m<0 || m>59)
                return (1);
        pt++;
        y = gpair(pt++);
        if (y<0) {
                time(&nt);
		y = localtime(&nt)->tm_year;
        }
        *tptr = 0;
        y += 1900;
        for(i=1970; i<y; i++)
                *tptr += dysize(i);
	i = t;
        /* Leap year */
        if (dysize(y)==366 && t >= 3)
                *tptr += 1;
        while(--t)
                *tptr += dmsize[t-1];
        *tptr += (d-1);
        *tptr = (*tptr *24) + h;
	/* get the time zone */
	if ((env = getenv("TZ")) == 0) {
		/* time zone variable set */
		*tptr += env[3] - '0';
		/* if time zone variable not set... assume zero */
		}
        *tptr = (*tptr*60) + m;
	if (localtime(tptr)->tm_isdst)
                *tptr -= 60;
        *tptr *= 60;
        return(0);

}
gpair(pt)
char *pt;
{
        register int c, d;
        register char *cp;

        cp = pt;
        if(*cp == 0)
                return(-1);
        c = (*cp++ - '0') * 10;
        if (c<0 || c>100)
                return(-1);
        if(*cp == 0)
                return(-1);
        if ((d = *cp++ - '0') < 0 || d > 9)
                return(-1);
        return (c+d);
}

wanted ()
{
        /* Starting - ending limitations? */
        if(e_hdr.e_time < stime ) return (0);
        if(e_hdr.e_time > etime) return (0);
        /* Only fatal error flag? */
        if((fflg) && (e_hdr.e_type==E_BLK) &&
		    !(ercd.ebb.block.e_bflags&E_ERROR))
		return(0);
        /* Stray interrupts or parity errors to be considered */
        if((aflg) || ((e_hdr.e_type==E_STRAY)&&(optdev&(1<<INT))) ||
                ((e_hdr.e_type==E_PRTY)&&(optdev&(1<<MEM)))) {
                print=YES;
                return (1);
                }
        /* Device chosen for consideration or printing? */
        if(dflg == 0) {
                print=NO;
                return(1);
                }
        if((1<<MAJ)&optdev) {
                print=YES;
                return(1);
                }
        print=NO;
        return(0);
}
error(s1,s2)
char *s1, *s2;
{
        fprintf(stderr,"errpt:%s %s \n",s1,s2);
        exit(16);
}


recov()
{
        struct errhdr *p,*q;
        int i;
        for(;;) {
                p = q = &e_hdr;
                q++;
                for(i=0;i<((sizeof(struct errhdr) /2)-1);i++)
                        *p++ = *q++;
                fread(p,2,1,file);
                if(feof(file))return(0);
                if(valid()) return (1);
        }
}
valid()
{
        switch(e_hdr.e_type) {
                default:
                        return(0);
                case E_GOTS:
                case E_GORT:
                case E_STOP:
                case E_TCHG:
                case E_BLK:
                case E_STRAY:
                case E_CCHG:
                case E_PRTY:
        if((e_hdr.e_len <MINREC) ||
                 (e_hdr.e_len > MAXREC) ) return (0);
        if((e_hdr.e_time < atime) ||
                 (e_hdr.e_time > tloc))  return(0);
        return (1);
	}
}
printsum()
{
        int i;
        header = hd2;
        page = 1;
        puthead(header);
        need(11);
        printf(sumlines[12],choice[0]?choice:"All");
        printf(sumlines[13]);
        if(stime) {
                printf("On or after %s",ctime(&stime));
                printf(sumlines[14]);
        }
        else line--;
        if(etime!=017777777777L) {
                printf("On or before %s",ctime(&etime));
                printf(sumlines[14]);
        }
        else line--;
        if(fflg) {
                printf("Only fatal errors are printed.\n");
                printf(sumlines[14]);
        }
        else line--;
        if(limit != 10000) printf("Printing suppressed after page %d.\n",limit);
        else line--;
        printf(lines[0]);
        printf(sumlines[10],ctime(&fftime));
        printf(sumlines[11],ctime(&ltime));
        printf(lines[0]);
        if(readerr) printf(sumlines[7],readerr);
        else printf(lines[0]);
        printf(lines[0]);
        if((optdev&(1<<MEM)) || !(dflg) || (aflg)) {
                need(3);
                printf(lines[0]);
                printf(sumlines[9],straysum);
                printf(lines[0]);
                }
        if((optdev&(1<<INT)) || !(dflg) || (aflg)) {
                need(3);
                printf(lines[0]);
                printf(sumlines[8],parsum);
                printf(lines[0]);
                }
        if ((dflg == 0)||(aflg)) {
                for(i=0;i<NMAJOR;i++) (prsum(i)); }
        else
        for(i=0;i<NMAJOR;i++)
                if(optdev & (1<<i)) prsum(i);
        if(line == 7) printf("No errors for this report\n");
}

prsum(i)
register int i;
{
        register int j;

        for(j=0;j<NLUNIT;j++) {
                if(sums[i][j].totalio) {
                        need(10);
                        printf(sumlines[1],dev[i],j);
                        printf(sumlines[0]);
                        printf(sumlines[2],sums[i][j].hard);
                        printf(sumlines[3],sums[i][j].soft);
                        printf(sumlines[4],sums[i][j].totalio);
                        printf(sumlines[5],sums[i][j].misc);
                        printf(sumlines[6],sums[i][j].missing);
                        printf(sumlines[0]);
                }
        }
}
/* Associate typed name with a specific bit in "optdev" */
encode(p)
char  *p;
{
	register int ii;

	if (!strcmp("int",p))
		return(1<<INT);
	if (!strcmp("mem",p))
		return(1<<MEM);
	for (ii=0; ii <= NMAJOR; ii++)
	    {
                if (!strcmp(dev[ii],p))
                        return(1<<ii);
        }
        return(0);
}

concat(a,b)
        register char *a,*b;
{
        while (*b) b++;
        while (*b++ = *a++);
}
setime()
{
        if(e_hdr.e_time < fftime)
        fftime = e_hdr.e_time;
        if(e_hdr.e_time > ltime)
        ltime = e_hdr.e_time;
}
