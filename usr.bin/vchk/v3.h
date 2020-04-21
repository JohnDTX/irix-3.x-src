/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#define index strchr
#define rindex strrchr
#define major(x) (((x) >> 8) & 0xFF)
#define minor(x) ((x) & 0xFF)
