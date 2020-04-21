/*
 * static char ID_trekh[] = "@(#)trek.h	1.2";
 */

#define ever (;;)

/* external function definitions */
extern double	franf();	/* floating random number function */
extern double	sqrt();		/* square root */
extern double	sin(), cos();	/* trig functions */
extern double	atan2();	/* fancy arc tangent function */
extern double	log();		/* log base e */
extern double	pow();		/* power function */
extern double	fabs();		/* absolute value function */
extern double	exp();		/* exponential function */

/*********************  GALAXY  **************************/

/* galactic parameters */
# define	NSECTS		10	/* dimensions of quadrant in sectors */
# define	NQUADS		10	/* dimension of galaxy in quadrants */
# define	NINHAB		32	/* number of quadrants which are inhabited */

#define	QUAD	struct quad
QUAD			/* definition for each quadrant */
{
	int	bases;		/* number of bases in this quadrant */
	int	qkling;		/* number of Klingons in this quadrant */
	int	holes;		/* number of black holes in this quadrant */
	int	stars;		/* number of stars in this quadrant */
	int	systemname;	/* starsystem name (see below) */
	int	scanned;	/* star chart entry (see below) */
};
/*  systemname conventions:
 *	1 -> NINHAB	index into Systemname table for live system.
 *	+ Q_GHOST	ghost starsystem.
 *	+ Q_DISTRESS	distressed starsystem.
 *			the index into the Distress table, which will
 *			have the starsystem name.
 *	0		dead or nonexistent starsystem
 *
 *  starchart ("scanned") conventions:
 *	0 -> 999	taken as is
 *	-1		not yet scanned ("...")
 *	1000		supernova ("///")
 *	1001		starbase + ??? (".1.")
 */

/* ascii names of systems */
extern char	*Systemname[NINHAB];

/* quadrant definition */
extern	QUAD		Quad[NQUADS][NQUADS];

/* defines for sector map  (below) */
# define	NTHINGS		9

# define	EMPTY		0
# define	STAR		1
# define	KLINGON		2
# define	BLACKHOLE	3
# define	ENTERPRISE	4
# define	QUEENE		5
# define	INHABIT		6
# define	BASE		7

# define	EM		'.'
# define	ST		'*'
# define	KL		'K'
# define	BL		' '
# define	EN		'E'
# define	QU		'Q'
# define	IN		'O'
# define	BA		'B'

extern	char	things[NTHINGS];
/* current sector map */
extern	int	Sect[NSECTS][NSECTS];

/* current position of Enterprise */
extern	int	Quadx, Quady;		/* current quadrant */
extern	int	Sectx, Secty;		/* current sector */

/************************ DEVICES ******************************/

# define	NDEV		16	/* max number of devices */
extern	int	Damage[NDEV];			/* set if device damaged */

/* device tokens */
# define	WARP		0	/* warp engines */
# define	SRSCAN		1	/* short range scanners */
# define	LRSCAN		2	/* long range scanners */
# define	PHASER		3	/* phaser control */
# define	TORPED		4	/* photon torpedo control */
# define	IMPULSE		5	/* impulse engines */
# define	SHIELD		6	/* shield control */
# define	COMPUTER	7	/* on board computer */
# define	SSRADIO		8	/* subspace radio */
# define	LIFESUP		9	/* life support systems */
# define	SINS		10	/* Space Inertial Navigation System */
# define	CLOAK		11	/* cloaking device */
# define	XPORTER		12	/* transporter */
# define	SHUTTLE		13	/* shuttlecraft */

/* device names */
#define	DEVICE	struct device
DEVICE
{
	char	*name;		/* device name */
	char	*person;	/* the person who fixes it */
};

extern	DEVICE		Device[NDEV];

/***************************  EVENTS  ****************************/

# define	NEVENTS		12	/* number of different event types */

# define	E_SNOVA		0	/* supernova occurs */
# define	E_LRTB		1	/* long range tractor beam */
# define	E_KATSB		2	/* Klingon attacks starbase */
# define	E_KDESB		3	/* Klingon destroys starbase */
# define	E_ISSUE		4	/* distress call is issued */
# define	E_ENSLV		5	/* Klingons enslave a quadrant */
# define	E_REPRO		6	/* a Klingon is reproduced */
# define	E_FIXDV		7	/* fix a device */
# define	E_ATTACK	8	/* Klingon attack during rest period */
# define	E_SNAP		9	/* take a snapshot for time warp */
# define	E_NREPORT	32	/* distress call not yet reported */
# define	Q_DISTRESS	64
# define	Q_GHOST		32
# define	Q_STARNAME	31

#define	EVENT	struct event
EVENT
{
	int	x, y;			/* coordinates */
	int	evcode;			/* event type */
	int	evdata;			/* starsystem name */
	float	date;			/* trap stardate */
};
/* systemname conventions:
 *	1 -> NINHAB	index into Systemname table for reported distress calls
 *	+ E_NREPORT	flag marking distress call not reported (SS radio out)
 */

# define	MAXEVENTS	50	/* max number of concurrently pending events */

extern	EVENT		Event[MAXEVENTS];	/* dynamic event list; one entry per pending event */

/*****************************  KLINGONS  *******************************/

#define	KLINGONS	struct klingon
KLINGONS
{
	int	x, y;		/* coordinates */
	int	power;		/* power left */
	float	dist;		/* distance to Enterprise */
	float	avgdist;	/* average over this move */
};
# define	MAXKLQUAD	12	/* maximum klingons per quadrant */
extern	KLINGONS	Kling[MAXKLQUAD];
extern	int		Nkling;		/* number of Klingons in this sector */

/********************** MISCELLANEOUS ***************************/

/* condition codes */
# define	GREEN		0
# define	DOCKED		1
# define	YELLOW		2
# define	RED		3

/*
 *	note that much of the stuff in the following structs CAN NOT
 *	be moved around!!!!
 */

/* initial information */
struct	Initstruct
{
	int	bases;		/* number of starbases */
	int	kling;		/* number of klingons */
	int	torped;		/* photon torpedos */
	float	date;		/* stardate */
	float	time;		/* time left */
	float	resource;	/* Federation resources */
	int	energy;		/* starship's energy */
	int	shield;		/* energy in shields */
	float	reserves;	/* life support reserves */
	int	crew;		/* size of ship's complement */
	int	brigfree;	/* max possible number of captives */
};
extern	struct	Initstruct	Initial;

/* status information */
struct	Statstruct
{
	int	bases;		/* number of starbases */
	int	kling;		/* number of klingons */
	int	torped;		/* torpedoes */
	float	date;		/* stardate */
	float	time;		/* time left */
	float	resource;	/* Federation resources */
	int	energy;		/* starship's energy */
	int	shield;		/* energy in shields */
	float	reserves;	/* life support reserves */
	int	crew;		/* ship's complement */
	int	brigfree;	/* space left in brig */
	int	shldup;		/* shield up flag */
	int	cond;		/* condition code */
	int	sinsbad;	/* Space Inertial Navigation System condition */
	int	cloaked;	/* set if cloaking device on */
	float	warp;		/* warp factor */
	float	warp2;		/* warp factor squared */
	float	warp3;		/* warp factor cubed */
	float	cloakdate;	/* stardate we became cloaked */
	char	*shipname;	/* name of current starship */
	int	ship;		/* current starship */
	int	distressed;	/* number of currently distressed quadrants */
};
extern	struct	Statstruct	Status;

/* sinsbad is set if SINS is working but not calibrated */

/* game related information, mostly scoring */
#define PWDLEN 15
extern	long	inittime;
struct	Gamestruct
{
	int	gkillk;		/* number of klingons killed */
	int	helps;		/* number of help calls */
	int	deaths;		/* number of deaths onboard Enterprise */
	int	negenbar;	/* number of hits on negative energy barrier */
	int	killb;		/* number of starbases killed */
	int	kills;		/* number of stars killed */
	int	skill;		/* skill rating of player */
	int	length;		/* length of game */
	int	killed;		/* set if you were killed */
	int	killinhab;	/* number of inhabited starsystems killed */
	int	tourn;		/* set if a tournament game */
	char	passwd[PWDLEN];	/* game password */
	int	snap;		/* set if snapshot taken */
	int	captives;	/* total number of captives taken */
};
extern	struct	Gamestruct	Game;

/* per move information */
struct	Movestruct
{
	int	free;		/* set if a move is free */
	int	endgame;	/* end of game flag */
	int	shldchg;	/* set if shields changed this move */
	int	newquad;	/* set if just entered this quadrant */
	int	resting;	/* set if this move is a rest */
	float	delta;		/* time used this move */
};
extern	struct	Movestruct	Move;

/* parametric information */
struct	Paramstruct
{
	float	damfac[NDEV];	/* damage factor */
	float	dockfac;	/* docked repair time factor */
	float	regenfac;	/* regeneration factor */
	int	stopengy;	/* energy to do emergency stop */
	int	shupengy;	/* energy to put up shields */
	int	klingpwr;	/* Klingon initial power */
	int	warptime;	/* time chewer multiplier */
	float	phasfac;	/* Klingon phaser power eater factor */
	int	moveprob[6];	/* probability that a Klingon moves */
	float	movefac[6];	/* Klingon move distance multiplier */
	float	eventdly[NEVENTS];	/* event time multipliers */
	float	navigcrud[2];	/* navigation crudup factor */
	int	cloakenergy;	/* cloaking device energy per stardate */
	float	damprob[NDEV];	/* damage probability */
	float	hitfac;		/* Klingon attack factor */
};
extern	struct	Paramstruct	Param;

/* Sum of damage probabilities must add to 1000 */

/* Other stuff, mostly redundant, kept for efficiency reasons */
struct	Etcstruct
{
	EVENT		*eventptr[NEVENTS];	/* pointer to event structs */
};
extern	struct	Etcstruct	Etc;

/*
 *	eventptr is a pointer to the event[] entry of the last
 *	scheduled event of each type.  Zero if no such event scheduled.
 *	NOTE: There may be multiple super novas, a result of the detonate cmd.
 */

/* Klingon move indicies */
# define	KM_OB		0	/* Old quadrant, Before attack */
# define	KM_OA		1	/* Old quadrant, After attack */
# define	KM_EB		2	/* Enter quadrant, Before attack */
# define	KM_EA		3	/* Enter quadrant, After attack */
# define	KM_LB		4	/* Leave quadrant, Before attack */
# define	KM_LA		5	/* Leave quadrant, After attack */

/* you lose codes */
# define	L_NOTIME	1	/* ran out of time */
# define	L_NOENGY	2	/* ran out of energy */
# define	L_DSTRYD	3	/* destroyed by a Klingon */
# define	L_NEGENB	4	/* ran into the negative energy barrier */
# define	L_SUICID	5	/* destroyed in a nova */
# define	L_SNOVA		6	/* destroyed in a supernova */
# define	L_NOLIFE	7	/* life support died (so did you) */
# define	L_NOHELP	8	/* you could not be rematerialized */
# define	L_TOOFAST	9	/* pretty stupid going at warp 10 */
# define	L_STAR		10	/* ran into a star */
# define	L_DSTRCT	11	/* self destructed */
# define	L_CAPTURED	12	/* captured by Klingons */
# define	L_NOCREW	13	/* you ran out of crew */
#define		L_CHEAT		14	/* probably cheating */

# define	CVNTAB	struct cvntab
CVNTAB		/* used for getcodpar() paramater list */
{
	char	*abrev;
	char	*full;
};

#define	XY	struct xy
XY
{
	int	x, y;		/* coordinates */
};

/* starbase coordinates */
# define	MAXBASES	12	/* maximum number of starbases in galaxy */

#define	SIGINT	2
extern	int	mkfault;			/* marks outstanding signal */

extern	XY		Base[MAXBASES];		/* quad coords of starbases */
extern	XY		Starbase;		/* starbase in current quadrant */

/*  distress calls  */
#define	MAXDISTR	8	/* maximum concurrent distress calls */

/* command line flags */
extern	int	cflg;		/* suppress "command: " */
extern	int	rflg;		/* suppress random messages */
extern	int	aflg;		/* scan new quadrants at entry */

/* flags to indicate if various random messages have been given */
# define	NMSGS		4

# define	ALONE		0
# define	BASES		1
# define	SRSC		2
# define	LRSC		3

extern	char	rmsgs[NMSGS];

extern	int	violations;		/* number of Federation regulation violations */
extern	long	violat0, violat1;
extern	long	violats;

#include "setjmp.h"
extern	jmp_buf env;

extern char *bmove();
extern EVENT *schedule();
extern char *ctime();
