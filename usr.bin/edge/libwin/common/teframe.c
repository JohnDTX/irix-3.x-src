/*
 * Return the frame that this terminal emulator is using.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/teframe.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:44 $
 */

#include "tf.h"
#include "te.h"

textframe *
teframe(te)
	termulator *te;
{
	return (te->te_tf);
}
