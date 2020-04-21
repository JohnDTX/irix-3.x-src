/*
 *	NSC HYPERchannel with IKON Multibus Interface
 *
 * $Source: /d2/3.7/src/sys/multibusif/RCS/if_hyreg.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:32:07 $
 */

struct hydevice {
	u_short		d_csr;		/* control and status register */
	u_short		d_cmd;		/* latched command register */
	u_short		d_xx1;
	u_short		d_pulse;	/* pulse command register */
	u_long		d_xx2;
	u_long		d_xx3;
	u_short		d_dma_cntl;	/* DMA Control */
	u_short		d_lo_addr;	/* Multibus low 16 bits of address */
	u_short		d_lo_cnt;	/* Word Count lower 16 bits */
	u_short		d_xx4;
	u_short		d_xx5;
	u_short		d_hi_addr;	/* Multibus high 16 bits of address */
	u_short		d_hi_cnt;	/* Word Count High */
	u_short		d_xx6;
};

#define	CS_GO		0x0001		/* Indicate presence of new command */
#define	CS_IWC		0x0002
#define	CS_IATTN	0x0008
#define	CS_IE		0x0040		/* enable interrupts */
#define	CS_RESET	0x1000		/* reset interface (MCLR) */
#define	CS_INTRESET	0x4000		/* reset interrupt flag (RATN) */
#define	CS_DMARESET	0x8000		/* reset DMA END flag */

#define	CS_READY	0x0080
#define	CS_ABNORMAL	0x0200
#define	CS_NORMAL	0x0400
#define	CS_MSGPEND	0x0800
#define	CS_ATTF		0x4000

#define	CMD_SEND_RMSG	0x05		/* Transmit remote message */
#define	CMD_SEND_LMSG	0x11		/* Transmit local message */
#define	CMD_SEND_DATA	0x09		/* Transmit data */
#define	CMD_SEND_LAST	0x0d		/* Transmit "last" data */
#define	CMD_RECV_MSG	0x25		/* Receive message */
#define	CMD_RECV_DATA	0x29		/* Receive data */
#define	CMD_STATUS	0x41		/* Get Adaptor Status */
#define CMD_GET_EXT_REG	0x51		/* Get Extension Registers */
#define	CMD_GET_STAT	0xa1		/* get NSC usage statistics */
#define	CMD_CLR_STAT	0xa5		/* get & clear NSC usage statistics */
#define	CMD_MASTER_CLR	0xe0		/* Master clear the adaptor */
#define	CMD_END		0xe4		/* Signal end of operation */
#define	CMD_CLR_WAIT	0xe6		/* Clear wait for message */
#define	CMD_SET_WAIT	0xe8		/* Set wait for message */

#define	CMD_LSTAT	0xf1		/* Ioctl pseudo command */

#define	CLEAR_INTERRUPTS(dr)	((dr)->d_pulse = (CS_INTRESET|CS_DMARESET))
