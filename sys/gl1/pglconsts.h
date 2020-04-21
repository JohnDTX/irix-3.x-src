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

(* pglconsts.h - const section for Pascal graphics library.
                 This includes all the constants normally found
                 in gl.h
*)

    MAXARRAY = 1023;
    MAXRASTER = 4095;
    MAXOBJECTS = 4095;

(* maximum X and Y screen coordinates *)

     XMAXSCREEN = 1023;
     YMAXSCREEN  = 767;

(* approximate interval between retraces in milliseconds *)

     RETRACEINTERVAL     = 33.333333;

(* names for colors in color map loaded by ginit() *)

     BLACK       = 0;
     RED         = 1;
     GREEN       = 2;
     YELLOW      = 3;
     BLUE        = 4;
     MAGENTA     = 5;
     CYAN        = 6;
     WHITE       = 7;
