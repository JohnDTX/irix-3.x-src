/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/buttons.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:48 $
 */
#define NSIDEBUTTS 11
#define NBOTTOMS 0

typedef	struct {
	rect_t	b_position;
	char	*b_string;
	int	(*b_func)();
	int	backcolor;
	int	charcolor;
} BUTT_T;

extern	int	dostep();
extern	int	dostop();
extern	int	donext();
extern	int	runuser();
extern	int	dolist();
extern	int	docont();
extern	int	domake();
extern	int	doquit();
extern	int	dosh();
extern	int	dowhere();
extern	int	dotrace();
extern	int	dointerrupt();
extern	int	doprint();

extern	BUTT_T	sidebutts[];
/*
extern	BUTT_T	bottombutts[];
*/
