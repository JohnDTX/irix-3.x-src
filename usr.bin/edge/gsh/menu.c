/*
 * Menu handling code
 * XXX clean up to avoid having multiple menu funnctions (menufunc)
 *
 * $Source: /d2/3.7/src/usr.bin/edge/gsh/RCS/menu.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:38 $
 */
#include "gl.h"
#include "device.h"
#include "gsh.h"
#include "window.h"
#include "sys/types.h"
#include "dirent.h"
#include "string.h"

int	opened_menu, closed_menu, size_menu, font_menu;
int	last_rows, last_cols;
char	*font_name[1000];
int	fonts;
char	need_clone, need_select;
#ifdef	SHRINK
char	noicon, need_open, need_close;
#endif
int	(*menufunc)();

#ifdef	lint
/*LINTLIBRARY*/
/*VARARGS1*/
int defpup(str, x) char *str; int x; { str=str; x=x; return(0); }
#endif

/*
 * Reshape a window to the given size
 */
void
reshapeit(rows, cols)
	int rows, cols;
{
	int xsize, ysize;
	int xorg, yorg;
	int newxsize, newysize;
	int newx, newy;

	getsize(&xsize, &ysize);
	getorigin(&xorg, &yorg);
	newxsize = XSIZE(cols);
	newysize = YSIZE(rows);

	/*
	 * Find center of old size.  Put center of new window there,
	 * by subtracting off the size of half of the new windows
	 * dimensions.
	 */
	newx = (xorg + xsize / 2) - newxsize / 2;
	newy = (yorg + ysize / 2) - newysize / 2;
	winconstraints();
	stepunit(font_width, font_height);
	fudge(XSIZE(0), YSIZE(0));
	minsize(XSIZE(3), YSIZE(3));
	maxsize(XSIZE(MAXCOLS), YSIZE(MAXROWS));
	winposition(newx, newx + newxsize - 1, newy, newy + newysize - 1);
	winconstraints();
}

/*
 * Process the font menu commands
 */
void
fontfunc(selection)
	int selection;
{
	if (font_menu == 0)
		return;
	if ((selection < 1) || (selection > fonts))
		return;
	/*
	 * Switch to new font.
	 */
	flag_font = font_name[selection - 1];
	setup_font();
	reshapeit(txport[0].tx_rows, txport[0].tx_cols);
	qenter(REDRAW, (short) winget());
}

/*
 * Process the size menu commands
 */
void
sizefunc(selection)
	int selection;
{
	int rows, cols;

	switch (selection) {
	  case 1:			/* go to last size */
		rows = txport[0].tx_rows;
		cols = txport[0].tx_cols;
		reshapeit(last_rows, last_cols);
		last_rows = rows;
		last_cols = cols;
		break;
	  case 2:			/* force to 80 columns */
		last_rows = txport[0].tx_rows;
		last_cols = txport[0].tx_cols;
		reshapeit(txport[0].tx_rows, 80);
		break;
	  case 3:			/* 40x80 */
		last_rows = txport[0].tx_rows;
		last_cols = txport[0].tx_cols;
		reshapeit(40, 80);
		break;
	  case 4:			/* 24x80 */
		last_rows = txport[0].tx_rows;
		last_cols = txport[0].tx_cols;
		reshapeit(24, 80);
		break;
	}
}

/*
 * Function to handle the menus with font menu
 */
int
main_font(selection)
	int selection;
{
	switch (selection) {
	  case 1:			/* select */
		need_select = 1;
		break;
	  case 2:			/* size menu */
		sizefunc(1);
		break;
	  case 3:			/* font menu */
		break;
	  case 4:			/* clone this gsh */
		need_clone = 1;
		break;
#ifdef	SHRINK
	  case 5:
		need_close = 1;		/* shrink and iconize */
		break;
#endif
	}
}

/*
 * Function to handle the menus with font menu
 */
int
main_nofont(selection)
	int selection;
{
	switch (selection) {
	  case 1:			/* select */
		need_select = 1;
		break;
	  case 2:			/* size menu */
		sizefunc(1);
		break;
	  case 3:			/* clone this gsh */
		need_clone = 1;
		break;
#ifdef	SHRINK
	  case 4:
		need_close = 1;		/* shrink and iconize */
		break;
#endif
	}
}

#ifdef	SHRINK
void
reopenfunc(selection)
	int selection;
{
	switch (selection) {
	  case 1:
		need_open = 1;
		break;
	}
}
#endif

/*
 * Given a name of the passed in length, figure out the root
 * name by stripping off the ".fnt" extension.  If the file name
 * doesn't end in ".fnt", then ignore the name.
 */
char *
fixname(name)
	char *name;
{
	char *c, *d;

	c = (char *) malloc((unsigned) (strlen(name) + 1));
	(void) strcpy(c, name);

	/*
	 * See if name ends in a ".fnt"
	 */
	d = strrchr(c, '.');
	if (d) {
		if (strcmp(d, ".fnt") == 0) {
			*d = 0;
			return (c);
		}
	}
	free(c);
	return (NULL);
}

/*
 * fontsort is used to sort the font names, in ascii order
 */
fontsort(n1, n2)
	char **n1, **n2;
{
	return (strcmp(*n1, *n2));
}

/*
 * Build font menu
 */
build_font_menu()
{
	register DIR *dirp;
	register struct dirent *d;
	register int i;

	/*
	 * Find font library directory
	 */
	dirp = opendir(fontlib);
	if (dirp == NULL)
		return;
	font_menu = defpup("font %t %F", fontfunc);

	for (;;) {
		d = readdir(dirp);
		if (d == NULL)
			break;
		font_name[fonts] = fixname(d->d_name);
		if (font_name[fonts])
			fonts++;
	}
	closedir(dirp);
	if (fonts == 0) {
		freepup(font_menu);
		font_menu = 0;
		return;
	}
	/*
	 * Now sort the font names
	 */
	qsort((char *)&font_name[0], (unsigned) fonts, sizeof(font_name[0]),
	      fontsort);
	/*
	 * Now build the menu.
	 */
	for (i = 0; i < fonts; i++)
		addtopup(font_menu, font_name[i], fontfunc);
}

/*
 * Exported interface to menu handling.  Called when the MENUBUTTON goes
 * down
 */
void
domenu()
{
	if (opened) {
		(*menufunc)(dopup(opened_menu));
	} else {
		(void) dopup(closed_menu);
	}

	if (need_clone) {
		need_clone = 0;
		clone();
	}
	if (need_select) {
		need_select = 0;
		winpop();
		winattach();
	}
#ifdef	SHRINK
	if (!noicon && need_close) {
		need_close = 0;
		shrink();
	}
	if (need_open) {
		need_open = 0;
		unshrink();
	}
#endif
}

#define	MENU_NOICON_NOFONT \
	"gsh %t|select|size %m|clone"
#define	MENU_NOICON_FONT \
	"gsh %t|select|size %m|font %m|clone"
#define	MENU_SIZE \
	"size %t %F|previous|80 columns|40 x 80|24 x 80"

#ifdef	SHRINK
#define	MENU_ICON_NOFONT \
	"gsh %t|select|size %m|clone|shrink"
#define	MENU_ICON_FONT \
	"gsh %t|select|size %m|font %m|clone|shrink"
#endif

/*
 * Initialize the menus
 */
void
initmenus()
{
	build_font_menu();
	size_menu = defpup(MENU_SIZE, sizefunc);
#ifdef	SHRINK
	if (noicon) {
#endif
		/* no icon */
		if (font_menu) {
			opened_menu = defpup(MENU_NOICON_FONT, size_menu,
							       font_menu);
			menufunc = main_font;
		} else {
			opened_menu = defpup(MENU_NOICON_NOFONT, size_menu);
			menufunc = main_nofont;
		}
#ifdef	SHRINK
	} else {
		/* have icon */
		if (font_menu) {
			opened_menu = defpup(MENU_ICON_FONT, size_menu,
							     font_menu);
			menufunc = main_font;
		} else {
			opened_menu = defpup(MENU_ICON_NOFONT, size_menu,
							       font_menu);
			menufunc = main_nofont;
		}
	}
	closed_menu = defpup("gsh %t %F|unshrink", reopenfunc);
#endif
}
