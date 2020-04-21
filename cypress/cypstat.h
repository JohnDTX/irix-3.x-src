
/* 
 * cypstat.h -- definitions and structures for statistics passed between the
 *     site being monitored and the monitor program.
 * 
 * author -- Cathy Privette
 *
 * date -- 1/29/86
 */

/* NOTE: sys/dk.h must be included before this file */

/* field types in cystats structure and stats computed from them */
#define CYD_CIPUP             0
#define CYD_CIPLN             1
#define CYD_COPUP             2
#define CYD_COPLN             3
#define CYD_CSILO             4   /* computed global no. of silo overflows */
#define CYL_CPSDIRECT         5
#define CYL_CPSFLOOD          6
#define CYL_CPSRPF            7
#define CYL_CPRDIRECT         8
#define CYL_CPRFLOOD          9 
#define CYL_CPRRPF            10
#define CYL_CPSIP             11
#define CYL_CPRIP             12
#define CYL_FLAGS             13
#define CYL_COPDROP           14
#define CYL_CSEND_IFQLEN      15
#define CYL_CSILO             16
#define CYL_CTPBUSY           17
#define CYL_CTPIDLE           18
#define CYL_CCHS              19
#define CYL_CCHR              20
#define CYD_CCHS              21   /* computed global no. of chars sent     */
#define CYD_CCHR              22   /* computed global no. of chars received */
#define CYL_ACHS	      23   /* computed avg no of chars sent         */
#define CYL_APKTS             24   /* computed avg no of pkts sent          */
#define CYL_ACHPKTS           25   /* computed avg no of chars/pkt sent     */
#define CYL_ACHR              26   /* computed avg no of chars received     */
#define CYL_APKTR             27   /* computed avg no of pkts received      */
#define CYL_ACHPKTR           28   /* computed avg no of chars/pkt received */
#define CYD_ACHINTR	      29   /* computed avg no of chars received/intr*/
#define CYL_APCTESC           30   /* computed avg percent of esc chars sent*/
#define CYL_CESC              31   
#define CYL_CPSNN             32 /* count of nearest neighbor packets */
#define CYL_CPRNN             33 /* count of NN packets recieved */

#define TYPEMAX               34   /* max # of types of log records    */
#define INTERVAL	      5    /* no. of secs between data samples */

struct cystats {
    u_long tv_sec;      	/* time record was made */
    long avenrun[3]; 		/* avg. length of run Q for 1, 5, 15 min. */
    long cputime[CPUSTATES];	/* per interval time spent in ea. state   */
       				/* USER, NICE, SYS, IDLE */

    /* global stats */
    long cyd_cipup;              /* # input packets recieved from above (TCP)*/
    long cyd_cipln;              /* # input packets recieved from lines */
    long cyd_copln;              /* # output packets sent out on lines  */
    long cyd_copup;              /* # output packets sent upward (TCP)  */
    long cyd_crint;	        /* # of times dmf interrupt routine called */

    /* line stats */
    struct ln {
	long cyl_cpsNN;	        /* count of nearest neighbor packets sent */
	long cyl_cpsdirect;	/* count of direct packets sent */
	long cyl_cpsflood;	/* count of flood packets sent */
	long cyl_cpsrpf; 	/* count of RPF packets sent */
	long cyl_cprNN;    	/* count of NN packets recieved */
	long cyl_cprdirect;	/* count of direct packets recieved */
	long cyl_cprflood;	/* count of flood packets recieved */
	long cyl_cprrpf;	        /* count of RPF packets recieved */
	long cyl_cchr;		/* number of characters recieved */
	long cyl_cchs;		/* number of characters sent */
	long cyl_cpsip;	        /* number of packets sent up to IP */
	long cyl_cprip;	        /* number of packets recieved from IP */
	long cyl_csilo;	        /* number of silo overflows on this line */
	long cyl_flags;		/* status of the line */
        long cyl_copdrop;        /* # of dropped forwarded packets */
        long cyl_ctpbusy;        /* # of times tty was active */
        long cyl_ctpidle;        /* # of times tty was idle   */
        long cyl_cesc;	        /* # of chars escaped in packets sent */
        long cyl_csend_ifqlen;   /* len of Q for packets being sent */
	struct in_addr cyl_dest;/* host addr on other end of the line */
        long  cyl_ln;           /* logical serial line # corresponding to */
                                /* to the rest of the data in this struct */
				/* in network order                   */
    } rgln[CLINEMAX];
	
} ;
