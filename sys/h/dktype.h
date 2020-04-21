/*
 * Generic disk type names for pretty printout
 *
 * $Source: /d2/3.7/src/sys/h/RCS/dktype.h,v $
 * $Date: 89/03/27 17:29:21 $
 * $Revision: 1.1 $
 */

struct dk_type {
	u_long	d_type;
	char	*d_name;
};

/* this function searchs the disk type list for the given disk */
char	*prdtype();

struct dk_type dk_dtypes[] = {
	{ DT_3046,	"Atasi 3046" },
	{ DT_V170,	"Priam V170" },
	{ DT_2312,	"Fujitsu 2312" },
	{ DT_2351A,	"Fujitsu 2351A" },
	{ DT_1085,	"Maxtor 1085" },
	{ DT_WRENII,	"Wren II" },
	{ DT_WREN3,	"Wren III" },
	{ DT_V185,	"Priam V185" },
	{ DT_5118,	"Hitachi 511-8" },
	{ DT_1140,	"Maxtor 1140" },
	{ DT_1325,	"Micropolis 1325" },
	{ DT_V130, 	"Priam V130" },
	{ DT_2243, 	"Fujitsu 2243" },
	{ DT_1055,	"NEC 1055" },
	{ DT_T101, 	"Tandon TM101" },		/* OBSOLETE */
	{ DT_TM252, 	"Tandon TM252" },
	{ DT_QUME, 	"Qume 592" },			/* OBSOLETE */
	{ DT_96202, 	"AST 96202" },
	{ DT_96203, 	"AST 96203" },
	{ DT_D570, 	"Cynthia D570" },
	{ DT_3212, 	"Miniscribe 3212" },
	{ DT_WRENESDI, 	"Wren II" },
	{ DT_TM362, 	"Tandon TM262" },
	{ DT_4175, 	"Maxtor 4175" },
	{ DT_5128, 	"Hitachi 512-8" },
	{ DT_51212, 	"Hitachi 512-12" },
	{ DT_51217, 	"Hitachi 512-17" },
	{ DT_AIMSMD, 	"Aim 130" },
	{ DT_3426, 	"Mitsubishi 3426" },
	{ DT_D5126, 	"NEC D5126" },
	{ DT_2246,	"Fujitsu 2246" },
	{ DT_2085,	"Maxtor 2085" },
	{ DT_4380,	"1550-15" },
	{ DT_1100,	"Siemens 1100" },
	{ DT_1200,	"Siemens 1200" },
	{ DT_1300,	"Siemens 1300" },
	{ DT_156FA,     "Toshiba 156FA" },
	{ DT_MK56,      "Toshiba MK56FB" },
	{ DT_9766,	"CDC 9766" },
	{ DT_2249,	"Fujitsu 2249" },
	{ DT_51438,	"Hitachi 514-38" },
	{ DT_513,	"AMS 513" },
};
#define NTYPES 	(sizeof (dk_dtypes) / sizeof (struct dk_type))

#ifndef	KERNEL
struct dk_type dk_dcont[] = {
	{ DC_DSD5217,		"Qualogy 5217" },
	{ DC_XYLOGICS450,	"Xylogics 450" },
	{ DC_INTERPHASE2190,	"Interphase 2190" },
	{ DC_STORAGER,		"Interphase 3030" },
};
#define NCONTS 	(sizeof (dk_dcont) / sizeof (struct dk_type))
#endif
