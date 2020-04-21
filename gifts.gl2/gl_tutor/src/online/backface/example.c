#include "back2.h"

redraw_exam()
/*---------------------------------------------------------------------------
 * Routine called in the event of a REDRAW token coming down the queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_example();
    reshapeviewport();
    draw_example();
}

init_exam()
/*---------------------------------------------------------------------------
 * Initialize the example window and return the window id.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(513, 1021, 545, 735);
    res = winopen("exam");
    keepaspect(508, 190);
    wintitle("Backface -- EXAMPLE");
    winconstraints();
    setup_exam_environ();

    return(res);
}

setup_exam_environ()
/*---------------------------------------------------------------------------
 * Setup for drawing in the example window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 512.0, 0.0, 192.0);
}

draw_example()
/*---------------------------------------------------------------------------
 * Draw the normal example in upper right corner of the screen.
 *---------------------------------------------------------------------------
 */
{
    attach_to_example();
    color(EXAMBC);
    clear();
    draw_template();
    draw_example_poly();
}

attach_to_example()
/*---------------------------------------------------------------------------
 * Setup the viewport etc for this window..
 *---------------------------------------------------------------------------
 */
{
    winset(examw);
}

draw_template()
/*---------------------------------------------------------------------------
 * Draw the template to work on.
 *---------------------------------------------------------------------------
 */
{
    color(RED);
    setpattern(HALFTONE);
    pmv2s(32, 96);
    pdr2s(192, 192);
    pdr2s(320, 192);
    pdr2s(320, 0);
    pdr2s(192, 0);
    pclos();

    color(BLUE);
    rectfi(320, 192, 512, 0);
    setpattern(0);

    color(WHITE);
    linewidth(2);
    setlinestyle(DASHED);
    lsbackup(TRUE);

    line2(320.0, 0.0, 320.0, 192.0);
    line2(32.0, 96.0, 320.0, 96.0);

    color(YELLOW);
    arcf(32.0, 96.0, 22.4, -309, 309);
    setlinestyle(0);
    lsbackup(FALSE);

    color(WHITE);
    line2(32.0, 96.0, 64.0, 115.2);
    line2(32.0, 96.0, 64.0, 76.8);

    color(TEXTC);
    cmov2i(22, 128);
    charstr("Eye");
    cmov2i(192, 128);
    charstr("Drawn");
    cmov2i(400, 96);
    charstr("Not Drawn");
}

draw_example_poly()
/*---------------------------------------------------------------------------
 * Draw the example polygon on the template.
 *---------------------------------------------------------------------------
 */
{
    linewidth(2);
    color(GREEN);

    pushmatrix();
	translate(320.0, 96.0, 0.0);
	rotate(curexam, 'z');
	arrow(0.0, 0.0, 0.0, 0.0, 32.0, 0.0);

	if (curexam < 1800)
	    line2(-16.0, 0.0, 16.0, 0.0);
    popmatrix();
}
