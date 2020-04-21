/*
 * Button object stuff.
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/button.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:17 $
 */
#include "pane.h"
#include "button.h"

Type	ButtonType;

ColorButton(b, on, off)
	Button *b;
	int on, off;
{
	CheckType(&b->pane.obj, ButtonType);
	b->b_oncolor = on;
	b->b_offcolor = off;
}

void
SizeButton(b, r)
	Button *b;
	Rect r;
{
	CheckType(&b->pane.obj, ButtonType);
	SizePane(b, r);
}

int
HandleButton(b, event)
	Button *b;
	Event *event;
{
	CheckType(&b->pane.obj, ButtonType);
	switch (event->e_msg) {
	  case EVENT_DRAW:
	  case EVENT_REDRAW:
		FocusPane(&b->pane);
		if (b->b_state == 0) {
			color(b->b_offcolor);
			clear();
		} else {
			color(b->b_oncolor);
			clear();
		}
		break;
	  default:
		BadEvent(&b->pane.obj, "button", event);
		break;
	}
}

Button *
NewButton()
{
	Button *b;

	InitType(ButtonType);
	b = New(Button, ButtonType, HandleButton);
	if (b) {
		SetType(&b->pane.obj, ButtonType);
		b->b_offcolor = 0;		/* XXX preferences */
		b->b_oncolor = 7;		/* XXX preferences */
	}
	return (b);
}

void
FreeButton(b)
	Button *b;
{
	CheckType(&b->pane.obj, ButtonType);
	FreePane(&b->pane);
}
