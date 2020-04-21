/*
**	$Source
**	$Revision
**	$Date
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include "disk.h"
#include "ipreg.h"

#define FJ 7*32			/* bpc for the Fuji 2312 */
#define EG 20*44		/* bpc for the Eagle 2351A */
#define CD 19*32		/* bpc for the CDC 9766 */
#define AM 19*44		/* bpc for the AMS 513 */

struct disk_label e_lab = {
	D_MAGIC, DT_2351A, DC_INTERPHASE2190,  842, 20, 44, EG*837, EG*5, 0, 1,
	EG*1, EG*32,	EG*33, EG*48,	EG*81, EG*56,	EG*137, EG*700,
	EG*137, EG*350,	EG*487, EG*350,	0, 0,	EG*0, EG*837,
	1, 0, 44, 0, "Eagle 2351", "0000", 20, 30, 22, 0,
};
struct disk_label f_lab = {
	D_MAGIC, DT_2312, DC_INTERPHASE2190,  589, 7, 32, FJ*585, FJ*4, 0, 1,
	FJ*1, FJ*150,	FJ*151, FJ*85,	FJ*236, FJ*350,	FJ*1, FJ*584,
	0, 0,		0, 0,		0, 0,		FJ*0, FJ*585,
	1, 0, 32, 0, "Fujitsu 2312", "0000",  20, 22, 16,
	0,
};
struct disk_label c_lab = {
	D_MAGIC, DT_9766, DC_INTERPHASE2190, 823, 19, 32, CD*815, CD*8, 0, 1,
	CD*1, CD*44, CD*45, CD*66, CD*112, CD*77, CD*190, CD*625,
	CD*190, CD*481, CD*671, CD*144, 0, 0, CD*0, CD*815,
	1, 0, 32, 0, "CDC 9766", "0000", 20, 30, 16, 0,
};
struct disk_label a_lab = {
	D_MAGIC, DT_513, DC_INTERPHASE2190, 823, 19, 44, AM*800, AM*23, 0, 1,
	AM*1, AM*44, AM*45, AM*66, AM*112, AM*77, AM*190, AM*625,
	AM*190, AM*481, AM*671, AM*144, 0, 0, AM*0, AM*800,
	1, 0, 44, 0, "AMS 513", "0000", 20, 30, 22, 0,
};

struct dtypes dtypes[] = {
	"2351A",	DT_2351A,	&e_lab,		/* Eagle 2351A */
	"eagle",	DT_2351A,	&e_lab,		/* Eagle 2351A */
	"2312",		DT_2312,	&f_lab,		/* Fujitsu 2312 */
	"9766",		DT_9766,	&c_lab,		/* CDC 9766 */
	"513",		DT_513, 	&a_lab,		/* AMS 513 */
	0
};

int	errno;
