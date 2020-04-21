/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
#include "shmem.h"
#include "window.h"
#include "glerror.h"

/* Every error causes a call to gl_ErrorHandler.  The parameters include
 * an error number, a string, and a severity code.  The string may be
 * empty, and is typically used when the error needs to return more than
 * just the error number -- for example, in an out of memory error, the
 * routine name causing the error is passed.
 *
 * The error array contains, for each error, a number, a string and a
 * method.  The string is the canonical error message associated
 * with the error (and may be null), and the method is a
 * function that knows how to create the appropriate error
 * message for that sort of error.
 */

typedef struct {
    int		errornumber;
    char 	*(*method)();
    char	*message;
} errorentry;

char	*standarderr(), *nameproc(), *badnumber(), *fatalerr();
static char temperrorstring[100];

static errorentry errorarray[] = {
    {ERR_SINGMATRIX,	standarderr,	"singular matrix"},
    {ERR_OUTMEM,	nameproc,	"out of memory"},
    {ERR_NEGSIDES,	nameproc,	"negative or zero number of sides"},
    {ERR_BADWINDOW,	nameproc,	"illegal window parameters"},
    {ERR_NOOPENOBJ,	nameproc,	"no open object"},
    {ERR_NOFONTRAM,	nameproc,	"out of font memory"},
    {ERR_FOV,		standarderr,	"fov not in range [2, 1800]"},
    {ERR_BASISID,	nameproc,	"undefined basis id"},
    {ERR_NEGINDEX,	nameproc,	"index < 0"},
    {ERR_NOCLIPPERS,	standarderr,	"no clippers turned on"},
    {ERR_STRINGBUG,	fatalerr,	"string bug"},
    {ERR_NOCURVBASIS,	nameproc,	"should call curvbasis first"},
    {ERR_BADCURVID,	nameproc,	"id = 0 or already defined"},
    {ERR_NOPTCHBASIS,	standarderr,	"must call patchbasis first"},
    {ERR_FEEDPICK,	nameproc,	"no feedback in pick mode"},
    {ERR_INPICK,	nameproc,	"already in pick mode"},
    {ERR_NOTINPICK,	nameproc,	"not in pick mode"},
    {ERR_ZEROPICK,	standarderr,	"picksize of zero"},
    {ERR_FONTBUG,	fatalerr,	"fontaddr not found"},
    {ERR_INRGB,		nameproc,	"you are in RGB mode"},
    {ERR_NOTINRGB,	nameproc,	"you are not in RGB mode"},
    {ERR_BADINDEX,	badnumber,	"illegal index"},
    {ERR_BADVALUATOR,	badnumber,	"illegal valuator"},
    {ERR_BADBUTTON,	badnumber,	"illegal button"},
    {ERR_NOTDBMODE,	nameproc,	"not in doublebuffer mode"},
    {ERR_BADINDEXBUG,	fatalerr,	"bad max index"},
    {ERR_ZEROVIEWPORT,	standarderr,	"zero size viewport"},
    {ERR_DIALBUG,	fatalerr,	"dial init error"},
    {ERR_MOUSEBUG,	fatalerr,	"mouse init error"},
    {ERR_RETRACEBUG,	fatalerr,	"retrace init error"},
    {ERR_MAXRETRACE,	standarderr,	"too many retrace events"},
    {ERR_NOSUCHTAG,	badnumber,	"no such tag"},
    {ERR_DELBUG,	fatalerr,	"bug in string list"},
    {ERR_DELTAG,	fatalerr,	"bug -- object messed up"},
    {ERR_NEGTAG,	nameproc,	"negative tag"},
    {ERR_TAGEXISTS,	nameproc,	"tag already exists"},
    {ERR_OFFTOOBIG,	standarderr,	"offset too big"},
    {ERR_ILLEGALID,	nameproc,	"illegal id"},
    {ERR_GECONVERT,	badnumber,	"bad GE conversion"},
    {ERR_BADAXIS,	standarderr,	"illegal axis"},
    {ERR_BADDEVICE,	badnumber,	"bad device"},
    {ERR_PATCURVES,	standarderr,	"pat curves"},
    {ERR_PATPREC,	standarderr,	"pat prec"},
    {ERR_CURVPREC,	standarderr,	"curve pre"},
    {ERR_PUSHATTR,	standarderr,	"attribute stack overflow"},
    {ERR_POPATTR,	standarderr,	"attribute stack underflow"},
    {ERR_PUSHMATRIX,	standarderr,	"matrix stack overflow"},
    {ERR_POPMATRIX,	standarderr,	"matrix stack underflow"},
    {ERR_PUSHVIEWPORT,	standarderr,	"viewport stack overflow"},
    {ERR_POPVIEWPORT,	standarderr,	"viewport stack underflow"},
    {ERR_SIZEFIXED,	standarderr,	"chunk size is frozen"},
    {ERR_SETMONITOR,	standarderr,	"illegal setmonitor parameter"},
    {ERR_CHANGEINDEX0,	nameproc,	"can't change index 0"},
    {ERR_BADPATTERN,	badnumber,	"illegal pattern size"},
    {ERR_BADCURSOR,	nameproc,	"undefined cursor"},
    {ERR_FONTHOLES,	nameproc,	"internal fonthole error"},
    {ERR_REPLACE,	nameproc,	"illegal replace"},
    {ERR_STARTFEED,	nameproc,	"bad feedback buffer size: fatal"},
    {ERR_CYCLEMAP,	standarderr,	"illegal map number"},
    {ERR_TAGINREPLACE,	nameproc,	"can't make tag in replace mode"},
    {ERR_TOOFEWPTS,	standarderr,	"too few points for a curve"},
    {0,			standarderr,	"unknown error"}  /* last entry */
};

void gl_ErrorHandler (errno, severity, str, arg0, arg1, arg2, arg3)
long errno, severity;
char *str;
long arg0, arg1, arg2, arg3;
{
    char 	gl_temp[100];
    register	i;
    char	*errorstring;
    register struct inputchan *ic = getic();

    if(!ic)
	return;
    if (ic->ic_errordevice == 0)
	return;
    if ((ic->ic_doqueue & DQ_ERRORS) && (errno > 0)) {
	qenter(GERROR, errno);
	return;
    }
    if (errno < 0)	/* errno < 0 means signalerror() was called */
	errno = -errno;
    for (i = 0; errorarray[i].errornumber; i++) {
	if (errorarray[i].errornumber == errno) 
	    break;
    }
    errorstring = (*errorarray[i].method)(errno, str, errorarray[i].message);
printf(errorstring);
printf("\n");
    if (ic->ic_errordevice)	{ /* fix me later */
	return;
    }
}

static char *standarderr(num, userstring, errorstring)
int num;
char *userstring, *errorstring;
{
    return errorstring;
}

static char *fatalerr(num, userstring, errorstring)
int num;
char *userstring, *errorstring;
{
    temperrorstring[0] = '\000';
    strcat(temperrorstring, "fatal error: ");
    strcat(temperrorstring, errorstring);
    return temperrorstring;
}

static char *nameproc(num, userstring, errorstring)
int num;
char *userstring, *errorstring;
{
    temperrorstring[0] = '\000';
    strcat(temperrorstring, errorstring);
    strcat(temperrorstring, " in ");
    strcat(temperrorstring, userstring);
    return temperrorstring;
}

/* call errors having the following method as:
 * ErrorHandler(n, "procname: 4", severity), where '4' is the
 * illegal index, bad button number, or whatever.
 */

static char *badnumber(num, userstring, errorstring)
int num;
char *userstring, *errorstring;
{
    temperrorstring[0] = '\000';
    strcat(temperrorstring, userstring);
    strcat(temperrorstring, ": ");
    strcat(temperrorstring, errorstring);
    return temperrorstring;
}

