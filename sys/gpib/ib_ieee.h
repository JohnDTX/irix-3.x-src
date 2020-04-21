# define MAXIBSLOTS	15			/*max #devs on one bus*/

# define TALK_BASE	0100			/*lowest talk address*/
# define LSTN_BASE	0040			/*lowest listen address*/
# define UNT_CMD	(037|TALK_BASE)		/*untalk cmd*/
# define UNL_CMD	(037|LSTN_BASE)		/*unlisten cmd*/
# define GTL_CMD	001			/*go to local cmd*/
# define SDC_CMD	004			/*selected dev clr cmd*/
# define PPC_CMD	005			/*ppoll config cmd*/
# define GET_CMD	010			/*grp exec trig cmd*/
# define TCT_CMD	011			/*take ctrl cmd*/
# define LLO_CMD	021			/*local lockout cmd*/
# define DCL_CMD	024			/*dev clr cmd*/
# define PPU_CMD	025			/*ppoll unconfig cmd*/
# define SPE_CMD	030			/*spoll enable cmd*/
# define SPD_CMD	031			/*ppoll disable cmd*/
# define	TALKADDR(x)	((x)|TALK_BASE)
# define	LSTNADDR(x)	((x)|LSTN_BASE)
# define	CMDMSG(x)	((x)+0)
# define	SCGMSG(x)	((x)+96)
# define	PPD_CMD		SCGMSG(16)
