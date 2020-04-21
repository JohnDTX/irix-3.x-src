#include <stdio.h>

/* One of `gl1' or `gl2' must be defined. */

#ifdef gl1
#define GL1
#undef GL2
#endif

#ifdef gl2
#define GL2
#undef GL1
#endif

#define MAXARGS		20	/* the maximum number of args a RGL routine
				 * can have */

typedef int (*FUNPTR)();

typedef struct {
	FUNPTR func;
	char *format;
	short arg, token;
	short framesize;	
	short returnsdata;
} dispatchEntry;

char *alloca();
char *reca();

#define allocBa(n)	(char *)alloca(n+3)
#define allocFa(n)	(float *)alloca((n)<<2)
#define allocLa(n)	(long *)alloca((n)<<2)
#define allocSa(n)	(short *)alloca((n+1)<<1)

#define recbs()	(char *)reca()
#define recfs()	(float *)reca()
#define recls()	(long *)reca()
#define recss()	(short *)reca()

/* the graphics contexts */

#define TEXT		1
#define GRAPHICS	2

/* the pipe commands */

#define SWITCHCONTEXT	1
#define SETMONITOR	2
#define TOGGLEPFLAG	3
#define TOGGLEXFLAG	4
#define RESETDISPLAY	5
#define INSUBSHELL	6
#define TOGGLETP	7

extern int dflag[];
extern int pflag;
extern int rflag;
extern int xflag;
extern int yflag;
extern int zflag[];
extern int speedcode;
extern int insubshell;
extern int context;
extern int kbdlocked;
extern short maxcom; 
extern int graphinited;
extern int ignoretext;
extern int ingraphprog;
extern int pipeready;
extern int pipereaderpid;
extern int fromreader;
extern int towriter;
extern int fromwriter;
extern int toreader;
extern int host;
extern int gcmdcnt;
extern int tpison;
extern FILE *lf;
extern dispatchEntry dispatch[];
extern int haveconnection;

extern char *getcmdname();

#ifdef GL1
#define LAMP_KBDLOCKED	(1<<2)
#define LAMP_LOCAL	(1<<1)
#else GL2
#define LAMP_KBDLOCKED	(1<<5)
#define LAMP_LOCAL	(1<<4)
#endif GL2

#define putscreenchar(c)    putc(c, stdout)
#define flushscreen()	    fflush(stdout)

#define GetBuflenAheadOfTime	gl_GetBuflenAheadOfTime
