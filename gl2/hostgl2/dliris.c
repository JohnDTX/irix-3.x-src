/**************************************************************************
 *									  *
 * 		 Copyright (C) 1983, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
**			 serial download program for iris
*/
#include <stdio.h>
#include "boot.h"
#include "remprom.h"	
#include "irisboot.h"

#define INBUFFSIZE		1024
#define OUTBUFFSIZE		1024
char outbuff[OUTBUFFSIZE];
long bp = 0;

main(argc,argv) 
int argc;
char **argv;
{
    short col;

    if(argc<2)
   	download(IRISBOOT);
    else
   	download(argv[1]);
}
 
download(filename)
char filename[];
{
    long infile;
    short nbytes;
    char buffer[INBUFFSIZE];
    struct ibhdr *irisboothead;

    if( (infile=open(filename,0)) == -1 ) {
	printf("error: couldn't open input file %s.\n",filename);	
	return;
    } 
    if( (nbytes = read(infile,buffer,sizeof(buffer))) == -1) {
	printf("dliris: error on read\n");
	return;
    }
    irisboothead = (struct ibhdr *)buffer;
    if( (reverse(irisboothead->fmagic) != IBMAGIC) &&
		(reverse(irisboothead->fmagic) != FMAGIC) ) {
	printf("dliris: bad magic number on boot file %o\n",
						reverse(irisboothead->fmagic));
	return;
    }
    gcmd(PDOWNLOAD);
    sendbytes(buffer,nbytes);
    while( (nbytes=read(infile,buffer,sizeof(buffer))) != 0)
        sendbytes(buffer,nbytes);
    flushg();
}

reverse(lwrd) 
long lwrd;
{
    if(onvax())
	return((lwrd>>24 & 0xff)	| 
	       (lwrd>>8 & 0xff00) 	| 
	       (lwrd<<8 & 0xff0000) 	| 
	       (lwrd<<24) 		);
    else
	return lwrd;
}


/*
**	gcmd - send the graphics escape and the specified command code
**	
*/
gcmd( comcode )
register long comcode;
{
    putgchar(PESC);
    putgchar((comcode&077) + ' ');
    putgchar(((comcode>>6)&077) + ' ');
}

/*
** 	sendbytes - send a block of data
**
*/
sendbytes( buffer, nbytes )
register char buffer[];
register short nbytes;
{
    register short i, nshorts;
    register short ashort, checksum;

    sends(nbytes);
    nshorts = (nbytes+1)>>1;
    checksum = 0;
    for(i=0; i<nshorts; i++) {
	ashort = (buffer[(i<<1)+1] << 8) + (buffer[i<<1] & 0xff);
	checksum += ashort;
	sends(ashort);
    }
    sends(checksum);
}

/*
**	sends - send a short 6+6+4 
**
*/
sends( value ) 
register short value;
{
    putgchar((value&077) + ' ');
    putgchar(((value>>6)&077) + ' ');
    putgchar(((value>>12)&017) + ' ');
}

putgchar( onechar )
char onechar;
{
    outbuff[bp++] = onechar;
    if(bp > OUTBUFFSIZE)
	flushg();
}

flushg()
{
    write(1,outbuff,bp);
    bp = 0;
}

onvax()
{
    long along;

    along = 1;
    return *(char *)&along;
}
