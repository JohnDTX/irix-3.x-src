/*	@(#)Trtmpl.h	1.3	*/
/*
 * @(#)$Header: /d2/3.7/src/usr.bin/trenter/RCS/Trtmpl.h,v 1.1 89/03/27 18:29:48 root Exp $
 * $Log:	Trtmpl.h,v $
 * Revision 1.1  89/03/27  18:29:48  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  85/02/06  19:11:24  bob
 * Customized for SGI (untested)
 * 
 */
#include	"Objdefs.h"
#include	"Trhelp"

#define	F_EDIT	\
"\nYou may now use edit commands to fix any data you have entered\n\
(hit CR to continue)"

DOBJECT(TrobRpt)
SFILE,	TRSAVEF,

"_lnum_",	OUT,	"Local Trouble Report Number",
"_date_",	OUT,	"Date of Trouble Report",
"NAME",		IO3,	"Name",		"[%FN]",  REQ,  HELP,  H_NAME,
"CO",		IO3,	"Company",	"[%FN]",  REQ,  HELP,  H_CO,
"PHONE",	IO3,	"Phone",	"[%FN]",  REQ,  HELP,  H_PHONE,
"ROOM",		IO3,	"Room Number",	"[%FN]",  REQ,  HELP,  H_ROOM,
"ADDR",		IO3,	"Address",	"[%FN]",  REQ,  HELP,  H_ADDR,
"CITY",		IO3,	"City",		"[%FN]",  REQ,  HELP,  H_CITY,
"STATE",	IO3,	"State",	"[%FN]",  REQ,  HELP,  H_STATE,
"ZIP",		IO3,	"Zip Code",	"[%FN]",  REQ,  HELP,  H_ZIP,
"COUNTRY",	IO3,	"Country",	"[%FN]",  REQ,  HELP,  H_COUNTRY,
"CID",		IO3,	"Customer ID",	"[%FN]",  REQ,  HELP,  H_CID,
"SID",		IO3,	"Site ID",	"[%FN]",  REQ,  HELP,  H_SID,
"CPUNO",	IO3,	"CPU Serial Number",
					"[%FN]",  REQ,  HELP,  H_CPUNO,
"TYPE",		IO3,	"Trouble Report Type",
					"[%FN]",  REQ,  HELP,  H_TYPE,
					VERIFY,	"doc|enh|sw|hdw|fw|cs|unk",
"MACH",		IO3,	"Machine Type",	"[%FN]",  REQ,  HELP,  H_MACH,
"OS_REL",	IO3,	"Operating System Release",
					"[%FN]",  REQ,  HELP,  H_OS_REL,
"PROD",		IO3,	"SGI Product Name",
					"[%FN]",  REQ,  HELP,  H_PROD,
"PROD_REL",	IO3,	"Product Release",
					"[%FN]",  REQ,  HELP,  H_PROD_REL,
					PRE,	  "PROD:!unix",
"SEV",		IO3,	"Severity",	"[%FN]",  REQ,  HELP,  H_SEV,
					POST,	  "postmsg", VERIFY,  "1|2|3|4",
"RDATE",	IO3,	"Required Date",
					"[%FN]",  REQ,  HELP, H_RDATE,
					PRE,	  "SEV:2",    VERIFY, "tverify",
"ABS",		IO3,	"Abstract",	"[%FN]",  REQ,  HELP, H_ABS,
"DESC",		IO3,	"Description",	"[%FN]",  REQ,  HELP, H_DESC, MULTI,
"ATT",		IO3,	"Attachments",	"[%FN] (y or n)",
		POST,	"postmsg",       REQ,	  HELP, H_ATT,
"_xed_",	IN,	F_EDIT,
"_logn_",	OUT,	"Login",
"_uun_",	OUT,	"Uucp Node",
