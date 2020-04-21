/*
 * Menu handling code
 *
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/menu.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:59 $
 */
#include "gl.h"
#include "device.h"
#include "gsh.h"
#include "tf.h"
#include "window.h"
#include "manage.h"

#include "stdio.h"
#include "sys/types.h"
#include "dirent.h"
#include "dbxshm.h"

extern	char *strrchr();
char	*vi_argv[40];
WINTTYMAP	*vi_win;
char	start_vi;
char	start_file;
char	runflag;
struct	Filetab	{
	char	**filenames;
	int	nfiles;
	int	base_value;
}	filetab[20];
int	nfiletabs;
int	old_ntabs;
int	edit_selection;
int	file_selection;
int	gx;
int	gy;
int	glob_nfiles;

int	main_menu, size_menu, font_menu, edit_menu[20], file_menu[20];
char	*font_name[1000];
int	fonts;

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
	stepunit(charwidth, charheight);
	fudge(XSIZE(0), YSIZE(0));
	minsize(XSIZE(3), YSIZE(3));
	maxsize(XSIZE(MAXCOLS), YSIZE(MAXROWS));
	winposition(newx, newx + newxsize - 1, newy, newy + newysize - 1);
	winconstraints();
}

#ifdef notdef
/*
 * Process the font menu commands
 */
void
fontfunc(selection)
	int selection;
{
	int	gid;
	int	cur_win;
	WINTTYMAP	*wtmp;

#ifdef mips
	gid = gl_winat(gx, gy);
#else
	gid = winat(gx, gy);
#endif
	cur_win = winget();
	winset(gid);
	wtmp = gid_to_wintty(gid);
	if (font_menu == 0)
		return;
	if ((selection < 1) || (selection > fonts))
		return;
	/*
	 * Switch to new font.
	 */
	flag_font = font_name[selection - 1];
	setup_font();
	reshapeit(txport[wtmp->wt_textnum].tx_rows, 
		txport[wtmp->wt_textnum].tx_cols);
	qenter(REDRAW, winget());
/*
	winset(cur_win);
*/
}
#endif

/*
 * Process the main menu commands
 */
void
mainfunc(selection)
	int selection;
{
	int	gid;
	int	ogid;

	switch (selection) {
	  case 1:			/* attach command; */
#ifdef mips
		gid = gl_winat(gx, gy);
#else
		gid = winat(gx, gy);
#endif
		winattach(gid);
		winset(gid);
		/*
		 * We do this so that those of us who are used to having
		 * mex menu with attach at the top, can just boink on top
		 * of our window without changing any state
		 */
		break;
	  case 2:			/* select */ 
#ifdef mips
		gid = gl_winat(gx, gy); 
#else
		gid = winat(gx, gy); 
#endif
		winset(gid);
		winpop(gid); 
		winattach(gid); 
		break;
	  case 3:			/* get an editor going */
		(void) dopup(edit_menu);
		break;
	  case 4:			/* Send file command to dbx */
		(void) dopup(file_menu);
		break;
	}
}

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
	char	*malloc();

	c = (char *) malloc(strlen(name) + 1);
	strcpy(c, name);

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

#ifdef notdef
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
	register char *cp;
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
	qsort((char *)&font_name[0], fonts, sizeof(font_name[0]), fontsort);
	/*
	 * Now build the menu.
	 */
	for (i = 0; i < fonts; i++)
		addtopup(font_menu, font_name[i], fontfunc);
}

#endif
/*
 * Exported interface to menu handling.  Called when the MENUBUTTON goes
 * down
 */
void
domenu(mousex, mousey)
int	mousex;
int	mousey;
{
	char	*filename;
	char	*select_2_file();

	gx = mousex;
	gy = mousey;
	winset(dbx_win->wt_gid);
	(void) dopup(main_menu);
	if (start_vi) {
		filename = select_2_file(edit_selection);
		make_vi(filename, -1);
	}
	if (start_file) {
		filename = select_2_file(file_selection);
		do_file(filename);
	}
}

do_file(filename)
{
	char	file_buf[512];
	char	*p;
	char	*strip_usedir();

	p = strip_usedir(filename);
	sprintf(file_buf, "file %s\n", p);
	send_shell(dbx_win, file_buf, strlen(file_buf));
	start_file = 0;
}

filefunc(selection)
int	selection;
{
	start_file = 1;
	file_selection = selection;
}

editfunc(selection)
{

	start_vi = 1;
	
	edit_selection = selection;

} 

char	*
select_2_file(selection)
int	selection;
{
	struct	Filetab	*ftp;

	for (ftp = &(filetab[0]); ftp < &(filetab[nfiletabs]); ftp++) {
		if ((selection >= ftp->base_value) &&
			(selection < (ftp->base_value + ftp->nfiles))) {
			return(ftp->filenames[selection - ftp->base_value]);
		}
	}
	return(NULL);
}

make_vi(filename, lineno) 
char	*filename;
int	lineno;
{
	int	av = 0;
	char	linestr[20];
	char	*editor;
	int	pid;
	char	color_buf[40];
	char	bcolor[5];
	char	fcolor[5];
	char	*getenv();
	int i;

	if ((filename == NULL) && (((edit_selection - 1) < 0) 
		|| ((edit_selection - 1) > glob_nfiles))) {
		return;
	}
#ifdef mips
	vi_argv[av++] = "wsh";
#else
	vi_argv[av++] = "/usr/lib/gsh";
#endif
#ifdef notdef
	vi_argv[av++] = "-s";
	vi_argv[av++] = "40";
	vi_argv[av++] = "80";
#endif
	if (flag_font) {
		vi_argv[av++] = "-f";
		vi_argv[av++] = flag_font;
	}
#ifdef mips
	vi_argv[av++] = "-C";
	sprintf(color_buf, "%d,%d,%d,%d", dtextcolor, dpagecolor, 1, 2);
	vi_argv[av++] = color_buf;
#else
	vi_argv[av++] = "-C";
	sprintf(fcolor, "%d", dtextcolor);
	vi_argv[av++] = fcolor;
	sprintf(bcolor, "%d", dpagecolor);
	vi_argv[av++] = bcolor;
	vi_argv[av++] = "1";

#endif
	vi_argv[av++] = "-c";
	if ((editor = getenv("EDITOR")) != NULL) {
		vi_argv[av++] = editor;
	} else if ((editor = getenv("EDIT")) != NULL) {
		vi_argv[av++] = editor;
	} else {
		vi_argv[av++] = "vi";
	}
	if (lineno != -1) {
		sprintf(linestr, "+%d\n", lineno);
		vi_argv[av++] = linestr;
		vi_argv[av++] = filename;
	} else {
		if ( cur_file && 
			(strcmp(cur_file, filename) == 0)) {
			sprintf(linestr, "+%d\n", cur_line);
			vi_argv[av++] = linestr;
		}
		vi_argv[av++] = filename;
	}
	vi_argv[av++] = NULL;
	if ((pid = fork()) == 0) {
		shmdetach();
#ifdef mips
		execvp("wsh", vi_argv);
#else
		execv("/usr/lib/gsh", vi_argv);
#endif
		perror("child:edge");
	}
	if (pid == -1) {
		perror("parent:edge");
	}
	start_vi = 0;
}

/*
 * Initialize the menus
 */
void
initmenus()
{
	edit_menu[0] = defpup("edit %t %F", editfunc);
	file_menu[0] = defpup("file %t %F", filefunc);
#ifdef notdef
	build_font_menu();
#endif
	main_menu = defpup(
	       "dbx %t %F|attach|select|file %m %f|edit %m %f",
		   mainfunc, file_menu[0], filefunc, edit_menu[0], editfunc);
}

static	int	been_here = 0;

make_edit_menu()
{

	int	i;
	char	*fnamep;
	char	*malloc();
	struct	Filetab	*ftp;
	int	j;
	int	select_num = 0;
	char	menu_buf[100];

	
	if (been_here != 0) {
		freepup(edit_menu[0]);
		edit_menu[0] = defpup("edit %t %F", editfunc);
		freepup(file_menu[0]);
		file_menu[0] = defpup("file %t %F", filefunc);
		for (i = 1; i < old_ntabs; i++) {
			freepup(edit_menu[i]);
			freepup(file_menu[i]);
		}
	}
	for (i = 1; i < nfiletabs; i++) {
		edit_menu[i] = defpup("edit %t", editfunc);
		file_menu[i] = defpup("file %t", filefunc);
	}
	for (i = 0; i < nfiletabs; i++) {
		ftp = &filetab[i];
		if (i != (nfiletabs - 1)) {
			addtopup(edit_menu[i]," more %m", 
				edit_menu[i+1], editfunc);
			addtopup(file_menu[i]," more %m", 
				file_menu[i+1], filefunc);
		}
		for (j = 0; j < ftp->nfiles; j++) {
			sprintf(menu_buf, "%s %%x%d", ftp->filenames[j],
				select_num++);
			addtopup(edit_menu[i], menu_buf);
			addtopup(file_menu[i], menu_buf);
			
		}
	}
	been_here = 1;
}

add_filetab(filenames, nfiles, isfirst)
char	*filenames;
int	nfiles;
int	isfirst;
{
	int	i;
	char	*fnamep;
	char	*ftnamep;
	char	*malloc();
	struct	Filetab	*ftp;


	if (isfirst) {
		glob_nfiles = 0;
		old_ntabs = nfiletabs;
		nfiletabs = 0;
	}
		
	fnamep = filenames;
	ftp = &(filetab[nfiletabs++]);
	ftp->filenames = (char **) malloc(nfiles * sizeof(char *));
	ftp->base_value = glob_nfiles;
	glob_nfiles += nfiles;
	fnamep = filenames;
	ftp->nfiles = nfiles;
	for (i = 0; i < nfiles; i++) {
		ftp->filenames[i] = (char *) malloc(strlen(fnamep) + 1);
		ftnamep = ftp->filenames[i];
		strcpy(ftnamep, fnamep);
		fnamep += strlen(fnamep) + 1;
		addsrcfile(ftnamep);
	}
}

char	*
strip_usedir(pathname)
char	*pathname;
{
	int	i;
	char	*p;

	for (i = 0; i < nuses; i++) {
		if (strncmp(pathname, use_string[i], strlen(use_string[i]))
			== 0) {
			return(pathname + strlen(use_string[i]) + 1);
		}
	}
	return(pathname);
}
