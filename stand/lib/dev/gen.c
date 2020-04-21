/*
* $Source: /d2/3.7/src/stand/lib/dev/RCS/gen.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:14:30 $
*/

#include "stand.h"
#include "sysmacros.h"
#include "ctype.h"
#include "dprintf.h"


struct gendev {
	char		*g_name;
	struct devsw	*g_dp;
};

struct gendev hdinfo[] = {
	{ "ip" },
	{ "sd" },
	{ "md" },
};

struct gendev ctinfo[] = {
	{ "st" },
	{ "mt" },
};

struct gendev fdinfo[] = {
	{ "sf" },
	{ "mf" },
};

hdopen( io, ext, file )
register struct iob *io;
char *ext, *file;
{
	return( genopen( io, ext, file, hdinfo,
			sizeof(hdinfo)/sizeof(hdinfo[0])  ));
}

ctopen( io, ext, file )
register struct iob *io;
char *ext, *file;
{
	return( genopen( io, ext, file, ctinfo,
			sizeof(ctinfo)/sizeof(ctinfo[0])  ));
}

fdopen( io, ext, file )
register struct iob *io;
char *ext, *file;
{
	return( genopen( io, ext, file, fdinfo,
			sizeof(fdinfo)/sizeof(fdinfo[0])  ));
}

genopen( io, ext, file, gp, numdev )
register struct iob *io;
char *ext, *file;
register struct gendev *gp;
int numdev;
{
	register struct devsw	*dp;
	register struct inode	*ip;
	register int i;

	ip = io->i_ip;

	/* match the devices */
	for ( i = 0; i < numdev; i++, gp++ ) {
		for ( dp = devsw; dp->dv_name; dp++ ) {
			if ( !strcmp( gp->g_name, dp->dv_name ) ) {
				/* found it */
				gp->g_dp = dp;
				break;
			}
		}
		if ( !dp->dv_name )		/* no luck   try again */
			continue;

		/* now try to open the device */
		io->i_flgs |= dp->dv_flags;
		/* the device open will fix the minor */
		ip->i_dev = makedev( dp - devsw, 0 );

		if ( _devopen( io, ext, file ) < 0 ) {
			continue;
		} else {		/* it openned!! */
			return(0);
		}
	}

	/* no open passed */
	dprintf(("no generic device"));
	io->i_error = ENXIO;
	return(-1);
}


exttodrive(ext)
register char *ext;
{
	register char c;

	/* determine drive and filesystem from extension */
	if ( ext == NULL ) 
		return(0);
	else {
		c = *ext++;
		if ( isdigit( c ) )
			return(c - '0');
		else
			return(-1);
	}
}

exttodev(ext,rootslice)
register char *ext;
u_char rootslice;
{
	register int slice;
	register int drive;
	register int dev;
	register char c;

	if ( rootslice == 0xff )
		rootslice = 0;
	/* determine drive and filesystem from extension */
	if ( ext == NULL ) 
		drive = 0;
	else {
		c = *ext++;
		if ( isdigit( c ) ) 
			drive = c - '0';
		else
			goto error;
	}
	/* now the slice	*/
	c = *ext++;
	if ( c == '\0' ) {
		slice = rootslice;
	} else {
		if ( isupper( c ) )
			c = _tolower( c );
		if ( c < 'a' || c > 'h' )
			goto error;
		slice = c - 'a';
	}
	dev = ( drive << 3 ) | slice;
	dprintf(("dev is 0x%x\n",dev));
	return(dev);

error:
	return(-1);
}
