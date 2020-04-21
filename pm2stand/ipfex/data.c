/*
**	$Source
**	$Revision
**	$Date
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include "disk.h"

#ifdef MDFEX
#include "dsdreg.h"
#endif

#ifdef IPFEX
#include "ipreg.h"
#endif

#ifdef STFEX
#define CONTROLLER 	DC_STORAGER
#else
#define CONTROLLER	DC_DSD5217
#endif

#define FJ 7*32			/* bpc for the Fuji 2312 */
#define EG 880			/* bpc for the Eagle 2351A */
#define	C 7*17			/* bpc for the atasi 3046 & Vertex V170 */
#define NV 3*17			/* bpc for the Vertex V130 */
#define M 15*17			/* bpc for the 140 MB Maxtor */
#define MX 8*17			/* bpc for the 85 MB Maxtor */
#define F 11*17			/* bpc for the Fujitsu 2243 */
#define AM 7*32			/* bpc for the Aim Dart 130 */

#ifdef MDFEX
struct disk_label a_lab = {
	D_MAGIC, DT_3046, CONTROLLER, 645,  7, 17,  C*635,  C*10,  0,  1,
	C*1, C*150,	C*151, C*99,	C*250, C*385,	C*1, C*634,
	0, 0,		0, 0,		0, 0,		C*0, C*635, 1,
	0,	11,	0,	"Atasi 3046",	"0000",	17, 17, 32, 1,
	1, 10, 0x90, 1, 0xff, 0xff, 0,
};
struct disk_label v_lab = {
#ifdef MDFEX
	17,		C*970,		C*17,		0,		1,
	C*1, C*150,	C*151, C*149,	C*300, C*670,	C*1, C*969,
	0, 0,		0, 0,		0, 0,		C*0, C*970, 	1,
#else
	17,		C*977,		C*10,		0,		1,
	C*1, C*150,	C*151, C*149,	C*300, C*677,	0, 0,
	0, 0,		0, 0,		C*1, C*976,	C*0, C*977, 	1,
#endif
	0,	11,	0,	"Vertex",	"0000",	17, 17, 32, 	1,
	1, 10, 0x90, 1, 0xff, 0xff,
	0,
};

/* Maxtor 1140 */
struct disk_label mx_lab = {
	D_MAGIC,	DT_1140,	CONTROLLER,	918,		15,
	17,		M*910,		M*17,		0,		1,
	M*1, M*70,	M*71, M*70,	M*141, M*384,	M*525, M*385,
	M*1, M*909,	0, 0,		0, 0,		M*0, M*910, 1,
	0,	11,	0,	"Maxtor 1140",	"0000",	17, 17, 32, 1,
	1, 10, 0x90, 1, 0xff, 0xff,
	0,
};

/* Maxtor 1085 */
struct disk_label m8_lab = {
	D_MAGIC,	DT_1085,	CONTROLLER,	1024,		8,
	17,		MX*1014,	MX*10,		0,		1,
	MX*1, MX*150,	MX*151, MX*149,	MX*300, MX*714,	MX*300, MX*357,
	MX*657, MX*357, MX*1, MX*1013,	0, 0,		MX*0, MX*1014, 1,
	0,	11,	0,	"Maxtor 1085",	"0000",	17, 17, 32, 1,
	1, 10, 0x90, 1, 0xff, 0xff, 0,
};
#endif MDFEX

#ifdef IPFEX
struct disk_label e_lab = {
	D_MAGIC, DT_2351A, DC_INTERPHASE2190,  842, 20, 44, EG*837, EG*5, 0, 1,
	EG*1, EG*32,	EG*33, EG*48,	EG*81, EG*56,	EG*137, EG*700,
	EG*137, EG*350,	EG*487, EG*350,	0, 0,	EG*0, EG*837,	1,
	0, 11, 0, "Eagle 2351", "0000", 20, 30, 11, 0,
};
struct disk_label f_lab = {
	D_MAGIC, DT_2312, DC_INTERPHASE2190,  589, 7, 32, FJ*585, FJ*4, 0, 1,
	FJ*1, FJ*150,	FJ*151, FJ*85,	FJ*236, FJ*350,	FJ*1, FJ*584,
	0, 0,		0, 0,		0, 0,		FJ*0, FJ*585,
	1, 0, 8, 0, "Fujitsu 2312", "0000",  20, 22, 8,
	0,
};
struct disk_label aim_lab = {
	D_MAGIC, DT_AIMSMD, DC_INTERPHASE2190,  916, 7, 32, AM*898, AM*18, 0, 1,
	AM*1, AM*100,	AM*111, AM*89,	AM*200, AM*698,	AM*200, AM*349,
	AM*549, AM*349,		0, 0,	AM*1, AM*897,	AM*0, AM*898,
	2, 0, 8, 0, "Aim Dart 130", "0000",  20, 30, 8,
	0,
};
#endif IPFEX

struct dtypes dtypes[] = {
	"2351A",	DT_2351A,	&e_lab,		/* Eagle 2351A */
	"2312",		DT_2312,	&f_lab,		/* Fujitsu 2312 */
	"Aim",		DT_AIMSMD,	&aim_lab,	/* Dart 130 */
	0
};

int	errno;
