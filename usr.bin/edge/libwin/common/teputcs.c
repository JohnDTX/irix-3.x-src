/*
 * Put some characters into the emulator.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/teputcs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:49 $
 */

#include "tf.h"
#include "te.h"

void
teputchars(te, buf, nb)
	termulator *te;
	char *buf;
	int nb;
{
	(*te->te_putchars)(te, buf, nb);
}
