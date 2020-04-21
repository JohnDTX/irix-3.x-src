#ifndef GLERRORDEF
#define GLERRORDEF
/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#define FATAL		1	/* exit from program after printing message */
#define WARNING		2	/* print message and continue */
#define ASK_CONT	3	/* ask if program should continue */
#define ASK_RESTART	4	/* ask if program should be restarted */

#define ERR_SINGMATRIX		1
#define ERR_OUTMEM		2
#define ERR_NEGSIDES		3
#define ERR_BADWINDOW		4
#define ERR_NOOPENOBJ		5
#define ERR_NOFONTRAM		6
#define ERR_FOV			7
#define ERR_BASISID		8
#define ERR_NEGINDEX		9
#define ERR_NOCLIPPERS		10
#define ERR_STRINGBUG		11
#define ERR_NOCURVBASIS		12
#define ERR_BADCURVID		13
#define ERR_NOPTCHBASIS		14
#define ERR_FEEDPICK		15
#define ERR_INPICK		16
#define ERR_NOTINPICK		17
#define ERR_ZEROPICK		18
#define ERR_FONTBUG		19
#define ERR_INRGB		20
#define ERR_NOTINRGB		21
#define ERR_BADINDEX		22
#define ERR_BADVALUATOR		23
#define ERR_BADBUTTON		24
#define ERR_NOTDBMODE		25
#define ERR_BADINDEXBUG		26
#define ERR_ZEROVIEWPORT	27
#define ERR_DIALBUG		28
#define ERR_MOUSEBUG		29
#define ERR_RETRACEBUG		30
#define ERR_MAXRETRACE		31
#define ERR_NOSUCHTAG		32
#define ERR_DELBUG		33
#define ERR_DELTAG		34
#define ERR_NEGTAG		35
#define ERR_TAGEXISTS		36
#define ERR_OFFTOOBIG		37
#define ERR_ILLEGALID		38
#define ERR_GECONVERT		39
#define ERR_BADAXIS		40
#define ERR_BADDEVICE		42
#define ERR_PATCURVES		44
#define ERR_PATPREC		45
#define ERR_CURVPREC		46
#define	ERR_PUSHATTR		47
#define	ERR_POPATTR		48
#define	ERR_PUSHMATRIX		49
#define	ERR_POPMATRIX		50
#define	ERR_PUSHVIEWPORT	51
#define	ERR_POPVIEWPORT		52
#define ERR_SIZEFIXED		53
#define ERR_SETMONITOR		54
#define ERR_CHANGEINDEX0	55
#define ERR_BADPATTERN		56
#define ERR_BADCURSOR		57
#define ERR_FONTHOLES		58
#define ERR_REPLACE		59
#define ERR_STARTFEED		60
#define ERR_CYCLEMAP		61
#define ERR_TAGINREPLACE	62
#define ERR_TOOFEWPTS		63

#endif GLERRORDEF
