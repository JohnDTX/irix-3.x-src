/*
**	data.c		- Copyright (C), JCS Computer Services 1983
**			- Author: chase and markb
**			- Date: January 1985 ->forward
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by authors.
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/data.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:10 $
*/

#include "types.h"
#include <sys/dklabel.h>
#include "disk.h"

#ifndef DT_QUME
#define DT_QUME DT_T101
#endif
#define CONTROLLER 	DC_STORAGER

#define FO 4*17			/* bpc for the TM-252 */
#define	C 7*17			/* bpc for the atasi 3046 & Vertex V170 */
#define NV 3*17			/* bpc for the Vertex V130 */
#define M 15*17			/* bpc for the 140 MB Maxtor */
#define MX 8*17			/* bpc for the 85 MB Maxtor */
#define F 11*17			/* bpc for the Fujitsu 2243 */
#define T 10*17			/* bpc for the Hitachi ST506 511-8 */

/* ESDI */
#define TS 10*32		/* bpc for the Hitachi ESDI 512-17 */
#define TO 10*34		/* bpc for the Toshiba MK156FA */
#define S 3*24			/* bpc for the ESDI AST 96202 */
#define SF 5*32			/* bpc for the ESDI AST 96203 & Hitachi 512-8 */
#define M7 7*31			/* bpc for the Hitachi ESDI 512-12 */
#define N9 9*16			/* bpc for the ESDI CDC Wren II */
#define WR 9*34                 /* bpc for the ESDI CDC Wren III */
#define M15 15*34		/* bpc for the 380 Micropolis and Fujitsu */
#define S12 12*35		/* Siemens 300MB Drive */
#define HI 14*50		/* bpc for the Hitachi 514-38 */

#ifndef DT_4380
#define DT_4380 99
#endif
#ifndef DT_1300
#define DT_1300	99
#endif
#ifndef DT_2246
#define DT_2246	99
#endif
#ifndef DT_2249
#define DT_2249 99
#endif
#ifndef DT_51438
#define DT_51438 99
#endif
struct disk_label fj2249_lab = {
	D_MAGIC, DT_2249, CONTROLLER, 1243, 15, 34, M15*1210, M15*24, 0, 1,
	M15*1, M15*36,	M15*37, M15*98, 	M15*135, M15*122, M15*257, M15*272,
	0, 0, 		M15*135, M15*1076, M15*1, M15*1210,	M15*0, M15*1211,
	2, 0, 5, 0, "Fujitsu 2249", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc, opt */
	0x19, 0x13, 0x26, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 0x2f, 0x20, 0x43, 0,
};
struct disk_label fj2246_lab = {
	D_MAGIC, DT_2246, CONTROLLER, 823, 10, 32, TS*806, TS*8, 0, 1,
	TS*1, TS*59,	TS*60, TS*100,	TS*160, TS*200,	TS*360, TS*446,
	0, 0,		TS*160, TS*646,	TS*1,TS*805,	TS*0, TS*806,
	1, 0, 5, 0, "Fujitsu 2246", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc, opt */
	0x0d, 0x11, 0x20, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 0x2f, 0x20, 0x43, 0,
};
struct disk_label to156FA_lab = {
	D_MAGIC, DT_156FA, CONTROLLER, 830, 10, 34, TO*810, TO*8, 0, 1,
	TO*1, TO*59,	TO*60, TO*100, TO*160, TO*200, TO*360, TO*446,
	0, 0, 		TO*160, TO*646, TO*1, TO*809,	TO*0, TO*810,
	1, 0, 5, 0, "Toshiba 156FA", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc, opt */
	0x0b, 0x0d, 0x12, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 0x2f, 0x20, 0x43, 0,
}; 
struct disk_label wren3_lab = {
	D_MAGIC, DT_WREN3, CONTROLLER, 961, 9, 34, WR*950, WR*11, 0, 1,
 	WR*1, WR*56, WR*57, WR*210, WR*267, WR*300, WR*567, WR*383,
	0, 0, 		WR*267, WR*683, WR*1, WR*949,	WR*0, WR*950,
	1, 0, 3, 0, "CDC WrenIII", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc, opt */
	0x11, 0x11, 0x18, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 0x2F, 0x20, 0x43, 0,
};
struct disk_label si300_lab = {
	D_MAGIC, DT_1300, CONTROLLER, 1216, 12, 35, S12*1200, S12*16, 0, 1,
	S12*1, S12*59, S12*60, S12*100, S12*160, S12*200, S12*360, S12*446,
	0, 0,  S12*160, S12*646, S12*1,S12*1199, S12*0, S12*1200,
	1, 0, 3, 0, "Siemens 1300", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc, opt */
	0xb, 0x11, 0x12, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 0x37, 0x20, 0x43, 0,
};
struct disk_label hi17_lab = {
	D_MAGIC, DT_51217, CONTROLLER, 823, 10, 32, TS*806, TS*8, 0, 1,
	TS*1, TS*59,	TS*60, TS*100,	TS*160, TS*200,	TS*360, TS*446,
	0, 0,		TS*160, TS*646,	TS*1,TS*805,	TS*0, TS*806,
	1, 0, 5, 0, "Hitachi 512-17", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc, opt */
	0x0d, 0x11, 0x20, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 0x2f, 0x20, 0x43, 0,
};
struct disk_label hi38_lab = {
	D_MAGIC, DT_51438, CONTROLLER, 903, 14, 50, HI*875, HI*28, 0, 1,
	HI*1, HI*27,	HI*28, HI*74,	HI*102, HI*92, HI*194, HI*704,
	0, 0, 		HI*102, HI*801, HI*1, HI*875,	HI*0, HI*876,
	1, 0, 5, 0, "Hitachi 514-38", "0000",
	/* gaps, spw, spi, ttst, wpc, rpre, hlst, eccon, mohu, ddb, smc, opt */
	0x13, 0x13, 0x1e, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 0x2f, 0x20, 0x43, 0,
};
struct disk_label hi12_lab = {
	D_MAGIC, DT_51212, CONTROLLER, 823, 7, 32, M7*806, M7*17, 0, 1,
	M7*1, M7*50,	M7*51, M7*49,	M7*100, M7*353,	M7*453, M7*353,
	0, 0,		M7*100, M7*706,	M7*1,M7*805,	M7*0, M7*806,
	1, 0, 5, 0, "Hitachi 512-12", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc */
	33, 33, 32, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 0x27, 0x20, 0x43, 0,
};
struct disk_label hi8_lab = {
	D_MAGIC, DT_5128, CONTROLLER, 823, 5, 32, SF*806, SF*17, 0, 1,
	SF*1, SF*50,	SF*51, SF*49,	SF*100, SF*353,	SF*453, SF*353,
	0, 0,		SF*100, SF*706,	SF*1,SF*805,	SF*0, SF*806,
	1, 0, 5, 0, "Hitachi 512-17", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc */
	33, 33, 32, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 0x27, 0x20, 0x43, 0,
};
struct disk_label as1_lab = {
	D_MAGIC, DT_96203, CONTROLLER, 921,  5, 32,  SF*900,  SF*21,  0,  1,
	SF*1, SF*200,	SF*201, SF*209,	SF*410, SF*511,	0, 0,
	0, 0,		0, 0,		SF*1, SF*920,	SF*0, SF*921,
	1, 0, 5, 0, "AST 96203", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc */
	33, 33, 32, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 0x27, 0x20, 0x43, 0,
};
struct disk_label as6_lab = {
	D_MAGIC, DT_96202, CONTROLLER, 921,  3, 24,  S*900,  S*21,  0,  1,
	S*1, S*200,	S*201, S*209,	S*410, S*511,	0, 0,
	0, 0,		0, 0,		S*1, S*920,	S*0, S*921,
	1, 0, 5, 0, "AST 96202", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc */
	0x19, 0x19, 0x20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 0x27, 0x20, 0x43, 0,
};
struct disk_label mini_lab = {
	D_MAGIC, DT_3212, CONTROLLER, 306,  4, 17,  FO*303,  FO*3,  0,  1,
	FO*1, FO*61,	FO*62, FO*180,	FO*242, FO*61,	FO*1, FO*302,
	0, 0,	0, 0,	0, 0,	FO*0, FO*303,
	1, 0, 11, 0, "Miniscribe 3212", "0000", 0,
	17, 17, 24, 1, 1, 6, 0xffff, 128, 3,1, 0, 6,0x20, 0x43, 0,
};
struct disk_label tm_lab = {
	D_MAGIC, DT_TM252, CONTROLLER, 306,  4, 17,  FO*303,  FO*3,  0,  1,
	FO*1, FO*61,	FO*62, FO*180,	FO*242, FO*61,	FO*1, FO*302,
	0, 0,	0, 0,	0, 0,	FO*0, FO*303,
	1, 0, 11, 0, "Tandon TM-252", "0000",
	17, 17, 24, 1, 1, 6, 0xffff, 128, 3,1, 0, 6,0x20, 0x43, 0,
};
struct disk_label a4_lab = {
	D_MAGIC, DT_3046, CONTROLLER, 645,7, 17,  C*635,  C*10,  0,1,
	C*1, C*150,	C*151, C*100,	C*251, C*384,	0, 0,
	0, 0,		0, 0,		C*1,C*634,	C*0, C*635,
	1, 0,	5,	0,	"Atasi 3046",	"0000",
	17, 17, 24, 1, 1, 3, 0xffff, 128, 3,1, 0, 6,0x20, 0x43, 0,
};
struct disk_label tosh_lab = {
	D_MAGIC, DT_MK56, CONTROLLER, 830, 10, 17, T*820, T*10, 0, 1,
	T*1, T*110,	T*111, T*99,	T*210, T*604,	T*210, T*302,
	T*512, T*302,	T*1, T*819,	0, 0,		T*0, T*820,
	1, 0, 11, 0, "Toshiba MK56FB", "0000", 0,
	17, 17, 24, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label hi_lab = {
	D_MAGIC, DT_5118, CONTROLLER, 823, 10, 17, T*814, T*9, 0, 1,
	T*1, T*120,	T*121, T*119,	T*240, T*571,	T*240, T*286,
	T*526, T*286,	0, 0,		T*1,T*813,	T*0, T*814,
	1, 0, 5, 0, "Hitachi 511-8", "0000",
	17, 17, 24, 1, 1, 6, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label d7_lab = {
	D_MAGIC, DT_D570, CONTROLLER, 987, 7, 17,  C*977,  C*10,  0, 1,
	C*1, C*150,	C*151, C*149,	C*300, C*677,	0, 0,
	0, 0,		0, 0,		C*1, C*976,	C*0, C*977,
	1, 0, 5, 0, "Cynthia D570", "0000",
	17, 17, 24, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label v7_lab = {
	D_MAGIC, DT_V170, CONTROLLER, 987, 7, 17,  C*977,  C*10,  0, 1,
	C*1, C*150,	C*151, C*149,	C*300, C*677,	0, 0,
	0, 0,		0, 0,		C*1, C*976,	C*0, C*977,
	1, 0, 5, 0, "Vertex V170", "0000",
	17, 17, 20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label m4_lab = {
	D_MAGIC, DT_1140, CONTROLLER, 918, 15, 17, M*910, M*17, 0,1,
	M*1, M*70,	M*71, M*70,	M*141, M*384,	M*525, M*385,
	M*1, M*909,	0, 0,		0, 0,		M*0, M*910,
	1, 0, 5, 0, "Maxtor 1140", "0000",
	17, 17, 20,1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label m8_lab = {
	D_MAGIC, DT_1085, CONTROLLER, 1024, 8, 17, MX*1014, MX*10, 0, 1,
	MX*1, MX*150,	MX*151, MX*149,	MX*300, MX*714,	MX*300, MX*357,
	MX*657, MX*357, MX*1, MX*1013,	0, MX*1,	MX*0, MX*1014,
	1, 0, 5, 0, "Maxtor 1085", "0000",
	17, 17, 20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label m175_lab = {
	D_MAGIC, DT_4175, CONTROLLER, 1224, 7, 31, M7*1200, M7*24, 0, 1,
	M7*1, M7*89, M7*90, M7*150,	M7*240, M7*280,	M7*520, M7*680,
	0,0,  M7*240, M7*960,	M7*1, M7*1199,	M7*0, M7*1200,
	1, 0, 5, 0, "Maxtor 4175", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc */
	0x1D,0x21,0x20,1,1,5,0xffff,0xffff,0,1,0,0x27,0x20,0x43, 0,
};
struct disk_label m380_lab = {
	D_MAGIC, DT_4380, CONTROLLER, 1224, 15, 34, M15*1200, M15*24, 0, 1,
	M15*1, M15*39, M15*40, M15*70, M15*110, M15*150, M15*260, M15*940,
	0,0,  M15*110, M15*1090, M15*1, M15*1199, M15*0, M15*1200,
	2, 0, 5, 0, "155015", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc */
	0x15,0x11,0x26,1,1,5,0xffff,0xffff,0,1,0,0x2f,0x20,0x43, 0,
};
struct disk_label v3_lab = {
	D_MAGIC, DT_V130, CONTROLLER, 987, 3, 17, NV*977, NV*10, 0, 1,
	NV*1, NV*150,	NV*151, NV*149,	NV*300, NV*200,	NV*500, NV*477,
	NV*1, NV*488,	NV*489, NV*488,	0, NV*1,	NV*0, NV*970,
	1, 0, 5, 0, "Vertex V130", "0000",
	17, 17, 20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label t_lab = {
	D_MAGIC, DT_T101, CONTROLLER, 80,  2, 8,  16*80,  16*0,  0,  0,
	0, 0,	0, 0,	0, 0,	0, 0,
	0, 0,	0, 0,	0, 0,	0, 0,
	0, 0, 0, 0, "Tandon 101-4", "0000",
	17, 17, 50, 2, 400, 3, 0, 0xffff, 15, 2, 0x11, 0x44, 0x03, 0x43, 0,
};
struct disk_label q_lab = {
	D_MAGIC, DT_QUME, CONTROLLER, 80,  2, 8,  16*80,  16*0,  0, 0,
	0, 0,	0, 0,	0, 0,	0, 0,
	0, 0,	0, 0,	0, 0,	0, 0,
	0, 0, 0, 0, "Qume 592", "0000",
	17, 17, 50, 2, 400, 3, 0, 0xffff, 15, 2, 0x11, 0x44,    3, 0x43, 0,
};
#ifdef ALIEN
struct disk_label te_lab = {
	D_MAGIC,	DT_35EFD,	CONTROLLER,	80,		1,
	8,		8*80,		16*0,		0,		0,
	0, 0,		0, 0,		0, 0,		0, 0,
	0, 0,		0, 0,		0, 0, 0, 0,
	0, 0, 0, 0, "Teac FD-35E", "0000",
	17, 17, 50, 2, 800, 6, 0, 0xffff, 15, 2, 0x11, 0x44, 3, 0
};
struct disk_label mi_lab = {
	D_MAGIC,	DT_MITSU,	CONTROLLER,	80,		1,
	8,		8*80,		16*0,		0,		0,
	0, 0,		0, 0,		0, 0,		0, 0,
	0, 0,		0, 0,		0, 0,		0, 0,
	0,      0,	    0,	      0, "Mitsubishi MF-351", "0000",
	17, 17, 50, 1, 380, 3, 43, 0xffff, 15, 2, 0x11, 0x44, 4, 0
};
struct disk_label tb_lab = {
	D_MAGIC,DT_35BFD,CONTROLLER,40,2, 8,16*40,16*0,0,0,
	0, 0,		0, 0,		0, 0,		0, 0,
	0, 0,		0, 0,		0, 0,		0, 0,
	0,      0,	    0,	      0, "Teac FD-35B", "0000",
	17, 17, 50, 2, 800, 6, 0, 0xffff, 15, 2, 0x11, 0x44, 3, 0
};
#endif ALIEN
struct disk_label fu_lab = {
	D_MAGIC, DT_2243, CONTROLLER, 754, 11, 17,F*748,F*7,0,1,
	F*1, F*150,	F*151, F*149,	F*300, F*448,	0, 0,
	0, 0,		0, 0,		F*1, F*747,	F*0, F*748,
	1, 0,	5,	0,	"Fujitsu 2243",	"0000",
	17, 17, 20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label me_lab = {
	D_MAGIC,DT_MEM514,CONTROLLER,961,7, 17,	C*958,	C*17,0,	1,
	C*1, C*150,	C*151, C*149,	C*300, C*658,	C*1, C*957,
	0, 0,		0, 0,		0, 0,		C*0, C*958,
	1, 0,	5,	0,	"Memorex 514",	"0000",
	17, 17, 20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label wr_lab = {
	D_MAGIC, DT_WRENII, CONTROLLER, 961, 7, 17, C*958, C*17, 0, 1,
	C*1, C*150,	C*151, C*149,	C*300, C*658,	C*1, C*957,
	0, 0,		0, 0,		0, 0,		C*0, C*958,
	1, 0,	5,	0,	"CDC Wren II",	"0000",
	17, 17, 20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};
struct disk_label wr_esdi_lab = {
	D_MAGIC, DT_WRENESDI, CONTROLLER, 924, 9, 16, N9*914, N9*10, 0, 1,
	N9*1, N9*150,	N9*151, N9*149,	N9*300, N9*614,	N9*1, N9*913,
	0, 0,		0, 0,		0, 0,		N9*0, N9*914,
	1, 0, 5, 0, "Wren II ESDI", "0000",
	/* gaps, spw, spi, ttst, wpc, rwpre, hlst, eccon, mohu, ddb, smc */
	0x11, 0x11, 0x18, 1, 1, 10, 0x0, 0xffff, 0, 1, 0, 0x2F, 0x20, 0x43, 0,
};
struct disk_label v8_lab = {
	D_MAGIC, DT_V185,CONTROLLER, 1166,7, 17, C*1154, C*12, 0,	1,
	C*1, C*150,	C*151, C*149,	C*300, C*854,	C*300, C*427,
	C*712, C*427,	0, 0,		C*1, C*1154,	C*0, C*1154, 	1,
	0,	11,	0,	"V185",	"0000",
	17, 17, 20, 1, 1, 5, 0xffff, 0xffff, 0, 1, 0, 6, 0x20, 0x43, 0,
};

struct dtypes dtypes[] = {
	"3046",		 DT_3046,	&a4_lab,	/* Atasi 3046 */
	"V170",		 DT_V170,	&v7_lab, 	/* Vertex V170 */
	"1085",		 DT_1085,	&m8_lab,	/* Maxtor 1085 */
	"wren",		 DT_WRENII,	&wr_lab,	/* Wren II */
	"V185",		 DT_V185,	&v8_lab,	/* Vertex V185 */
	"5118",		 DT_5118,	&hi_lab,	/* Hitachi 511-8 */
	"1140",		 8,	&m4_lab,		/* Maxtor 1140 */
/*	"1325",		 9,  	&mi_lab,		/* Micropolis 1325 */
	"V130",		 10, 	&v3_lab,		/* Vertex V130 */
	"2243",	 	 11,	&fu_lab,		/* Fujitsu 220 */
/*	"T35E",		 11,	&te_lab,		/* Teac FD-35E */
	"M514",	 	 12,	&me_lab,		/* Memorex 514 */
	"T101",		 13,	&t_lab,			/* Tandon 101-4 */
	"tm252",	 14,	&tm_lab,		/* Tandon TM-252 */
/*	"T35B",		 13,	&tb_lab,		/* Teac FD-35B */
/*	"MITS",		 14,	&mi_lab,		/* Mitsubishi */
	"qume",		 15,	&q_lab,			/* Qume Floppy */
	"96202",	 16,	&as6_lab,		/* AST 96202 */
	"96203",	 17,	&as1_lab,		/* AST 96203 */
	"D570",	 	 18,	&d7_lab,		/* Cynthia D570 */
	"3212",	 	 19,	&mini_lab,		/* Miniscribe 3212 */
	"94156",	 20,    &wr_esdi_lab,		/* Wren II 86 ESDI */
	"4175",		 22,	&m175_lab,		/* Maxtor 4175 */
	"5128",		 23,	&hi8_lab,		/* Hitachi 512-8 */
	"51212",	 24,	&hi12_lab,		/* Hitachi 512-12 */
	"51217",	 25,	&hi17_lab,		/* Hitachi 512-17 */
/*	"AIMSMD",	 26,	&xxx_lab,		/* AIM SMD dart 130 */
	"155015",	DT_4380,	&m380_lab,	/* 1550-15 */
	"1300",		DT_1300,	&si300_lab,	/* Siemens 1300 */
	"2246",		DT_2246,	&fj2246_lab,	/* Fujitsu 2246 */
	"2249",		DT_2249,	&fj2249_lab,	/* Fujitsu 2249 */
	"MK56FB",	DT_MK56,	&tosh_lab,	/* Toshiba MK56FB */
 	"156FA",        DT_156FA,    &to156FA_lab,      /* Toshiba 156FA */
        "WREN3",	DT_WREN3,	&wren3_lab,     /* CDC Wren III */
	"51438",	DT_51438,       &hi38_lab,	/* Hitachi 514-38 */
	0
};
int errno;


