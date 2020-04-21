# include "iriskeybd.h"
# include "duart.h"

beep()
{
    msdelay(43);	/*20000*/
    putcraw(ShortBeep|ClickDisable,SCREEN);
}
