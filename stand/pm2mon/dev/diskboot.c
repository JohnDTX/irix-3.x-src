#
/*
 * diskboot.c --
 * disk boot routines (using the unix file package).
 * called via the boot switch.
 */
# include "pmII.h"

# include "sys/types.h"
# include "dklabel.h"
# include "ctype.h"



/*----- globals used by disk drivers*/
short	drive;			/* Physical Drive */
long 	boffset;		/* block offset to start of fs */
/*----- */

# ifdef ROOTHACK
char ForceRoot = 'a';
# endif ROOTHACK


struct
{
    int (*init)();
    int (*rblk)();
    int (*clr)();
}   D;


int
disk_open(initfunc, readblk, clrfunc, ext, file)
    int (*initfunc)(), (*readblk)(), (*clrfunc)();
    char *ext, *file;
{
    int partition, slave;
    register char *cp;
    int fstype;

    if( *ext == 000 )
	ext = "0";

    cp = ext;
    slave = 0;
    if( isdigit(*cp) )
	slave = *cp++ - '0';

    partition = -1;
    if( islower(*cp) )
	partition = *cp++ - 'a';

    if( *cp != 000
     || !(-1 <= partition && partition < NFS) )
    {
	printf("Illegal extension \"%s\"\n", ext);
	return -1;
    }

    D.init = initfunc; D.rblk = readblk; D.clr = clrfunc;

    if( disk_start(slave, partition, &fstype) < 0 )
	return -1;

    return openf(file);
}

disk_close()
{
    closef();
    msdelay(214);	/*100000*/
    (*D.clr)();
}

int
disk_read(_ptr, len)
    char (**_ptr);
    int len;
{
    return readf(*_ptr);
}

int
disk_start(slave, partition, _fstype)
    int slave, partition;
    int *(_fstype);
{
    union
    {
	char c[DBLOCK];
	struct disk_label l;
    }   x;
    char found;

    /* set up disk driver globals */
    drive = slave;
    boffset = 0;

    if( (*D.init)(&x.l) )
	return -1;

    /* fix them based on results of init */
    printf("(");

# ifdef ROOTHACK
    if( ForceRoot )
	if( partition < 0 || partition == x.l.d_rootfs )
	{
	    partition = ForceRoot - 'a';
	    printf("partition \"%c\", ", partition+'a');
	}
# endif ROOTHACK

    if( partition < 0 )
    {
	partition = x.l.d_rootfs;
	if( (unsigned) partition >= NFS )
	    partition = 0;
	printf("partition \"%c\", ", partition+'a');
    }

    boffset = x.l.d_map[partition].d_base;

    /* first try bell fs */
    found = bell_probefs(D.rblk, _fstype);

    /* now try bky file system */
    if( !found )
    found = bky_probefs(D.rblk, _fstype);

    /* now try extent file system */
    if( !found )
    found = efs_probefs(D.rblk, _fstype);

    if( !found )
    printf("Unrecognizable FS");

    printf(")\n");
    return found;
}

disk_list(openfunc, ext, file)
    int (*openfunc)();
    char *ext, *file;
{
    if( (*openfunc)(ext, file) < 0 )
	return -1;

    fs_list();

    disk_close();
}
