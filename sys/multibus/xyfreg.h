/*
 * Data structures and constants for the Xylogics 421 st506/qic-02 disk-tape
 * controller.  All the structures defined in this file are char swapped
 * for the 68000.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/multibus/RCS/xyfreg.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:49 $
 */

/* multibus i/o register block */
struct	xyreg {
	u_char	rrhigh;		/* +01 relocation register high byte (rw) */
	u_char	rrlow;		/* +00 relocation register low byte (rw) */
	u_char	arhigh;		/* +03 address register high byte (rw) */
	u_char	arlow;		/* +02 address register low byte (rw) */
	u_char	dummy;		/* +05 byte swapping padding (not used) */
	u_char	csr;		/* +04 control and status register (read) */
};

/* xy_csr bit definitions */
#define	CSR_ERR		0x80		/* general hard error */
#define	CSR_DERR	0x40		/* double error */
#define	CSR_ADRM	0x08		/* addressing mode */
#define	CSR_IPND	0x04		/* interrupt pending */
#define	CSR_AACK	0x02		/* attention acknowledge */
#define	CSR_CRDY	0x01		/* controller ready */

/* xy_ccr bit defintions */
#define	CCR_CLRE	0x80		/* clear general hard error */
#define	CCR_IAA		0x40		/* interrupt on AACK */
#define	CCR_CRST	0x20		/* controller reset */
#define	CCR_IEC		0x10		/* interrupt at end of chain */
#define	CCR_BWM		0x08		/* byte or word mode */
#define	CCR_CLRI	0x04		/* clear interrupt pending */
#define	CCR_AREQ	0x02		/* attention request */
#define	CCR_GO		0x01		/* go */

/* io parameter block in multibus memory */
struct	iopb {
	u_char	i_command;		/* +01 command */
	u_char	i_unit;			/* +00 device type/unit select */
	u_char	i_linkhigh;		/* +03 link to next iopb, high byte */
	u_char	i_linklow;		/* +02 link to next iopb, low byte */
	u_char	i_csb1;			/* +05 controller status byte 1 */
	u_char	i_csb0;			/* +04 controller status byte 0 */
	u_char	i_sector;		/* +07 sector address */
	u_char	i_head;			/* +06 head address */
	u_char	i_cylhigh;		/* +09 cylinder address high */
	u_char	i_cyllow;		/* +08 cylinder address low */
	u_char	i_scchigh;		/* +0B sector count high */
	u_char	i_scclow;		/* +0A sector count low */
	u_char	i_bufhigh;		/* +0D memory data address high */
	u_char	i_buflow;		/* +0C memory data address low */
	u_char	i_relhigh;		/* +0F memory data relocation high */
	u_char	i_rellow;		/* +0E memory data relocation low */
	u_char	i_mode;			/* +11 mode */
	u_char	i_throttle;		/* +10 dma burst length */
	u_char	i_headoffset;		/* +13 head offset */
	u_char	i_subfunction;		/* +12 subfunction code */
	u_char	i_eccmaskhigh;		/* +15 ecc mask pattern high */
	u_char	i_eccmasklow;		/* +14 ecc mask pattern low */
	u_char	i_eccbitaddrhigh;	/* +17 ecc bit address high */
	u_char	i_eccbitaddrlow;	/* +16 ecc bit address low */
};

/* redefines for other usage of iopb entries */
#define	i_driveoption	i_head		/* drive option */
#define	i_maxsector	i_sector	/* max sector */
#define	i_maxcyllow	i_cyllow	/* max cylinder low */
#define	i_maxcylhigh	i_cylhigh	/* max cylinder high */
#define	i_drivestatus	i_scclow	/* read drive status */
#define	i_revision	i_scchigh	/* firmware revision code */
#define	i_bpslow	i_buflow	/* bytes per sector low */
#define	i_bpshigh	i_bufhigh	/* bytes per sector high */
#define	i_rwcsclow	i_rellow	/* reduced write current/precomp
					   starting cylinder low */
#define	i_rwcschigh	i_relhigh	/* reduced write current/precomp
					   starting cylinder high */
/* tape redefines */
#define	t_bclow		i_scclow	/* tape byte count low */
#define	t_bchigh	i_scchigh	/* tape byte count high */
#define	t_sb0		i_head		/* tape status byte 0 */
#define	t_sb1		i_sector	/* tape status byte 1 */
#define	t_sb2		i_cyllow	/* tape status byte 2 */
#define	t_sb3		i_cylhigh	/* tape status byte 3 */
#define	t_sb4		i_scclow	/* tape status byte 4 */
#define	t_sb5		i_scchigh	/* tape status byte 5 */

/* i_unit (drive type) definitions */
#define	IU_WINNY	0x20		/* drive type of winchester */
#define	IU_TAPE		0x40		/* drive type of tape */
#define	IU_LTC		0x08		/* lock tape cartridge */
#define	IU_RCD		0x04		/* removable cartridge disk */
#define	IU_UNITMASK	0x03		/* mask to get unit bits */

/* i_command defintions */
#define	IC_AUD		0x80		/* auto update iopb */
#define	IC_CHEN		0x40		/* chain enable */
#define	IC_ITI		0x20		/* interrupt on this iopb */
#define	IC_NOP		0x00		/* no operation */
#define	IC_WRITE	0x01		/* write data */
#define	IC_READ		0x02		/* read data */
#define	IC_WRITEHEADER	0x03		/* write track header (disk only) */
#define	IC_READHEADER	0x04		/* read track header (disk only) */
#define	IC_SEEK		0x05		/* seek/position */
#define	IC_DRIVERESET	0x06		/* drive reset */
#define	IC_FORMAT	0x07		/* format disk (disk only) */
#define	IC_FILEMARK	0x07		/* write file mark (tape only) */
#define	IC_READSTATUS	0x08		/* read drive status */
#define	IC_SETPARAMS	0x09		/* set drive parameters (disk only) */
#define	IC_SELFTEST	0x0A		/* self test */
#define	IC_MAINTENANCE	0x0B		/* maintenance (disk only) */

/* i_csb0 defintions */
#define	IS0_ERSM	0x80		/* error summary */
#define	IS0_CTYPMASK	0x1E		/* controller type mask */
#define	IS0_CTYPSHIFT	1		/* shift for controller type */
#define	IS0_DONE	0x01		/* iopb done */

/* t_sb0 TO BE DEFINED */
/* t_sb1 TO BE DEFINED */
/* t_sb2 TO BE DEFINED */
/* t_sb3 TO BE DEFINED */
/* t_sb4 TO BE DEFINED */
/* t_sb5 TO BE DEFINED */

/* i_mode defintions */
#define	IM_NUE		0x20		/* no update on error */
#define	IM_RTRY		0x10		/* ready retry */
#define	IM_ASR		0x08		/* auto seek retry */
#define	IM_SO		0x04		/* sequential order iopb's */
#define	IM_ECM_CORRECT	0x02		/* correct ecc errors */

/* i_driveoption definitions */
#define	DO_OLSK		0x80		/* overlapped seek */
#define	DO_HST_3ms	0x00		/* 3ms head step time */
#define	DO_HST_15us	0x20		/* 15us head step time */
#define	DO_HST_7us	0x40		/* 7us head step time */

/* Miscellaneous variables for the disk */
#define BLKSIZE		512		/* Size for the physical sectors */
#define BLKSHIFT	9		/* log2(BLK_SIZE) for shifting */

/* Macros for the high -- mid -- low bytes of longs */
#define LB(x)	((long)(x) & 0xff)		/* Low Byte */
#define MB(x)	(((long)(x) >> 8) & 0xff)	/* Mid Byte */
#define HB(x)	(((long)(x) >> 16) & 0x0f)	/* High Byte (HACK) */

#ifdef KERNEL

#define SPL()		spl5()		/* Interrupt Level */
#define MAX_DRIVES	2		/* Maximum Number of Disk drives */

/* sc_flags bits */
#define SC_ALIVE	0x01

/* per drive data base */
struct	softc_disk {
	short	sc_flags;		/* drive state bits */
	short	sc_spc;			/* sectors per cylinder */
	struct 	disk_map sc_fs[NFS];	/* file system map */
	short	sc_cyl;			/* cylinders on drive */
	char	sc_sec;			/* sectors per track */
	char	sc_hd;			/* heads on drive */
};

/* controller data base */
struct	softc {
	short	sc_flags;		/* controller state bits */
	struct	xyreg *sc_reg;		/* io registers */
	struct	iopb sc_iopb;		/* Multibus iopb */
	struct	iobuf sc_tab;		/* work/active queue */
	struct	buf sc_diskbuf;		/* raw buffer header */
	struct	softc_disk sc_disk[MAX_DRIVES];
} xyfsoftc;

#endif
