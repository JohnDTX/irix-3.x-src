/* resource struct */
typedef struct resource {
	int flag;
	int wanted;			/* wakeup needed */
	int id;				/* timeout id */
	caddr_t chan;			/* arg to sleep on */
	int timeout_val;		/* used by spinfor() */
} resource;

/*
 * Bits in resource flag
 */
	/* availibility bits */
#define RES_BIT_BUSY	0x1
#define RES_BIT_LCK	0x2
	/* error bits */
#define RES_BIT_TO	0x10		/* timeout */
#define RES_BIT_ERR	0x20		/* other error */
#define RES_BITS_ANYERR (RES_BIT_TO|RES_BIT_ERR)

	/* Bits settable by set_res() */
#define RES_SETTABLE_BITS (RES_BIT_LCK|RES_BIT_ERR)

	/* Bits clearable by clear_res() */
#define RES_CLEARABLE_BITS (RES_BIT_BUSY|RES_BIT_LCK|RES_BITS_ANYERR)

/*
 * The following test for resource availability.
 */
#define RES_TAKEN(x)		(((x)->flag) & (RES_BIT_BUSY|RES_BIT_LCK))
#define RES_BUSY_IGNLCK(x)	(((x)->flag) & RES_BIT_BUSY)

/*
 * The following test the resource error/timeout status.
 */
#define RES_TIMED_OUT(x)	(((x)->flag) & RES_BIT_TO)
#define RES_ERR(x)		(((x)->flag) & RES_BIT_ERR)
#define RES_ANYERR(x)		(((x)->flag) & RES_BITS_ANYERR)
#define RES_ERRNO(x)		(RES_ANYERR(x) ? EIO : 0)


#define TIME_FOREVER	(60*2)		/* default #secs for spin loop */
#ifdef juniper
#define SPINS_PER_SEC	700000		/* tested in spinfor on an IRIS */
#else
#define SPINS_PER_SEC	190000		/* tested in spinfor on a 68010 */
#endif

#define CANSLEEP	1
#define NOSLEEP		0

#define WAITFOR(x, pri, cansleep)	\
	((cansleep) ? sleepfor(x, pri) : spinfor(x))


int set_res(), busy_res(), sleepfor(), spinfor(), sleepmax();
void clear_res();

