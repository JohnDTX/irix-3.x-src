#include "tutor.h"

int backw;

init_back()
{
    int res;

    prefposition(0, 1023, 0, 767);
    res = winopen("ba");

    backw = res;
    return(res);
}

draw_back()
{
    winset(backw);
    color(NORMCOLOR);
    clear();
}

redraw_back()
{
    winset(backw);
    color(NORMCOLOR);
    clear();
    swapbuffers();
/*    winset(backw);
    color(NORMCOLOR);*/
    clear();
}

