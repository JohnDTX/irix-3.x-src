#ifndef	__WINDOW_H__
#define	__WINDOW_H__
/*
 * Primitive window data type
 */
#include "pane.h"

typedef	struct	windowStruct {
	int	w_id;			/* window identifier */
	pane_t	*w_contents;		/* panes in the window */
} window_t;

#endif	/* __WINDOW_H__ */
