/*
 */
typedef struct sbar {
	char	sb_state;		/* state of scroll bar */
	long	sb_xorg, sb_yorg;
	long	sb_xsize, sb_ysize;
	long	sb_position;		/* current position */
	long	sb_range;		/* boundary limits (0 to range - 1) */
} sbar;
