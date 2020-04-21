/*
* $Source: /d2/3.7/src/stand/include/RCS/net.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:47 $
*/

/*
**			Structures for standalone XNS code
**
**			    Paul Haeberli - July 1983
**
*/
#ifndef NETDEF
#define NETDEF

#include "Xns.h" 

#define FAILURE	(-1)	
#define ERROR	(-1)	
#define NULL 0
#define TRUE 1
#define FALSE 0

#define MAXINT		2147483647

#define	MIN_ENET_PACKET	60
#define	MAX_ENET_PACKET 1514
#define	PKTBUFSIZE	MAX_ENET_PACKET
#define MAXDATA		(1024+100)

/*	Buffer structure	*/

typedef struct pbuf 
  {
    int		number;		/* buffer number, for buffer mgr */
    int		size;		/* max number of bytes of data */
    struct pbuf	*next;		/* link to next pbuf in a Queue */
    int		length;		/* number of valid data bytes */
    char	*dataptr;	/* pointer to first valid data byte */
	/* the next field must start on an even byte */
    char	data [PKTBUFSIZE];	/* actual data bytes are in here */
  } *PktBuf;


#endif
