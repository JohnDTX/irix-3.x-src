#define	ZVERSION	1

struct zmsg {
	char dst[6];
	char src[6];
	short etype;
	char version;
	char ztype;
	char zmsg[16];
};

typedef struct zmsg * ZMSG;

