# include "Qdevices.h"
# include "Qglobals.h"
# include "common.h"
# include "Xns.h"
		

# undef DEBUG do_debug
# include "dprintf.h"


#define PRIMADDR_DEFAULT	7
#define READSIZE		32
#define BOOTCHAR		'B'

int
gpib_open(ext,file)
    char *ext,*file;
{
    extern char *skipnum();

    char str[3 + sizeof _commdat->bootstr];
    register int my488addr;
    long junk;

    bootstr(str,sizeof str,ext,file);

    if( *skipnum(ext,10,&junk) != 0 )
    {
	illegalnum(ext);
	return -1;
    }
    if( *ext == 000 )
	my488addr = PRIMADDR_DEFAULT;
    printf("(GPIB boot, MLA = %d)\n",my488addr);

    _commdat->flags |= ADDR488_VALID;
    _commdat->addr488 = my488addr;

    return gstart(my488addr,str);
}

gpib_close()
{
}

int
gpib_read(_ptr,len)
    char (**_ptr);
    int len;
{
    return read488(*_ptr,len);
}



/*
**
** protocol:
**
**	IRIS					WorkStation
**	----					___________
**
**	SRQ to boot	
**						do Serial poll
**	write confstr
**						read confstr
**						write reply ( )
**	read response
**						write bootfile
**	read bootfile
*/
gstart(primaddr,confstr)
    int primaddr;
    char *confstr;
{
	char read488databuf[READSIZE];


	if ( init488(primaddr) < 0 ) {
		printf("? Can't init GPIB iface\n");
		return( -1 );
	}

	if ( srq488(BOOTCHAR) < 0 || !polled488() ) {
		printf("? No response to SRQ\n");
		return( -1 );
	}

	if ( write488(confstr,strlen(confstr)) < 0 ) {
		printf("? Timeout writing boot request\n");
		return( -1 );
	}

	if ( read488(read488databuf,READSIZE ) < 0 ) {
		printf("? No response to boot request\n");
		return( -1 );
	}
	dprintf(("boot server returned %c\n",read488databuf[0]));

	switch( read488databuf[0] ) {

	case SERV_REPLY:
		dprintf(("has file\n"));
		/* download(I488_DL); */
		return( 0 );

	case SERV_NOFILE:
		printf("File not found\n");
		return( -1 );

	default:
		printf("? Garbage boot reply\n");
		return( -1 );
	}
}
