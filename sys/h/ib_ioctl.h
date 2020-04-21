/* ioctl codes for gpib-796 driver */

/* maps to ioctl codes for ib driver */
# define ibiocode(n)		('i'<<8|(n))

/* get / set node parameters */
# define IBIOGETNODE		ibiocode('a')
# define IBIOSETNODE		ibiocode('b')

/* ib driver's per-node system integrator info */
struct ibnode
{
    short n_flags;
#	define IBN_VALID	(1<<0)	/*valid entry*/
#	define IBN_SC		(1<<1)	/*is SC*/
#	define IBN_AINIT	(1<<2)	/*do autoinit*/
#	define IBN_NOTRI	(1<<3)	/*not tristate*/
#	define IBN_SWAB		(1<<5)	/*byte-reverse*/
#	define IBN_PPE		(1<<7)	/*can ppoll*/
#	define IBN_PPC		(1<<8)	/*remote configurable ppr*/
#	define IBN_SRQ		(1<<10)	/*can SRQ*/

    u_char n_tag1;		/*gpib address*/
    u_char n_tag2;		/*not used*/

    u_char n_pollstat;		/*reply from last poll*/
    u_char n_idleresp;		/*spoll replies - not used*/
    u_char n_erroresp;
    u_char n_talkresp;
    u_char n_lstnresp;
    u_char n_tctlresp;

    u_char n_ppr;		/*	- ppoll resp*/

    u_char n_talkslot;		/*presumed talker when listening*/
    short n_lstnmap;		/*presumed listeners when talking*/
};

/* struct used by IBIOGETNODE and IBIOSETNODE */
struct sgnode
{
    u_char slotno;		/*node number*/
    struct ibnode node;		/*node info*/
};


/* take CIC */
# define IBIOTAKECTL		ibiocode('m')

/* pass CIC to given node */
# define IBIOPASSCTL		ibiocode('l')

/* lock / unlock */
# define IBIOLOCK		ibiocode('n')

/* ppoll config given nodes */
# define IBIOPPC		ibiocode('o')
# define IBIOPPU		ibiocode('c')

/* return poll responses */
# define IBIOPOLL		ibiocode('p')

/* raise SRQ */
# define IBIOSRQ		ibiocode('q')

/* move to / from started state */
# define IBIOSTART		ibiocode('t')

/* get events */
# define IBIOGETEV		ibiocode('r')

/* (re-)initialise driver variables */
# define IBIOINIT		ibiocode('w')

/* various debug */
# define IBIOPOKE		ibiocode('u')
# define IBIOPEEK		ibiocode('v')
# define IBIODEBUG		ibiocode('x')
# define IBIOINTR		ibiocode('z')
# define IBIOFLUSH		ibiocode('f')
# define IBIOLISTEN		ibiocode('g')
# define IBIOTALK		ibiocode('h')
# define IBIOREN		ibiocode('k')
# define IBIOGTL		ibiocode('j')
# define IBIOGETCONN		ibiocode('d')
# define IBIOSETCONN		ibiocode('e')
# define IBIOCUTOFF		ibiocode('i')
# define IBIOTEST		ibiocode('s')

/* struct for IBIOPOKE */
struct poke
{
    int f;
    int v;
};

/* struct for IBIOLISTEN and IBIOTALK */
struct ioreq
{
    char *addr;
    unsigned count;
    int done;
    int async;
};

/* struct used by IBIOGETCONN and IBIOSETCONN */
struct tlist
{
    u_char talkslot;		/*who talks when conn is read*/
    int listenmap;		/*who listens when conn is written*/
};
