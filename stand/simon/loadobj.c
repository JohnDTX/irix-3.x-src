/*
* $Source: /d2/3.7/src/stand/simon/RCS/loadobj.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:50 $
*/

#include	"sys/types.h"
#include	"cpureg.h"
#include	"common.h"
#include	"errno.h"
#include	"a.out.h"
#include	"sysmacros.h"
#include	"dprintf.h"

#define	BTPGSTART	0x3e00		/* 62megabytes			*/
#define	BTPGMAX		(ONEMEGPG << 1)	/* currently 2 megabytes	*/
#define HIGHLOAD	0x50000

extern int Inprom;

struct exec	execbuf;

/*
** loadpgm
**   load a bootable file.  We only load 0407 type object files.
**   The loadable file must run in the kernel segment.
*/
loadobj( file )
char	*file;
{
	extern		bss_sz;
	int		fd;
	register int	cnt;
	char		*ptr;
	register long	*fptr,
			*tptr;
	long		pgmpgs;
	char		*pre, *ext, *path;
	char		*loadname;	 /* fully specified loadname	*/


	if ( ! _commdat->c_memmb ) {
		printf( "%s: Cannot load - no memory installed\n", file );
		return (-1);
	}

	if ( file == 0 ) {
		argc = 1;
		argv[1] = (char *)0;
		loadname = argbuf;
	} else {
		loadname = (char *)(argv[argc-1] + strlen(argv[argc-1]) + 1);
	}

	/* create a full load file specification to pass to open	*/
	splitspec(file, &pre, &ext, &path);
	setdflts(&pre, &ext, &path);


	strcpy(loadname,pre);
	strcat(loadname,".");
	strcat(loadname,ext);
	strcat(loadname,":");
	strcat(loadname,path);

	_commdat->c_argv[0] = loadname;
	if ( ( fd = open( loadname, 0 ) ) < 0 ) {
		perror( loadname );
		goto outerr;
	}
	dprintf(("open of %s returned %d\n",loadname,fd));

	if ( read( fd, &execbuf, sizeof (struct exec) )
						!= sizeof (struct exec) ) {
		printf( "%s: cannot read header\n", loadname );
		goto outerr;
	}


	if ( execbuf.a_magic != OMAGIC )
	{
		if ( N_BADMAG( execbuf ) )
			printf( "%s: unsupported object type (%x)\n", execbuf.a_magic );
		else
			printf( "%s: not in a.out format\n", loadname );
		goto outerr;
	}

	if ( ( execbuf.a_entry & SEG_MSK ) != SEG_OS )
	{
		printf( "%s: entry point is not in kernel segment\n" );
		goto outerr;
	}

	/*
	** calc # of pages in the loadable program, stack is 2 pages worth
	*/
	pgmpgs = btopr( execbuf.a_text + execbuf.a_data + execbuf.a_bss + 0x2000 );

	/*
	** check if too large: larger than kernel space or memory.
FIX THIS CODE!!!
	if ( pgmpgs > BTPGMAX ||
	     ( ( pgmpgs + ONEMEGPG + bss_sz ) >> 8 ) > _commdat->c_memmb )
	{
		errno = EFBIG;
		perror( loadname );
printf("pgmpgs = %x,bss_sz 0x%x, memmb 0x%x\n",
			pgmpgs,bss_sz,_commdat->c_memmb );
		goto outerr;
	}
*/

	/*
	** now copy over the ptes to the area in the map that
	** we are going to use for the loading pgm: last 2meg entries
	** in the map. We always start loading at physical 0.
	*/
	fptr = (long *)PTMAP_BASE;	/* from pointer		*/
/* is this RIGHT??: */
	tptr = (long *)PTMAP_BASE + BTPGSTART;

	for ( cnt = 0; cnt < BTPGMAX; cnt++ )
		*tptr++ = *fptr++;

	if ( ( execbuf.a_entry & ~SEG_MSK ) >= HIGHLOAD ) {
		ptr = (char *)HIGHLOAD;
		printf("256k load - ");
	} else
		ptr = (char *)0;

	printf( "Loading: %s\n", loadname );
	printf( "   Text:  %06x bytes\n", execbuf.a_text );

	/*
	** yes, this assumes the map is setup!!!! It should be!!!!
	*/

	cnt = read( fd, ptr, execbuf.a_text );
	if ( cnt == 0 )
	{
		printf( "%s: premature eof\n", loadname );
		goto outerr;
	}
	if ( cnt < 0 )
	{
		perror( loadname );
		goto outerr;
	}
	if ( cnt != execbuf.a_text )
	{
		printf( "%s: read error\n", loadname );
		goto outerr;
	}

	printf( "   Data:  %06x bytes\n", execbuf.a_data );

	ptr += execbuf.a_text;

	if ( execbuf.a_data )
	{
		cnt = read( fd, ptr, execbuf.a_data );
		if ( cnt == 0 )
		{
			printf( "%s: premature eof\n", loadname );
			goto outerr;
		}
		if ( cnt < 0 )
		{
			perror( loadname );
			goto outerr;
		}
		if ( cnt != execbuf.a_data )
		{
			printf( "%s: read error\n", loadname );
			goto outerr;
		}
	}

	close( fd );

	printf( "   Bss :  %06x bytes ", execbuf.a_bss );

	if ( !Inprom )
		printf("(not cleared)\n");
	else {
		printf("(cleared)\n");
		ptr += execbuf.a_data;
		bzero( ptr, execbuf.a_bss );
	}

	*OS_BASE = (u_char)( BTPGSTART >> 8 );
	_commdat->c_entrypt = execbuf.a_entry;
	_commdat->c_gostk = SEG_OS + ( BTPGMAX * NBPG );

	printf( "Jumping to loaded program @ %x.\n", _commdat->c_entrypt );
	return (1);

outerr:
	close( fd );
	return (-1);
}

#ifdef DEBUG
printexec( ep )
struct exec *ep;
{
	printf("magic 0x%x\ntext 0x%x, data 0x%x, bss 0x%x\n",
		ep->a_magic,ep->a_text,ep->a_data,ep->a_bss);
	printf("syms 0x%x, trsize 0x%x, drsize 0x%x\n",
		ep->a_syms,ep->a_trsize,ep->a_drsize);
	printf("entry 0x%x\n",ep->a_entry);
}
#endif
