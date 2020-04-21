/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/manage.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:58 $
 */
#define PTYNMSIZE 20
#define NWINDOWS 2
#define DEF_DELAY 6

typedef struct {
	short	x;
	short	y;
} POINT_T;

typedef struct {
	rect_t	rect;
	short	is_default;
	short	hassize;
	short	hasorig;
} SIZE_DESC;

extern	SIZE_DESC	src_rect;

#define WT_DBX	0
#define WT_USER	1
#define WT_SRC 	2
#define WT_EDIT 3

rect_t	init_vals[4];

typedef struct winttymap {
	char	wt_type;
	textframe	*wt_tf;
	textview	*wt_tv;
	int	wt_textnum;
	int	wt_masterfd;
	int	wt_slavefd;
	int	wt_ptynum;
	int	wt_gid;
	int	wt_pid;
	char	*wt_mastername;
	char	*wt_slavename;
	int	read_bytes;
	char	stopped;
	char	disconnected;
	char	need_update;
	char	need_read;
	char	need_redisplay;
	int	savedcursorcolor;
	int	realcursorcolor;
	rect_t	position;
	struct	winttymap	*nextwintty;
	struct	winttymap	*lastwintty;
} WINTTYMAP;


WINTTYMAP	winttymap[20];
extern	WINTTYMAP	*dbx_win;
extern	WINTTYMAP	*user_win;
extern	WINTTYMAP	*vi_win;
extern	WINTTYMAP	*cur_win;
extern	WINTTYMAP	*shell_win;
extern	WINTTYMAP	*last_link;
extern	char	*flag_font;

extern	WINTTYMAP	*ptynum_to_window();
extern	WINTTYMAP	*pid_to_wintty();
extern	WINTTYMAP	*gid_to_wintty();
extern	int	srcwin;
extern	int	speed_win;
extern	int	var_win;
extern	int	selecting;
extern	int	dbflag;
extern	int	current_delay;
extern	int	nuses;
extern	char	*use_string[];
extern	int	scroll_win;
extern	int	scrolling;
#ifndef mips
extern	int	commpipe[2];
#endif


#define XBORDER 2
#define YBORDER 2
