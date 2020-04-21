#include <stdio.h>
#include <math.h>
#include <gl.h>
#include "slider.h"

init_r_slide()
{
int yd;
int dx,dy,dz;

    getorigin(&sl_xl,&sl_yb);
    getsize(&sl_xr,&sl_yt);
    pushmatrix();
    ortho2((float) sl_xl,(float) (sl_xl+sl_xr),(float) sl_yb,(float) (sl_yb+(sl_yt)));
    getmatrix(barmat);
    sl_xr = sl_xl+sl_xr-10;
    sl_yt = sl_yb+(sl_yt/5)-10;
    sl_xl = sl_xl+12;
    sl_yb = sl_yb+12;
    sl_rxd = 4000 / (sl_xr - sl_xl);
    yd = (sl_yt - sl_yb) / 3;
    sl_y1 = sl_yb+5;
    sl_y2 = sl_y1+yd;
    sl_y3 = sl_y2+yd;
    dx = (int)r_xlast / sl_rxd;
    dy = (int)r_ylast / sl_rxd;
    dz = (int)r_zlast / sl_rxd;

    slider = 292;
    make_bars(slider,dx,dy,dz);
    popmatrix();
}

init_t_slide(tmaxx,tmaxy,tmaxz)
float tmaxx,tmaxy,tmaxz;
{
int yd;
int dx,dy,dz;

    getorigin(&sl_xl,&sl_yb);
    getsize(&sl_xr,&sl_yt);
    pushmatrix();
    ortho2((float) sl_xl,(float) (sl_xl+sl_xr),(float) sl_yb,(float) (sl_yb+(sl_yt)));
    getmatrix(barmat);
    sl_xr = sl_xl+sl_xr-10;
    sl_yt = sl_yb+(sl_yt/5)-10;
    sl_xl = sl_xl+12;
    sl_yb = sl_yb+12;
    sl_txd = sl_xr - sl_xl;
    yd = (sl_yt - sl_yb) / 3;
    sl_y1 = sl_yb+5;
    sl_y2 = sl_y1+yd;
    sl_y3 = sl_y2+yd;
    dx = (int)((t_xlast+tmaxx) / (2.0 * tmaxx) * (float)sl_txd);
    dy = (int)((t_ylast+tmaxy) / (2.0 * tmaxy) * (float)sl_txd);
    dz = (int)((t_zlast+tmaxz) / (2.0 * tmaxz) * (float)sl_txd);

    slidet = 293;
    make_bars(slidet,dx,dy,dz);
    popmatrix();
}

init_s_slide()
{
int yd;
int dx,dy,dz;

    getorigin(&sl_xl,&sl_yb);
    getsize(&sl_xr,&sl_yt);
    pushmatrix();
    ortho2((float) sl_xl,(float) (sl_xl+sl_xr),(float) sl_yb,(float) (sl_yb+(sl_yt)));
    getmatrix(barmat);
    sl_xr = sl_xl+sl_xr-10;
    sl_yt = sl_yb+(sl_yt/5)-10;
    sl_xl = sl_xl+12;
    sl_yb = sl_yb+12;
    sl_sxd = (sl_xr - sl_xl) / 2;
    yd = (sl_yt - sl_yb) / 3;
    sl_y1 = sl_yb+5;
    sl_y2 = sl_y1+yd;
    sl_y3 = sl_y2+yd;
    dx = sl_sxd;
    dy = sl_sxd;
    dz = sl_sxd;

    slides = 294;
    make_bars(slides,dx,dy,dz);
    popmatrix();
}

make_bars(objt,dx,dy,dz)
Object objt;
int dx,dy,dz;

{
    makeobj(objt);
    color(BLUE);
    recti(sl_xl,sl_y1,sl_xr,sl_y2-1);
    recti(sl_xl,sl_y2+1,sl_xr,sl_y3-1);
    recti(sl_xl,sl_y3+1,sl_xr,sl_yt);
    recti(sl_xl,sl_yt+1,sl_xr,sl_yt+15);
    color(WHITE);
    cmov2i(sl_xl+12,sl_yt+3);
    switch (objt) {
    case 292:
    charstr("Rotate Bars");
    break;
    case 293:
    charstr("Translate Bars");
    break;
    case 294:
    charstr("Scale Bars");
    break;
    }
    color(GREEN);
    maketag(1);
    rectfi(sl_xl+dz-5,sl_y1,sl_xl+dz+5,sl_y2-1);
    color(BLACK);
    maketag(11);
    cmov2i(sl_xl+dz-4,sl_y1+3);
    charstr("Z");
    color(YELLOW);
    maketag(2);
    rectfi(sl_xl+dy-5,sl_y2+1,sl_xl+dy+5,sl_y3-1);
    color(BLACK);
    maketag(12);
    cmov2i(sl_xl+dy-4,sl_y2+3);
    charstr("Y");
    color(CYAN);
    maketag(3);
    rectfi(sl_xl+dx-5,sl_y3+1,sl_xl+dx+5,sl_yt);
    color(BLACK);
    maketag(13);
    cmov2i(sl_xl+dx-4,sl_y3+3);
    charstr("X");
    closeobj();
}

slidebars(del,ax,objt)
int del;
char ax;
Object objt;
{
    pushmatrix();
    loadmatrix(barmat);
    editobj(objt);
    switch (ax) {
    case 'z' :
        objreplace(1);
        rectfi(sl_xl+del-5,sl_y1,sl_xl+del+5,sl_y2-1);
        objreplace(11);
        cmov2i(sl_xl+del-4,sl_y1+3);
        break;
    case 'y':
        objreplace(2);
        rectfi(sl_xl+del-5,sl_y2+1,sl_xl+del+5,sl_y3-1);
        objreplace(12);
        cmov2i(sl_xl+del-4,sl_y2+3);
        break;
    case 'x':
        objreplace(3);
        rectfi(sl_xl+del-5,sl_y3+1,sl_xl+del+5,sl_yt);
        objreplace(13);
        cmov2i(sl_xl+del-4,sl_y3+3);
        break;
    }
    closeobj();
    popmatrix();
}

draw_slider(objt)
Object objt;
{
    pushmatrix();
    loadmatrix(barmat);
    callobj(objt);
    popmatrix();
}

which_ax(yd,type)
int yd;
char type;
{
int val;

    val = 1;
    if (yd < sl_y2) {
        axis = 'z';
        switch (type) {
        case 'r':
        rlast = r_zlast;
        break;
        case 's':
        slast = s_zlast;
        break;
        case 't':
        tlast = t_zlast;
        break;
        }
    } else if (yd > (sl_yt+5)) {
        val = -1;
    } else if ((yd > sl_y3) && (yd < (sl_yt+5))) {
        axis = 'x';
        switch (type) {
        case 'r':
        rlast = r_xlast;
        break;
        case 's':
        slast = s_xlast;
        break;
        case 't':
        tlast = t_xlast;
        break;
        }
    } else {
        axis = 'y';
        switch (type) {
        case 'r':
        rlast = r_ylast;
        break;
        case 's':
        slast = s_ylast;
        break;
        case 't':
        tlast = t_ylast;
        break;
        }
    }
    return(val);
}

get_r_vals(scrn,pos,rot)
int scrn,*pos;
Angle *rot;
{
int val;

*pos = ((scrn>sl_xr)?sl_xr:((scrn<sl_xl)?sl_xl:scrn))-sl_xl;
*rot = *pos * sl_rxd - rlast;
}

get_t_vals(scrn,pos,tra,max)
int scrn,*pos;
Coord *tra,max;
{
int val;

*pos = ((scrn>sl_xr)?sl_xr:((scrn<sl_xl)?sl_xl:scrn))-sl_xl;
*tra = (((float) *pos) / ((float) sl_txd)) * 2.0 * max - max - tlast;
}

get_s_vals(scrn,pos,sca)
int scrn,*pos;
float *sca;
{
int val;

*pos = ((scrn>sl_xr)?sl_xr:((scrn<sl_xl)?sl_xl:scrn))-sl_xl;
if (*pos < sl_sxd)
*sca = (((float) *pos)/((float) sl_sxd)) * 0.99 + .01;
else *sca = (((float) (*pos-sl_sxd))/((float) sl_sxd)) * 9.0 + 1.0;
}
