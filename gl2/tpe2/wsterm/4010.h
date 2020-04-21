/*		Modified by Jonathan Bowen - July 1984	*/

#ifndef DOGRAPHICS
#define NOGRAPHICS
#endif

/* ASCII control characters */

#ifndef NUL
#define NUL 0
#endif

#ifndef ENQ
#define ENQ 5
#endif

#ifndef BEL
#define BEL 7
#endif

#ifndef BS
#define BS 8
#endif

#ifndef HT
#define HT 9
#endif

#ifndef LF
#define LF 10
#endif

#ifndef VT
#define VT 11
#endif

#ifndef FF
#define FF 12
#endif

#ifndef CR
#define CR 13
#endif

#ifndef SUB
#define SUB 26
#endif

#ifndef ESC
#define ESC 27
#endif

#ifndef GS
#define GS 29
#endif

#ifndef US
#define US 31
#endif

#ifndef DEL
#define DEL 127
#endif


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef CR
#define CR 13
#endif

#define ASCIIMASK 0x7f

/* 4010 status byte definitions */
#define	AUXSENSE 0x1
#define MARGIN	0x2
#define GRAPH	0x4
#define NOLI /* 0x8 */ 0
#define HCU	0x10

/* just a boolean */
#define GRAPHMODE 1
#define ALPHAMODE 0

#ifdef DOGRAPHICS
extern Screencoord crossx, crossy;
#else
extern short crossx, crossy;
#endif

extern init4010();
extern char crosshair();

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif
