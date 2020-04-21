#ifndef	__PANE_H__
#define	__PANE_H__

/*
 * Pane stuff.
 *
 * $Source: /d2/3.7/src/usr.bin/edge/h/RCS/pane.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:02 $
 */
#include "rect.h"

typedef	struct	pane {
	struct	pane *p_next, *p_prev;		/* linkage to neighbors */
	struct	pane *p_contents;		/* panes inside this pane */
	rect_t	p_r;				/* bounds of the pane */
	/* methods */
	int	(*p_redraw)();			/* redraw */
} pane_t;

/* exported functions */
extern	pane_t	*newPane();
extern	void	freePane();
extern	void	setPaneSize();
extern	void	setPaneOrigin();
extern	void	setPaneRect();
extern	void	setPaneView();
extern	void	setPaneRedraw();
extern	void	paneManager();
#endif	/* __PANE_H__ */
