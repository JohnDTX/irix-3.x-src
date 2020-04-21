/*
** data.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/pm2stand/mdfex/RCS/data.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:11:37 $
**
**	uname(1) uses the disktype to determine if it's running on
**	a 2400 or a 2300.  Currently it thinks disk types DT_TM525,
**	DT_TM362, and DT_3212 mean 2300.  If any others are added,
**	please update uname!!  CSK 6/19/85
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include "disk.h"

#ifdef MDFEX
#include "dsdreg.h"
#define CONTROLLER	DC_DSD5217
#endif

#define FO 4*17			/* bpc for the TM-252 */
#define FJ 7*34			/* bpc for the Fuji 2312 */
#define EG 880			/* bpc for the Eagle 2351A */
#define	C 7*17			/* bpc for the atasi 3046 & Vertex V170 */
#define NV 3*17			/* bpc for the Vertex V130 */
#define M 15*17			/* bpc for the 140 MB Maxtor */
#define MX 8*17			/* bpc for the 85 MB Maxtor */
#define F 11*17			/* bpc for the Fujitsu 2243 */
#define WR 9*17			/* bpc for the CDC Wren II */
#define T 10*17			/* bpc for the Hitachi */

struct disk_label nec_lab = {
	D_MAGIC, DT_D5126, CONTROLLER, 615, 4, 17, FO*607, FO*8, 0, 1,
	FO*1, FO*123,	FO*124, FO*360,	FO*484, FO*123,	FO*0, FO*0,
	FO*0, FO*0,	FO*0, FO*0,	FO*1, FO*606,	FO*0, FO*607,
	1, 0, 11, 0, "NEC D5126", "0000", 0,
};
struct disk_label cmi_lab = {
	D_MAGIC, DT_3426, CONTROLLER, 615, 4, 17, FO*607, FO*8, 0, 1,
	FO*1, FO*123,	FO*124, FO*360,	FO*484, FO*123,	FO*0, FO*0,
	FO*0, FO*0,	FO*0, FO*0,	FO*1, FO*606,	FO*0, FO*607,
	1, 0, 11, 0, "CMI 3426", "0000", 0,
};
struct disk_label t3_lab = {
	D_MAGIC, DT_TM362, CONTROLLER, 612, 4, 17, FO*607, FO*5, 0, 1,
	FO*1, FO*123,	FO*124, FO*360,	FO*484, FO*123,	FO*0, FO*0,
	FO*0, FO*0,	FO*0, FO*0,	FO*1, FO*606,	FO*0, FO*607,
	1, 0, 11, 0, "Tandon TM-362/TM-262", "0000", 0,
};
struct disk_label t2_lab = {
	D_MAGIC, DT_TM252, CONTROLLER, 306, 4, 17, FO*303, FO*3, 0, 1,
	FO*1, FO*61,	FO*62, FO*180,	FO*242, FO*61,	FO*1, FO*302,
	0, 0,		0, 0,		0, 0,		FO*0, FO*303,
	1, 0, 11, 0, "Tandon TM-252", "0000", 0,
};
struct disk_label a_lab = {
	D_MAGIC, DT_3046, CONTROLLER, 645, 7, 17, C*635, C*10, 0, 1,
	C*1, C*150,	C*151, C*99,	C*250, C*385,	C*1, C*634,
	0, 0,		0, 0,		0, 0,		C*0, C*635,
	1, 0, 11, 0, "Atasi 3046", "0000", 0,
};
struct disk_label w_lab = {
	D_MAGIC, DT_WRENII, CONTROLLER, 925, 9, 17, WR*916, WR*9, 0, 1,
	WR*1, WR*150,	WR*151, WR*99,	WR*250, WR*300,	WR*550, WR*366,
	WR*1, WR*916,	0, 0,		0, 0,		WR*0, WR*916,
	1, 0, 11, 0, "CDC Wren II(86)", "0000", 0,
};
struct disk_label v_lab = {
	D_MAGIC, DT_V170, CONTROLLER, 987, 7, 17, C*970, C*17, 0, 1,
	C*1, C*150,	C*151, C*149,	C*300, C*670,	C*1, C*969,
	0, 0,		0, 0,		0, 0,		C*0, C*970, 
	1, 0, 11, 0, "Vertex V170", "0000", 0,
};
struct disk_label h_lab = {
	D_MAGIC, DT_5118, CONTROLLER, 823, 10, 17, T*814, T*9, 0, 1,
	T*1, T*110,	T*111, T*99,	T*210, T*604,	T*210, T*302,
	T*512, T*302,	T*1, T*813,	0, 0,		T*0, T*814, 
	1, 0, 11, 0, "Hitachi DK-511-8", "0000", 0,
};
struct disk_label v8_lab = {
	D_MAGIC, DT_V185, CONTROLLER, 1166, 7, 17, C*1154, C*12, 0, 1,
	C*1, C*150,	C*151, C*149,	C*300, C*854,	C*300, C*427,
	C*712, C*427,	0, 0,		C*1, C*1154,	C*0, C*1154, 
	1, 0, 11, 0, "Vertex V185", "0000", 0,
};
struct disk_label mx_lab = {
	D_MAGIC,	DT_1140,	CONTROLLER,	918,		15,
	17,		M*910,		M*17,		0,		1,
	M*1, M*70,	M*71, M*70,	M*141, M*384,	M*525, M*385,
	M*1, M*909,	0, 0,		0, 0,		M*0, M*910, 1,
	0,	11,	0,	"Maxtor 1140",	"0000", 0,
};
struct disk_label mic_lab = {
	D_MAGIC, DT_1325, CONTROLLER, 1024, 8, 17, MX*1014, MX*10, 0, 1,
	MX*1, MX*150, MX*151, MX*149, MX*300, MX*714, MX*300, MX*357,
	MX*657, MX*357, MX*1, MX*1013, 0, 0,  MX*0, MX*1014,
	1, 0, 2, 0, "Micropolis 1325", "0000", 0,
};
struct disk_label m8_lab = {
	D_MAGIC, DT_1085, CONTROLLER, 1024, 8, 17, MX*1014, MX*10, 0, 1,
	MX*1, MX*150, MX*151, MX*149, MX*300, MX*586, MX*886, MX*128,
	MX*300, MX*357, MX*657, MX*357, MX*1, MX*1013, MX*0, MX*1014,
	1, 0, 11, 0, "Maxtor 1085", "0000", 0,
};
#ifdef IPFEX
struct disk_label e_lab = {
	D_MAGIC,	DT_2351,	DC_INTERPHASE2190,  842,	20,
	44,		EG*837,		EG*5,		0,		1,
	EG*1, EG*32,	EG*33, EG*48,	EG*81, EG*56,	EG*137, EG*700,
	EG*137, EG*350,	EG*487, EG*350,	0, 0,		EG*0, EG*837,	1,
	0,	11,	0,	"Eagle 2351",	"0000",		20, 30, 11,
	0,
};
struct disk_label f_lab = {
	D_MAGIC, DT_2312, DC_INTERPHASE2190,  589, 7,
	34,  FJ*585,  FJ*4,  0,  1,
	FJ*1, FJ*150, FJ*151, FJ*85, FJ*236, FJ*350, FJ*1, FJ*585,
	0, 0,  0, 0,  0, 0,  FJ*0, FJ*585,
	2, 0, 13, 0, "Fujitsu 2312", "0000",  20, 22, 16, 0,
};
#endif IPFEX
struct disk_label flp_lab = {
	D_MAGIC,	DT_T101,	CONTROLLER,	80,		2,
	8,		16*80,		16*0,		0,		0,
	0, 0,		0, 0,		0, 0,		0, 0,
	0, 0,		0, 0,		0, 0,		0, 0,
};
struct disk_label fu_lab = {
	D_MAGIC,	DT_2243,	CONTROLLER,	754,		11,
	17,		F*748,		F*7,		0,		1,
	F*1, F*150,	F*151, F*149,	F*300, F*448,	0, 0,
	0, 0,		0, 0,		F*1, F*747,	F*0, F*748, 1,
	0,	11,	0,	"Fujitsu 2243",	"0000",	0,
};
struct disk_label me_lab = {
	D_MAGIC,	DT_MEM514,	CONTROLLER,	961,		7,
	17,		C*958,		C*17,		0,		1,
	C*1, C*150,	C*151, C*149,	C*300, C*658,	C*1, C*957,
	0, 0,		0, 0,		0, 0,		C*0, C*958, 1,
	0,	11,	0,	"Memorex",	"0000", 0,
};
struct disk_label v3_lab = {
	D_MAGIC,	DT_V130,	CONTROLLER,	987,		3,
	17,		NV*970,		NV*17,		0,		1,
	NV*1, NV*200,	NV*201, NV*149,	NV*350, NV*620,	NV*1, NV*969,
	NV*1, NV*600,	NV*601, NV*369,	0, 0,		NV*0, NV*970,	0,
	0,	0,	0,	"Vertex V130",	"0000", 0,
};

struct dtypes dtypes[] = {
	"3046",		 0,	D_WIN,	&a_lab, 	/* Atasi 3046 */
	"V170",		 1,	D_WIN,	&v_lab, 	/* Vertex V170 */
#ifdef IPFEX
	"2351",		 2,	&e_lab,			/* Eagle 2351A */
	"2312",		 3,	&f_lab,			/* Fujitsu 2312 */
#endif IPFEX
	"1085",		 4,	D_WIN,	&m8_lab,	/* Maxtor 1085 */
	"WREN",		 5,	D_WIN,	&w_lab,		/* CDC Wren II */
	"V185",	 	 6,	D_WIN,	&v8_lab,	/* Vertex V185 */
	"5118",		 7,	D_WIN,	&h_lab,		/* Hitachi 85MB */
	"1140",		 8,	D_WIN,	&mx_lab,	/* Maxtor 1140 */
	"1325",		 9,	D_WIN,	&mic_lab,	/* Micropolis 1325 */
	"V130",	 	10,	D_WIN,  &v3_lab,	/* Vertex V130 */
	"2243",	 	11,	D_WIN,	&fu_lab,	/* Fujitsu 2243 */
	"M514",	 	12,	D_WIN,  &me_lab,	/* Memorex 514 */
	"T101",		13,	D_FLP,	&flp_lab,   	/* Tandon 100-4 */
	"TM252",	14,	D_WIN,	&t2_lab,   	/* Tandon TM252 */
	"TM262",	21,	D_WIN,	&t3_lab,   	/* Tandon TM262 */
	"TM362",	21,	D_WIN,	&t3_lab,   	/* Tandon TM362 */
	"3426",		27, 	D_WIN,	&cmi_lab,	/* CMI 3426 */
	"D5126",	28,	D_WIN,	&nec_lab,	/* NEC D5126 */
	0
};

int	errno;
