/*
** $Source: /d2/3.7/src/stand/cmd/ipfex/RCS/iperrlist.h,v $
** $Date: 89/03/27 17:12:23 $
** $Revision: 1.1 $
*/

/*
 * ip.h	- Copyright (C) JCS Computer Services - Sunnyvale CA 94089
 *		- Chase - December 1983
 *		- Any use, copy or alteration is strictly prohibited.
 * $Source: /d2/3.7/src/stand/cmd/ipfex/RCS/iperrlist.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:12:23 $
 */

#define ERROFFSET	0x10
char *iperrlist[] = {
/*10-15*/  "Not Ready","Invalid Address","Seek Error","ECC Data","Invalid CMD",
/*15-1a*/  "Not Used","Invalid Sec","Not Used","Bus Timeout","Not Used",
/*1a-1c*/  "Write Protected","Unit Not Selected","No Address Mark",
/*1d-1e*/  "Not Used","Drive Faulted",
/*1f-23*/  "Not Used","Not Used","Not Used","Not Used","Uncorrectable Error",
/*24-28*/  "Not Used","Not Used","No sector Pulse","Data Overrun","No Index",
/*29-2d*/  "Sector Not Found","ID Field Wrong","Invalid Sync","Seek Timeout",
/*2e-31*/  "Busy Timeout","Not On Cylinder","RTZ Timeout","Format Overrun",
/*32-37*/  "Not Used","Not Used","Not Used","Not Used","Not Used","Not Used",
/*38-3d*/  "Not Used","Not Used","Not Used","Not Used","Not Used","Not Used",
/*3e-3f*/  "Not Used","Not Used",
/*40-41*/  "Not Inited","Gap Error",
/*42-47*/  "Not Used","Not Used","Not Used","Not Used","Not Used","Not Used",
/*48-4a*/  "Not Used","Not Used","Not Used",
/*4b-4c*/  "Seek Error","Map Hd Err",
/*4d-4f*/  "Not Used","Not Used","Not Used",
/*50-53*/  "Sec/Trk Err","Byt/Sec Err","ILV Err","Invalid Head",
/*54-59*/  "Not Used","Not Used","Not Used","Not Used","Not Used","Not Used",
/*5a-5c*/  "Not Used","Not Used","Not Used",
/*5d   */  "Invalid DMA Count",
	    0
};

#define CMDOFFSET	0x80
char *ipcmdlist[] = {
/* */  	"Not Used","Read","Write","Verify","Format","Map",
/* */	"Report Configuration","Initialize","Not Used",
/* */	"Restor","Seek","Not Used","Not Used","Not Used","Not Used",
/* */	"Reset","Not Used","Direct Read","Direct Write","Read Absolute",
/* */  	"Read Non-Cached",
	0
};
