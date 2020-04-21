/* @(#)pkon.c	1.2 */
pkon(i,j) int i,j; { if (i == j); return(1); } /* shuts up lint */
pkoff(i) int i; { if (i); return; } /*shuts up lint */
