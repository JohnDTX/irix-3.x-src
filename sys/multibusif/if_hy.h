/*
 *	NSC HYPERchannel with IKON Multibus Interface
 *
 * $Source: /d2/3.7/src/sys/multibusif/RCS/if_hy.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:32:06 $
 */

struct hy_hdr {
	u_short		h_ctl;
	u_short		h_access;
	u_short		h_dest;
	u_short		h_src;
	u_short		h_param;
	u_char		h_type;
	u_char		h_offset;
};

struct hy_swhdr {				/* for transmit only */
	int		sw_retry;
	struct hy_hdr	sw_hdr;
	char		sw_pad[4];
};

#define	HY_HWOFFSET	4

#define	UNIT_NO(x)		((x) & 0xff00)
#define	CHAN_NO(x)		(((x)>>2) & 0x3f)

#define	C_ASSOC_DATA	0x0001
#define	C_SEND_TRUNKS	0xf000			/* trunks to try */
#define	C_RECV_TRUNKS	0x0f00			/* remote trunks for loopback */


struct hy_status {
	u_char		s_status;
	u_char		s_function;
	u_char		s_trunk_response;
	u_char		s_trunk_status;
	u_char		s_recv_response;
	u_char		s_error;
	u_char		s_msg_addr;
	u_char		s_pad;
};

#define	PORT_NUMBER(p)	(((p)->s_status >> 6) & 0x03)

/*
 *	Error Codes in byte 6 of status from adaptor
 */
#define	NSC_MSG_PEND		0x01
#define	NSC_PORT_DOWN		0x04
#define	NSC_TRANS_ABORT		0x14
#define	NSC_ADAPTOR_RES		0x15
#define	NSC_RETRY_COUNT		0x16

struct hy_statistics {
	u_char		st_df0[3];
	u_char		st_df1[3];
	u_char		st_df2[3];
	u_char		st_df3[3];
	u_char		st_cancel[2];
	u_char		st_abort[2];
	u_char		st_rt0[3];
	u_char		st_rt1[3];
	u_char		st_rt2[3];
	u_char		st_rt3[3];
	u_char		st_rev[3];
	u_char		st_adaptor;		/* adaptor unit number */
};

#define	ST_DEAD		 0
#define	ST_STARTUP	 1
#define	ST_IDLE		 2
#define	ST_SET_WAIT	 3
#define	ST_CLR_WAIT	 4
#define	ST_RECV_MSG	 5
#define	ST_RECV_DATA	 6
#define	ST_SEND_LMSG	 7
#define	ST_SEND_RMSG	 8
#define	ST_SEND_MSG	 9
#define	ST_SEND_DATA	10
#define	ST_SEND_LAST	11
#define	ST_STAT_MSG	12
#define	ST_STAT_DATA	13
#define	ST_STAT_END	14
#define	ST_END		15

/*
 *	Parameters
 */

#define	HY_SCANINTERVAL		2
#define	HY_MAXINTERVAL		10
#define	HY_CHANNELS		64
#define	HY_QSIZE		10
#define	HY_MAXRETRY		4
#define	HY_IP_CHANNEL		0

/*
 *	Message size parameters
 */

#define	HY_OFFSET_IP		4
#define	HY_HDR_SZ		(sizeof(struct hy_hdr) + HY_OFFSET_IP)

#define	HY_MPROP_SIZE		64
#define	HY_ASSOC_SIZE		(1024*64)
#define	HY_LOCAL_SIZE		(1024*4)

#define	HY_MTU			(HY_LOCAL_SIZE+HY_MPROP_SIZE-HY_HDR_SZ)
#define	HY_SWMSG_SIZE		(HY_MPROP_SIZE+HY_HWOFFSET)

#define	HY_OFFSET_MAX		(HY_MPROP_SIZE - sizeof(struct hy_hdr))

#define	HY_RAW_MIN		(sizeof(struct hy_hdr))
#define	HY_RAW_MAX		(HY_MPROP_SIZE+HY_ASSOC_SIZE)
