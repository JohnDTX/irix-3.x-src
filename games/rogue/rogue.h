/*
 * @(#)rogue.h	1.20 4/6/85 Rogue definitions and variable declarations
 *
 * @(#)rogue.h	3.38 (Berkeley) 6/15/81
 */

#ifdef	Curses
#define	IRISCURSES
#include "Curses.h"
#else
#include "curses.h"
#endif

#ifndef	IRISCURSES
#define	Move	move
#define	Clear	clear
#endif

#ifndef	V7
#define	index	strchr
#define	rindex	strrchr
#endif

#include "mach_dep.h"
#define	crypt	Crypt

#ifndef	BSIZE
#define	BSIZE	1024
#endif
#ifndef	BUFSIZ
#define	BUFSIZ	BSIZE
#endif

#ifndef	uchar
#define	uchar	unsigned char
#endif

/*
 * set up number of register variables allowed
 */

#ifdef	m68000

#ifndef	regt
#define	regt	0
#endif	regt

#ifndef	reg
#define	rega	4
#endif	reg

#ifndef	reg
#define	regd	4
#endif	reg

#endif

#ifndef	regt
#define	regt	3
#endif	regt

#ifndef	reg
#define	rega	0
#endif	reg

#ifndef	reg
#define	regd	0
#endif	reg

/* define reg[1-9] */

#if	regt >= 1
#define	reg1	reg
#endif

#if	regt >= 2
#define	reg2	reg
#endif

#if	regt >= 3
#define	reg3	reg
#endif

#if	regt >= 4
#define	reg4	reg
#endif

#if	regt >= 5
#define	reg5	reg
#endif

#if	regt >= 6
#define	reg6	reg
#endif

#if	regt >= 7
#define	reg7	reg
#endif

#if	regt >= 8
#define	reg8	reg
#endif

#if	regt >= 9
#define	reg9	reg
#endif

/* define undefined reg[1-9] to be No-op */

#ifndef	reg1
#define	reg1
#endif

#ifndef	reg2
#define	reg2
#endif

#ifndef	reg3
#define	reg3
#endif

#ifndef	reg4
#define	reg4
#endif

#ifndef	reg5
#define	reg5
#endif

#ifndef	reg6
#define	reg6
#endif

#ifndef	reg7
#define	reg7
#endif

#ifndef	reg8
#define	reg8
#endif

#ifndef	reg9
#define	reg9
#endif

/* define rega[1-9] */

#if	rega >= 1
#define	rega1	reg
#endif

#if	rega >= 2
#define	rega2	reg
#endif

#if	rega >= 3
#define	rega3	reg
#endif

#if	rega >= 4
#define	rega4	reg
#endif

#if	rega >= 5
#define	rega5	reg
#endif

#if	rega >= 6
#define	rega6	reg
#endif

#if	rega >= 7
#define	rega7	reg
#endif

#if	rega >= 8
#define	rega8	reg
#endif

#if	rega >= 9
#define	rega9	reg
#endif

/* define undefined rega[1-9] to be No-op */

#ifndef	rega1
#define	rega1
#endif

#ifndef	rega2
#define	rega2
#endif

#ifndef	rega3
#define	rega3
#endif

#ifndef	rega4
#define	rega4
#endif

#ifndef	rega5
#define	rega5
#endif

#ifndef	rega6
#define	rega6
#endif

#ifndef	rega7
#define	rega7
#endif

#ifndef	rega8
#define	rega8
#endif

#ifndef	rega9
#define	rega9
#endif

/* define regd[1-9] */

#if	regd >= 1
#define	regd1	reg
#endif

#if	regd >= 2
#define	regd2	reg
#endif

#if	regd >= 3
#define	regd3	reg
#endif

#if	regd >= 4
#define	regd4	reg
#endif

#if	regd >= 5
#define	regd5	reg
#endif

#if	regd >= 6
#define	regd6	reg
#endif

#if	regd >= 7
#define	regd7	reg
#endif

#if	regd >= 8
#define	regd8	reg
#endif

#if	regd >= 9
#define	regd9	reg
#endif

/* define undefined regd[1-9] to be No-op */

#ifndef	regd1
#define	regd1
#endif

#ifndef	regd2
#define	regd2
#endif

#ifndef	regd3
#define	regd3
#endif

#ifndef	regd4
#define	regd4
#endif

#ifndef	regd5
#define	regd5
#endif

#ifndef	regd6
#define	regd6
#endif

#ifndef	regd7
#define	regd7
#endif

#ifndef	regd8
#define	regd8
#endif

#ifndef	regd9
#define	regd9
#endif

/*
 * Maximum number of different things
 */
#define	MAXROOMS	 9
#define	MAXTHINGS	14
#define	MAXOBJ		14
#define	MAXPACK		28
#define	MAXTRAPS	10
#define	NUMTHINGS	10	/* number of types of things (scrolls,rings,etc.) */

/*
 * return values for get functions
 */
#define	NORM		 0	/* normal exit */
#define	QUIT		 1	/* quit option setting */
#define	MINUS		 2	/* back up one option */

/*
 * misc defines
 */
#define	WON		2
#define	QUITIT		1
#define	DIED		0
#define	C_PLAYER	C_yellow/* player's color */
#define	C_vary		10	/* a color map entry I can play with */

/*
 * All the fun defines
 */
#ifdef	FAST
#define	next(ptr)	(*ptr).l_next
#define	prev(ptr)	(*ptr).l_prev
#define	ldata(ptr)	(*ptr).l_data
#define	inroom(rp,cp) (\
  (cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && (rp)->r_pos.x <= (cp)->x \
  && (cp)->y <= (rp)->r_pos.y + ((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)
#define	winat(y,x)	(mvwinch(mw,y,x)==' '?mvwinch(stdscr,y,x):winch(mw))
#endif
#define	debug		if (wizard) msg
#define	RN		(((seed = seed*11109+13849) & 0x7fff) >> 1)
#define	unc(cp)		(cp).y,(cp).x
#define	Cmov(xy)	Move((xy).y,(xy).x)
#define	DISTANCE(y1,x1,y2,x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define	when		break;case
#define	otherwise	break;default
#define	until(expr)	while (!(expr))
#define	ce(a,b)		((a).x == (b).x && (a).y == (b).y)
#define	Draw(window)	wrefresh(window)
#define	hero		player.t_pos
#define	pstats		player.t_stats
#define	pack		player.t_pack
#define	attach(a,b)	_attach(&a,b)
#define	detach(a,b)	_detach(&a,b)
#define	free_list(a)	_free_list(&a)
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	on(thing,flag)	(((thing).t_flags & flag) != 0)
#define	off(thing,flag)	(((thing).t_flags & flag) == 0)
#undef	CTRL
#define	CTRL(ch)	('ch' & 037)
#define	ALLOC(x)	malloc((unsigned int) x)
#define	FREE(x)		cfree((char *) x)
#define	EQSTR(a,b,c)	(strncmp(a,b,c) == 0)
#define	GOLDCALC	(rnd(50 + 10 * level) + 2)
#define	iswall(ch)	(ch==wallv || ch==wallh || ch==wallul || ch==wallur \
			|| ch==wallll ||  ch==walllr)
#define isvowel(c)	(c=='a' || c=='e' || c=='i' || c=='o' || c=='u')
#define	ISRING(h,r)	(cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define	ISWEARING(r)	(ISRING(LEFT,r) || ISRING(RIGHT,r))
#define	newgrp()	++group
#define	o_charges	o_ac
#define	ISMULT(type)	(type == POTION || type == SCROLL || type == FOOD)

/*
 * Things that appear on the screens
 */
#define Door		'+'
#define Floor		'.'
#define	Passage		'#'
#define	Wallur		CTRL(A)
#define	Walllr		CTRL(B)
#define	Wallul		CTRL(C)
#define	Wallll		CTRL(D)
#define	PLAYER		'@'
#define	TRAP		'^'
#define	TRAPDOOR	'>'
#define	ARROWTRAP	'{'
#define	SLEEPTRAP	'$'
#define	BEARTRAP	'}'
#define	TELTRAP 	'~'
#define	DARTTRAP 	'`'
#define	SECRETDOOR	'&'
#define	STAIRS		'%'
#define	GOLD		'*'
#define	POTION		'!'
#define	SCROLL		'?'
#define	MAGIC		'$'
#define	FOOD		':'
#define	WEAPON		')'
#define	ARMOR		']'
#define	AMULET		','
#define	ZAP		'~'
#define	Cursor		'^'
#define	RING 		'='
#define	WAND 		'/'
#define	CALLABLE 	(-1)

/*
 * Various constants
 */
#define	PASSWD		"mTYUat3uYi7wE"
#define	BEARTIME	   3
#define	SLEEPTIME	   5
#define	HEALTIME	  30
#define	HOLDTIME	   2
#define	STPOS		   0
#define	WANDERTIME	  70
#define	BEFORE		   1
#define	AFTER		   2
#define	HUHDURATION	  20
#define	SEEDURATION	850
#define	HUNGERTIME	1300
#define	MORETIME	 150
#define	STOMACHSIZE	2000
#define	ESCAPE		  27
#define	REST		'.'	/* character to rest (was space)	*/
#define	LEFT		0
#define	RIGHT		1
#define	BOLT_LENGTH	6

/*
 * Save against things
 */
#define	VS_POISON	  00
#define	VS_PARALYZATION	  00
#define	VS_DEATH	  00
#define	VS_PETRIFICATION  01
#define	VS_BREATH	  02
#define	VS_MAGIC	  03

/*
 * Various flag bits
 */
#define	ISDARK		0000001
#define	ISCURSED	0000001
#define	ISBLIND		0000001
#define	ISGONE		0000002
#define	ISKNOW		0000002
#define	ISRUN		0000004
#define	ISFOUND		0000010
#define	ISINVIS		0000020
#define	ISMEAN		0000040
#define	ISGREED		0000100
#define	ISBLOCK		0000200
#define	ISHELD		0000400
#define	ISHUH		0001000
#define	ISREGEN		0002000
#define	CANHUH		0004000
#define	CANSEE		0010000
#define	ISMISL		0020000
#define	ISCANC		0020000
#define	ISMANY		0040000
#define	ISSLOW		0040000
#define	ISHASTE		0100000

/*
 * Potion types
 */
#define	P_CONFUSE	 0
#define	P_PARALYZE	 1
#define	P_POISON	 2
#define	P_STRENGTH	 3
#define	P_SEEINVIS	 4
#define	P_HEALING	 5
#define	P_MFIND		 6
#define	P_TFIND		 7
#define	P_RAISE		 8
#define	P_XHEAL		 9
#define	P_HASTE		10
#define	P_RESTORE	11
#define	P_BLIND		12
#define	P_NOP		13
#define	MAXPOTIONS	14

/*
 * Scroll types
 */
#define	S_CONFUSE	 0
#define	S_MAP		 1
#define	S_LIGHT		 2
#define	S_HOLD		 3
#define	S_SLEEP		 4
#define	S_ARMOR		 5
#define	S_IDENT		 6
#define	S_SCARE		 7
#define	S_GFIND		 8
#define	S_TELEP		 9
#define	S_ENCH		10
#define	S_CREATE	11
#define	S_REMOVE	12
#define	S_AGGR		13
#define	S_NOP		14
#define	S_GENOCIDE	15
#define	MAXSCROLLS	16

/*
 * Weapon types
 */
#define	MACE		 0
#define	SWORD		 1
#define	BOW		 2
#define	ARROW		 3
#define	DAGGER		 4
#define	ROCK		 5
#define	TWOSWORD	 6
#define	SLING		 7
#define	DART		 8
#define	CROSSBOW	 9
#define	BOLT		10
#define	SPEAR		11
#define	MAXWEAPONS	12

/*
 * Armor types
 */
#define	LEATHER		 0
#define	RING_MAIL	 1
#define	STUDDED_LEATHER	 2
#define	SCALE_MAIL	 3
#define	CHAIN_MAIL	 4
#define	SPLINT_MAIL	 5
#define	BANDED_MAIL	 6
#define	PLATE_MAIL	 7
#define	MAXARMORS	 8

/*
 * hunger levels for status line
 * Ring types
 */
#define	H_NONE		 0
#define	H_SOME		 1
#define	H_VERY		 2
#define	H_FAINT		 3
#define	hungry_state	 hunglev	/* 3.6 vs. 2.6+ */
#define	R_PROTECT	0
#define	R_ADDSTR	1
#define	R_SUSTSTR	2
#define	R_SEARCH	3
#define	R_SEEINVIS	4
#define	R_NOP		5
#define	R_AGGR		6
#define	R_ADDHIT	7
#define	R_ADDDAM	8
#define	R_REGEN		9
#define	R_DIGEST	10
#define	R_TELEPORT	11
#define	R_STEALTH	12
#define	MAXRINGS	13

/*
 * pseudo types
 * Rod/Wand/Staff types
 */
#define	UNSIGN	unsigned
#define	reg	register
#define	rstruct	register struct

/* color cabability disable */
#ifndef	COLOR
#define	Dcolor(a,b)
#undef	acolor
#define	acolor(a,b,c)
#endif
#define	WS_LIGHT	0
#define	WS_HIT		1
#define	WS_ELECT	2
#define	WS_FIRE		3
#define	WS_COLD		4
#define	WS_POLYMORPH	5
#define	WS_MISSILE	6
#define	WS_HASTE_M	7
#define	WS_SLOW_M	8
#define	WS_DRAIN	9
#define	WS_NOP		10
#define	WS_TELAWAY	11
#define	WS_TELTO	12
#define	WS_CANCEL	13
#define	MAXWANDS	14

#ifndef	Curses
#ifdef	V7
# ifdef	BSD
				/* May need for some V7 systems */
#define	flushi()	cbreak()
# else	BSD
struct	sgttyb		clrbuf;
#define	flushi()	gtty(0,&clrbuf),stty(0,&clrbuf)
# endif	BSD
#else	V7
#define	flushi()	ioctl(_tty_ch, TCFLSH, 0)
#endif	V7
#endif	Curses

/* debugging */
/*
 * #define	LED(x)	{printf("\033[%dq",x);fflush(stdout);sleep(1);}
 * #define	G()	{printf("\007");fflush(stdout);sleep(1);}
 * #define	K()	{printf("\033#>");fflush(stdout);sleep(1);}
 */
#define	LED(x)
#define	G()
#define	K()	G()

/*
 * Now we define the structures and types
 */

/*
 * Help list
 */

struct	h_list {
	char	h_ch;
	char	*h_desc;
} helpstr[], wizhelp[];

/*
 * Coordinate data type
 */
typedef	struct {
	int	x;
	int	y;
} coord;

typedef struct	{
	short	st_str;
	short	st_add;
} str_t;

/*
 * Linked list data type
 */
struct	linked_list {
	struct	linked_list	*l_next;
	struct	linked_list	*l_prev;
	char	*l_data;			/* Various structure pointers */
};

/*
 * Stuff about magic items
 */

struct	magic_item {
	char	*mi_name;
	int	mi_prob;
	int	mi_worth;
};

/*
 * Room structure
 */
struct	room {
	coord	r_pos;			/* Upper left corner */
	coord	r_max;			/* Size of room */
	coord	r_gold;			/* Where the gold is */
	int	r_goldval;		/* How much the gold is worth */
	int	r_flags;		/* Info about the room */
	int	r_nexits;		/* Number of exits */
	coord	r_exit[4];		/* Where the exits are */
};

/*
 * Array of all traps on this level
 */
struct	trap {
	coord	tr_pos;			/* Where trap is */
	char	tr_type;		/* What kind of trap */
	int	tr_flags;		/* Info about trap (i.e. ISFOUND) */
} traps[MAXTRAPS];

/*
 * Structure describing a fighting being
 */
struct	stats {
	str_t	s_str;			/* Strength */
	long	s_exp;				/* Experience */
	int	s_lvl;				/* Level of mastery */
	int	s_arm;				/* Armor class */
	int	s_hpt;				/* Hit points */
	char	*s_dmg;				/* String for damage done */
};

/*
 * Structure for monsters and player
 */
struct	thing {
	coord	t_pos;			/* Position */
	bool	t_turn;			/* If slowed,is it a turn to move */
	char	t_type;			/* What it is */
	char	t_disguise;		/* What mimic looks like */
	char	t_oldch;		/* Character that was where it was */
	coord	*t_dest;		/* Where it is running to */
	short	t_flags;		/* State word */
	struct	stats	t_stats;	/* Physical description */
	struct	linked_list *t_pack;	/* What the thing is carrying */
};

/*
 * Array containing information on all the various types of monsters
 */
struct	monster {
	char	*m_name;		/* What to call the monster */
	short	m_carry;		/* Probability of carrying something */
	short	m_flags;		/* Things about the monster */
	struct	stats	m_stats;	/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct	object {
	int	o_type;			/* What kind of object it is */
	coord	o_pos;			/* Where it lives on the screen */
	char	*o_text;		/* What it says if you read it */
	char	o_launch;		/* What you need to launch it */
	char	*o_damage;		/* Damage if used like sword */
	char	*o_hurldmg;		/* Damage if thrown */
	int	o_count;		/* Count for plural objects */
	int	o_which;		/* Which object of a type it is */
	int	o_hplus;		/* Plusses to hit */
	int	o_dplus;		/* Plusses to damage */
	int	o_ac;			/* Armor class */
	int	o_flags;		/* Information about objects */
	int	o_group;		/* Group number for this object */
};

/*
 * Now all the global variables
 */

struct	room rooms[MAXROOMS];		/* One for each room -- A level */
struct	room *oldrp;			/* Roomin(&oldpos) */
struct	linked_list *mlist;		/* List of monsters on the level */
struct	thing player;			/* The rogue */
struct	stats max_stats;		/* The maximum for the player */
struct	monster monsters[26];		/* The initial monster states */
struct	linked_list *lvl_obj;		/* List of objects on this level */
struct	object *cur_weapon;		/* Which weapon he is weilding */
struct	object *cur_armor;		/* What a well dresssed rogue wears */
struct	object *cur_ring[2];		/* Which rings are being worn */
struct	magic_item things[NUMTHINGS];	/* Chances for each type of item */
struct	magic_item s_magic[MAXSCROLLS];	/* Names and chances for scrolls */
struct	magic_item p_magic[MAXPOTIONS];	/* Names and chances for potions */
struct	magic_item r_magic[MAXRINGS];	/* Names and chances for rings */
struct	magic_item W_magic[MAXWANDS];	/* Names and chances for wands */

int	level;				/* What level rogue is on */
int	purse;				/* How much gold the rogue has */
int	mpos;				/* Where cursor is on top line */
int	ntraps;				/* Number of traps on this level */
int	no_move;			/* Number of turns held in place */
int	no_command;			/* Number of turns asleep */
int	inpack;				/* Number of things in pack */
int	max_hp;				/* Player's max hit points */
int	total;				/* Total dynamic memory bytes */
#ifdef	COLOR
int	p_Colors[MAXPOTIONS];		/* actual colors of the potions */
#endif
int	a_chances[MAXARMORS];		/* Probabilities for armor */
int	a_class[MAXARMORS];		/* Armor class for various armors */
int	lastscore;			/* Score before this turn */
int	no_food;			/* Number of levels without food */
int	seed;				/* Random number seed */
int	count;				/* Number of times to repeat command */
int	dnum;				/* Dungeon number */
int	fung_hit;			/* Number of time fungi has hit */
int	hunglev;			/* level of hunger - for status line */
#ifdef	COLOR
int	C_gold;				/* color of gold */
int	C_black;			/* color of black */
int	C_white;			/* color of white */
int	C_red;				/* color of red */
int	C_gold;				/* color of gold */
int	C_yellow;			/* color of yellow */
int	C_blue;				/* color of blue */
int	C_green;			/* color of green */
int	C_cyan;				/* color of cyan */
int	C_magenta;			/* color of magenta */
#endif
int	maxmsg;
#ifdef	iris
int	regfont;
int	msgfont;
#endif
int	quiet;				/* Number of quiet turns */
int	max_level;			/* Deepest player has gone */
int	food_left;			/* Amount of food in hero's stomach */
int	group;				/* Current group number */
int	hungry_state;			/* How hungry is he */

char	take;				/* Thing the rogue is taking */
char	prbuf[200];			/* Buffer for sprintfs */
char	home[80];			/* Home directory */
char	file_name[80];			/* Save file */
char	outbuf[BUFSIZ];			/* Output buffer for stdout */
char	runch;				/* Direction player is running */
char	*s_names[MAXSCROLLS];		/* Names of the scrolls */
char	*p_colors[MAXPOTIONS];		/* Colors of the potions */
char	*r_rings[MAXRINGS];		/* Names of the various rings */
char	*w_names[MAXWEAPONS];		/* Names of the various weapons */
char	*a_names[MAXARMORS];		/* Names of armor types */
char	*W_made[MAXWANDS];		/* What wands are made of */
char	*release;			/* Release number of rogue */
char	whoami[80];			/* Name of player */
char	fruit[80];			/* Favorite fruit */
char	huh[80];			/* The last message printed */
char	*s_guess[MAXSCROLLS];		/* Players guess at what scroll is */
char	*p_guess[MAXPOTIONS];		/* Players guess at what potion is */
char	*r_guess[MAXRINGS];		/* Players guess at what ring is */
uchar	wallv;				/* wall:vertical	*/
uchar	wallh;				/* wall:horizontal	*/
uchar	wallur;				/* wall:upper right	*/
uchar	walllr;				/* wall:lower right	*/
uchar	wallul;				/* wall:upper left	*/
uchar	wallll;				/* wall:lower left	*/
uchar	door;				/* door			*/
uchar	floor;				/* floor		*/
uchar	passage;			/* passage way		*/
uchar	cursor;				/* cursor char		*/
char	*W_guess[MAXWANDS];		/* Players guess at what wand is */
char	*W_type[MAXWANDS];		/* Is it a wand or a staff */
char	*plusstr;			/* "+" or "Plus "	*/
char	*more;

WINDOW	*cw;				/* Window that the player sees	      */
WINDOW	*hw;				/* Used for the help command	      */
WINDOW	*mw;				/* Used to store monsters	      */
WINDOW	*msgw;				/* Used for msg(): usually cw	      */
WINDOW	*oldmsgw;			/* Used for msg(): usually cw	      */

bool	running;			/* True if player is running	      */
bool	playing;			/* True until he quits		      */
bool	wizard;				/* True if allows wizard commands     */
bool	after;				/* True if we want after daemons      */
bool	fromfile;			/* True if game was restored	      */
bool	notify;				/* True if player wants to know	      */
bool	fight_flush;			/* True if toilet input		      */
bool	terse;				/* True if we should be short	      */
bool	door_stop;			/* Stop running when we pass a door   */
bool	jump;				/* Show running as series of jumps    */
bool	slow_invent;			/* Inventory one line at a time	      */
bool	firstmove;			/* First move after setting door_stop */
bool	waswizard;			/* Was a wizard sometime	      */
bool	askme;				/* Ask about unidentified things      */
bool	s_know[MAXSCROLLS];		/* Does he know what a scroll does    */
bool	p_know[MAXPOTIONS];		/* Does he know what a potion does    */
bool	r_know[MAXRINGS];		/* Does he know what a ring does      */
bool	W_know[MAXWANDS];		/* Does he know what a wand does      */
bool	amulet;				/* He found the amulet */
bool	in_shell;			/* True if executing a shell */

#ifdef	COLOR
PIXEL	Rcolor;				/* R command color */
#endif
coord	oldpos;				/* Position before last look() call */
coord	delta;				/* Change indicated to get_dir() */

struct	linked_list *find_mons(),*find_obj(),*get_item(),*new_item();
struct	linked_list *new_thing(),*wake_monster();

#ifndef	FAST
struct	linked_list	*next();
struct	linked_list	*prev();
char	*ldata();
#endif
char	*malloc(),*getenv(),*unctrl(),*tr_name(),*new(),*sprintf();
char	*vowelstr(),*inv_name(),*strcpy(),*strcat(),*sbrk(),*brk();
char	*ctime(),*num(),*ring_num();

char	*calloc();
char	*getenv();
char	*unctrl();
char	*tr_name();
char	*new();
char	*sprintf();
char	*vowelstr();
char	*killname();
char	*hand();

struct	room *roomin();

coord	*rndmove();

int	endit();
int	nohaste();
int	rollwand();
int	runners();
int	swander();
int	tstp();
int	unconfuse();
int	unsee();
int	auto_save(),endit(),nohaste(),doctor(),runners(),swander();
int	tstp(),unconfuse(),unsee(),rollwand(),stomach(),sight();

#ifdef CHECKTIME
int	checkout();
#endif

long	lseek();

struct	trap	*trap_at();
