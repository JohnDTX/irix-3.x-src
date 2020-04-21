#define TRUE 1
#define FALSE 0
#ifdef sgi
#undef NULL
#endif
#define NULL 0
#define ERROR -1
#define INFINITY 999999
#define MAXENTRIES 100
#define AUXENTRIES 50
#define NAMESIZE   50
#define BASESPEED  9600
#define BASEWT     100
#define CONNECT    3
#define CONNECTFILE "/etc/cypress/lib/connect.dat"
#define LINEFILE   "/etc/cypress/lib/line.dat"
#define HOSTCACHE   "/etc/cypress/lib/hostcache"
#define CYPRESSNET "cypress"

#define New(x) (x *) malloc(sizeof(x))
#define Min(x, y) ((x) < (y) ? (x) : (y))

struct Line {
    char *sbLineName;             /* our name for a leased line */
    int speed;                    /*         baud rate          */
    char *sbProtocol;             /*       line protocol        */
    char *sbBillTo;               /*    who foots the bill      */
    char *sbCircuitNum;           /* ma bells circuit number    */
    };
     
struct AdjList {
    char *sbNodeName;             /*  in the form site-cypress  */
    char *sbNetNumber;            /*       internet number      */
    int wt;                       /* weight associated with this*/
				  /* connection                 */
    struct AdjList *pAdjNext;     /* nodes adjacent to this one */
    };

struct Connect {
    char *sbNodeName;             /*  in the form site-cypress  */
    char *sbNetNumber;            /*       internet number      */
    struct Links *pConnections;   /* list of leased lines that  */
    };                            /* connect to this site       */

/* the Links structure is used generically to form a linked list */
/* pointed to from the Connect structure.  If pointed to by the  */
/* Connect structure, the sb entry is a pointer to a string      */
/* representing our name for a leased line and the w entry is the*/
/* logical interface number for this connection.                 */

struct Links {
    char *sb;                     /* leased line name           */
    int w;                        /* logical interface number   */
    struct Links *pConnections;   /* rest of the list of leased */
    };                            /* lines                      */

struct Links *pLinks;             /* temporary pointer used in  */
				  /* route when allocating a   */
				  /* node                       */

struct AuxAdjNode {               /* structure for auxiliery adj. list  */
    int iAdjNode;                 /* index of  adj. node in AuxAdjTable */
    int fEdge;                    /* flag, TRUE if  edge from  current  */
				  /* node to node represented by        */
				  /* iAdjNode has been traversed.       */
    struct AuxAdjNode *pAuxNext;  /* ptr to the next adj. node.         */
    } ;

struct StackEntry {               /* stack entry for edge stack         */
    int Node1;                    /* index of node on one end of edge   */
    int Node2;                    /* index of node on other end of edge */
    struct StackEntry *pPrevEntry;/* ptr to previous entry in the stack */
    } ;

struct BlkXRef {                  /* struct for cross referencing         */
				  /* articulation pts. to blocks.         */
    int BlockNum;                 /* the no. of the containing block      */
    struct BlkXRef *pNextBlock;   /* ptr to the next block for a art. pt. */
    } ;


struct CypRoute {                 /* contains cnumber-interface pairs */
    char *c_number;               /* for cypress routing table        */
    int Interface;
    };

struct host {
    char *sbNodeName;
    char *sbNetNumber;
};

