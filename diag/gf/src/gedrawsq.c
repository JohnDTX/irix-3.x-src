/* gedraw.c */

#include "/usr/sun/include/pcmap.h"
#include "/usr/sun/include/m68000.h"

#include "fbcld.h"
#include "gfdev.h"
#define WAIT	0x73
#define REQ	0x63

#include "../geofdef.h"

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */

extern short devstatus;	/* copy of currently written status reg */
extern short GEstatus;
extern char cmd,which,how;

extern unsigned short realdraw[];

unsigned short *pgearray;

gedraw()
{
short i,num,lim;
register	short	sendingreg;	/* d7 */
register	short	*GEaddr = (short *)&GEdata;	/* a5 */
static unsigned short drawx = 0;
static unsigned short drawy = 0;
static unsigned short sx = 0x7f;
static unsigned short sy = 0x7f;

#define SGEdata(x)	{ *GEaddr = x; \
			while(!( FBCflags & INREQ_BIT_)) ; }

num = getnum()+1;
while (num--) {
	realdraw[78] = 0x740 + drawx;
	realdraw[80] = 0x740 + drawy;
	realdraw[51] = sx;
	realdraw[62] = sy;

/*	realdraw[i=78] = (drawx<<8) + 0x40;	/* this works */
/*	realdraw[++i]  = 0;
	realdraw[++i]  = (drawy<<8) + 0x40;
	realdraw[++i]  = 0;*/

/*	drawx++;
	if (drawx >= 0x3f) drawx -= 0x3f;
	drawy++;
	if (drawy >= 0x3f) drawy -= 0x3f;*/

	--sx;
	if (sx==0) sx = 0x7f;
	--sy;
	if (sy==0) sy = 0x7f;

	pgearray = realdraw;
	while (*pgearray!=GEOF ) SGEdata(*pgearray++);
    }
/*for (i=0; i<64; i++) SGEdata(8);*/

}
