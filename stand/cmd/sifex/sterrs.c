/*
**	sterrs.c	- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: January 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/sterrs.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:24 $
*/

#include "types.h"
static char nu[] = "(not used)";
static char rs[] = "(reserved)";

char *sterrlist[] = {
/*10-15*/  "Not Ready","Invalid Address","Seek Error","ECC Data","Invalid CMD",
/*15-1a*/  "Invalid Cyl Address","Invalid Sec",nu,"Bus Timeout",nu,
/*1a-1c*/  "Write Protected","Unit Not Selected","No Address Mark - Header",
/*1d-1e*/  "No Address Mark - Data","Drive Faulted",
/*1f-21*/  nu,"Disk Surface Overrun","ID Field Error - Wrong Sector",
/*22-23*/  "CRC Error - ID Field","Uncorrectable Error",
/*24-28*/  nu,nu,"No sector Pulse","Format Timeout","No Index",
/*29-2d*/  "Sector Not Found","ID Field Wrong",nu,nu,"Seek Timeout",
/*2e-31*/  nu,"Not On Cylinder","RTZ Timeout",nu,
/*32-3d*/  nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,
/*3e-4a*/  nu,nu,"Not Inited",nu,"Gap Spec Error",nu,nu,nu,nu,nu,nu,nu,nu,
/*4b-4f*/  nu,"Map Hd Err",nu,nu,nu,
/*50-53*/  "Sec/Trk Err","Byt/Sec Err","ILV Err","Invalid Head",
/*54-5f*/  nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,
/*60-61*/  "Protection Timeout Error","Maximum Cylinder Number Spec Error",
/*62-63*/  "Number of Heads Spec Error","Step Pulse Spec Error",
/*64-65*/  "Reserved Byte Spec Error","Ram Failure -odd byte",
/*66-67*/  "RAM Failure - even byte","Event ROM Failure",
/*68-69*/  "Device Not Previously Recalibrated", "Controller Error",
/*6a-6b*/  "Invalid Sector Number", "Timer Error", "ROM Failure - odd byte",
/*6d-6d*/  "ROM Failure - even byte",nu,nu,nu,
		0
};
#ifdef NOTDEF
/*70-7f*/  nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,nu,
/*80-81*/  "Tape Drive Not Selected","Tape Drive Not Ready",
/*82-83*/  "Tape Drive Not Online","Cartridge Not In Place",
/*84-85*/  "Unexpected Beginning of Tape","Unexpected End of Tape",
/*86-87*/  "Unexpected file Mark Encountered","Unrecoverable Data Error",
/*88-8a*/  "Block in Error Not Located","No Data Detected","Write Protected",
/*8b-8c*/  "Illegal Command","Command Sequence Timeout",
/*8d-8e*/  "Status Sequence Timeout","Data Block Transfer Timeout",
/*8f-90*/  "Filemark Search Timeout","Unexpected Exception",
/*91-92*/  "Invalied Unit Address - Tape","Ready Timeout", 
/*93-94*/  "Tape Timeout Specification Error","Invalid Block Count",
	    0
};
#endif

/* Needs to be updated (GB) */

char *stcmdlist[] = {
/*80-85 */  	nu,"Read","Write","Verify","Format","Map Bad Track",
/* */	"Report Configuration","Initialize","Disk to Tape Transfer",
/* */	"Restore","Seek","ReFormat","Format with Sector Data",
/* */	"Tape to Disk Transfer","Motor Control (Floppy)",
/* */	"Reset","Map Bad Sector",nu,nu,"Read Absolute",
/* */  	"Read Non-Cached",
	0
};
