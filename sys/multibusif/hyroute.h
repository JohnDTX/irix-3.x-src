/*
 *	HYPERchannel routing table definitions
 *
 * $Source: /d2/3.7/src/sys/multibusif/RCS/hyroute.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:59 $
 */

#if 0
struct hyr_header {
	struct if_address	hyr_inet_addr;
	u_short			hyr_base;
	u_short			hyr_count;
	u_short			hyr_index;
};

struct hyr_entry {
	u_char			hyr_flags;
	u_char			hyr_errors;
	u_short			hyr_interface;
	u_short			hyr_phys_addr;		/* Adaptor & Port */
	u_short			hyr_ctl;		/* Trunks to try */
};

#define	HYR_DOWN		0x01

struct hyr_tables {
	u_short			hyr_hash[65];
	struct hyr_header	hyr_header[128];
	struct hyr_entry	hyr_entry[512];
};
#endif

/*
 * Routing database
 */
#define HYRSIZE  100 /* was 37	/* max number of adapters in routing tables */

struct hy_route {
	time_t hyr_lasttime;		/* last update time */
	struct hyr_hash {
		u_long	hyr_key;	/* desired address */
		u_short hyr_flags;	/* status flags - see below */
		union {
			/*
			 * direct entry (can get there directly)
			 */
			struct {
				u_short hyru_dst;	/* adapter number & port */
				u_short hyru_ctl;	/* trunks to try */
				u_short hyru_access;	/* access code (mostly unused) */
			} hyr_d;
#define hyr_dst		hyr_u.hyr_d.hyru_dst
#define hyr_ctl		hyr_u.hyr_d.hyru_ctl
#define hyr_access	hyr_u.hyr_d.hyru_access
			/*
			 * indirect entry (one or more hops required)
			 */
			struct {
				u_char hyru_pgate;	/* 1st gateway slot */
				u_char hyru_egate;	/* # gateways */
				u_char hyru_nextgate;	/* gateway to use next */
			} hyr_i;
#define hyr_pgate	hyr_u.hyr_i.hyru_pgate
#define hyr_egate	hyr_u.hyr_i.hyru_egate
#define hyr_nextgate	hyr_u.hyr_i.hyru_nextgate
		} hyr_u;
	} hyr_hash[HYRSIZE];
	u_char hyr_gateway[256];
};

/*
 * routing table set/get structure
 *
 * used to just pass the entire routing table through, but 4.2 ioctls
 * limit the data part of an ioctl to 128 bytes or so and use the
 * interface name to get things sent the right place.
 * see ../net/if.h for additional details.
 */
struct hyrsetget {
	char	hyrsg_name[IFNAMSIZ];	/* if name, e.g. "hy0" */
	struct hy_route *hyrsg_ptr;	/* pointer to routing table */
	unsigned	hyrsg_len;	/* size of routing table provided */
};

#define HYR_INUSE	0x01	/* entry in use */
#define HYR_DIR		0x02	/* direct entry */
#define HYR_GATE	0x04	/* gateway entry */
#define HYR_LOOP	0x08	/* hardware loopback entry */
#define HYR_RLOOP	0x10	/* remote adapter hardware loopback entry */

#define HYRHASH(x) (((x) ^ ((x) >> 16)) % HYRSIZE)

#define HYSETROUTE	_IOW(i, 0x80, struct hyrsetget)
#define HYGETROUTE	_IOW(i, 0x81, struct hyrsetget)
