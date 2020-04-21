/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* must include term.h ahead of this file */

#define SERIAL_COM	234
#define ETHER_COM	235
#define I488_COM	236

#define TCP_TYPE	1
#define XNS_TYPE	2
#define SERIAL_TYPE	3
#define I488_TYPE	4

#define MAXARGS		20	/* the maximum number of args a RGL routine
				 * can have
				 */

#define RWBUFSIZE	1024
#define WFLUSHLIMIT	100

#define gethostchar() 	(--rc >= 0 ? *rp++ : fillhostbuffer())
#define puthostchar(c)  { *wp++ = (c); if (++wc >= WFLUSHLIMIT) flushhost(); }

typedef struct {
	FUNPTR func;
	char *format;
	short arg, token;
	short framesize;	
	short returnsdata;
} dispatchEntry;

char *alloca();
char *reca();
char recb();
short recs();
long recl();

#define allocBa(n)	(char *)alloca(n+3)
#define allocFa(n)	(float *)alloca((n)<<2)
#define allocLa(n)	(long *)alloca((n)<<2)
#define allocSa(n)	(short *)alloca((n+1)<<1)

#define recbs()	(char *)reca()
#define recfs()	(float *)reca()
#define recls()	(long *)reca()
#define recss()	(short *)reca()

extern unsigned char *wp, *rp;
extern unsigned wc;
extern int rc;

extern dispatchEntry dispatch[];
extern int dispatchLen;
extern char *getcmdname();

