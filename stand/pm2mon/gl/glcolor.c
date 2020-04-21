# include "glx.h"


ColorVec glx_Black = {0x0000,0x0000,0x0000};
ColorVec glx_White = {0xFFFF,0xFFFF,0xFFFF};
ColorVec glx_Red = {0xFFFF,0x0000,0x0000};
ColorVec glx_Green = {0x0000,0xFFFF,0x0000};
ColorVec glx_Blue = {0x0000,0x0000,0xFFFF};
ColorVec glx_Yellow = {0xFFFF,0xFFFF,0x0000};
ColorVec glx_Purple = {0xFFFF,0x0000,0xFFFF};

ColorVec *glx_Colors[] =
{
	&glx_Black,
	&glx_White,
	&glx_White,
	&glx_White,
	&glx_Green,
	&glx_Red,
	&glx_Blue,
	&glx_Yellow
};

glmapcolor(c,r,g,b)
    int c;
    short r,g,b;
{
    ColorVec v;
    register int oldmapnum;

    oldmapnum = GLX.mapnum;

    /* set the color in all maps */
    v.r = r; v.g = g; v.b = b;
    (*GLX.mapcolor)(3,c,&v);
    (*GLX.mapcolor)(2,c,&v);
    (*GLX.mapcolor)(1,c,&v);
    (*GLX.mapcolor)(0,c,&v);

    /* restore old map */
    glsetmap(oldmapnum);
}

glloadcolormap()
{
    register ColorVec **vp;
    register short i;

    /* set up both color maps same */
    vp = glx_Colors;
    for( i = 0; i < N_GLX_COLORS; i++ )
    {
	glmapcolor(i,(*vp)->r,(*vp)->g,(*vp)->b);
	vp++;
    }

    /* map 0:  logical plane 0 for front text, 1 for cursor */
    (*GLX.mapcolor)(0,CHARWEB,&glx_Green);
    /* map 2:  logical plane 0 for front text, 1 for back text */
    (*GLX.mapcolor)(2,CHARWEB,&glx_Black);

    /* map 1:  logical plane 1 for front text, 0 for cursor */
    (*GLX.mapcolor)(1,CHARWEA,&glx_Green);
    /* map 3:  logical plane 1 for front text, 0 for back text */
    (*GLX.mapcolor)(3,CHARWEA,&glx_Black);

    /* leave it set to map 0 */
    glsetmap(0);
}

glsetmap(m)
    int m;
{
    (*GLX.setmap)(m);
    GLX.mapnum = m;
}
