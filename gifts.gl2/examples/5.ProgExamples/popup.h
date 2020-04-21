/* "popup.h" */
#ifndef POPUPHEADER
#define POPUPHEADER

#define BLACKDRAW	0
#define GREENDRAW	1
#define REDDRAW 	2
#define YELLOWDRAW 	3

typedef struct {
    short type;
    char *text;
} popupentry;

#endif
