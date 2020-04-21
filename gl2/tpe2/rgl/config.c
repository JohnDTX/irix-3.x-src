/*
 * V Kernel - Copyright (c) 1982 by David Cheriton
 * (Transliterated from Zed and Verex Kernel)
 *
 * Device configuration table
 */
#include "gl.h"
#include "config.h"
#include "Vdevtypes.h"
#include "printf.h"

extern SystemCode EnetCreate();
extern SystemCode SerialCreate();
extern SystemCode ConsoleCreate();
extern SystemCode GECreate();
extern SystemCode WinCreate();
extern SystemCode I488Create();
extern SystemCode PxdCreate();


SystemCode NullCreate()
{
    fprintf(duputchar,"NullCreate\n");
}

DeviceConfigTable DeviceCreationTable[MAX_DEV_TYPE] = { 
    EnetCreate, ETHERNET,
    NullCreate, MOUSE,
    SerialCreate, SERIAL,
    ConsoleCreate, CONSOLE,
    NullCreate, GEDEVICE,
    NullCreate, RETRACEDEVICE,
    WinCreate,WINDOW,
    I488Create,IEEE488,
    PxdCreate,IBMPXD,
};
