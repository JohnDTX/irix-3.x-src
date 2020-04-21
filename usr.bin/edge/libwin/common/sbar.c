/*
 * Scroll bar process.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/sbar.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:34 $
 */

#include "rect.h"
#include "pane.h"
#include "tf.h"
#include "lw.h"
#include "message.h"
#include "sbar.h"

/*
 * Manage a scroll bar.
 */
void
sbproc(proc, sb)
	lwproc *proc;
	sbar *sb;
{
	lwmsg *msg, *reply;

	lwregister(LEFTMOUSE, 0, 0);
	lwregister(MOUSEX, 0, 0);
	lwregister(MOUSEY, 0, 0);
	qdevice(LEFTMOUSE);

	for (;;) {
		msg = lwgetmsg();
		swtich (msg->m_type) {
		  case LEFTMOUSE:
			if (msg->m_value) {
				if (IsInMyPane(getvaluator(MOUSEX),
					       getvaluator(MOUSEY))) {
					sb->sb_state = 1;
					qdevice(MOUSEX);
					qdevice(MOUSEY);
				}
			} else {
				sb->sb_state = 0;
				unqdevice(MOUSEX);
				unqdevice(MOUSEY);
			}
			break;
		  case SETSCROLLRANGE:
			sb->sb_range = msg->m_value;
			break;
		  case SETSCROLLLOCATION:
			sb->sb_position = msg->m_value;
			break;
		}
		lwputmsg(msg);
	}
}

lwproc *
sbnew()
{
	sbar *sb;
	lwproc *sp;
	int xsize, ysize;

	sp = 0;
	sb = MALLOC(sbar *, sizeof(sbar));
	if (sb) {
		sb->sb_xorg = 0;
		sb->sb_yorg = 0;
		getsize(&xsize, &ysize);
		sb->sb_xsize = 20;
		sb->sb_ysize = ysize;
		sp = lwstart("scroll bar", sbproc, 0, sb, 0, 0, 0);
	}
	return (sp);
}
