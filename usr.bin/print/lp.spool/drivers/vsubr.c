/**************************************************************************
 *									  *
 * 		 Copyright (C) 1986, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include "vcmd.h"

#define VESCAPE 0x9b

/*********************************************************/
/*		Mode changing and remote functions
/*********************************************************/

static int printmode[] = {VPRINT,600,0};
static int plotmode[] = {VPLOT,600,0};
static int printplotmode[] = {VPRINTPLOT,600,0};

/* put the plotter into print mode */
vers_printmode (f)
    int f;
{
    ioctl (f,VSETSTATE,printmode);
}

/* put the plotter into plot mode */
vers_plotmode (f)
    int f;
{
    ioctl (f,VSETSTATE,plotmode);
}

/* put the plotter into printplot mode */
vers_printplotmode (f)
    int f;
{
    ioctl (f,VSETSTATE,printplotmode);
}

vers_formfeed (f)
    int f;
{
    write (f,"\014",2);
}

/*********************************************************/
/*		Miscellaneous paramaterized control
/*********************************************************/

/* s is the speed in .125 inches per second	*/
vers_speed (f,s)
    int f;
    unsigned short s;
{
    static unsigned char buf[] = { VESCAPE, 'S', 0, 2, 0, 0 };

    buf[4] = s>>8;
    buf[5] = s;
    write (f,buf,6);
}

vers_preamble (f,length,control)
    int f;
    unsigned short length;
    unsigned char control;
{
    static unsigned char buf[] = { VESCAPE, 'P', 0, 4, 0, 0, 0, 0 };

    buf[4] = length>>8;
    buf[5] = length;
    buf[6] = control;
    write (f,buf,8);
}

/*********************************************************/
/*		Miscellaneous simple control
/*********************************************************/

vers_command (f,cmd)
    int f;
    unsigned char cmd;
{
    static unsigned char buf[] = { VESCAPE, 0, 0, 0 };

    buf[1] = cmd;
    write (f,buf,4);
}

vers_clear_buffer (f)
    int f;
{
    vers_command (f,'C');
}

vers_line_enhance (f)
    int f;
{
    vers_command (f,'E');
}

vers_inverse_image (f)
    int f;
{
    vers_command (f,'I');
}

vers_mirror_image (f)
    int f;
{
    vers_command (f,'M');
}

vers_100dpi (f)
    int f;
{
    vers_command (f,'R');
}

vers_rewind (f)
    int f;
{
    vers_command (f,'W');
}

vers_disable_reset (f)
    int f;
{
    vers_command (f,'>');
}

vers_enable_reset (f)
    int f;
{
    vers_command (f,'<');
}
