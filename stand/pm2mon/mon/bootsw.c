#
/*
 * bootsw.c --
 * boot device table.
 * each row in the table contains the name of a boot device
 * and the entry points expected by the bootload() routine.
 *
 *	int dev_open(ext,file)
 *	    char *ext,*file;
 *	{
 *		take the parsed boot file name minus prefix
 *		and return 0 (success) or -1 (failure).
 *	}
 *
 *	void dev_close()
 *	{
 *		perform cleanup
 *	}
 *
 *	int dev_read(_ptr,len)
 *	    char (**_ptr);
 *	    int len;
 *	{
 *		read up to len bytes. on entry,
 *		_ptr points to a pointer to the target buffer.
 *		return the actual number of bytes read,
 *		and pass back (in *_ptr) their actual location.
 *	}
 *
 */

struct bootdesc
{
    char *name;
    char *desc;
    int (*open)();
    int (*close)();
    int (*read)();
};

# define md_close	disk_close
# define md_read	disk_read
# define ip_close	disk_close
# define ip_read	disk_read
# define mf_close	disk_close
# define mf_read	disk_read

extern int gpib_open(),gpib_close(),gpib_read();
extern int net_open(),net_close(),net_read();
extern int tape_open(),tape_close(),tape_read();
extern int md_open(),md_close(),md_read();
extern int ip_open(),ip_close(),ip_read();
extern int mf_open(),mf_close(),mf_read();
extern int serial_open(),serial_close(),serial_read();
extern int mem_open(),mem_close(),mem_read();
extern int mmem_open(),mmem_close(),mmem_read();

struct bootdesc bootswitch[] =
{
	{"g", "g[.N]     (gpib address N)",
	 gpib_open,gpib_close,gpib_read},
	{"ip","ipN?      (ip disk slave N, partition ?)",
	 ip_open,ip_close,ip_read},
	{"md","mdN?      (md disk slave N, partition ?)",
	 md_open,md_close,md_read},
	{"mf","mfN?      (mf disk slave N, partition ?)",
	 mf_open,mf_close,mf_read},
	{"n", "n[.HOST]  (ethernet host HOST)",
	 net_open,net_close,net_read},
	{"p", "p[.ADDR]  (prom @ ADDR)",
	 mem_open,mem_close,mem_read},
	{"pb","pb[.ADDR] (prom board MB mem @ ADDR)",
	 mmem_open,mmem_close,mmem_read},
	{"s", "s         (serial line)",
	 serial_open,serial_close,serial_read},
	{"t", "t         (qic tape)",
	 tape_open,tape_close,tape_read},
	{"mt","mt        (same as t)",
	 tape_open,tape_close,tape_read},
	{0}
};

int
bootlookup(prefix,_open,_close,_read)
    char *prefix;
    int (**_open)(),(**_close)(),(**_read)();
{
    register struct bootdesc *bp;

    for( bp = bootswitch; bp->name != 0; bp++ )
	if( strcmp(prefix,bp->name) == 0 )
	{
	    *_open = bp->open;
	    *_close = bp->close;
	    *_read = bp->read;
	    return 0;
	}

    return -1;
}

int
boot_specs()
{
    register struct bootdesc *bp;

    for( bp = bootswitch; bp->name != 0; bp++ )
	noteprint(bp->desc);
}
