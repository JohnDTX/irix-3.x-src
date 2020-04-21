/* main.c */

#include "mas.h"

main()
{
	generate();

	fsm();

	dispatch();
	lowmem();
	vectors();
	stm();
	points();
	block();
	attributes();
	modes();
	hitrept();
	chars();
	chdraw();
#ifndef NORUNLEN
	readrun();
	runlen();
#endif
	diag();
	cursor();
	pixels();
	readpixels();
	mvppixels();
	fixchar();
	divide();
	ldivide();
	viewport();
	copyfont();
	copypixels();
	depthvec();
	add_sentinels();
	bump_indexes();
	check_masking();
	fill_trapezoid();
	finish_side();
	get_first_sides();
	polyclose();
	polydraw2d();
	poly_exit();
	start_side();
	swap();
	trapezoid_loop();
	advance();
	computeslope();

	shfinish_side();
	shade_line();
	shade_trapezoid();
	shget_first_sides();
	shstart_side();
	shaded_trap_loop();
	buffcopy();
	zfinish_side();

#ifndef NOZPOLY
	zvectors();
	dbzline();
	zfill_trap();
	z_fill_line();
	zget_first_sides();
	z_shade_line();
	zshade_trap();
	zstart_side();
	zshaded_trap_loop();
	zbuff();
	szbuff();
#endif
#ifndef NOZSCAN
	zshade_scanline();
	scanline_init();
#endif
	endfsm();
}
