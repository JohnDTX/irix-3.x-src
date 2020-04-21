static char ID[] = "@(#)externs.c	1.1";

# include	"trek.h"

jmp_buf		env;

/**
 **	global variable definitions
 **/

DEVICE		Device[NDEV] =
{
	"warp drive",		"Scotty",
	"S.R. scanners",	"Scotty",
	"L.R. scanners",	"Scotty",
	"phaser control",	"Sulu",
	"photon tubes",		"Sulu",
	"impulse engines",	"Scotty",
	"shield control",	"Sulu",
	"computer",		"Spock",
	"subspace radio",	"Uhura",
	"life support",		"Scotty",
	"navigation system",	"Chekov",
	"cloaking device",	"Scotty",
	"transporter",		"Scotty",
	"shuttlecraft",		"Scotty",
	"*** ERROR 14 ***",	"Nobody",
	"*** ERROR 15 ***",	"Nobody"
};

char	*Systemname[NINHAB] =
{
	"*** STARTREK SYSTEM ERROR ***",
	"Talos IV",
	"Rigel III",
	"Deneb VII",
	"Canopus V",
	"Icarus I",
	"Prometheus II",
	"Omega VII",
	"Elysium I",
	"Scalos IV",
	"Procyon IV",
	"Arachnid I",
	"Argo VIII",
	"Triad III",
	"Echo IV",
	"Nimrod III",
	"Nemisis IV",
	"Centarurus I",
	"Kronos III",
	"Spectros V",
	"Beta III",
	"Gamma Tranguli VI",
	"Pyris III",
	"Triachus",
	"Marcus XII",
	"Kaland",
	"Ardana",
	"Stratos",
	"Eden",
	"Arrikis",
	"Epsilon Erandi IV",
	"Exo III"
};

char	things[NTHINGS]	= {".*K EQOB"};	/* things that can be in quadrants */
QUAD	Quad[NQUADS][NQUADS];
int	Sect[NSECTS][NSECTS];
int	Quadx, Quady;		/* current quadrant */
int	Sectx, Secty;		/* current sector */
int	Damage[NDEV];		/* set if device damaged */
EVENT	Event[MAXEVENTS];	/* dynamic event list; one entry per pending event */
KLINGONS	Kling[MAXKLQUAD];
int	Nkling;			/* number of Klingons in this sector */
struct	Initstruct	Initial;
struct	Statstruct	Status;
struct	Gamestruct	Game;
struct	Movestruct	Move;
struct	Paramstruct	Param;
struct	Etcstruct	Etc;
int	mkfault;		/* marks outstanding signal */
XY	Base[MAXBASES];		/* quad coords of starbases */
XY	Starbase;		/* starbase in current quadrant */
int	cflg;			/* suppress "command: " */
int	rflg;			/* suppress random messages */
int	aflg;			/* scan new quadrants upon entry */
char	rmsgs[NMSGS];
int	violations;		/* number of Federation regulation violations */
long	violat0, violat1;
long	violats;
long	inittime;
