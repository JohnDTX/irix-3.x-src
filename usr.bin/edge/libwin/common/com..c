#line 1 "com.c"

/* <<cfront 05/20/86>> */
/* < com.c */

#line 8 "/usr/include/CC/stdio.h"
int *_new ();

#line 8 "/usr/include/CC/stdio.h"
int _delete ();

#line 8 "/usr/include/CC/stdio.h"
int *_vec_new ();

#line 8 "/usr/include/CC/stdio.h"
int _vec_delete ();

#line 8 "/usr/include/CC/stdio.h"
typedef char *va_list ;

#line 12 "/usr/include/CC/stdio.h"
struct _iobuf { /* sizeof = 14 */
int __iobuf__cnt ;
char *__iobuf__ptr ;
char *__iobuf__base ;

#line 18 "/usr/include/CC/stdio.h"
char __iobuf__flag ;
char __iobuf__file ;
} ;

#line 20 "/usr/include/CC/stdio.h"
extern struct _iobuf _iob [20];

#line 36 "/usr/include/CC/stdio.h"
extern int _flsbuf ();
extern int _filbuf ();

#line 50 "/usr/include/CC/stdio.h"
extern struct _iobuf *fopen ();
extern struct _iobuf *fdopen ();
extern struct _iobuf *freopen ();
extern long ftell ();
extern char *fgets ();

#line 61 "/usr/include/CC/stdio.h"
extern char *gets ();
extern int puts ();
extern int fputs ();
extern int printf ();
extern int fprintf ();
extern int sprintf ();
extern int scanf ();
extern int fscanf ();
extern int sscanf ();
extern int fread ();
extern int fwrite ();
extern int fclose ();
extern int fflush ();
extern int clearerr ();
extern int fseek ();
extern int rewind ();
extern int getw ();
extern int fgetc ();
extern struct _iobuf *popen ();
extern int pclose ();
extern int putw ();
extern int fputc ();
extern int setbuf ();
extern int ungetc ();

#line 86 "/usr/include/CC/stdio.h"
extern int exit ();
extern int abort ();

#line 89 "/usr/include/CC/stdio.h"
extern int atoi ();
extern double atof ();
extern long atol ();

#line 96 "/usr/include/CC/stdio.h"
extern struct _iobuf *tmpfile ();
extern char *ctermid ();
extern char *cuserid ();
extern char *tempnam ();
extern char *tmpnam ();
extern int vprintf ();
extern int vfprintf ();
extern int vsprintf ();
extern int setvbuf ();

#line 106 "/usr/include/CC/stdio.h"
extern int perror ();

#line 108 "/usr/include/CC/stdio.h"
extern int errno ;
extern char *sys_errlist [];
extern int sys_nerr ;
extern unsigned char *_bufendtab [];

#line 18 "/usr/include/CC/stream.h"
/* enum state_value */

#line 19 "/usr/include/CC/stream.h"
/* enum open_mode */

#line 21 "/usr/include/CC/stream.h"
struct streambuf { /* sizeof = 26 */

#line 23 "/usr/include/CC/stream.h"
char *_streambuf_base ;
char *_streambuf_pptr ;
char *_streambuf_gptr ;
char *_streambuf_eptr ;
char _streambuf_alloc ;
struct _iobuf *_streambuf_fp ;

#line 90 "/usr/include/CC/stream.h"
int (**_streambuf__vptr )();
} ;
int _streambuf_overflow ();
int _streambuf_underflow ();
static int (*streambuf__vtbl[])() = {
(int(*)()) _streambuf_overflow , 
(int(*)()) _streambuf_underflow , 0};

#line 84 "/usr/include/CC/stream.h"
int _streambuf_doallocate ();

#line 87 "/usr/include/CC/stream.h"
	/* overload _ctor: */
;
;

#line 92 "/usr/include/CC/stream.h"
extern int close ();

#line 94 "/usr/include/CC/stream.h"
struct filebuf { /* sizeof = 32 */

#line 23 "/usr/include/CC/stream.h"
char *_streambuf_base ;
char *_streambuf_pptr ;
char *_streambuf_gptr ;
char *_streambuf_eptr ;
char _streambuf_alloc ;
struct _iobuf *_streambuf_fp ;

#line 90 "/usr/include/CC/stream.h"
int (**_streambuf__vptr )();

#line 96 "/usr/include/CC/stream.h"
int _filebuf_fd ;
char _filebuf_opened ;
} ;
int _filebuf_overflow ();
int _filebuf_underflow ();
static int (*filebuf__vtbl[])() = {
(int(*)()) _filebuf_overflow , 
(int(*)()) _filebuf_underflow , 0};

#line 107 "/usr/include/CC/stream.h"
struct filebuf *_filebuf_open ();

#line 112 "/usr/include/CC/stream.h"
	/* overload _ctor: */
;
;
;
;

#line 119 "/usr/include/CC/stream.h"
struct circbuf { /* sizeof = 26 */

#line 23 "/usr/include/CC/stream.h"
char *_streambuf_base ;
char *_streambuf_pptr ;
char *_streambuf_gptr ;
char *_streambuf_eptr ;
char _streambuf_alloc ;
struct _iobuf *_streambuf_fp ;

#line 90 "/usr/include/CC/stream.h"
int (**_streambuf__vptr )();
} ;
int _circbuf_overflow ();
int _circbuf_underflow ();
static int (*circbuf__vtbl[])() = {
(int(*)()) _circbuf_overflow , 
(int(*)()) _circbuf_underflow , 0};

#line 141 "/usr/include/CC/stream.h"
struct whitespace { /* sizeof = 2 */
char _dummy; } ;

#line 145 "/usr/include/CC/stream.h"
extern char *oct ();
extern char *dec ();
extern char *hex ();

#line 149 "/usr/include/CC/stream.h"
extern char *chr ();
extern char *str ();
extern char *form ();

#line 156 "/usr/include/CC/stream.h"
struct ostream { /* sizeof = 6 */

#line 159 "/usr/include/CC/stream.h"
struct streambuf *_ostream_bp ;
short _ostream_state ;
} ;
	/* overload _lshift: */
struct ostream *_ostream__lshiftFPC__ ();
;
struct ostream *_ostream__lshiftFL__ ();
struct ostream *_ostream__lshiftFD__ ();
struct ostream *_ostream__lshiftFRCstreambuf___ ();
struct ostream *_ostream__lshiftFRCwhitespace___ ();
struct ostream *_ostream__lshiftFRCcommon___ ();

#line 170 "/usr/include/CC/stream.h"
struct ostream *_ostream_put ();

#line 184 "/usr/include/CC/stream.h"
	/* overload _ctor: */
;
;
;

#line 209 "/usr/include/CC/stream.h"
struct istream { /* sizeof = 12 */

#line 212 "/usr/include/CC/stream.h"
struct streambuf *_istream_bp ;
struct ostream *_istream_tied_to ;
char _istream_skipws ;
short _istream_state ;
} ;

#line 224 "/usr/include/CC/stream.h"
	/* overload _rshift: */
struct istream *_istream__rshiftFPC__ ();
struct istream *_istream__rshiftFRC__ ();
struct istream *_istream__rshiftFRS__ ();
struct istream *_istream__rshiftFRI__ ();
struct istream *_istream__rshiftFRL__ ();
struct istream *_istream__rshiftFRF__ ();
struct istream *_istream__rshiftFRD__ ();
struct istream *_istream__rshiftFRCstreambuf___ ();
struct istream *_istream__rshiftFRCwhitespace___ ();
struct istream *_istream__rshiftFRCcommon___ ();

#line 238 "/usr/include/CC/stream.h"
	/* overload get: */
struct istream *_istream_getFPC_I_I__ ();
struct istream *_istream_getFRCstreambuf__I__ ();
;

#line 249 "/usr/include/CC/stream.h"
struct istream *_istream_putback ();

#line 262 "/usr/include/CC/stream.h"
	/* overload _ctor: */
;

#line 270 "/usr/include/CC/stream.h"
;

#line 280 "/usr/include/CC/stream.h"
;

#line 217 "/usr/include/CC/stream.h"
extern int eatwhite ();

#line 290 "/usr/include/CC/stream.h"
extern struct istream cin ;
extern struct ostream cout ;
extern struct ostream cerr ;

#line 294 "/usr/include/CC/stream.h"
extern struct whitespace WS ;

#line 8 "/usr/include/CC/errno.h"
extern int errno ;

#line 4 "/usr/include/CC/math.h"
extern int errno ;

#line 4 "/usr/include/CC/math.h"
extern int signgam ;

#line 6 "/usr/include/CC/math.h"
extern int abs ();
extern double atof ();

#line 7 "/usr/include/CC/math.h"
extern double frexp ();
extern double ldexp ();

#line 8 "/usr/include/CC/math.h"
extern double modf ();
extern double j0 ();

#line 9 "/usr/include/CC/math.h"
extern double j1 ();

#line 9 "/usr/include/CC/math.h"
extern double jn ();
extern double y0 ();

#line 10 "/usr/include/CC/math.h"
extern double y1 ();

#line 10 "/usr/include/CC/math.h"
extern double yn ();
extern double erf ();

#line 11 "/usr/include/CC/math.h"
extern double erfc ();
extern double exp ();

#line 12 "/usr/include/CC/math.h"
extern double log ();

#line 12 "/usr/include/CC/math.h"
extern double log10 ();
extern double pow ();

#line 13 "/usr/include/CC/math.h"
extern double sqrt ();
extern double floor ();

#line 14 "/usr/include/CC/math.h"
extern double ceil ();

#line 14 "/usr/include/CC/math.h"
extern double fmod ();

#line 14 "/usr/include/CC/math.h"
extern double fabs ();
extern double gamma ();
extern double hypot ();

#line 18 "/usr/include/CC/math.h"
extern int matherr ();
extern double sinh ();

#line 19 "/usr/include/CC/math.h"
extern double cosh ();

#line 19 "/usr/include/CC/math.h"
extern double tanh ();
extern double sin ();

#line 20 "/usr/include/CC/math.h"
extern double cos ();

#line 20 "/usr/include/CC/math.h"
extern double tan ();
extern double asin ();

#line 21 "/usr/include/CC/math.h"
extern double acos ();

#line 21 "/usr/include/CC/math.h"
extern double atan ();

#line 21 "/usr/include/CC/math.h"
extern double atan2 ();

#line 62 "/usr/include/CC/math.h"
struct exception { /* sizeof = 20 */
int _exception_type ;
char *_exception_name ;
double _exception_arg1 ;
double _exception_arg2 ;
double _exception_retval ;
} ;

#line 17 "./complex.h"
;

#line 19 "./complex.h"
struct complex { /* sizeof = 8 */
double _complex_re ;

#line 20 "./complex.h"
double _complex_im ;
} ;

#line 53 "./complex.h"
int _complex__asplus ();
int _complex__asminus ();
int _complex__asmul ();
int _complex__asdiv ();

#line 51 "./complex.h"
extern int _neFCcomplex__Ccomplex___ ();

#line 50 "./complex.h"
extern int _eqFCcomplex__Ccomplex___ ();

#line 49 "./complex.h"
extern struct complex _divFCcomplex__Ccomplex___ ();

#line 48 "./complex.h"
extern struct complex _mulFCcomplex__Ccomplex___ ();

#line 47 "./complex.h"
extern struct complex _minusFCcomplex__Ccomplex___ ();

#line 46 "./complex.h"
extern struct complex _minusFCcomplex___ ();

#line 44 "./complex.h"
;

#line 42 "./complex.h"
extern struct complex sqrtFCcomplex___ ();

#line 41 "./complex.h"
extern struct complex sinhFCcomplex___ ();

#line 40 "./complex.h"
extern struct complex sinFCcomplex___ ();

#line 39 "./complex.h"
extern struct complex polar ();

#line 38 "./complex.h"
extern struct complex powFCcomplex__Ccomplex___ ();

#line 37 "./complex.h"
extern struct complex powFCcomplex__D__ ();

#line 36 "./complex.h"
extern struct complex powFCcomplex__I__ ();

#line 35 "./complex.h"
extern struct complex powFD_Ccomplex___ ();

#line 34 "./complex.h"
extern struct complex logFCcomplex___ ();

#line 33 "./complex.h"
extern struct complex expFCcomplex___ ();

#line 32 "./complex.h"
extern struct complex coshFCcomplex___ ();

#line 31 "./complex.h"
extern struct complex cosFCcomplex___ ();

#line 30 "./complex.h"
extern struct complex conj ();

#line 29 "./complex.h"
extern double arg ();

#line 28 "./complex.h"
extern double norm ();

#line 27 "./complex.h"
extern double absFCcomplex___ ();

#line 25 "./complex.h"
extern double imag ();

#line 24 "./complex.h"
extern double real ();

#line 59 "./complex.h"
struct ostream *_lshiftFRCostream__Ccomplex___ ();
struct istream *_rshiftFRCistream__RCcomplex___ ();

#line 62 "./complex.h"
extern int errno ;

#line 64 "./complex.h"
;

#line 69 "./complex.h"
;

#line 81 "./complex.h"
;

#line 86 "./complex.h"
;

#line 91 "./complex.h"
;

#line 96 "./complex.h"
;

#line 101 "./complex.h"
;

#line 106 "./complex.h"
;

#line 112 "./complex.h"
;

#line 119 "./complex.h"
static struct complex complex_zero ;

#line 122 "./complex.h"
struct c_exception { /* sizeof = 32 */
int _c_exception_type ;
char *_c_exception_name ;
struct complex _c_exception_arg1 ;
struct complex _c_exception_arg2 ;
struct complex _c_exception_retval ;
} ;

#line 34 "./complex.h"
extern struct complex logFCcomplex___ ();

#line 32 "./complex.h"
extern struct complex coshFCcomplex___ ();

#line 41 "./complex.h"
extern struct complex sinhFCcomplex___ ();

#line 33 "./complex.h"
extern struct complex expFCcomplex___ ();

#line 133 "./complex.h"
extern int complex_error ();

#line 3 "com.c"
extern char *sbrk ();

#line 44 "./complex.h"
static struct complex _plusFCcomplex__Ccomplex___ ();

#line 5 "com.c"
int main (){ _main(); 
#line 6 "com.c"
{ 
#line 7 "com.c"
struct complex _auto_c1 ;
struct complex _auto_c2 ;
struct complex _auto_c3 ;
struct complex _auto_c4 ;

#line 13 "com.c"
int _auto_i ;

#line 14 "com.c"
struct complex _auto__X_V2__plusFCcomplex__Ccomplex____global ;

#line 7 "com.c"
( ( (((struct complex *)(& _auto_c1 ))-> _complex_re = ((double )0 )), ( (((struct complex *)(& _auto_c1 ))-> _complex_im = ((double
#line 7 "com.c"
)0 )), ((struct complex *)(& _auto_c1 ))) ) ) ;
( ( (((struct complex *)(& _auto_c2 ))-> _complex_re = ((double )0 )), ( (((struct complex *)(& _auto_c2 ))-> _complex_im = ((double
#line 8 "com.c"
)0 )), ((struct complex *)(& _auto_c2 ))) ) ) ;
( ( (((struct complex *)(& _auto_c3 ))-> _complex_re = ((double )0 )), ( (((struct complex *)(& _auto_c3 ))-> _complex_im = ((double
#line 9 "com.c"
)0 )), ((struct complex *)(& _auto_c3 ))) ) ) ;
( ( (((struct complex *)(& _auto_c4 ))-> _complex_re = ((double )0 )), ( (((struct complex *)(& _auto_c4 ))-> _complex_im = ((double
#line 10 "com.c"
)0 )), ((struct complex *)(& _auto_c4 ))) ) ) ;

#line 12 "com.c"
printf ( (char *)"Brk before loop = %x\n", sbrk ( (int )0 ) ) ;
for(_auto_i = 0 ;_auto_i < 10000 ;_auto_i ++ ) { 
#line 14 "com.c"
_auto_c4 = _plusFCcomplex__Ccomplex___ ( ( ( ( ( (((struct complex *)(& _auto__X_V2__plusFCcomplex__Ccomplex____global ))->
#line 14 "com.c"
_complex_re = ((double )(_auto_c3 . _complex_re + _auto_c2 . _complex_re ))), ( (((struct complex *)(& _auto__X_V2__plusFCcomplex__Ccomplex____global ))-> _complex_im = ((double )(_auto_c3 . _complex_im +
#line 14 "com.c"
_auto_c2 . _complex_im ))), ((struct complex *)(& _auto__X_V2__plusFCcomplex__Ccomplex____global ))) ) ) , _auto__X_V2__plusFCcomplex__Ccomplex____global ) ) , _auto_c1 ) ;
}
printf ( (char *)"c1=%f.%f c2=%f.%f c3=%f.%f\n", _auto_c1 , _auto_c2 , _auto_c3 ) ;
printf ( (char *)"Brk after loop = %x\n", sbrk ( (int )0 ) ) ;
}
};

#line 19 "com.c"
extern int _STIcom_c_ ()
#line 119 "./complex.h"
{ ( ( (((struct complex *)(& complex_zero ))-> _complex_re = ((double )0 )), ( (((struct complex *)(&
#line 119 "./complex.h"
complex_zero ))-> _complex_im = ((double )0 )), ((struct complex *)(& complex_zero ))) ) ) ;
}
;

#line 44 "./complex.h"
static struct complex _plusFCcomplex__Ccomplex___ (_auto_a1 , _auto_a2 )
#line 44 "./complex.h"
struct complex _auto_a1 ;

#line 44 "./complex.h"
struct complex _auto_a2 ;

#line 45 "./complex.h"
{ 
#line 45 "./complex.h"
struct complex _auto__X_V2__plusFCcomplex__Ccomplex____global ;
{ return ( ( ( (((struct complex *)(& _auto__X_V2__plusFCcomplex__Ccomplex____global ))-> _complex_re = ((double )(_auto_a1 . _complex_re + _auto_a2 . _complex_re ))),
#line 45 "./complex.h"
( (((struct complex *)(& _auto__X_V2__plusFCcomplex__Ccomplex____global ))-> _complex_im = ((double )(_auto_a1 . _complex_im + _auto_a2 . _complex_im ))), ((struct complex *)(& _auto__X_V2__plusFCcomplex__Ccomplex____global ))) )
#line 45 "./complex.h"
) , _auto__X_V2__plusFCcomplex__Ccomplex____global ) ;
} }

/* the end */
