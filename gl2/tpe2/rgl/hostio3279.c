/*
**		    Standard I/O stuff for terminal program
**
**			     Paul Haeberli - Aug 1983
*/
#include "gl.h"
#include "Vxns.h"
#include "term.h"
#include "hostio.h"
#include "Vserial.h"
#include "Vteams.h"
#ifdef GL2
#undef EOF
#include "stdio.h"
#else
#include "Vioprotocl.h"
#endif GL2
#include <types.h>
#undef ERROR
#include "pxw.h"

#define ESC		0x1b

#ifdef GL2
FILE 			*serialin;
FILE			*serialout;
#else
File 		*serialin;
File		*serialout;
#endif GL2
int 			netfid;

int 			wc;
unsigned char	 	*wp;
static unsigned char	wbuffer[WBUFFSIZE]; 
int 			rc;
unsigned char 		*rp;
static unsigned char	rbuffer[6144]; /* IBM LENGTH INCLUDED IN MESSAGE */
short			graphmedium, ttymedium;

struct irismsg {
	unsigned	vkernelsucks;
	unsigned char	*m_buffer;
	unsigned short	m_count;
	unsigned short	pad[11];
};

rglft			frcv;


extern Process_id 	dadypid;
extern int		errno; 
extern InstanceId	fid;
extern px_status	outb;
extern px_bufs		pxl;
extern Process_id	rgraphid,rttyhid;
extern RootMessage	*RootMsg;
extern InstanceId	stdinfile;
extern u_char		towrite;


/*
**	initcom - intialize serial i/o or xns.
**
*/
initcom( medium )
register int medium;
{

    wc = 0;
    wp = wbuffer;
    rc = 0;
    rp= rbuffer;

    if(medium == I3270_COM) {
	if (pxdopen()<=0) {
		(void)messagef("Cannot open '/dev/pxd':  errno = %d\n", errno);
		(void)printf("Cannot open '/dev/pxd':  errno = %d\n", errno);
		exit(1);
	}
#ifdef GL2TERM
        message("\n\rIBM 3279 iris terminal GL2 . . .\n\r");
#else
        message("\n\rIBM 3279 iris terminal GL1 . . .\n\r");
#endif GL2TERM
	ttymedium = graphmedium = I3270_COM;
    }
}


closecom()
{
    if(ttymedium == ETHER_COM)
        CloseSeq(netfid);
}


flushhost()
{
    flushhostbuffer();
}

u_char
vgetc()
{
	Process_id serverid;
	union message {
		QuerySerialRequest	req;
		QuerySerialReply   	reply;
	} msg;
	union message *mp;
	u_char c;
	int kbdcount;

	mp = &msg;
	serverid = GetPid(DEVICE_SERVER,LOCAL_PID);
	mp->req.requestcode = QUERY_FILE;
	mp->req.fileid = RootMsg->stdinfile;
	mp->req.linenum = 0;
	serverid = GetPid(DEVICE_SERVER,LOCAL_PID);

	if ( Send(mp,serverid) == 0 ) {
		messagef("vgetc Send failure ");
		return(ERROR);
	}

	if ( mp->reply.replycode != OK ) {
		messagef("vgetc replycode failure ");
		return(ERROR);
	}
	/* first test chars of data */
	kbdcount = mp->reply.charsofdata;
	if (!kbdcount)
		return(0);
	/* do getc(stdin) */
	c = getc(stdin);
	return (c);
}



fillhostbuffer()
{

	register u_char c, cnext;
	Message msg;

	rp = rbuffer;
	frcv.bodyaddr = rbuffer;
	frcv.bodylen = 6144;
	(void)set_rglout_ptr(&frcv);
	while (context == GRAPHICS && !(rc = update3270())) {
		c = vgetc();
		if (c) {
			if (c == 0x7f)/* was 80,cntl shift ~ */
				sendbreakchar();
			else if (c == ESC) {	/* escape */
				c = vgetc();
				if (c == 0x7f)
					sendbreakchar();
				else
					sendkbdchar(c & 0177);
			}
		}
	}
	if( rc == ERROR ) {
	    printf("errno %d ftdone %d\n\r",errno, outb.ftdone);
	    printf("Fillbuffer: error reading IBM 3270\n\r");
	    return -1;
	}
	if (context != GRAPHICS)
		return -1;
	
/*

printf("R%d ",rc);
{
int i;
for ( i=0;i<rc;i++)
printf("%x ",rbuffer[i]);
}
*/

	rc--;
	return *rp++;
}


flushhostbuffer()
{

	int i;
	if( wc == 0 )
		return;
	if(ttymedium == I3270_COM) {
		if((i = send_binary_str(wbuffer,wc)))
			messagef("Flushbuffer: error %d writing IBM\n\r",i);
	}
/*
printf("W%d ",wc);
*/
	wp = wbuffer;
	wc = 0;
}


sendkbdchar(c)
int c;
{
    char onechar;

    onechar = c & 0x7f;
    printf("send %02x ",onechar);
	send_binary_str(&onechar,1);
}


/*
**	initserial - Open the serial line for reading and writing.
**
*/
initserial()
{
	CreateInstanceRequest req;
#ifndef GL2
 	SystemCode serror;
 	File *_Open(), *OpenFile();
#endif GL2

	req.requestcode = CREATE_INSTANCE;
	req.filenamelen = 6;
	req.filename = "Serial";
	req.type = SERIAL;
	req.unspecified[0] = 1;

#ifdef GL2
	serialout = fopen("/dev/tty2","w+");
	if (serialout <= 0) {
	 	message("Error in Serial Write Open\r\n");
		messagef("errno is 0x%x\n",errno);
	 	return 0;
#else
	serialout = _Open(&req, FCREATE,GetPid(DEVICE_SERVER,LOCAL_PID), &serror);
 	if (serror || (serialout == NULL)) {
		message("Error in Serial Write Open\r\n");
 		return 0;
#endif GL2
	}
#ifdef GL2
	serialin = serialout;
#else
 	serialin = OpenFile( GetPid(DEVICE_SERVER,LOCAL_PID),
 	FileId(serialout)+1, FREAD, &serror);
 	if (serror || (serialin == NULL)) {
		message("Error in Serial Read Open\r\n");
 		return 0;
 	}
 	while( serialin->readindex < serialin->bytes )
 		rbuffer[rc++] = serialin->buffer[serialin->readindex++];
#endif GL2
	return 1;
}


setlocalecho( value )
int value;
{
	return 1;
}

setkeypadmode( value )
int value;
{
    if(value) 
	return setserialmode( MOD_STARTALTMODE );
    else
	return setserialmode( MOD_ENDALTMODE );
}


setserialmode( modcode, modarg )
int modcode;
int modarg;
{
    ModifySerialRequest req;

    req.requestcode = MODIFY_FILE;
#ifdef GL2TERM
    req.fileid = fileno(stdin);
#else
    req.fileid = FileId(stdin);
#endif GL2TERM
    req.linenum = 0;
    req.modcode = modcode;
    req.modarg = modarg;
    if( Send(&req,GetPid(DEVICE_SERVER,LOCAL_PID)) == 0 ) {
	printf("SetSerialMode: error\n");
	return 0;
    }
    return 1;
}


sendbreakchar()
{

	send_x_key(X_PA1);
	xgexit();
}


static char outlock = 0;

/*
**	lockoutput - lock output stream
**
*/
lockoutput(addr)
char *addr;
{
/*    while( !lock(&outlock) ) 
	Delay(0,1);*/
}


/*
**	unlockoutput - unlock output stream
**
*/
unlockoutput()
{
    outlock = 0;
}
