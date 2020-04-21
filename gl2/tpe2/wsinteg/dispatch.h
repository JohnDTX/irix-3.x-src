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
extern int xtpon();
extern int xtpoff();
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
extern int xgbegin();
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
extern int backface();
extern int cyclemap();
extern int minsize();
extern int maxsize();
extern int keepaspect();
extern int prefsize();
extern int stepunit();
extern int fudge();
extern int prefposition();
extern int xgetport();
extern int xgetpor();
extern int getsize();
extern int getorigin();
extern int screenspace();
extern int reshapeviewport();
extern int winopen();
extern int winope();
extern int winclose();
extern int winset();
extern int winget();
extern int winpush();
extern int winpop();
extern int winattach();
extern int winmove();
extern int winposition();
extern int wintitle();
extern int wintit();
extern int iftpsetup();
extern int iftpse();
extern int capture();
extern int captur();
extern int rcapture();
extern int rcaptu();
extern int rects();
extern int rectfs();
extern int foreground();
extern int devport();
extern int ret_getdev();

dispatchEntry dispatch[] = {
/*   0 */	{ xsetslowcom, "V", 0, 0, 0, 0 },
/*   1 */	{ xsetfastcom, "V", 0, 0, 0, 0 },
/*   2 */	{ gversion, "L", 0, 0, 0, 1 },
/*   3 */	{ bogus, "V", 0, 0, 0, 0 },
/*   4 */	{ pagecolor, "Vs", 0, 0, 4, 0 },
/*   5 */	{ textcolor, "Vs", 0, 0, 4, 0 },
/*   6 */	{ textport, "Vssss", 0, 0, 16, 0 },
/*   7 */	{ arc, "Vfffss", 0, 0, 20, 0 },
/*   8 */	{ arcf, "Vfffss", 0, 0, 20, 0 },
/*   9 */	{ arcfi, "Vlllss", 0, 0, 20, 0 },
/*  10 */	{ arci, "Vlllss", 0, 0, 20, 0 },
/*  11 */	{ attachcursor, "Vss", 0, 0, 8, 0 },
/*  12 */	{ backbuffer, "Vo", 0, 0, 4, 0 },
/*  13 */	{ bogus, "V", 0, 0, 0, 0 },
/*  14 */	{ bbox2, "Vssffff", 0, 0, 24, 0 },
/*  15 */	{ bbox2i, "Vssllll", 0, 0, 24, 0 },
/*  16 */	{ bogus, "V", 0, 0, 0, 0 },
/*  17 */	{ blink, "Vsssss", 0, 0, 20, 0 },
/*  18 */	{ callobj, "Vl", 0, 0, 4, 0 },
/*  19 */	{ charstr, "Vc", 0, 0, 4, 0 },
/*  20 */	{ circ, "Vfff", 0, 0, 12, 0 },
/*  21 */	{ circf, "Vfff", 0, 0, 12, 0 },
/*  22 */	{ circfi, "Vlll", 0, 0, 12, 0 },
/*  23 */	{ circi, "Vlll", 0, 0, 12, 0 },
/*  24 */	{ clear, "V", 0, 0, 0, 0 },
/*  25 */	{ clearhitcode, "V", 0, 0, 0, 0 },
/*  26 */	{ bogus, "V", 0, 0, 0, 0 },
/*  27 */	{ bogus, "V", 0, 0, 0, 0 },
/*  28 */	{ bogus, "V", 0, 0, 0, 0 },
/*  29 */	{ closeobj, "V", 0, 0, 0, 0 },
/*  30 */	{ cmov, "Vfff", 0, 0, 12, 0 },
/*  31 */	{ cmov2, "Vff", 0, 0, 8, 0 },
/*  32 */	{ cmov2i, "Vll", 0, 0, 8, 0 },
/*  33 */	{ cmovi, "Vlll", 0, 0, 12, 0 },
/*  34 */	{ color, "Vs", 0, 0, 4, 0 },
/*  35 */	{ cursoff, "V", 0, 0, 0, 0 },
/*  36 */	{ curson, "V", 0, 0, 0, 0 },
/*  37 */	{ bogus, "V", 0, 0, 0, 0 },
/*  38 */	{ curveit, "Vs", 0, 0, 4, 0 },
/*  39 */	{ defcursor, "Vsa", 0, 0, 8, 0 },
/*  40 */	{ deflinestyle, "Vss", 0, 0, 8, 0 },
/*  41 */	{ bogus, "V", 0, 0, 0, 0 },
/*  42 */	{ defrasterfont, "Vsssasa", 0, 0, 24, 0 },
/*  43 */	{ bogus, "V", 0, 0, 0, 0 },
/*  44 */	{ bogus, "V", 0, 0, 0, 0 },
/*  45 */	{ delobj, "Vl", 0, 0, 4, 0 },
/*  46 */	{ doublebuffer, "V", 0, 0, 0, 0 },
/*  47 */	{ draw, "Vfff", 0, 0, 12, 0 },
/*  48 */	{ draw2, "Vff", 0, 0, 8, 0 },
/*  49 */	{ draw2i, "Vll", 0, 0, 8, 0 },
/*  50 */	{ drawi, "Vlll", 0, 0, 12, 0 },
/*  51 */	{ editobj, "Vl", 0, 0, 4, 0 },
/*  52 */	{ ret_endpick, "V", 0, 0, 0, 0 },
/*  53 */	{ ret_endselect, "V", 0, 0, 0, 0 },
/*  54 */	{ finish, "V", 0, 0, 0, 0 },
/*  55 */	{ font, "Vs", 0, 0, 4, 0 },
/*  56 */	{ frontbuffer, "Vo", 0, 0, 4, 0 },
/*  57 */	{ gconfig, "V", 0, 0, 0, 0 },
/*  58 */	{ genobj, "L", 0, 0, 0, 1 },
/*  59 */	{ gentag, "L", 0, 0, 0, 1 },
/*  60 */	{ getbuffer, "L", 0, 0, 0, 1 },
/*  61 */	{ getbutton, "Os", 0, 0, 4, 1 },
/*  62 */	{ getcmmode, "O", 0, 0, 0, 1 },
/*  63 */	{ getcolor, "L", 0, 0, 0, 1 },
/*  64 */	{ getcursor, "VSSSO", 0, 0, 16, 1 },
/*  65 */	{ getdepth, "VSS", 0, 0, 8, 1 },
/*  66 */	{ getdisplaymode, "L", 0, 0, 0, 1 },
/*  67 */	{ getfont, "L", 0, 0, 0, 1 },
/*  68 */	{ getheight, "L", 0, 0, 0, 1 },
/*  69 */	{ gethitcode, "L", 0, 0, 0, 1 },
/*  70 */	{ getlsbackup, "O", 0, 0, 0, 1 },
/*  71 */	{ getlstyle, "L", 0, 0, 0, 1 },
/*  72 */	{ getlwidth, "L", 0, 0, 0, 1 },
/*  73 */	{ getmap, "L", 0, 0, 0, 1 },
/*  74 */	{ ret_getmatrix, "V", 0, 0, 0, 0 },
/*  75 */	{ bogus, "V", 0, 0, 0, 0 },
/*  76 */	{ getplanes, "L", 0, 0, 0, 1 },
/*  77 */	{ getresetls, "O", 0, 0, 0, 1 },
/*  78 */	{ getscrmask, "VSSSS", 0, 0, 16, 1 },
/*  79 */	{ bogus, "V", 0, 0, 0, 0 },
/*  80 */	{ getvaluator, "Ls", 0, 0, 4, 1 },
/*  81 */	{ getviewport, "VSSSS", 0, 0, 16, 1 },
/*  82 */	{ getwritemask, "L", 0, 0, 0, 1 },
/*  83 */	{ xginit, "V", 0, 0, 0, 0 },
/*  84 */	{ xgreset, "V", 0, 0, 0, 0 },
/*  85 */	{ gRGBcolor, "VSSS", 0, 0, 12, 1 },
/*  86 */	{ gRGBcursor, "VSSSSSSSO", 0, 0, 32, 1 },
/*  87 */	{ gRGBmask, "VSSS", 0, 0, 12, 1 },
/*  88 */	{ bogus, "V", 0, 0, 0, 0 },
/*  89 */	{ isobj, "Ol", 0, 0, 4, 1 },
/*  90 */	{ istag, "Ol", 0, 0, 4, 1 },
/*  91 */	{ bogus, "V", 0, 0, 0, 0 },
/*  92 */	{ linewidth, "Vs", 0, 0, 4, 0 },
/*  93 */	{ loadmatrix, "Va", 0, 0, 4, 0 },
/*  94 */	{ lookat, "Vffffffs", 0, 0, 28, 0 },
/*  95 */	{ lsbackup, "Vo", 0, 0, 4, 0 },
/*  96 */	{ makeobj, "Vl", 0, 0, 4, 0 },
/*  97 */	{ maketag, "Vl", 0, 0, 4, 0 },
/*  98 */	{ mapcolor, "Vssss", 0, 0, 16, 0 },
/*  99 */	{ mapw, "VlssFFFFFF", 0, 0, 36, 1 },
/* 100 */	{ mapw2, "VlssFF", 0, 0, 20, 1 },
/* 101 */	{ bogus, "V", 0, 0, 0, 0 },
/* 102 */	{ move, "Vfff", 0, 0, 12, 0 },
/* 103 */	{ move2, "Vff", 0, 0, 8, 0 },
/* 104 */	{ move2i, "Vll", 0, 0, 8, 0 },
/* 105 */	{ movei, "Vlll", 0, 0, 12, 0 },
/* 106 */	{ multimap, "V", 0, 0, 0, 0 },
/* 107 */	{ multmatrix, "Va", 0, 0, 4, 0 },
/* 108 */	{ noise, "Vss", 0, 0, 8, 0 },
/* 109 */	{ bogus, "V", 0, 0, 0, 0 },
/* 110 */	{ bogus, "V", 0, 0, 0, 0 },
/* 111 */	{ onemap, "V", 0, 0, 0, 0 },
/* 112 */	{ ortho, "Vffffff", 0, 0, 24, 0 },
/* 113 */	{ ortho2, "Vffff", 0, 0, 16, 0 },
/* 114 */	{ perspective, "Vsfff", 0, 0, 16, 0 },
/* 115 */	{ spl_pick, "V", 0, 0, 0, 0 },
/* 116 */	{ picksize, "Vss", 0, 0, 8, 0 },
/* 117 */	{ pnt, "Vfff", 0, 0, 12, 0 },
/* 118 */	{ pnt2, "Vff", 0, 0, 8, 0 },
/* 119 */	{ pnt2i, "Vll", 0, 0, 8, 0 },
/* 120 */	{ pnti, "Vlll", 0, 0, 12, 0 },
/* 121 */	{ polarview, "Vfsss", 0, 0, 16, 0 },
/* 122 */	{ polf, "Vla", 0, 0, 8, 0 },
/* 123 */	{ polf2, "Vla", 0, 0, 8, 0 },
/* 124 */	{ polf2i, "Vla", 0, 0, 8, 0 },
/* 125 */	{ polfi, "Vla", 0, 0, 8, 0 },
/* 126 */	{ poly, "Vla", 0, 0, 8, 0 },
/* 127 */	{ poly2, "Vla", 0, 0, 8, 0 },
/* 128 */	{ poly2i, "Vla", 0, 0, 8, 0 },
/* 129 */	{ polyi, "Vla", 0, 0, 8, 0 },
/* 130 */	{ popattributes, "V", 0, 0, 0, 0 },
/* 131 */	{ popmatrix, "V", 0, 0, 0, 0 },
/* 132 */	{ popviewport, "V", 0, 0, 0, 0 },
/* 133 */	{ pushattributes, "V", 0, 0, 0, 0 },
/* 134 */	{ pushmatrix, "V", 0, 0, 0, 0 },
/* 135 */	{ pushviewport, "V", 0, 0, 0, 0 },
/* 136 */	{ bogus, "V", 0, 0, 0, 0 },
/* 137 */	{ qenter, "Vss", 0, 0, 8, 0 },
/* 138 */	{ bogus, "V", 0, 0, 0, 0 },
/* 139 */	{ qread, "LS", 0, 0, 4, 1 },
/* 140 */	{ qreset, "V", 0, 0, 0, 0 },
/* 141 */	{ qtest, "L", 0, 0, 0, 1 },
/* 142 */	{ bogus, "V", 0, 0, 0, 0 },
/* 143 */	{ ret_readpixels, "V", 0, 0, 0, 0 },
/* 144 */	{ ret_readRGB, "V", 0, 0, 0, 0 },
/* 145 */	{ rect, "Vffff", 0, 0, 16, 0 },
/* 146 */	{ rectf, "Vffff", 0, 0, 16, 0 },
/* 147 */	{ rectfi, "Vllll", 0, 0, 16, 0 },
/* 148 */	{ recti, "Vllll", 0, 0, 16, 0 },
/* 149 */	{ bogus, "V", 0, 0, 0, 0 },
/* 150 */	{ resetls, "Vo", 0, 0, 4, 0 },
/* 151 */	{ RGBcolor, "Vsss", 0, 0, 12, 0 },
/* 152 */	{ RGBcursor, "Vsssssss", 0, 0, 28, 0 },
/* 153 */	{ RGBmode, "V", 0, 0, 0, 0 },
/* 154 */	{ RGBwritemask, "Vsss", 0, 0, 12, 0 },
/* 155 */	{ rotate, "Vsb", 0, 0, 8, 0 },
/* 156 */	{ scale, "Vfff", 0, 0, 12, 0 },
/* 157 */	{ bogus, "V", 0, 0, 0, 0 },
/* 158 */	{ scrmask, "Vssss", 0, 0, 16, 0 },
/* 159 */	{ spl_select, "V", 0, 0, 0, 0 },
/* 160 */	{ setbutton, "Vso", 0, 0, 8, 0 },
/* 161 */	{ setcursor, "Vsss", 0, 0, 12, 0 },
/* 162 */	{ setdepth, "Vss", 0, 0, 8, 0 },
/* 163 */	{ setlinestyle, "Vs", 0, 0, 4, 0 },
/* 164 */	{ setmap, "Vs", 0, 0, 4, 0 },
/* 165 */	{ bogus, "V", 0, 0, 0, 0 },
/* 166 */	{ bogus, "V", 0, 0, 0, 0 },
/* 167 */	{ setvaluator, "Vssss", 0, 0, 16, 0 },
/* 168 */	{ singlebuffer, "V", 0, 0, 0, 0 },
/* 169 */	{ strwidth, "Lc", 0, 0, 4, 1 },
/* 170 */	{ swapbuffers, "V", 0, 0, 0, 0 },
/* 171 */	{ swapinterval, "Vs", 0, 0, 4, 0 },
/* 172 */	{ gsync, "V", 0, 0, 0, 0 },
/* 173 */	{ tie, "Vsss", 0, 0, 12, 0 },
/* 174 */	{ bogus, "V", 0, 0, 0, 0 },
/* 175 */	{ translate, "Vfff", 0, 0, 12, 0 },
/* 176 */	{ bogus, "V", 0, 0, 0, 0 },
/* 177 */	{ bogus, "V", 0, 0, 0, 0 },
/* 178 */	{ bogus, "V", 0, 0, 0, 0 },
/* 179 */	{ viewport, "Vssss", 0, 0, 16, 0 },
/* 180 */	{ window, "Vffffff", 0, 0, 24, 0 },
/* 181 */	{ writemask, "Vs", 0, 0, 4, 0 },
/* 182 */	{ writepixels, "Vsa", 0, 0, 8, 0 },
/* 183 */	{ writeRGB, "Vsaaa", 0, 0, 16, 0 },
/* 184 */	{ xtpon, "V", 0, 0, 0, 0 },
/* 185 */	{ xtpoff, "V", 0, 0, 0, 0 },
/* 186 */	{ bogus, "V", 0, 0, 0, 0 },
/* 187 */	{ textwritemask, "Vs", 0, 0, 4, 0 },
/* 188 */	{ xgexit, "V", 0, 0, 0, 0 },
/* 189 */	{ clkon, "V", 0, 0, 0, 0 },
/* 190 */	{ clkoff, "V", 0, 0, 0, 0 },
/* 191 */	{ lampon, "Vb", 0, 0, 4, 0 },
/* 192 */	{ lampoff, "Vb", 0, 0, 4, 0 },
/* 193 */	{ setbell, "Vb", 0, 0, 4, 0 },
/* 194 */	{ ringbell, "V", 0, 0, 0, 0 },
/* 195 */	{ tadelay, "Vs", 0, 0, 4, 0 },
/* 196 */	{ arcfs, "Vsssss", 0, 0, 20, 0 },
/* 197 */	{ arcs, "Vsssss", 0, 0, 20, 0 },
/* 198 */	{ bogus, "V", 0, 0, 0, 0 },
/* 199 */	{ bbox2s, "Vssssss", 0, 0, 24, 0 },
/* 200 */	{ blankscreen, "Vo", 0, 0, 4, 0 },
/* 201 */	{ bogus, "V", 0, 0, 0, 0 },
/* 202 */	{ ret_blkqread, "V", 0, 0, 0, 0 },
/* 203 */	{ getmcolor, "VsSSS", 0, 0, 16, 1 },
/* 204 */	{ bogus, "V", 0, 0, 0, 0 },
/* 205 */	{ chunksize, "Vl", 0, 0, 4, 0 },
/* 206 */	{ circfs, "Vsss", 0, 0, 12, 0 },
/* 207 */	{ circs, "Vsss", 0, 0, 12, 0 },
/* 208 */	{ cmov2s, "Vss", 0, 0, 8, 0 },
/* 209 */	{ cmovs, "Vsss", 0, 0, 12, 0 },
/* 210 */	{ compactify, "Vl", 0, 0, 4, 0 },
/* 211 */	{ qdevice, "Vs", 0, 0, 4, 0 },
/* 212 */	{ unqdevice, "Vs", 0, 0, 4, 0 },
/* 213 */	{ curvebasis, "Vs", 0, 0, 4, 0 },
/* 214 */	{ curveprecision, "Vs", 0, 0, 4, 0 },
/* 215 */	{ crv, "Va", 0, 0, 4, 0 },
/* 216 */	{ gettp, "VSSSS", 0, 0, 16, 1 },
/* 217 */	{ xgbegin, "V", 0, 0, 0, 0 },
/* 218 */	{ textinit, "V", 0, 0, 0, 0 },
/* 219 */	{ crvn, "Vla", 0, 0, 8, 0 },
/* 220 */	{ defbasis, "Vsa", 0, 0, 8, 0 },
/* 221 */	{ deltag, "Vl", 0, 0, 4, 0 },
/* 222 */	{ depthcue, "Vo", 0, 0, 4, 0 },
/* 223 */	{ draw2s, "Vss", 0, 0, 8, 0 },
/* 224 */	{ draws, "Vsss", 0, 0, 12, 0 },
/* 225 */	{ ret_endfeedback, "V", 0, 0, 0, 0 },
/* 226 */	{ spl_feedback, "V", 0, 0, 0, 0 },
/* 227 */	{ getcpos, "VSS", 0, 0, 8, 1 },
/* 228 */	{ getdcm, "O", 0, 0, 0, 1 },
/* 229 */	{ getgpos, "VFFFF", 0, 0, 16, 1 },
/* 230 */	{ getlsrepeat, "L", 0, 0, 0, 1 },
/* 231 */	{ getmem, "L", 0, 0, 0, 1 },
/* 232 */	{ getmonitor, "L", 0, 0, 0, 1 },
/* 233 */	{ getopenobj, "L", 0, 0, 0, 1 },
/* 234 */	{ getzbuffer, "O", 0, 0, 0, 1 },
/* 235 */	{ gewrite, "Val", 0, 0, 8, 0 },
/* 236 */	{ initnames, "V", 0, 0, 0, 0 },
/* 237 */	{ loadname, "Vs", 0, 0, 4, 0 },
/* 238 */	{ lsrepeat, "Vl", 0, 0, 4, 0 },
/* 239 */	{ move2s, "Vss", 0, 0, 8, 0 },
/* 240 */	{ moves, "Vsss", 0, 0, 12, 0 },
/* 241 */	{ newtag, "Vlll", 0, 0, 12, 0 },
/* 242 */	{ passthrough, "Vs", 0, 0, 4, 0 },
/* 243 */	{ patchbasis, "Vll", 0, 0, 8, 0 },
/* 244 */	{ patchprecision, "Vll", 0, 0, 8, 0 },
/* 245 */	{ patch, "Vaaa", 0, 0, 12, 0 },
/* 246 */	{ pclos, "V", 0, 0, 0, 0 },
/* 247 */	{ pdr, "Vfff", 0, 0, 12, 0 },
/* 248 */	{ pdr2, "Vff", 0, 0, 8, 0 },
/* 249 */	{ pdri, "Vlll", 0, 0, 12, 0 },
/* 250 */	{ pdr2i, "Vll", 0, 0, 8, 0 },
/* 251 */	{ pdrs, "Vsss", 0, 0, 12, 0 },
/* 252 */	{ pdr2s, "Vss", 0, 0, 8, 0 },
/* 253 */	{ polf2s, "Vla", 0, 0, 8, 0 },
/* 254 */	{ polfs, "Vla", 0, 0, 8, 0 },
/* 255 */	{ poly2s, "Vla", 0, 0, 8, 0 },
/* 256 */	{ polys, "Vla", 0, 0, 8, 0 },
/* 257 */	{ pmv, "Vfff", 0, 0, 12, 0 },
/* 258 */	{ pmv2, "Vff", 0, 0, 8, 0 },
/* 259 */	{ pmvi, "Vlll", 0, 0, 12, 0 },
/* 260 */	{ pmv2i, "Vll", 0, 0, 8, 0 },
/* 261 */	{ pmvs, "Vsss", 0, 0, 12, 0 },
/* 262 */	{ pmv2s, "Vss", 0, 0, 8, 0 },
/* 263 */	{ pnt2s, "Vss", 0, 0, 8, 0 },
/* 264 */	{ pnts, "Vsss", 0, 0, 12, 0 },
/* 265 */	{ popname, "V", 0, 0, 0, 0 },
/* 266 */	{ pushname, "Vs", 0, 0, 4, 0 },
/* 267 */	{ rdr, "Vfff", 0, 0, 12, 0 },
/* 268 */	{ rdr2, "Vff", 0, 0, 8, 0 },
/* 269 */	{ rdri, "Vlll", 0, 0, 12, 0 },
/* 270 */	{ rdr2i, "Vll", 0, 0, 8, 0 },
/* 271 */	{ rdrs, "Vsss", 0, 0, 12, 0 },
/* 272 */	{ rdr2s, "Vss", 0, 0, 8, 0 },
/* 273 */	{ rectcopy, "Vssssss", 0, 0, 24, 0 },
/* 274 */	{ rmv, "Vfff", 0, 0, 12, 0 },
/* 275 */	{ rmv2, "Vff", 0, 0, 8, 0 },
/* 276 */	{ rmvi, "Vlll", 0, 0, 12, 0 },
/* 277 */	{ rmv2i, "Vll", 0, 0, 8, 0 },
/* 278 */	{ rmvs, "Vsss", 0, 0, 12, 0 },
/* 279 */	{ rmv2s, "Vss", 0, 0, 8, 0 },
/* 280 */	{ rpdr, "Vfff", 0, 0, 12, 0 },
/* 281 */	{ rpdr2, "Vff", 0, 0, 8, 0 },
/* 282 */	{ rpdri, "Vlll", 0, 0, 12, 0 },
/* 283 */	{ rpdr2i, "Vll", 0, 0, 8, 0 },
/* 284 */	{ rpdrs, "Vsss", 0, 0, 12, 0 },
/* 285 */	{ rpdr2s, "Vss", 0, 0, 8, 0 },
/* 286 */	{ rpmv, "Vfff", 0, 0, 12, 0 },
/* 287 */	{ rpmv2, "Vff", 0, 0, 8, 0 },
/* 288 */	{ rpmvi, "Vlll", 0, 0, 12, 0 },
/* 289 */	{ rpmv2i, "Vll", 0, 0, 8, 0 },
/* 290 */	{ rpmvs, "Vsss", 0, 0, 12, 0 },
/* 291 */	{ rpmv2s, "Vss", 0, 0, 8, 0 },
/* 292 */	{ setdblights, "Vl", 0, 0, 4, 0 },
/* 293 */	{ setmonitor, "Vs", 0, 0, 4, 0 },
/* 294 */	{ setshade, "Vs", 0, 0, 4, 0 },
/* 295 */	{ shaderange, "Vssss", 0, 0, 16, 0 },
/* 296 */	{ spclos, "V", 0, 0, 0, 0 },
/* 297 */	{ splf, "Vlaa", 0, 0, 12, 0 },
/* 298 */	{ splf2, "Vlaa", 0, 0, 12, 0 },
/* 299 */	{ splfi, "Vlaa", 0, 0, 12, 0 },
/* 300 */	{ splf2i, "Vlaa", 0, 0, 12, 0 },
/* 301 */	{ splfs, "Vlaa", 0, 0, 12, 0 },
/* 302 */	{ splf2s, "Vlaa", 0, 0, 12, 0 },
/* 303 */	{ xfpt, "Vfff", 0, 0, 12, 0 },
/* 304 */	{ xfpti, "Vlll", 0, 0, 12, 0 },
/* 305 */	{ xfpts, "Vsss", 0, 0, 12, 0 },
/* 306 */	{ xfpt2, "Vff", 0, 0, 8, 0 },
/* 307 */	{ xfpt2i, "Vll", 0, 0, 8, 0 },
/* 308 */	{ xfpt2s, "Vss", 0, 0, 8, 0 },
/* 309 */	{ xfpt4, "Vffff", 0, 0, 16, 0 },
/* 310 */	{ xfpt4i, "Vllll", 0, 0, 16, 0 },
/* 311 */	{ xfpt4s, "Vssss", 0, 0, 16, 0 },
/* 312 */	{ zbuffer, "Vo", 0, 0, 4, 0 },
/* 313 */	{ charst, "Val", 0, 0, 8, 0 },
/* 314 */	{ strwid, "Lal", 0, 0, 8, 1 },
/* 315 */	{ defpattern, "Vssa", 0, 0, 12, 0 },
/* 316 */	{ getpattern, "L", 0, 0, 0, 1 },
/* 317 */	{ setpattern, "Vs", 0, 0, 4, 0 },
/* 318 */	{ objinsert, "Vl", 0, 0, 4, 0 },
/* 319 */	{ objdelete, "Vll", 0, 0, 8, 0 },
/* 320 */	{ objreplace, "Vl", 0, 0, 4, 0 },
/* 321 */	{ zclear, "V", 0, 0, 0, 0 },
/* 322 */	{ curorigin, "Vsss", 0, 0, 12, 0 },
/* 323 */	{ pagewritemask, "Vs", 0, 0, 4, 0 },
/* 324 */	{ patchcurves, "Vll", 0, 0, 8, 0 },
/* 325 */	{ dbtext, "Va", 0, 0, 4, 0 },
/* 326 */	{ backface, "Vo", 0, 0, 4, 0 },
/* 327 */	{ cyclemap, "Vsss", 0, 0, 12, 0 },
/* 328 */	{ minsize, "Vll", 0, 0, 8, 0 },
/* 329 */	{ maxsize, "Vll", 0, 0, 8, 0 },
/* 330 */	{ keepaspect, "Vll", 0, 0, 8, 0 },
/* 331 */	{ prefsize, "Vll", 0, 0, 8, 0 },
/* 332 */	{ stepunit, "Vll", 0, 0, 8, 0 },
/* 333 */	{ fudge, "Vll", 0, 0, 8, 0 },
/* 334 */	{ prefposition, "Vllll", 0, 0, 16, 0 },
/* 335 */	{ xgetport, "Vc", 0, 0, 4, 0 },
/* 336 */	{ xgetpor, "Val", 0, 0, 8, 0 },
/* 337 */	{ getsize, "VLL", 0, 0, 8, 1 },
/* 338 */	{ getorigin, "VLL", 0, 0, 8, 1 },
/* 339 */	{ screenspace, "V", 0, 0, 0, 0 },
/* 340 */	{ reshapeviewport, "V", 0, 0, 0, 0 },
/* 341 */	{ winopen, "Lc", 0, 0, 4, 1 },
/* 342 */	{ winope, "Lal", 0, 0, 8, 1 },
/* 343 */	{ winclose, "Vl", 0, 0, 4, 0 },
/* 344 */	{ winset, "Ll", 0, 0, 4, 1 },
/* 345 */	{ winget, "L", 0, 0, 0, 1 },
/* 346 */	{ winpush, "V", 0, 0, 0, 0 },
/* 347 */	{ winpop, "V", 0, 0, 0, 0 },
/* 348 */	{ winattach, "L", 0, 0, 0, 1 },
/* 349 */	{ winmove, "VLL", 0, 0, 8, 1 },
/* 350 */	{ winposition, "VLLLL", 0, 0, 16, 1 },
/* 351 */	{ wintitle, "Vc", 0, 0, 4, 0 },
/* 352 */	{ wintit, "Val", 0, 0, 8, 0 },
/* 353 */	{ iftpsetup, "Vcc", 0, 0, 8, 0 },
/* 354 */	{ iftpse, "Valb", 0, 0, 12, 0 },
/* 355 */	{ capture, "Vca", 0, 0, 8, 0 },
/* 356 */	{ captur, "Vala", 0, 0, 12, 0 },
/* 357 */	{ rcapture, "Vcasssslal", 0, 0, 36, 0 },
/* 358 */	{ rcaptu, "Valasssslal", 0, 0, 40, 0 },
/* 359 */	{ rects, "Vssss", 0, 0, 16, 0 },
/* 360 */	{ rectfs, "Vssss", 0, 0, 16, 0 },
/* 361 */	{ foreground, "V", 0, 0, 0, 0 },
/* 362 */	{ devport, "Vll", 0, 0, 8, 0 },
/* 363 */	{ ret_getdev, "V", 0, 0, 0, 0 },
/* 364 */	{ bogus, "V", 0, 0, 0, 0 },
/* 365 */	{ bogus, "V", 0, 0, 0, 0 },
/* 366 */	{ bogus, "V", 0, 0, 0, 0 },
/* 367 */	{ bogus, "V", 0, 0, 0, 0 },
/* 368 */	{ bogus, "V", 0, 0, 0, 0 },
/* 369 */	{ bogus, "V", 0, 0, 0, 0 },
/* 370 */	{ bogus, "V", 0, 0, 0, 0 },
/* 371 */	{ bogus, "V", 0, 0, 0, 0 },
/* 372 */	{ bogus, "V", 0, 0, 0, 0 },
/* 373 */	{ bogus, "V", 0, 0, 0, 0 },
/* 374 */	{ bogus, "V", 0, 0, 0, 0 },
/* 375 */	{ bogus, "V", 0, 0, 0, 0 },
/* 376 */	{ bogus, "V", 0, 0, 0, 0 },
/* 377 */	{ bogus, "V", 0, 0, 0, 0 },
/* 378 */	{ bogus, "V", 0, 0, 0, 0 },
/* 379 */	{ bogus, "V", 0, 0, 0, 0 },
/* 380 */	{ bogus, "V", 0, 0, 0, 0 },
/* 381 */	{ bogus, "V", 0, 0, 0, 0 },
/* 382 */	{ bogus, "V", 0, 0, 0, 0 },
/* 383 */	{ bogus, "V", 0, 0, 0, 0 },
/* 384 */	{ bogus, "V", 0, 0, 0, 0 },
/* 385 */	{ bogus, "V", 0, 0, 0, 0 },
/* 386 */	{ bogus, "V", 0, 0, 0, 0 },
/* 387 */	{ bogus, "V", 0, 0, 0, 0 },
/* 388 */	{ bogus, "V", 0, 0, 0, 0 },
/* 389 */	{ bogus, "V", 0, 0, 0, 0 },
/* 390 */	{ bogus, "V", 0, 0, 0, 0 },
/* 391 */	{ bogus, "V", 0, 0, 0, 0 },
/* 392 */	{ bogus, "V", 0, 0, 0, 0 },
/* 393 */	{ bogus, "V", 0, 0, 0, 0 },
/* 394 */	{ bogus, "V", 0, 0, 0, 0 },
/* 395 */	{ bogus, "V", 0, 0, 0, 0 },
/* 396 */	{ bogus, "V", 0, 0, 0, 0 },
/* 397 */	{ bogus, "V", 0, 0, 0, 0 },
/* 398 */	{ bogus, "V", 0, 0, 0, 0 },
/* 399 */	{ bogus, "V", 0, 0, 0, 0 },
/* xxx */	{ NULL, "V", 0, 0, 0, 0 }, 
};
static char *cmdnametab[] = {
    "xsetslowcom",
    "xsetfastcom",
    "gversion",
    "bogus",
    "pagecolor",
    "textcolor",
    "textport",
    "arc",
    "arcf",
    "arcfi",
    "arci",
    "attachcursor",
    "backbuffer",
    "bogus",
    "bbox2",
    "bbox2i",
    "bogus",
    "blink",
    "callobj",
    "charstr",
    "circ",
    "circf",
    "circfi",
    "circi",
    "clear",
    "clearhitcode",
    "bogus",
    "bogus",
    "bogus",
    "closeobj",
    "cmov",
    "cmov2",
    "cmov2i",
    "cmovi",
    "color",
    "cursoff",
    "curson",
    "bogus",
    "curveit",
    "defcursor",
    "deflinestyle",
    "bogus",
    "defrasterfont",
    "bogus",
    "bogus",
    "delobj",
    "doublebuffer",
    "draw",
    "draw2",
    "draw2i",
    "drawi",
    "editobj",
    "ret_endpick",
    "ret_endselect",
    "finish",
    "font",
    "frontbuffer",
    "gconfig",
    "genobj",
    "gentag",
    "getbuffer",
    "getbutton",
    "getcmmode",
    "getcolor",
    "getcursor",
    "getdepth",
    "getdisplaymode",
    "getfont",
    "getheight",
    "gethitcode",
    "getlsbackup",
    "getlstyle",
    "getlwidth",
    "getmap",
    "ret_getmatrix",
    "bogus",
    "getplanes",
    "getresetls",
    "getscrmask",
    "bogus",
    "getvaluator",
    "getviewport",
    "getwritemask",
    "xginit",
    "xgreset",
    "gRGBcolor",
    "gRGBcursor",
    "gRGBmask",
    "bogus",
    "isobj",
    "istag",
    "bogus",
    "linewidth",
    "loadmatrix",
    "lookat",
    "lsbackup",
    "makeobj",
    "maketag",
    "mapcolor",
    "mapw",
    "mapw2",
    "bogus",
    "move",
    "move2",
    "move2i",
    "movei",
    "multimap",
    "multmatrix",
    "noise",
    "bogus",
    "bogus",
    "onemap",
    "ortho",
    "ortho2",
    "perspective",
    "spl_pick",
    "picksize",
    "pnt",
    "pnt2",
    "pnt2i",
    "pnti",
    "polarview",
    "polf",
    "polf2",
    "polf2i",
    "polfi",
    "poly",
    "poly2",
    "poly2i",
    "polyi",
    "popattributes",
    "popmatrix",
    "popviewport",
    "pushattributes",
    "pushmatrix",
    "pushviewport",
    "bogus",
    "qenter",
    "bogus",
    "qread",
    "qreset",
    "qtest",
    "bogus",
    "ret_readpixels",
    "ret_readRGB",
    "rect",
    "rectf",
    "rectfi",
    "recti",
    "bogus",
    "resetls",
    "RGBcolor",
    "RGBcursor",
    "RGBmode",
    "RGBwritemask",
    "rotate",
    "scale",
    "bogus",
    "scrmask",
    "spl_select",
    "setbutton",
    "setcursor",
    "setdepth",
    "setlinestyle",
    "setmap",
    "bogus",
    "bogus",
    "setvaluator",
    "singlebuffer",
    "strwidth",
    "swapbuffers",
    "swapinterval",
    "gsync",
    "tie",
    "bogus",
    "translate",
    "bogus",
    "bogus",
    "bogus",
    "viewport",
    "window",
    "writemask",
    "writepixels",
    "writeRGB",
    "xtpon",
    "xtpoff",
    "bogus",
    "textwritemask",
    "xgexit",
    "clkon",
    "clkoff",
    "lampon",
    "lampoff",
    "setbell",
    "ringbell",
    "tadelay",
    "arcfs",
    "arcs",
    "bogus",
    "bbox2s",
    "blankscreen",
    "bogus",
    "ret_blkqread",
    "getmcolor",
    "bogus",
    "chunksize",
    "circfs",
    "circs",
    "cmov2s",
    "cmovs",
    "compactify",
    "qdevice",
    "unqdevice",
    "curvebasis",
    "curveprecision",
    "crv",
    "gettp",
    "xgbegin",
    "textinit",
    "crvn",
    "defbasis",
    "deltag",
    "depthcue",
    "draw2s",
    "draws",
    "ret_endfeedback",
    "spl_feedback",
    "getcpos",
    "getdcm",
    "getgpos",
    "getlsrepeat",
    "getmem",
    "getmonitor",
    "getopenobj",
    "getzbuffer",
    "gewrite",
    "initnames",
    "loadname",
    "lsrepeat",
    "move2s",
    "moves",
    "newtag",
    "passthrough",
    "patchbasis",
    "patchprecision",
    "patch",
    "pclos",
    "pdr",
    "pdr2",
    "pdri",
    "pdr2i",
    "pdrs",
    "pdr2s",
    "polf2s",
    "polfs",
    "poly2s",
    "polys",
    "pmv",
    "pmv2",
    "pmvi",
    "pmv2i",
    "pmvs",
    "pmv2s",
    "pnt2s",
    "pnts",
    "popname",
    "pushname",
    "rdr",
    "rdr2",
    "rdri",
    "rdr2i",
    "rdrs",
    "rdr2s",
    "rectcopy",
    "rmv",
    "rmv2",
    "rmvi",
    "rmv2i",
    "rmvs",
    "rmv2s",
    "rpdr",
    "rpdr2",
    "rpdri",
    "rpdr2i",
    "rpdrs",
    "rpdr2s",
    "rpmv",
    "rpmv2",
    "rpmvi",
    "rpmv2i",
    "rpmvs",
    "rpmv2s",
    "setdblights",
    "setmonitor",
    "setshade",
    "shaderange",
    "spclos",
    "splf",
    "splf2",
    "splfi",
    "splf2i",
    "splfs",
    "splf2s",
    "xfpt",
    "xfpti",
    "xfpts",
    "xfpt2",
    "xfpt2i",
    "xfpt2s",
    "xfpt4",
    "xfpt4i",
    "xfpt4s",
    "zbuffer",
    "charst",
    "strwid",
    "defpattern",
    "getpattern",
    "setpattern",
    "objinsert",
    "objdelete",
    "objreplace",
    "zclear",
    "curorigin",
    "pagewritemask",
    "patchcurves",
    "dbtext",
    "backface",
    "cyclemap",
    "minsize",
    "maxsize",
    "keepaspect",
    "prefsize",
    "stepunit",
    "fudge",
    "prefposition",
    "xgetport",
    "xgetpor",
    "getsize",
    "getorigin",
    "screenspace",
    "reshapeviewport",
    "winopen",
    "winope",
    "winclose",
    "winset",
    "winget",
    "winpush",
    "winpop",
    "winattach",
    "winmove",
    "winposition",
    "wintitle",
    "wintit",
    "iftpsetup",
    "iftpse",
    "capture",
    "captur",
    "rcapture",
    "rcaptu",
    "rects",
    "rectfs",
    "foreground",
    "devport",
    "ret_getdev",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    "bogus",
    NULL,
    "",
};
