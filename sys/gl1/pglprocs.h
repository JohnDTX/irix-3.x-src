(**************************************************************************
 *                                                                        *
 *          Copyright (C) 1984, Silicon Graphics, Inc.                    *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************)

(* pglprocs.h - procedure declarations section for 
    Pascal graphics *)

(* these entries are for the remote gl and are here (commented out)
   for future expansion

procedure    G_arcdata(x5,x4,x3,x2,x1;i,j:longint);cexternal;
procedure    G_arcfdata(x5,x4,x3,x2,x1;i,j:longint);cexternal;
procedure    G_balways(x1;i,j:longint);cexternal;
procedure    G_bifneg(x2,x1;i,j:longint);cexternal;
procedure    G_circdata(x3,x2,x1;i,j:longint);cexternal;
procedure    G_circfdata(x3,x2,x1;i,j:longint);cexternal;
procedure    G_curvepit(x1;i,j:longint);cexternal;
procedure    G_loccharstr(x1;i,j:longint);cexternal;
procedure    G_lookatdata(x7,x6,x5,x4,x3,x2,x1;i,j:longint);cexternal;
procedure    G_movep(x3,x2,x1;i,j:longint);cexternal;
procedure    G_nop;i,j:longint);cexternal;
procedure    G_objchar(x1;i,j:longint);cexternal;
procedure    G_objcharstring(x1;i,j:longint);cexternal;
procedure    G_objfont(x1;i,j:longint);cexternal;
procedure    G_ortho2data(x4,x3,x2,x1;i,j:longint);cexternal;
procedure    G_orthodata(x6,x5,x4,x3,x2,x1;i,j:longint);cexternal;
procedure    G_perspectivedata(x4,x3,x2,x1;i,j:longint);cexternal;
procedure    G_polarviewdata(x4,x3,x2,x1;i,j:longint);cexternal;
procedure    G_rotatex(x1;i,j:longint);cexternal;
procedure    G_rotatey(x1;i,j:longint);cexternal;
procedure    G_rotatez(x1;i,j:longint);cexternal;
procedure    G_windowdata(x6,x5,x4,x3,x2,x1;i,j:longint);cexternal;
*)

(*  G_P2CSTR is a function that transforms Pascal strings
    to C strings.  It can be used only when the string is
    a temporary value (reference parameter)
*)

function  G_P2CSTR(str:String128):pstrng;cexternal;

(*  Each Pascal entry in the graphics library is defined below.
    Those which are intcmd calls are named by affixing G_ to the
    routine name.  A jump to _gl_intcmd (the intcmd entry) is 
    made in a jump table which is loaded by pc for all pascal programs
    which use graphics.  Note that the argument lists of ALL graphics
    routines have been reversed (see the file pglmacs.h).
*)

procedure G_backbuffer(x1,i,j:longint);cexternal;
procedure G_bbox(z2,y2,x2,z1,y1,x1:Coord; 
                 ymin,xmin:longint;i,j:longint);cexternal;
procedure G_bbox2(y2,x2,y1,x1:Coord; ymin,xmin:longint;
                  i,j:longint);cexternal;
procedure G_bbox2i(y2,x2,y1,x1:Icoord;ymin,xmin:longint;
                   i,j:longint);cexternal;
procedure G_bboxi(z2,y2,x2,z1,y1,x1:Icoord;xmin,ymin:longint;
                  i,j:longint);cexternal;
procedure G_callobj(obj:Object;i,j:longint);cexternal;
procedure G_charstr(str:pstrng;i,j:longint);cexternal;
procedure G_clear(i,j:longint);cexternal;
procedure G_closep(i,j:longint);cexternal;
procedure G_cmov(z,y,x:Coord;i,j:longint);cexternal;
procedure G_cmov2(y,x:Coord;i,j:longint);cexternal;
procedure G_cmov2i(y,x:Icoord;i,j:longint);cexternal;
procedure G_cmovi(z,y,x:Icoord;i,j:longint);cexternal;
procedure G_color(c:longint;i,j:longint);cexternal;
procedure G_curvedata(i,j:longint);cexternal;
procedure G_curveit(iterationcount:longint;i,j:longint);cexternal;
procedure G_draw(z,y,x:Coord;i,j:longint);cexternal;
procedure G_draw2(y,x:Coord;i,j:longint);cexternal;
procedure G_draw2i(y,x:Icoord;i,j:longint);cexternal;
procedure G_drawi(z,y,x:Icoord;i,j:longint);cexternal;
procedure G_font(fntnum:longint;i,j:longint);cexternal;
procedure G_frontbuffer(b:longint;i,j:longint);cexternal;
procedure G_linewidth(n:longint;i,j:longint);cexternal;
procedure G_loadmatrix(var m:Matrix;i,j:longint);cexternal;
procedure G_lsbackup(b:longint;i,j:longint);cexternal;
procedure G_modify(v:longint;b,a:real;argnum:longint;
                   offst:Offset;tagname:Tag;obj:Object;
                   i,j:longint);cexternal;
procedure G_move(z,y,x:Coord;i,j:longint);cexternal;
procedure G_move2(y,x:Coord;i,j:longint);cexternal;
procedure G_move2i(y,x:Icoord;i,j:longint);cexternal;
procedure G_movei(z,y,x:Icoord;i,j:longint);cexternal;
procedure G_multmatrix(var m:Matrix;i,j:longint);cexternal;
procedure G_namesoff(i,j:longint);cexternal;
procedure G_nameson(i,j:longint);cexternal;
procedure G_nop(i,j:longint);cexternal;
procedure G_objfont(fntnum:longint;i,j:longint);cexternal;
procedure G_pnt(z,y,x:Coord;i,j:longint);cexternal;
procedure G_pnt2(y,x:Coord;i,j:longint);cexternal;
procedure G_pnt2i(y,x:Icoord;i,j:longint);cexternal;
procedure G_pnti(z,y,x:Icoord;i,j:longint);cexternal;
procedure G_polf(var parray:Coord3array;n:longint;
                 i,j:longint);cexternal;
procedure G_polf2(var parray:Coord2array;n:longint;
                  i,j:longint);cexternal;
procedure G_polf2i(var parray:Icoord2array;n:longint;
                   i,j:longint);cexternal;
procedure G_polfi(var parray:Icoord3array;n:longint;
                  i,j:longint);cexternal;
procedure G_poly(var parray:Coord3array;n:longint;
                 i,j:longint);cexternal;
procedure G_poly2(var parray:Coord2array;n:longint;
                  i,j:longint);cexternal;
procedure G_poly2i(var parray:Icoord2array;n:longint;
                   i,j:longint);cexternal;
procedure G_polyi(var parray:Icoord3array;n:longint;
                  i,j:longint);cexternal;
procedure G_popattributes(i,j:longint);cexternal;
procedure G_popmatrix(i,j:longint);cexternal;
procedure G_popviewport(i,j:longint);cexternal;
procedure G_pushattributes(i,j:longint);cexternal;
procedure G_pushmatrix(i,j:longint);cexternal;
procedure G_pushviewport(i,j:longint);cexternal;
procedure G_rect(y2,x2,y1,x1:Coord;i,j:longint);cexternal;
procedure G_rectf(y2,x2,y1,x1:Coord;i,j:longint);cexternal;
procedure G_rectfi(y2,x2,y1,x1:Icoord;i,j:longint);cexternal;
procedure G_recti(y2,x2,y1,x1:Icoord;i,j:longint);cexternal;
procedure G_resetls(b:longint;i,j:longint);cexternal;
procedure G_RGBcolor(blue,green,red:longint;i,j:longint);cexternal;
procedure G_RGBwritemask(blue,green,red:longint;i,j:longint);cexternal;
procedure G_scale(z,y,x:real;i,j:longint);cexternal;
procedure G_scrmask(top,bottom,right,left:longint;
                    i,j:longint);cexternal;
procedure G_setdepth(far,near:longint;i,j:longint);cexternal;
procedure G_setlinestyle(index:longint;i,j:longint);cexternal;
procedure G_settexture(index:longint;i,j:longint);cexternal;
procedure G_translate(z,y,x:Coord;i,j:longint);cexternal;
procedure G_viewport(top,bottom,right,left:longint;
                     i,j:longint);cexternal;
procedure G_writemask(wtm:longint;i,j:longint);cexternal;

(*  The following routines are NOT calls to intcmd.
    These routine names are simply the C entries 
    (with an underbar prefix)
*)

procedure _arc(endang,startang:longint;radius,y,x:Coord);cexternal;
procedure _arcf(endang,startang:longint;radius,y,x:Coord);cexternal;
procedure _arci(endang,startang:longint;radius,y,x:Icoord);cexternal;
procedure _arcfi(endang,startang:longint;radius,y,x:Icoord);cexternal;
procedure _attachcursor(vy,vx:longint);cexternal;
procedure _blink(blue,green,red:longint;color:longint;
            rate:longint);cexternal;
procedure _circ(radius,y,x:Coord);cexternal;
procedure _circf(radius,y,x:Coord);cexternal;
procedure _circfi(radius,y,x:Icoord);cexternal;
procedure _circi(radius,y,x:Icoord);cexternal;
procedure _clearhitcode;cexternal;
function _clipline(var index:Boolarray;var postline:Coord4array;
                   var preline:Coord3array;
                   n:longint;mask:pstrng):longint;cexternal;
procedure _clippnt(var index:Boolarray;var postrans:Coord4array;
                   var pretrans:Coord3array;
                   n:longint;mask:pstrng);cexternal;
function _clippoly(var postpoly:Coord4array;var prepoly:Coord3array;
                   n:longint;mask:pstrng):longint;cexternal;
procedure _closeobj;cexternal;
procedure _cursoff;cexternal;
procedure _curson;cexternal;
procedure _curve(var geom:Coord3array; var basis:Matrix;
                 precision:longint);cexternal;
procedure _defcursor(var curs:Cursor;n:longint);cexternal;
procedure _deflinestyle(ls:longint;n:longint);cexternal;
procedure _defobjfont(var chars:Objarray;nc,n:longint);cexternal;
procedure _defrasterfont(var raster:Fontraster;nr:longint;
                         var chars:Fntchrarray;nc,ht,n:longint);cexternal;
procedure _deftexture(var tex:Texture;n:longint);cexternal;
procedure _delete(count:longint;offst:Offset;t:Tag);cexternal;
procedure _delobj(obj:Object);cexternal;
procedure _doublebuffer;cexternal;
procedure _editobj(obj:Object);cexternal;
function _endpick(var buffer:lbuffer):longint;cexternal;
function _endselect(var buffer:lbuffer):longint;cexternal;
procedure _finish;cexternal;
procedure _gconfig;cexternal;
function _genobj:Object;cexternal;
function _gentag:Tag;cexternal;
function _getbuffer:Short;cexternal;
function _getbutton(b:longint):Boolean;cexternal;
function _getcmmode:Short;cexternal;
function _getcolor:Colorindex;cexternal;
procedure _getcursor(var b:Boolean;var wtm,color:Colorindex;
                     var index:Short);cexternal;
procedure _getdepth(var far,near:Screencoord);cexternal;
function _getdisplaymode:Short;cexternal;
function _getfont:Short;cexternal;
function _getheight:Short;cexternal;
function _gethitcode:Short;cexternal;
function _getlsbackup:Boolean;cexternal;
function _getlstyle:Short;cexternal;
function _getlwidth:Short;cexternal;
function _getmap:Short;cexternal;
procedure _getmatrix(var m:Matrix);cexternal;
function _getobjfont:Short;cexternal;
function _getplanes:Short;cexternal;
function _getresetls:Boolean;cexternal;
procedure _getscrmask(var top,bottom,right,left:Screencoord);cexternal;
function _gettexture:Short;cexternal;
function _getvaluator(v:longint):Short;cexternal;
procedure _getviewport(var top,bottom,right,left:Screencoord);cexternal;
function _getwritemask:Short;cexternal;
procedure _gRGBcolor(var blue,green,red:RGBvalue);cexternal;
procedure _gRGBcursor(var b:Boolean;
                      var bluem,greenm,redm,blue,green,red:RGBvalue;
                      var index:Short);cexternal;
procedure _gRGBmask(var bluem,greenm,redm:RGBvalue);cexternal;
procedure _ginit;cexternal;
procedure _gexit;cexternal;
procedure _gflush;cexternal;
procedure _greset;cexternal;
procedure _insert(offst:Offset;t:Tag);cexternal;
procedure _isobj(obj:Object);cexternal;
function _istag(t:Tag):Boolean;cexternal;
function _keyboard:char;cexternal;
procedure _lookat(twist:longint;pz,py,px,vz,vy,vx:Coord);cexternal;
procedure _makeobj(obj:Object);cexternal;
procedure _maketag(t:Tag);cexternal;
procedure _mapcolor(blue,green,red:longint;color:longint);cexternal;
procedure _mapw(var wz2,wy2,wx2,wz1,wy1,wx1:Coord;
                sy,sx:longint;vobj:Object);cexternal;
procedure _mapw2(var wy,wx:Coord;sy,sx:longint;vobj:Object);cexternal;
procedure _multimap;cexternal;
procedure _noise(delta:longint;v:longint);cexternal;
procedure _objstr(str:pstrng);cexternal;
procedure _onemap;cexternal;
procedure _ortho(far,near,top,bottom,right,left:Coord);cexternal;
procedure _ortho2(top,bottom,right,left:Coord);cexternal;
procedure _perspective(far,near:Coord;aspect:real;fovy:longint);cexternal;
procedure _pick(numnames:longint);cexternal;
procedure _picksize(deltay,deltax:longint);cexternal;
procedure _polarview(twist,inc,azim:longint;dist:Coord);cexternal;
procedure _qbutton(b:longint);cexternal;
procedure _qenter(val,qtype:longint);cexternal;
procedure _qkeyboard;cexternal;
function _qread(var data:Short):Short;cexternal;
procedure _qreset;cexternal;
function _qtest:Device;cexternal;
procedure _qvaluator(v:longint);cexternal;
function _readpixels(var colors:Colorarray;n:longint):Short;cexternal;
function _readRGB(var blue,green,red:RGBarray;n:longint):Short;cexternal;
procedure _replace(offst:Offset;t:Tag);cexternal;
procedure _RGBcursor(bluem,greenm,redm,blue,green,red:longint;
                     index:longint);cexternal;
procedure _RGBmode;cexternal;
procedure _rotate(axis,zero:char;a:longint);cexternal;
procedure _screenpnt(var index:Boolarray;var postrans:Screenarray;
                     var pretrans:Coord3array;n:longint);cexternal;
procedure _select(numnames:longint);cexternal;
procedure _setbutton(value:longint;b:longint);cexternal;
procedure _setcursor(wtm,color:longint;index:longint);cexternal;
procedure _setmap(mapnum:longint);cexternal;
procedure _setplanes(p:longint);cexternal;
procedure _setvaluator(max,min,init:longint;v:longint);cexternal;
procedure _singlebuffer;cexternal;
function _strwidth(str:pstrng):longint;cexternal;
procedure _swapbuffers;cexternal;
procedure _swapinterval(i:longint);cexternal;
procedure _sync;cexternal;
procedure _tie(v2,v1,b:longint);cexternal;
procedure _transform(var postrans,pretrans:Coord4array;
                     n:longint);cexternal;
procedure _unqbutton(b:longint);cexternal;
procedure _unqkeyboard;cexternal;
procedure _unqvaluator(v:longint);cexternal;
procedure _window(far,near,top,bottom,right,left:Coord);cexternal;
procedure _writepixels(var colors:Colorarray;n:longint);cexternal;
procedure _writeRGB(var blue,green,red:RGBarray;n:longint);cexternal;
