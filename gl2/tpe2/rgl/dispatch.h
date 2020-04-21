/*
**	Dispatch table
*/

#define NULL 0

extern int bogus();
extern int spl_feedback();
extern int spl_pick();
extern int spl_select();
extern int xsetslowcom();
extern int xsetfastcom();
extern int gversion();
extern int gdownload();
extern int pagecolor();
extern int textcolor();
extern int textport();
extern int arc();
extern int arcf();
extern int arcfi();
extern int arci();
extern int attachcursor();
extern int backbuffer();
extern int bbox2();
extern int bbox2i();
extern int blink();
extern int callobj();
extern int charstr();
extern int circ();
extern int circf();
extern int circfi();
extern int circi();
extern int clear();
extern int clearhitcode();
extern int closeobj();
extern int cmov();
extern int cmov2();
extern int cmov2i();
extern int cmovi();
extern int color();
extern int cursoff();
extern int curson();
extern int curveit();
extern int defcursor();
extern int deflinestyle();
extern int defrasterfont();
extern int delobj();
extern int doublebuffer();
extern int draw();
extern int draw2();
extern int draw2i();
extern int drawi();
extern int editobj();
extern int ret_endpick();
extern int ret_endselect();
extern int finish();
extern int font();
extern int frontbuffer();
extern int gconfig();
extern int genobj();
extern int gentag();
extern int getbuffer();
extern int getbutton();
extern int getcmmode();
extern int getcolor();
extern int getcursor();
extern int getdepth();
extern int getdisplaymode();
extern int getfont();
extern int getheight();
extern int gethitcode();
extern int getlsbackup();
extern int getlstyle();
extern int getlwidth();
extern int getmap();
extern int ret_getmatrix();
extern int getplanes();
extern int getresetls();
extern int getscrmask();
extern int getvaluator();
extern int getviewport();
extern int getwritemask();
extern int xginit();
extern int xgreset();
extern int gRGBcolor();
extern int gRGBcursor();
extern int gRGBmask();
extern int isobj();
extern int istag();
extern int linewidth();
extern int loadmatrix();
extern int lookat();
extern int lsbackup();
extern int makeobj();
extern int maketag();
extern int mapcolor();
extern int mapw();
extern int mapw2();
extern int move();
extern int move2();
extern int move2i();
extern int movei();
extern int multimap();
extern int multmatrix();
extern int noise();
extern int onemap();
extern int ortho();
extern int ortho2();
extern int perspective();
extern int spl_pick();
extern int picksize();
extern int pnt();
extern int pnt2();
extern int pnt2i();
extern int pnti();
extern int polarview();
extern int polf();
extern int polf2();
extern int polf2i();
extern int polfi();
extern int poly();
extern int poly2();
extern int poly2i();
extern int polyi();
extern int popattributes();
extern int popmatrix();
extern int popviewport();
extern int pushattributes();
extern int pushmatrix();
extern int pushviewport();
extern int qenter();
extern int qread();
extern int qreset();
extern int qtest();
extern int ret_readpixels();
extern int ret_readRGB();
extern int rect();
extern int rectf();
extern int rectfi();
extern int recti();
extern int resetls();
extern int RGBcolor();
extern int RGBcursor();
extern int RGBmode();
extern int RGBwritemask();
extern int rotate();
extern int scale();
extern int scrmask();
extern int spl_select();
extern int setbutton();
extern int setcursor();
extern int setdepth();
extern int setlinestyle();
extern int setmap();
extern int setvaluator();
extern int singlebuffer();
extern int strwidth();
extern int swapbuffers();
extern int swapinterval();
extern int gsync();
extern int tie();
extern int translate();
extern int viewport();
extern int window();
extern int writemask();
extern int writepixels();
extern int writeRGB();
extern int tpon();
extern int tpoff();
extern int textwritemask();
extern int xgexit();
extern int clkon();
extern int clkoff();
extern int lampon();
extern int lampoff();
extern int setbell();
extern int ringbell();
extern int tadelay();
extern int arcfs();
extern int arcs();
extern int bbox2s();
extern int blankscreen();
extern int ret_blkqread();
extern int getmcolor();
extern int callfunc();
extern int chunksize();
extern int circfs();
extern int circs();
extern int cmov2s();
extern int cmovs();
extern int compactify();
extern int qdevice();
extern int unqdevice();
extern int curvebasis();
extern int curveprecision();
extern int crv();
extern int gettp();
extern int gbegin();
extern int textinit();
extern int crvn();
extern int defbasis();
extern int deltag();
extern int depthcue();
extern int draw2s();
extern int draws();
extern int ret_endfeedback();
extern int spl_feedback();
extern int getcpos();
extern int getdcm();
extern int getgpos();
extern int getlsrepeat();
extern int getmem();
extern int getmonitor();
extern int getopenobj();
extern int getzbuffer();
extern int gewrite();
extern int initnames();
extern int loadname();
extern int lsrepeat();
extern int move2s();
extern int moves();
extern int newtag();
extern int passthrough();
extern int patchbasis();
extern int patchprecision();
extern int patch();
extern int pclos();
extern int pdr();
extern int pdr2();
extern int pdri();
extern int pdr2i();
extern int pdrs();
extern int pdr2s();
extern int polf2s();
extern int polfs();
extern int poly2s();
extern int polys();
extern int pmv();
extern int pmv2();
extern int pmvi();
extern int pmv2i();
extern int pmvs();
extern int pmv2s();
extern int pnt2s();
extern int pnts();
extern int popname();
extern int pushname();
extern int rdr();
extern int rdr2();
extern int rdri();
extern int rdr2i();
extern int rdrs();
extern int rdr2s();
extern int rectcopy();
extern int rmv();
extern int rmv2();
extern int rmvi();
extern int rmv2i();
extern int rmvs();
extern int rmv2s();
extern int rpdr();
extern int rpdr2();
extern int rpdri();
extern int rpdr2i();
extern int rpdrs();
extern int rpdr2s();
extern int rpmv();
extern int rpmv2();
extern int rpmvi();
extern int rpmv2i();
extern int rpmvs();
extern int rpmv2s();
extern int setdblights();
extern int setmonitor();
extern int setshade();
extern int shaderange();
extern int spclos();
extern int splf();
extern int splf2();
extern int splfi();
extern int splf2i();
extern int splfs();
extern int splf2s();
extern int xfpt();
extern int xfpti();
extern int xfpts();
extern int xfpt2();
extern int xfpt2i();
extern int xfpt2s();
extern int xfpt4();
extern int xfpt4i();
extern int xfpt4s();
extern int zbuffer();
extern int charst();
extern int strwid();
extern int defpattern();
extern int getpattern();
extern int setpattern();
extern int objinsert();
extern int objdelete();
extern int objreplace();
extern int zclear();
extern int curorigin();
extern int pagewritemask();
extern int patchcurves();
extern int dbtext();
extern int lastone();

dispatchEntry dispatch[] = {
	{ xsetslowcom, "V", 0, 0, 0, 0 },
	{ xsetfastcom, "V", 0, 0, 0, 0 },
	{ gversion, "L", 0, 0, 0, 1 },
	{ gdownload, "V", 0, 0, 0, 0 },
	{ pagecolor, "Vs", 0, 0, 4, 0 },
	{ textcolor, "Vs", 0, 0, 4, 0 },
	{ textport, "Vssss", 0, 0, 16, 0 },
	{ arc, "Vfffss", 0, 0, 20, 0 },
	{ arcf, "Vfffss", 0, 0, 20, 0 },
	{ arcfi, "Vlllss", 0, 0, 20, 0 },
	{ arci, "Vlllss", 0, 0, 20, 0 },
	{ attachcursor, "Vss", 0, 0, 8, 0 },
	{ backbuffer, "Vo", 0, 0, 4, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bbox2, "Vssffff", 0, 0, 24, 0 },
	{ bbox2i, "Vssllll", 0, 0, 24, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ blink, "Vsssss", 0, 0, 20, 0 },
	{ callobj, "Vl", 0, 0, 4, 0 },
	{ charstr, "Vc", 0, 0, 4, 0 },
	{ circ, "Vfff", 0, 0, 12, 0 },
	{ circf, "Vfff", 0, 0, 12, 0 },
	{ circfi, "Vlll", 0, 0, 12, 0 },
	{ circi, "Vlll", 0, 0, 12, 0 },
	{ clear, "V", 0, 0, 0, 0 },
	{ clearhitcode, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ closeobj, "V", 0, 0, 0, 0 },
	{ cmov, "Vfff", 0, 0, 12, 0 },
	{ cmov2, "Vff", 0, 0, 8, 0 },
	{ cmov2i, "Vll", 0, 0, 8, 0 },
	{ cmovi, "Vlll", 0, 0, 12, 0 },
	{ color, "Vs", 0, 0, 4, 0 },
	{ cursoff, "V", 0, 0, 0, 0 },
	{ curson, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ curveit, "Vs", 0, 0, 4, 0 },
	{ defcursor, "Vsa", 0, 0, 8, 0 },
	{ deflinestyle, "Vss", 0, 0, 8, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ defrasterfont, "Vsssasa", 0, 0, 24, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ delobj, "Vl", 0, 0, 4, 0 },
	{ doublebuffer, "V", 0, 0, 0, 0 },
	{ draw, "Vfff", 0, 0, 12, 0 },
	{ draw2, "Vff", 0, 0, 8, 0 },
	{ draw2i, "Vll", 0, 0, 8, 0 },
	{ drawi, "Vlll", 0, 0, 12, 0 },
	{ editobj, "Vl", 0, 0, 4, 0 },
	{ ret_endpick, "V", 0, 0, 0, 0 },
	{ ret_endselect, "V", 0, 0, 0, 0 },
	{ finish, "V", 0, 0, 0, 0 },
	{ font, "Vs", 0, 0, 4, 0 },
	{ frontbuffer, "Vo", 0, 0, 4, 0 },
	{ gconfig, "V", 0, 0, 0, 0 },
	{ genobj, "L", 0, 0, 0, 1 },
	{ gentag, "L", 0, 0, 0, 1 },
	{ getbuffer, "L", 0, 0, 0, 1 },
	{ getbutton, "Os", 0, 0, 4, 1 },
	{ getcmmode, "O", 0, 0, 0, 1 },
	{ getcolor, "L", 0, 0, 0, 1 },
	{ getcursor, "VSSSO", 0, 0, 16, 1 },
	{ getdepth, "VSS", 0, 0, 8, 1 },
	{ getdisplaymode, "L", 0, 0, 0, 1 },
	{ getfont, "L", 0, 0, 0, 1 },
	{ getheight, "L", 0, 0, 0, 1 },
	{ gethitcode, "L", 0, 0, 0, 1 },
	{ getlsbackup, "O", 0, 0, 0, 1 },
	{ getlstyle, "L", 0, 0, 0, 1 },
	{ getlwidth, "L", 0, 0, 0, 1 },
	{ getmap, "L", 0, 0, 0, 1 },
	{ ret_getmatrix, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ getplanes, "L", 0, 0, 0, 1 },
	{ getresetls, "O", 0, 0, 0, 1 },
	{ getscrmask, "VSSSS", 0, 0, 16, 1 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ getvaluator, "Ls", 0, 0, 4, 1 },
	{ getviewport, "VSSSS", 0, 0, 16, 1 },
	{ getwritemask, "L", 0, 0, 0, 1 },
	{ xginit, "V", 0, 0, 0, 0 },
	{ xgreset, "V", 0, 0, 0, 0 },
	{ gRGBcolor, "VSSS", 0, 0, 12, 1 },
	{ gRGBcursor, "VSSSSSSSO", 0, 0, 32, 1 },
	{ gRGBmask, "VSSS", 0, 0, 12, 1 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ isobj, "Ol", 0, 0, 4, 1 },
	{ istag, "Ol", 0, 0, 4, 1 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ linewidth, "Vs", 0, 0, 4, 0 },
	{ loadmatrix, "Va", 0, 0, 4, 0 },
	{ lookat, "Vffffffs", 0, 0, 28, 0 },
	{ lsbackup, "Vo", 0, 0, 4, 0 },
	{ makeobj, "Vl", 0, 0, 4, 0 },
	{ maketag, "Vl", 0, 0, 4, 0 },
	{ mapcolor, "Vssss", 0, 0, 16, 0 },
	{ mapw, "VlssFFFFFF", 0, 0, 36, 1 },
	{ mapw2, "VlssFF", 0, 0, 20, 1 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ move, "Vfff", 0, 0, 12, 0 },
	{ move2, "Vff", 0, 0, 8, 0 },
	{ move2i, "Vll", 0, 0, 8, 0 },
	{ movei, "Vlll", 0, 0, 12, 0 },
	{ multimap, "V", 0, 0, 0, 0 },
	{ multmatrix, "Va", 0, 0, 4, 0 },
	{ noise, "Vss", 0, 0, 8, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ onemap, "V", 0, 0, 0, 0 },
	{ ortho, "Vffffff", 0, 0, 24, 0 },
	{ ortho2, "Vffff", 0, 0, 16, 0 },
	{ perspective, "Vsfff", 0, 0, 16, 0 },
	{ spl_pick, "V", 0, 0, 0, 0 },
	{ picksize, "Vss", 0, 0, 8, 0 },
	{ pnt, "Vfff", 0, 0, 12, 0 },
	{ pnt2, "Vff", 0, 0, 8, 0 },
	{ pnt2i, "Vll", 0, 0, 8, 0 },
	{ pnti, "Vlll", 0, 0, 12, 0 },
	{ polarview, "Vfsss", 0, 0, 16, 0 },
	{ polf, "Vla", 0, 0, 8, 0 },
	{ polf2, "Vla", 0, 0, 8, 0 },
	{ polf2i, "Vla", 0, 0, 8, 0 },
	{ polfi, "Vla", 0, 0, 8, 0 },
	{ poly, "Vla", 0, 0, 8, 0 },
	{ poly2, "Vla", 0, 0, 8, 0 },
	{ poly2i, "Vla", 0, 0, 8, 0 },
	{ polyi, "Vla", 0, 0, 8, 0 },
	{ popattributes, "V", 0, 0, 0, 0 },
	{ popmatrix, "V", 0, 0, 0, 0 },
	{ popviewport, "V", 0, 0, 0, 0 },
	{ pushattributes, "V", 0, 0, 0, 0 },
	{ pushmatrix, "V", 0, 0, 0, 0 },
	{ pushviewport, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ qenter, "Vss", 0, 0, 8, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ qread, "LS", 0, 0, 4, 1 },
	{ qreset, "V", 0, 0, 0, 0 },
	{ qtest, "L", 0, 0, 0, 1 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ ret_readpixels, "V", 0, 0, 0, 0 },
	{ ret_readRGB, "V", 0, 0, 0, 0 },
	{ rect, "Vffff", 0, 0, 16, 0 },
	{ rectf, "Vffff", 0, 0, 16, 0 },
	{ rectfi, "Vllll", 0, 0, 16, 0 },
	{ recti, "Vllll", 0, 0, 16, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ resetls, "Vo", 0, 0, 4, 0 },
	{ RGBcolor, "Vsss", 0, 0, 12, 0 },
	{ RGBcursor, "Vsssssss", 0, 0, 28, 0 },
	{ RGBmode, "V", 0, 0, 0, 0 },
	{ RGBwritemask, "Vsss", 0, 0, 12, 0 },
	{ rotate, "Vsb", 0, 0, 8, 0 },
	{ scale, "Vfff", 0, 0, 12, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ scrmask, "Vssss", 0, 0, 16, 0 },
	{ spl_select, "V", 0, 0, 0, 0 },
	{ setbutton, "Vso", 0, 0, 8, 0 },
	{ setcursor, "Vsss", 0, 0, 12, 0 },
	{ setdepth, "Vss", 0, 0, 8, 0 },
	{ setlinestyle, "Vs", 0, 0, 4, 0 },
	{ setmap, "Vs", 0, 0, 4, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ setvaluator, "Vssss", 0, 0, 16, 0 },
	{ singlebuffer, "V", 0, 0, 0, 0 },
	{ strwidth, "Lc", 0, 0, 4, 1 },
	{ swapbuffers, "V", 0, 0, 0, 0 },
	{ swapinterval, "Vs", 0, 0, 4, 0 },
	{ gsync, "V", 0, 0, 0, 0 },
	{ tie, "Vsss", 0, 0, 12, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ translate, "Vfff", 0, 0, 12, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ viewport, "Vssss", 0, 0, 16, 0 },
	{ window, "Vffffff", 0, 0, 24, 0 },
	{ writemask, "Vs", 0, 0, 4, 0 },
	{ writepixels, "Vsa", 0, 0, 8, 0 },
	{ writeRGB, "Vsaaa", 0, 0, 16, 0 },
	{ tpon, "V", 0, 0, 0, 0 },
	{ tpoff, "V", 0, 0, 0, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ textwritemask, "Vs", 0, 0, 4, 0 },
	{ xgexit, "V", 0, 0, 0, 0 },
	{ clkon, "V", 0, 0, 0, 0 },
	{ clkoff, "V", 0, 0, 0, 0 },
	{ lampon, "Vb", 0, 0, 4, 0 },
	{ lampoff, "Vb", 0, 0, 4, 0 },
	{ setbell, "Vb", 0, 0, 4, 0 },
	{ ringbell, "V", 0, 0, 0, 0 },
	{ tadelay, "Vs", 0, 0, 4, 0 },
	{ arcfs, "Vsssss", 0, 0, 20, 0 },
	{ arcs, "Vsssss", 0, 0, 20, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ bbox2s, "Vssssss", 0, 0, 24, 0 },
	{ blankscreen, "Vo", 0, 0, 4, 0 },
	{ bogus, "V", 0, 0, 0, 0 },
	{ ret_blkqread, "V", 0, 0, 0, 0 },
	{ getmcolor, "VsSSS", 0, 0, 16, 1 },
	{ callfunc, "V", 0, 0, 0, 0 },
	{ chunksize, "Vl", 0, 0, 4, 0 },
	{ circfs, "Vsss", 0, 0, 12, 0 },
	{ circs, "Vsss", 0, 0, 12, 0 },
	{ cmov2s, "Vss", 0, 0, 8, 0 },
	{ cmovs, "Vsss", 0, 0, 12, 0 },
	{ compactify, "Vl", 0, 0, 4, 0 },
	{ qdevice, "Vs", 0, 0, 4, 0 },
	{ unqdevice, "Vs", 0, 0, 4, 0 },
	{ curvebasis, "Vs", 0, 0, 4, 0 },
	{ curveprecision, "Vs", 0, 0, 4, 0 },
	{ crv, "Va", 0, 0, 4, 0 },
	{ gettp, "VSSSS", 0, 0, 16, 1 },
	{ gbegin, "V", 0, 0, 0, 0 },
	{ textinit, "V", 0, 0, 0, 0 },
	{ crvn, "Vla", 0, 0, 8, 0 },
	{ defbasis, "Vsa", 0, 0, 8, 0 },
	{ deltag, "Vl", 0, 0, 4, 0 },
	{ depthcue, "Vo", 0, 0, 4, 0 },
	{ draw2s, "Vss", 0, 0, 8, 0 },
	{ draws, "Vsss", 0, 0, 12, 0 },
	{ ret_endfeedback, "V", 0, 0, 0, 0 },
	{ spl_feedback, "V", 0, 0, 0, 0 },
	{ getcpos, "VSS", 0, 0, 8, 1 },
	{ getdcm, "O", 0, 0, 0, 1 },
	{ getgpos, "VFFFF", 0, 0, 16, 1 },
	{ getlsrepeat, "L", 0, 0, 0, 1 },
	{ getmem, "L", 0, 0, 0, 1 },
	{ getmonitor, "L", 0, 0, 0, 1 },
	{ getopenobj, "L", 0, 0, 0, 1 },
	{ getzbuffer, "O", 0, 0, 0, 1 },
	{ gewrite, "Val", 0, 0, 8, 0 },
	{ initnames, "V", 0, 0, 0, 0 },
	{ loadname, "Vs", 0, 0, 4, 0 },
	{ lsrepeat, "Vl", 0, 0, 4, 0 },
	{ move2s, "Vss", 0, 0, 8, 0 },
	{ moves, "Vsss", 0, 0, 12, 0 },
	{ newtag, "Vlll", 0, 0, 12, 0 },
	{ passthrough, "Vs", 0, 0, 4, 0 },
	{ patchbasis, "Vll", 0, 0, 8, 0 },
	{ patchprecision, "Vll", 0, 0, 8, 0 },
	{ patch, "Vaaa", 0, 0, 12, 0 },
	{ pclos, "V", 0, 0, 0, 0 },
	{ pdr, "Vfff", 0, 0, 12, 0 },
	{ pdr2, "Vff", 0, 0, 8, 0 },
	{ pdri, "Vlll", 0, 0, 12, 0 },
	{ pdr2i, "Vll", 0, 0, 8, 0 },
	{ pdrs, "Vsss", 0, 0, 12, 0 },
	{ pdr2s, "Vss", 0, 0, 8, 0 },
	{ polf2s, "Vla", 0, 0, 8, 0 },
	{ polfs, "Vla", 0, 0, 8, 0 },
	{ poly2s, "Vla", 0, 0, 8, 0 },
	{ polys, "Vla", 0, 0, 8, 0 },
	{ pmv, "Vfff", 0, 0, 12, 0 },
	{ pmv2, "Vff", 0, 0, 8, 0 },
	{ pmvi, "Vlll", 0, 0, 12, 0 },
	{ pmv2i, "Vll", 0, 0, 8, 0 },
	{ pmvs, "Vsss", 0, 0, 12, 0 },
	{ pmv2s, "Vss", 0, 0, 8, 0 },
	{ pnt2s, "Vss", 0, 0, 8, 0 },
	{ pnts, "Vsss", 0, 0, 12, 0 },
	{ popname, "V", 0, 0, 0, 0 },
	{ pushname, "Vs", 0, 0, 4, 0 },
	{ rdr, "Vfff", 0, 0, 12, 0 },
	{ rdr2, "Vff", 0, 0, 8, 0 },
	{ rdri, "Vlll", 0, 0, 12, 0 },
	{ rdr2i, "Vll", 0, 0, 8, 0 },
	{ rdrs, "Vsss", 0, 0, 12, 0 },
	{ rdr2s, "Vss", 0, 0, 8, 0 },
	{ rectcopy, "Vssssss", 0, 0, 24, 0 },
	{ rmv, "Vfff", 0, 0, 12, 0 },
	{ rmv2, "Vff", 0, 0, 8, 0 },
	{ rmvi, "Vlll", 0, 0, 12, 0 },
	{ rmv2i, "Vll", 0, 0, 8, 0 },
	{ rmvs, "Vsss", 0, 0, 12, 0 },
	{ rmv2s, "Vss", 0, 0, 8, 0 },
	{ rpdr, "Vfff", 0, 0, 12, 0 },
	{ rpdr2, "Vff", 0, 0, 8, 0 },
	{ rpdri, "Vlll", 0, 0, 12, 0 },
	{ rpdr2i, "Vll", 0, 0, 8, 0 },
	{ rpdrs, "Vsss", 0, 0, 12, 0 },
	{ rpdr2s, "Vss", 0, 0, 8, 0 },
	{ rpmv, "Vfff", 0, 0, 12, 0 },
	{ rpmv2, "Vff", 0, 0, 8, 0 },
	{ rpmvi, "Vlll", 0, 0, 12, 0 },
	{ rpmv2i, "Vll", 0, 0, 8, 0 },
	{ rpmvs, "Vsss", 0, 0, 12, 0 },
	{ rpmv2s, "Vss", 0, 0, 8, 0 },
	{ setdblights, "Vl", 0, 0, 4, 0 },
	{ setmonitor, "Vs", 0, 0, 4, 0 },
	{ setshade, "Vs", 0, 0, 4, 0 },
	{ shaderange, "Vss", 0, 0, 8, 0 },
	{ spclos, "V", 0, 0, 0, 0 },
	{ splf, "Vlaa", 0, 0, 12, 0 },
	{ splf2, "Vlaa", 0, 0, 12, 0 },
	{ splfi, "Vlaa", 0, 0, 12, 0 },
	{ splf2i, "Vlaa", 0, 0, 12, 0 },
	{ splfs, "Vlaa", 0, 0, 12, 0 },
	{ splf2s, "Vlaa", 0, 0, 12, 0 },
	{ xfpt, "Vfff", 0, 0, 12, 0 },
	{ xfpti, "Vlll", 0, 0, 12, 0 },
	{ xfpts, "Vsss", 0, 0, 12, 0 },
	{ xfpt2, "Vff", 0, 0, 8, 0 },
	{ xfpt2i, "Vll", 0, 0, 8, 0 },
	{ xfpt2s, "Vss", 0, 0, 8, 0 },
	{ xfpt4, "Vffff", 0, 0, 16, 0 },
	{ xfpt4i, "Vllll", 0, 0, 16, 0 },
	{ xfpt4s, "Vssss", 0, 0, 16, 0 },
	{ zbuffer, "Vo", 0, 0, 4, 0 },
	{ charst, "Val", 0, 0, 8, 0 },
	{ strwid, "Lal", 0, 0, 8, 1 },
	{ defpattern, "Vssa", 0, 0, 12, 0 },
	{ getpattern, "L", 0, 0, 0, 1 },
	{ setpattern, "Vs", 0, 0, 4, 0 },
	{ objinsert, "Vl", 0, 0, 4, 0 },
	{ objdelete, "Vll", 0, 0, 8, 0 },
	{ objreplace, "Vl", 0, 0, 4, 0 },
	{ zclear, "V", 0, 0, 0, 0 },
	{ curorigin, "Vsss", 0, 0, 12, 0 },
	{ pagewritemask, "Vs", 0, 0, 4, 0 },
	{ patchcurves, "Vll", 0, 0, 8, 0 },
	{ dbtext, "Va", 0, 0, 4, 0 },
	{ lastone, "V", 0, 0, 0, 0 },
	{ NULL, "V", 0, 0, 0, 0 }, 
};
