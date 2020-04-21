#ifndef GLIPCDEF
#define GLIPCDEF
/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* processes we can send to */

#define WINDOWMAN	(-1)
#define CONMAN		1

#define PORTNAMESIZE	14	/* how big a name getport etc can take */

#define	SENDSIZE	36	/* < sizeof(sh->smallbuf) */
struct	sendrec {
	short	msg;		/* message to process */
	short	len;		/* length of message */
	short	data[SENDSIZE];	/* data to msg */
};

/* messages to window manager */

struct portreq {
    short	minsizex;
    short	minsizey;
    short	maxsizex;
    short	maxsizey;
    short	keepaspect;
    short	aspectx;
    short	aspecty;
    short	prefsize;
    short	prefsizex;
    short	prefsizey;
    short	preforigin;
    short	preforiginx;
    short	preforiginy;
    short	xunit;
    short	yunit;
    short	xunitfudge;
    short	yunitfudge;
    short	imakebackground;
    short	imakemap;
    short	gfnum;
    long	pid;
    char	name[PORTNAMESIZE+1];
};

struct textportreq {
    short	x1;
    short	x2;
    short	y1;
    short	y2;
};

#define PORTREQ		10
#define TEXTPORTREQ	11
#define BACKGROUNDREQ	12
#define PUSHREQ		13
#define POPREQ		14
#define ATTACHREQ	15
#define MOVEREQ		16
#define RESHAPEREQ	17
#define SETTITLEREQ	18
#define BEGINPUPMODE	20
#define ENDPUPMODE	21
#define WINDOWAT	22
#define INCHANAT	23
#define SETNAMEREQ	24
#define TXOPEN		25
#define TXCLOSE		26
#define NEWHINTS	27
#define BEGINFSMODE	28
#define ENDFSMODE	29

#endif GLIPCDEF
