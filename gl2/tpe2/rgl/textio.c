/*	
**	sdemux - functions to support reading and writing of sdemux
**
*/
#include "Vdevtypes.h"
#include "Vioprotocl.h"
#include "Vserial.h"
#include "grioctl.h"
#include "term.h"

static	int 	deviceserverpid;

/*
 *	open - open a file for i/o
 *
 */
text_open(name,mode)
char *name;
int mode;
{
    int msg[8];
    register CreateSerialInstanceRequest *req;
    register CreateInstanceReply *reply;

    req = (CreateSerialInstanceRequest *) msg;
    reply = (CreateInstanceReply *) msg;
    req->requestcode = CREATE_INSTANCE;
    req->type = WINDOW;
    req->filemode = FCREATE;
    req->filename = "sdemux";
    req->filenamelen = 6;
    req->linenum = 0;
    deviceserverpid = GetPid(DEVICE_SERVER,LOCAL_PID);
    if ( Send(req,deviceserverpid)==0 )
	  return 0;
    if( reply->replycode != OK )
	  return 0;
    return (int)reply->fileid;
}

/*
 * 	close - close a file
 *
 */
text_close( d )
int d;
{
    IoRequest req;

    req.requestcode = RELEASE_INSTANCE;
    req.fileid = (InstanceId)d;
    if( Send(&req,deviceserverpid) == 0)
	  return -1;
    if( req.requestcode != OK )
	  return -1;
    return 1;
}

/*
 *	read - read bytes from a file 
 *
 */
text_read( d, buf, nbytes )
int d;
char buf[];
int nbytes;
{
   int msg[8];
   register IoRequest *req;
   register IoReply *reply;
 
   req = (IoRequest *) msg;
   reply = (IoReply *) msg;
   req->requestcode = READ_INSTANCE;
   req->fileid = (InstanceId) d+1;
   req->bufferptr = buf;
   req->bytecount = nbytes;
   if( Send(req,deviceserverpid) == 0)
         return -1;
   if( reply->replycode != OK )
      	 return -1;
   blt(buf,reply->shortbuffer,reply->bytecount);
   return reply->bytecount;
}

/*
 *	write - write to a file 
 *
 */
text_write( d, buf, nbytes )
int d;
char buf[];
int nbytes;
{
    int msg[8];
    register IoRequest *req;
    register IoReply *reply;
    register int i;

    req = (IoRequest *) msg;
    reply = (IoReply *) msg;
    req->requestcode = WRITE_INSTANCE;
    req->fileid = (InstanceId)d;
    req->bufferptr = buf;
    req->bytecount = nbytes+100;	/* warning: TOTAL hack */
    if( Send(req,deviceserverpid) == 0)
        return -1;
    if( reply->replycode != OK )
        return -1;
    return reply->bytecount;
}
