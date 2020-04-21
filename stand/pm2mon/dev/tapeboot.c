#
/*
 * tapeboot.c --
 * tape boot (using the cpio package).
 * called via the boot switch.
 */
# include "sys/types.h"

# define TAPEBSIZE	512



int
tape_open(ext,file)
    char *ext,*file;
{
    extern int prom_taperead();
    extern int cpio_fullmatch();

    if( prom_tapeinit(0) )
	return -1;

    if( cpio_init(prom_taperead,TAPEBSIZE) < 0
     || cpio_scan(file,cpio_fullmatch) < 0 )
    {
	tape_close();
	return -1;
    }

    return 0;
}

tape_close()
{
    /* minit is used to clear the pending interrupt */
    minit();

    cpio_close();
}

int
tape_read(_ptr,len)
    char (**_ptr);
    int len;
{
    return cpio_read(*_ptr,len);
}

tape_list(openfunc,ext,file)
    int (*openfunc)();
    char *ext,*file;
{
    extern int prom_taperead();
    extern int cpio_partmatch();

    if( prom_tapeinit(0) )
	return;

    if( cpio_init(prom_taperead,TAPEBSIZE) < 0
     || cpio_scan(file,cpio_partmatch) < 0 )
	;

    tape_close();
}
