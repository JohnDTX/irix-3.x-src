/*
 * cymon.h -- include file for cypress monitor program.
 *
 * author -- Cathy Privette
 *
 * date -- 11/1/85
 *
 */

/*
 * Note: cypstat.h must be included before this file.
 */

#define MAXLINES 1                  /* the maximum number of serial lines */
                                    /* that can be monitored at once      */
#define MAXPLOTS 49                 /* max # of plots per log record graph */ 
#define MAXWINDOWS 4                /* the no. of available windows  */
#define NMSZ   25                   /* the size of a host name      */
#define NINTV  1		    /* no. of intervals over which   */
				    /* to compute the avg. stats.    */
#define GLOBAL 0                    /* site-wide statistics window   */
#define LINE   1                    /* active line statistics window */
#define TOTAL  2                    /* active line totals window     */
#define HELP   3                    /* help function window          */
#define NOTSET -1                   /* when a yloc is not in use */
#define NOTCONF 'N'                 /* when a serial line is not configured */
#define REQUESTSIZE 25              /* maximum size of a request   */
                                    /* to change windows           */

static char *mptysb[] = {           /* log record types descriptions */
    "Pkts received from IP",
    "Pkts from remote implet",
    "Pkts passed to IP",
    "Pkts sent to remote implet",
    "Device silo overflow",
    "Direct pkts sent",
    "Flood pkts sent",
    "RPF pkts sent",
    "Direct pkts received",
    "Flood pkts received",
    "RPF pkts received",
    "Pkts passed to IP",
    "Pkts received from IP",
    "Status of line",
    "Pkts dropped",
    "Queue length",
    "Line silo overflow",
    "Tty busy          :",
    "Percent tty idle",
    "Characters sent   :",
    "     Characters received   :",
    "Characters sent :",
    "Characters received :",
    "Avg chars sent/sec:",
    "Avg pkts sent/sec :",
    "Avg chars/pkt sent:",
    "     Avg chars received/sec:",
    "     Avg pkts received/sec :",
    "     Avg chars/pkt received:",
    "Avg chars received/interrupt",
    "     Avg chars escaped     :",
    "Escape chars sent",
    "NN pkts sent",
    "NN pkts received"

};

struct cylr {             /* cypress log record structure                 */
    int yloc;             /* the y-position to graph the packet counts    */
                          /* with the corresponding attributes.           */
    int old_cpkt;         /* old count of packets so can update           */
                          /* monitor plots without redrawing entire plot  */
    int new_cpkt;         /* new count packets                            */
    float new_cpktf;      /* new count packets when we want a float       */
    int ln;               /* logical line number, 0 for global stats      */
    int fwindow;          /* flag which indicates which window to display */
                          /* this log record in                           */
};

int globalstats(), linestats(), helpwindow(), totalstats();

static struct windows {   /* structure describing each window     */
    char *sbTitle;        /* title of window                      */
    int fwindow;          /* flag for which window                */
    int nargs;            /* number of arguments for function to  */
                          /* create this window                   */
    char *sbDescription;  /* what this window displays            */
    int (*pfc)();         /* function to create this window       */
}  rgwindow[] = {
    {"GLOBAL STATISTICS ", 0, 0, "g - display site global statistics", globalstats},
    {"LINE", 1, 1, "l - display serial line statistics (l must be followed by the logical line # of\n\ta serial line to be monitored.  Ex. l 0)", linestats},
    {"TOTALS", 2, 1, "t - display total statistics (takes the same arguments as l or no argument\n\tto get global totals. Ex. t 0)", totalstats},
    {"MONITOR COMMANDS ", 3, 0, "h or ? - help function", helpwindow}
};

struct avstats {		  /* one set of data to be used   */
				  /* to compute avgs.             */
    int chs;			  /* no. of chars sent            */
    int pkts;			  /* no. of pkts sent		  */
    int chr;			  /* no. of chars received	  */
    int pktr;			  /* no. of pkts received         */
    int escs;			  /* no. of escape chars sent     */
    int sec;	                  /* no. of secs. between getting */
				  /* new data                     */
};

struct cylr *mptypcylr[TYPEMAX];        /* array of ptrs to log records    */
					/* which hold 5 sec data, logical  */
					/* line number this log record     */
					/* applies to and the y-position   */
					/* where this data is printed      */

struct avstats avtab[CLINEMAX][NINTV];  /* table containing data for avgs. */


int maxyloc;                      /* maximum possible y-location  */
                                  /* for graph window             */
int fwindow;                      /* option flag indicating which window */
                                  /* is currently being monitored        */
int fline;                        /* flag which indicates which serial is */
                                  /* being monitored when fwindow = LINE  */
int freset;                       /* flag set to true if using reset totals */
                                  /* set to false if using real totals      */
int newdata;			  /* ptr into avtab where new data should   */
				  /* be stored                              */
char sbh_name[CLINEMAX][NMSZ];	  /* names of hosts on the other end of     */
				  /* each logical line			    */
