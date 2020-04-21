/* /usr/include/sys/tek4692.h - include file for Tektronix 4692 color printer */

	/* coloropen types */
#define	CO_DEFAULT	85	/* default 1024 by 768, standard parameters   */
#define	CO_XY		86	/* user-specified dimensions, standard params */
#define	CO_TEK4692	87	/* user-specified dimensions and parameters   */

	/* colorwrite types */
#define	CW_RAW		0	/* raw data in Tek format */
#define	CW_RLE		1	/* run-length encoded data (not implemented) */

	/* structure for data for CO_DEFAULT */
struct	co_default {
	int	res;	/* resolution in bits/color/pixel */
};

	/* structure for data for CO_XY */
struct	co_xy {
	int	res;	/* resolution in bits/color/pixel */
	int	x_size;	/* pixels in x dir (parallel to long paper edge) */
	int	y_size;	/* pixels in x dir (parallel to long paper edge) */
};

	/* structure for data for CO_TEK4692 */
struct	co_tek4692 {
	char	mode;
	char	repaint;
	char	x_hi;
	char	x_lo;
	char	y_hi;
	char	y_lo;
};

union	co_union {
	struct	co_default	codefault;
	struct	co_xy		coxy;
	struct	co_tek4692	cotek4692;
};

FILE	*coloropen();
