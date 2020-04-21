/**************************************************************************
 *                                                                        *
 *          Copyright (C) 1984, Silicon Graphics, Inc.                    *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/
/*
    pglmacs.h - constants and argument list reversing macros
                for the graphics library.  
/*
    macros for transorming the Pascal argument lists
*/
/*
 * $Source: /d2/3.7/src/sys/gl1/RCS/pglmacs.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:32 $
 */
/* graphics libary header file */
/* maximum X and Y screen coordinates */
/* approximate interval between retraces in milliseconds */
/* names for colors in color map loaded by ginit() */
/* typedefs */
/* command type tokens */
/*  
    the following routines are really intcmds in disguise
*/
#define arcdata(x1,x2,x3,x4,x5) \
                G_arcdata(x5,x4,x3,x2,x1,260,	37)
#define arcfdata(x1,x2,x3,x4,x5) \
                G_arcfdata(x5,x4,x3,x2,x1,268,	38)
#define backbuffer(x1)    G_backbuffer(x1,400,	12)
#define balways(x1)   G_balways(x1,340,	12)
#define bbox(x1,x2,x3,x4,x5,x6,x7,x8) \
                G_bbox(x8,x7,x6,x5,x4,x3,x2,x1,224,	22)
#define bbox2(x1,x2,x3,x4,x5,x6) \
                G_bbox2(x6,x5,x4,x3,x2,x1,232,	24) 
#define bbox2i(x1,x2,x3,x4,x5,x6) \
                G_bbox2i(x6,x5,x4,x3,x2,x1,236,	25)
#define bboxi(x1,x2,x3,x4,x5,x6,x7,x8) \
                G_bboxi(x8,x7,x6,x5,x4,x3,x2,x1,228,	23)
#define bifneg(x1,x2)     G_bifneg(x2,x1,336,	32	/* hidden surface add-on */)
#define callobj(x1)   G_callobj(x1,8,	11)
#define charstr(x1)   G_charstr(x1,168,	40) 
#define circdata(x1,x2,x3)    G_circdata(x3,x2,x1,244,	35)
#define circfdata(x1,x2,x3)   G_circfdata(x3,x2,x1,252,36)
#define clear     G_clear(180,	13)
#define closep    G_closep(332,	13)
#define cmov(x1,x2,x3)    G_cmov(x3,x2,x1,152,	1)
#define cmov2(x1,x2)  G_cmov2(x2,x1,160,	3)
#define cmov2i(x1,x2)     G_cmov2i(x2,x1,164,	4)
#define cmovi(x1,x2,x3)   G_cmovi(x3,x2,x1,156,	2)
#define color(x1)     G_color(x1,196,	12)
#define curvedata     G_curvedata(392,	13)
#define curveit(x1)   G_curveit(x1,144,	12)
#define curvepit(x1)  G_curvepit(x1,328,	12)
#define draw(x1,x2,x3)    G_draw(x3,x2,x1,28,	1)
#define draw2(x1,x2)  G_draw2(x2,x1,36,	3)
#define draw2i(x1,x2)     G_draw2i(x2,x1,40,	4)
#define drawi(x1,x2,x3)   G_drawi(x3,x2,x1,32,	2)
#define drawp(x1,x2,x3)   G_drawp(x3,x2,x1,380,	1)
#define font(x1)  G_font(x1,148,	12)
#define frontbuffer(x1)   G_frontbuffer(x1,396,	12)
#define linewidth(x1)     G_linewidth(x1,188,	12)
#define loadmatrix(x1)    G_loadmatrix(x1,116,	19)
#define loccharstr(x1)    G_loccharstr(x1,172,	11)
#define lookatdata(x1,x2,x3,x4,x5,x6,x7) \
                G_lookatdata(x7,x6,x5,x4,x3,x2,x1,280,	30	/* lookat */)
#define lsbackup(x1)  G_lsbackup(x1,208,	12)
#define modify(x1,x2,x3,x4,x5,x6,x7) \
                G_modify(x7,x6,x5,x4,x3,x2,x1,272,	26	/* modify */)
#define move(x1,x2,x3)    G_move(x3,x2,x1,12,	1)
#define move2(x1,x2)  G_move2(x2,x1,20,	3)
#define move2i(x1,x2)     G_move2i(x2,x1,24,	4)
#define movei(x1,x2,x3)   G_movei(x3,x2,x1,16,	2)
#define movep(x1,x2,x3)   G_movep(x3,x2,x1,324,	1)
#define multmatrix(x1)    G_multmatrix(x1,92,	18)
#define namesoff  G_namesoff(404,	13)
#define nameson   G_nameson(408,	13)
#define nop   G_nop(384,	13)
#define objchar(x1)   G_objchar(x1,376,	11)
#define objcharstring(x1)     G_objcharstring(x1,372,	12)
#define objfont(x1)   G_objfont(x1,356,	12)
#define ortho2data(x1,x2,x3,x4) \
                G_ortho2data(x4,x3,x2,x1,304,	29	/* ortho2 */)
#define orthodata(x1,x2,x3,x4,x5,x6) \
                G_orthodata(x6,x5,x4,x3,x2,x1,288,	27	/* ortho, window */)
#define perspectivedata(x1,x2,x3,x4) \
                G_perspectivedata(x4,x3,x2,x1,312,	28	/* perspective */)
#define pnt(x1,x2,x3)     G_pnt(x3,x2,x1,128,	1)
#define pnt2(x1,x2)   G_pnt2(x2,x1,136,	3)
#define pnt2i(x1,x2)  G_pnt2i(x2,x1,140,	4)
#define pnti(x1,x2,x3)    G_pnti(x3,x2,x1,132,	2)
#define polarviewdata(x1,x2,x3,x4) \
                G_polarviewdata(x4,x3,x2,x1,320,	31	/* polarview */)
#define polf(x1,x2)   G_polf(x2,x1,60,	5)
#define polf2(x1,x2)  G_polf2(x2,x1,68,	7)
#define polf2i(x1,x2)     G_polf2i(x2,x1,72,	8)
#define polfi(x1,x2)  G_polfi(x2,x1,64,	6)
#define poly(x1,x2)   G_poly(x2,x1,44,	5)
#define poly2(x1,x2)  G_poly2(x2,x1,52,	7)
#define poly2i(x1,x2)     G_poly2i(x2,x1,56,	8)
#define polyi(x1,x2)  G_polyi(x2,x1,48,	6)
#define popattributes     G_popattributes(216,	13)
#define popmatrix     G_popmatrix(124,	13)
#define popviewport   G_popviewport(368,	13)
#define pushattributes    G_pushattributes(212,	13)
#define pushmatrix    G_pushmatrix(120,	13)
#define pushviewport  G_pushviewport(364,	13)
#define rect(x1,x2,x3,x4)     G_rect(x4,x3,x2,x1,76,	9)
#define rectf(x1,x2,x3,x4)    G_rectf(x4,x3,x2,x1,84,	9)
#define rectfi(x1,x2,x3,x4)   G_rectfi(x4,x3,x2,x1,88,	10)
#define recti(x1,x2,x3,x4)    G_recti(x4,x3,x2,x1,80,	10)
#define resetls(x1)   G_resetls(x1,204,	12)
#define RGBcolor(x1,x2,x3)    G_RGBcolor(x3,x2,x1,348,	39)
#define RGBwritemask(x1,x2,x3) \
                G_RGBwritemask(x3,x2,x1,352,	39)
#define rotatex(x1)   G_rotatex(x1,104,	17)
#define rotatey(x1)   G_rotatey(x1,108,	17)
#define rotatez(x1)   G_rotatez(x1,112,	17)
#define scale(x1,x2,x3)   G_scale(x3,x2,x1,96,	1)
#define scrmask(x1,x2,x3,x4)  G_scrmask(x4,x3,x2,x1,360,	34	/* character viewport */)
#define setdepth(x1,x2)   G_setdepth(x2,x1,344,	33	/* setdepth */)
#define setlinestyle(x1)  G_setlinestyle(x1,184,	12)
#define settexture(x1)    G_settexture(x1,192,	12)
#define translate(x1,x2,x3)   G_translate(x3,x2,x1,100,	1)
#define viewport(x1,x2,x3,x4)     G_viewport(x4,x3,x2,x1,176,	20)
#define windowdata(x1,x2,x3,x4,x5,x6) \
                G_windowdata(x6,x5,x4,x3,x2,x1,296,	27	/* ortho, window */)
#define writemask(x1)     G_writemask(x1,200,	12)
/* 
    The following are NOT intcmds 
*/
#define arc(x,y,radius,startang,endang) \
                _arc(endang,startang,radius,y,x) 
#define arcf(x,y,radius,startang,endang) \
                _arcf(endang,startang,radius,y,x)    
#define arci(x,y,radius,startang,endang) \
                _arci(endang,startang,radius,y,x)    
#define arcfi(x,y,radius,startang,endang) \
                _arcfi(endang,startang,radius,y,x)
#define attachcursor(vx,vy) \
                _attachcursor(vy,vx)     
#define blink(rate,color,red,green,blue) \
                _blink(blue,green,red,color,rate)    
#define circ(x,y,radius) _circ(radius,y,x)    
#define circf(x,y,radius) _circf(radius,y,x)  
#define circfi(x,y,radius) _circfi(radius,y,x)    
#define circi(x,y,radius) _circi(radius,y,x)  
#define clipline(mask,n,preline,postline,index) \
                _clipline(index,postline,preline,n,mask)     
#define clippnt(mask,n,pretrans,postrans,index) \
                _clippnt(index,postrans,pretrans,n,mask)     
#define clippoly(mask,n,prepoly,postpoly) \
                _clippoly(postpoly,prepoly,n,mask)
#define closeobj _closeobj
#define cursoff  _cursoff
#define curson _curson
#define curve(precision,basis,geom) _curve(geom,basis,precision)  
#define defcursor(n,curs) _defcursor(curs,n)  
#define deflinestyle(n,ls) _deflinestyle(ls,n)    
#define defobjfont(n,nc,chars) _defobjfont(chars,nc,n)    
#define defrasterfont(n,ht,nc,nr,chars,raster) \
                _defrasterfont(raster,chars,nr,nc,ht,n)  
#define deftexture(n,tex) _deftexture(tex,n)  
#define delete(t,offst,count) _delete(count,offst,t)  
#define delobj(obj) _delobj(obj)  
#define doublebuffer _doublebuffer
#define editobj(obj) _editobj(obj)    
#define endpick(buffer) _endpick(buffer)  
#define endselect(buffer) _endselect(buffer)  
#define finish _finish
#define gconfig _gconfig
#define genobj _genobj
#define gettag _gentag
#define getbuffer _getbuffer
#define getbutton(b) _getbutton(b)    
#define getcmmode _getcmmode
#define getcolor _getcolor
#define getcursor(index,color,wtm,b) _getcursor(b,wtm,color,index)    
#define getdepth(near,far) _getdepth(far,near)    
#define getdisplaymode _getdisplaymode
#define getfont _getfont
#define getheight _getheight
#define gethitcode _gethitcode
#define getlsbackup _getlsbackup
#define getlstyle _getlstyle
#define getlwidth _getlwidth 
#define getmap _getmap
#define getmatrix(m) _getmatrix(m)    
#define getobjfont:longint;cexternal;
#define getplanes _getplanes
#define getresetls _getresetls
#define getscrmask(left,right,bottom,top) \
                _getscrmask(top,bottom,right,left)
#define gettexture _gettexture
#define getvaluator(v) _getvaluator(v)    
#define getwritemask _getwritemask
#define getviewport(left,right,bottom,top) \
                _getviewport(top,bottom,right,left)
#define ginit _ginit
#define gexit _gexit
#define gflush _gflush
#define greset _greset
#define gRGBcolor(red,green,blue) _gRGBcolor(blue,green,red)  
#define gRGBcursor(index,red,green,blue,redm,greenm,bluem,b) \
            _gRGBcursor(b,bluem,greenm,redm,blue,green,red,index)    
#define gRGBmask(redm,greenm,bluem) _gRGBmask(bluem,greenm,redm)  
#define insert(t,offst) _insert(offst,t)  
#define isobj(obj) _isobj(obj)    
#define istag(t) _istag(t)    
#define keyboard _keyboard
#define lookat(vx,vy,vz,px,py,pz,twist) \
                _lookat(twist,pz,py,px,vz,vy,vx)     
#define makeobj(obj) _makeobj(obj)    
#define maketag(t) _maketag(t)    
#define mapcolor(color,red,green,blue) \
                _mapcolor(blue,green,red,color)  
#define mapw(vobj,sx,sy,wx1,wy1,wz1,wx2,wy2,wz2) \
            _mapw(wz2,wy2,wx2,wz1,wy1,wx1,sy,sx,vobj)    
#define mapw2(vobj,sx,sy,wx,wy) _mapw2(wy,wx,sy,sx,vobj)  
#define multimap _multimap
#define noise(v,delta) _noise(delta,v)    
#define objstr(str) _objstr(str)  
#define onemap _onemap
#define ortho(left,right,bottom,top,near,far)  \
            _ortho(far,near,top,bottom,right,left)   
#define ortho2(left,right,bottom,top) _ortho2(top,bottom,right,left)  
#define perspective(fovy,aspect,near,far) \
                _perspective(far,near,aspect,fovy)
#define pick(numnames) _pick(numnames)    
#define picksize(deltax,deltay) _picksize(deltay,deltax)  
#define polarview(dist,azim,inc,twist) \
                _polarview(twist,inc,azim,dist)  
#define qbutton(b) _qbutton(b)    
#define qenter(qtype,val) _qenter(val,qtype)  
#define qkeyboard _qkeyboard
#define qread(data) _qread(data)  
#define qreset _qreset
#define qtest _qtest
#define qvaluator(v) _qvaluator(v)    
#define readpixels(n,colors) _readpixels(colors,n)    
#define readRGB(n,red,green,blue) _readRGB(blue,green,red,n)  
#define replace(t,offst) _replace(offst,t)    
#define RGBcursor(index,red,green,blue,redm,greenm,bluem) \
            _RGBcursor(bluem,greenm,redm,blue,green,red,index)   
#define RGBmode _RGBmode
#define rotate(a,axis) _rotate(axis,'\0',a)   
#define screenpnt(n,pretrans,postrans,index) \
            _screenpnt(index,postrans,pretrans,n)    
#define select(numnames) _select(numnames)    
#define setbutton(b,value) _setbutton(value,b)    
#define setcursor(index,color,wtm) \
                _setcursor(wtm,color,index)  
#define setmap(mapnum) _setmap(mapnum)    
#define setplanes(p) _setplanes(p)    
#define setvaluator(v,init,min,max) \
                _setvaluator(max,min,init,v)     
#define singlebuffer _singlebuffer
#define strwidth(str) _strwidth(str)  
#define swapbuffers _swapbuffers
#define swapinterval(i) _swapinterval(i)  
#define sync _sync
#define tie(b,v1,v2) _tie(v2,v1,b)    
#define transform(n,pretrans,postrans) \
                _transform(postrans,pretrans,n)  
#define unqbutton(b) _unqbutton(b)    
#define unqkeyboard _unqkeyboard
#define unqvaluator(v) _unqvaluator(v)    
#define window(left,right,bottom,top,near,far)   \
            _window(far,near,top,bottom,right,left)  
#define writepixels(n,colors) _writepixels(colors,n)  
#define writeRGB(n,red,green,blue) _writeRGB(blue,green,red,n)    
