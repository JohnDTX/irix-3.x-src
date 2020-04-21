typedef int (*FUNPTR)();

typedef struct {
	FUNPTR func;
	char *format;
	short arg, token;
	short framesize;	
	short returnsdata;	/* this makes the struct 16 bytes long. */
} dispatchEntry;

#define STACKMODE 1
#define ARRAYMODE 2

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

#ifdef BAD_CODE
#define sendBs( array, n )	senda( array, n )
#define sendFs( array, n )	senda( array, ((n)<<2) )
#define sendLs( array, n )	senda( array, ((n)<<2) )
#define sendSs( array, n )	senda( array, ((n)<<1) )
#endif

/* the graphics contexts */

#define TEXT		1
#define GRAPHICS	2

short 	context;
short 	ttymedium;
short 	graphmedium;
short   turnaround;
short	graphinited;
short	textfile;
short 	maxcom;

#define LAMP_LOCAL	(1<<4)
#define LAMP_KBDLOCKED	(1<<5)
#define LAMP_L1		(1<<0)
#define LAMP_L2		(1<<1)
#define LAMP_L3		(1<<2)
#define LAMP_L4		(1<<3)

#define SCRBUFFSIZE	100

extern unsigned char *swp;
extern int swc;

#define putscreenchar(c)  {*swp++ = (c); if(++swc>=SCRBUFFSIZE) flushscreen();}
