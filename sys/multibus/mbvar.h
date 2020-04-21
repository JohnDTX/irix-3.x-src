/*
 * For each "controller" on the multibus, there is a multibus controller
 * structure defined by the config program.  A "controller" is a device
 * which can have multiple "drives" (or "units") attached to it.
 */
struct	mb_ctlr {
	struct	mb_driver *mm_driver;	/* per driver info */
	u_char	mm_ctlr;		/* controller # */
	u_char	mm_alive;		/* did it probe? (NOT USED) */
	long	mm_addr;		/* address of device */
	u_char	mm_priority;		/* interrupt priority */
	long	mm_flags;		/* controller flags */
};

/* return values from driver probe/attach routines */
#define	CONF_DEAD	1
#define	CONF_FAULTED	2
#define	CONF_ALIVE	3

/*
 * For each "device" on the multibus, there is a data structure
 * intialized by the config program, which contains all the information
 * needed to operate that particular device.  The information in this
 * structure is passed to each "drive" when it is attached to its
 * controller, or to each "device" when it is being configured.
 */
struct	mb_device {
	struct	mb_driver *mi_driver;	/* per driver information */
	u_char	mi_unit;		/* which logical unit this is */
	char	mi_ctlr;		/* which controller this belongs too */
	u_char	mi_slave;		/* which physical unit this is */
	long	mi_addr;		/* address of device */
	u_char	mi_priority;		/* interrupt priority */
	char	mi_dk;			/* driver # for iostat */
	long	mi_flags;		/* flag information */
	short	mi_alive;		/* is device alive? */
};

/*
 * Each multibus device driver defines entries for a set of routines used
 * by the multibus driver to coordinate bus activity.
 */
struct	mb_driver {
	int	(*md_probe)();		/* controller probe routine */
	int	(*md_attach)();		/* device attach routine */
	int	(*md_ustart)();		/* unit start routine */
	int	(*md_start)();		/* data transfer start routine */
	int	(*md_intr)();		/* interrupt routine */
	char	*(*md_sname)();		/* device name routine */
	char	*md_dname;		/* simple name (not always right) */
	struct	mb_device **md_dinfo;	/* backpointers to mbdinit structs */
	char	*md_cname;		/* generic controller name */
	struct	mb_ctlr **md_cinfo;	/* backpointers to mbcinit structs */
};

#ifdef	KERNEL
struct	mb_ctlr mbcinit[];
struct	mb_device mbdinit[];

long	mbmapkget();
#endif
